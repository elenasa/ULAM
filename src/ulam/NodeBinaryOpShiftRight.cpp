#include "NodeBinaryOpShiftRight.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpShiftRight::NodeBinaryOpShiftRight(Node * left, Node * right, CompilerState & state) : NodeBinaryOpShift(left,right,state) {}

  NodeBinaryOpShiftRight::NodeBinaryOpShiftRight(const NodeBinaryOpShiftRight& ref) : NodeBinaryOpShift(ref) {}

  NodeBinaryOpShiftRight::~NodeBinaryOpShiftRight(){}

  Node * NodeBinaryOpShiftRight::instantiate()
  {
    return new NodeBinaryOpShiftRight(*this);
  }

  const char * NodeBinaryOpShiftRight::getName()
  {
    return ">>";
  }

  const std::string NodeBinaryOpShiftRight::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpShiftRight::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_ShiftOpRight" << NodeBinaryOpShift::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  s32 NodeBinaryOpShiftRight::resultBitsize(UTI lt, UTI rt)
  {
    s32 lbs = UNKNOWNSIZE, rbs = UNKNOWNSIZE, wordsize = UNKNOWNSIZE;
    NodeBinaryOp::resultBitsizeCalcInBits(lt, rt, lbs, rbs, wordsize);

    return lbs;
  } //resultBitsize

  UlamValue NodeBinaryOpShiftRight::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); //left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpRightInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpRightUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpRightBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpRightUnary32(ldata, rdata, len), len);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpRightBits32(ldata, rdata, len), len);
	break;
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpShiftRight::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); //left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpRightInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpRightUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpRightBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpRightUnary64(ldata, rdata, len), len);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpRightBits64(ldata, rdata, len), len);
	break;
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpShiftRight::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    m_state.abortNotImplementedYet(); //not implemented yet!
#if 0
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _ShiftOpRightInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _ShiftOpRightUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _ShiftOpRightBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _ShiftOpRightUnary32(ldata, rdata, len));
	break;
      case Bits:
      case Unary:
	refUV.putData(pos, len, _ShiftOpRightBits32(ldata, rdata, len));
	break;
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
#endif
    return;
  } //appendBinaryOp

} //end MFM
