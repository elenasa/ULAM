#include "NodeBinaryOpEqualShiftRight.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualShiftRight::NodeBinaryOpEqualShiftRight(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualShift(left,right,state) {}

  NodeBinaryOpEqualShiftRight::NodeBinaryOpEqualShiftRight(const NodeBinaryOpEqualShiftRight& ref) : NodeBinaryOpEqualShift(ref) {}

  NodeBinaryOpEqualShiftRight::~NodeBinaryOpEqualShiftRight(){}

  Node * NodeBinaryOpEqualShiftRight::instantiate()
  {
    return new NodeBinaryOpEqualShiftRight(*this);
  }

  const char * NodeBinaryOpEqualShiftRight::getName()
  {
    return ">>=";
  }

  const std::string NodeBinaryOpEqualShiftRight::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpEqualShiftRight::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_ShiftOpRight" << NodeBinaryOpEqualShift::methodNameForCodeGen();
    return methodname.str();
  }

  UlamValue NodeBinaryOpEqualShiftRight::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
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
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpEqualShiftRight::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
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
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpEqualShiftRight::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
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
	refUV.putData(pos, len, _ShiftOpRightBits32(ldata, rdata, len));
	break;
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
