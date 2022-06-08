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
    fp->write(" -b");
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
    s32 lbs = UNKNOWNSIZE, rbs = UNKNOWNSIZE, wordsize = UNKNOWNSIZE;
    NodeBinaryOp::resultBitsizeCalc(lt, rt, lbs, rbs, wordsize);

    s32 maxbs = (lbs > rbs ? lbs : rbs);
    maxbs += 1; //for addition/subtraction
    return (maxbs >= wordsize ? wordsize : maxbs);
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
	m_state.abortUndefinedUlamPrimitiveType();
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
	m_state.abortUndefinedUlamPrimitiveType();
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
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
