#include "NodeBinaryOpEqualBitwiseAnd.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualBitwiseAnd::NodeBinaryOpEqualBitwiseAnd(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualBitwise(left,right,state) {}
  NodeBinaryOpEqualBitwiseAnd::NodeBinaryOpEqualBitwiseAnd(const NodeBinaryOpEqualBitwiseAnd& ref) : NodeBinaryOpEqualBitwise(ref) {}
  NodeBinaryOpEqualBitwiseAnd::~NodeBinaryOpEqualBitwiseAnd(){}

  Node * NodeBinaryOpEqualBitwiseAnd::clone()
  {
    return new NodeBinaryOpEqualBitwiseAnd(*this);
  }


  const char * NodeBinaryOpEqualBitwiseAnd::getName()
  {
    return "&=";
  }


  const std::string NodeBinaryOpEqualBitwiseAnd::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpEqualBitwiseAnd::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BitwiseAnd" << NodeBinaryOpEqualBitwise::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen


  UlamValue NodeBinaryOpEqualBitwiseAnd::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseAndInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseAndUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseAndBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseAndUnary32(ldata, rdata, len), len);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediate(type, _BitwiseAndBits32(ldata, rdata, len), len);
	break;
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp


  void NodeBinaryOpEqualBitwiseAnd::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BitwiseAndInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BitwiseAndUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BitwiseAndBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BitwiseAndUnary32(ldata, rdata, len));
	break;
      case Bits:
	refUV.putData(pos, len, _BitwiseAndBits32(ldata, rdata, len));
	break;
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
