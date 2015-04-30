#include "NodeBinaryOpCompareGreaterThan.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpCompareGreaterThan::NodeBinaryOpCompareGreaterThan(Node * left, Node * right, CompilerState & state) : NodeBinaryOpCompare(left,right,state) {}

  NodeBinaryOpCompareGreaterThan::NodeBinaryOpCompareGreaterThan(const NodeBinaryOpCompareGreaterThan& ref) : NodeBinaryOpCompare(ref) {}

  NodeBinaryOpCompareGreaterThan::~NodeBinaryOpCompareGreaterThan(){}

  Node * NodeBinaryOpCompareGreaterThan::instantiate()
  {
    return new NodeBinaryOpCompareGreaterThan(*this);
  }

  const char * NodeBinaryOpCompareGreaterThan::getName()
  {
    return ">";
  }

  const std::string NodeBinaryOpCompareGreaterThan::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpCompareGreaterThan::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpCompareGreaterThan" << NodeBinaryOpCompare::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  UlamValue NodeBinaryOpCompareGreaterThan::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool
    u32 nodelen = m_state.getBitSize(nuti);

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareGreaterThanInt32(ldata, rdata, len), nodelen);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareGreaterThanUnsigned32(ldata, rdata, len), nodelen);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareGreaterThanBool32(ldata, rdata, len), nodelen);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareGreaterThanUnary32(ldata, rdata, len), nodelen);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  void NodeBinaryOpCompareGreaterThan::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not implemented yet!
  }

} //end MFM
