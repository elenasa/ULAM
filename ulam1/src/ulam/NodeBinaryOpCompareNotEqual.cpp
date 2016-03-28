#include "NodeBinaryOpCompareNotEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpCompareNotEqual::NodeBinaryOpCompareNotEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpCompare(left,right,state) {}

  NodeBinaryOpCompareNotEqual::NodeBinaryOpCompareNotEqual(const NodeBinaryOpCompareNotEqual& ref) : NodeBinaryOpCompare(ref) {}

  NodeBinaryOpCompareNotEqual::~NodeBinaryOpCompareNotEqual(){}

  Node * NodeBinaryOpCompareNotEqual::instantiate()
  {
    return new NodeBinaryOpCompareNotEqual(*this);
  }

  const char * NodeBinaryOpCompareNotEqual::getName()
  {
    return "!=";
  }

  const std::string NodeBinaryOpCompareNotEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpCompareNotEqual::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpCompareNotEq" << NodeBinaryOpCompare::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  UTI NodeBinaryOpCompareNotEqual::calcNodeType(UTI lt, UTI rt)
  {
    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
      return Hzy; //short-circuit

    UTI newType = Nav; //init
    ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
    ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

    //if either is Bits, cast to Bits
    if(ltypEnum == Bits || rtypEnum == Bits)
      {
	if(NodeBinaryOp::checkScalarTypesOnly(lt, rt))
	  {
	    s32 lbs = UNKNOWNSIZE, rbs = UNKNOWNSIZE, wordsize = UNKNOWNSIZE;
	    NodeBinaryOp::resultBitsizeCalcInBits(lt, rt, lbs, rbs, wordsize);
	    s32 newbs = (lbs > rbs ? lbs : rbs);

	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bits"), newbs);
	    newType = m_state.makeUlamType(newkey, Bits);

	    checkSafeToCastTo(getNodeType(), newType); //Nav, Hzy or no change; outputs error msg
	  }
	return newType; //done
      }

    //if both are Bool, cast to Bool
    if(ltypEnum == Bool && rtypEnum == Bool)
      {
	if(NodeBinaryOp::checkScalarTypesOnly(lt, rt))
	  {
	    newType = Bool;
	    checkSafeToCastTo(getNodeType(), newType); //Nav, Hzy or no change; outputs error msg
	  }
	return newType; //done
      }

    //o.w. revert to ordered comparison rules
    return NodeBinaryOpCompare::calcNodeType(lt,rt);
  } //calcNodeType

  UlamValue NodeBinaryOpCompareNotEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool
    u32 nodelen = m_state.getBitSize(nuti);

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareNotEqInt32(ldata, rdata, len), nodelen);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareNotEqUnsigned32(ldata, rdata, len), nodelen);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareNotEqBool32(ldata, rdata, len), nodelen);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareNotEqUnary32(ldata, rdata, len), nodelen);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareNotEqBits32(ldata, rdata, len), nodelen);
	break;
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpCompareNotEqual::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool
    u32 nodelen = m_state.getBitSize(nuti);

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareNotEqInt64(ldata, rdata, len), nodelen);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareNotEqUnsigned64(ldata, rdata, len), nodelen);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareNotEqBool64(ldata, rdata, len), nodelen);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareNotEqUnary64(ldata, rdata, len), nodelen);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareNotEqBits64(ldata, rdata, len), nodelen);
	break;
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpCompareNotEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not implemented yet!
  }

} //end MFM
