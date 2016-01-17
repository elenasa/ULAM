#include <stdlib.h>
#include "NodeConstant.h"
#include "NodeIdent.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "NodeModelParameter.h"
#include "NodeTypeBitsize.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "SymbolParameterValue.h"
#include "SymbolTypedef.h"
#include "SymbolVariable.h"

namespace MFM {

  NodeIdent::NodeIdent(Token tok, SymbolVariable * symptr, CompilerState & state) : Node(state), m_token(tok), m_varSymbol(symptr), m_currBlockNo(0)
  {
    if(symptr)
      m_currBlockNo = symptr->getBlockNoOfST();
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
    return m_state.getTokenDataAsString(&m_token).c_str();
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
	NodeBlockClass * memberclass = m_state.getClassBlock();
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
    UTI it = Nav;  //init
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

	NodeBlock * currBlock = getBlock();
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
	      }
	    else if(asymptr->isConstant())
	      {
		// replace ourselves with a constant node instead;
		// same node no, and loc
		NodeConstant * newnode = new NodeConstant(*this);
		NNO pno = Node::getYourParentNo();
		Node * parentNode = m_state.findNodeNoInThisClass(pno);
		if(!parentNode)
		  {
		    std::ostringstream msg;
		    msg << "Named Constant variable '" << getName();
		    msg << "' cannot be exchanged at this time while compiling class: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		    msg << " Parent required";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    assert(0); //parent required
		  }

		AssertBool swapOk = parentNode->exchangeKids(this, newnode);
		assert(swapOk);

		std::ostringstream msg;
		msg << "Exchanged kids! <" << m_state.getTokenDataAsString(&m_token).c_str();
		msg << "> a named constant, in place of a variable with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

		newnode->setYourParentNo(pno); //missing?
		newnode->resetNodeNo(getNodeNo()); //missing?

		m_state.popClassContext(); //restore

		delete this; //suicide is painless..

		return newnode->checkAndLabelType();
	      }
	    else if(asymptr->isModelParameter())
	      {
		// replace ourselves with a parameter node instead;
		// same node no, and loc
		NodeModelParameter * newnode = new NodeModelParameter(*this);
		NNO pno = Node::getYourParentNo();
		Node * parentNode = m_state.findNodeNoInThisClass(pno);
		if(!parentNode)
		  {
		    std::ostringstream msg;
		    msg << "Model Parameter variable '" << getName();
		    msg << "' cannot be exchanged at this time while compiling class: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		    msg << " Parent required";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    assert(0); //parent required
		  }

		AssertBool swapOk = parentNode->exchangeKids(this, newnode);
		assert(swapOk);

		std::ostringstream msg;
		msg << "Exchanged kids! <" << m_state.getTokenDataAsString(&m_token).c_str();
		msg << "> a model parameter, in place of a variable with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

		newnode->setYourParentNo(pno); //missing?
		newnode->resetNodeNo(getNodeNo()); //missing?

		m_state.popClassContext(); //restore

		delete this; //suicide is painless..

		return newnode->checkAndLabelType();
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.getTokenDataAsString(&m_token).c_str();
		msg << "> is not a variable, and cannot be used as one with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errCnt++;
	      }
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    m_state.popClassContext(); //restore
	    std::ostringstream msg;
	    msg << "(2) <" << m_state.getTokenDataAsString(&m_token).c_str();
	    msg << "> is not defined, and cannot be used with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    if(!hazyKin)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    errCnt++;
	  }
      } //lookup symbol

    if(!errCnt && m_varSymbol)
      {
	it = m_varSymbol->getUlamTypeIdx();
	setStoreIntoAble(true); //store into an array entotal?

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

	    if(!m_state.isComplete(it)) //reloads to recheck for debug message
	      {
		std::ostringstream msg;
		msg << "Incomplete identifier for type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << " used with list symbol name '" << getName();
		msg << "' UTI" << it << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		//it = Hzy;
		//m_state.setGoAgain();
	      }
	  }
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
	if((ttype == Ptr) || (UlamType::compare(ttype, nuti, m_state) == UTIC_NOTSAME))
	  {
	    if(m_state.isClassACustomArray(nuti))
	      {
		UTI caType = m_state.getAClassCustomArrayType(nuti);
		UlamType * caut = m_state.getUlamTypeByIndex(caType);
		if(caut->getUlamTypeEnum() == UAtom || caut->getBitSize() > MAXBITSPERINT)
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
		if(nut->getUlamTypeEnum() == UAtom || nut->getUlamClass() == UC_ELEMENT)
		  {
		    uv = m_state.getPtrTarget(uvp);
		  }
		else
		  {
		    UTI vuti = uv.getUlamValueTypeIdx();
		    // does this handle a ptr to a ptr (e.g. "self")? (see makeUlamValuePtr)
		    if(vuti == Ptr)
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
      uv = uvp;

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

    if(!isStoreIntoAble()) //i.e. an MP
      {
	std::ostringstream msg;
	msg << "Variable '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' is not a valid lefthand side. Eval FAILS";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return ERROR;
      }

    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();

    if(m_state.isReference(rtnUVPtr.getPtrTargetType()))
      rtnUVPtr = m_state.getPtrTarget(rtnUVPtr);

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeIdent::makeUlamValuePtr()
  {
    UlamValue ptr;

    if(m_varSymbol->isSelf())
      {
	//when "self/atom" is a quark, we're inside a func called on a quark (e.g. dm or local)
	//'atom' gets entire atom/element containing this quark; including its type!
	//'self' gets type/pos/len of the quark from which 'atom' can be extracted
	UlamValue selfuvp = m_state.m_currentSelfPtr;
	UTI ttype = selfuvp.getPtrTargetType();
	if((m_state.getUlamTypeByIndex(ttype)->getUlamClass() == UC_QUARK))
	  {
	    u32 vid = m_varSymbol->getId();
	    if(vid == m_state.m_pool.getIndexForDataString("atom"))
	      {
		selfuvp = m_state.getAtomPtrFromSelfPtr();
	      }
	    //else
	  }
	return selfuvp;
      } //done

    //can't use global m_currentAutoObjPtr, since there might be nested as conditional blocks.
    // NodeVarDecl for this autolocal sets AutoPtrForEval during its eval.
    if(m_varSymbol->getAutoLocalType() == ALT_AS)
      return ((SymbolVariableStack *) m_varSymbol)->getAutoPtrForEval(); //haha! we're done.

    ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(getNodeType())->getUlamClass();
    if(classtype == UC_ELEMENT)
      {
	  // ptr to explicit atom or element, (e.g.'f' in f.a=1) becomes new m_currentObjPtr
	  ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), UNPACKED, m_state, 0, m_varSymbol->getId());
      }
    else
      {
	if(m_varSymbol->isDataMember())
	  {
	    // return ptr to this data member within the m_currentObjPtr
	    // 'pos' modified by this data member symbol's packed bit position
	    ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());
	  }
	else
	  {
	    //local variable on the stack; could be array ptr!
	    ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	  }
      }
    return ptr;
  } //makeUlamValuePtr

  //new
  void NodeIdent::makeUlamValuePtrForCodeGen(UlamValue& uvpass)
  {
    assert(m_varSymbol);
    s32 tmpnum = m_state.getNextTmpVarNumber();

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClass();

    if(classtype == UC_ELEMENT || nut->getUlamTypeEnum() == UAtom)
      {
	// ptr to explicit atom or element, (e.g. 'f' in f.a=1;)
	uvpass = UlamValue::makePtr(tmpnum, TMPBITVAL, nuti, UNPACKED, m_state, 0, m_varSymbol->getId());
      }
    else
      {
	if(m_varSymbol->isDataMember())
	  {
	    u32 pos = 0;
	    if(uvpass.getUlamValueTypeIdx() == Ptr && uvpass.getPtrStorage() == TMPAUTOREF)
	      {
		//pos = uvpass.getPtrPos(); //runtime, not known now.
		UTI newnuti = m_state.getUlamTypeAsRef(nuti); //an ALT_REF
		uvpass = UlamValue::makePtr(tmpnum, TMPAUTOREF, newnuti, m_state.determinePackable(newnuti), m_state, pos + m_varSymbol->getPosOffset(), m_varSymbol->getId());
	      }
	    else
	      {
		if(!m_state.m_currentObjSymbolsForCodeGen.empty())
		  {
		    SymbolVariable * sym = (SymbolVariable *) m_state.m_currentObjSymbolsForCodeGen.back();
		    //here, we haven't taken into account any array indexes, So autoref instead
		    // e.g. m_bar[0].cb, and this NI is for the rhs of member select, 'cb'
		    //if(m_state.isScalar(sym->getUlamTypeIdx())) //???
		    pos = sym->getPosOffset();
		  }
		// 'pos' modified by this data member symbol's packed bit position
		uvpass = UlamValue::makePtr(tmpnum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, pos + m_varSymbol->getPosOffset(), m_varSymbol->getId());
	      }
	  }
	else
	  {
	    //local variable on the stack; could be array ptr!
	    uvpass = UlamValue::makePtr(tmpnum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, m_varSymbol->getId());
	  }
      }
  } //makeUlamValuePtrForCodeGen

  bool NodeIdent::installSymbolTypedef(TypeArgs& args, Symbol *& asymptr)
  {
    bool brtn = false;
    UTI tduti = Nav;
    UTI tdscalaruti = Nav;
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex,asymptr))
      {
	tduti = asymptr->getUlamTypeIdx();
	if(asymptr->isTypedef() && m_state.isHolder(tduti))
	  {
	    //not Nav when tduti's an array; might know?
	    args.m_declListOrTypedefScalarType = tdscalaruti;

	    ULAMCLASSTYPE tclasstype = m_state.getUlamTypeByIndex(tduti)->getUlamClass();
	    // keep the out-of-band name; other's might refer to its UTI.
	    // if its UTI is a unseen class, we can update the name of the class later
	    // don't want to rush this step since we might have a class w args and diff UTI.
	    if(tclasstype == UC_NOTACLASS)
	      {
		// if not a class, but a primitive type update the key
		if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
		  {
		    ULAMTYPE bUT = m_state.getBaseTypeFromToken(args.m_typeTok);
		    if(args.m_bitsize == 0)
		      args.m_bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];
		    // update the type of holder key
		    UlamKeyTypeSignature newkey(m_state.getTokenAsATypeNameId(args.m_typeTok), args.m_bitsize, args.m_arraysize, Nouti);
		    m_state.makeUlamTypeFromHolder(newkey, bUT, tduti); //update key, same uti
		  }
		else
		  {
		    //update holder key with name_id and possible array (UNKNOWNSIZE)
		    UlamKeyTypeSignature newkey(m_state.getTokenAsATypeNameId(args.m_typeTok), args.m_bitsize, args.m_arraysize, Nouti);
		    m_state.makeUlamTypeFromHolder(newkey, Holder, tduti); //update key, same uti
		  }
	      }
	    brtn = true;
	  }
	return brtn; //already there, and updated
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
	tduti = m_state.makeUlamType(args.m_typeTok, args.m_bitsize, NONARRAYSIZE, Nouti);
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
	if(m_state.isScalar(uti) && args.m_arraysize != NONARRAYSIZE)
	  {
	    args.m_declListOrTypedefScalarType = scalarUTI = uti;
	    // o.w. build symbol (with bit and array sizes);
	    // arrays can't have their scalar as classInstance;
	    // o.w., no longer findable by token.
	    UlamType * ut = m_state.getUlamTypeByIndex(uti);
	    ULAMTYPE bUT = ut->getUlamTypeEnum();
	    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();

	    if(args.m_bitsize == 0)
	      args.m_bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

	    UlamKeyTypeSignature newarraykey(key.getUlamKeyTypeSignatureNameId(), args.m_bitsize, args.m_arraysize);
	    newarraykey.append(scalarUTI); //removed if not a class

	    uti = m_state.makeUlamType(newarraykey, bUT);
	  }

	//create a symbol for this new ulam type, a typedef, with its type
	SymbolTypedef * symtypedef = new SymbolTypedef(m_token, uti, scalarUTI, m_state);
	m_state.addSymbolToCurrentScope(symtypedef);
	return (m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr)); //true
      }
    return false;
  } //installSymbolTypedef

  bool NodeIdent::installSymbolConstantValue(TypeArgs& args, Symbol*& asymptr)
  {
    // ask current scope block if this constant name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr))
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
	//create a symbol for this new named constant, a constant-def, with its value
	SymbolConstantValue * symconstdef = new SymbolConstantValue(m_token, uti, m_state);
	m_state.addSymbolToCurrentScope(symconstdef);

	//gets the symbol just created by makeUlamType; true.
	return (m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr));
      }
    return false;
  } //installSymbolConstantValue

  bool NodeIdent::installSymbolParameterValue(TypeArgs& args, Symbol*& asymptr)
  {
    // ask current scope block if this constant name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr))
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
	SymbolParameterValue * symparam = new SymbolParameterValue(m_token, uti, m_state);
	m_state.addSymbolToCurrentScope(symparam);

	//gets the symbol just created by makeUlamType; true.
	return (m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr));
      }
    return false;
  } //installSymbolParameterValue

  //see also NodeSquareBracket
  bool NodeIdent::installSymbolVariable(TypeArgs& args, Symbol *& asymptr)
  {
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr))
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

	    if(args.m_bitsize == 0)
	      args.m_bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

	    UlamKeyTypeSignature newarraykey(key.getUlamKeyTypeSignatureNameId(), args.m_bitsize, args.m_arraysize, scalarUTI, key.getUlamKeyTypeSignatureReferenceType()); //same reftype as key (or args?)
	    auti = m_state.makeUlamType(newarraykey, bUT);
	  }

	uti = m_state.getUlamTypeAsRef(auti, args.m_declRef); //ut not current

	SymbolVariable * sym = makeSymbol(uti, args.m_declRef);
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

  SymbolVariable *  NodeIdent::makeSymbol(UTI auti, ALT reftype)
  {
    //adjust decl count and max_depth, used for function definitions
    PACKFIT packit = m_state.determinePackable(auti);

    if(m_state.m_currentFunctionBlockDeclSize == 0)
      {
	u32 baseslot = 1;  //unpacked fixed later

	//variable-index, ulamtype, ulamvalue(ownership to symbol); always packed
	if(reftype != ALT_NOT)
	  return NULL; //error! dm's not references
	return (new SymbolVariableDataMember(m_token, auti, packit, baseslot, m_state));
      }

    //Symbol is a parameter; always on the stack
    if(m_state.m_currentFunctionBlockDeclSize < 0)
      {
	//1 slot for scalar or packed array
	m_state.m_currentFunctionBlockDeclSize -= m_state.slotsNeeded(auti);

	SymbolVariableStack * rtnSym = (new SymbolVariableStack(m_token, auti, packit, m_state.m_currentFunctionBlockDeclSize, m_state)); //slot after adjust
	assert(rtnSym);

	rtnSym->setAutoLocalType(reftype);

	return rtnSym;
      }

    //(else) Symbol is a local variable, always on the stack
    SymbolVariableStack * rtnLocalSym = new SymbolVariableStack(m_token, auti, packit, m_state.m_currentFunctionBlockDeclSize, m_state); //slot before adjustment
    assert(rtnLocalSym);
    rtnLocalSym->setAutoLocalType(reftype);

    m_state.m_currentFunctionBlockDeclSize += m_state.slotsNeeded(auti);

    //adjust max depth, excluding parameters and initial start value (=1)
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
	msg <<  m_state.getTokenDataAsString(&args.m_typeTok).c_str();
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
	    msg <<  m_state.getTokenDataAsString(&args.m_typeTok).c_str();
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
	msg <<  m_state.getTokenDataAsString(&args.m_typeTok).c_str();
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
	msg <<  m_state.getTokenDataAsString(&args.m_typeTok).c_str();
	msg  << "', and cannot be used by a named constant '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    assert(args.m_arraysize == NONARRAYSIZE);

    args.m_bitsize = tdut->getBitSize(); //ok to use typedef bitsize

    // constants can't be classes either
    if(tdut->getUlamClass() != UC_NOTACLASS)
      {
	std::ostringstream msg;
	msg << "Named Constant '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' cannot be based on a class type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(tduti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    return rtnb;
  } //checkConstantTypedefSizes

  void NodeIdent::genCode(File * fp, UlamValue & uvpass)
  {
    UlamValue savuvpass = uvpass;

    //return the ptr for an array; square bracket will resolve down to the immediate data
    makeUlamValuePtrForCodeGen(uvpass);

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol); //*********UPDATED GLOBAL;

    //before we lose savuvpass, generate next chain of related reference
    if((savuvpass.getUlamValueTypeIdx() == Ptr) && (savuvpass.getPtrStorage() == TMPAUTOREF))
      {
	Node::genCodeARefFromARefStorage(fp, savuvpass, uvpass);

	//now read from it
	s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data
	UTI vuti = uvpass.getPtrTargetType();
	UlamType * vut = m_state.getUlamTypeByIndex(vuti);

	m_state.indent(fp);
	fp->write("const ");
	fp->write(vut->getTmpStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2).c_str());
	fp->write(" = ");
	fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex(), uvpass.getPtrStorage()).c_str());
	fp->write(".read();\n");
	// uvpass updated again
	uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, vuti, m_state.determinePackable(vuti), m_state, 0); //POS 0 justified (atom-based).
      }
    else
      // UNCLEAR: should this be consistent with constants?
      genCodeReadIntoATmpVar(fp, uvpass);
  } //genCode

  void NodeIdent::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    UlamValue savuvpass = uvpass;

    //e.g. return the ptr for an array;
    //square bracket will resolve down to the immediate data
   makeUlamValuePtrForCodeGen(uvpass);

    //******UPDATED GLOBAL; no restore!!!**************************
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol);

    if((savuvpass.getUlamValueTypeIdx() == Ptr) && (savuvpass.getPtrStorage() == TMPAUTOREF))
      Node::genCodeARefFromARefStorage(fp, savuvpass, uvpass);
    else if(uvpass.getPtrStorage() == TMPAUTOREF)
      Node::genCodeConvertATmpVarIntoAutoRef(fp, uvpass); //uvpass becomes the autoref, and clears stack
  } //genCodeToStoreInto

  // overrides NodeTerminal that reads into a tmp var BitVector
  void NodeIdent::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    if(uvpass.getPtrStorage() == TMPAUTOREF)
      Node::genCodeConvertATmpVarIntoAutoRef(fp, uvpass); //uvpass becomes the autoref, and clears stack

    Node::genCodeReadIntoATmpVar(fp, uvpass);
  }

} //end MFM
