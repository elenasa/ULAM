#include "NodeBinaryOpCompareGreaterEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpCompareGreaterEqual::NodeBinaryOpCompareGreaterEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpCompare(left,right,state) {}
  NodeBinaryOpCompareGreaterEqual::NodeBinaryOpCompareGreaterEqual(const NodeBinaryOpCompareGreaterEqual& ref) : NodeBinaryOpCompare(ref) {}
  NodeBinaryOpCompareGreaterEqual::~NodeBinaryOpCompareGreaterEqual(){}

  Node * NodeBinaryOpCompareGreaterEqual::instantiate()
  {
    return new NodeBinaryOpCompareGreaterEqual(*this);
  }


  const char * NodeBinaryOpCompareGreaterEqual::getName()
  {
    return ">=";
  }


  const std::string NodeBinaryOpCompareGreaterEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpCompareGreaterEqual::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpCompareGreaterEqual" << NodeBinaryOpCompare::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen


  UlamValue NodeBinaryOpCompareGreaterEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool
    u32 nodelen = m_state.getBitSize(nuti);

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareGreaterEqualInt32(ldata, rdata, len), nodelen);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareGreaterEqualUnsigned32(ldata, rdata, len), nodelen);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareGreaterEqualBool32(ldata, rdata, len), nodelen);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareGreaterEqualUnary32(ldata, rdata, len), nodelen);
	break;
      case String:
	m_state.abortNotImplementedYet(); //not implemented yet!
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpCompareGreaterEqual::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool
    u32 nodelen = m_state.getBitSize(nuti);

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareGreaterEqualInt64(ldata, rdata, len), nodelen);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareGreaterEqualUnsigned64(ldata, rdata, len), nodelen);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareGreaterEqualBool64(ldata, rdata, len), nodelen);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareGreaterEqualUnary64(ldata, rdata, len), nodelen);
	break;
      case String:
	m_state.abortNotImplementedYet(); //not implemented yet!
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpCompareGreaterEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    m_state.abortNotImplementedYet(); //not implemented yet!
  }

} //end MFM
