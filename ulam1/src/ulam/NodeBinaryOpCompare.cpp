#include <stdio.h>
#include "NodeBinaryOpCompare.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpCompare::NodeBinaryOpCompare(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpCompare::NodeBinaryOpCompare(const NodeBinaryOpCompare& ref) : NodeBinaryOp(ref) {}

  NodeBinaryOpCompare::~NodeBinaryOpCompare() {}

  UTI NodeBinaryOpCompare::checkAndLabelType()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();

    UTI newType = calcNodeType(leftType, rightType); //for casting
    if(m_state.isComplete(newType))
      {
	u32 errCnt = 0;
	if(UlamType::compare(rightType, newType, m_state) != UTIC_SAME)
	  {
	    if(!Node::makeCastingNode(m_nodeRight, newType, m_nodeRight))
	      errCnt++;
	  }

	if(UlamType::compare(leftType, newType, m_state) != UTIC_SAME)
	  {
	    if(!Node::makeCastingNode(m_nodeLeft, newType, m_nodeLeft))
	      errCnt++;
	  }

	if(errCnt)
	  newType = Nav;
	else
	  newType = Bool; //always Bool (default size) for node; after castings!
      }
    setNodeType(newType);
    setStoreIntoAble(false);

    //still may need casting (e.g. unary compared to an int) before constantfolding
    if((newType != Nav) && isAConstant() && m_nodeLeft->isReadyConstant() && m_nodeRight->isReadyConstant())
      return NodeBinaryOp::constantFold();

    return newType;
  } //checkAndLabelType

  //same as arith rules for relative comparisons.
  UTI NodeBinaryOpCompare::calcNodeType(UTI lt, UTI rt)
  {
    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
      return Hzy;

    //no atoms, elements nor void as either operand
    if(!NodeBinaryOp::checkForPrimitiveTypes(lt, rt))
      return Nav;

    // only int, unsigned, unary types; not bool, bits, etc..
    if(!NodeBinaryOp::checkForNumericTypes(lt, rt))
      return Nav; //err output

    UTI newType = Nav; //init
    // all operations are performed as Int(32) or Unsigned(32) in CastOps.h
    // if one is unsigned, and the other isn't -> output error if unsafe;
    // Signed Int wins, unless its a constant.
    // Class (i.e. quark) + anything goes to Int(32)
    if(checkScalarTypesOnly(lt, rt))
      {
	s32 newbs = NodeBinaryOp::resultBitsize(lt, rt);
	ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

	// treat Unary using Unsigned rules
	if(ltypEnum == Unary)
	  ltypEnum = Unsigned;

	if(rtypEnum == Unary)
	  rtypEnum = Unsigned;

	if(ltypEnum == Unsigned && rtypEnum == Unsigned)
	  {
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Unsigned"), newbs);
	    newType = m_state.makeUlamType(newkey, Unsigned);
	  }
	else
	  {
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Int"), newbs);
	    newType = m_state.makeUlamType(newkey, Int);
	  }

	if(!NodeBinaryOp::checkSafeToCastTo(newType))
	  newType = Nav; //outputs error msg
      } //both scalars
    return newType;
  } //calcNodeType

  const std::string NodeBinaryOpCompare::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_BitwiseOr";  determined by each op
    UlamType * lut = m_state.getUlamTypeByIndex(m_nodeLeft->getNodeType());

    // common part of name
    ULAMTYPE etyp = lut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	methodname << "Int";
	break;
      case Unsigned:
	methodname << "Unsigned";
	break;
      case Bits:
	methodname << "Bits";
	break;
      case Bool:
	methodname << "Bool";
	break;
      default:
	assert(0);
	methodname << "NAV";
	break;
      };
    methodname << lut->getTotalWordSize();
    return methodname.str();
  } // methodNameForCodeGen

  bool NodeBinaryOpCompare::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType())) //not an array
      {
	return doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
#ifdef SUPPORT_ARITHMETIC_ARRAY_OPS
	return doBinaryOperationArray(lslot, rslot, slots);
#else
	assert(0);
#endif //defined below...
      }
    return false;
  } //end dobinaryop

  //unlike NodeBinaryOp, NodeBinaryOpCompare has a node type that's different from
  // its nodes, where left and right nodes are casted to be the same.
  bool NodeBinaryOpCompare::doBinaryOperationImmediate(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots == 1);
    UTI luti = m_nodeLeft->getNodeType();
    u32 len = m_state.getTotalBitSize(luti);

    UlamValue luv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(lslot); //immediate value
    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot); //immediate value
    if((luv.getUlamValueTypeIdx() == Nav) || (ruv.getUlamValueTypeIdx() == Nav))
      return false;

    if((luv.getUlamValueTypeIdx() == Hzy) || (ruv.getUlamValueTypeIdx() == Hzy))
      return false;

    UlamValue rtnUV;
    u32 wordsize = m_state.getTotalWordSize(luti);
    if(wordsize == MAXBITSPERINT)
      {
	u32 ldata = luv.getImmediateData(len, m_state);
	u32 rdata = ruv.getImmediateData(len, m_state);
	rtnUV = makeImmediateBinaryOp(luti, ldata, rdata, len);
      }
    else if(wordsize == MAXBITSPERLONG)
      {
	u64 ldata = luv.getImmediateDataLong(len);
	u64 rdata = ruv.getImmediateDataLong(len);
	rtnUV = makeImmediateLongBinaryOp(luti, ldata, rdata, len);
      }
    else
      assert(0); //e.g. 0

    if(rtnUV.getUlamValueTypeIdx() == Nav)
      return false;

    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);
    return true;
  } //dobinaryopImmediate

  //unlike NodeBinaryOp, NodeBinaryOpCompare has a node type that's different from
  // its nodes, where left and right nodes are casted to be the same.
  bool NodeBinaryOpCompare::doBinaryOperationArray(s32 lslot, s32 rslot, u32 slots)
  {
    assert(0); //not implemented yet..TODO return bool.
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool, same array size as lhs/rhs

    UTI luti = m_nodeLeft->getNodeType();
    UTI ruti = m_nodeRight->getNodeType();
    s32 arraysize = m_state.getArraySize(luti);

    assert(arraysize == m_state.getArraySize(nuti)); //node is same array size as lhs/rhs

    s32 bitsize = m_state.getBitSize(luti);
    UTI scalartypidx = m_state.getUlamTypeAsScalar(luti);
    PACKFIT packRtn = m_state.determinePackable(nuti);

    if(WritePacked(packRtn))
      {
	// pack result too. (slot size known ahead of time)
	rtnUV = UlamValue::makeAtom(nuti); //accumulate result here
      }

    // point to base array slots, packedness determines its 'pos'
    UlamValue lArrayPtr = UlamValue::makePtr(lslot, EVALRETURN, luti, packRtn, m_state);
    UlamValue rArrayPtr = UlamValue::makePtr(rslot, EVALRETURN, ruti, packRtn, m_state);

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
	  //use calc position where base [0] is furthest from the end.
	  appendBinaryOp(rtnUV, ldata, rdata, (BITSPERATOM-(bitsize * (arraysize - i))), bitsize);
	else
	  {
	    rtnUV = makeImmediateBinaryOp(scalartypidx, ldata, rdata, bitsize);

	    //cp result UV to stack, -1 (first array element deepest) relative to current frame pointer
	    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -slots + i);
	  }
	AssertBool isNextLeft = lp.incrementPtr(m_state);
	assert(isNextLeft);
	AssertBool isNextRight = rp.incrementPtr(m_state);
	assert(isNextRight);
      } //forloop

    if(WritePacked(packRtn))
      m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1); //store accumulated packed result

    return false;
  } //end dobinaryoparray

  void NodeBinaryOpCompare::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************

    // generate rhs first; may update current object globals (e.g. function call)
    UlamValue ruvpass;
    m_nodeRight->genCode(fp, ruvpass);

    // restore current object globals
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
    luti = luvpass.getPtrTargetType(); //reset
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");

    UTI ruti = ruvpass.getUlamValueTypeIdx();
    assert(ruti == Ptr);
    ruti = ruvpass.getPtrTargetType(); //reset
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");

    //compare needs size of left/right nodes (only difference!)
    fp->write_decimal(m_state.getUlamTypeByIndex(luti)->getTotalBitSize());

    fp->write(");\n");

    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0);  //P

    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************
  } //genCode

} //end MFM
