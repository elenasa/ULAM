#include "NodeBinaryOpEqualArithMultiply.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArithMultiply::NodeBinaryOpEqualArithMultiply(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualArith(left,right,state) {}

  NodeBinaryOpEqualArithMultiply::~NodeBinaryOpEqualArithMultiply(){}


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
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpMultiplyUnary32(ldata, rdata, len), len);
	break;
      case Bool:
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  }


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
      case Unary:
	refUV.putData(pos, len, _BinOpMultiplyUnary32(ldata, rdata, len));
	break;
      case Bool:
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  }


#if 0
  //same as NodeBinaryOpMultiply
  UlamValue NodeBinaryOpEqualArithMultiply::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    //    return UlamValue::makeImmediate(type, (s32) ldata * (s32) rdata, len);
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  u32 prodOf1s = leftCount1s * rightCount1s;
	  rtnUV = UlamValue::makeImmediate(type, _GetNOnes32(prodOf1s), len);
	}
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, ldata * rdata, len);
	break;
      case Bits:
	assert(0);
	break;
      default:
	rtnUV = UlamValue::makeImmediate(type, (s32) ldata * (s32) rdata, len);
	break;
      };
    return rtnUV;
  }

  //same as NodeBinaryOpMultiply
  void NodeBinaryOpEqualArithMultiply::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    //refUV.putData(pos, len, (s32) ldata * (s32) rdata);
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  u32 prodOf1s = leftCount1s * rightCount1s;
	  refUV.putData(pos, len, _GetNOnes32(prodOf1s));
	}
	break;
      case Unsigned:
	refUV.putData(pos, len, ldata * rdata);
	break;
      case Bits:
	assert(0);
	break;
      default:
	refUV.putData(pos, len, (s32) ldata * (s32) rdata);
	break;
      };
    return;
  }
#endif

} //end MFM
