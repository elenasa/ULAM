#include "NodeBinaryOpArithRemainder.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithRemainder::NodeBinaryOpArithRemainder(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}
  NodeBinaryOpArithRemainder::NodeBinaryOpArithRemainder(const NodeBinaryOpArithRemainder& ref) : NodeBinaryOpArith(ref) {}

  NodeBinaryOpArithRemainder::~NodeBinaryOpArithRemainder(){}

  Node * NodeBinaryOpArithRemainder::instantiate()
  {
    return new NodeBinaryOpArithRemainder(*this);
  }

  const char * NodeBinaryOpArithRemainder::getName()
  {
    return "%";
  }


  const std::string NodeBinaryOpArithRemainder::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpArithRemainder::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpMod" << NodeBinaryOpArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen


  UlamValue NodeBinaryOpArithRemainder::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;

    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Division By Zero Attempt in Modulus", ERR);
	return rtnUV;
      }

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpModInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpModUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpModBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpModUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp


  void NodeBinaryOpArithRemainder::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Remainder By Zero Attempt", ERR);
	return;
      }

    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpModInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpModUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpModBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpModUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
