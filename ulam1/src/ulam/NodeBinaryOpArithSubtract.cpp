#include "NodeBinaryOpArithSubtract.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithSubtract::NodeBinaryOpArithSubtract(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}

  NodeBinaryOpArithSubtract::NodeBinaryOpArithSubtract(const NodeBinaryOpArithSubtract& ref) : NodeBinaryOpArith(ref) {}

  NodeBinaryOpArithSubtract::~NodeBinaryOpArithSubtract(){}

  Node * NodeBinaryOpArithSubtract::instantiate()
  {
    return new NodeBinaryOpArithSubtract(*this);
  }

  void NodeBinaryOpArithSubtract::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %sb", getName());
    fp->write(myname);
  }

  const char * NodeBinaryOpArithSubtract::getName()
  {
    return "-";
  }

  const std::string NodeBinaryOpArithSubtract::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  s32 NodeBinaryOpArithSubtract::resultBitsize(UTI lt, UTI rt)
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
    s32 lwordsize = (s32) lut->getTotalWordSize();
    s32 rwordsize = (s32) rut->getTotalWordSize();

    if(ltypEnum == Class)
      {
	if(lut->isNumericType()) //i.e. a quark
	  lwordsize = lbs = MAXBITSPERINT; //32
      }
    else if(ltypEnum == Unary)
      {
	lbs = (s32) _getLogBase2(lbs) + 1; //fits into unsigned
	ltypEnum = Unsigned; //for mix test
      }
    else
      assert(ltypEnum == Unsigned || ltypEnum == Int);

    if(rtypEnum == Class)
      {
	if(rut->isNumericType()) //i.e. a quark
	  rwordsize = rbs = MAXBITSPERINT; //32
      }
    else if(rtypEnum == Unary)
      {
	rbs = (s32) _getLogBase2(rbs) + 1; //fits into unsigned
	rtypEnum = Unsigned;
      }
    else
      assert(rtypEnum == Unsigned || rtypEnum == Int);

    assert(lwordsize == rwordsize);

    s32 maxbs = (lbs > rbs ? lbs : rbs);
    if(ltypEnum == rtypEnum)
      maxbs += 1;
    else
      maxbs += 2; //compensate for mixed signed and unsigned

    return (maxbs >= lwordsize ? lwordsize : maxbs);
  } //resultBitsize

  const std::string NodeBinaryOpArithSubtract::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpSubtract" << NodeBinaryOpArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  UlamValue NodeBinaryOpArithSubtract::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpArithSubtract::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpArithSubtract::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpSubtractInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpSubtractUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpSubtractBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpSubtractUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
