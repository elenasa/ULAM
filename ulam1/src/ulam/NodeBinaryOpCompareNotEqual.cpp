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
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  void NodeBinaryOpCompareNotEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not implemented yet!
  }

} //end MFM
