#include "NodeBinaryOpEqualBitwiseOr.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualBitwiseOr::NodeBinaryOpEqualBitwiseOr(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualBitwise(left,right,state) {}

  NodeBinaryOpEqualBitwiseOr::~NodeBinaryOpEqualBitwiseOr(){}


  const char * NodeBinaryOpEqualBitwiseOr::getName()
  {
    return "|=";
  }


  const std::string NodeBinaryOpEqualBitwiseOr::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpEqualBitwiseOr::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BitwiseOr" << NodeBinaryOpEqualBitwise::methodNameForCodeGen();
    return methodname.str();
  }


  UlamValue NodeBinaryOpEqualBitwiseOr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
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
      case Bits:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseOrBits32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseOrUnary32(ldata, rdata, len), len);
	break;
      default:
	assert(0);
	break;
      };
    return rtnUV;
  }


  void NodeBinaryOpEqualBitwiseOr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
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
      case Bits:
	refUV.putData(pos, len, _BitwiseOrBits32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BitwiseOrUnary32(ldata, rdata, len));
	break;
      default:
	assert(0);
	break;
      };
    return;
  }


#if 0
  UlamValue NodeBinaryOpEqualBitwiseOr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata | rdata, len);
  }


  void NodeBinaryOpEqualBitwiseOr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, ldata | rdata);
  }
#endif

} //end MFM
