#include "NodeFunctionCall.h"
#include "CompilerState.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeProgram.h"
#include "SymbolFunction.h"
#include "SymbolFunctionName.h"
#include "CallStack.h"


namespace MFM {

  NodeFunctionCall::NodeFunctionCall(Token tok, SymbolFunction * fsym, CompilerState & state) : Node(state), m_functionNameTok(tok), m_funcSymbol(fsym) {}

  NodeFunctionCall::~NodeFunctionCall()
  {
    //may need to delete the nodes
    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	delete m_argumentNodes[i];
      }
    m_argumentNodes.clear();
  }


  void NodeFunctionCall::printPostfix(File * fp)
  {
    fp->write(" (");
    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	m_argumentNodes[i]->printPostfix(fp);
      }

    fp->write(" )");
    fp->write(getName());
  }


  const char * NodeFunctionCall::getName()
  {
    return m_state.getDataAsString(&m_functionNameTok).c_str();
  }


  const std::string NodeFunctionCall::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeFunctionCall::checkAndLabelType()
  {
    UTI it = Nav;  // init return type
    u32 numErrorsFound = 0;

    //might be related to m_currentSelf..???
    bool saveUseMemberBlock = m_state.m_useMemberBlock; //doesn't apply to arguments

    //look up in class block, and match argument types to parameters
    //assert(m_funcSymbol == NULL);
    SymbolFunction * funcSymbol = NULL;
    Symbol * fnsymptr = NULL;

    //label argument types; used to pinpoint the exact function symbol in case of overloading
    std::vector<UTI> argTypes;
    u32 constantArgs = 0;

    if(m_state.isFuncIdInClassScope(m_functionNameTok.m_dataindex,fnsymptr))
      {
	m_state.m_useMemberBlock = false;   //doesn't apply to arguments!

	for(u32 i = 0; i < m_argumentNodes.size(); i++)
	  {
	    UTI argtype = m_argumentNodes[i]->checkAndLabelType();  //plus side-effect
	    argTypes.push_back(argtype);
	    // track constants and potential casting to be handled
	    if(m_state.isConstant(argtype)) 
	      constantArgs++;
	  }
	
	// still need to pinpoint the SymbolFunction for m_funcSymbol! currently requires exact match
	// (let constant match any size of same type)
	if(!((SymbolFunctionName *) fnsymptr)->findMatchingFunction(argTypes, funcSymbol))
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.m_pool.getDataAsString(m_functionNameTok.m_dataindex).c_str() << "> has no defined function with " << m_argumentNodes.size() << " matching argument types, and cannot be called";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    numErrorsFound++;
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) <" << m_state.m_pool.getDataAsString(m_functionNameTok.m_dataindex).c_str() << "> is not a defined function, and cannot be called";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	numErrorsFound++;
      }
    

    if(!numErrorsFound)
      {
	if(m_funcSymbol == NULL)
	  m_funcSymbol = funcSymbol;

	assert(m_funcSymbol == funcSymbol);

	it = m_funcSymbol->getUlamTypeIdx();
	setNodeType(it);

	// insert casts of constant args, now that we have a "matching" function symbol
	if(constantArgs > 0)
	  {
	    u32 argsWithCast = 0;
	    for(u32 i = 0; i < m_argumentNodes.size(); i++)
	      {
		if(m_state.isConstant(argTypes[i]))
		  {
		    Symbol * psym = m_funcSymbol->getParameterSymbolPtr(i);
		    UTI ptype = psym->getUlamTypeIdx();
		    m_argumentNodes[i] = new NodeCast(m_argumentNodes[i], ptype, m_state);
		    m_argumentNodes[i]->setNodeLocation(getNodeLocation());
		    m_argumentNodes[i]->checkAndLabelType();
		    argsWithCast++;
		  }
	      }
	    assert(argsWithCast == constantArgs);
	  }
      }
   
    m_state.m_useMemberBlock = saveUseMemberBlock; //doesn't apply to arguments; restore

    return it;
  }


  // since functions are defined at the class-level; a function call
  // must be PRECEDED by a member selection (element or quark) --- a
  // local variable instance that provides the storage (i.e. atom) for
  // its data members on the STACK, as the first argument.
  EvalStatus NodeFunctionCall::eval()
  {
    assert(m_funcSymbol);
    NodeBlockFunctionDefinition * func = m_funcSymbol->getFunctionNode();
    assert(func);

    // before processing arguments, get the "self" atom ptr,
    // so that arguments will be relative to it, and not the possible 
    // selected member instance this function body could effect.
    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*********
    m_state.m_currentObjPtr = m_state.m_currentSelf;

    evalNodeProlog(0); //new current frame pointer on node eval stack
    u32 argsPushed = 0;
    EvalStatus evs;

    // place values of arguments on call stack (reverse order) before calling function
    for(s32 i= m_argumentNodes.size() - 1; i >= 0; i--)
      {
	UTI argType = m_argumentNodes[i]->getNodeType();

	// extra slot for a Ptr to unpacked array;
	// arrays are handled by CS/callstack, and passed by value
	u32 slots = makeRoomForNodeType(argType); //for eval return
	
	evs = m_argumentNodes[i]->eval();
	if(evs != NORMAL)
	  {
	    evalNodeEpilog();
	    return evs;
	  }

	// transfer to call stack
	if(slots==1)
	  {
	    UlamValue auv = m_state.m_nodeEvalStack.popArg();
	    m_state.m_funcCallStack.pushArg(auv);
	    argsPushed++;
	  }
	else
	  {
	    //array
	    PACKFIT packed = m_state.determinePackable(argType);
	    assert(WritePacked(packed));

	    //array to transfer without reversing order again
	    u32 baseSlot = m_state.m_funcCallStack.getRelativeTopOfStackNextSlot();
	    argsPushed  += makeRoomForNodeType(argType, STACK);

	    //both either unpacked or packed
	    UlamValue basePtr = UlamValue::makePtr(baseSlot, STACK, argType, packed, m_state);

	    //positive to current frame pointer  
	    UlamValue auvPtr = UlamValue::makePtr(1, EVALRETURN, argType, packed, m_state); 

	    m_state.assignValue(basePtr, auvPtr);
	    m_state.m_nodeEvalStack.popArgs(slots);
	  }
      } //done with args


    //before pushing return slot(s) last (on both STACKS for now) 
    UTI rtnType = m_funcSymbol->getUlamTypeIdx();
    s32 rtnslots = makeRoomForNodeType(rtnType);

    // insert "first" hidden arg (adjusted index pointing to atom);
    // atom index (negative) relative new frame, includes ALL the pushed args, 
    // and upcoming rtnslots: current_atom_index - relative_top_index (+ returns) 
    m_state.m_currentObjPtr = saveCurrentObjectPtr;  // RESTORE *********
    UlamValue atomPtr = m_state.m_currentObjPtr;              //*********
    if(saveCurrentObjectPtr.getPtrStorage() == STACK)
      {
	//adjust index if on the STACK, not for Event Window site
	s32 nextslot = m_state.m_funcCallStack.getRelativeTopOfStackNextSlot();
	s32 atomslot = atomPtr.getPtrSlotIndex();
	s32 adjustedatomslot = atomslot - (nextslot + rtnslots + 1); //negative index; 1 more for atomPtr
	atomPtr.setPtrSlotIndex(adjustedatomslot);
      }
    // push the "hidden" first arg, and update the current object ptr (restore later)
    m_state.m_funcCallStack.pushArg(atomPtr);                 //*********
    argsPushed++;
    m_state.m_currentObjPtr = atomPtr;                        //*********

    UlamValue saveSelf = m_state.m_currentSelf;      // restore upon return from func *****
    m_state.m_currentSelf = m_state.m_currentObjPtr; // set for subsequent func calls *****

    //(con't) push return slot(s) last (on both STACKS for now) 
    makeRoomForNodeType(rtnType, STACK);

    assert(rtnslots == m_state.slotsNeeded(rtnType));

    //********************************************    
    //*  FUNC CALL HERE!!
    //*
    evs = func->eval();   //NodeBlockFunctionDefinition..
    if(evs != NORMAL)
      {
	assert(evs != RETURN);
	m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args and return slots on callstack
	m_state.m_currentObjPtr = saveCurrentObjectPtr;    //restore current object ptr *************
	m_state.m_currentSelf = saveSelf;                  //restore previous self      *****
	evalNodeEpilog();
	return evs;
      }
    //*
    //**********************************************

    // ANY return value placed on the STACK by a Return Statement,
    // was copied to EVALRETURN by the NodeBlockFunctionDefinition
    // before arriving here! And may be ignored at this point.
 
    //positive to current frame pointer; pos is (BITSPERATOM - rtnbitsize * rtnarraysize)
    UlamValue rtnPtr = UlamValue::makePtr(1, EVALRETURN, rtnType, m_state.determinePackable(rtnType), m_state);

    assignReturnValueToStack(rtnPtr);                     //into return space on eval stack;

    m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args and return slots on callstack

    m_state.m_currentObjPtr = saveCurrentObjectPtr;       //restore current object ptr *************
    m_state.m_currentSelf = saveSelf;                     //restore previous self      *************
    evalNodeEpilog();                                     //clears out the node eval stack
    return NORMAL;
  }


  EvalStatus NodeFunctionCall::evalToStoreInto()
  {
    std::ostringstream msg;
    msg << "Use of function calls as lefthand values is not currently supported. Save the results of <" << m_state.m_pool.getDataAsString(m_functionNameTok.m_dataindex).c_str() << "> to a variable, type: <" << m_state.getUlamTypeNameBriefByIndex(getNodeType()).c_str() << ">";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    assert(!isStoreIntoAble());
    return ERROR;
  }


  void NodeFunctionCall::addArgument(Node * n)
  {
    //check types later
    m_argumentNodes.push_back(n);
    return;
  }


  u32 NodeFunctionCall::getNumberOfArguments()
  {
    return m_argumentNodes.size();
  }


  bool NodeFunctionCall::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_funcSymbol;
    return true;
  }


  void NodeFunctionCall::genCode(File * fp)
  {
    // before processing arguments, get the "self" symbol
    // so that arguments will be relative to it, and not the possible 
    // selected member instance this function body could effect.
    Symbol * saveCurrentObjectSymbol = m_state.m_currentObjSymbolForCodeGen; //*********
    m_state.m_currentObjSymbolForCodeGen = m_state.m_currentSelfSymbolForCodeGen;


    fp->write(m_funcSymbol->getMangledName().c_str());
    fp->write("(");

    //fp->write(HIDDEN_ARG_NAME);  //first arg
    if(m_state.m_currentObjSymbolForCodeGen == saveCurrentObjectSymbol)
      fp->write(m_state.getHiddenArgName());
    else
      fp->write(m_state.m_currentObjSymbolForCodeGen->getMangledName().c_str());


    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	//if(i>0)
	  fp->write(", ");

	m_argumentNodes[i]->genCode(fp);	
      }
    fp->write(")");


    m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol;  // RESTORE *********
    Symbol * saveSelf = m_state.m_currentSelfSymbolForCodeGen;      // restore upon return from func *****
    m_state.m_currentSelfSymbolForCodeGen = m_state.m_currentObjSymbolForCodeGen; // set for subsequent func calls *****

    //func called..

    m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol; //restore current object ptr *************
    m_state.m_currentSelfSymbolForCodeGen = saveSelf;               //restore previous self      *************
  } //codeGen


  std::string NodeFunctionCall::genCodeReadIntoATmpVar(File * fp)
  {
    Symbol * saveCurrentObjectSymbol = m_state.m_currentObjSymbolForCodeGen; //*********
    m_state.m_currentObjSymbolForCodeGen = m_state.m_currentSelfSymbolForCodeGen;

    std::string tmpVar;
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    m_state.indent(fp);    
    tmpVar = "UH_tmp_loadable";  //???
    fp->write(nut->getImmediateTypeAsString(&m_state).c_str()); //e.g. u32, s32, u64, etc.
    fp->write(" ");
    
    fp->write(tmpVar.c_str());
    fp->write(" = ");

    // name of func
    fp->write(m_funcSymbol->getMangledName().c_str());
    fp->write("(");

    //if(m_state.m_currentObjSymbolForCodeGen->isDataMember())
    if(m_state.m_currentObjSymbolForCodeGen == saveCurrentObjectSymbol)
      fp->write(m_state.getHiddenArgName());
    else
      fp->write(m_state.m_currentObjSymbolForCodeGen->getMangledName().c_str());

    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	//if(i>0)
	fp->write(", ");
	m_argumentNodes[i]->genCode(fp);	
      }

    fp->write(");\n");

    m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol;  // RESTORE *********
    Symbol * saveSelf = m_state.m_currentSelfSymbolForCodeGen;      // restore upon return from func *****
    m_state.m_currentSelfSymbolForCodeGen = m_state.m_currentObjSymbolForCodeGen; // set for subsequent func calls *****

    //func called..

    m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol; //restore current object ptr *************
    m_state.m_currentSelfSymbolForCodeGen = saveSelf;               //restore previous self      *************

    return tmpVar;
  } //genCodeReadIntoATmpVar

} //end MFM
