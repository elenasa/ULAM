#include "NodeBinaryOpArithMultiply.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithMultiply::NodeBinaryOpArithMultiply(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}

  NodeBinaryOpArithMultiply::NodeBinaryOpArithMultiply(const NodeBinaryOpArithMultiply& ref) : NodeBinaryOpArith(ref) {}

  NodeBinaryOpArithMultiply::~NodeBinaryOpArithMultiply() {}

  Node * NodeBinaryOpArithMultiply::instantiate()
  {
    return new NodeBinaryOpArithMultiply(*this);
  }

  const char * NodeBinaryOpArithMultiply::getName()
  {
    return "*";
  }

  const std::string NodeBinaryOpArithMultiply::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  s32 NodeBinaryOpArithMultiply::resultBitsize(UTI lt, UTI rt)
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

    s32 wordsize = (s32) lut->getTotalWordSize();
    assert(wordsize == (s32) rut->getTotalWordSize());

    s32 maxbs = lbs + rbs;

    return (maxbs >= wordsize ? wordsize : maxbs);
  } //resultBitsize

  const std::string NodeBinaryOpArithMultiply::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpMultiply" << NodeBinaryOpArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  UlamValue NodeBinaryOpArithMultiply::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpMultiplyInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpMultiplyUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpMultiplyBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpMultiplyUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpArithMultiply::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpMultiplyInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpMultiplyUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpMultiplyBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpMultiplyUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpArithMultiply::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpMultiplyInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpMultiplyUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpMultiplyBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpMultiplyUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
