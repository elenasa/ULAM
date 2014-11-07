#include "NodeBinaryOpBitwiseXor.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseXor::NodeBinaryOpBitwiseXor(Node * left, Node * right, CompilerState & state) : NodeBinaryOpBitwise(left,right,state) {}

  NodeBinaryOpBitwiseXor::~NodeBinaryOpBitwiseXor(){}


  const char * NodeBinaryOpBitwiseXor::getName()
  {
    return "^";
  }


  const std::string NodeBinaryOpBitwiseXor::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpBitwiseXor::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BitwiseXor" << NodeBinaryOpBitwise::methodNameForCodeGen();
    return methodname.str();
  }


  UlamValue NodeBinaryOpBitwiseXor::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata ^ rdata, len);
  }


  void NodeBinaryOpBitwiseXor::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, ldata ^ rdata);
  }

} //end MFM
