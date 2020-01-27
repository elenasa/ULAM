#include "NodeFunctionCall.h"
#include "CompilerState.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeIdent.h"
#include "NodeMemberSelect.h"
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
    UTI it = getNodeType(); //Nav;  // init return type

    u32 numErrorsFound = 0;
    u32 numHazyFound = 0;

    //might be related to m_currentSelfPtr?
    //member selection doesn't apply to arguments
    //look up in class block, and match argument types to parameters
    SymbolFunction * funcSymbol = NULL;
    Symbol * fnsymptr = NULL;
    u32 funcid = m_state.getTokenDataAsStringId(m_functionNameTok); //t41087

    //label argument types; used to pinpoint the exact function symbol in case of overloading
    std::vector<Node *> argNodes;
    u32 constantArgs = 0;
    u32 navArgs = 0;
    u32 hzyArgs = 0;
    u32 noutiArgs = 0;
    UTI listuti = Nav;
    bool hazyKin = false;

    UTI cuti = m_state.getCompileThisIdx();
    if(m_state.useMemberBlock())
      {
	NodeBlockClass * memberblock = m_state.getCurrentMemberClassBlock();
	assert(memberblock);
	cuti = memberblock->getNodeType();
      }

    if(m_state.isFuncIdInAClassScopeOrAncestor(cuti, funcid, fnsymptr, hazyKin) && !hazyKin)
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
	    else if((argtype == Hzy) || m_state.isStillHazy(argtype))
	      hzyArgs++;
	    else if(argtype == Nouti)
	      noutiArgs++;
	    else //t3984,5
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
	UTI foundInAncestor = Nouti;
	u32 numFuncs = m_state.findMatchingFunctionWithSafeCastsInAClassScopeOrAncestor(cuti, funcid, argNodes, funcSymbol, hasHazyArgs, foundInAncestor);
	if(numFuncs == 0)
	  {
	    assert(foundInAncestor == Nouti); //sanity
	    std::ostringstream msg;
	    msg << "(1) '" << m_state.getTokenDataAsString(m_functionNameTok).c_str();
	    msg << "' has no defined function with " << numargs;
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
	    assert(foundInAncestor == Nav); //sanity
	    std::ostringstream msg;
	    msg << "Ambiguous matches (" << numFuncs << ") of function '";
	    msg << m_state.getTokenDataAsString(m_functionNameTok).c_str();
	    if(numargs > 0)
	      {
		msg << "' called with " << numargs << " argument type";
		if(numargs != 1)
		  msg << "s";
		msg << ": ";
		for(u32 i = 0; i < argNodes.size(); i++)
		  {
		    UTI auti = argNodes[i]->getNodeType();
		    msg << m_state.getUlamTypeNameBriefByIndex(auti).c_str() << ", ";
		  }
		msg << "explicit casting is required";
	      }
	    else
	      {
		msg << "' called without arguments"; //t41329, t41305
	      }
	    if(hasHazyArgs)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		numHazyFound++;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //t3479
		numErrorsFound++;
		m_state.noteAmbiguousFunctionSignaturesInAClassHierarchy(cuti, funcid, argNodes, numFuncs);
	      }
	  }
	else
	  {
	    assert(numFuncs==1); //sanity
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
			msg << "Invalid argument " << i + 1 << " to function '";
			msg << m_state.getTokenDataAsString(m_functionNameTok).c_str();
			msg << "'; Cannot be used as a reference parameter";
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
	TBOOL foundit = TBOOL_FALSE;
	if(hazyKin)
	  foundit = TBOOL_HAZY;
	else
	  foundit = lookagainincaseimplicitselfchanged(); //TBOOL_HAZY is good!(t41346)

	if(foundit != TBOOL_TRUE)
	  {
	    std::ostringstream msg;
	    msg << "(2) '" << m_state.getTokenDataAsString(m_functionNameTok).c_str();
	    msg << "' is not a defined function, or cannot be safely called in this context";
	    if(foundit == TBOOL_HAZY)
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
      }

    if(funcSymbol && m_funcSymbol != funcSymbol)
      {
	//may precede function parameter c&l, and fail names of args with type
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
	assert(m_state.okUTItoContinue(it));

	it = specifyimplicitselfexplicitly();

	if(m_state.okUTItoContinue(it))
	  {
	    if(m_state.isComplete(it))
	      setNodeType(it);
	    else
	      {
		//Sun Aug 11 2019 Dave issue w Bounce.ulam: nodeType stays incomplete
		setNodeType(Hzy);
		m_state.setGoAgain(); //for compier counts
		return Hzy; //short circuit
	      }
	  }
	else
	  {
	    setNodeType(it); //t41388 error
	    if(it == Hzy)
	      m_state.setGoAgain();
	    return it;
	  }

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
			msg << " to function '";
			msg << m_state.getTokenDataAsString(m_functionNameTok).c_str();
			msg <<"'; type ";
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

	      msg << " to function '";
	      msg << m_state.getTokenDataAsString(m_functionNameTok).c_str() <<"'";
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

  TBOOL NodeFunctionCall::lookagainincaseimplicitselfchanged()
  {
    TBOOL rtn = TBOOL_FALSE;

    Symbol * fnsymptr = NULL;
    bool hazyKin = false;

    if(!m_state.useMemberBlock())
      {
	//not found in current context, perhaps 'self' has changed scope (t41344)
	u32 selfid = m_state.m_pool.getIndexForDataString("self");
	Symbol * selfsym = NULL;
	bool hazykin = false; //unused
	bool gotSelf = m_state.alreadyDefinedSymbolHere(selfid, selfsym, hazykin);
	if(gotSelf)
	  {
	    UTI selfuti = selfsym->getUlamTypeIdx();
	    SymbolClass * csym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(selfuti, csym);
	    assert(isDefined);

	    NodeBlockClass * memberClassNode = csym->getClassBlockNode();
	    assert(memberClassNode);

	    UTI selfblockuti = memberClassNode->getNodeType();
	    if(m_state.okUTItoContinue(selfblockuti))
	      {
		//set up compiler state to use the member class block for variable
		m_state.pushClassContextUsingMemberClassBlock(memberClassNode);

		u32 funcid = m_state.getTokenDataAsStringId(m_functionNameTok);
		bool foundit = m_state.isFuncIdInAClassScopeOrAncestor(selfblockuti, funcid, fnsymptr, hazyKin);
		if(foundit)
		  rtn = TBOOL_TRUE;
		m_state.popClassContext();
	      }
	    else if(selfblockuti == Nav)
	      rtn = TBOOL_FALSE;
	    else if(selfblockuti == Hzy)
	      rtn = TBOOL_HAZY;
	    else if(selfblockuti == Nouti)
	      rtn = TBOOL_HAZY;
	    //else
	  } //ends gotself (t3415,t3431,t3455,t3460,t41283)
      } //ends not using memberblock

    if(rtn == TBOOL_TRUE)
      {
	SymbolFunction * tmpfuncsym = NULL;
	((SymbolFunctionName *) fnsymptr)->anyFunctionSymbolPtr(tmpfuncsym);
	assert(tmpfuncsym);
	m_funcSymbol = tmpfuncsym;
	UTI it = specifyimplicitselfexplicitly(); //returns Hzy
	if(it == Hzy)
	  rtn = TBOOL_HAZY;
	else if(!m_state.okUTItoContinue(it))
	  rtn = TBOOL_FALSE;
	//else still TBOOL_TRUE
      }
    assert(m_funcSymbol==NULL);
    return rtn;
  } //lookagainincaseimplicitselfchanged

  UTI NodeFunctionCall::specifyimplicitselfexplicitly()
  {
    assert(m_funcSymbol);
    UTI futi = m_funcSymbol->getUlamTypeIdx();

    //a func call needs to be rhs of member select "."
    if(m_state.useMemberBlock())
      {
	return futi; //t3200
      }

    NNO pno = Node::getYourParentNo();

    NodeBlock * currBlock = m_state.getCurrentBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock); //push again


    Node * parentNode = m_state.findNodeNoInThisClassForParent(pno);
    assert(parentNode);

    m_state.popClassContext(); //restore

    bool implicitself = true;

    if(parentNode->isAMemberSelect())
      {
	Symbol * rhsym = NULL;
	if(!parentNode->getSymbolPtr(rhsym))
	  futi = Hzy;

	implicitself = (rhsym != m_funcSymbol);
      }
    //else

    if(!implicitself)
      return futi; //done, no change

    Token selfTok(TOK_KW_SELF, getNodeLocation(), 0);
    NodeIdent * explicitself = new NodeIdent(selfTok, NULL, m_state);
    assert(explicitself);

    NodeMemberSelect * newnode = new NodeMemberSelect(explicitself, this, m_state);
    assert(newnode);

    AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
    assert(swapOk);

    //redo look-up given explicit self
    m_funcSymbol = NULL;

    //reusing this, no suicide
    return Hzy;
  } //specifyimplicitselfexplicitly

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
    msg << " Save the results of '";
    msg << m_state.getTokenDataAsString(m_functionNameTok).c_str();
    msg << "' to a variable, type: ";
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
      } //end virtual function call

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
    UTI vownuti = m_funcSymbol->getVirtualMethodOriginatingClassUTI();

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
		UTI baseuti = atomPtr.getPtrTargetType();
		atomPtr.setPtrTargetType(auti); //t3746
		u32 relposofbase = 0; //t41364
		AssertBool gotpos = m_state.getABaseClassRelativePositionInAClass(auti, baseuti, relposofbase);
		assert(gotpos);
		atomPtr.setPtrPos(atomPtr.getPtrPos() - relposofbase);
		atomPtr.setPtrLen(m_state.getTotalBitSize(auti)); //as complete object
	      }
	  }
      } //else can't be an autolocal

    UTI cuti = atomPtr.getPtrTargetType(); //must be a class
    SymbolClass * vcsym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, vcsym);
    assert(isDefined);

    u32 coffset = vcsym->getVTstartoffsetOfRelatedOriginatingClass(vownuti);
    assert(coffset<UNRELIABLEPOS);
    UTI vtcuti = vcsym->getClassForVTableEntry(vtidx + coffset); //t41304

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
	//setup compilerstate to use the member class block for symbol search
	m_state.pushClassContextUsingMemberClassBlock(memberClassNode);

	u32 funcid = m_state.getTokenDataAsStringId(m_functionNameTok); //t41087
	Symbol * fnsymptr = NULL;
	bool hazyKin = false;
	AssertBool isDefinedFunc = (m_state.isFuncIdInClassScope(funcid, fnsymptr, hazyKin) && !hazyKin);
	assert(isDefinedFunc);

	//find this func in the virtual class; get its func def.
	std::vector<UTI> pTypes;
	m_funcSymbol->getVectorOfParameterTypes(pTypes);

	SymbolFunction * funcSymbol = NULL;
	bool tmphazyargs = false;
	u32 numFuncs = ((SymbolFunctionName *) fnsymptr)->findMatchingFunctionStrictlyByTypes(pTypes, funcSymbol, tmphazyargs);
	assert(!tmphazyargs);

	if(numFuncs != 1)
	  {
	    std::ostringstream msg;
	    msg << "Virtual function '" << funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "' is ";
	    if(numFuncs > 1)
	      msg << "ambiguous";
	    else
	      msg << "not found";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }

	if(!funcSymbol->isVirtualFunction())
	  {
	    std::ostringstream msg;
	    msg << "Function '" << funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "' is not virtual";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnok = false;
	  }
	else if(funcSymbol->isPureVirtualFunction())
	  {
	    std::ostringstream msg;
	    msg << "(1) Virtual function '" << funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "' is pure; cannot be called for eval";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnok = false;
	  }

	m_state.popClassContext(); //restore here

	if(rtnok)
	  rtnfunc = funcSymbol->getFunctionNode(); //replace with virtual function def!!!
      } //end lookup virtual function
    else
      {
	//t41361 cannot go by vtclass when it needs its subclass' table
	if(m_funcSymbol->isPureVirtualFunction())
	  {
	    std::ostringstream msg;
	    msg << "(2) Virtual function '" << m_funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "' is pure; cannot be called for eval";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnok = false; //t41094, t41158, t41160, t41313
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
	msg << "(3) '" << m_state.getTokenDataAsString(m_functionNameTok).c_str();
	msg << "' is not a fully resolved function definition; ";
	msg << "A call to it cannot be generated in this context";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	m_state.clearCurrentObjSymbolsForCodeGen();
	return;
      }

    UTI nuti = getNodeType();

    // The Call:
    //treat atom/ref as tmpbitval (t41143)
    if((uvpass.getPassStorage() == TMPAUTOREF) && !m_state.isAtom(nuti))
      genCodeAReferenceIntoABitValue(fp, uvpass);
    else
      genCodeIntoABitValue(fp, uvpass);

    // Result:
    if(nuti != Void)
      {
	UTI vuti = uvpass.getPassTargetType();
	// skip reading classes and atoms
	if(m_state.getUlamTypeByIndex(vuti)->isPrimitiveType())
	  {
	    //e.g. t3653, t3946, t41001,7,34,71,73, t41333,6, t41351,3, t41358,9
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
    TMPSTORAGE nstor = nut->isReference() ? TMPAUTOREF : TMPBITVAL; //not TMPREGISTER for primitives (t3114) or quarks (t3272)

    UTI vownuti = (m_funcSymbol->isVirtualFunction() ? m_funcSymbol->getVirtualMethodOriginatingClassUTI() : (UTI) Nouti);
    UTI futi = m_funcSymbol->getDataMemberClass();
    u32 urtmpnum = 0;
    std::string hiddenarg2str = Node::genHiddenArg2(uvpass, urtmpnum, vownuti, futi);
    if(urtmpnum > 0)
      {
	m_state.indentUlamCode(fp);
	fp->write(hiddenarg2str.c_str()); GCNL;
      }

    u32 tvfpnum = m_state.getNextTmpVarNumber();
    if(m_funcSymbol->isVirtualFunction())
      {
	u32 urtmpnumvfc = 0;
	genCodeVirtualFunctionCallVTableEntry(fp, tvfpnum, urtmpnum, urtmpnumvfc); //indirect call thru func ptr
	urtmpnum = urtmpnumvfc; //update w new ur
      }

    std::ostringstream arglist;
    // presumably there's no = sign.., and no open brace for tmpvars
    arglist << genHiddenArgs(urtmpnum).c_str();

    //loads any variables into tmps before used as args (needs fp)
    arglist << genRestOfFunctionArgs(fp, uvpass).c_str();

    //non-void RETURN value saved in a tmp BitValue; depends on return type
    m_state.indentUlamCode(fp);
    if(nuti != Void)
      {
	u32 pos = 0; //POS 0 leftjustified;
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

	uvpass = UVPass::makePass(rtntmpnum, nstor, nuti, m_state.determinePackable(nuti), m_state, pos, selfid); //POS adjusted for BitVector, justified; self id in Pass;

	// put result of function call into a variable;
	// (C turns it into the copy constructor)
	if(getStoreIntoAble() == TBOOL_FALSE)
	  fp->write("const ");
	fp->write(nut->getLocalStorageTypeAsString().c_str()); //e.g. Ui_
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, rtntmpnum, nstor).c_str());
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
    UTI nuti = getNodeType(); //func return type
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    TMPSTORAGE nstor = nut->getTmpStorageTypeForTmpVar(); //tmpautoref

    u32 urtmpnum = 0;
    //    if(m_state.m_currentObjSymbolsForCodeGen.empty() || m_state.m_currentObjSymbolsForCodeGen[0]->isSelf())
      {
	std::string hiddenarg2str = genHiddenArg2ForARef(fp, uvpass, urtmpnum);

	m_state.indentUlamCode(fp);
	fp->write(hiddenarg2str.c_str()); GCNL;
      }

    u32 tvfpnum = m_state.getNextTmpVarNumber();
    if(m_funcSymbol->isVirtualFunction())
      {
	u32 urtmpnumvfc = 0;
	genCodeVirtualFunctionCallVTableEntry(fp, tvfpnum, urtmpnum, urtmpnumvfc); //indirect call thru func ptr
	urtmpnum = urtmpnumvfc; //new ur
      }

    std::ostringstream arglist;
    // presumably there's no = sign.., and no open brace for tmpvars
    arglist << genHiddenArgs(urtmpnum).c_str();

    //loads any variables into tmps before used as args (needs fp)
    arglist << genRestOfFunctionArgs(fp, uvpass).c_str();


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

	rtnuvpass = UVPass::makePass(rtnSlot, nstor, nuti, m_state.determinePackable(nuti), m_state, pos, selfid); //POS adjusted for BitVector, justified; self id in Pass;

	// put result of function call into a variable;
	// (C turns it into the copy constructor)
	fp->write("const ");
	fp->write(nut->getLocalStorageTypeAsString().c_str()); //e.g. BitVector<32>
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, rtnSlot, nstor).c_str());
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

  void NodeFunctionCall::genCodeVirtualFunctionCallVTableEntry(File * fp, u32 tvfpnum, u32 urtmpnum, u32& urtmpnumvfc)
  {
    assert(m_funcSymbol);
    //Often requires runtime lookup for virtual function pointer;
    //need typedef typename for this vfunc by any owner of this vfunc
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = NULL; //any owner of func

    if(cosSize != 0)
      cos = m_state.m_currentObjSymbolsForCodeGen.back(); //"owner" of func, or id
    else
      cos = m_state.getCurrentSelfSymbolForCodeGen(); //'self'

    UTI cosuti = cos->getUlamTypeIdx();
    UTI decosuti = m_state.getUlamTypeAsDeref(cosuti); // t3758?

    // (ulam-5) requires runtime lookup for start index into virtual table,
    // using registration number of orig class (both known at compile time)
    UTI vownuti = m_funcSymbol->getVirtualMethodOriginatingClassUTI();
    //u32 vownregnum = m_state.getAClassRegistrationNumber(vownuti);
    //VOWNED_IDX enum is the same regardless of effective self (e.g. t3600)
    // belongs to originating class; subclass knows offset in VT (ulam-5)
    UlamType * vownut = m_state.getUlamTypeByIndex(vownuti);

    bool knownatcompiletime = false;
    bool lhscallseffself = false;
    bool lhscallsvtbyrn = false;
    bool checkforpure = false;

    //MAKE A STRING for reuse: lhs vtable accessor
    std::ostringstream lhsstr; //caller string
    if(cos->isSelf() || cosSize == 0)
      {
	lhsstr << m_state.getHiddenArgName(); //ur
	lhsstr << ".GetEffectiveSelf()->";
	lhscallseffself = true;
      }
    else if(cos->isSuper())
      {
	lhsstr << m_state.getTheInstanceMangledNameByIndex(decosuti).c_str();
	lhsstr << "."; //t41161
	checkforpure = true;
      }
    else if(cos->isTmpVarSymbol() && ((SymbolTmpVar *) cos)->isBaseClassRef())
      {
	lhsstr << m_state.getTheInstanceMangledNameByIndex(decosuti).c_str();
	lhsstr << "."; //t41311, t41314
	//was, knownatcompiletime = true; //t41353
	checkforpure = true; //error:t41313
      }
    else if(cos->isTmpVarSymbol() && ((SymbolTmpVar *) cos)->isBaseClassRegNum())
      {
	lhsstr << cos->getMangledName().c_str(); //Unsigned: tmpvar
	lhscallsvtbyrn = true;
      }
    else if(m_state.isReference(cosuti) && (urtmpnum > 0)) //t41301
      {
	// could be ALT_AS: t3601,36,39, t3747, t3835, t41046, t41315,18,20,23
	lhsstr << m_state.getUlamRefTmpVarAsString(urtmpnum).c_str();
	lhsstr <<".GetEffectiveSelf()->";
	lhscallseffself = true;
      }
    else
      {
	//unless local or dm, known at compile time! (t41354)
	//t3357,8, t3361,t3531, t3600, t3719,20,21,22, t3743,5,7,8, t3804,5
	//t3880,t41000,1,7,11,12,97, t41298,9, t41304,17,19,22,25,27,28,32,54
	lhsstr << m_state.getTheInstanceMangledNameByIndex(decosuti).c_str();
	lhsstr << ".";
	knownatcompiletime = true;
	checkforpure = true; //must be!
      }

    //use shorthand UlamRef for virtual funcs when effSelf is used (t41322)
    if(lhscallseffself)
      {
	return genCodeVirtualFunctionCallVTableEntryUsingEffectiveSelf(fp,tvfpnum,urtmpnum,urtmpnumvfc);
      } //done

    //use shorthand UlamRef for virtual funcs when VTtable class (not
    //effSelf) specified by a variable (t41376-9)
    if(lhscallsvtbyrn)
      {
	//note: check base related to vfunc originating class already happened (t41378)
	u32 tmpvtclassrn = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write("const u32 ");
	fp->write(m_state.getTmpVarAsString(Unsigned, tmpvtclassrn, TMPREGISTER).c_str());
	fp->write(" = ");
	fp->write(lhsstr.str().c_str()); //cos is Unsigned classid tmpvar
	fp->write(";"); GCNL;

	return genCodeVirtualFunctionCallVTableEntryUsingSpecifiedVTable(fp,Nouti,tmpvtclassrn,tvfpnum,urtmpnum,urtmpnumvfc);
      } //done

    if(!knownatcompiletime)
      {
	//Create UlamRef for this vfunc call into urtmpnumvfc:
	//no longer needs to "spell it out" since lhs doesn't use EffectiveSelf(),
	// uses new shorthand UlamRef instead (ulam-5)
	//t3743,4,5,6,t41097,t41161,
	//t41298,9, t41304,7,8,9,10,11,14,15,16,17,18, t41320,1,2,3,7,8,
	//t41333,6,8,t41351,3,t41361,3,4,6
	genCodeVirtualFunctionCallVTableEntryUsingSpecifiedVTable(fp,decosuti,0,tvfpnum,urtmpnum,urtmpnumvfc);
      }
    else
      {
	//knownatcompiletime..VfuncPtr read into variable after checkforpure
	assert(checkforpure);
      }

  // may also check for pure when relpos is not known at compiletime, but we
  // know which VTable to use (e.g. BaseType/Super specified)
  // t41313,t41330; t41160,t41158,t41094
  if(checkforpure)
    {
      //error/t41313, error/t41330
      SymbolClass * csym = NULL;
      AssertBool gotClass = m_state.alreadyDefinedSymbolClass(decosuti,csym);
      assert(gotClass);

      u32 vfidx = m_funcSymbol->getVirtualMethodIdx();
      u32 startoffset = csym->getVTstartoffsetOfRelatedOriginatingClass(vownuti);
      u32 vt = vfidx + startoffset;

      if(csym->isPureVTableEntry(vt))
	{
	  std::ostringstream msg;
	  msg << "Virtual function '" << m_funcSymbol->getMangledNameWithTypes().c_str();
	  msg << "' is pure; cannot be called directly in baseclass: ";
	  msg << m_state.getUlamTypeNameBriefByIndex(decosuti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      else if(knownatcompiletime)
	{
	  UTI veuti = csym->getClassForVTableEntry(vt); //t3600
	  UlamType * veut = m_state.getUlamTypeByIndex(veuti);
	  if(UlamType::compare(veuti, decosuti, m_state) != UTIC_SAME)
	    {
	      u32 verelpos;
	      AssertBool gotrelpos = m_state.getABaseClassRelativePositionInAClass(decosuti, veuti, verelpos);
	      assert(gotrelpos);

	      //Create UlamRef for this vfunc call to override class (t41007)
	      urtmpnumvfc = m_state.getNextTmpVarNumber();
	      m_state.indentUlamCode(fp);
	      fp->write("UlamRef<EC> ");
	      fp->write(m_state.getUlamRefTmpVarAsString(urtmpnumvfc).c_str());
	      fp->write("(");
	      if(urtmpnum > 0)
		fp->write(m_state.getUlamRefTmpVarAsString(urtmpnum).c_str());
	      else
		fp->write(m_state.getHiddenArgName()); //ur
	      fp->write(", ");
	      fp->write_decimal(verelpos); //override pos
	      fp->write(", ");
	      fp->write_decimal_unsigned(veut->getSizeofUlamType()); //override len
	      fp->write("u, ");
	      fp->write(Node::genUlamRefUsageAsString(veuti).c_str()); //usage
	      fp->write(", true"); //(always true!)
	      fp->write(");"); GCNL;
	    }
	  else
	    urtmpnumvfc = urtmpnum; //same ur

	  m_state.indentUlamCode(fp);
	  fp->write("VfuncPtr "); //legitimize this tmp label TODO
	  fp->write(m_state.getVFuncPtrTmpNumAsString(tvfpnum).c_str()); //Uf_tvfpNNN
	  fp->write(" = ");
	  fp->write("(VfuncPtr) "); //cast to void
	  fp->write("((typename "); //cast to contextual type info
	  fp->write(vownut->getUlamTypeMangledName().c_str());
	  fp->write("<EC>::"); //same for elements and quarks
	  fp->write(csym->getMangledFunctionNameWithTypesForVTableEntry(vt).c_str());
	  fp->write(") &");
	  fp->write(veut->getUlamTypeMangledName().c_str());
	  fp->write("<EC>::"); //same for elements and quarks
	  fp->write(csym->getMangledFunctionNameForVTableEntry(vt).c_str());
	  fp->write(");"); GCNL; //reading into a separate VfuncPtr tmp var
	}
    }
    // READY to make virtual function call ur, with appropriate
    // overriding class pos/len, same effself and usage;
    return;
  } //genCodeVirtualFunctionCallVTableEntry

  void NodeFunctionCall::genCodeVirtualFunctionCallVTableEntryUsingSpecifiedVTable(File * fp, UTI vtclassuti, u32 tmpvtclassrn, u32 tvfpnum, u32 urtmpnum, u32& urtmpnumvfc)
  {
    assert(m_funcSymbol);
    //requires runtime lookup for virtual function pointer; needs
    //typedef typename for this vfunc (in any owner of this vfunc);
    //(ulam-5) requires runtime lookup for vfunc vowned index, plus
    //start index into virtual table (using registration number of
    //orig class), both known at compile time.
    UTI vownuti = m_funcSymbol->getVirtualMethodOriginatingClassUTI();
    u32 vownregnum = m_state.getAClassRegistrationNumber(vownuti);
    //VOWNED_IDX enum is the same regardless of effective self (e.g. t3600)
    // belongs to originating class; subclass knows offset in VT (ulam-5)
    UlamType * vownut = m_state.getUlamTypeByIndex(vownuti);

    //MAKE A STRING for reuse: vtable index between the parens w tmpvtstartidx
    std::ostringstream vtindexstring; //between parens
    vtindexstring << vownut->getUlamTypeMangledName().c_str();
    vtindexstring << "<EC>::"; //orignating class
    vtindexstring << "VOWNED_IDX_"; //== m_funcSymbol->getVirtualMethodIdx()
    vtindexstring << m_funcSymbol->getMangledNameWithTypes().c_str();

    //requires runtime lookup for virtual function pointer
    m_state.indentUlamCode(fp);
    fp->write("VfuncPtr "); //legitimize this tmp label TODO
    fp->write(m_state.getVFuncPtrTmpNumAsString(tvfpnum).c_str()); //Uf_tvfpNNN
    fp->write(";"); GCNL;

    // UlamClass ptr is UlamRef arg for vtable lookup
    s32 tmpvtclass = m_state.getNextTmpVarNumber();
    if(tmpvtclassrn > 0)
      {
	s32 tmpucr = m_state.getNextTmpVarNumber();
	//get UlamClass ptr from regid via the registry
	m_state.indentUlamCode(fp);
	fp->write("const UlamClassRegistry<EC> & ");
	fp->write(m_state.getUlamClassRegistryTmpVarAsString(tmpucr).c_str());
	fp->write(" = uc.GetUlamClassRegistry();\n");
	m_state.indentUlamCode(fp);
	fp->write("const UlamClass<EC> * ");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpvtclass).c_str());
	fp->write(" = ");
	fp->write(m_state.getUlamClassRegistryTmpVarAsString(tmpucr).c_str());
	fp->write(".GetUlamClassOrNullByIndex(");
	fp->write(m_state.getTmpVarAsString(Unsigned,tmpvtclassrn,TMPREGISTER).c_str()); //vtable class regnum
	fp->write(");"); GCNL;

	//runtime check that vtclass (rn) is/related to the Base provided (t41377)
	u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
	assert(cosSize >= 3);
	Symbol * basecos = m_state.m_currentObjSymbolsForCodeGen[cosSize - 2];
	UTI basecosuti = basecos->getUlamTypeIdx();

	m_state.indentUlamCode(fp);
	fp->write("if(! ");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpvtclass).c_str());
	fp->write("->");
	fp->write(m_state.getIsMangledFunctionName(basecosuti));
	fp->write("(& ");
	fp->write(m_state.getTheInstanceMangledNameByIndex(basecosuti).c_str());
	fp->write("))"); GCNL;

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(BAD_VIRTUAL_CALL);"); GCNL;
	m_state.m_currentIndentLevel--;
      }
    else
      {
	//vtable class specified explicitly at compiletime
	m_state.indentUlamCode(fp);
	fp->write("const UlamClass<EC> * ");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpvtclass).c_str());
	fp->write(" = &");
	fp->write(m_state.getTheInstanceMangledNameByIndex(vtclassuti).c_str());
	fp->write(";"); GCNL;
      }

    //generate shorthand UlamRef constructor for vfunc calls, with specified vtable class:
    //UlamRef checks: vtclass is/related to vownuti, and effSelf is/related to vtclass.
    urtmpnumvfc = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("UlamRef<EC> ");
    fp->write(m_state.getUlamRefTmpVarAsString(urtmpnumvfc).c_str());
    fp->write("(");
    if(urtmpnum > 0)
      fp->write(m_state.getUlamRefTmpVarAsString(urtmpnum).c_str());
    else
      fp->write(m_state.getHiddenArgName()); //ur
    fp->write(", ");
    fp->write(m_state.getUlamClassTmpVarAsString(tmpvtclass).c_str()); //vtable class regnum
    fp->write(", ");
    fp->write(vtindexstring.str().c_str());
    fp->write(", ");
    fp->write_decimal_unsigned(vownregnum);
    fp->write("u /*");
    fp->write(m_state.getUlamTypeNameBriefByIndex(vownuti).c_str());
    fp->write("*/, "); //usage based on override class
    fp->write(m_state.getVFuncPtrTmpNumAsString(tvfpnum).c_str()); //Uf_tvfpNNN
    fp->write(");"); GCNL;

    // READY to make virtual function call ur, with appropriate
    // overriding class pos/len, same effself and usage;
    return;
  } //genCodeVirtualFunctionCallVTableEntryUsingSpecifiedVTable

  void NodeFunctionCall::genCodeVirtualFunctionCallVTableEntryUsingEffectiveSelf(File * fp, u32 tvfpnum, u32 urtmpnum, u32& urtmpnumvfc)
  {
    assert(m_funcSymbol);
    //requires runtime lookup for virtual function pointer; needs
    //typedef typename for this vfunc (in any owner of this vfunc);
    //(ulam-5) requires runtime lookup for vfunc vowned index, plus
    //start index into virtual table (using registration number of
    //orig class), both known at compile time.
    UTI vownuti = m_funcSymbol->getVirtualMethodOriginatingClassUTI();
    u32 vownregnum = m_state.getAClassRegistrationNumber(vownuti);
    //VOWNED_IDX enum is the same regardless of effective self (e.g. t3600)
    // belongs to originating class; subclass knows offset in VT (ulam-5)
    UlamType * vownut = m_state.getUlamTypeByIndex(vownuti);

    //MAKE A STRING for reuse: vtable index between the parens w tmpvtstartidx
    std::ostringstream vtindexstring; //between parens
    vtindexstring << vownut->getUlamTypeMangledName().c_str();
    vtindexstring << "<EC>::"; //orignating class
    vtindexstring << "VOWNED_IDX_"; //== m_funcSymbol->getVirtualMethodIdx()
    vtindexstring <<m_funcSymbol->getMangledNameWithTypes().c_str();

    //requires runtime lookup for virtual function pointer
    m_state.indentUlamCode(fp);
    fp->write("VfuncPtr "); //legitimize this tmp label TODO
    fp->write(m_state.getVFuncPtrTmpNumAsString(tvfpnum).c_str()); //Uf_tvfpNNN
    fp->write(";"); GCNL;

    //generate shorthand UlamRef constructor for vfunc calls:
    urtmpnumvfc = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("UlamRef<EC> ");
    fp->write(m_state.getUlamRefTmpVarAsString(urtmpnumvfc).c_str());
    fp->write("(");
    if(urtmpnum > 0)
      fp->write(m_state.getUlamRefTmpVarAsString(urtmpnum).c_str());
    else
      fp->write(m_state.getHiddenArgName()); //ur
    fp->write(", ");
    fp->write(vtindexstring.str().c_str());
    fp->write(", ");
    fp->write_decimal_unsigned(vownregnum);
    fp->write("u /*");
    fp->write(m_state.getUlamTypeNameBriefByIndex(vownuti).c_str());
    fp->write("*/, true, "); //usage based on override class, (always true!)
    fp->write(m_state.getVFuncPtrTmpNumAsString(tvfpnum).c_str()); //Uf_tvfpNNN
    fp->write(");"); GCNL;

    // READY to make virtual function call ur, with appropriate
    // overriding class pos/len, same effself and usage;
    return;
  } //genCodeVirtualFunctionCallVTableEntryUsingEffectiveSelf

  void NodeFunctionCall::genCodeVirtualFunctionCall(File * fp, u32 tvfpnum)
  {
    assert(m_funcSymbol);
    //requires runtime lookup for virtual function pointer
    u32 vfidx = m_funcSymbol->getVirtualMethodIdx();
    UTI vownuti = m_funcSymbol->getVirtualMethodOriginatingClassUTI();
    UlamType * vownut = m_state.getUlamTypeByIndex(vownuti);

    //need typedef typename for this vfunc, any vtable of any owner of this vfunc
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = NULL; //any owner of func

    if(cosSize != 0)
      cos = m_state.m_currentObjSymbolsForCodeGen.back(); //"owner" of func t41376,t41354
    else
      cos = m_state.getCurrentSelfSymbolForCodeGen(); //'self'

    UTI cosuti = cos->getUlamTypeIdx();
    if(m_state.isAPrimitiveType(cosuti))
      {
	//special regnum syntax for specific vtable class (t41376)
	assert(cosSize > 1);
	cos = m_state.m_currentObjSymbolsForCodeGen[cosSize - 2]; //next to last
	cosuti = cos->getUlamTypeIdx();
	assert(!m_state.isAPrimitiveType(cosuti));
      }

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cosuti, csym);
    assert(isDefined);

    u32 coffset = csym->getVTstartoffsetOfRelatedOriginatingClass(vownuti);
    assert(coffset < UNRELIABLEPOS);
    UTI cvfuti = csym->getClassForVTableEntry(vfidx + coffset); //t41304

    // check that we are not trying to call a pure virtual function:
    //   t41158, t41160, t41094, safe t41161
    // too limiting (ulam-5) to limit to 'super' special case:
    //   t3606, t3608, t3774, t3779, t3788, t3794, t3795, t3967, t41131
    // once cos is a ref, all bets off! unclear effSelf, e.g. cos is 'self'
    //    if(m_state.getUlamTypeAsDeref(cosuti) != cvfuti) //multiple bases possible (ulam-5)
    if(!m_state.isAltRefType(cosuti) && (m_state.getUlamTypeAsDeref(cosuti) != cvfuti)) //multiple bases possible (ulam-5); issue +(t41361 TODO)
      {
	SymbolClass * cvfsym = NULL;
	AssertBool iscvfDefined = m_state.alreadyDefinedSymbolClass(cvfuti, cvfsym);
	assert(iscvfDefined);
	if(cvfsym->isPureVTableEntry(vfidx))
	  {
	    std::ostringstream msg;
	    msg << "Virtual function '" << m_funcSymbol->getMangledNameWithTypes().c_str();
	    msg << "' is pure; cannot be called";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }

    fp->write("((typename ");
    fp->write(vownut->getUlamTypeMangledName().c_str()); //any class fine for its typedef
    fp->write("<EC>::"); //same for elements and quarks
    fp->write(m_funcSymbol->getMangledNameWithTypes().c_str());
    fp->write(") (");
    fp->write(m_state.getVFuncPtrTmpNumAsString(tvfpnum).c_str());
    fp->write(")) ");
    //args to function pointer remain..
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

    bool usePassVal = m_state.m_currentObjSymbolsForCodeGen.empty();
    Symbol * stgcos = !usePassVal ? m_state.m_currentObjSymbolsForCodeGen[0] : NULL;

    //use possible dereference type for mangled name
    UTI derefuti = m_state.getUlamTypeAsDeref(vuti);
    assert(m_state.isAClass(derefuti));

   u32 tmpvarur = m_state.getNextTmpVarNumber();
    std::ostringstream hiddenarg2;
    //new ur to reflect "effective" self and the ref storage, for this funccall
    hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvarur).c_str() << "(";

    if(isaref)
      {
	if(usePassVal)
	  hiddenarg2 << uvpass.getTmpVarAsString(m_state).c_str();
	else
	  hiddenarg2 << stgcos->getMangledName().c_str();
	hiddenarg2 << ", 0u"; //references already offset t3811
      }
    else
      hiddenarg2 << uvpass.getPassPos() << "u"; //element refs already +25

    hiddenarg2 << ", " << getLengthOfMemberClassForHiddenArg(derefuti) << "u"; //len t3370

    if(!isaref)
      {
	hiddenarg2 << ", ";
	if(usePassVal)
	  hiddenarg2 << uvpass.getTmpVarAsString(m_state).c_str();
	else
	  hiddenarg2 << stgcos->getMangledName().c_str();
	hiddenarg2 << ", &";
	hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(derefuti).c_str();
	hiddenarg2 << ", " << Node::genUlamRefUsageAsString(derefuti).c_str();
	hiddenarg2 << ", uc";
      }

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
	    //ref arg to match non-ref parameter (t41120)
	    m_argumentNodes->genCode(fp, auvpass, i);
	    UTI auti = auvpass.getPassTargetType();
	    if(m_state.isReference(auti))
	      {
		auti = m_state.getUlamTypeAsDeref(auti);
		auvpass.setPassTargetType(auti); //t41120, casted for immediate constr
	      }
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

    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	if(UlamType::compare(uvpass.getPassTargetType(), vuti, m_state) == UTIC_SAME)
	  return; //t41199
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

} //end MFM
