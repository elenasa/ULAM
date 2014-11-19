#include "NodeBinaryOpArithAdd.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithAdd::NodeBinaryOpArithAdd(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}

  NodeBinaryOpArithAdd::~NodeBinaryOpArithAdd(){}


  void NodeBinaryOpArithAdd::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %sb", getName());
    fp->write(myname);
  }


  const char * NodeBinaryOpArithAdd::getName()
  {
    return "+";
  }


  const std::string NodeBinaryOpArithAdd::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpArithAdd::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpAdd" << NodeBinaryOpArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen


  UlamValue NodeBinaryOpArithAdd::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
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


  void NodeBinaryOpArithAdd::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
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


#if 0
  UlamValue NodeBinaryOpArithAdd::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	assert(0);
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  u32 sumOf1s = leftCount1s + rightCount1s;
	  rtnUV = UlamValue::makeImmediate(type, _GetNOnes32(sumOf1s), len);
	}
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, ldata + rdata, len);
	break;
      case Bits:
	assert(0);
	break;
      default:
	rtnUV = UlamValue::makeImmediate(type, (s32) ldata + (s32) rdata, len);
	break;
      };
    return rtnUV;
  }


  void NodeBinaryOpArithAdd::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  u32 sumOf1s = leftCount1s + rightCount1s;
	  refUV.putData(pos, len, _GetNOnes32(sumOf1s));
	}
	break;
      case Unsigned:
	refUV.putData(pos, len, ldata + rdata);
	break;
      case Bits:
	assert(0);
	break;
      default:
	refUV.putData(pos, len, (s32) ldata + (s32) rdata);
	break;
      };
    return;
  }
#endif  
} //end MFM
