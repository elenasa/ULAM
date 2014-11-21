#include "NodeFunctionCall.h"
#include "CompilerState.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeProgram.h"
#include "SymbolFunction.h"
#include "SymbolFunctionName.h"
#include "SymbolVariableDataMember.h"
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

    //might be related to m_currentSelfPtr?
    //member selection doesn't apply to arguments
    bool saveUseMemberBlock = m_state.m_useMemberBlock; 

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
		    //m_argumentNodes[i] = new NodeCast(m_argumentNodes[i], ptype, m_state);
		    //m_argumentNodes[i]->setNodeLocation(getNodeLocation());
		    //m_argumentNodes[i]->checkAndLabelType();
		    m_argumentNodes[i] = makeCastingNode(m_argumentNodes[i], ptype);
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
    m_state.m_currentObjPtr = m_state.m_currentSelfPtr;

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

    UlamValue saveSelfPtr = m_state.m_currentSelfPtr;      // restore upon return from func *****
    m_state.m_currentSelfPtr = m_state.m_currentObjPtr; // set for subsequent func calls *****

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
	m_state.m_currentSelfPtr = saveSelfPtr;                  //restore previous self      *****
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
    m_state.m_currentSelfPtr = saveSelfPtr;                     //restore previous self      *************
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


  // during genCode of a single function body "self" doesn't change!!!
  // note: uvpass arg is not equal to m_currentObjPtr; it is blank.
  void NodeFunctionCall::genCode(File * fp, UlamValue& uvpass)
  {
    // generate for value
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    //wiped out by arg processing; needed to determine owner of called function
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;

    //UTI selfuti = m_state.m_currentSelfSymbolForCodeGen->getUlamTypeIdx();
    //assert(m_state.m_currentObjPtr.getPtrTargetType() == uvpass.getPtrTargetType());
    //assert(m_state.m_currentObjPtr.getPtrNameId() == uvpass.getPtrNameId());

    std::ostringstream arglist;

    // presumably there's no = sign.., and no open brace for tmpvars
#ifdef TMPVARBRACES
    if(nuti == Void)
      {
	m_state.indent(fp);
	fp->write("{\n");    //open for tmpvar arg's
	m_state.m_currentIndentLevel++;
      }
#endif

    //"hidden" first arg
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	arglist << m_state.getHiddenArgName();
      }
    else if(isCurrentObjectsContainingAnElementParameter())
      {
	
	// here, cos is symbol used to determine read method: either self or last of cos.
	// stgcos is symbol used to determine first "hidden" arg
	Symbol * cos = NULL;
	Symbol * stgcos = NULL;
	if(m_state.m_currentObjSymbolsForCodeGen.empty())
	  {
	    stgcos = cos = m_state.m_currentSelfSymbolForCodeGen;
	  }
	else
	  {	
	    cos = m_state.m_currentObjSymbolsForCodeGen.back();
	    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
	  }

	UTI stgcosuti = stgcos->getUlamTypeIdx();
	UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
	ULAMCLASSTYPE stgcosclasstype = stgcosut->getUlamClass();		
     
	Symbol * css = m_state.m_currentSelfSymbolForCodeGen;
	UTI selfuti = css->getUlamTypeIdx();
	UlamType * selfut = m_state.getUlamTypeByIndex(selfuti);
	arglist << selfut->getUlamTypeMangledName(&m_state).c_str();
	arglist << "<CC>::THE_INSTANCE.";
	arglist << stgcos->getMangledName().c_str();

	//element parameter (could be array?)
	if(cos->isDataMember() && !cos->isElementParameter())
	  {	    
	    if(stgcosclasstype == UC_ELEMENT)
	      {
		arglist << ".GetBits()";
	      }
	    else if(stgcosclasstype == UC_QUARK)
	      {
		arglist << ".getBits()"; 
	      }
	    else
	      {
		//NOT A CLASS
	      }
	  }
	else //default
	  {
	    if(stgcosclasstype == UC_QUARK)
	      {
		arglist << ".m_stg"; //the T storage within the struct for immediate quarks
	      }
	  }
      }
    else  //local var
      { 
	Symbol * stgcos = NULL;
	if(m_state.m_currentObjSymbolsForCodeGen.empty())
	  {
	    stgcos = m_state.m_currentSelfSymbolForCodeGen;
	  }
	else
	  {	
	    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
	  }

	arglist << stgcos->getMangledName().c_str();
	
	UTI stgcosuti = stgcos->getUlamTypeIdx();       
	if(m_state.getUlamTypeByIndex(stgcosuti)->getUlamClass() == UC_QUARK)
	  {
	    arglist << ".m_stg"; //the T storage within the struct for immediate quarks
	  }
      }
    
    // since non-datamember variables can modify globals, save/restore before/after each
    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	UlamValue auvpass;
	UTI auti;
	UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************
	//Symbol * saveCurrentObjectSymbol = m_state.m_currentObjSymbolForCodeGen;
	m_state.m_currentObjSymbolsForCodeGen.clear(); //*************

	m_argumentNodes[i]->genCode(fp, auvpass);
	Node::genCodeConvertATmpVarIntoBitVector(fp, auvpass);

	auti = auvpass.getUlamValueTypeIdx();
	if(auti == Ptr)
	  {
	    auti = auvpass.getPtrTargetType();
	  }

	arglist << ", " << m_state.getTmpVarAsString(auti, auvpass.getPtrSlotIndex(), auvpass.getPtrStorage()).c_str();

	m_state.m_currentObjPtr = saveCurrentObjectPtr;  //restore current object ptr ***
	//m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol; //restore *******
      } //next arg..
    

    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector;  //restore vector after args*************
    
    //non-void return value saved in a tmp BitValue; depends on return type
    m_state.indent(fp);
    if(nuti != Void)
      {
	u32 pos = 0;   //POS 0 rightjustified;
	if(nut->getUlamClass() == UC_NOTACLASS)
	  {
	    s32 wordsize = nut->getTotalWordSize();
	    pos = wordsize - nut->getTotalBitSize();
	  }

	s32 rtnSlot = m_state.getNextTmpVarNumber();

	u32 selfid = 0;
	if(m_state.m_currentObjSymbolsForCodeGen.empty())
	  selfid = m_state.m_currentSelfSymbolForCodeGen->getId();  //a use for CSS
	else
	  selfid = m_state.m_currentObjSymbolsForCodeGen[0]->getId();

	uvpass = UlamValue::makePtr(rtnSlot, TMPBITVAL, nuti, m_state.determinePackable(nuti), m_state, pos, selfid); //POS adjusted for BitVector, rightjustified; self id in Ptr;

	// put result of function call into a variable;
	// (C turns it into the copy constructor)
	fp->write("const ");
	fp->write(nut->getImmediateStorageTypeAsString(&m_state).c_str()); //BitVector<32>
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, rtnSlot, TMPBITVAL).c_str());
	fp->write(" = ");
      } //not void return


    // static functions..oh yeah.
    //who's function is it? can we use m_cos' type
    if(!isCurrentObjectALocalVariableOrArgument())
      genMemberNameOfMethod(fp, m_state.m_currentObjPtr);
    else if(isCurrentObjectsContainingAnElementParameter())
      genElementParameterMemberNameOfMethod(fp);
    else
      genLocalMemberNameOfMethod(fp, m_state.m_currentObjPtr);

    fp->write(m_funcSymbol->getMangledName().c_str());
    fp->write("(");
    fp->write(arglist.str().c_str());
    fp->write(");\n");

    if(nuti != Void)
      {
	Node::genCodeConvertABitVectorIntoATmpVar(fp, uvpass);
      }

#ifdef TMPVARBRACES
    if(nuti == Void)
      {
	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("}\n");  //close for tmpVar
      }
#endif

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //codeGen


  // during genCode of a single function body "self" doesn't change!!!
  void NodeFunctionCall::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    return genCode(fp,uvpass);
  } //codeGenToStoreInto


  // overrides Node in case of memberselect genCode
  void NodeFunctionCall::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    return;  //no-op
  }

  void NodeFunctionCall::genMemberNameOfMethod(File * fp, UlamValue uvpass)
  {
    assert(!isCurrentObjectALocalVariableOrArgument());

    //nothing if current object is self
    //if(m_state.m_currentObjSymbolsForCodeGen.empty())
    //  return;

    //iterate over COS vector; empty if current object is self
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(u32 i = 0; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
      }

    //NOT FOR Funccalls
    //if last cos is a quark, for Read/WriteRaw to work it needs an
    // atomic Parameter type (i.e. Up_Us);
  } //genMemberNameOfMethod


  void NodeFunctionCall::genLocalMemberNameOfMethod(File * fp, UlamValue uvpass)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[0]; 

    UTI uti = cos->getUlamTypeIdx();
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    ULAMCLASSTYPE classtype = ut->getUlamClass();

    if(!ut->isScalar())
      {    //?? can't call a func on an array!
	assert(0);
      }

    //local (static functions)
    // if local element, first arg of read is all that's req'd for static func
    if(classtype == UC_QUARK)
      {
	fp->write(ut->getImmediateStorageTypeAsString(&m_state).c_str());
	fp->write("::");
	fp->write("Us::");   //typedef
      }

    for(u32 i = 1; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
      } 
  } //genLocalMemberNameOfMethod


  void NodeFunctionCall::genElementParameterMemberNameOfMethod(File * fp)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[0]; 

    UTI uti = cos->getUlamTypeIdx();
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    ULAMCLASSTYPE classtype = ut->getUlamClass();

    if(!ut->isScalar())
      {    //?? can't call a func on an array!
	assert(0);
      }

    if(classtype == UC_QUARK)
      {
	fp->write(ut->getImmediateStorageTypeAsString(&m_state).c_str());
	fp->write("::");
	fp->write("Us::");   //typedef
      }
    else if(classtype == UC_ELEMENT)  //??
      {
	fp->write(ut->getUlamTypeMangledName(&m_state).c_str()); 
	fp->write("<CC>::");
      }

    for(u32 i = 1; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	ULAMCLASSTYPE sclasstype = sut->getUlamClass();
	if(sym->isElementParameter())
	  {
	    if(sclasstype == UC_NOTACLASS)
	      {
		fp->write(sym->getMangledName().c_str());
		fp->write(".");
	      }
	    else
	      {
		fp->write(sut->getImmediateStorageTypeAsString(&m_state).c_str());
		fp->write("::");
		if( ((i + 1) < cosSize))  //still another cos refiner, use
		  fp->write("Us::");   //typedef	    
	      }
	  }
	else
	  {
	    fp->write(sym->getMangledNameForParameterType().c_str());
	    fp->write("::");
	  }
      }
  } //genElementParamenterMemberNameOfMethod

} //end MFM
