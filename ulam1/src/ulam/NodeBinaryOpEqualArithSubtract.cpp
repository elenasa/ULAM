#include "NodeBinaryOpEqualArithSubtract.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArithSubtract::NodeBinaryOpEqualArithSubtract(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualArith(left,right,state) {}

  NodeBinaryOpEqualArithSubtract::NodeBinaryOpEqualArithSubtract(const NodeBinaryOpEqualArithSubtract& ref) : NodeBinaryOpEqualArith(ref) {}

  NodeBinaryOpEqualArithSubtract::~NodeBinaryOpEqualArithSubtract(){}

  Node * NodeBinaryOpEqualArithSubtract::instantiate()
  {
    return new NodeBinaryOpEqualArithSubtract(*this);
  }

  const char * NodeBinaryOpEqualArithSubtract::getName()
  {
    return "-=";
  }

  const std::string NodeBinaryOpEqualArithSubtract::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpEqualArithSubtract::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpSubtract" << NodeBinaryOpEqualArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  UlamValue NodeBinaryOpEqualArithSubtract::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpEqualArithSubtract::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpEqualArithSubtract::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpSubtractInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpSubtractUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpSubtractBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpSubtractUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
