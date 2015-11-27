#include "NodeFunctionCall.h"
#include "CompilerState.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolFunction.h"
#include "SymbolFunctionName.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "CallStack.h"


namespace MFM {

  NodeFunctionCall::NodeFunctionCall(Token tok, SymbolFunction * fsym, CompilerState & state) : Node(state), m_functionNameTok(tok), m_funcSymbol(fsym), m_argumentNodes(NULL)
  {
    m_argumentNodes = new NodeList(state);
    assert(m_argumentNodes);
    m_argumentNodes->setNodeLocation(tok.m_locator); //same as func call
  }

  NodeFunctionCall::NodeFunctionCall(const NodeFunctionCall& ref) : Node(ref), m_functionNameTok(ref.m_functionNameTok), m_funcSymbol(NULL), m_argumentNodes(NULL)
  {
    m_argumentNodes = (NodeList *) ref.m_argumentNodes->instantiate();
  }

  NodeFunctionCall::~NodeFunctionCall()
  {
    delete m_argumentNodes;
    m_argumentNodes = NULL;
  }

  Node * NodeFunctionCall::instantiate()
  {
    return new NodeFunctionCall(*this);
  }

  void NodeFunctionCall::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_argumentNodes->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeFunctionCall::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    return m_argumentNodes->exchangeKids(oldnptr, newnptr);
  } //exchangeKids

  bool NodeFunctionCall::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;

    if(m_argumentNodes->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeFunctionCall::printPostfix(File * fp)
  {
    fp->write(" (");
    m_argumentNodes->printPostfix(fp);
    fp->write(" )");
    fp->write(getName());
  } //printPostfix

  const char * NodeFunctionCall::getName()
  {
    return m_state.getTokenDataAsString(&m_functionNameTok).c_str();
  }

  const std::string NodeFunctionCall::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeFunctionCall::safeToCastTo(UTI newType)
  {
    //ulamtype checks for complete, non array, and type specific rules
    return m_state.getUlamTypeByIndex(newType)->safeCast(getNodeType());
  } //safeToCastTo

  UTI NodeFunctionCall::checkAndLabelType()
  {
    UTI it = Nav;  // init return type
    u32 numErrorsFound = 0;

    //might be related to m_currentSelfPtr?
    //member selection doesn't apply to arguments
    //look up in class block, and match argument types to parameters
    SymbolFunction * funcSymbol = NULL;
    Symbol * fnsymptr = NULL;

    //label argument types; used to pinpoint the exact function symbol in case of overloading
    std::vector<UTI> argTypes;
    std::vector<Node *> constArgs;
    u32 constantArgs = 0;
    u32 navArgs = 0;
    UTI listuti = Nav;
    bool hazyKin = false;

    if(m_state.isFuncIdInClassScope(m_functionNameTok.m_dataindex,fnsymptr, hazyKin) && !hazyKin)
      {
        //use member block doesn't apply to arguments; no change to current block
	m_state.pushCurrentBlockAndDontUseMemberBlock(m_state.getCurrentBlock()); //set forall args
	listuti = m_argumentNodes->checkAndLabelType(); //plus side-effect; void return is ok

	u32 numargs = getNumberOfArguments();
	for(u32 i = 0; i < numargs; i++)
	  {
	    UTI argtype = m_argumentNodes->getNodeType(i); //plus side-effect
	    argTypes.push_back(argtype);
	    if(argtype == Nav)
	      navArgs++;
	    // track constants and potential casting to be handled
	    if(m_argumentNodes->isAConstant(i))
	      {
		constArgs.push_back(m_argumentNodes->getNodePtr(i));
		constantArgs++;
	      }
	    else
	      constArgs.push_back(NULL);
	  }
	m_state.popClassContext(); //restore here

	if(navArgs)
	  {
	    argTypes.clear();
	    constArgs.clear();
	    setNodeType(Nav);
	    return Nav; //short circuit
	  }

	// still need to pinpoint the SymbolFunction for m_funcSymbol!
	// exact match if possible; o.w. allow safe casts to find matches
	bool hasHazyArgs = false;
	u32 numFuncs = ((SymbolFunctionName *) fnsymptr)->findMatchingFunctionWithConstantsAsArgs(argTypes, constArgs, funcSymbol, hasHazyArgs);
	if(numFuncs == 0)
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.getTokenDataAsString(&m_functionNameTok).c_str();
	    msg << "> has no defined function with " << numargs;
	    msg << " matching argument type";
	    if(numargs != 1)
	      msg << "s";
	    msg << ": ";
	    for(u32 i = 0; i < argTypes.size(); i++)
	      {
		msg << m_state.getUlamTypeNameBriefByIndex(argTypes[i]).c_str() << ", ";
	      }
	    msg << "and cannot be called";
	    if(hasHazyArgs)
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    numErrorsFound++;
	  }
	else if(numFuncs > 1)
	  {
	    std::ostringstream msg;
	    msg << "Ambiguous matches (" << numFuncs << ") of function <";
	    msg << m_state.getTokenDataAsString(&m_functionNameTok).c_str();
	    msg << "> called with " << numargs << " argument type";
	    if(numargs != 1)
	      msg << "s";
	    msg << ": ";
	    for(u32 i = 0; i < argTypes.size(); i++)
	      {
		msg << m_state.getUlamTypeNameBriefByIndex(argTypes[i]).c_str() << ", ";
	      }
	    msg << "explicit casting is required";
	    if(hasHazyArgs)
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    numErrorsFound++;
	  }
	else //==1
	  {
	    if(hasHazyArgs)
	      numErrorsFound++; //wait to cast
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) <" << m_state.getTokenDataAsString(&m_functionNameTok).c_str();
	msg << "> is not a defined function, and cannot be called";
	if(hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	numErrorsFound++;
      }

    if(m_funcSymbol && m_funcSymbol != funcSymbol)
      {
	std::ostringstream msg;
	if(funcSymbol)
	  {
	    msg << "Substituting <" << funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "> for <" << m_funcSymbol->getMangledNameWithTypes().c_str() <<">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_funcSymbol = funcSymbol;
	  }
      }

    if(numErrorsFound == 0)
      {
	if(m_funcSymbol == NULL)
	  m_funcSymbol = funcSymbol;

	assert(m_funcSymbol && m_funcSymbol == funcSymbol);

	it = m_funcSymbol->getUlamTypeIdx();
	setNodeType(it);

	// insert safe casts of complete arg types, now that we have a "matching" function symbol
	{
	  std::vector<u32> argsWithCastErr;
	  u32 argsWithCast = 0;
	  u32 numParams = m_funcSymbol->getNumberOfParameters();
	  for(u32 i = 0; i < numParams; i++)
	    {
	      Symbol * psym = m_funcSymbol->getParameterSymbolPtr(i);
	      UTI ptype = psym->getUlamTypeIdx();
	      Node * argNode = m_argumentNodes->getNodePtr(i); //constArgs[i];
	      UTI atype = argNode->getNodeType();
	      if(UlamType::compare(ptype, atype, m_state) == UTIC_NOTSAME) //o.w. known same
		{
		  Node * argCast = NULL;
		  if(!Node::makeCastingNode(argNode, ptype, argCast))
		    {
		      argsWithCastErr.push_back(i); //error!
		    }
		  m_argumentNodes->exchangeKids(argNode, argCast, i);
		  argsWithCast++;
		}
	    }

	  // do similar casting on any variable arg constants (without parameters)
	  if(m_funcSymbol->takesVariableArgs())
	    {
	      u32 numargs = getNumberOfArguments();
	      for(u32 i = numParams; i < numargs; i++)
		  {
		    if(constArgs[i])
		      {
			Node * argNode = constArgs[i];//m_argumentNodes->getNodePtr(i);
			Node * argCast = NULL;
			if(!Node::makeCastingNode(argNode, m_state.getDefaultUlamTypeOfConstant(argTypes[i]), argCast))
			  {
			    argsWithCastErr.push_back(i); //error!
			  }
			m_argumentNodes->exchangeKids(argNode, argCast, i);
			argsWithCast++;
		      }
		  }
	    } //var args

	  if(!argsWithCastErr.empty())
	      {
		std::ostringstream msg;
		msg << "Casting errors for args with constants: " ;
		for(u32 i = 0; i < argsWithCastErr.size(); i++)
		  {
		    if(i > 0)
		      msg << ", ";
		    msg << i + 1;
		  }

		msg << " to function <";
		msg << m_state.getTokenDataAsString(&m_functionNameTok).c_str() <<">";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		argsWithCastErr.clear();
	      }
	} //constants
      } // no errors found

    // late, important to do, but not too soon;
    // o.w. NodeIdents can't find their blocks.
    if(listuti == Nav || numErrorsFound > 0)
      {
	setNodeType(Nav); //happens when the arg list has incomplete types.
	it = Nav;
      }

    argTypes.clear();
    constArgs.clear();
    return it;
  } //checkAndLabelType

  void NodeFunctionCall::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt); //missing
    m_argumentNodes->countNavNodes(cnt);
  } //countNavNodes

  void NodeFunctionCall::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    u32 argbase = 0;
    //allot enough stack space for the function call to another func
    argbase += m_argumentNodes->getTotalSlotsNeeded(); //args assigned at eval
    argbase += m_state.slotsNeeded(getNodeType()); //return
    argbase += 1; //hidden
    depth += argbase;
  } //calcMaxDepth

  bool NodeFunctionCall::isFunctionCall()
  {
    return true;
  }

  // since functions are defined at the class-level; a function call
  // must be PRECEDED by a member selection (element or quark) --- a
  // local variable instance that provides the storage (i.e. atom) for
  // its data members on the STACK, as the first argument.
  EvalStatus NodeFunctionCall::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

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

    // for now we're going to bypass variable arguments for eval purposes
    // since our NodeFunctionDef has no way to know how many extra args to expect!
    u32 numargs = getNumberOfArguments();
    s32 diffInArgs = numargs - m_funcSymbol->getNumberOfParameters();
    assert(diffInArgs == 0 || m_funcSymbol->takesVariableArgs()); //sanity check

    // place values of arguments on call stack (reverse order) before calling function
    for(s32 i= numargs - diffInArgs - 1; i >= 0; i--)
      {
	UTI argType = m_argumentNodes->getNodeType(i);

	// extra slot for a Ptr to unpacked array;
	// arrays are handled by CS/callstack, and passed by value
	u32 slots = makeRoomForNodeType(argType); //for eval return

	evs = m_argumentNodes->eval(i);
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
    m_state.m_currentObjPtr = saveCurrentObjectPtr; // RESTORE *********
    UlamValue atomPtr = m_state.m_currentObjPtr; //*********

    //update func def (at eval time) based on class in virtual table
    // this requires symbol search like used in c&l and parsing;
    // t.f. we use the classblock stack to track the block ST's
    if(m_funcSymbol->isVirtualFunction())
      {
	u32 vtidx = m_funcSymbol->getVirtualMethodIdx();
	u32 atomid = atomPtr.getPtrNameId();
	if(atomid != 0)
	  {
	    //if auto local as, set shadowed lhs type (, and pos?)
	    Symbol * asym = NULL;
	    bool hazyKin = false;
	    if(m_state.alreadyDefinedSymbol(atomid, asym, hazyKin) && !hazyKin)
	      {
		ALT autolocaltype = asym->getAutoLocalType();
		if(autolocaltype == ALT_AS) //must be a class
		  {
		    /*
		    NodeBlock * currblock = m_state.getCurrentBlock();
		    assert(currblock);
		    NodeBlock * prevblock = currblock->getPreviousBlockPointer();
		    assert(prevblock);
		    m_state.pushCurrentBlock(prevblock);

		    Symbol * shadsym = NULL; //same name in some prev block
		    assert(m_state.alreadyDefinedSymbol(atomid, shadsym, hazyKin) && !hazyKin);

		    //RECURSE UNTIL NON-AUTO or HAS-AUTO??? refactor this!!!

		    UTI shadowtype = shadsym->getUlamTypeIdx();
		    //when autolocal, use original (lhs) auto storage to lookup class to use
		    atomPtr.setPtrTargetType(shadowtype); //what about POS? e.g. has-conditional
		    m_state.popClassContext(); //restore
		    */
		    atomPtr.setPtrTargetType(((SymbolVariableStack *) asym)->getAutoStorageTypeForEval());
		  }
		else if(autolocaltype == ALT_HAS)
		  {
		    // auto type is the type of the data member,
		    // rather than the base (rhs)
		  }
	      }
	  } //else can't be an autolocal

	UTI cuti = atomPtr.getPtrTargetType(); //must be a class
	SymbolClass * vcsym = NULL;
	assert(m_state.alreadyDefinedSymbolClass(cuti, vcsym));
	UTI vtcuti = vcsym->getClassForVTableEntry(vtidx);

	//is the virtual class uti the same as what we already have?
	NNO funcstnno = m_funcSymbol->getBlockNoOfST();
	UTI funcclassuti = m_state.findAClassByNodeNo(funcstnno);
	if(funcclassuti != vtcuti)
	  {
	    SymbolClass * vtcsym = NULL;
	    assert(m_state.alreadyDefinedSymbolClass(vtcuti, vtcsym));

	    NodeBlockClass * memberClassNode = vtcsym->getClassBlockNode();
	    assert(memberClassNode);  //e.g. forgot the closing brace on quark definition
	    //set up compiler state to use the member class block for symbol searches
	    m_state.pushClassContextUsingMemberClassBlock(memberClassNode);

	    Symbol * fnsymptr = NULL;
	    bool hazyKin = false;
	    assert(m_state.isFuncIdInClassScope(m_functionNameTok.m_dataindex, fnsymptr, hazyKin) && !hazyKin);

	    //find this func in the virtual class; get its func def.
	    std::vector<UTI> pTypes;
	    u32 numparams = m_funcSymbol->getNumberOfParameters();
	    for(u32 j = 0; j < numparams; j++)
	      {
		Symbol * psym = m_funcSymbol->getParameterSymbolPtr(j);
		assert(psym);
		pTypes.push_back(psym->getUlamTypeIdx());
	      }

	    SymbolFunction * funcSymbol = NULL;
	    u32 numFuncs = ((SymbolFunctionName *) fnsymptr)->findMatchingFunction(pTypes, funcSymbol);
	    assert(numFuncs == 1);

	    m_state.popClassContext(); //restore here

	    func = funcSymbol->getFunctionNode(); //replace with virtual function def!!!
	  } //end use virtual function
      } //end virtual function

    if(atomPtr.getPtrStorage() == STACK)
      {
	//adjust index if on the STACK, not for Event Window site
	s32 nextslot = m_state.m_funcCallStack.getRelativeTopOfStackNextSlot();
	s32 atomslot = atomPtr.getPtrSlotIndex();
	s32 adjustedatomslot = atomslot - (nextslot + rtnslots + 1); //negative index; 1 more for atomPtr
	atomPtr.setPtrSlotIndex(adjustedatomslot);
      }
    // push the "hidden" first arg, and update the current object ptr (restore later)
    m_state.m_funcCallStack.pushArg(atomPtr); //*********
    argsPushed++;
    m_state.m_currentObjPtr = atomPtr; //*********

    UlamValue saveSelfPtr = m_state.m_currentSelfPtr; // restore upon return from func *****
    m_state.m_currentSelfPtr = m_state.m_currentObjPtr; // set for subsequent func calls ****

    //(continue) push return slot(s) last (on both STACKS for now)
    makeRoomForNodeType(rtnType, STACK);

    assert(rtnslots == m_state.slotsNeeded(rtnType));

    //********************************************
    //*  FUNC CALL HERE!!
    //*
    evs = func->eval(); //NodeBlockFunctionDefinition..
    if(evs != NORMAL)
      {
	assert(evs != RETURN);
	//drops all the args and return slots on callstack
	m_state.m_funcCallStack.popArgs(argsPushed+rtnslots);
	m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr *******
	m_state.m_currentSelfPtr = saveSelfPtr; //restore previous self *****
	evalNodeEpilog();
	return evs;
      }
    //*
    //**********************************************

    // ANY return value placed on the STACK by a Return Statement,
    // was copied to EVALRETURN by the NodeBlockFunctionDefinition
    // before arriving here! And may be ignored at this point.
    if(rtnType == UAtom)
      {
	UlamValue rtnUV = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
	assignReturnValueToStack(rtnUV); //into return space on eval stack;
      }
    else
      {
	//positive to current frame pointer; pos is (BITSPERATOM - rtnbitsize * rtnarraysize)
	UlamValue rtnPtr = UlamValue::makePtr(1, EVALRETURN, rtnType, m_state.determinePackable(rtnType), m_state);
	assignReturnValueToStack(rtnPtr); //into return space on eval stack;
      }

    m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args and return slots on callstack

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr *****
    m_state.m_currentSelfPtr = saveSelfPtr; //restore previous self      *************
    evalNodeEpilog(); //clears out the node eval stack
    return NORMAL;
  } //eval

  EvalStatus NodeFunctionCall::evalToStoreInto()
  {
    std::ostringstream msg;
    msg << "Eval of function calls as lefthand values is not currently supported.";
    msg << " Save the results of <";
    msg << m_state.getTokenDataAsString(&m_functionNameTok).c_str();
    msg << "> to a variable, type: ";
    msg << m_state.getUlamTypeNameBriefByIndex(getNodeType()).c_str();
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    assert(!isStoreIntoAble());
    return ERROR;
  } //evalToStoreInto

  void NodeFunctionCall::addArgument(Node * n)
  {
    //check types later
    m_argumentNodes->addNodeToList(n);
    return;
  }

  u32 NodeFunctionCall::getNumberOfArguments()
  {
    return m_argumentNodes->getNumberOfNodes();
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
    genCodeIntoABitValue(fp, uvpass);

    if(getNodeType() != Void)
      {
	Node::genCodeConvertABitVectorIntoATmpVar(fp, uvpass); //inc uvpass slot
      }
  } //genCode

  // during genCode of a single function body "self" doesn't change!!!
  void NodeFunctionCall::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    return genCodeIntoABitValue(fp,uvpass);
  } //codeGenToStoreInto

  void NodeFunctionCall::genCodeIntoABitValue(File * fp, UlamValue& uvpass)
  {
    // generate for value
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    std::ostringstream arglist;
    // presumably there's no = sign.., and no open brace for tmpvars
    arglist << genHiddenArgs().c_str();

    //loads any variables into tmps before used as args (needs fp)
    arglist << genRestOfFunctionArgs(fp, uvpass).c_str();

    //non-void RETURN value saved in a tmp BitValue; depends on return type
    m_state.indent(fp);
    if(nuti != Void)
      {
	u32 pos = 0; //POS 0 rightjustified;
	if(nut->getUlamClass() == UC_NOTACLASS) //atom too???
	  {
	    u32 wordsize = nut->getTotalWordSize();
	    pos = wordsize - nut->getTotalBitSize();
	  }

	s32 rtnSlot = m_state.getNextTmpVarNumber();

	u32 selfid = 0;
	if(m_state.m_currentObjSymbolsForCodeGen.empty())
	  selfid = m_state.getCurrentSelfSymbolForCodeGen()->getId(); //a use for CSS
	else
	  selfid = m_state.m_currentObjSymbolsForCodeGen[0]->getId();

	uvpass = UlamValue::makePtr(rtnSlot, TMPBITVAL, nuti, m_state.determinePackable(nuti), m_state, pos, selfid); //POS adjusted for BitVector, justified; self id in Ptr;

	// put result of function call into a variable;
	// (C turns it into the copy constructor)
	fp->write("const ");
	fp->write(nut->getImmediateStorageTypeAsString().c_str()); //e.g. BitVector<32>
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, rtnSlot, TMPBITVAL).c_str());
	fp->write(" = ");
      } //not void return

    // static functions..oh yeah..but only for quarks and virtual funcs
    // who's function is it?
    if(m_funcSymbol->isVirtualFunction())
      genCodeVirtualFunctionCall(fp, uvpass); //indirect call thru func ptr
    else
      {
	if(!isCurrentObjectALocalVariableOrArgument())
	  genMemberNameOfMethod(fp);
	else
	  {
	    s32 epi = isCurrentObjectsContainingAModelParameter();
	    if(epi >= 0)
	      genModelParameterMemberNameOfMethod(fp,epi);
	    else
	      genLocalMemberNameOfMethod(fp);
	  }
	fp->write(m_funcSymbol->getMangledName().c_str());
      }

    // the arguments
    fp->write("(");
    fp->write(arglist.str().c_str());
    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeIntoABitValue

  void NodeFunctionCall::genCodeVirtualFunctionCall(File * fp, UlamValue & uvpass)
  {
    //requires runtime lookup for virtual function pointer
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = NULL; //any owner of func

    if(cosSize != 0)
      cos = m_state.m_currentObjSymbolsForCodeGen.back(); //"owner" of func
    else
      cos = m_state.getCurrentSelfSymbolForCodeGen();
    UTI cosuti = cos->getUlamTypeIdx();

    SymbolClass * csym = NULL;
    assert(m_state.alreadyDefinedSymbolClass(cosuti, csym));

    //UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    //typedef of conextual type info from any class with this function
    //Symbol * stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    //UTI stgcosuti = stgcos->getUlamTypeIdx();
    //UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    UTI cvfuti = csym->getClassForVTableEntry(m_funcSymbol->getVirtualMethodIdx());
    UlamType * cvfut = m_state.getUlamTypeByIndex(cvfuti);

    fp->write("((typename ");
    fp->write(cvfut->getUlamTypeMangledName().c_str());
    if(cvfut->getUlamClass() == UC_ELEMENT)
      fp->write("<EC>::");
    else
      {
	fp->write("<EC,");
	fp->write("T::ATOM_FIRST_STATE_BIT"); //presumes ancestor or immediate quark
	fp->write(">::");
      }
    fp->write(m_funcSymbol->getMangledNameWithTypes().c_str());
    fp->write(") ");
    fp->write("UlamElement<EC>::GetVTableEntry(");
    fp->write(genHiddenArgs().c_str());
    fp->write(", ");
    fp->write_decimal_unsigned(m_funcSymbol->getVirtualMethodIdx());
    fp->write("u)) ");
    //args to function pointer remain
  } //genCodeVirtualFunctionCall

  // overrides Node in case of memberselect genCode
  void NodeFunctionCall::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    return; //no-op
  }

  void NodeFunctionCall::genMemberNameOfMethod(File * fp)
  {
    assert(!isCurrentObjectALocalVariableOrArgument());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    u32 startcos = 0;

    Symbol * stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    UTI stgcosuti = stgcos->getUlamTypeIdx(); //more general instead of current class

    // use NodeNo for inheritance
    NNO cosBlockNo = m_funcSymbol->getBlockNoOfST();
    NNO stgcosBlockNo = stgcos->getBlockNoOfST(); //m_state.getAClassBlockNo(stgcosuti);

    Symbol * cos = NULL;
    UTI cosuti = Nav;
    if(cosSize != 0)
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back(); //"owner" of func
	cosuti = cos->getUlamTypeIdx();
	if(!m_state.isClassASubclass(cosuti))
	  cosBlockNo = cos->getBlockNoOfST(); //compare owner and self
	else
	  stgcosBlockNo = cos->getBlockNoOfST();
      }

    if(cosBlockNo != stgcosBlockNo)
      {
	s32 subcos = Node::isCurrentObjectsContainingASubClass();
	if(subcos >= 0)
	  {
	    startcos = subcos + 1;

	    UTI cosclassuti = Node::findTypeOfSubClassAndBlockNo(cosBlockNo, subcos);
	    UlamType * cosclassut = m_state.getUlamTypeByIndex(cosclassuti);

	    fp->write(cosclassut->getUlamTypeMangledName().c_str());
	    if(cosclassut->getUlamClass() == UC_ELEMENT)
	      fp->write("<EC>::THE_INSTANCE.");
	    else
	      {
		fp->write("<EC,");
		fp->write("T::ATOM_FIRST_STATE_BIT");
		fp->write(">::");
	      }
	  }
	else if(m_state.isClassASubclass(stgcosuti)) //self is subclass
	  {
	    Node * foundnode = m_state.findNodeNoInAClass(cosBlockNo, stgcosuti);
	    assert(foundnode);
	    UTI superuti = foundnode->getNodeType();
	    UlamType * superut = m_state.getUlamTypeByIndex(superuti);
	    fp->write(superut->getUlamTypeMangledName().c_str());
	    if(superut->getUlamClass() == UC_ELEMENT)
	      {
		fp->write("<EC>::THE_INSTANCE.");
		startcos = cosSize; //bypass all that follows
	      }
	    else
	      {
		//self is a quark
		fp->write("<EC,");
		fp->write("T::ATOM_FIRST_STATE_BIT");
		fp->write(">::");
	      }
	  }
	//else do nothing for inheritance
      }

    //iterate over COS vector; empty if current object is self
    for(u32 i = startcos; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(sym->isSelf())
	  continue;
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
      }

    //NOT FOR Funccalls
    //if last cos is a quark, for Read/Write to work it needs an
    // atomic Parameter type (i.e. Up_Us);
  } //genMemberNameOfMethod

  void NodeFunctionCall::genModelParameterMemberNameOfMethod(File * fp, s32 epi)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    assert(epi >= 0);

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[epi]; //the MP

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    ULAMCLASSTYPE cosclasstype = cosut->getUlamClass();

    // the MP:
    if(cosclasstype == UC_NOTACLASS) //atom too? atoms not MP
      {
	assert(cosut->getUlamTypeEnum() != UAtom);
	assert(cosut->getUlamTypeEnum() != Void);
	fp->write(cos->getMangledName().c_str());
	fp->write(".");
      }
    else
      {
	assert(0); //no longer may MP be an element or a quark type
	if(cosclasstype == UC_ELEMENT)
	  {
	    fp->write(cosut->getUlamTypeMangledName().c_str());
	    fp->write("<EC>::");

	    //depending on the "owner" of the func, the instance is needed
	    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
	    UTI cosuti = cos->getUlamTypeIdx();
	    if(m_state.getUlamTypeByIndex(cosuti)->getUlamClass() == UC_ELEMENT)
	      fp->write("THE_INSTANCE."); //non-static functions require an instance
	  }
	else
	  {
	    //for immmediate quark MP..?
	    fp->write(cosut->getImmediateStorageTypeAsString().c_str());
	    fp->write("::");
	    fp->write("Us::"); //typedef, always for funccalls
	  }
      }

    u32 cosStart = epi + 1;
    for(u32 i = cosStart; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];

	//not the model parameter, but a data member..
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
	//NOT FOR Funccalls
	//  fp->write("Up_Us::");   //atomic parameter needed
      }
  } //genModelParamenterMemberNameOfMethod

  std::string NodeFunctionCall::genHiddenArgs()
  {
    std::ostringstream hiddenargs;

    // first "hidden" arg is the context
    hiddenargs << m_state.getHiddenContextArgName() << ", ";

    //"hidden" self arg
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	hiddenargs << m_state.getHiddenArgName();
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    hiddenargs << genModelParameterHiddenArgs(epi).c_str();
	  }
	else //local var
	  {
	    Symbol * stgcos = NULL;
	    if(m_state.m_currentObjSymbolsForCodeGen.empty())
	      {
		stgcos = m_state.getCurrentSelfSymbolForCodeGen();
	      }
	    else
	      {
		stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
	      }

	    hiddenargs << stgcos->getMangledName().c_str();

	    // for both immediate quarks and elements now..not self.
	    if(!stgcos->isSelf())
	      hiddenargs << ".getRef()"; //the T storage within the struct for immediate quarks
	  }
      }
    return hiddenargs.str();
  } //genHiddenArgs

  // "static" data member, a mixture of local variable and dm;
  // requires THE_INSTANCE, and local variables are superfluous.
  std::string NodeFunctionCall::genModelParameterHiddenArgs(s32 epi)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    assert(epi >= 0);

    std::ostringstream hiddenlist;
    Symbol * stgcos = NULL;

    if(epi == 0)
      stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      stgcos = m_state.m_currentObjSymbolsForCodeGen[epi - 1]; //***

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    Symbol * epcos = m_state.m_currentObjSymbolsForCodeGen[epi]; //***
    UTI epcosuti = epcos->getUlamTypeIdx();
    UlamType * epcosut = m_state.getUlamTypeByIndex(epcosuti);
    ULAMCLASSTYPE epcosclasstype = epcosut->getUlamClass();

    hiddenlist << stgcosut->getUlamTypeMangledName().c_str();
    hiddenlist << "<EC>::THE_INSTANCE.";

    // the MP (only primitive, no longer an element, or quark):
    hiddenlist << epcos->getMangledName().c_str();

    if(epcosclasstype != UC_NOTACLASS)
      {
	hiddenlist << ".getRef()";
      }
    return hiddenlist.str();
  } //genModelParameterHiddenArgs

  std::string NodeFunctionCall::genRestOfFunctionArgs(File * fp, UlamValue& uvpass)
  {
    std::ostringstream arglist;

    //wiped out by arg processing; needed to determine owner of called function
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;

    u32 numParams = m_funcSymbol->getNumberOfParameters();
    // handle any variable number of args separately
    // since non-datamember variables can modify globals, save/restore before/after each
    for(u32 i = 0; i < numParams; i++)
      {
	UlamValue auvpass;
	UTI auti;
	m_state.m_currentObjSymbolsForCodeGen.clear(); //*************

	m_argumentNodes->genCode(fp, auvpass, i);
	Node::genCodeConvertATmpVarIntoBitVector(fp, auvpass);
	auti = auvpass.getUlamValueTypeIdx();
	if(auti == Ptr)
	  {
	    auti = auvpass.getPtrTargetType();
	  }
	arglist << ", " << m_state.getTmpVarAsString(auti, auvpass.getPtrSlotIndex(), auvpass.getPtrStorage()).c_str();
      } //next arg..

    if(m_funcSymbol->takesVariableArgs())
      {
	u32 numargs = getNumberOfArguments();
	for(u32 i = numParams; i < numargs; i++)
	  {
	    UlamValue auvpass;
	    UTI auti;
	    m_state.m_currentObjSymbolsForCodeGen.clear(); //*************

	    m_argumentNodes->genCode(fp, auvpass, i);
	    Node::genCodeConvertATmpVarIntoBitVector(fp, auvpass);

	    auti = auvpass.getUlamValueTypeIdx();
	    if(auti == Ptr)
	      {
		auti = auvpass.getPtrTargetType();
	      }

	    // use pointer for variable arg's since all the same size that way
	    arglist << ", &" << m_state.getTmpVarAsString(auti, auvpass.getPtrSlotIndex(), auvpass.getPtrStorage()).c_str();
	  } //end forloop through variable number of args

	arglist << ", (void *) 0"; //indicates end of args
      } //end of handling variable arguments

    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore vector after args******

    return arglist.str();
  } //genRestOfFunctionArgs

  void NodeFunctionCall::genLocalMemberNameOfMethod(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 startcos = 1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();

    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    //if(stgcos->isSelf())
    //  return;

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    if(!stgcosut->isScalar())
      {
	//can't call a func on an array!
	std::ostringstream msg;
	msg << "Calling a function on an array is not currently supported.";
	msg << " <" << m_state.getTokenDataAsString(&m_functionNameTok).c_str();
	msg << ">, type: " << m_state.getUlamTypeNameBriefByIndex(stgcosuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	//assert(0);
      }

    // use NodeNo for inheritance
    bool useSuperClassName = false;
    NNO cosBlockNo = m_funcSymbol->getBlockNoOfST();
    NNO stgcosBlockNo = m_state.getAClassBlockNo(stgcosuti);
    if(stgcosBlockNo != cosBlockNo)
      {
	s32 subcos = Node::isCurrentObjectsContainingASubClass();
	if(subcos >= 0)
	  {
	    startcos = subcos + 1; //for loop later

	    UTI cosclassuti = Node::findTypeOfSubClassAndBlockNo(cosBlockNo, subcos);
	    stgcosuti = cosclassuti; // resets stgcosuti here!!
	    stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
	    useSuperClassName = true;
	  }
      }

    ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClass();
    if(stgclasstype == UC_ELEMENT)
      {
	fp->write(stgcosut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::");
	//depending on the "owner" of the func, the instance is needed
	Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
	UTI cosuti = cos->getUlamTypeIdx();
	if(m_state.getUlamTypeByIndex(cosuti)->getUlamClass() == UC_ELEMENT)
	  fp->write("THE_INSTANCE."); //non-static functions require an instance
      }
    else
      {
	if(useSuperClassName)
	  {
	    fp->write(stgcosut->getUlamTypeMangledName().c_str());
	    fp->write("<EC,");
	    fp->write("T::ATOM_FIRST_STATE_BIT");
	    fp->write(">::");
	  }
	else
	  {
	  //immediate quark..
	  fp->write(stgcosut->getImmediateStorageTypeAsString().c_str());
	  fp->write("::Us::"); //typedef
	  }
      }

    for(u32 i = startcos; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
      }
  } //genLocalMemberNameOfMethod

} //end MFM
