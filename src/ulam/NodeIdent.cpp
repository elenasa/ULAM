#include <stdlib.h>
#include "NodeConstant.h"
#include "NodeIdent.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "NodeModelParameter.h"
#include "NodeTypeBitsize.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "SymbolModelParameterValue.h"
#include "SymbolTypedef.h"
#include "SymbolVariable.h"

namespace MFM {

  NodeIdent::NodeIdent(const Token& tok, SymbolVariable * symptr, CompilerState & state) : Node(state), m_token(tok), m_varSymbol(symptr), m_currBlockNo(0)
  {
    if(symptr)
      m_currBlockNo = symptr->getBlockNoOfST();
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeIdent::NodeIdent(const NodeIdent& ref) : Node(ref), m_token(ref.m_token), m_varSymbol(NULL), m_currBlockNo(ref.m_currBlockNo) {}

  NodeIdent::~NodeIdent(){}

  Node * NodeIdent::instantiate()
  {
    return new NodeIdent(*this);
  }

  void NodeIdent::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }

  const char * NodeIdent::getName()
  {
    return m_state.getTokenDataAsString(m_token).c_str();
  }

  const std::string NodeIdent::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeIdent::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return true;
  }

  void NodeIdent::setSymbolPtr(SymbolVariable * vsymptr)
  {
    assert(vsymptr);
    m_varSymbol = vsymptr;
    m_currBlockNo = vsymptr->getBlockNoOfST();
    assert(m_currBlockNo);
  }

  const Token& NodeIdent::getToken() const
  {
    return m_token;
  }

  bool NodeIdent::isAConstant()
  {
    bool rtn = false;
    //may not have known at parse time;
    if(!m_varSymbol)
      {
	//is it a constant within the member?
	NodeBlockContext * memberclass = m_state.getContextBlock();
	assert(memberclass);

	m_state.pushCurrentBlock(memberclass);

	Symbol * asymptr = NULL;
	bool hazyKin = false;
	if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
	  {
	    if(asymptr->isConstant())
	      {
		rtn = true; //no side-effects until c&l
	      }
	  }
	m_state.popClassContext(); //restore
      }
    return rtn;
  } //isAConstant

  FORECAST NodeIdent::safeToCastTo(UTI newType)
  {
    //ulamtype checks for complete, non array, and type specific rules
    return m_state.getUlamTypeByIndex(newType)->safeCast(getNodeType());
  } //safeToCastTo

  UTI NodeIdent::checkAndLabelType()
  {
    UTI it = Nouti;  //init (was Nav)
    u32 errCnt = 0;

    //checkForSymbol:
    //2 cases: use was before def, look up in class block; cloned unknown
    if(m_varSymbol == NULL)
      {
	//used before defined, start search with current block
	if(m_currBlockNo == 0)
	  {
	    if(m_state.useMemberBlock())
	      {
		NodeBlockClass * memberclass = m_state.getCurrentMemberClassBlock();
		assert(memberclass);
		m_currBlockNo = memberclass->getNodeNo();
	      }
	    else
	      m_currBlockNo = m_state.getCurrentBlock()->getNodeNo();
	  }

	UTI cuti = m_state.getCompileThisIdx(); //for error messages
	NodeBlock * currBlock = getBlock();
	if(m_state.useMemberBlock())
	  {
	    m_state.pushCurrentBlock(currBlock); //e.g. memberselect needed for already defined
	    cuti = m_state.getCurrentMemberClassBlock()->getNodeType();
	  }
	else
	  m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

	Symbol * asymptr = NULL;
	bool hazyKin = false;
	// don't capture symbol ptr yet if part of incomplete chain.
	if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin) && !hazyKin)
	  {
	    if(!asymptr->isFunction() && !asymptr->isTypedef() && !asymptr->isConstant() && !asymptr->isModelParameter())
	      {
		setSymbolPtr((SymbolVariable *) asymptr);
		//assert(asymptr->getBlockNoOfST() == m_currBlockNo); not necessarily true
		// e.g. var used before defined, and then is a data member outside current func block.
		m_currBlockNo = asymptr->getBlockNoOfST(); //refined
	      }
	    else if(asymptr->isConstant())
	      {
		// replace ourselves with a constant node instead;
		// same node no, and loc (e.g. t3573)
		//NodeConstant * newnode = new NodeConstant(*this);
		NodeConstant * newnode = new NodeConstant(m_token, (SymbolWithValue *) asymptr, m_state);
		assert(newnode);

		NNO pno = Node::getYourParentNo();
		m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock); //push again
		Node * parentNode = m_state.findNodeNoInThisClass(pno);
		if(!parentNode)
		  {
		    std::ostringstream msg;
		    msg << "Named Constant variable '" << getName();
		    msg << "' cannot be exchanged at this time while compiling class: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		    msg << " Parent required";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    assert(0); //parent required
		  }

		AssertBool swapOk = parentNode->exchangeKids(this, newnode);
		assert(swapOk);

		std::ostringstream msg;
		msg << "Exchanged kids! <" << m_state.getTokenDataAsString(m_token).c_str();
		msg << "> a named constant, in place of a variable with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

		newnode->setNodeLocation(getNodeLocation());
		newnode->setYourParentNo(pno); //missing?
		newnode->resetNodeNo(getNodeNo()); //missing?

		m_state.popClassContext(); //restore
		m_state.popClassContext(); //restore

		delete this; //suicide is painless..

		return newnode->checkAndLabelType();
	      }
	    else if(asymptr->isModelParameter())
	      {
		// replace ourselves with a parameter node instead;
		// same node no, and loc
		//NodeModelParameter * newnode = new NodeModelParameter(*this);
		NodeModelParameter * newnode = new NodeModelParameter(m_token, (SymbolModelParameterValue*) asymptr, m_state);
		assert(newnode);

		NNO pno = Node::getYourParentNo();
		m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock); //push again
		Node * parentNode = m_state.findNodeNoInThisClass(pno);
		if(!parentNode)
		  {
		    std::ostringstream msg;
		    msg << "Model Parameter variable '" << getName();
		    msg << "' cannot be exchanged at this time while compiling class: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		    msg << " Parent required";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    assert(0); //parent required
		  }

		AssertBool swapOk = parentNode->exchangeKids(this, newnode);
		assert(swapOk);

		std::ostringstream msg;
		msg << "Exchanged kids! <" << m_state.getTokenDataAsString(m_token).c_str();
		msg << "> a model parameter, in place of a variable with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

		newnode->setNodeLocation(getNodeLocation());
		newnode->setYourParentNo(pno); //missing?
		newnode->resetNodeNo(getNodeNo()); //missing?

		m_state.popClassContext(); //restore
		m_state.popClassContext(); //restore

		delete this; //suicide is painless..

		return newnode->checkAndLabelType();
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.getTokenDataAsString(m_token).c_str();
		msg << "> is not a variable, and cannot be used as one with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		it = Nav;
		errCnt++;
	      }
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) <" << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "> is not defined, and cannot be used with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    if(!hazyKin)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		it = Nav;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		it = Hzy;
		m_state.setGoAgain();
	      }
	    errCnt++;
	    m_state.popClassContext(); //restore
	  }
      } //lookup symbol

    if(!errCnt && m_varSymbol)
      {
	it = m_varSymbol->getUlamTypeIdx();
	Node::setStoreIntoAble(TBOOL_TRUE); //store into an array entotal?

	//from NodeTypeDescriptor..e.g. for function call args in NodeList.
	if(!m_state.isComplete(it))
	  {
	    // if Nav, use token
	    UTI mappedUTI = it;
	    UTI cuti = m_state.getCompileThisIdx();

	    // the symbol associated with this type, was mapped during instantiation
	    // since we're call AFTER that (not during), we can look up our
	    // new UTI and pass that on up the line of NodeType Selects, if any.
	    if(m_state.mappedIncompleteUTI(cuti, it, mappedUTI))
	      {
		std::ostringstream msg;
		msg << "Substituting Mapped UTI" << mappedUTI;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
		msg << " for incomplete list type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << "' UTI" << it << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		m_state.mapTypesInCurrentClass(it, mappedUTI); //before setting equal?
		m_varSymbol->resetUlamType(mappedUTI); //consistent!
		it = mappedUTI;
	      }
	    else if(m_varSymbol->isSelf() || m_state.isReference(it) || m_varSymbol->isSuper())
	      {
		m_state.completeAReferenceType(it);
	      }

	    if(!m_state.isComplete(it)) //reloads to recheck for debug message
	      {
		std::ostringstream msg;
		msg << "Incomplete identifier for type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << ", used with list symbol name '" << getName() << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		it = Hzy; //missing t3754 case 1
		m_state.setGoAgain();
	      }
	  }
      }

    if(m_varSymbol && !m_varSymbol->isDataMember() && (((SymbolVariableStack *) m_varSymbol)->getDeclNodeNo() > getNodeNo()))
      {
	std::ostringstream msg;
	msg << "Local variable '" << getName();
	msg << "' was used before declared";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	it = Nav; //error/t3797
      }
    setNodeType(it);
    return it;
  } //checkAndLabelType

  NNO NodeIdent::getBlockNo() const
  {
    return m_currBlockNo;
  }

  NodeBlock * NodeIdent::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  EvalStatus NodeIdent::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return UNEVALUABLE;

    evalNodeProlog(0); //new current frame pointer

    //return the ptr for an array; square bracket will resolve down to the immediate data
    UlamValue uv;
    UlamValue uvp = makeUlamValuePtr();

    if(m_state.isScalar(nuti))
      {
	if(m_state.isReference(uvp.getPtrTargetType()))
	  uvp = m_state.getPtrTarget(uvp);

	uv = m_state.getPtrTarget(uvp);
	UTI ttype = uv.getUlamValueTypeIdx();

	// redo what getPtrTarget use to do, when types didn't match due to
	// an element/quark or a requested scalar of an arraytype
	if(m_state.isPtr(ttype) || (UlamType::compare(ttype, nuti, m_state) == UTIC_NOTSAME))
	  {
	    if(m_state.isClassACustomArray(nuti))
	      {
		UTI caType = m_state.getAClassCustomArrayType(nuti);
		UlamType * caut = m_state.getUlamTypeByIndex(caType);
		if(m_state.isAtom(caType) || (caut->getBitSize() > MAXBITSPERINT))
		  {
		    uv = uvp; //customarray
		  }
		else
		  {
		    s32 len = uvp.getPtrLen();
		    assert(len != UNKNOWNSIZE);
		    if(len <= MAXBITSPERINT)
		      {
			u32 datavalue = uv.getDataFromAtom(uvp, m_state);
			uv = UlamValue::makeImmediate(nuti, datavalue, m_state);
		      }
		    else if(len <= MAXBITSPERLONG)
		      {
			u64 datavalue = uv.getDataLongFromAtom(uvp, m_state);
			uv = UlamValue::makeImmediateLong(nuti, datavalue, m_state);
		      }
		    else
		      assert(0);
		  }
	      }
	    else
	      {
		UlamType * nut = m_state.getUlamTypeByIndex(nuti);
		if((m_state.isAtom(nuti) || (classtype == UC_ELEMENT)) && (nut->isScalar() || nut->isReference()))
		  {
		    uv = m_state.getPtrTarget(uvp); //error/t3676, error/t3677
		  }
		else
		  {
		    //includes (evaluable) transients (t3739)
		    UTI vuti = uv.getUlamValueTypeIdx();
		    // does this handle a ptr to a ptr (e.g. "self")? (see makeUlamValuePtr)
		    if(m_state.isPtr(vuti))
		      {
			uvp = uv;
			uv = m_state.getPtrTarget(uvp);
		      }

		    s32 len = uvp.getPtrLen();
		    assert(len != UNKNOWNSIZE);
		    if(len <= MAXBITSPERINT)
		      {
			u32 datavalue = uv.getDataFromAtom(uvp, m_state);
			uv = UlamValue::makeImmediate(nuti, datavalue, m_state);
		      }
		    else if(len <= MAXBITSPERLONG)
		      {
			u64 datavalue = uv.getDataLongFromAtom(uvp, m_state);
			uv = UlamValue::makeImmediateLong(nuti, datavalue, m_state);
		      }
		    else
		      assert(0);
		  }
	      }
	  } // not node type
      } //scalar
    else
      uv = uvp; //even if reference???

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeIdent::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    assert(m_varSymbol);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return UNEVALUABLE;

    TBOOL stor = Node::getStoreIntoAble();
    if(stor != TBOOL_TRUE) //i.e. an MP
      {
	std::ostringstream msg;
	msg << "Variable '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' is not a valid lefthand side. Eval FAILS";
	if(stor == TBOOL_HAZY)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    return NOTREADY;
	  }
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return ERROR;
      }

    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();

    //must remain a ptr!!!
    if(m_state.isReference(rtnUVPtr.getPtrTargetType()) && (rtnUVPtr.getPtrStorage() == STACK))
      {
	UlamValue tmpref = m_state.getPtrTarget(rtnUVPtr);
	 if(tmpref.isPtr())
	   rtnUVPtr = tmpref;
	 //else no change? (e.g. t3407)
      }

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeIdent::makeUlamValuePtr()
  {
    if(m_varSymbol->isSelf())
      {
	//when "self" is a quark, we're inside a func called on a quark (e.g. dm or local)
	//'atomof' gets entire atom/element containing this quark; including its type!
	//'self' gets type/pos/len of the quark from which 'atom' can be extracted
	UlamValue selfuvp = m_state.m_currentSelfPtr;
	UTI ttype = selfuvp.getPtrTargetType();
	assert(m_state.okUTItoContinue(ttype));
	if((m_state.getUlamTypeByIndex(ttype)->getUlamClassType() == UC_QUARK))
	  {
	    if(m_varSymbol->isSelf())
	      selfuvp = m_state.getAtomPtrFromSelfPtr();
	    //else
	  }
	return selfuvp;
      } //done

    //can't use global m_currentAutoObjPtr, since there might be nested as conditional blocks.
    // NodeVarDecl for this autolocal sets AutoPtrForEval during its eval.
    // ALT_REF, ALT_ARRAYITEM cannot guarantee its NodeVarRef init was last encountered, like ALT_AS.
    if(m_varSymbol->getAutoLocalType() == ALT_AS)
      return ((SymbolVariableStack *) m_varSymbol)->getAutoPtrForEval(); //haha! we're done.

    UlamValue ptr;
    if(m_varSymbol->isDataMember())
      {
	// return ptr to this data member within the m_currentObjPtr
	// 'pos' modified by this data member symbol's packed bit position
	ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + ((SymbolVariableDataMember *) m_varSymbol)->getPosOffset(), m_varSymbol->getId());

	ptr.checkForAbsolutePtr(m_state.m_currentObjPtr); //t3810
      }
    else
      {
	//DEBUG ONLY!!, to view ptr saved with Ref's m_varSymbol.
#if 0
	if(m_varSymbol->isAutoLocal()) //ALT_REF or ALT_ARRAYITEM
	  ptr = ((SymbolVariableStack *) m_varSymbol)->getAutoPtrForEval();
#endif
	//local variable on the stack; could be array ptr! could be 'super'
	ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
      }
    return ptr;
  } //makeUlamValuePtr

  //new
  void NodeIdent::makeUVPassForCodeGen(UVPass& uvpass)
  {
    assert(m_varSymbol);
    s32 tmpnum = m_state.getNextTmpVarNumber();

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    u32 pos = uvpass.getPassPos(); //Thu Jun 23 16:01:08 2016
    if(m_varSymbol->isDataMember())
      {
	if(!m_state.m_currentObjSymbolsForCodeGen.empty())
	  {
	    Symbol * sym = m_state.m_currentObjSymbolsForCodeGen.back();
	    //here, we haven't taken into account any array indexes, So autoref instead
	    // e.g. m_bar[0].cb, and this NI is for the rhs of member select, 'cb'

	    //if sym is an element, and not isSelf, and not a ref, pos += 25 (t3637)
	    UTI suti = sym->getUlamTypeIdx();
	    if(!sym->isSelf() && Node::needAdjustToStateBits(suti))
	      pos += ATOMFIRSTSTATEBITPOS;
	  }
	// 'pos' modified by this data member symbol's packed bit position
	uvpass = UVPass::makePass(tmpnum, nut->getTmpStorageTypeForTmpVar(), nuti, m_state.determinePackable(nuti), m_state, pos + ((SymbolVariableDataMember *) m_varSymbol)->getPosOffset(), m_varSymbol->getId());
      }
    else
      //local variable on the stack; could be array ptr!
      uvpass = UVPass::makePass(tmpnum, nut->getTmpStorageTypeForTmpVar(), nuti, m_state.determinePackable(nuti), m_state, 0, m_varSymbol->getId());
  } //makeUVPassForCodeGen

  bool NodeIdent::installSymbolTypedef(TypeArgs& args, Symbol *& asymptr)
  {
    bool brtn = false;
    UTI tduti = Nav;
    UTI tdscalaruti = Nav;
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr))
      {
	tduti = asymptr->getUlamTypeIdx();
	if(asymptr->isTypedef())
	  {
	    if(m_state.isHolder(tduti))
	      {
		//not Nav when tduti's an array; might know?
		args.m_declListOrTypedefScalarType = tdscalaruti;

		UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
		ULAMCLASSTYPE tclasstype = tdut->getUlamClassType();
		// keep the out-of-band name; other's might refer to its UTI.
		// if its UTI is a unseen class, we can update the name of the class later
		// don't want to rush this step since we might have a class w args and diff UTI.
		if((tclasstype == UC_NOTACLASS))
		  {
		    ULAMTYPE bUT = m_state.getBaseTypeFromToken(args.m_typeTok);

		    // if not a class, but a primitive type update the key
		    if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
		      {
			if(args.m_bitsize == 0)
			  args.m_bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];
			// update the type of holder key
			UlamKeyTypeSignature newkey(m_state.getTokenAsATypeNameId(args.m_typeTok), args.m_bitsize, args.m_arraysize, Nouti, args.m_declRef);
			m_state.makeUlamTypeFromHolder(newkey, bUT, tduti, UC_NOTACLASS); //update key, same uti
			if(m_state.hasUnknownTypeInThisClassResolver(tduti))
			  m_state.removeKnownTypeTokenFromThisClassResolver(tduti);
		      }
		    else
		      {
			//update holder key with name_id and possible array (UNKNOWNSIZE)
			//possibly a class (t3379)
			UlamKeyTypeSignature newkey(m_state.getTokenAsATypeNameId(args.m_typeTok), args.m_bitsize, args.m_arraysize, args.m_classInstanceIdx, args.m_declRef);
			m_state.makeUlamTypeFromHolder(newkey, bUT, tduti, bUT == Class ? UC_UNSEEN : UC_NOTACLASS); //update holder key, same uti
		      }
		  }
		brtn = true;
	      } //holder done
	    else if(asymptr->getId() == m_state.m_pool.getIndexForDataString("Self"))
	      brtn = false; //e.g. error/t3391, error/t3698
	    else if(asymptr->getId() == m_state.m_pool.getIndexForDataString("Super"))
	      brtn = false;
	    else
	      brtn = true;
	  } //a typedef already there
	return brtn; //already there, and updated
      }
    //    else

    SymbolClassName * prematureclass = NULL;
    bool isUnseenClass = false;
    UTI pmcuti = Nouti;
    if(m_state.alreadyDefinedSymbolClassName(m_token.m_dataindex, prematureclass))
      {
	pmcuti = prematureclass->getUlamTypeIdx();
	isUnseenClass = (m_state.getUlamTypeByIndex(pmcuti)->getUlamClassType() == UC_UNSEEN);

	std::ostringstream msg;
	msg << "Typedef alias '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' already exists as ";
	if(isUnseenClass)
	  msg << " UNSEEN ";
	msg << "class type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(pmcuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //issue 5/6/16
	return false; //quit!
      }

    if(args.m_anothertduti != Nouti)
      {
	//typedef might have bitsize and arraysize info..
	if(checkTypedefOfTypedefSizes(args, args.m_anothertduti)) //ref
	  {
	    tduti = args.m_anothertduti;
	    brtn = true;
	  }
      }
    else if(m_state.getUlamTypeByTypedefName(args.m_typeTok.m_dataindex, tduti, tdscalaruti))
      {
	args.m_declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	if(checkTypedefOfTypedefSizes(args, tduti)) //ref
	  {
	    brtn = true;
	  }
      }
    else if(args.m_declListOrTypedefScalarType != Nouti)
      {
	// if m_anothertduti fails first (this could be the scalar for it!)
	if(!checkTypedefOfTypedefSizes(args, args.m_declListOrTypedefScalarType))
	  return false;
	tduti = args.m_declListOrTypedefScalarType;
	brtn = true;
      }
    else if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first. scalar uti
	tduti = m_state.makeUlamType(args.m_typeTok, args.m_bitsize, NONARRAYSIZE, Nouti, args.m_declRef);
	brtn = true;
      }
    else
      {
	tduti = args.m_classInstanceIdx;
	brtn = true;
      }

    if(brtn)
      {
	UTI uti = tduti;
	UTI scalarUTI = args.m_declListOrTypedefScalarType;
	UlamType * ut = m_state.getUlamTypeByIndex(uti);
	ULAMTYPE bUT = ut->getUlamTypeEnum();
	UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
	ULAMCLASSTYPE classtype = ut->getUlamClassType();

	if(ut->isScalar() && (args.m_arraysize != NONARRAYSIZE))
	  {
	    args.m_declListOrTypedefScalarType = scalarUTI = uti;
	    // o.w. build symbol (with bit and array sizes);
	    // arrays can't have their scalar as classInstance;
	    // o.w., no longer findable by token.
	    if(args.m_bitsize == 0)
	      args.m_bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

	    UlamKeyTypeSignature newarraykey(key.getUlamKeyTypeSignatureNameId(), args.m_bitsize, args.m_arraysize, scalarUTI, args.m_declRef); //classinstanceidx removed if not a class
	    uti = m_state.makeUlamType(newarraykey, bUT, classtype);
	  }
	else if(m_state.getReferenceType(uti) != args.m_declRef)
	  {
	    // could be array or scalar ref
	    UlamKeyTypeSignature newkey(key.getUlamKeyTypeSignatureNameId(), key.getUlamKeyTypeSignatureBitSize(), key.getUlamKeyTypeSignatureArraySize(), key.getUlamKeyTypeSignatureClassInstanceIdx(), args.m_declRef); //classinstanceidx removed if not a class
	    uti = m_state.makeUlamType(newkey, bUT, classtype);
	  }
	//create a symbol for this new ulam type, a typedef, with its type
	SymbolTypedef * symtypedef = new SymbolTypedef(m_token, uti, scalarUTI, m_state);
	m_state.addSymbolToCurrentScope(symtypedef);

	//remember tduti for references
	symtypedef->setAutoLocalType(m_state.getReferenceType(uti));

	//check if also a holder (not necessary)
	//if(ut->isHolder()) m_state.addUnknownTypeTokenToThisClassResolver(m_token, uti);

	return (m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr)); //true
      }
    return false;
  } //installSymbolTypedef

  bool NodeIdent::installSymbolConstantValue(TypeArgs& args, Symbol*& asymptr)
  {
    // ask current scope block if this constant name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    UTI holdUTIForAlias = Nouti;
    if(m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr))
      {
	if(m_state.isHolder(asymptr->getUlamTypeIdx()))
	  holdUTIForAlias = asymptr->getUlamTypeIdx(); //t3862
	else
	  return false; //already there
      }

    // maintain specific type (see isAConstant() Node method)
    bool brtn = false;
    UTI uti = Nav;
    UTI tdscalaruti = Nav;

    if(args.m_anothertduti != Nouti)
      {
	if(checkConstantTypedefSizes(args, args.m_anothertduti))
	  {
	    brtn = true;
	    uti = args.m_anothertduti;
	  }
      }
    else if(m_state.getUlamTypeByTypedefName(args.m_typeTok.m_dataindex, uti, tdscalaruti))
      {
	args.m_declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	if(checkConstantTypedefSizes(args, uti))
	  {
	    brtn = true;
	  }
      }
    else if(args.m_declListOrTypedefScalarType != Nouti)
      {
	if(!checkConstantTypedefSizes(args, args.m_declListOrTypedefScalarType))
	  return false;
	uti = args.m_declListOrTypedefScalarType;
	brtn = true;
      }
    else if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first.
	// scalar uti
	uti = m_state.makeUlamType(args.m_typeTok, args.m_bitsize, NONARRAYSIZE, Nouti);
	brtn = true;
      }
    else
      {
	// no class types for constants
	std::ostringstream msg;
	msg << "Named Constant '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' cannot be based on a class type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(args.m_classInstanceIdx).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    if(brtn)
      {
	if(!asymptr)
	  {
	    //create a symbol for this new named constant, a constant-def, with its value
	    SymbolConstantValue * symconstdef = new SymbolConstantValue(m_token, uti, m_state);
	    m_state.addSymbolToCurrentScope(symconstdef);
	  }
	else
	  {
	    UlamType * newut = m_state.getUlamTypeByIndex(uti);
	    UlamKeyTypeSignature newkey = newut->getUlamKeyTypeSignature();
	    m_state.makeUlamTypeFromHolder(newkey, newut->getUlamTypeEnum(), holdUTIForAlias, newut->getUlamClassType());
	  }

	if(holdUTIForAlias != Nouti)
	  m_state.updateUTIAliasForced(holdUTIForAlias, uti); //t3862

	//gets the symbol just created by makeUlamType; true.
	return (m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr));
      }
    return false;
  } //installSymbolConstantValue

  bool NodeIdent::installSymbolModelParameterValue(TypeArgs& args, Symbol*& asymptr)
  {
    // ask current scope block if this constant name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr))
      {
	if(m_state.isHolder(asymptr->getUlamTypeIdx()))
	  {
	    //remove it! then continue..
	    Symbol * rmsym = NULL;
	    if(m_state.getCurrentBlock()->removeIdFromScope(m_token.m_dataindex, rmsym))
	      {
		assert(rmsym == asymptr); //sanity check removal
		asymptr = NULL;
	      }
	  }
	else
	  return false; //already there
      }

    // maintain specific type (see isAConstant() Node method)
    bool brtn = false;
    UTI uti = Nav;
    UTI tdscalaruti = Nav;
    if(args.m_anothertduti != Nouti)
      {
	if(checkConstantTypedefSizes(args, args.m_anothertduti))
	  {
	    brtn = true;
	    uti = args.m_anothertduti;
	  }
      }
    else if(m_state.getUlamTypeByTypedefName(args.m_typeTok.m_dataindex, uti, tdscalaruti))
      {
	args.m_declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	if(checkConstantTypedefSizes(args, uti))
	  {
	    brtn = true;
	  }
      }
    else if(args.m_declListOrTypedefScalarType != Nouti)
      {
	if(!checkConstantTypedefSizes(args, args.m_declListOrTypedefScalarType))
	  return false;
	uti = args.m_declListOrTypedefScalarType;
	brtn = true;
      }
    else if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first.
	// scalar uti
	uti = m_state.makeUlamType(args.m_typeTok, args.m_bitsize, NONARRAYSIZE, Nouti);
	brtn = true;
      }
    else
      {
	// no class types for model parameters
	std::ostringstream msg;
	msg << "Model Parameter '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' cannot be based on a class type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(args.m_classInstanceIdx).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    if(brtn)
      {
	//create a symbol for this new model parameter, a parameter-def, with its value
	SymbolModelParameterValue * symparam = new SymbolModelParameterValue(m_token, uti, m_state);
	m_state.addSymbolToCurrentScope(symparam);

	//gets the symbol just created by makeUlamType; true.
	return (m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr));
      }
    return false;
  } //installSymbolModelParameterValue

  //see also NodeSquareBracket
  bool NodeIdent::installSymbolVariable(TypeArgs& args, Symbol *& asymptr)
  {
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr))
      {
	if(!(asymptr->isFunction()) && !(asymptr->isTypedef()) && !(asymptr->isConstant()) && !(asymptr->isModelParameter()))
	  setSymbolPtr((SymbolVariable *) asymptr); //updates Node's symbol, if is variable
	return false; //already there
      }

    bool brtn = false;
    UTI auti = Nav;
    UTI tdscalaruti = Nav;
    // verify typedef exists for this scope; or is a primitive keyword type
    // if a primitive (NONARRAYSIZE), we may need to make a new arraysize type for it;
    // or if it is a class type (quark, element).
    //list of decls can use the same 'scalar' type (arg); adjusted for arrays
    if(args.m_anothertduti != Nouti)
      {
	if(!checkVariableTypedefSizes(args, args.m_anothertduti))
	  return false;
	//use another classes' typedef uti; (is classInstanceIdx relevant?)
	auti = args.m_anothertduti;
	brtn = true;
      }
    else if(m_state.getUlamTypeByTypedefName(args.m_typeTok.m_dataindex, auti, tdscalaruti))
      {
	args.m_declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	// check typedef types here..
	if(!checkVariableTypedefSizes(args, auti))
	  return false;
	brtn = true;
      }
    else if(args.m_declListOrTypedefScalarType != Nouti)
      {
	if(!checkVariableTypedefSizes(args, args.m_declListOrTypedefScalarType))
	  return false;
	auti = args.m_declListOrTypedefScalarType;
	brtn = true;
      }
    else if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first.
	auti = m_state.makeUlamType(args.m_typeTok, args.m_bitsize, args.m_arraysize, Nouti, args.m_declRef);
	brtn = true;
      }
    else
      {
	auti = args.m_classInstanceIdx;
	brtn = true;
      }

    if(brtn)
      {
	UTI uti = auti;
	UTI scalarUTI = args.m_declListOrTypedefScalarType;
	if(m_state.isScalar(uti) && args.m_arraysize != NONARRAYSIZE)
	  {
	    args.m_declListOrTypedefScalarType = scalarUTI = uti;
	    // o.w. build symbol (with bit and array sizes);
	    // array's can't have their scalar as classInstance;
	    // o.w., no longer findable by token.
	    UlamType * ut = m_state.getUlamTypeByIndex(uti);
	    ULAMTYPE bUT = ut->getUlamTypeEnum();
	    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
	    ULAMCLASSTYPE classtype = ut->getUlamClassType();

	    if(args.m_bitsize == 0)
	      args.m_bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

	    UlamKeyTypeSignature newarraykey(key.getUlamKeyTypeSignatureNameId(), args.m_bitsize, args.m_arraysize, scalarUTI, key.getUlamKeyTypeSignatureReferenceType()); //same reftype as key
	    auti = m_state.makeUlamType(newarraykey, bUT, classtype);
	  }

	uti = m_state.getUlamTypeAsRef(auti, args.m_declRef); //ut not current; no deref.

	SymbolVariable * sym = makeSymbol(uti, m_state.getReferenceType(uti), args.m_referencedUTI);
	if(sym)
	  {
	    m_state.addSymbolToCurrentScope(sym); //ownership goes to the block
	    setSymbolPtr(sym); //sets m_varSymbol and st block no.
	    asymptr = sym;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Variable symbol '";
	    msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	    msg << "' cannot be a reference";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    brtn = false;
	  }
      }
    return brtn;
  } //installSymbolVariable

  SymbolVariable *  NodeIdent::makeSymbol(UTI auti, ALT reftype, UTI referencedUTI)
  {
    if(m_state.m_currentFunctionBlockDeclSize == 0)
      {
	u32 baseslot = 1;  //unpacked fixed later

	//variable-index, ulamtype, ulamvalue(ownership to symbol); always packed
	if(reftype != ALT_NOT)
	  return NULL; //error! dm's not references
	return (new SymbolVariableDataMember(m_token, auti, baseslot, m_state));
      }

    //Symbol is a parameter; always on the stack
    if(m_state.m_currentFunctionBlockDeclSize < 0)
      {
	//1 slot for scalar or packed array
	m_state.m_currentFunctionBlockDeclSize -= m_state.slotsNeeded(auti);

	SymbolVariableStack * rtnSym = (new SymbolVariableStack(m_token, auti, m_state.m_currentFunctionBlockDeclSize, m_state)); //slot after adjust
	assert(rtnSym);

	rtnSym->setAutoLocalType(reftype);

	return rtnSym;
      }

    //(else) Symbol is a local variable, always on the stack
    SymbolVariableStack * rtnLocalSym = new SymbolVariableStack(m_token, auti, m_state.m_currentFunctionBlockDeclSize, m_state); //slot before adjustment
    assert(rtnLocalSym);

    rtnLocalSym->setAutoLocalType(reftype);

    m_state.m_currentFunctionBlockDeclSize += m_state.slotsNeeded(auti);

    //adjust max depth, excluding parameters and initial start slot (=1)
    if(m_state.m_currentFunctionBlockDeclSize - 1 > m_state.m_currentFunctionBlockMaxDepth)
      m_state.m_currentFunctionBlockMaxDepth = m_state.m_currentFunctionBlockDeclSize - 1;

    return rtnLocalSym;
  } //makeSymbol

  bool NodeIdent::checkVariableTypedefSizes(TypeArgs& args, UTI auti)
  {
    bool rtnb = true;
    UlamType * tdut = m_state.getUlamTypeByIndex(auti);
    s32 tdarraysize = tdut->getArraySize();
    if(args.m_arraysize != NONARRAYSIZE && tdarraysize != NONARRAYSIZE)
      {
	//error can't support double arrays
	std::ostringstream msg;
	msg << "Array size [] is included in typedef '";
	msg <<  m_state.getTokenDataAsString(args.m_typeTok).c_str();
	msg << "', and cannot be redefined by variable '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    else  //variable not array; leave as-is when unknown
      {
	if(args.m_arraysize == UNKNOWNSIZE || tdarraysize == UNKNOWNSIZE)
	  args.m_arraysize = UNKNOWNSIZE;
	else
	  args.m_arraysize = tdarraysize; //use whatever typedef is
      }

    s32 tdbitsize = tdut->getBitSize();
    if(args.m_bitsize > 0)  //variable's
      {
	if(tdbitsize != args.m_bitsize)  //was (tdbitsize > 0)
	  {
	    //error can't support different bitsizes
	    std::ostringstream msg;
	    msg << "Bit size (" << tdbitsize << ") is included in typedef '";
	    msg <<  m_state.getTokenDataAsString(args.m_typeTok).c_str();
	    msg << "', and cannot be redefined by variable '";
	    msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false;
	  }
      }
    else  //variable unknown bitsize
      {
	args.m_bitsize = tdbitsize; //use whatever typedef is
      }
    return rtnb;
  } //checkVariableTypedefSizes

  bool NodeIdent::checkTypedefOfTypedefSizes(TypeArgs& args, UTI tduti)
  {
    bool rtnb = true;
    UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
    s32 tdarraysize = tdut->getArraySize();
    if(args.m_arraysize != NONARRAYSIZE && tdarraysize != NONARRAYSIZE)
      {
	//error can't support typedefs changing arraysizes
	std::ostringstream msg;
	msg << "Array size [] is included in typedef '";
	msg <<  m_state.getTokenDataAsString(args.m_typeTok).c_str();
	msg << "', and cannot be redefined by typedef '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    else
      {
	if(args.m_arraysize == UNKNOWNSIZE || tdarraysize == UNKNOWNSIZE)
	  args.m_arraysize = UNKNOWNSIZE;
	else
	  args.m_arraysize = tdarraysize; //use whatever typedef is
      }

    args.m_bitsize = tdut->getBitSize(); //ok to use typedef bitsize
    return rtnb;
  } //checkTypedefOfTypedefSizes

  bool NodeIdent::checkConstantTypedefSizes(TypeArgs& args, UTI tduti)
  {
    bool rtnb = true;
    UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
    s32 tdarraysize = tdut->getArraySize();
    if(args.m_arraysize != NONARRAYSIZE || tdarraysize != NONARRAYSIZE)
      {
	//error can't support named constant arrays
	std::ostringstream msg;
	msg << "Array size [] is included in typedef '";
	msg <<  m_state.getTokenDataAsString(args.m_typeTok).c_str();
	msg  << "', and cannot be used by a named constant '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    assert(args.m_arraysize == NONARRAYSIZE);

    args.m_bitsize = tdut->getBitSize(); //ok to use typedef bitsize

    // constants can't be classes either
    if(tdut->getUlamClassType() != UC_NOTACLASS)
      {
	std::ostringstream msg;
	msg << "Named Constant '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' cannot be a class type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(tduti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    return rtnb;
  } //checkConstantTypedefSizes

  void NodeIdent::genCode(File * fp, UVPass & uvpass)
  {
    //return the ptr for an array; square bracket will resolve down to the immediate data
    makeUVPassForCodeGen(uvpass);

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol); //*********UPDATED GLOBAL;

    // UNCLEAR: should this be consistent with constants?
    genCodeReadIntoATmpVar(fp, uvpass);
  } //genCode

  void NodeIdent::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    //e.g. return the ptr for an array;
    //square bracket will resolve down to the immediate data
   makeUVPassForCodeGen(uvpass);

    //******UPDATED GLOBAL; no restore!!!**************************
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol);

    if(uvpass.getPassStorage() == TMPAUTOREF)
      Node::genCodeConvertATmpVarIntoAutoRef(fp, uvpass); //uvpass becomes the autoref, and clears stack
  } //genCodeToStoreInto

  // overrides NodeTerminal that reads into a tmp var BitVector
  void NodeIdent::genCodeReadIntoATmpVar(File * fp, UVPass & uvpass)
  {
    if(uvpass.getPassStorage() == TMPAUTOREF)
      Node::genCodeConvertATmpVarIntoAutoRef(fp, uvpass); //uvpass becomes the autoref, and clears stack

    Node::genCodeReadIntoATmpVar(fp, uvpass);
  }

} //end MFM
