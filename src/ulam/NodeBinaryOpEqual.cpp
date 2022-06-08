#include "NodeBinaryOpEqual.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "SymbolFunctionName.h"
#include "SymbolFunction.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqual::NodeBinaryOpEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state)
  {
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeBinaryOpEqual::NodeBinaryOpEqual(const NodeBinaryOpEqual& ref) : NodeBinaryOp(ref) {}

  NodeBinaryOpEqual::~NodeBinaryOpEqual(){}

  Node * NodeBinaryOpEqual::instantiate()
  {
    return new NodeBinaryOpEqual(*this);
  }

  const char * NodeBinaryOpEqual::getName()
  {
    return "=";
  }

  const std::string NodeBinaryOpEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpEqual::methodNameForCodeGen()
  {
    return "_Equal_Stub";
  }

  bool NodeBinaryOpEqual::checkSafeToCastTo(UTI unused, UTI& newType)
  {
    bool rtnOK = true;
    //bypass same bitsize check when a primitive ref for assignment: t3625,32,51,t3817,t41001
    bool isAClassOrAtom = m_state.isAClass(newType) || m_state.isAtom(newType);
    UTI chknewtype = isAClassOrAtom ? newType : m_state.getUlamTypeAsDeref(newType);
    FORECAST scr = m_nodeRight->safeToCastTo(chknewtype);
    if(scr != CAST_CLEAR)
      {
	ULAMTYPE etyp = m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum();
	UTI ruti =  m_nodeRight->getNodeType();
	std::ostringstream msg;
	if(etyp == Bool)
	  msg << "Use a comparison operation";
	else if(etyp == UAtom)
	  msg << "Use .atomof"; //t41540
	else if(etyp == String)
	  msg << "Invalid";
	else if(!m_state.isScalar(newType) || !m_state.isScalar(ruti))
	  msg << "Not possible";
	else
	  msg << "Use explicit cast";
	msg << " to convert "; // the real converting-message
	if(m_state.isAClass(ruti))
	  msg << m_state.getUlamTypeNameBriefByIndex(ruti).c_str();
	else
	  msg << m_state.getUlamTypeNameByIndex(ruti).c_str();
	msg << " to ";
	if(m_state.isAClass(newType))
	  msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
	else
	  msg << m_state.getUlamTypeNameByIndex(newType).c_str();
	msg << " for " << getName();
	if(scr == CAST_HAZY)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    newType = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
	rtnOK = false;
      } //not safe
    else if(m_nodeRight->isExplicitReferenceCast())
      {
	std::ostringstream msg;
	msg << "Explicit Reference cast of assignment is invalid";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
	rtnOK = false;
      }
    return rtnOK;
  } //checkSafeToCastTo

  UTI NodeBinaryOpEqual::checkAndLabelType(Node * thisparentnode)
  {
    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType(this);
    UTI rightType = m_nodeRight->checkAndLabelType(this);

    if(!m_state.neitherNAVokUTItoContinue(leftType, rightType))
      {
	std::ostringstream msg;
	msg << "Assignment is invalid";
	msg << "; LHS: ";
	msg << m_state.getUlamTypeNameByIndex(leftType); //t41388
	msg << "; RHS: ";
	msg << m_state.getUlamTypeNameByIndex(rightType);

	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    if(!m_state.isComplete(leftType) || !m_state.isComplete(rightType))
      {
    	setNodeType(Hzy);
	m_state.setGoAgain(); //for compiler counts
    	return Hzy; //not quietly
      }

    TBOOL stor = checkStoreIntoAble();
    if(stor == TBOOL_FALSE)
      {
	setNodeType(Nav);
	return Nav;
      }
    else if(stor == TBOOL_HAZY)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
      }

    if(!NodeBinaryOp::checkNotVoidTypes(leftType, rightType, false))
      {
    	setNodeType(Nav);
    	return Nav;
      }

    if(m_nodeRight->isExplicitReferenceCast())
      {
	std::ostringstream msg;
	msg << "Explicit Reference cast of assignment is invalid";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    if(m_state.isScalar(leftType) ^ m_state.isScalar(rightType))
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types: ";
	msg << m_state.getUlamTypeNameByIndex(leftType).c_str();
	msg << " and ";
	msg << m_state.getUlamTypeNameByIndex(rightType).c_str();
	msg << " used with binary " << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }


    UTI newType = leftType;
    UlamType * lut = m_state.getUlamTypeByIndex(leftType);
    if(lut->getUlamTypeEnum() == Class)
      {
	bool rhsIsClassOrAtom = m_state.isAClass(rightType) || m_state.isAtom(rightType);
	TBOOL replaced = replaceOurselves(rhsIsClassOrAtom, thisparentnode);

	if(replaced == TBOOL_HAZY)
	  newType = Hzy;
	else if(replaced == TBOOL_TRUE)
	  {
	    m_state.setGoAgain(); //t41119,t41120,t41121,t41134,t41332
	    delete this; //suicide is painless..
	    return Hzy;
	  }
	else //tbool_false
	  {
	    if(rhsIsClassOrAtom)
	      {
		makeDefaultStructAssignment(leftType, rightType, newType);
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Incompatible class type ";
		msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
		msg << " and ";
		msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
		msg << " used with binary " << getName();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		newType = Nav; //error
	      }
	  }
      }
    else //lhs is not a class
      makeDefaultStructAssignment(leftType, rightType, newType);

    setNodeType(newType);
    if(newType == Hzy) m_state.setGoAgain();
    return newType;
  } //checkAndLabelType

  void NodeBinaryOpEqual::makeDefaultStructAssignment(UTI lt, UTI rt, UTI& newtyperef)
  {
    //cast RHS if necessary and safe; newtyperef initialized to lt
    // "safeness" based on deref of newtype
    if(UlamType::compareForAssignment(lt, rt, m_state) != UTIC_SAME)
      {
	//special case for Bits to Bits assignments; if lbs < rbs, i.e. not safe-to-cast, drop bits
	if((m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum() == Bits) && (m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum() == Bits))
	  {
	    UTI derefLeft = m_state.getUlamTypeAsDeref(lt); //tmp deref type
	    if(!Node::makeCastingNode(m_nodeRight, derefLeft, m_nodeRight))
	      newtyperef = Nav; //error
	    else
	      newtyperef = derefLeft; //t41563
	  }
	else if(checkSafeToCastTo(rt, newtyperef))
	  {
	    UTI derefLeft = m_state.getUlamTypeAsDeref(lt); //tmp deref type
	    if(!Node::makeCastingNode(m_nodeRight, derefLeft, m_nodeRight))
	      newtyperef = Nav; //error
	  }
	//else not safe, error msg, newTyperef changed
      } //else the same
  }

  TBOOL NodeBinaryOpEqual::replaceOurselves(bool classoratom, Node * parentnode)
  {
    //here, when lefthand side is a class
    TBOOL rtntb = TBOOL_FALSE;

    if(!classoratom)
      {
	//try for operator overload first (e.g. (pre) +=,-=, (post) ++,-- )
	rtntb = NodeBinaryOp::buildandreplaceOperatorOverloadFuncCallNode(parentnode);
      }
    else
      {
	//RHS is class or atom;
	//first look for (safe cast) arg match overload operator=
	bool hazyArg = false;
	Node * newnode = buildOperatorOverloadFuncCallNodeForMatchingArg(hazyArg);
	if(hazyArg)
	  {
	    rtntb = TBOOL_HAZY;
	  }
	else if(newnode)
	  {
	    AssertBool swapOk = Node::exchangeNodeWithParent(newnode, parentnode);
	    assert(swapOk);

	    m_nodeLeft = NULL; //recycle as memberselect
	    m_nodeRight = NULL; //recycle as func call arg

	    rtntb = TBOOL_TRUE;
	  }
	//else no replacement, use default struct assign
      }
    return rtntb;
  } //replaceOurselves

  //here, we check for func with matching argument when both sides the same Class type,
  // or rhs is an atom, so we can use the default struct equal if no overload defined.
  Node * NodeBinaryOpEqual::buildOperatorOverloadFuncCallNodeForMatchingArg(bool& hazyArg)
  {
    UTI leftType = m_nodeLeft->getNodeType();
    Token identTok;
    TokenType opTokType = Token::getTokenTypeFromString(getName());
    assert(opTokType != TOK_LAST_ONE);
    Token opTok(opTokType, getNodeLocation(), 0);
    u32 opolId = Token::getOperatorOverloadFullNameId(opTok, &m_state);
    if(opolId == 0)
      {
	std::ostringstream msg;
	msg << "Overload for operator" << getName();
	msg << " is not supported as operand for class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return NULL;
      }

    identTok.init(TOK_IDENTIFIER, getNodeLocation(), opolId);

    //may need to fall back to default struct equal when the same class (t41119), or ref (t41120)

    Node * rtnNode = NULL;
    Symbol * fnsymptr = NULL;
    bool hazyKin = false;

    if(m_state.isFuncIdInAClassScopeOrAncestor(leftType, opolId, fnsymptr, hazyKin) && !hazyKin)
      {
	// still need to pinpoint the SymbolFunction; ok to look for safe casts (t41120).
	std::vector<Node *> argNodes;
	argNodes.push_back(m_nodeRight); //(destructor not called)

	SymbolFunction * funcSymbol = NULL;
	bool tmphazyargs = false;
	UTI foundInAncestor = Nouti;
	u32 numFuncs = m_state.findMatchingFunctionWithSafeCastsInAClassScopeOrAncestor(leftType, opolId, argNodes, funcSymbol, tmphazyargs, foundInAncestor);

	if(tmphazyargs)
	  hazyArg = true;
	if(numFuncs >= 1)
	  {
	    // ambiguous (>1) overload will produce an error later
	    //fill in func symbol during type labeling;
	    NodeFunctionCall * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
	    assert(fcallNode);
	    fcallNode->setNodeLocation(identTok.m_locator);

	    fcallNode->addArgument(m_nodeRight);

	    NodeMemberSelect * mselectNode = new NodeMemberSelect(m_nodeLeft, fcallNode, m_state);
	    assert(mselectNode);
	    mselectNode->setNodeLocation(identTok.m_locator);
	    rtnNode = mselectNode;
	  }//else use default struct equal, or wait for hazy arg
      }
    else if(hazyKin)
      hazyArg = hazyKin;

    //redo check and type labeling done by caller!!
    return rtnNode; //replace right node with new branch
  } //buildOperatorOverloadFuncCallNodeForMatchingArg

  TBOOL NodeBinaryOpEqual::checkStoreIntoAble()
  {
    TBOOL lstor = m_nodeLeft->getStoreIntoAble();
    if(lstor != TBOOL_TRUE)
      {
	UTI lt = m_nodeLeft->getNodeType();
	std::ostringstream msg;
	msg << "Unmodifiable lefthand side of assignment expression '";
	msg << m_nodeLeft->getName();
	msg << "', type: " << m_state.getUlamTypeNameByIndex(lt).c_str(); //t41186
	if(m_nodeLeft->isFunctionCall())
	  msg << "; may be a function call";
	else if(m_nodeLeft->hasASymbolReference() && m_nodeLeft->hasASymbolReferenceConstant())
	  msg << "; may be a constant function parameter";
	if(lstor == TBOOL_HAZY)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    setStoreIntoAble(lstor);
    return lstor;
  } //checkStoreIntoAble

  bool NodeBinaryOpEqual::checkNotUnpackedArray()
  {
    bool rtnOK = true;
    UTI lt = m_nodeLeft->getNodeType();
    UTI rt = m_nodeRight->getNodeType();
    PACKFIT lpacked = m_state.determinePackable(lt);
    PACKFIT rpacked = m_state.determinePackable(rt);
    bool unpackedArrayLeft = !WritePacked(lpacked) && !m_state.isScalar(lt);
    bool unpackedArrayRight = !WritePacked(rpacked) && !m_state.isScalar(rt);

    if(unpackedArrayLeft || unpackedArrayRight)
      {
	if(unpackedArrayLeft)
	  {
	    std::ostringstream msg;
	    msg << "Lefthand side of equals requires UNPACKED array support '";
	    msg << m_nodeLeft->getName();
	    msg << "', type: " << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	if(unpackedArrayRight)
	  {
	    std::ostringstream msg;
	    msg << "Righthand side of equals requires UNPACKED array support '";
	    msg << m_nodeRight->getName();
	    msg << "', type: " << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	rtnOK = false;
      }
    return rtnOK;
  } //checkNotUnpackedArray

  UTI NodeBinaryOpEqual::calcNodeType(UTI lt, UTI rt)
  {
    m_state.abortShouldntGetHere();
    return Nav;
  }

  EvalStatus NodeBinaryOpEqual::eval()
  {
    assert(m_nodeLeft && m_nodeRight);

    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0); //new current frame pointer on node eval stack

    makeRoomForSlots(1); //always 1 slot for ptr

    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    u32 slot = makeRoomForNodeType(nuti);
    evs = m_nodeRight->eval(); //a Node Function Call here
    if(evs != NORMAL) return evalStatusReturn(evs);

    //assigns rhs to lhs UV pointer (handles arrays);
    //also copy result UV to stack, -1 relative to current frame pointer
    if(slot)
      if(!doBinaryOperation(1, 2, slot))
	return evalStatusReturn(ERROR);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeBinaryOpEqual::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0);

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue luvPtr = UlamValue::makePtr(1, EVALRETURN, nuti, m_state.determinePackable(nuti), m_state); //positive to current frame pointer

    Node::assignReturnValuePtrToStack(luvPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  bool NodeBinaryOpEqual::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    UTI nuti = getNodeType();
    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(lslot);
    UlamValue ruv;

    if(!pluv.isPtr())
      return false; //e.g. t41089 func call used on lhs (gen code ok)

    if(m_state.isScalar(nuti))
      {
	ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot); //immediate scalar
      }
    else
      {
	PACKFIT packed = m_state.determinePackable(nuti);
	if(WritePacked(packed))
	  {
	    ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot); //packed/PL array
	  }
	else
	  {
	    // unpacked array requires a ptr
	    ruv = UlamValue::makePtr(rslot, EVALRETURN, nuti, packed, m_state); //ptr
	  }
      }

    UTI ruti = ruv.getUlamValueTypeIdx();
    if(ruti == Nav || nuti == Nav)
      return false;

    if(ruti == Hzy || nuti == Hzy)
      return false;

    //before assert in CS, fails
    UTI luti = pluv.getPtrTargetType();
    if( m_state.isPtr(ruti) || (UlamType::compareForUlamValueAssignment(luti, ruti, m_state) == UTIC_SAME) || m_state.isAtom(luti) || m_state.isAtom(ruti))
      m_state.assignValue(pluv,ruv);
    else
      return false;

    //also copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(ruv);
    return true;
  } //dobinaryoperation

  bool NodeBinaryOpEqual::doBinaryOperationImmediate(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots == 1);
    UTI nuti = getNodeType();
    u32 len = m_state.getTotalBitSize(nuti);

    // 'pluv' is where the resulting sum needs to be stored
    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(lslot); //a Ptr
    assert(m_state.isPtr(pluv.getUlamValueTypeIdx()) && (UlamType::compare(pluv.getPtrTargetType(),nuti, m_state) == UTIC_SAME));

    assert(slots == 1);
    UlamValue luv = m_state.getPtrTarget(pluv);  //no eval!!
    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot); //immediate value

    if((luv.getUlamValueTypeIdx() == Nav) || (ruv.getUlamValueTypeIdx() == Nav))
      return false;

    if((luv.getUlamValueTypeIdx() == Hzy) || (ruv.getUlamValueTypeIdx() == Hzy))
      return false;

    UlamValue rtnUV;
    u32 wordsize = m_state.getTotalWordSize(nuti);
    if(wordsize == MAXBITSPERINT)
      {
	u32 ldata = luv.getDataFromAtom(pluv, m_state);
	u32 rdata = ruv.getImmediateData(len, m_state);
	rtnUV = makeImmediateBinaryOp(nuti, ldata, rdata, len);
      }
    else if(wordsize == MAXBITSPERLONG)
      {
	u64 ldata = luv.getDataLongFromAtom(pluv, m_state);
	u64 rdata = ruv.getImmediateDataLong(len);
	rtnUV = makeImmediateLongBinaryOp(nuti, ldata, rdata, len);
      }
    else
      m_state.abortGreaterThanMaxBitsPerLong();//e.g. 0

    if(rtnUV.getUlamValueTypeIdx() == Nav)
      return false;

    if(rtnUV.getUlamValueTypeIdx() == Hzy)
      return false;

    m_state.assignValue(pluv,rtnUV);

    //also copy result UV to stack, -1 relative to current frame pointer
    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);
    return true;
  } //dobinaryopImmediate

  bool NodeBinaryOpEqual::doBinaryOperationArray(s32 lslot, s32 rslot, u32 slots)
  {
    UlamValue rtnUV;
    UTI nuti = getNodeType();
    s32 arraysize = m_state.getArraySize(nuti); //could be zero length array
    s32 bitsize = m_state.getBitSize(nuti);
    UTI scalartypidx = m_state.getUlamTypeAsScalar(nuti);
    PACKFIT packRtn = m_state.determinePackable(nuti);

    if(WritePacked(packRtn))
      {
	//pack result too. (slot size known ahead of time)
	rtnUV = UlamValue::makeAtom(nuti); //accumulate result here
      }

    // 'pluv' is where the resulting sum needs to be stored
    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(lslot); //a Ptr
    assert(m_state.isPtr(pluv.getUlamValueTypeIdx()) && (UlamType::compare(pluv.getPtrTargetType(), nuti, m_state) == UTIC_SAME));

    // point to base array slots, packedness determines its 'pos'
    UlamValue lArrayPtr = pluv;
    UlamValue rArrayPtr = UlamValue::makePtr(rslot, EVALRETURN, nuti, packRtn, m_state);

    // to use incrementPtr(), 'pos' depends on packedness
    UlamValue lp = UlamValue::makeScalarPtr(lArrayPtr, m_state);
    UlamValue rp = UlamValue::makeScalarPtr(rArrayPtr, m_state);

    u32 navCount = 0;
    u32 hzyCount = 0;

    //make immediate result for each element inside loop
    for(s32 i = 0; i < arraysize; i++)
      {
	UlamValue luv = m_state.getPtrTarget(lp);
	UlamValue ruv = m_state.getPtrTarget(rp);

	u32 ldata = luv.getData(lp.getPtrPos(), bitsize); //'pos' doesn't vary for unpacked
	u32 rdata = ruv.getData(rp.getPtrPos(), bitsize); //'pos' doesn't vary for unpacked

	if(WritePacked(packRtn))
	  // use calc position where base [0] is furthest from the end.
	  appendBinaryOp(rtnUV, ldata, rdata, (BITSPERATOM-(bitsize * (arraysize - i))), bitsize);
	else
	  {
	    // else, unpacked array
	    rtnUV = makeImmediateBinaryOp(scalartypidx, ldata, rdata, bitsize);
	    if(rtnUV.getUlamValueTypeIdx() == Nav)
	      navCount++;
	    else if(rtnUV.getUlamValueTypeIdx() == Hzy)
	      hzyCount++;

	    // overwrite lhs copy with result UV
	    m_state.assignValue(lp, rtnUV);

	    //copy result UV to stack, -1 (1st array element deepest) relative to current frame pointer
	    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -slots + i);
	  }

	AssertBool isNextLeft = lp.incrementPtr(m_state);
	assert(isNextLeft);
	AssertBool isNextRight = rp.incrementPtr(m_state);
	assert(isNextRight);
      } //forloop

    if(navCount > 0)
      return false;

    if(hzyCount > 0)
      return false;

    if(WritePacked(packRtn))
      {
	m_state.assignValue(pluv, rtnUV); //overwrite lhs copy with result UV
	m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1); //store accumulated packed result
      }
    return true;
  } //dobinaryoparray

  UlamValue NodeBinaryOpEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    m_state.abortShouldntGetHere(); //unused
    return UlamValue();
  }

  UlamValue NodeBinaryOpEqual::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    m_state.abortShouldntGetHere(); //unused
    return UlamValue();
  }

  void NodeBinaryOpEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    m_state.abortShouldntGetHere(); //unused
  }

  void NodeBinaryOpEqual::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty());

    // generate rhs first; may update current object globals (e.g. function call)
    UVPass ruvpass;
    m_nodeRight->genCode(fp, ruvpass);

    // restore current object globals
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************

    // lhs should be the new current object: node member select updates them,
    // but a plain NodeIdent does not!!!  because genCodeToStoreInto has been repurposed
    // to mean "don't read into a TmpVar" (e.g. by NodeCast).
    UVPass luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //may update m_currentObjSymbol, m_currentSelfSymbol

    // current object globals should pertain to lhs for the write
    Node::genCodeWriteFromATmpVar(fp, luvpass, ruvpass); //uses rhs' tmpvar

    uvpass = ruvpass; //in case we're the rhs of an equals..

    assert(m_state.m_currentObjSymbolsForCodeGen.empty());
  } //genCode

} //end MFM
