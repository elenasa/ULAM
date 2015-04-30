#include "NodeBinaryOpBitwiseXor.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseXor::NodeBinaryOpBitwiseXor(Node * left, Node * right, CompilerState & state) : NodeBinaryOpBitwise(left,right,state) {}

  NodeBinaryOpBitwiseXor::NodeBinaryOpBitwiseXor(const NodeBinaryOpBitwiseXor& ref) : NodeBinaryOpBitwise(ref) {}

  NodeBinaryOpBitwiseXor::~NodeBinaryOpBitwiseXor(){}

  Node * NodeBinaryOpBitwiseXor::instantiate()
  {
    return new NodeBinaryOpBitwiseXor(*this);
  }

  const char * NodeBinaryOpBitwiseXor::getName()
  {
    return "^";
  }

  const std::string NodeBinaryOpBitwiseXor::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpBitwiseXor::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BitwiseXor" << NodeBinaryOpBitwise::methodNameForCodeGen();
    return methodname.str();
  }

  UlamValue NodeBinaryOpBitwiseXor::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
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
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  void NodeBinaryOpBitwiseXor::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
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
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
