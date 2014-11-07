#include "NodeBinaryOpArithMultiply.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithMultiply::NodeBinaryOpArithMultiply(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}

  NodeBinaryOpArithMultiply::~NodeBinaryOpArithMultiply(){}


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


  void NodeBinaryOpArithMultiply::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
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

} //end MFM
