#include "NodeBinaryOpEqualBitwiseXor.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualBitwiseXor::NodeBinaryOpEqualBitwiseXor(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualBitwise(left,right,state) {}

  NodeBinaryOpEqualBitwiseXor::NodeBinaryOpEqualBitwiseXor(const NodeBinaryOpEqualBitwiseXor& ref) : NodeBinaryOpEqualBitwise(ref) {}

  NodeBinaryOpEqualBitwiseXor::~NodeBinaryOpEqualBitwiseXor(){}

  Node * NodeBinaryOpEqualBitwiseXor::instantiate()
  {
    return new NodeBinaryOpEqualBitwiseXor(*this);
  }

  const char * NodeBinaryOpEqualBitwiseXor::getName()
  {
    return "^=";
  }

  const std::string NodeBinaryOpEqualBitwiseXor::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpEqualBitwiseXor::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BitwiseXor" << NodeBinaryOpEqualBitwise::methodNameForCodeGen();
    return methodname.str();
  }

  UlamValue NodeBinaryOpEqualBitwiseXor::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseXorInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseXorUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseXorBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseXorUnary32(ldata, rdata, len), len);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseXorBits32(ldata, rdata, len), len);
	break;
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpEqualBitwiseXor::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BitwiseXorInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BitwiseXorUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BitwiseXorBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BitwiseXorUnary64(ldata, rdata, len), len);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediateLong(type, _BitwiseXorBits64(ldata, rdata, len), len);
	break;
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpEqualBitwiseXor::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BitwiseXorInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BitwiseXorUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BitwiseXorBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BitwiseXorUnary32(ldata, rdata, len));
	break;
      case Bits:
	refUV.putData(pos, len, _BitwiseXorBits32(ldata, rdata, len));
	break;
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
