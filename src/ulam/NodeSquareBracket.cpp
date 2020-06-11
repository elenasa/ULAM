#include "NodeSquareBracket.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "NodeIdent.h"
#include "CompilerState.h"

namespace MFM {

  NodeSquareBracket::NodeSquareBracket(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state), m_isCustomArray(false), m_tmpvarSymbol(NULL)
  {
    if(m_nodeRight)
      m_nodeRight->updateLineage(getNodeNo()); //for unknown subtrees
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeSquareBracket::NodeSquareBracket(const NodeSquareBracket& ref) : NodeBinaryOp(ref), m_isCustomArray(ref.m_isCustomArray), m_tmpvarSymbol(NULL) {}

  NodeSquareBracket::~NodeSquareBracket()
  {
    delete m_tmpvarSymbol;
    m_tmpvarSymbol = NULL;
  }

  Node * NodeSquareBracket::instantiate()
  {
    return new NodeSquareBracket(*this);
  }

  void NodeSquareBracket::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_nodeLeft->updateLineage(getNodeNo());
    if(m_nodeRight)
      m_nodeRight->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeSquareBracket::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeLeft && m_nodeLeft->findNodeNo(n, foundNode))
      return true;
    if(m_nodeRight && m_nodeRight->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeSquareBracket::printOp(File * fp)
  {
    NodeBinaryOp::printOp(fp);
  }

  const char * NodeSquareBracket::getName()
  {
    return "[]";
  }

  const std::string NodeSquareBracket::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeSquareBracket::methodNameForCodeGen()
  {
    return "_SquareBracket_Stub";
  }

  bool NodeSquareBracket::isArrayItem()
  {
    return true; //not for array declaration; includes custom array items
  }

  // used to select an array item; not for declaration
  UTI NodeSquareBracket::checkAndLabelType()
  {
    assert(m_nodeLeft);
    u32 errorCount = 0;
    u32 hazyCount = 0;
    UTI newType = Nav; //init
    UTI idxuti = Nav;

    UTI leftType = m_nodeLeft->checkAndLabelType();

    //Not caught during parsing since array size may be blank if declared with initialization
    if(!m_nodeRight)
      {
	std::ostringstream msg;
	msg << "Array item specifier is missing for ";
	msg << m_nodeLeft->getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    //for example, f.chance[i] where i is local, same as f.func(i);
    NodeBlock * currBlock = m_state.getCurrentBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock); //currblock doesn't change
    UTI rightType = m_nodeRight->checkAndLabelType();

    m_state.popClassContext();

    if(m_state.isComplete(leftType))
      {
	UlamType * lut = m_state.getUlamTypeByIndex(leftType);

	if(lut->isScalar())
	  {
	    m_isCustomArray = m_state.isClassACustomArray(leftType); //e.g. t3653

	    if(lut->isHolder())
	      {
		std::ostringstream msg;
		msg << "Incomplete Type: " << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
		msg << " used with " << getName();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		hazyCount++;
	      }
	    else if(lut->getUlamTypeEnum() == String)
	      {
		//ok!
	      }
	    else
	      {
		assert(lut->getUlamTypeEnum() == Class);
		//overload operator[] supercedes custom array (t41129)
		if(NodeBinaryOp::buildandreplaceOperatorOverloadFuncCallNode())
		  {
		    m_state.setGoAgain();
		    delete this; //suicide is painless..
		    return Hzy;
		  }
		else if(!m_isCustomArray)
		  {
		    std::ostringstream msg;
		    msg << "Invalid Type: " << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
		    msg << " used with " << getName();
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    errorCount++;
		  }
		else
		  {
		    // either an array of custom array classes, or a custom array;
		    // Note: A diff approach, substitute a func call node for sq bkt, not used.
		    UTI caType = m_state.getAClassCustomArrayType(leftType);
		    if(!m_state.isComplete(caType))
		      {
			std::ostringstream msg;
			msg << "Incomplete Custom Array Type: ";
			msg << m_state.getUlamTypeNameBriefByIndex(caType).c_str();
			msg << " used with class: ";
			msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
			msg << getName();
			if(caType == Nav)
			  {
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			    newType = Nav; //error!
			    errorCount++;
			  }
			else
			  {
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
			    newType = Hzy;
			    hazyCount++;
			  }
		      }
		  }
	      }
	  }
	else
	  {
	    // arraysize is zero! not accessible. runtime check
	    // unless the index is a "ready" constant
	    if(m_nodeRight->isAConstant())
	      {
		s32 rindex;
		UTI rt;
		if(!getArraysizeInBracket(rindex,rt))
		  {
		    std::ostringstream msg;
		    msg << "Erroneous constant array item specifier for ";
		    msg << m_nodeLeft->getName() << " type: ";
		    msg << m_state.getUlamTypeNameByIndex(leftType).c_str();
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		    return Nav;
		  }
		else if(!m_state.okUTItoContinue(rt) || !m_state.isComplete(rt) || !m_state.isComplete(leftType))
		  {
		    std::ostringstream msg;
		    msg << "Constant array item specifier may be out-of-bounds for ";
		    msg << m_nodeLeft->getName();
		    msg << m_state.getUlamTypeNameByIndex(leftType).c_str();
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		    setNodeType(Hzy);
		    m_state.setGoAgain();
		    return Hzy;
		  }
		else
		  {
		    s32 arraysize = m_state.getArraySize(leftType);
		    assert(arraysize >= 0);
		    if(rindex > arraysize)
		      {
			std::ostringstream msg;
			msg << "Constant array item specifier (" << rindex;
			msg << ") for " << m_nodeLeft->getName();
			msg << " is OUT-OF-BOUNDS; type: ";
			msg << m_state.getUlamTypeNameByIndex(leftType).c_str();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			setNodeType(Nav);
			return Nav;
		      }
		  }
	      } //right index is a constant
	  }

	//set up idxuti..RHS
	//cant proceed with custom array subscript if lhs is incomplete
	if((errorCount == 0) && (hazyCount == 0))
	  {
	    if(m_isCustomArray)
	      {
		bool hasHazyArgs = false;
		u32 camatches = m_state.getAClassCustomArrayIndexType(leftType, m_nodeRight, idxuti, hasHazyArgs);
		if(camatches == 0)
		  {
		    std::ostringstream msg;
		    msg << "No defined custom array get function with";
		    msg << " matching argument type ";
		    msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
		    msg << "; and cannot be called";
		    if(hasHazyArgs)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
			idxuti = Hzy;
			hazyCount++;
		      }
		    else
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			idxuti = Nav; //error!
			errorCount++;
		      }
		  }
		else if(camatches > 1)
		  {
		    std::ostringstream msg;
		    msg << "Ambiguous matches (" << camatches;
		    msg << ") of custom array get function for argument type ";
		    msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
		    msg << "; Explicit casting required";
		    if(hasHazyArgs)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
			idxuti = Hzy;
			hazyCount++;
		      }
		    else
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			idxuti = Nav; //error!
			errorCount++;
		      }
		  }
		else //a match! (surgery to func call node? what if, lhs of assignment?)
		  {
		    //replace node with func call to 'aref' (t41000, t41001)
		    Node * newnode = buildArefFuncCallNode();
		    AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
		    assert(swapOk);

		    m_nodeRight = NULL; //recycled
		    m_nodeLeft = NULL; //recycled

		    m_state.setGoAgain();

		    delete this; //suicide is painless..

		    return Hzy;
		  }
	      }
	    else
	      {
		//not custom array
		//must be some kind of numeric type: Int, Unsigned, or Unary..of any bit size
		//or a String
		UlamType * rut = m_state.getUlamTypeByIndex(rightType);
		ULAMTYPE retyp = rut->getUlamTypeEnum();
		if(m_state.okUTItoContinue(rightType) && !rut->isNumericType())
		  {
		    std::ostringstream msg;
		    msg << "Array item specifier requires numeric type: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    idxuti = Nav; //error!
		    errorCount++;
		  }
		else if(retyp == Class)
		  idxuti = Int;
		else if(retyp == Unary)
		  idxuti = Unsigned; //t3877, t3543, t3591, t3594, t3702
		else
		  idxuti = rightType; //default unless caarray
	      }
	  } //errorcount is zero

	if(m_state.okUTItoContinue(idxuti) && (UlamType::compare(idxuti, rightType, m_state) == UTIC_NOTSAME))
	  {
	    FORECAST rscr = m_nodeRight->safeToCastTo(idxuti);
	    if(rscr == CAST_CLEAR)
	      {
		if(!Node::makeCastingNode(m_nodeRight, idxuti, m_nodeRight))
		  errorCount++;
	      }
	    else if(rscr == CAST_HAZY)
	      hazyCount++;
	    else
	      errorCount++;
	  }
      } //lt not nav, might be Hzy
    else
      {
	if(leftType == Nav)
	  errorCount++;
	else if(leftType == Hzy)
	  hazyCount++;
	else
	  hazyCount++;
      }

    if((errorCount == 0) && (hazyCount == 0))
      {
	UlamType * lut = m_state.getUlamTypeByIndex(leftType);
	bool isScalar = lut->isScalar();
	// sq bracket purpose in life is to account for array elements;
	if((lut->getUlamTypeEnum() == String) && isScalar)
	  newType = ASCII; //not storeintoable default (t41076)
	else
	  {
	    newType = m_state.getUlamTypeAsScalar(leftType);
	    // multi-dimensional possible; MP not ok lhs.
	    Node::setStoreIntoAble(m_nodeLeft->getStoreIntoAble());
	  }
      }
    else
      {
	if(errorCount != 0)
	  newType = Nav;
	else if(hazyCount != 0)
	  newType = Hzy;
	else
	  m_state.abortShouldntGetHere();
      }
    setNodeType(newType);
    if(newType == Hzy) m_state.setGoAgain(); //covers non-error(debug) messages for incompletes
    return newType;
  } //checkAndLabelType

  bool NodeSquareBracket::getConstantArrayItemValue(BV8K& bvitem)
  {
    bool rtnok = false;
    assert(m_nodeLeft->isAConstant() && m_nodeRight->isAConstant());
    UTI leftType = m_nodeLeft->getNodeType();
    s32 rindex;
    UTI rt;
    if(getArraysizeInBracket(rindex,rt)) //t41198
      {
	assert((rindex >= 0) && (rindex < m_state.getArraySize(leftType))); //catchable during c&l
	//fold into a constant class (t41273); not a list
	BV8K bvccatmp;
	if(m_nodeLeft->getConstantValue(bvccatmp))
	  {
	    UTI scalarLeft = m_state.getUlamTypeAsScalar(leftType);
	    u32 itemlen = m_state.getUlamTypeByIndex(scalarLeft)->getSizeofUlamType();
	    bvccatmp.CopyBV(rindex * itemlen, 0u, itemlen, bvitem); //src pos, dest pos, dst bv
	    rtnok = true;
	  }
      }
    return rtnok;
  } //getConstantArrayItemValue

  bool NodeSquareBracket::getConstantValue(BV8K& bval)
  {
    return getConstantArrayItemValue(bval);
  }

  //here, we check for existence, do we can default to custom array, aref.
  Node * NodeSquareBracket::buildOperatorOverloadFuncCallNode()
  {
    UTI leftType = m_nodeLeft->getNodeType();
    TokenType opTokType = Token::getTokenTypeFromString(getName());
    assert(opTokType != TOK_LAST_ONE);
    Token opTok(opTokType, getNodeLocation(), 0);
    u32 opolId = Token::getOperatorOverloadFullNameId(opTok, &m_state);
    if(opolId == 0)
      {
	std::ostringstream msg;
	msg << "Overload for operator " << getName();
	msg << " is not supported as operand for class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return NULL;
      }

    //may need to fall back to a custom array
    Symbol * fnsymptr = NULL;
    bool hazyKin = false;
    if(m_state.isFuncIdInAClassScopeOrAncestor(leftType, opolId, fnsymptr, hazyKin) && !hazyKin)
      {
	// ambiguous (>1) overload will produce an error later
	//fill in func symbol during type labeling;
	return Node::buildOperatorOverloadFuncCallNodeHelper(m_nodeLeft, m_nodeRight, getName());
      }//else use default struct equal, or wait for hazy arg

    //redo check and type labeling done by caller!!
    return NULL; //replace right node with new branch
  } //buildOperatorOverloadFuncCallNode

  Node * NodeSquareBracket::buildArefFuncCallNode()
  {
    Token identTok;
    u32 arefId = m_state.getCustomArrayGetFunctionNameId();
    identTok.init(TOK_IDENTIFIER, getNodeLocation(), arefId);

    //fill in func symbol during type labeling;
    Node * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
    assert(fcallNode);
    fcallNode->setNodeLocation(identTok.m_locator);
    Node * mselectNode = new NodeMemberSelect(m_nodeLeft, fcallNode, m_state);
    assert(mselectNode);
    mselectNode->setNodeLocation(identTok.m_locator);

    ((NodeFunctionCall *) fcallNode)->addArgument(m_nodeRight);
    //redo check and type labeling done by caller!!
    return mselectNode; //replace self
  } //buildArefFuncCallNode


  bool NodeSquareBracket::trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    if(nut->getUlamClassType() == UC_ELEMENT)
      {
	if(fromleftnode)
	  *fromleftnode = NULL; //clear for future deletion
	rtnnodeptr = this;
	return true;
      }
    return false;
  } //trimToTheElement

  UTI NodeSquareBracket::calcNodeType(UTI lt, UTI rt)
  {
    m_state.abortShouldntGetHere();
    return Int;
  }

  EvalStatus NodeSquareBracket::eval()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    UTI leftType = m_nodeLeft->getNodeType();
    UlamType * lut = m_state.getUlamTypeByIndex(leftType);
    if((lut->getUlamTypeEnum() == String) && (nuti == ASCII))
      {
	return evalAUserStringByte();
      }

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue pluv = m_state.m_nodeEvalStack.popArg();
    UTI ltype = pluv.getPtrTargetType();

    makeRoomForNodeType(m_nodeRight->getNodeType()); //offset a constant expression
    evs = m_nodeRight->eval();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue offset = m_state.m_nodeEvalStack.popArg();
    UlamType * offut = m_state.getUlamTypeByIndex(offset.getUlamValueTypeIdx());
    s32 offsetInt = 0;
    if(offut->isNumericType())
      {
	// constant expression only required for array declaration //t3321,t3503,t3895
	s32 arraysize = m_state.getArraySize(ltype);
	u64 offsetdata = 0;
	if(offut->getBitSize() <= MAXBITSPERINT)
	  {
	    offsetdata = offset.getImmediateData(m_state);
	    offsetInt = offut->getDataAsCs32(offsetdata);
	  }
	else if(offut->getBitSize() <= MAXBITSPERLONG)
	  {
	    offsetdata = offset.getImmediateDataLong(m_state); //t3895
	    offsetInt = (s32) offut->getDataAsCs64(offsetdata);
	  }
	else
	  m_state.abortGreaterThanMaxBitsPerLong();

	if((offsetInt >= arraysize))
	  {
	    Symbol * lsymptr;
	    u32 lid = 0;
	    if(getSymbolPtr(lsymptr))
	      lid = lsymptr->getId();

	    std::ostringstream msg;
	    msg << "Array subscript [" << offsetInt << "] exceeds the size (" << arraysize;
	    msg << ") of array '" << m_state.m_pool.getDataAsString(lid).c_str() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return evalStatusReturn(ERROR);
	  }
      }
    else
      {
	Symbol * lsymptr;
	u32 lid = 0;
	if(getSymbolPtr(lsymptr))
	  lid = lsymptr->getId();

	std::ostringstream msg;
	msg << "Array subscript of array '";
	msg << m_state.m_pool.getDataAsString(lid).c_str();
	msg << "' requires a numeric type";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return evalStatusReturn(ERROR);
      }

    Node::assignReturnValueToStack(pluv.getValAt(offsetInt, m_state));

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeSquareBracket::evalAUserStringByte()
  {
    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for index into user string pool
    EvalStatus evs = m_nodeLeft->eval();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue luv = m_state.m_nodeEvalStack.popArg();
    u32 usrStr = 0;
    usrStr = luv.getImmediateData(m_state);

    if(!m_state.isValidUserStringIndex(usrStr))
      return evalStatusReturn(ERROR); //uninitialized or out-of-bounds

    makeRoomForNodeType(m_nodeRight->getNodeType()); //offset a constant expression
    evs = m_nodeRight->eval();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue offset = m_state.m_nodeEvalStack.popArg();
    UlamType * offut = m_state.getUlamTypeByIndex(offset.getUlamValueTypeIdx());
    u32 offsetdata = 0;
    if(offut->isNumericType())
      {
	if(!m_state.isValidUserStringIndex(usrStr))
	  {
	    std::ostringstream msg;
	    msg << "Uninitialized String cannot access subscript of '";
	    msg << m_state.getDataAsFormattedUserString(usrStr).c_str() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return evalStatusReturn(ERROR);
	  }

	// constant expression only required for array declaration
	u32 strlen = m_state.getUserStringLength(usrStr);
	offsetdata = offset.getImmediateData(m_state);

	if((offsetdata >= strlen))
	  {
	    std::ostringstream msg;
	    msg << "String subscript [" << offsetdata << "] exceeds the length (" << strlen;
	    msg << ") of '" << m_state.getDataAsFormattedUserString(usrStr).c_str() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return evalStatusReturn(ERROR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "String subscript of '";
	msg << m_state.getDataAsFormattedUserString(usrStr).c_str();
	msg << "' requires a numeric type";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return evalStatusReturn(ERROR);
      }

    Node::assignReturnValueToStack(m_state.getByteOfUserStringForEval(usrStr, offsetdata));

    evalNodeEpilog();
    return NORMAL;
  } //evalAUserStringByte

  EvalStatus NodeSquareBracket::evalToStoreInto()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue pluv = m_state.m_nodeEvalStack.popArg();
    UTI auti = pluv.getPtrTargetType();

    makeRoomForNodeType(m_nodeRight->getNodeType()); //offset a constant expression
    evs = m_nodeRight->eval();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue offset = m_state.m_nodeEvalStack.popArg();
    // constant expression only required for array declaration
    u32 offsetdata = offset.getImmediateData(m_state);
    s32 offsetInt = m_state.getUlamTypeByIndex(offset.getUlamValueTypeIdx())->getDataAsCs32(offsetdata);
    //adjust pos by offset * len, according to its scalar type
    UlamValue scalarPtr = UlamValue::makeScalarPtr(pluv, m_state);

    if(scalarPtr.incrementPtr(m_state, offsetInt))
      //copy result UV to stack, -1 relative to current frame pointer
      Node::assignReturnValuePtrToStack(scalarPtr);
    else
      {
	s32 arraysize = m_state.getArraySize(auti);
	Symbol * lsymptr;
	u32 lid = 0;
	if(getSymbolPtr(lsymptr))
	  lid = lsymptr->getId();

	std::ostringstream msg;
	msg << "Array subscript [" << offsetInt << "] exceeds the size (" << arraysize;
	msg << ") of array '" << m_state.m_pool.getDataAsString(lid).c_str() << "'";
	msg << " to store into";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return evalStatusReturn(ERROR);
      }

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  bool NodeSquareBracket::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    return false;
  }

  UlamValue NodeSquareBracket::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    m_state.abortShouldntGetHere(); //unused
    return UlamValue();
  }

  UlamValue NodeSquareBracket::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    m_state.abortShouldntGetHere(); //unused
    return UlamValue();
  }

  bool NodeSquareBracket::getSymbolPtr(Symbol *& symptrref)
  {
    if(m_nodeLeft)
      return m_nodeLeft->getSymbolPtr(symptrref);

    MSG(getNodeLocationAsString().c_str(), "No symbol", ERR);
    return false;
  }

  bool NodeSquareBracket::getStorageSymbolPtr(Symbol *& symptrref)
  {
    assert(m_nodeLeft);
    return m_nodeLeft->getStorageSymbolPtr(symptrref);
  }

  //see also NodeIdent
  bool NodeSquareBracket::installSymbolTypedef(TypeArgs& args, Symbol *& asymptr)
  {
    assert(m_nodeLeft && m_nodeRight);

    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build typedef symbol", ERR);
	return false;
      }

    if(args.m_arraysize > NONARRAYSIZE)
      {
	MSG(getNodeLocationAsString().c_str(), "Array size specified twice for typedef", ERR);
	return false;
      }

    args.m_arraysize = UNKNOWNSIZE; // no eval yet
    return m_nodeLeft->installSymbolTypedef(args, asymptr);
  } //installSymbolTypedef

  //see also NodeIdent (t3890)
  bool NodeSquareBracket::installSymbolConstantValue(TypeArgs& args, Symbol *& asymptr)
  {
    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build array symbol", ERR);
	return false;
      }

    if(args.m_arraysize > NONARRAYSIZE)
      {
	MSG(getNodeLocationAsString().c_str(), "Array size specified twice", ERR);
	return false;
      }

    args.m_arraysize = UNKNOWNSIZE; // no eval yet
    assert(m_nodeLeft);
    return m_nodeLeft->installSymbolConstantValue(args, asymptr);
  } //installSymbolConstantValue

  //see also NodeIdent
  bool NodeSquareBracket::installSymbolModelParameterValue(TypeArgs& args, Symbol *& asymptr)
  {
    MSG(getNodeLocationAsString().c_str(), "Array type specified for model parameter", ERR);
    return false;
  } //installSymbolModelParameterValue

  //see also NodeIdent
  bool NodeSquareBracket::installSymbolVariable(TypeArgs& args,  Symbol *& asymptr)
  {
    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build array symbol", ERR);
	return false;
      }

    if(args.m_arraysize > NONARRAYSIZE)
      {
	MSG(getNodeLocationAsString().c_str(), "Array size specified twice", ERR);
	return false;
      }

    args.m_arraysize = UNKNOWNSIZE; // no eval yet
    assert(m_nodeLeft);
    return m_nodeLeft->installSymbolVariable(args, asymptr);
  } //installSymbolVariable

  bool NodeSquareBracket::assignClassArgValueInStubCopy()
  {
    return true;
  }

  // eval() no longer performed before check and label
  // returns false if error; UNKNOWNSIZE is not an error!
  bool NodeSquareBracket::getArraysizeInBracket(s32 & rtnArraySize, UTI& sizetype)
  {
    bool noerr = true;
    // since square brackets determine the constant size for this type, else error
    s32 newarraysize = NONARRAYSIZE;
    if(m_nodeRight == NULL)
      {
	rtnArraySize = UNKNOWNSIZE; //determined by number of initialization items
	return true;
      }

    sizetype = m_nodeRight->checkAndLabelType(); //t3504
    if((sizetype == Nav))
      {
	rtnArraySize = UNKNOWNSIZE;
	return false;
      }

    if((sizetype == Hzy))
      {
	rtnArraySize = UNKNOWNSIZE;
	return true;
      }

    UlamType * sizeut = m_state.getUlamTypeByIndex(sizetype);

    // expects a constant, numeric type within []
    if(sizeut->isNumericType() && m_nodeRight->isAConstant())
      {
	evalNodeProlog(0); //new current frame pointer
	makeRoomForNodeType(sizetype); //offset a constant expression
	EvalStatus evs = m_nodeRight->eval();
	if(evs == NORMAL)
	  {
	    UlamValue arrayUV = m_state.m_nodeEvalStack.popArg();
	    u32 arraysizedata = 0;
	    u32 wordsize = sizeut->getTotalWordSize();
	    if(wordsize <= MAXBITSPERINT)
	      arraysizedata = arrayUV.getImmediateData(m_state);
	    else if(wordsize <= MAXBITSPERLONG)
	      arraysizedata = (u32) arrayUV.getImmediateDataLong(m_state);
	    else
	      m_state.abortGreaterThanMaxBitsPerLong();

	    newarraysize = sizeut->getDataAsCs32(arraysizedata);
	    if(newarraysize < 0 && newarraysize != UNKNOWNSIZE) //NONARRAY or UNKNOWN
	      {
		MSG(getNodeLocationAsString().c_str(),
		    "Array size specifier in [] is not a positive number", ERR);
		sizetype = Nav;
		noerr = false;
	      }
	    //else unknown is not an error
	  }
	else if(evs == NOTREADY)
	  {
	    noerr = false;
	    sizetype = Hzy;
	  }
	else //newarraysize = UNKNOWNSIZE; //still true
	  {
	    noerr = false;
	    sizetype = Nav;
	  }
	evalNodeEpilog();
      }
    else
      {
	MSG(getNodeLocationAsString().c_str(),
	    "Array size specifier in [] is not a constant number", ERR);
	sizetype = Nav;
	noerr = false;
      }
    rtnArraySize = newarraysize;
    return noerr;
  } //getArraysizeInBracket

  void NodeSquareBracket::genCode(File * fp, UVPass& uvpass)
  {
    UTI leftType = m_nodeLeft->getNodeType();
    UlamType * lut = m_state.getUlamTypeByIndex(leftType);
    bool isString = (lut->getUlamTypeEnum() == String); //t3973, t3953
    if(isString && lut->isScalar())
      {
	return genCodeAUserStringByte(fp, uvpass);
      }
    else if(Node::isCurrentObjectsContainingAConstantClass() >= 0)
      {
	genCodeToStoreInto(fp, uvpass); //t41198
	m_state.clearCurrentObjSymbolsForCodeGen();
	return;
      }
    //else continue..

    genCodeToStoreInto(fp, uvpass);

    if(!(isString || m_nodeLeft->isAConstant()) || m_state.isReference(uvpass.getPassTargetType())) //t3953,t3973, not isAltRefType t3908, nor constant class (t41266)
      Node::genCodeReadIntoATmpVar(fp, uvpass);
    else
      m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCode

  void NodeSquareBracket::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    //wipe out before getting item within sq brackets
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    m_state.clearCurrentObjSymbolsForCodeGen();

    UVPass offset;
    m_nodeRight->genCode(fp, offset);
    offset.setPassStorage(TMPARRAYIDX);

    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector;  //restore

    UVPass luvpass = uvpass; //t3615 passes along if rhs of memberselect
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    //runtime check to avoid accessing beyond array (non-custom array only)
    UTI luti = luvpass.getPassTargetType();
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    s32 arraysize = lut->getArraySize();
    assert(!lut->isScalar());

    if(!m_nodeRight->isAConstant()) //Wed Jul 11 18:02:46 2018
      {
	m_state.indentUlamCode(fp);
	fp->write("if(");
	fp->write(offset.getTmpVarAsString(m_state).c_str());
	fp->write(" >= ");
	fp->write_decimal(arraysize);
	fp->write(")"); GCNL;

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);"); GCNL;
	m_state.m_currentIndentLevel--;
      }

    //save autoref into a tmpvar symbol
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * cossym = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cossuti = cossym->getUlamTypeIdx();
    UlamType * cossut = m_state.getUlamTypeByIndex(cossuti);
    assert(!cossut->isScalar());

    if(Node::isCurrentObjectsContainingAConstantClass() >= 0)
      {
	Node::genCodeReadArrayItemFromAConstantClassIntoATmpVar(fp, luvpass, offset);
	uvpass = luvpass;
	if(m_state.isAClass(cossuti))
	  Node::genCodeConvertATmpVarIntoBitVector(fp, uvpass); //not for t41198, for t41263
	else if(cossut->getUlamTypeEnum() == String) //t41274, t41267, t41273
	  uvpass.setPassTargetType(m_state.getUlamTypeAsDeref(luvpass.getPassTargetType()));
	m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL); //dm to avoid leaks
	m_tmpvarSymbol->setDivinedByConstantClass();
	m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
	return; //t41261
      }

    //luvpass becomes the (BV) autoref, and clears stack
    if(cossym->isConstant())
      {
	//a constant array of strings is treated like a NodeTerminal DBLQUOTED token.
	// in order to "fix" the registration number of the index, at use, rather
	// than at compiler/constructor time when the registration number is not yet
	// available. (t3953, t41277,8)
	Node::genCodeConvertATmpVarIntoConstantAutoRef(fp, luvpass, offset);
      }
    else
      Node::genCodeConvertATmpVarIntoAutoRef(fp, luvpass, offset);

    uvpass = luvpass;
    m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, cossym); //dm to avoid leaks
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
    // NO RESTORE -- up to caller for lhs.
  } //genCodeToStoreInto

  void NodeSquareBracket::genCodeAUserStringByte(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    //wipe out before getting item within sq brackets
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    m_state.clearCurrentObjSymbolsForCodeGen();

    UVPass offset;
    m_nodeRight->genCode(fp, offset);
    offset.setPassStorage(TMPARRAYIDX);

    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector;  //restore

    UVPass luvpass = uvpass; //passes along if rhs of memberselect
    m_nodeLeft->genCode(fp, luvpass);

    //runtime check to avoid accessing beyond array (t3932)
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const u32 ");
    fp->write(m_state.getTmpVarAsString(Unsigned, tmpVarNum2, TMPREGISTER).c_str());
    fp->write(" = ");
    fp->write(m_state.getGetStringLengthFunctionName());
    fp->write("(");
    fp->write(luvpass.getTmpVarAsString(m_state).c_str());
    fp->write(");"); GCNL;

    m_state.indentUlamCode(fp);
    fp->write("if(");
    fp->write(offset.getTmpVarAsString(m_state).c_str());
    fp->write(" >= ");
    fp->write(m_state.getTmpVarAsString(Unsigned, tmpVarNum2, TMPREGISTER).c_str());
    fp->write(")"); GCNL;

    m_state.m_currentIndentLevel++;
    m_state.indentUlamCode(fp);
    fp->write("FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);"); GCNL;
    m_state.m_currentIndentLevel--;

    //get the ascii byte in a tmp var
    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const unsigned char ");
    fp->write(m_state.getTmpVarAsString(ASCII, tmpVarNum, TMPREGISTER).c_str());
    fp->write(" = *(");
    fp->write(m_state.getGetStringFunctionName());
    fp->write("(");
    fp->write(luvpass.getTmpVarAsString(m_state).c_str());
    fp->write(") + ");
    fp->write(offset.getTmpVarAsString(m_state).c_str()); //INDEX of byte
    fp->write(");"); GCNL; //t3945

    uvpass = UVPass::makePass(tmpVarNum, TMPREGISTER, ASCII, m_state.determinePackable(ASCII), m_state, 0, 0); //POS 0 rightjustified (atom-based).

    m_state.clearCurrentObjSymbolsForCodeGen();
    // NO RESTORE -- up to caller for lhs.
  } //genCodeAUserStringByte

} //end MFM
