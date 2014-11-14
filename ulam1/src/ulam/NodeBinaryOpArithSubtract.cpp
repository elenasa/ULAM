#include "NodeBinaryOpArithSubtract.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithSubtract::NodeBinaryOpArithSubtract(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}

  NodeBinaryOpArithSubtract::~NodeBinaryOpArithSubtract(){}


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
      case Bits:
      case Unary:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  }


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
      case Bits:
      case Unary:
      default:
	assert(0);
	break;
      };
    return;
  }


#if 0
  UlamValue NodeBinaryOpArithSubtract::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  s32 diffof1s = leftCount1s - rightCount1s;
	  if(diffof1s > 0)
	    rtnUV = UlamValue::makeImmediate(type, _GetNOnes32(diffof1s), len);
	  else
	    rtnUV = UlamValue::makeImmediate(type, 0, len); //least surprising
	}
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, ldata - rdata, len);
	break;
      case Bits:
	assert(0);
	break;
      default:
	rtnUV = UlamValue::makeImmediate(type, (s32) ldata - (s32) rdata, len);
	break;
      };
    return rtnUV;

  }


  void NodeBinaryOpArithSubtract::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
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
	  s32 diffOf1s = leftCount1s - rightCount1s;
	  if(diffOf1s > 0)
	    refUV.putData(pos, len, _GetNOnes32(diffOf1s));
	  else
	    refUV.putData(pos, len, 0);
	}
	break;
      case Unsigned:
	refUV.putData(pos, len, ldata - rdata);
	break;
      case Bits:
	assert(0);
	break;
      default:
	refUV.putData(pos, len, (s32) ldata - (s32) rdata);
	break;
      };
    return;
  }
#endif

} //end MFM
