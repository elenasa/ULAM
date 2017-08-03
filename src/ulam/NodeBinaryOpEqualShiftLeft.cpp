#include "NodeBinaryOpEqualShiftLeft.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualShiftLeft::NodeBinaryOpEqualShiftLeft(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualShift(left,right,state) {}

  NodeBinaryOpEqualShiftLeft::NodeBinaryOpEqualShiftLeft(const NodeBinaryOpEqualShiftLeft& ref) : NodeBinaryOpEqualShift(ref) {}

  NodeBinaryOpEqualShiftLeft::~NodeBinaryOpEqualShiftLeft(){}

  Node * NodeBinaryOpEqualShiftLeft::instantiate()
  {
    return new NodeBinaryOpEqualShiftLeft(*this);
  }

  const char * NodeBinaryOpEqualShiftLeft::getName()
  {
    return "<<=";
  }

  const std::string NodeBinaryOpEqualShiftLeft::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpEqualShiftLeft::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_ShiftOpLeft" << NodeBinaryOpEqualShift::methodNameForCodeGen();
    return methodname.str();
  }

  UlamValue NodeBinaryOpEqualShiftLeft::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftUnary32(ldata, rdata, len), len);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftBits32(ldata, rdata, len), len);
	break;
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpEqualShiftLeft::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpLeftInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpLeftUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpLeftBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpLeftUnary64(ldata, rdata, len), len);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediateLong(type, _ShiftOpLeftBits64(ldata, rdata, len), len);
	break;
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpEqualShiftLeft::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _ShiftOpLeftInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _ShiftOpLeftUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _ShiftOpLeftBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _ShiftOpLeftUnary32(ldata, rdata, len));
	break;
      case Bits:
	refUV.putData(pos, len, _ShiftOpLeftBits32(ldata, rdata, len));
	break;
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
