#include "NodeBinaryOpArithMultiply.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithMultiply::NodeBinaryOpArithMultiply(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}
  NodeBinaryOpArithMultiply::NodeBinaryOpArithMultiply(const NodeBinaryOpArithMultiply& ref) : NodeBinaryOpArith(ref) {}
  NodeBinaryOpArithMultiply::~NodeBinaryOpArithMultiply() {}

  Node * NodeBinaryOpArithMultiply::clone()
  {
    return new NodeBinaryOpArithMultiply(*this);
  }

  const char * NodeBinaryOpArithMultiply::getName()
  {
    return "*";
  }


  const std::string NodeBinaryOpArithMultiply::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpArithMultiply::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpMultiply" << NodeBinaryOpArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen


  UlamValue NodeBinaryOpArithMultiply::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
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


  void NodeBinaryOpArithMultiply::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
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
