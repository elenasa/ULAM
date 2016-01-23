#include "NodeBinaryOpEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqual::NodeBinaryOpEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state) {}

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

  UTI NodeBinaryOpEqual::checkAndLabelType()
  {
    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();

    if(!m_state.isComplete(leftType) || !m_state.isComplete(rightType))
      {
    	setNodeType(Hzy);
	m_state.setGoAgain(); //for compiler counts
    	return Hzy; //not quietly
      }

    if(!checkStoreIntoAble())
      {
	setNodeType(Nav);
	return Nav;
      }

    if(!checkNotUnpackedArray())
      {
	setNodeType(Nav);
	return Nav;
      }

    if(!NodeBinaryOp::checkNotVoidTypes(leftType, rightType))
      {
    	setNodeType(Nav);
    	return Nav;
      }

    if(m_state.isScalar(leftType) ^ m_state.isScalar(rightType))
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types: ";
	msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
	msg << " and ";
	msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
	msg << " used with binary operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    UTI newType = leftType; //init
    //cast RHS if necessary and safe
    if(UlamType::compare(newType, rightType, m_state) != UTIC_SAME)
      {
	//different msg if try to assign non-class to a class type
	if((m_state.getUlamTypeByIndex(leftType)->getUlamTypeEnum() == Class) && (m_state.getUlamTypeByIndex(rightType)->getUlamTypeEnum() != Class))
	  {
	    std::ostringstream msg;
	    msg << "Incompatible class type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
	    msg << " and ";
	    msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
	    msg << " used with binary operator" << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav; //error
	  }
	else
	  {
	    FORECAST rscr = m_nodeRight->safeToCastTo(newType);
	    if(rscr != CAST_CLEAR)
	      {
		std::ostringstream msg;
		if(m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum() == Bool)
		  msg << "Use a comparison operator";
		else
		  msg << "Use explicit cast";
		msg << " to convert "; // the real converting-message
		msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
		msg << " to ";
		msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
		msg << " for operator" << getName();
		if(rscr == CAST_BAD)
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    newType = Nav; //error
		  }
		else
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    m_state.setGoAgain(); //for compiler counts
		    newType = Hzy;
		  }
	      }
	    else if(!Node::makeCastingNode(m_nodeRight, newType, m_nodeRight))
	      newType = Nav; //error
	  }
      }
    setNodeType(newType);
    setStoreIntoAble(m_state.okUTItoContinue(newType)); //ok true
    return newType;
  } //checkAndLabelType

  bool NodeBinaryOpEqual::checkStoreIntoAble()
  {
    if(!m_nodeLeft->isStoreIntoAble())
      {
	UTI lt = m_nodeLeft->getNodeType();
	std::ostringstream msg;
	msg << "Invalid lefthand side of equals <" << m_nodeLeft->getName();
	msg << ">, type: " << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setStoreIntoAble(false);
	return false;
      }
    setStoreIntoAble(true);
    return true;
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
	    msg << "Lefthand side of equals requires UNPACKED array support <";
	    msg << m_nodeLeft->getName();
	    msg << ">, type: " << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	if(unpackedArrayRight)
	  {
	    std::ostringstream msg;
	    msg << "Righthand side of equals requires UNPACKED array support <";
	    msg << m_nodeRight->getName();
	    msg << ">, type: " << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	rtnOK = false;
      }
    return rtnOK;
  } //checkNotUnpackedArray

  UTI NodeBinaryOpEqual::calcNodeType(UTI lt, UTI rt)
  {
    assert(0);
    return Nav;
  }

  EvalStatus NodeBinaryOpEqual::eval()
  {
    assert(m_nodeLeft && m_nodeRight);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    evalNodeProlog(0); //new current frame pointer on node eval stack

    makeRoomForSlots(1); //always 1 slot for ptr

    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    u32 slot = makeRoomForNodeType(nuti);
    evs = m_nodeRight->eval(); //a Node Function Call here
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //assigns rhs to lhs UV pointer (handles arrays);
    //also copy result UV to stack, -1 relative to current frame pointer
    if(slot)
      if(!doBinaryOperation(1, 2, slot))
	evs = ERROR;

    evalNodeEpilog();
    return evs;
  } //eval

  EvalStatus NodeBinaryOpEqual::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    evalNodeProlog(0);

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

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

    if(ruv.getUlamValueTypeIdx() == Nav || nuti == Nav)
      return false;

    if(ruv.getUlamValueTypeIdx() == Hzy || nuti == Hzy)
      return false;

    m_state.assignValue(pluv,ruv);

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
    assert(pluv.getUlamValueTypeIdx() == Ptr && pluv.getPtrTargetType() == nuti);

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
      assert(0); //e.g. 0

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
    assert(pluv.getUlamValueTypeIdx() == Ptr && pluv.getPtrTargetType() == nuti);

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
    assert(0); //unused
    return UlamValue();
  }

  UlamValue NodeBinaryOpEqual::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    assert(0); //unused
    return UlamValue();
  }

  void NodeBinaryOpEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //unused
  }

  void NodeBinaryOpEqual::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    assert(m_nodeRight);
    m_nodeRight->calcMaxDepth(depth, maxdepth, base); //funccall?
    return; //work done by NodeStatements and NodeBlock
  }

  void NodeBinaryOpEqual::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty());

    // generate rhs first; may update current object globals (e.g. function call)
    UlamValue ruvpass;
    m_nodeRight->genCode(fp, ruvpass);

    // restore current object globals
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************

    // lhs should be the new current object: node member select updates them,
    // but a plain NodeIdent does not!!!  because genCodeToStoreInto has been repurposed
    // to mean "don't read into a TmpVar" (e.g. by NodeCast).
    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //may update m_currentObjSymbol, m_currentSelfSymbol

    // current object globals should pertain to lhs for the write
    Node::genCodeWriteFromATmpVar(fp, luvpass, ruvpass); //uses rhs' tmpvar

    uvpass = ruvpass; //in case we're the rhs of an equals..

    assert(m_state.m_currentObjSymbolsForCodeGen.empty());
  } //genCode

} //end MFM
