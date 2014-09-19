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

    //look up in class block, and match argument types to parameters
    //assert(m_funcSymbol == NULL);
    SymbolFunction * funcSymbol = NULL;
    Symbol * fnsymptr = NULL;
    if(m_state.isFuncIdInClassScope(m_functionNameTok.m_dataindex,fnsymptr))
      {
	//label argument types; used to pinpoint the exact function symbol in case of overloading
	std::vector<UTI> argTypes;

	for(u32 i = 0; i < m_argumentNodes.size(); i++)
	  {
	    UTI argtype = m_argumentNodes[i]->checkAndLabelType();  //plus side-effect
	    argTypes.push_back(argtype);
	  }
	
	// still need to pinpoint the SymbolFunction for m_funcSymbol! currently requires exact match
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
      }
    
    setNodeType(it);
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

    // before processing arguments, get the hidden atom ptr off the top
    // of the call stack, in order to adjust its index to be relative (negative)
    // to the upcoming new frame pointer (including all arg and return slots),
    // and return to the stack as the "first" arg.
    //UlamValue atomPtr = m_state.m_funcCallStack.popArg();  //could be packed!

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
    u32 rtnslots = makeRoomForNodeType(rtnType);

    // insert "first" hidden arg (adjusted index pointing to atom);
    // atom index (negative) relative new frame, includes all the pushed args, 
    // and upcoming rtnslots: current_atom_index - relative_top_index (+ returns) 
    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*********
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

    //(con't) push return slot(s) last (on both STACKS for now) 
    makeRoomForNodeType(rtnType, STACK);

    u32 rtnarraysize = m_state.getArraySize(rtnType);
    PACKFIT rtnpacked = m_state.determinePackable(rtnType);
    if(rtnType == Void)
      assert(rtnslots == 0);
    else
      {
	if(WritePacked(rtnpacked))
	  assert(rtnslots == 1);
	else
	  assert(rtnarraysize > 0 ? rtnslots == rtnarraysize : rtnslots == 1);
      }

    //********************************************    
    //*  FUNC CALL HERE!!
    //*
    evs = func->eval();   //NodeBlockFunctionDefinition..
    if(evs != NORMAL)
      {
	assert(evs != RETURN);
	m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args and return slots on callstack
	m_state.m_currentObjPtr = saveCurrentObjectPtr;    //restore current object ptr *************
	evalNodeEpilog();
	return evs;
      }
    //*
    //**********************************************

    // ANY return value placed on the STACK by a Return Statement,
    // was copied to EVALRETURN by the NodeBlockFunctionDefinition
    // before arriving here! And may be ignored at this point.
 
    //positive to current frame pointer; pos is (BITSPERATOM - rtnbitsize * rtnarraysize)
    UlamValue rtnPtr = UlamValue::makePtr(1, EVALRETURN, rtnType, rtnpacked, m_state);

    assignReturnValueToStack(rtnPtr);                     //into return space on eval stack;

    m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args and return slots on callstack

    m_state.m_currentObjPtr = saveCurrentObjectPtr;       //restore current object ptr *************
    evalNodeEpilog();                                     //clears out the node eval stack
    return NORMAL;
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
    fp->write(m_funcSymbol->getMangledName().c_str());
    fp->write("(");
    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	if(i>0)
	  fp->write(", ");

	m_argumentNodes[i]->genCode(fp);	
      }
    fp->write(")");

  }
} //end MFM
