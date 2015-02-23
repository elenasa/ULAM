#include "NodeBinaryOpBitwiseOr.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseOr::NodeBinaryOpBitwiseOr(Node * left, Node * right, CompilerState & state) : NodeBinaryOpBitwise(left,right,state) {}
  NodeBinaryOpBitwiseOr::NodeBinaryOpBitwiseOr(const NodeBinaryOpBitwiseOr& ref) : NodeBinaryOpBitwise(ref) {}
  NodeBinaryOpBitwiseOr::~NodeBinaryOpBitwiseOr(){}

  Node * NodeBinaryOpBitwiseOr::instantiate()
  {
    return new NodeBinaryOpBitwiseOr(*this);
  }


  const char * NodeBinaryOpBitwiseOr::getName()
  {
    return "|";
  }


  const std::string NodeBinaryOpBitwiseOr::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpBitwiseOr::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BitwiseOr" << NodeBinaryOpBitwise::methodNameForCodeGen();
    return methodname.str();
  }


  UlamValue NodeBinaryOpBitwiseOr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseOrInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseOrUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseOrBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseOrUnary32(ldata, rdata, len), len);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseOrBits32(ldata, rdata, len), len);
	break;
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp


  void NodeBinaryOpBitwiseOr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BitwiseOrInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BitwiseOrUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BitwiseOrBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BitwiseOrUnary32(ldata, rdata, len));
	break;
      case Bits:
	refUV.putData(pos, len, _BitwiseOrBits32(ldata, rdata, len));
	break;
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
