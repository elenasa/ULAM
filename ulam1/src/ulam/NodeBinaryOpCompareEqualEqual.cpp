#include "NodeBinaryOpCompareEqualEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpCompareEqualEqual::NodeBinaryOpCompareEqualEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpCompare(left,right,state) {}
  NodeBinaryOpCompareEqualEqual::NodeBinaryOpCompareEqualEqual(const NodeBinaryOpCompareEqualEqual& ref) : NodeBinaryOpCompare(ref) {}
  NodeBinaryOpCompareEqualEqual::~NodeBinaryOpCompareEqualEqual(){}

  Node * NodeBinaryOpCompareEqualEqual::instantiate()
  {
    return new NodeBinaryOpCompareEqualEqual(*this);
  }



  const char * NodeBinaryOpCompareEqualEqual::getName()
  {
    return "==";
  }


  const std::string NodeBinaryOpCompareEqualEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpCompareEqualEqual::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpCompareEqEq" << NodeBinaryOpCompare::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen


  UlamValue NodeBinaryOpCompareEqualEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool
    u32 nodelen = m_state.getBitSize(nuti);

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareEqEqInt32(ldata, rdata, len), nodelen);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareEqEqUnsigned32(ldata, rdata, len), nodelen);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareEqEqBool32(ldata, rdata, len), nodelen);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpCompareEqEqUnary32(ldata, rdata, len), nodelen);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpCompareEqualEqual::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool
    u32 nodelen = m_state.getBitSize(nuti);

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareEqEqInt64(ldata, rdata, len), nodelen);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareEqEqUnsigned64(ldata, rdata, len), nodelen);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareEqEqBool64(ldata, rdata, len), nodelen);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(nuti, _BinOpCompareEqEqUnary64(ldata, rdata, len), nodelen);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  void NodeBinaryOpCompareEqualEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not implemented yet!
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpCompareEqEqInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpCompareEqEqUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpCompareEqEqBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpCompareEqEqUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  }

} //end MFM
