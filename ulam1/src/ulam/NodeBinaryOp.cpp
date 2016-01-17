#include <stdio.h>
#include "NodeBinaryOp.h"
#include "NodeTerminal.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOp::NodeBinaryOp(Node * left, Node * right, CompilerState & state) : Node(state), m_nodeLeft(left), m_nodeRight(right) { }

  NodeBinaryOp::NodeBinaryOp(const NodeBinaryOp& ref) : Node(ref)
  {
    m_nodeLeft = ref.m_nodeLeft->instantiate();
    m_nodeRight = ref.m_nodeRight->instantiate();
  }

  NodeBinaryOp::~NodeBinaryOp()
  {
    delete m_nodeLeft;
    m_nodeLeft = NULL;
    delete m_nodeRight;
    m_nodeRight = NULL;
  }

  void NodeBinaryOp::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_nodeLeft->updateLineage(getNodeNo());
    m_nodeRight->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeBinaryOp::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeLeft == oldnptr)
      {
	m_nodeLeft = newnptr;
	return true;
      }
    if(m_nodeRight == oldnptr)
      {
	m_nodeRight = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeBinaryOp::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeLeft->findNodeNo(n, foundNode))
      return true;
    if(m_nodeRight->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeBinaryOp::checkAbstractInstanceErrors()
  {
      m_nodeLeft->checkAbstractInstanceErrors();
      m_nodeRight->checkAbstractInstanceErrors();
  } //checkAbstractInstanceErrors

  void NodeBinaryOp::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    if(m_nodeLeft)
      m_nodeLeft->print(fp);
    else
      fp->write("<NULLLEFT>\n");

    if(m_nodeRight)
      m_nodeRight->print(fp);
    else
      fp->write("<NULLRIGHT>\n");

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  } //print

  void NodeBinaryOp::printPostfix(File * fp)
  {
    if(m_nodeLeft)
      m_nodeLeft->printPostfix(fp);
    else
      fp->write("<NULLLEFT>");

    if(m_nodeRight)
      m_nodeRight->printPostfix(fp);
    else
      fp->write("<NULLRIGHT>");

    printOp(fp); //operators last
  } //printPostfix

  void NodeBinaryOp::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  } //printOp

  bool NodeBinaryOp::isAConstant()
  {
    assert(m_nodeLeft && m_nodeRight);
    return m_nodeLeft->isAConstant() && m_nodeRight->isAConstant();
  }

  bool NodeBinaryOp::isReadyConstant()
  {
    //needs constant folding
    return false;
  }

  bool NodeBinaryOp::isFunctionCall()
  {
    assert(m_nodeLeft && m_nodeRight);
    return m_nodeLeft->isFunctionCall() || m_nodeRight->isFunctionCall();
  }

  FORECAST NodeBinaryOp::safeToCastTo(UTI newType)
  {
    //ulamtype checks for complete, non array, and type specific rules
    return m_state.getUlamTypeByIndex(newType)->safeCast(getNodeType());
  } //safeToCastTo

  UTI NodeBinaryOp::checkAndLabelType()
  {
    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();

    // efficiency bites! no sooner, need left and right side-effects
    // (e.g. NodeControl condition is Bool at start; stubs need Symbol ptrs)
    if(m_state.isComplete(getNodeType()))
      return getNodeType();

    UTI newType = calcNodeType(leftType, rightType); //does safety check

    setNodeType(newType);
    setStoreIntoAble(false);

    if(m_state.isComplete(newType))
      {
	if(UlamType::compare(newType, leftType, m_state) != UTIC_SAME) //not same, or dontknow
	  {
	    if(!Node::makeCastingNode(m_nodeLeft, newType, m_nodeLeft))
	      newType = Nav;
	  }

	if(UlamType::compare(newType, rightType, m_state) != UTIC_SAME) //not same, or dontknow
	  {
	    if(!Node::makeCastingNode(m_nodeRight, newType, m_nodeRight))
	      newType = Nav;
	  }
      }

    //before constant folding; if needed (e.g. Remainder, Divide)
    castThyselfToResultType(rightType, leftType, newType);

    if((newType != Nav) && isAConstant() && m_nodeLeft->isReadyConstant() && m_nodeRight->isReadyConstant())
      return constantFold();

    return newType;
  } //checkAndLabelType

  UTI NodeBinaryOp::castThyselfToResultType(UTI rt, UTI lt, UTI newType)
  {
    return newType; //noop
  }

  bool NodeBinaryOp::checkSafeToCastTo(UTI newType)
  {
    bool rtnOK = true;
    FORECAST lsafe = m_nodeLeft->safeToCastTo(newType);
    FORECAST rsafe = m_nodeRight->safeToCastTo(newType);
    if( lsafe != CAST_CLEAR || rsafe != CAST_CLEAR )
      {
	std::ostringstream msg;
	if(m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum() == Bool)
	  msg << "Use a comparison operator";
	else
	  msg << "Use explicit cast";
	msg << " to convert "; // the real converting-message
	msg << m_state.getUlamTypeNameBriefByIndex(m_nodeLeft->getNodeType()).c_str();
	msg << " and ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_nodeRight->getNodeType()).c_str();
	msg << " to ";
	msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
	msg << " for binary operator" << getName();
	if(lsafe == CAST_HAZY || rsafe == CAST_HAZY)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnOK = false;
      } //not safe
    return rtnOK;
  } //checkSafeToCastTo

  //no atoms, elements nor voids as either operand
  bool NodeBinaryOp::checkForPrimitiveTypes(UTI lt, UTI rt)
  {
    bool rtnOK = true;
    if(!m_state.getUlamTypeByIndex(lt)->isPrimitiveType())
      {
	std::ostringstream msg;
	msg << "Non-primitive type <";
	msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	msg << "> is not supported as left operand type for binary operator";
	msg << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnOK = false;
      }

    if(!m_state.getUlamTypeByIndex(rt)->isPrimitiveType())
      {
	std::ostringstream msg;
	msg << "Non-primitive type <";
	msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	msg << "> is not supported as right operand type for binary operator";
	msg << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnOK = false;
      }

    rtnOK &= checkNotVoidTypes(lt, rt);
    return rtnOK;
  } //checkForPrimitiveTypes

  bool NodeBinaryOp::checkNotVoidTypes(UTI lt, UTI rt)
  {
    bool rtnOK = true;
    ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
    ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();
    if(ltypEnum == Void || rtypEnum == Void)
      {
	std::ostringstream msg;
	msg << "Void is not a supported type for binary operator";
	msg << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnOK = false;
      }
    return rtnOK;
  } //checkNotVoidTypes

  bool NodeBinaryOp::checkForNumericTypes(UTI lt, UTI rt)
  {
    bool rtnOK = true;
    bool lnum = m_state.getUlamTypeByIndex(lt)->isNumericType();
    bool rnum = m_state.getUlamTypeByIndex(rt)->isNumericType();
    if(!(lnum && rnum))
      {
	std::ostringstream msg;
	msg << "Incompatible types for binary operator";
	msg << getName() << " : ";
	msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	msg << ", ";
	msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	msg << "; Suggest casting to a numeric type first";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnOK = false;
      }
    return rtnOK;
  } //checkForNumericTypes

  bool NodeBinaryOp::checkScalarTypesOnly(UTI lt, UTI rt)
  {
    bool rtnOK = true;
    if( !(m_state.isScalar(lt) && m_state.isScalar(rt)))
      {
	rtnOK = false;

	//#define SUPPORT_ARITHMETIC_ARRAY_OPS
#ifdef SUPPORT_ARITHMETIC_ARRAY_OPS
	// Conflicted: we don't like the idea that the type might be
	// different for arrays than scalars; casting occurring differently.
	// besides, for arithmetic ops, unlike logical ops, we have to do each
	// op separately anyway, so no big win (let ulam programmer do the loop).
	// let arrays of same types through ??? Is SO for op equals, btw.
	if(lt == rt)
	  {
	    return lt;
	  }
#endif //SUPPORT_ARITHMETIC_ARRAY_OPS

	//array op scalar: defer since the question of matrix operations is unclear.
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types ";
	msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	msg << " and " << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	msg << " for binary operator";
	msg << getName() << " ; Suggest writing a loop";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return rtnOK;
  } //checkScalarTypesOnly

  s32 NodeBinaryOp::resultBitsize(UTI lt, UTI rt)
  {
    s32 lbs = UNKNOWNSIZE, rbs = UNKNOWNSIZE, wordsize = UNKNOWNSIZE;
    resultBitsizeCalc(lt, rt, lbs, rbs, wordsize);

    s32 maxbs = (lbs > rbs ? lbs : rbs);
    return (maxbs >= wordsize ? wordsize : maxbs);
  } //resultBitsize

  void NodeBinaryOp::resultBitsizeCalc(UTI lt, UTI rt, s32& lbs, s32&rbs, s32&lwordsize)
  {
    UlamType * lut = m_state.getUlamTypeByIndex(lt);
    UlamType * rut = m_state.getUlamTypeByIndex(rt);

    //both sides complete by now!!
    assert(lut->isComplete() && rut->isComplete());

    // types are either unsigned or signed (unary as unsigned)
    ULAMTYPE ltypEnum = lut->getUlamTypeEnum();
    ULAMTYPE rtypEnum = rut->getUlamTypeEnum();

    lbs = lut->getBitSize();
    rbs = rut->getBitSize();
    lwordsize = (s32) lut->getTotalWordSize();
    s32 rwordsize = (s32) rut->getTotalWordSize();

    if(ltypEnum == Class)
      {
	if(lut->isNumericType()) //i.e. a quark
	  {
	    lwordsize = lbs = MAXBITSPERINT; //32
	    ltypEnum = Int; //for mix test
	  }
      }
    else if(ltypEnum == Unary)
      {
	lbs = (s32) _getLogBase2(lbs) + 1; //fits into unsigned
	ltypEnum = Unsigned; //for mix test
      }
    else //could be Unsigned or Int, not Bits
      assert(ltypEnum == Unsigned || ltypEnum == Int);

    if(rtypEnum == Class)
      {
	if(rut->isNumericType()) //i.e. a quark
	  {
	    rwordsize = rbs = MAXBITSPERINT; //32
	    rtypEnum = Int; //for mix test
	  }
      }
    else if(rtypEnum == Unary)
      {
	rbs = (s32) _getLogBase2(rbs) + 1; //fits into unsigned
	rtypEnum = Unsigned; //for mix test
      }
    else //could be Unsigned or Int, not Bits
      assert(rtypEnum == Unsigned || rtypEnum == Int);

    if(lwordsize != rwordsize)
      {
	std::ostringstream msg;
	msg << "Word sizes incompatible for types ";
	msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	msg << " and " << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	msg << " for binary operator";
	msg << getName() << " ; Suggest a cast";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	assert(0);
      }

    // adjust for mixed sign and unsigned types
    if(ltypEnum != rtypEnum && (ltypEnum == Int || rtypEnum == Int))
      {
	if(ltypEnum != Int)
	  {
	    lbs = lut->bitsizeToConvertTypeTo(Int); //fits into signed
	    ltypEnum = Int;
	  }
	else if(rtypEnum != Int)
	  {
	    rbs = rut->bitsizeToConvertTypeTo(Int); //fits into signed
	    rtypEnum = Int;
	  }
      }
  } //resultBitsizeCalc

  void NodeBinaryOp::resultBitsizeCalcInBits(UTI lt, UTI rt, s32& lbs, s32&rbs, s32&lwordsize)
  {
    UlamType * lut = m_state.getUlamTypeByIndex(lt);
    UlamType * rut = m_state.getUlamTypeByIndex(rt);

    //both sides complete to be here!!
    assert(lut->isComplete() && rut->isComplete());

    // types are either unsigned or signed (unary as-is)
    ULAMTYPE ltypEnum = lut->getUlamTypeEnum();
    ULAMTYPE rtypEnum = rut->getUlamTypeEnum();

    lbs = lut->getBitSize();
    rbs = rut->getBitSize();
    lwordsize = (s32) lut->getTotalWordSize();
    s32 rwordsize = (s32) rut->getTotalWordSize();

    if(ltypEnum == Class)
      {
	if(lut->isNumericType()) //i.e. a quark
	  lwordsize = lbs = MAXBITSPERINT; //32
      }

    if(rtypEnum == Class)
      {
	if(rut->isNumericType()) //i.e. a quark
	  rwordsize = rbs = MAXBITSPERINT; //32
      }

    if(lwordsize != rwordsize)
      {
	std::ostringstream msg;
	msg << "Word sizes incompatible for types ";
	msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	msg << " and " << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	msg << " for binary operator";
	msg << getName() << " ; Suggest a cast";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	assert(0);
      }
  } //resultBitsizeCalcInBits

  void NodeBinaryOp::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt); //missing
    m_nodeLeft->countNavNodes(cnt);
    m_nodeRight->countNavNodes(cnt);
  }

  UTI NodeBinaryOp::constantFold()
  {
    u64 val = 0;
    UTI nuti = getNodeType();

    if(nuti == Nav) return Nav; //nothing to do yet
    //if(nuti == Hzy) return Hzy; //nothing to do yet TRY?

    // if here, must be a constant..
    assert(isAConstant());

    NNO pno = Node::getYourParentNo();
    assert(pno);
    Node * parentNode = m_state.findNodeNoInThisClass(pno);
    if(!parentNode)
      {
	std::ostringstream msg;
	msg << "Constant value expression for binary op" << getName();
	msg << " cannot be constant-folded at this time while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	msg << " Parent required";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	assert(0); //parent required
      }

    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(nuti); //offset a constant expression
    EvalStatus evs = eval();
    if( evs == NORMAL)
      {
	UlamValue cnstUV = m_state.m_nodeEvalStack.popArg();
	u32 wordsize = m_state.getTotalWordSize(nuti);
	if(wordsize == MAXBITSPERINT)
	  val = cnstUV.getImmediateData(m_state);
	else if(wordsize == MAXBITSPERLONG)
	  val = cnstUV.getImmediateDataLong(m_state);
	else
	  assert(0);
      }

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for binary op" << getName();
	msg << " is erroneous while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    if(evs == NOTREADY)
      {
	std::ostringstream msg;
	msg << "Constant value expression for binary op" << getName();
	msg << " is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	setNodeType(Hzy);
	return Hzy;
      }

    //replace ourselves (and kids) with a node terminal; new NNO unlike template's
    NodeTerminal * newnode = new NodeTerminal(val, nuti, m_state);
    assert(newnode);
    newnode->setNodeLocation(getNodeLocation());

    AssertBool swapOk = parentNode->exchangeKids(this, newnode);
    assert(swapOk);

    std::ostringstream msg;
    msg << "Exchanged kids! for binary operator" << getName();
    msg << ", with a constant == " << newnode->getName();
    msg << " while compiling class: ";
    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

    newnode->setYourParentNo(pno); //a leaf
    newnode->resetNodeNo(getNodeNo());

    delete this; //suicide is painless..

    return newnode->checkAndLabelType();
  } //constantFold

  bool NodeBinaryOp::assignClassArgValueInStubCopy()
  {
    bool aok = true;
    aok &= m_nodeLeft->assignClassArgValueInStubCopy();
    aok &= m_nodeRight->assignClassArgValueInStubCopy();
    return aok;
  }

  EvalStatus NodeBinaryOp::eval()
  {
    assert(m_nodeLeft && m_nodeRight);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    evalNodeProlog(0); //new current frame pointer

    u32 slot = makeRoomForNodeType(nuti);
    EvalStatus evs = m_nodeLeft->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    u32 slot2 = makeRoomForNodeType(nuti);
    evs = m_nodeRight->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //copies return UV to stack, -1 relative to current frame pointer
    if(slot && slot2)
      if(!doBinaryOperation(1, 1+slot, slot2))
	evs = ERROR;

    evalNodeEpilog();
    return evs;
  } //eval

  bool NodeBinaryOp::doBinaryOperationImmediate(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots == 1);
    UTI nuti = getNodeType();
    u32 len = m_state.getTotalBitSize(nuti);

    UlamValue luv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(lslot); //immediate value
    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot); //immediate value

    if((luv.getUlamValueTypeIdx() == Nav) || (ruv.getUlamValueTypeIdx() == Nav))
      return false;

    if((luv.getUlamValueTypeIdx() == Hzy) || (ruv.getUlamValueTypeIdx() == Hzy))
      return false;

    UlamValue rtnUV;
    u32 wordsize = m_state.getTotalWordSize(nuti);
    if(wordsize == MAXBITSPERINT)
      {
	u32 ldata = luv.getImmediateData(len, m_state);
	u32 rdata = ruv.getImmediateData(len, m_state);
	rtnUV = makeImmediateBinaryOp(nuti, ldata, rdata, len);
      }
    else if(wordsize == MAXBITSPERLONG)
      {
	u64 ldata = luv.getImmediateDataLong(len);
	u64 rdata = ruv.getImmediateDataLong(len);
	rtnUV = makeImmediateLongBinaryOp(nuti, ldata, rdata, len);
      }
    else
      assert(0);

    if(rtnUV.getUlamValueTypeIdx() == Nav)
      return false;

    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);
    return true;
  } //dobinaryopImmediate

  bool NodeBinaryOp::doBinaryOperationArray(s32 lslot, s32 rslot, u32 slots)
  {
    UlamValue rtnUV;

    UTI nuti = getNodeType();
    s32 arraysize = m_state.getArraySize(nuti);
    s32 bitsize = m_state.getBitSize(nuti);
    UTI scalartypidx = m_state.getUlamTypeAsScalar(nuti);
    PACKFIT packRtn = m_state.determinePackable(nuti);

    if(WritePacked(packRtn))
      {
	// pack result too. (slot size known ahead of time)
	rtnUV = UlamValue::makeAtom(nuti); //accumulate result here
      }

    // point to base array slots, packedness determines its 'pos'
    UlamValue lArrayPtr = UlamValue::makePtr(lslot, EVALRETURN, nuti, packRtn, m_state);
    UlamValue rArrayPtr = UlamValue::makePtr(rslot, EVALRETURN, nuti, packRtn, m_state);

    // to use incrementPtr(), 'pos' depends on packedness
    UlamValue lp = UlamValue::makeScalarPtr(lArrayPtr, m_state);
    UlamValue rp = UlamValue::makeScalarPtr(rArrayPtr, m_state);

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
	    rtnUV = makeImmediateBinaryOp(scalartypidx, ldata, rdata, bitsize);

	    //copy result UV to stack, -1 (first array element deepest) relative to current frame pointer
	    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -slots + i);
	  }

	AssertBool isNextLeft = lp.incrementPtr(m_state);
	assert(isNextLeft);
	AssertBool isNextRight = rp.incrementPtr(m_state);
	assert(isNextRight);
      } //forloop

    if(rtnUV.getUlamValueTypeIdx() == Nav)
      return false;

    if(WritePacked(packRtn))
	m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1); //store accumulated packed result
    return true;
  } //dobinaryoparray

  void NodeBinaryOp::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    assert(m_nodeLeft && m_nodeRight);
    m_nodeLeft->calcMaxDepth(depth, maxdepth, base); //funccall?
    m_nodeRight->calcMaxDepth(depth, maxdepth, base); //funccall?
    return; //work done by NodeStatements and NodeBlock
  }

  void NodeBinaryOp::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************

    //generate rhs first; may update current object globals (e.g. function call)
    UlamValue ruvpass;
    m_nodeRight->genCode(fp, ruvpass);

    //restore current object globals
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************

    UlamValue luvpass;
    m_nodeLeft->genCode(fp, luvpass); //updates m_currentObjSymbol

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64..
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(nuti,tmpVarNum).c_str());
    fp->write(" = ");

    fp->write(methodNameForCodeGen().c_str());
    fp->write("(");

    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType();
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");

    UTI ruti = ruvpass.getUlamValueTypeIdx();
    assert(ruti == Ptr);
    ruti = ruvpass.getPtrTargetType();
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");

    fp->write_decimal(nut->getTotalBitSize()); //if scalar, it's just the bitsize

    fp->write(");\n");

    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0); //P

    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************
  } //genCode

  void NodeBinaryOp::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    genCode(fp,uvpass);
  }

} //end MFM
