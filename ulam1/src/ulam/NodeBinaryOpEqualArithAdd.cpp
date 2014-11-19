#include "NodeBinaryOpEqualArithAdd.h"
#include "NodeBinaryOpArithAdd.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArithAdd::NodeBinaryOpEqualArithAdd(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualArith(left,right,state) {}

  NodeBinaryOpEqualArithAdd::~NodeBinaryOpEqualArithAdd(){}


  const char * NodeBinaryOpEqualArithAdd::getName()
  {
    return "+=";
  }


  const std::string NodeBinaryOpEqualArithAdd::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpEqualArithAdd::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpAdd" << NodeBinaryOpEqualArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen


  void NodeBinaryOpEqualArithAdd::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
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


  UlamValue NodeBinaryOpEqualArithAdd::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  }


  void NodeBinaryOpEqualArithAdd::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpAddInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpAddUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpAddBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpAddUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  }

} //end MFM
