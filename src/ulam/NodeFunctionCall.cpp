#include "NodeFunctionCall.h"
#include "CompilerState.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolFunction.h"
#include "SymbolFunctionName.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "CallStack.h"

namespace MFM {

  NodeFunctionCall::NodeFunctionCall(const Token& tok, SymbolFunction * fsym, CompilerState & state) : Node(state), m_functionNameTok(tok), m_funcSymbol(fsym), m_argumentNodes(NULL), m_tmpvarSymbol(NULL)
  {
    m_argumentNodes = new NodeList(state);
    assert(m_argumentNodes);
    m_argumentNodes->setNodeLocation(tok.m_locator); //same as func call
  }

  NodeFunctionCall::NodeFunctionCall(const NodeFunctionCall& ref) : Node(ref), m_functionNameTok(ref.m_functionNameTok), m_funcSymbol(NULL), m_argumentNodes(NULL), m_tmpvarSymbol(NULL){
    m_argumentNodes = (NodeList *) ref.m_argumentNodes->instantiate();
  }

  NodeFunctionCall::~NodeFunctionCall()
  {
    delete m_argumentNodes;
    m_argumentNodes = NULL;
    delete m_tmpvarSymbol;
    m_tmpvarSymbol = NULL;
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
    return m_state.getTokenDataAsString(m_functionNameTok).c_str();
  }

  const std::string NodeFunctionCall::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeFunctionCall::safeToCastTo(UTI newType)
  {
    UlamType * newut = m_state.getUlamTypeByIndex(newType);
    //ulamtype checks for complete, non array, and type specific rules
    return newut->safeCast(getNodeType());
  } //safeToCastTo

  UTI NodeFunctionCall::checkAndLabelType()
  {
    UTI it = Nav;  // init return type
    u32 numErrorsFound = 0;
    u32 numHazyFound = 0;

    //might be related to m_currentSelfPtr?
    //member selection doesn't apply to arguments
    //look up in class block, and match argument types to parameters
    SymbolFunction * funcSymbol = NULL;
    Symbol * fnsymptr = NULL;

    //label argument types; used to pinpoint the exact function symbol in case of overloading
    std::vector<Node *> argNodes;
    u32 constantArgs = 0;
    u32 navArgs = 0;
    u32 hzyArgs = 0;
    u32 noutiArgs = 0;
    UTI listuti = Nav;
    bool hazyKin = false;

    if(m_state.isFuncIdInClassScope(m_functionNameTok.m_dataindex, fnsymptr, hazyKin) && !hazyKin)
      {
        //use member block doesn't apply to arguments; no change to current block
	m_state.pushCurrentBlockAndDontUseMemberBlock(m_state.getCurrentBlock()); //set forall args
	listuti = m_argumentNodes->checkAndLabelType(); //plus side-effect; void return is ok

	u32 numargs = getNumberOfArguments();
	for(u32 i = 0; i < numargs; i++)
	  {
	    UTI argtype = m_argumentNodes->getNodeType(i); //plus side-effect
	    argNodes.push_back(m_argumentNodes->getNodePtr(i));
	    if(argtype == Nav)
	      navArgs++;
	    else if(argtype == Hzy)
	      hzyArgs++;
	    else if(argtype == Nouti)
	      noutiArgs++;

	    // track constants and potential casting to be handled
	    if(m_argumentNodes->isAConstant(i))
	      constantArgs++;
	  }
	m_state.popClassContext(); //restore here

	if(navArgs)
	  {
	    argNodes.clear();
	    setNodeType(Nav);
	    return Nav; //short circuit
	  }

	if(hzyArgs || noutiArgs)
	  {
	    argNodes.clear();
	    setNodeType(Hzy);
	    m_state.setGoAgain(); //for compier counts
	    return Hzy; //short circuit
	  }

	// still need to pinpoint the SymbolFunction for m_funcSymbol!
	// exact match if possible; o.w. allow safe casts to find matches
	bool hasHazyArgs = false;
	u32 numFuncs = ((SymbolFunctionName *) fnsymptr)->findMatchingFunctionWithSafeCasts(argNodes, funcSymbol, hasHazyArgs);
	if(numFuncs == 0)
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.getTokenDataAsString(m_functionNameTok).c_str();
	    msg << "> has no defined function with " << numargs;
	    msg << " matching argument type";
	    if(numargs != 1)
	      msg << "s";
	    msg << ": ";
	    for(u32 i = 0; i < argNodes.size(); i++)
	      {
		UTI auti = argNodes[i]->getNodeType();
		msg << m_state.getUlamTypeNameBriefByIndex(auti).c_str() << ", ";
	      }
	    msg << "and cannot be called";
	    if(hasHazyArgs)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		numHazyFound++;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		numErrorsFound++;
	      }
	  }
	else if(numFuncs > 1)
	  {
	    std::ostringstream msg;
	    msg << "Ambiguous matches (" << numFuncs << ") of function <";
	    msg << m_state.getTokenDataAsString(m_functionNameTok).c_str();
	    msg << "> called with " << numargs << " argument type";
	    if(numargs != 1)
	      msg << "s";
	    msg << ": ";
	    for(u32 i = 0; i < argNodes.size(); i++)
	      {
		UTI auti = argNodes[i]->getNodeType();
		msg << m_state.getUlamTypeNameBriefByIndex(auti).c_str() << ", ";
	      }
	    msg << "explicit casting is required";
	    if(hasHazyArgs)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		numHazyFound++;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //t3479
		numErrorsFound++;

		((SymbolFunctionName *) fnsymptr)->noteAmbiguousFunctionSignatures(argNodes, numFuncs);
	      }
	  }
	else //==1
	  {
	    if(hasHazyArgs)
	      numHazyFound++; //wait to cast

	    //check ref types and func calls here..care with variable args (2 pass)
	    u32 numParams = funcSymbol->getNumberOfParameters();
	    for(u32 i = 0; i < numParams; i++)
	      {
		Symbol * psym = funcSymbol->getParameterSymbolPtr(i);
		assert(psym && psym->isFunctionParameter()); //sanity
		if(m_state.isAltRefType(psym->getUlamTypeIdx()))
		  {
		    TBOOL argreferable = argNodes[i]->getReferenceAble();
		    if(argreferable != TBOOL_TRUE)
		      {
			std::ostringstream msg;
			msg << "Invalid argument " << i + 1 << " to function <";
			msg << m_state.getTokenDataAsString(m_functionNameTok).c_str();
			msg << ">; Cannot be used as a reference parameter";
			if(argreferable == TBOOL_HAZY)
			  {
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
			    numHazyFound++;
			  }
			else if(!((SymbolVariableStack *) psym)->isConstantFunctionParameter())
			  {
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			    numErrorsFound++; //t41189
			  }
			//else non-referenceable arg with const ref parameter, ok
		      }
		  }
	      }
	    //don't worry about variable args in native methods; they can't be refs.
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) <" << m_state.getTokenDataAsString(m_functionNameTok).c_str();
	msg << "> is not a defined function, or cannot be safely called in this context";
	if(hazyKin)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    numHazyFound++;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    numErrorsFound++;
	  }
      }

    if(funcSymbol && m_funcSymbol != funcSymbol)
      {
	//may preceed function parameter c&l, and fail names of args with type
	//(e.g. Class isn't really a class).
	std::ostringstream msg;
	msg << "Substituting <" << m_state.m_pool.getDataAsString(funcSymbol->getId()).c_str() << "> ";
	if(m_funcSymbol)
	  msg << "for <" << m_state.m_pool.getDataAsString(m_funcSymbol->getId()).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	m_funcSymbol = funcSymbol;
      }

    if(m_funcSymbol && m_funcSymbol != funcSymbol)
      {
	std::ostringstream msg;
	if(funcSymbol)
	  {
	    msg << "Substituting <" << m_state.m_pool.getDataAsString(funcSymbol->getId()).c_str() << "> ";
	    msg << "for <" << m_state.m_pool.getDataAsString(m_funcSymbol->getId()).c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_funcSymbol = funcSymbol;
	  }
      }

    if((numErrorsFound == 0) && (numHazyFound == 0))
      {
	if(m_funcSymbol == NULL)
	  m_funcSymbol = funcSymbol;

	assert(m_funcSymbol && m_funcSymbol == funcSymbol);

	it = m_funcSymbol->getUlamTypeIdx();
	setNodeType(it);

	// insert safe casts of complete arg types, now that we have a "matching" function symbol
        //use member block doesn't apply to arguments; no change to current block
	m_state.pushCurrentBlockAndDontUseMemberBlock(m_state.getCurrentBlock()); //set forall args

	{
	  std::vector<u32> argsWithCastErr;
	  u32 argsWithCast = 0;
	  u32 numParams = m_funcSymbol->getNumberOfParameters();
	  for(u32 i = 0; i < numParams; i++)
	    {
	      Symbol * psym = m_funcSymbol->getParameterSymbolPtr(i);
	      UTI ptype = psym->getUlamTypeIdx();
	      Node * argNode = m_argumentNodes->getNodePtr(i);
	      UTI atype = argNode->getNodeType();
	      if(UlamType::compareForArgumentMatching(ptype, atype, m_state) == UTIC_NOTSAME) //o.w. known same
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
		    UTI auti = argNodes[i]->getNodeType();
		    ALT alti = m_state.getReferenceType(auti);
		    if(argNodes[i]->isAConstant())
		      {
			Node * argCast = NULL;
			if(!Node::makeCastingNode(argNodes[i], m_state.getDefaultUlamTypeOfConstant(auti), argCast))
			  {
			    argsWithCastErr.push_back(i); //error!
			  }
			m_argumentNodes->exchangeKids(argNodes[i], argCast, i);
			argsWithCast++;
		      }
		    else if(alti == ALT_ARRAYITEM)
		      {
			//array item (ALT_ARRAYITEM) is okay, with a cast to its scalar (t3250)
			//don't use makeCastingNode, since UTIC_SAME short-circuits; req'd cast
			Node * argCast = Node::newCastingNode(argNodes[i], m_state.getUlamTypeAsDeref(auti));
			assert(argCast != NULL); //cannot fail
			m_argumentNodes->exchangeKids(argNodes[i], argCast, i);
			argsWithCast++;
		      }
		    else if((alti == ALT_REF) || (alti == ALT_CONSTREF))
		      {
			//ref not allowed since doesn't share base class w non-refs (t41099)
			std::ostringstream msg;
			if(alti == ALT_CONSTREF)
			  msg << "Constant ";
			msg << "Reference Vararg: " ;
			msg << "arg_" << i + 1;
			msg << " to function <";
			msg << m_state.getTokenDataAsString(m_functionNameTok).c_str();
			msg <<">; type ";
			msg << m_state.getUlamTypeNameBriefByIndex(auti).c_str();
			msg << " is currently unsupported";
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		      }
		    //else nothing to do
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
		  msg << "arg_" << i + 1;
		}

	      msg << " to function <";
	      msg << m_state.getTokenDataAsString(m_functionNameTok).c_str() <<">";
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      argsWithCastErr.clear();
	    }
	} //constants

	m_state.popClassContext(); //restore here
      } // no errors found

    // late, important to do, but not too soon;
    // o.w. NodeIdents can't find their blocks.
    if((listuti == Nav) || (numErrorsFound > 0))
      {
	setNodeType(Nav); //happens when the arg list has erroneous types.
	it = Nav;
      }

    if((listuti == Hzy) || (numHazyFound > 0))
      {
	setNodeType(Hzy); //happens when the arg list has incomplete types.
	m_state.setGoAgain(); //for compier counts
	it = Hzy;
      }

    argNodes.clear();
    assert(it == getNodeType());
    assert(m_funcSymbol || (getNodeType() == Nav) || (getNodeType() == Hzy));

    if(m_state.okUTItoContinue(it))
      {
	bool isref = m_state.isAltRefType(it);
	if(m_state.isAClass(it) || isref)
	  setStoreIntoAble(TBOOL_TRUE); //t3912 (class); t41085,t41077 (constructors)

	if(!isref)
	  setReferenceAble(TBOOL_FALSE); //set after storeintoable t3661,2; t3630
      }
    return it;
  } //checkAndLabelType

  void NodeFunctionCall::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //missing
    m_argumentNodes->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  void NodeFunctionCall::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    u32 argbase = 0;
    //allot enough stack space for the function call to another func
    u32 numargs = m_argumentNodes->getNumberOfNodes();
    for(u32 i = 0; i < numargs; i++)
      {
	u32 depthi = 0;
	u32 nomaxdepth = 0;
	u32 nobase = 0;
	m_argumentNodes->getNodePtr(i)->calcMaxDepth(depthi, nomaxdepth, nobase); //possible func call as arg
	u32 sloti = m_state.slotsNeeded(m_argumentNodes->getNodeType(i)); //just a variable or constant
	argbase += depthi > sloti ? depthi : sloti; //take the greater
      }
    argbase += m_state.slotsNeeded(getNodeType()); //return
    argbase += 1; //hidden uc Wed Sep 21 10:44:37 2016
    argbase += 1; //hidden ur
    depth += argbase;
  } //calcMaxDepth

  bool NodeFunctionCall::isFunctionCall()
  {
    return true;
  }

  bool NodeFunctionCall::isAConstructorFunctionCall()
  {
    assert(m_funcSymbol);
    return m_funcSymbol->isConstructorFunction();
  }

  // since functions are defined at the class-level; a function call
  // must be PRECEDED by a member selection (element or quark) --- a
  // local variable instance that provides the storage (i.e. atom) for
  // its data members on the STACK, as the first argument.
  EvalStatus NodeFunctionCall::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    assert(m_funcSymbol);

    NodeBlockFunctionDefinition * func = m_funcSymbol->getFunctionNode();
    assert(func);

    UTI rtnType = m_funcSymbol->getUlamTypeIdx();
    s32 rtnslots = m_state.slotsNeeded(rtnType);
    u32 argsPushed = 0;

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*********
    UlamValue saveSelfPtr = m_state.m_currentSelfPtr; // restore upon return from func *****

    evalNodeProlog(0); //new current frame pointer on node eval stack

    EvalStatus argevs = evalArgumentsInReverseOrder(argsPushed);
    if(argevs != NORMAL) return Node::evalStatusReturn(argevs);

    EvalStatus hiddenevs = evalHiddenArguments(argsPushed, func);
    if(hiddenevs != NORMAL) return Node::evalStatusReturn(hiddenevs);

    m_state.m_currentSelfPtr = m_state.m_currentObjPtr; // set for subsequent func calls ****
    //********************************************
    //*  FUNC CALL HERE!!
    //*
    EvalStatus evs = func->eval(); //NodeBlockFunctionDefinition..
    if(evs != NORMAL)
      {
	assert(evs != RETURN); //t3896
	//drops all the args and return slots on callstack
	m_state.m_funcCallStack.popArgs(argsPushed+rtnslots);
	m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr *******
	m_state.m_currentSelfPtr = saveSelfPtr; //restore previous self *****
	return Node::evalStatusReturn(evs);
      }
    //*
    //**********************************************

    // ANY return value placed on the STACK by a Return Statement,
    // was copied to EVALRETURN by the NodeBlockFunctionDefinition
    // before arriving here! And may be ignored at this point.
    if(Node::returnValueOnStackNeededForEval(rtnType))
      {
	UlamValue rtnUV = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
	UTI rtnuti = rtnUV.getUlamValueTypeIdx();
	if(m_state.isPtr(rtnuti))
	  rtnuti = rtnUV.getPtrTargetType();
	if(UlamType::compareForUlamValueAssignment(rtnuti, rtnType, m_state) == UTIC_SAME)
	  {
	    Node::assignReturnValueToStack(rtnUV); //into return space on eval stack;
	  }
	else
	  {
	    if(m_state.isAtom(rtnuti) || m_state.isAtom(rtnType))
	      evs = UNEVALUABLE;  //t3558
	    else
	      evs = ERROR;
	  }
      }
    else
      {
	//positive to current frame pointer; pos is (BITSPERATOM - rtnbitsize * rtnarraysize)
	UlamValue rtnPtr = UlamValue::makePtr(1, EVALRETURN, rtnType, m_state.determinePackable(rtnType), m_state);
	Node::assignReturnValueToStack(rtnPtr); //into return space on eval stack;
      }

    m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args and return slots on callstack

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr *****
    m_state.m_currentSelfPtr = saveSelfPtr; //restore previous self      *************

    if(evs != NORMAL) return evalStatusReturn(evs);

    evalNodeEpilog(); //clears out the node eval stack
    return NORMAL;
  } //eval

  EvalStatus NodeFunctionCall::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    std::ostringstream msg;
    msg << "Eval of function calls as lefthand values is not currently supported.";
    msg << " Save the results of <";
    msg << m_state.getTokenDataAsString(m_functionNameTok).c_str();
    msg << "> to a variable, type: ";
    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();

    if((getStoreIntoAble() != TBOOL_TRUE) && !isAConstructorFunctionCall())
      {
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return evalErrorReturn();
      }

    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
    //need a Ptr to the auto temporary variable, the result of func call
    // that belongs in m_currentObjPtr, but where to store the ans?
    // use the hidden 'uc' slot (under the return value)

    assert(m_state.isAClass(nuti) || m_state.isAltRefType(nuti) || isAConstructorFunctionCall()); //sanity?

    assert(m_funcSymbol);
    NodeBlockFunctionDefinition * func = m_funcSymbol->getFunctionNode();
    assert(func);

    UTI rtnType = m_funcSymbol->getUlamTypeIdx();
    s32 rtnslots = m_state.slotsNeeded(rtnType);
    u32 argsPushed = 0;

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*********
    UlamValue saveSelfPtr = m_state.m_currentSelfPtr; // restore upon return from func *****

    evalNodeProlog(0); //new current frame pointer on node eval stack

    EvalStatus argevs = evalArgumentsInReverseOrder(argsPushed);
    if(argevs != NORMAL) return Node::evalStatusReturn(argevs);

    EvalStatus hiddenevs = evalHiddenArguments(argsPushed, func);
    if(hiddenevs != NORMAL) return Node::evalStatusReturn(hiddenevs);

    m_state.m_currentSelfPtr = m_state.m_currentObjPtr; // set for subsequent func calls ****
    //********************************************
    //*  FUNC CALL HERE!!
    //*
    EvalStatus evs = func->evalToStoreInto(); //NodeBlockFunctionDefinition..
    if(evs != NORMAL)
      {
	assert(evs != RETURN); //t3896
	//drops all the args and return slots on callstack
	m_state.m_funcCallStack.popArgs(argsPushed+rtnslots);
	m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr *******
	m_state.m_currentSelfPtr = saveSelfPtr; //restore previous self *****
	return Node::evalStatusReturn(evs);
      }
    //*
    //**********************************************

    // ANY return value placed on the STACK by a Return Statement,
    // was copied to EVALRETURN by the NodeBlockFunctionDefinition
    // before arriving here! And may be ignored at this point.
    if(Node::returnValueOnStackNeededForEval(rtnType))
      {
	//t3189 returns a class (non-ref);
	//t3630 return a reference to a primitive;
	UlamValue rtnUV = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
	if(rtnUV.isPtr())
	  Node::assignReturnValuePtrToStack(rtnUV); //into return space on eval stack;
	else
	  Node::assignReturnValueToStack(rtnUV); //into return space on eval stack; t3189
      }
    else if(isAConstructorFunctionCall())
      {
	//Void. t41091
      }
    else
      {
	//positive to current frame pointer; pos is (BITSPERATOM - rtnbitsize * rtnarraysize)
	UlamValue rtnPtr = UlamValue::makePtr(1, EVALRETURN, rtnType, m_state.determinePackable(rtnType), m_state);
	Node::assignReturnValuePtrToStack(rtnPtr); //into return space on eval stack;
      }

    m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args and return slots on callstack

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr *****
    m_state.m_currentSelfPtr = saveSelfPtr; //restore previous self      *************

    if(evs != NORMAL) Node::evalStatusReturn(evs);

    evalNodeEpilog(); //clears out the node eval stack
    return NORMAL;
  } //evalToStoreInto

  EvalStatus NodeFunctionCall::evalArgumentsInReverseOrder(u32& argsPushed)
  {
    // before processing arguments, get the "self" atom ptr,
    // so that arguments will be relative to it, and not the possible
    // selected member instance this function body could effect.
    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*********
    m_state.m_currentObjPtr = m_state.m_currentSelfPtr;

    EvalStatus evs = NORMAL;

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

	UTI paramType = m_funcSymbol->getParameterType(i);
	bool useparamreftype = m_state.isAltRefType(paramType);
	if(useparamreftype)
	  evs = m_argumentNodes->evalToStoreInto(i);
	else
	  evs = m_argumentNodes->eval(i);

	if(evs != NORMAL) return evalStatusReturnNoEpilog(evs); //quit!

	// transfer to call stack
	if(slots==1)
	  {
	    UlamValue auv = m_state.m_nodeEvalStack.popArg();
	    if((useparamreftype) && (auv.getPtrStorage() == STACK))
	      {
		assert(m_state.isPtr(auv.getUlamValueTypeIdx()));
		if(!auv.isPtrAbs()) //do that conversion here
		  {
		    u32 absrefslot = m_state.m_funcCallStack.getAbsoluteStackIndexOfSlot(auv.getPtrSlotIndex());
		    auv.setPtrSlotIndex(absrefslot); //t3810, t3635
		    auv.setUlamValueTypeIdx(PtrAbs);
		  }
		//else t41030 already PtrAbs (e.g. passing along a function arg)
	      }
	    m_state.m_funcCallStack.pushArg(auv);
	    argsPushed++;
	  }
	else
	  {
	    //array
	    PACKFIT packed = m_state.determinePackable(argType);

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

    m_state.m_currentObjPtr = saveCurrentObjectPtr; // RESTORE *********
    return NORMAL;
  } //evalArgumentsInReverseOrder

  EvalStatus NodeFunctionCall::evalHiddenArguments(u32& argsPushed, NodeBlockFunctionDefinition *& func)
  {
    //before pushing return slot(s) last (on both STACKS for now)
    UTI rtnType = m_funcSymbol->getUlamTypeIdx();
    s32 rtnslots = makeRoomForNodeType(rtnType);

    // insert "first" hidden arg (adjusted index pointing to atom);
    // atom index (negative) relative new frame, includes ALL the pushed args,
    // and upcoming rtnslots: current_atom_index - relative_top_index (+ returns)
    UlamValue atomPtr = m_state.m_currentObjPtr; //*********

    //update func def (at eval time) based on class in virtual table
    // this requires symbol search like used in c&l and parsing;
    // t.f. we use the classblock stack to track the block ST's
    if(m_funcSymbol->isVirtualFunction())
      {
	if(!getVirtualFunctionForEval(atomPtr, func))
	  {
	    return evalErrorReturn();
	  }
      } //end virtual function

    //adjust index if on the STACK, not for Event Window site (t3114, and 160+ more tests);
    // convert to ABSOLUTE PTR for isLocal check (t3942,6,7,8).
    if((atomPtr.getPtrStorage() == STACK) && !atomPtr.isPtrAbs())
      {
	s32 atomslot = atomPtr.getPtrSlotIndex();
	u32 absidx = m_state.m_funcCallStack.getAbsoluteStackIndexOfSlot(atomslot);
       	atomPtr.setPtrSlotIndex(absidx);
	atomPtr.setUlamValueTypeIdx(PtrAbs);
      }

    // push the "hidden" first arg, and update the current object ptr (restore later)
    m_state.m_funcCallStack.pushArg(atomPtr); //*********
    argsPushed++;
    m_state.m_currentObjPtr = atomPtr; //*********

    makeRoomForSlots(1, STACK); // uc placeholder
    argsPushed++;

    //(continue) push return slot(s) last (on both STACKS for now)
    makeRoomForNodeType(rtnType, STACK);

    assert(rtnslots == m_state.slotsNeeded(rtnType));
    return NORMAL;
  } //evalHiddenArguments

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

  bool NodeFunctionCall::getVirtualFunctionForEval(UlamValue & atomPtr, NodeBlockFunctionDefinition *& rtnfunc)
  {
    bool rtnok = true; //false if error, o.w. rtnfunc is good (changed or not)
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
	    UTI auti = asym->getUlamTypeIdx();
	    if(autolocaltype == ALT_AS) //must be a class
	      {
		atomPtr.setPtrTargetType(((SymbolVariableStack *) asym)->getAutoStorageTypeForEval());
	      }
	    else if((autolocaltype == ALT_REF) || (autolocaltype == ALT_CONSTREF))
	      {
		if(!asym->isSuper())
		  //unlike alt_as, alt_ref can be a primitive or a class
		  atomPtr.setPtrTargetType(((SymbolVariableStack *) asym)->getAutoStorageTypeForEval());
	      }
	    else if(m_state.isClassASubclassOf(auti, atomPtr.getPtrTargetType()))
	      {
		atomPtr.setPtrTargetType(auti); //t3746
	      }
	  }
      } //else can't be an autolocal

    UTI cuti = atomPtr.getPtrTargetType(); //must be a class
    SymbolClass * vcsym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, vcsym);
    assert(isDefined);
    UTI vtcuti = vcsym->getClassForVTableEntry(vtidx);

    //is the virtual class uti the same as what we already have?
    NNO funcstnno = m_funcSymbol->getBlockNoOfST();
    UTI funcclassuti = m_state.findAClassByNodeNo(funcstnno);
    assert(funcclassuti != Nouti); //sanity
    if(funcclassuti != vtcuti)
      {
	SymbolClass * vtcsym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(vtcuti, vtcsym);
	assert(isDefined);

	NodeBlockClass * memberClassNode = vtcsym->getClassBlockNode();
	assert(memberClassNode);  //e.g. forgot the closing brace on quark definition
	//set up compiler state to use the member class block for symbol searches
	m_state.pushClassContextUsingMemberClassBlock(memberClassNode);

	Symbol * fnsymptr = NULL;
	bool hazyKin = false;
	AssertBool isDefinedFunc = (m_state.isFuncIdInClassScope(m_functionNameTok.m_dataindex, fnsymptr, hazyKin) && !hazyKin);
	assert(isDefinedFunc);

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
	bool tmphazyargs = false;
	u32 numFuncs = ((SymbolFunctionName *) fnsymptr)->findMatchingFunctionStrictlyByTypes(pTypes, funcSymbol, tmphazyargs);
	assert(!tmphazyargs);

	if(numFuncs != 1)
	  {
	    std::ostringstream msg;
	    msg << "Virtual function <" << funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "> is ";
	    if(numFuncs > 1)
	      msg << "ambiguous";
	    else
	      msg << "not found";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }

	if(!funcSymbol->isVirtualFunction())
	  {
	    std::ostringstream msg;
	    msg << "Function <" << funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "> is not virtual";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnok = false;
	  }
	else if(funcSymbol->isPureVirtualFunction())
	  {
	    std::ostringstream msg;
	    msg << "Virtual function <" << funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "> is pure; cannot be called";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnok = false;
	  }

	m_state.popClassContext(); //restore here

	if(rtnok)
	  rtnfunc = funcSymbol->getFunctionNode(); //replace with virtual function def!!!
      } //end lookup virtual function
    else
      {
	if(m_funcSymbol->isPureVirtualFunction())
	  {
	    std::ostringstream msg;
	    msg << "Virtual function <" << m_funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "> is pure; cannot be called";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnok = false; //t41094
	  }
      }
    //else no change to rtnfunc
    return rtnok;
  } //getVirtualFunctionForEval

  // during genCode of a single function body "self" doesn't change!!!
  // note: uvpass arg is not equal to m_currentObjPtr; it is blank.
  void NodeFunctionCall::genCode(File * fp, UVPass& uvpass)
  {
    if(!m_funcSymbol || !m_state.okUTItoContinue(getNodeType()))
      {
	std::ostringstream msg;
	msg << "(3) <" << m_state.getTokenDataAsString(m_functionNameTok).c_str();
	msg << "> is not a fully resolved function definition; ";
	msg << "A call to it cannot be generated in this context";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	m_state.clearCurrentObjSymbolsForCodeGen();
	return;
      }

    // The Call:
    if((uvpass.getPassStorage() == TMPAUTOREF))
      genCodeAReferenceIntoABitValue(fp, uvpass);
    else
      genCodeIntoABitValue(fp, uvpass);

    // Result:
    if(getNodeType() != Void)
      {
	UTI vuti = uvpass.getPassTargetType();
	// skip reading classes and atoms
	if(m_state.getUlamTypeByIndex(vuti)->isPrimitiveType())
	  {
	    Node::genCodeConvertABitVectorIntoATmpVar(fp, uvpass); //inc uvpass slot
	  }
	//else class or atom stays as a tmpbitval (e.g. t41143)
      }
  } //genCode

  // during genCode of a single function body "self" doesn't change!!!
  void NodeFunctionCall::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    genCodeIntoABitValue(fp,uvpass);
    //return atom ref (t41031,t41033), class ref(t41030,t41032),
    // primitive ref (t41034,t41035)
    m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL);
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
  } //genCodeToStoreInto

  void NodeFunctionCall::genCodeIntoABitValue(File * fp, UVPass& uvpass)
  {
    // generate for value
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    u32 urtmpnum = 0;
    std::string hiddenarg2str = Node::genHiddenArg2(uvpass, urtmpnum);
    if(urtmpnum > 0)
      {
	m_state.indentUlamCode(fp);
	fp->write(hiddenarg2str.c_str()); GCNL;
      }

    std::ostringstream arglist;
    // presumably there's no = sign.., and no open brace for tmpvars
    arglist << genHiddenArgs(urtmpnum).c_str();

    //loads any variables into tmps before used as args (needs fp)
    arglist << genRestOfFunctionArgs(fp, uvpass).c_str();

    u32 tvfpnum = m_state.getNextTmpVarNumber();
    if(m_funcSymbol->isVirtualFunction())
      {
	genCodeVirtualFunctionCallVTableEntry(fp, tvfpnum, urtmpnum); //indirect call thru func ptr
      }

    //non-void RETURN value saved in a tmp BitValue; depends on return type
    m_state.indentUlamCode(fp);
    if(nuti != Void)
      {
	u32 pos = 0; //POS 0 leftjustified;
	//bool isref = (nut->getReferenceType() == ALT_REF); //t3946
	bool isref = m_state.isAltRefType(nuti); //t3946, t41188
	if(!isref && (nut->getUlamClassType() == UC_NOTACLASS)) //includes atom too
	  {
	    u32 wordsize = nut->getTotalWordSize();
	    pos = wordsize - nut->getTotalBitSize();
	  }

	u32 rtntmpnum = m_state.getNextTmpVarNumber();

	u32 selfid = 0;
	if(m_state.m_currentObjSymbolsForCodeGen.empty())
	  selfid = m_state.getCurrentSelfSymbolForCodeGen()->getId(); //a use for CSS
	else
	  {
	    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[0];
	    selfid = cos->getId();
	    UTI cosuti = cos->getUlamTypeIdx();
	    bool iscustomarray = m_state.isClassACustomArray(cosuti);
	    if(!iscustomarray && !Node::isCurrentObjectALocalVariableOrArgument())
	      {
		// e.g. 'self' is not a dm, nor local var or arg (t3274, t3275, t3405)
		if(cos->isDataMember())
		  pos = cos->getPosOffset(); //data member position overrides
		//else 0
	      }
	    //else local var or arg, including references and model parameter, customarray (t3223)
	  }

	uvpass = UVPass::makePass(rtntmpnum, TMPBITVAL, nuti, m_state.determinePackable(nuti), m_state, pos, selfid); //POS adjusted for BitVector, justified; self id in Pass;

	// put result of function call into a variable;
	// (C turns it into the copy constructor)
	if(getStoreIntoAble() == TBOOL_FALSE)
	  fp->write("const ");
	fp->write(nut->getLocalStorageTypeAsString().c_str()); //e.g. BitVector<32>
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, rtntmpnum, TMPBITVAL).c_str());
	fp->write(" = ");
      } //not void return

    // no longer for quarks!!
    // static functions..oh yeah..but only for quarks and virtual funcs
    // who's function is it?
    if(m_funcSymbol->isVirtualFunction())
      {
	genCodeVirtualFunctionCall(fp, tvfpnum); //indirect call thru func ptr
      }
    else
      {
	if(!Node::isCurrentObjectALocalVariableOrArgument())
	  genMemberNameOfMethod(fp);
	else
	  {
	    s32 epi = Node::isCurrentObjectsContainingAModelParameter();
	    assert(epi < 0); //model parameters no longer classes
	    genLocalMemberNameOfMethod(fp);
	  }
	fp->write(m_funcSymbol->getMangledName().c_str());
      }
    // the arguments
    fp->write("(");
    fp->write(arglist.str().c_str());
    fp->write(");"); GCNL;

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeIntoABitValue

  void NodeFunctionCall::genCodeAReferenceIntoABitValue(File * fp, UVPass& uvpass)
  {
    UVPass rtnuvpass;
    // generate for value
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    u32 urtmpnum = 0;
    std::string hiddenarg2str = genHiddenArg2ForARef(fp, uvpass, urtmpnum);

    m_state.indentUlamCode(fp);
    fp->write(hiddenarg2str.c_str()); GCNL;

    std::ostringstream arglist;
    // presumably there's no = sign.., and no open brace for tmpvars
    arglist << genHiddenArgs(urtmpnum).c_str();

    //loads any variables into tmps before used as args (needs fp)
    arglist << genRestOfFunctionArgs(fp, uvpass).c_str();

    u32 tvfpnum = m_state.getNextTmpVarNumber();
    if(m_funcSymbol->isVirtualFunction())
      {
	genCodeVirtualFunctionCallVTableEntry(fp, tvfpnum, urtmpnum); //indirect call thru func ptr
      }

    m_state.indentUlamCode(fp);
    //non-void RETURN value saved in a tmp BitValue; depends on return type
    if(nuti != Void)
      {
	u32 pos = 0; //POS 0 rightjustified;
	if(nut->getUlamClassType() == UC_NOTACLASS) //includes atom too
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

	rtnuvpass = UVPass::makePass(rtnSlot, TMPBITVAL, nuti, m_state.determinePackable(nuti), m_state, pos, selfid); //POS adjusted for BitVector, justified; self id in Pass;

	// put result of function call into a variable;
	// (C turns it into the copy constructor)
	fp->write("const ");
	fp->write(nut->getLocalStorageTypeAsString().c_str()); //e.g. BitVector<32>
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, rtnSlot, TMPBITVAL).c_str());
	fp->write(" = ");
      } //not void return

    assert(uvpass.getPassStorage() == TMPAUTOREF);
    UTI vuti = uvpass.getPassTargetType();
    //assert(m_state.getUlamTypeByIndex(vuti)->getReferenceType() != ALT_NOT); //e.g. t3668

    // who's function is it?
    if(m_funcSymbol->isVirtualFunction())
      {
	genCodeVirtualFunctionCall(fp, tvfpnum); //indirect call thru func ptr
      }
    else
      {
	fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
        fp->write(".");
	fp->write(m_funcSymbol->getMangledName().c_str());
      }

    // the arguments
    fp->write("(");
    fp->write(arglist.str().c_str());
    fp->write(");"); GCNL;

    m_state.clearCurrentObjSymbolsForCodeGen();
    uvpass = rtnuvpass;
  } //genCodeAReferenceIntoABitValue

  void NodeFunctionCall::genCodeVirtualFunctionCallVTableEntry(File * fp, u32 tvfpnum, u32 urtmpnum)
  {
    assert(m_funcSymbol);
    //requires runtime lookup for virtual function pointer
    //need typedef typename for this vfunc, any vtable of any owner of this vfunc
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = NULL; //any owner of func

    if(cosSize != 0)
      cos = m_state.m_currentObjSymbolsForCodeGen.back(); //"owner" of func
    else
      cos = m_state.getCurrentSelfSymbolForCodeGen(); //'self'

    UTI cosuti = cos->getUlamTypeIdx();

    m_state.indentUlamCode(fp);
    fp->write("VfuncPtr "); //legitimize this tmp label TODO
    fp->write(m_state.getVFuncPtrTmpNumAsString(tvfpnum).c_str());
    fp->write(" = ");

    //requires runtime lookup for virtual function pointer
    if(cos->isSelf() || cosSize == 0)
      {
	fp->write(m_state.getHiddenArgName()); //ur
	fp->write(".GetEffectiveSelf()->getVTableEntry(");
      }
    else if(cos->isSuper())
      {
	fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str());
	fp->write(".getVTableEntry(");
      }
    else if(urtmpnum > 0)
      {
	fp->write(m_state.getUlamRefTmpVarAsString(urtmpnum).c_str());
	fp->write(".GetEffectiveSelf()->getVTableEntry(");
      }
    else if(cos->getAutoLocalType() == ALT_AS)
      {
	m_state.abortShouldntGetHere();
	fp->write(m_state.getHiddenArgName()); //ur, should use urtmpnum!!
	fp->write(".GetEffectiveSelf()->getVTableEntry(");
      }
    else
      {
	//unless local or dm, known at compile time!
	fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str());
	fp->write(".getVTableEntry(");
      }

    //VT_IDX enum is the same regardless of effective self (e.g. t3600)
    UTI decosuti = m_state.getUlamTypeAsDeref(cosuti); // t3758
    UlamType * decosut = m_state.getUlamTypeByIndex(decosuti);

    fp->write(decosut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::"); //any class
    fp->write("VTABLE_IDX_"); //== m_funcSymbol->getVirtualMethodIdx()
    fp->write(m_funcSymbol->getMangledNameWithTypes().c_str());
    fp->write(");"); GCNL; //reading into a separate VfuncPtr tmp var
  } //genCodeVirtualFunctionCallVTableEntry

  void NodeFunctionCall::genCodeVirtualFunctionCall(File * fp, u32 tvfpnum)
  {
    assert(m_funcSymbol);
    //requires runtime lookup for virtual function pointer
    u32 vfidx = m_funcSymbol->getVirtualMethodIdx();

    //need typedef typename for this vfunc, any vtable of any owner of this vfunc
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = NULL; //any owner of func

    if(cosSize != 0)
      cos = m_state.m_currentObjSymbolsForCodeGen.back(); //"owner" of func
    else
      cos = m_state.getCurrentSelfSymbolForCodeGen(); //'self'

    UTI cosuti = cos->getUlamTypeIdx();
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cosuti, csym);
    assert(isDefined);

    UTI cvfuti = csym->getClassForVTableEntry(vfidx);
    UlamType * cvfut = m_state.getUlamTypeByIndex(cvfuti);

    // check that we are not trying to call a pure virtual function: t41158, t41160, t41094, safe t41161
    // limit to 'super' special case: t3606, t3608, t3774, t3779, t3788, t3794, t3795, t3967, t41131
    if(cos->isSuper())
      {
	SymbolClass * cvfsym = NULL;
	AssertBool iscvfDefined = m_state.alreadyDefinedSymbolClass(cvfuti, cvfsym);
	assert(iscvfDefined);
	if(cvfsym->isPureVTableEntry(vfidx))
	  {
	    std::ostringstream msg;
	    msg << "Virtual function <" << m_funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "> is pure; cannot be called";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }

    fp->write("((typename ");
    fp->write(cvfut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::"); //same for elements and quarks
    fp->write(m_funcSymbol->getMangledNameWithTypes().c_str());
    fp->write(") (");
    fp->write(m_state.getVFuncPtrTmpNumAsString(tvfpnum).c_str());
    fp->write(")) ");
    //args to function pointer remain
  } //genCodeVirtualFunctionCall

  // overrides Node in case of memberselect genCode
  void NodeFunctionCall::genCodeReadIntoATmpVar(File * fp, UVPass & uvpass)
  {
    return; //no-op
  }

  void NodeFunctionCall::genMemberNameOfMethod(File * fp)
  {
    assert(!Node::isCurrentObjectALocalVariableOrArgument());
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    Symbol * cos = NULL;
    if(cosSize > 0)
      cos = m_state.m_currentObjSymbolsForCodeGen.back();
    else
      cos = stgcos;
    UTI cosuti = cos->getUlamTypeIdx();

    UTI futi = m_funcSymbol->getDataMemberClass();
    if((cosSize > 0) || (UlamType::compare(cosuti, futi, m_state) != UTIC_SAME))
      {
	fp->write(m_state.getTheInstanceMangledNameByIndex(futi).c_str());
	fp->write(".");
      }
    else
      fp->write("THE_INSTANCE."); //non-static functions require an instance
  } //genMemberNameOfMethod

  void NodeFunctionCall::genModelParameterMemberNameOfMethod(File * fp, s32 epi)
  {
    m_state.abortShouldntGetHere();
  } //genModelParamenterMemberNameOfMethod

  std::string NodeFunctionCall::genHiddenArgs(u32 urtmpnum)
  {
    std::ostringstream hiddenargs;
    hiddenargs << m_state.getHiddenContextArgName(); //same uc
    hiddenargs << ", ";
    if(urtmpnum != 0)
       hiddenargs << m_state.getUlamRefTmpVarAsString(urtmpnum).c_str();
    else
      hiddenargs << m_state.getHiddenArgName(); //same ur;

    return hiddenargs.str();
  } //genHiddenArgs

  std::string NodeFunctionCall::genHiddenArg2ForARef(File * fp, UVPass uvpass, u32& urtmpnumref)
  {
    assert(uvpass.getPassStorage() == TMPAUTOREF);

    UTI vuti = uvpass.getPassTargetType();

    //vuti may not be a ref (e.g. t3668, a QW that was deref'd by [].)
    bool isaref = m_state.isAltRefType(vuti);

    //use possible dereference type for mangled name
    UTI derefuti = m_state.getUlamTypeAsDeref(vuti);
    assert(m_state.isAClass(derefuti));

   u32 tmpvarnum = uvpass.getPassVarNum();
   u32 tmpvarur = m_state.getNextTmpVarNumber();

    std::ostringstream hiddenarg2;
    //new ur to reflect "effective" self and the ref storage, for this funccall
    hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvarur).c_str() << "(";
    hiddenarg2 << m_state.getTmpVarAsString(derefuti, tmpvarnum, TMPAUTOREF).c_str();
    if(isaref)
      hiddenarg2 << ", 0u"; //references already offset t3811
    else
      hiddenarg2 << ", " << uvpass.getPassPos() << "u"; //element refs already +25

    hiddenarg2 << ", " << getLengthOfMemberClassForHiddenArg(derefuti) << "u, "; //len t3370
    hiddenarg2 << "&";
    hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(derefuti).c_str();
    hiddenarg2 << ", " << genUlamRefUsageAsString(derefuti).c_str();
    hiddenarg2 << ");";

    urtmpnumref = tmpvarur;
    return hiddenarg2.str();
  } //genHiddenArg2ForARef

  std::string NodeFunctionCall::genStorageType()
  {
    std::ostringstream stype;

    //"hidden" self arg
    if(!Node::isCurrentObjectALocalVariableOrArgument())
      {
	stype << m_state.getHiddenArgName();
	stype << ".GetType()";
      }
    else
      {
	s32 epi = Node::isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    stype << genModelParameterHiddenArgs(epi).c_str();
	  }
	else //local var
	  {
	    Symbol * stgcos = NULL;
	    Symbol * costmp = NULL;
	    Node::loadStorageAndCurrentObjectSymbols(stgcos, costmp);
	    assert(costmp && stgcos);

	    stype << stgcos->getMangledName().c_str();
	    stype << ".GetType()";
	  }
      }
    return stype.str();
  } //genStorageType

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
    Symbol * epcos = m_state.m_currentObjSymbolsForCodeGen[epi]; //***
    UTI epcosuti = epcos->getUlamTypeIdx();
    UlamType * epcosut = m_state.getUlamTypeByIndex(epcosuti);
    ULAMCLASSTYPE epcosclasstype = epcosut->getUlamClassType();

    hiddenlist << m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str();
    hiddenlist << ".";

    // the MP (only primitive, no longer an element, or quark):
    hiddenlist << epcos->getMangledName().c_str();

    if(epcosclasstype != UC_NOTACLASS)
      {
	m_state.abortShouldntGetHere();
	hiddenlist << ".getRef()";
      }
    return hiddenlist.str();
  } //genModelParameterHiddenArgs

  std::string NodeFunctionCall::genRestOfFunctionArgs(File * fp, UVPass& uvpass)
  {
    std::ostringstream arglist;

    //wiped out by arg processing; needed to determine owner of called function
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;

    assert(m_funcSymbol);
    u32 numParams = m_funcSymbol->getNumberOfParameters();
    // handle any variable number of args separately
    // since non-datamember variables can modify globals, save/restore before/after each
    for(u32 i = 0; i < numParams; i++)
      {
	UVPass auvpass;
	m_state.clearCurrentObjSymbolsForCodeGen(); //*************

	if(m_state.getReferenceType(m_funcSymbol->getParameterType(i)) != ALT_NOT)
	  {
	    genCodeReferenceArg(fp, auvpass, i); //t41100
	  }
	else
	  {
	    m_argumentNodes->genCode(fp, auvpass, i);
	    Node::genCodeConvertATmpVarIntoBitVector(fp, auvpass);
	  }
	arglist << ", " << auvpass.getTmpVarAsString(m_state).c_str();
      } //next arg..

    if(m_funcSymbol->takesVariableArgs())
      {
	u32 numargs = getNumberOfArguments();
	for(u32 i = numParams; i < numargs; i++)
	  {
	    UVPass auvpass;
	    m_state.clearCurrentObjSymbolsForCodeGen(); //*************
	    UTI auti = m_argumentNodes->getNodeType(i);
	    ALT aalt = m_state.getReferenceType(auti);
	    if( aalt != ALT_NOT)
	      {
		//variable args cannot be references, but casted arrayitem okay;
		// c&l makes sure (t3250, t41099)
		//genCodeReferenceArg(fp, auvpass, i);
		m_state.abortShouldntGetHere();
	      }
	    else
	      {
		m_argumentNodes->genCode(fp, auvpass, i);
		Node::genCodeConvertATmpVarIntoBitVector(fp, auvpass);
	      }
	    // use pointer for variable arg's since all the same size that way
	    arglist << ", &" << auvpass.getTmpVarAsString(m_state).c_str();
	  } //end forloop through variable number of args

	arglist << ", (void *) 0"; //indicates end of args
      } //end of handling variable arguments

    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore vector after args******

    return arglist.str();
  } //genRestOfFunctionArgs

  // should be like NodeVarRef::genCode;
  void NodeFunctionCall::genCodeReferenceArg(File * fp, UVPass & uvpass, u32 n)
  {
    // get the right?-hand side, stgcos
    // can be same type (e.g. element, quark, or primitive),
    // or ancestor quark if a class.
    m_argumentNodes->genCodeToStoreInto(fp, uvpass, n);

    u32 id = 0;
    Symbol * cossym = NULL;
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	cossym = m_state.m_currentObjSymbolsForCodeGen.back();
	id = cossym->getId();
      }

    //tmp var for lhs
    assert(m_funcSymbol);
    u32 numParams = m_funcSymbol->getNumberOfParameters();
    UTI vuti = Nav;
    if(n < numParams)
      vuti = m_funcSymbol->getParameterType(n);
    else
      {
	assert(m_funcSymbol->takesVariableArgs()); //must also be native
	vuti = m_argumentNodes->getNodeType(n); //pass type we got
      }

    s32 tmpVarArgNum = m_state.getNextTmpVarNumber();

    UVPass luvpass = UVPass::makePass(tmpVarArgNum, TMPAUTOREF, vuti, m_state.determinePackable(vuti), m_state, 0, id);
    SymbolTmpVar * tmpvarsym = Node::makeTmpVarSymbolForCodeGen(luvpass, cossym); //cossym could be null

    Node::genCodeReferenceInitialization(fp, uvpass, tmpvarsym); //uvpass, not luvpass t3812, t3819

    delete tmpvarsym;
    uvpass = luvpass;
    return;
  } //genCodeReferenceArg

void NodeFunctionCall::genLocalMemberNameOfMethod(File * fp)
  {
    assert(Node::isCurrentObjectALocalVariableOrArgument());
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    UTI futi = m_funcSymbol->getDataMemberClass();
    UlamType * fut = m_state.getUlamTypeByIndex(futi);
    fp->write(fut->getUlamTypeMangledName().c_str()); //e.g. t3605
    fp->write("<EC>::THE_INSTANCE.");
  } //genLocalMemberNameOfMethod

  u32 NodeFunctionCall::getLengthOfMemberClassForHiddenArg(UTI cosuti)
  {
    //both virtuals and non- (original)
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    return cosut->getTotalBitSize();

    //change len.. (new, unused)
    UTI futi = m_funcSymbol->getDataMemberClass();
    UlamType * fut = m_state.getUlamTypeByIndex(futi);
    return fut->getTotalBitSize();
  }

} //end MFM
