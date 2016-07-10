#include "NodeBinaryOpEqualArithMultiply.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArithMultiply::NodeBinaryOpEqualArithMultiply(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualArith(left,right,state) {}

  NodeBinaryOpEqualArithMultiply::NodeBinaryOpEqualArithMultiply(const NodeBinaryOpEqualArithMultiply& ref) : NodeBinaryOpEqualArith(ref) {}

  NodeBinaryOpEqualArithMultiply::~NodeBinaryOpEqualArithMultiply(){}

  Node * NodeBinaryOpEqualArithMultiply::instantiate()
  {
    return new NodeBinaryOpEqualArithMultiply(*this);
  }

  const char * NodeBinaryOpEqualArithMultiply::getName()
  {
    return "*=";
  }

  const std::string NodeBinaryOpEqualArithMultiply::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpEqualArithMultiply::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpMultiply" << NodeBinaryOpEqualArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  UlamValue NodeBinaryOpEqualArithMultiply::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpMultiplyInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpMultiplyUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpMultiplyBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpMultiplyUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpEqualArithMultiply::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpMultiplyInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpMultiplyUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpMultiplyBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpMultiplyUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpEqualArithMultiply::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpMultiplyInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpMultiplyUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpMultiplyBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpMultiplyUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
