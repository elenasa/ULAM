#include "NodeBinaryOpPlusEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpPlusEqual::NodeBinaryOpPlusEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpPlusEqual::~NodeBinaryOpPlusEqual(){}


  const char * NodeBinaryOpPlusEqual::getName()
  {
    return "+=";
  }


  const std::string NodeBinaryOpPlusEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  void NodeBinaryOpPlusEqual::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	doBinaryOperationArray(lslot, rslot, slots);
      }
  } //end dobinaryop

  //same as NodeBinaryOpAdd
  UlamValue NodeBinaryOpPlusEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    //    return UlamValue::makeImmediate(type, (s32) ldata + (s32) rdata, len);
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  u32 sumOf1s = leftCount1s + rightCount1s;
	  rtnUV = UlamValue::makeImmediate(type, _GetNOnes32(sumOf1s), len);
	}
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, ldata + rdata, len);
	break;
      case Bits:
	assert(0);
	break;
      default:
	rtnUV = UlamValue::makeImmediate(type, (s32) ldata + (s32) rdata, len);
	break;
      };
    return rtnUV;
  }

  //same as NodeBinaryOpAdd
  void NodeBinaryOpPlusEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    //refUV.putData(pos, len, (s32) ldata + (s32) rdata);
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  u32 sumOf1s = leftCount1s + rightCount1s;
	  refUV.putData(pos, len, _GetNOnes32(sumOf1s));
	}
	break;
      case Unsigned:
	refUV.putData(pos, len, ldata + rdata);
	break;
      case Bits:
	assert(0);
	break;
      default:
	refUV.putData(pos, len, (s32) ldata + (s32) rdata);
	break;
      };
    return;
  }

} //end MFM
