#include "NodeBinaryOpArithDivide.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithDivide::NodeBinaryOpArithDivide(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}
  NodeBinaryOpArithDivide::NodeBinaryOpArithDivide(const NodeBinaryOpArithDivide& ref) : NodeBinaryOpArith(ref) {}
  NodeBinaryOpArithDivide::~NodeBinaryOpArithDivide(){}

  Node * NodeBinaryOpArithDivide::instantiate()
  {
    return new NodeBinaryOpArithDivide(*this);
  }


  const char * NodeBinaryOpArithDivide::getName()
  {
    return "/";
  }

  const std::string NodeBinaryOpArithDivide::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpArithDivide::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpDivide" << NodeBinaryOpArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  s32 NodeBinaryOpArithDivide::resultBitsize(UTI lt, UTI rt)
  {
    UlamType * lut = m_state.getUlamTypeByIndex(lt);
    UlamType * rut = m_state.getUlamTypeByIndex(rt);

    //both sides complete to be here!!
    assert(lut->isComplete() && rut->isComplete());

    // types are either unsigned or signed (unary as unsigned)
    ULAMTYPE ltypEnum = lut->getUlamTypeEnum();
    ULAMTYPE rtypEnum = rut->getUlamTypeEnum();

    s32 lbs = lut->getBitSize();
    s32 rbs = rut->getBitSize();

    if(ltypEnum == Class)
      {
	if(lut->isNumericType()) //i.e. a quark
	  lbs = MAXBITSPERINT; //32
      }
    else if(ltypEnum == Unary)
      lbs = (s32) _getLogBase2(lbs) + 1; //fits into unsigned
    else
      assert(ltypEnum == Unsigned || ltypEnum == Int);

    if(rtypEnum == Class)
      {
	if(rut->isNumericType()) //i.e. a quark
	  rbs = MAXBITSPERINT; //32
      }
    else if(rtypEnum == Unary)
      rbs = (s32) _getLogBase2(rbs) + 1; //fits into unsigned
    else
      assert(rtypEnum == Unsigned || rtypEnum == Int);

    return (lbs > rbs ? lbs : rbs);
  } //resultBitsize

  UlamValue NodeBinaryOpArithDivide::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;

    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	return rtnUV;
      }

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpArithDivide::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;

    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	return rtnUV;
      }

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpDivideInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpDivideUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpDivideBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpDivideUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpArithDivide::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	return;
      }

    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpDivideInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpDivideUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpDivideBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpDivideUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
