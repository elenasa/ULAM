#include "NodeBinaryOpCompareLessThan.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpCompareLessThan::NodeBinaryOpCompareLessThan(Node * left, Node * right, CompilerState & state) : NodeBinaryOpCompare(left,right,state) {}

  NodeBinaryOpCompareLessThan::NodeBinaryOpCompareLessThan(const NodeBinaryOpCompareLessThan& ref) : NodeBinaryOpCompare(ref) {}

  NodeBinaryOpCompareLessThan::~NodeBinaryOpCompareLessThan(){}

  Node * NodeBinaryOpCompareLessThan::instantiate()
  {
    return new NodeBinaryOpCompareLessThan(*this);
  }

  const char * NodeBinaryOpCompareLessThan::getName()
  {
    return "<";
  }

  const std::string NodeBinaryOpCompareLessThan::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpCompareLessThan::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpCompareLessThan" << NodeBinaryOpCompare::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  UlamValue NodeBinaryOpCompareLessThan::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool
    u32 nodelen = m_state.getBitSize(nuti);

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareLessThanInt32(ldata, rdata, len), nodelen);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareLessThanUnsigned32(ldata, rdata, len), nodelen);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareLessThanBool32(ldata, rdata, len), nodelen);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareLessThanUnary32(ldata, rdata, len), nodelen);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  void NodeBinaryOpCompareLessThan::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not implemented yet!
  }

} //end MFM
