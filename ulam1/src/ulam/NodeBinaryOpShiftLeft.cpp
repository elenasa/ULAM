#include "NodeBinaryOpShiftLeft.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpShiftLeft::NodeBinaryOpShiftLeft(Node * left, Node * right, CompilerState & state) : NodeBinaryOpShift(left,right,state) {}
  NodeBinaryOpShiftLeft::NodeBinaryOpShiftLeft(const NodeBinaryOpShiftLeft& ref) : NodeBinaryOpShift(ref) {}
  NodeBinaryOpShiftLeft::~NodeBinaryOpShiftLeft(){}

  Node * NodeBinaryOpShiftLeft::instantiate()
  {
    return new NodeBinaryOpShiftLeft(*this);
  }


  const char * NodeBinaryOpShiftLeft::getName()
  {
    return "<<";
  }


  const std::string NodeBinaryOpShiftLeft::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpShiftLeft::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_ShiftOpLeft" << NodeBinaryOpShift::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen



  UlamValue NodeBinaryOpShiftLeft::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftUnary32(ldata, rdata, len), len);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediate(type, _ShiftOpLeftBits32(ldata, rdata, len), len);
	break;
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp


  void NodeBinaryOpShiftLeft::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not implemented yet!
#if 0
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _ShiftOpLeftInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _ShiftOpLeftUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _ShiftOpLeftBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _ShiftOpLeftUnary32(ldata, rdata, len));
	break;
      case Bits:
      case Unary:
	refUV.putData(pos, len, _ShiftOpLeftBits32(ldata, rdata, len));
	break;
      default:
	assert(0);
	break;
      };
#endif
    return;
  }


} //end MFM
