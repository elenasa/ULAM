#include "NodeBinaryOpEqualBitwiseXor.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualBitwiseXor::NodeBinaryOpEqualBitwiseXor(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualBitwise(left,right,state) {}

  NodeBinaryOpEqualBitwiseXor::~NodeBinaryOpEqualBitwiseXor(){}


  const char * NodeBinaryOpEqualBitwiseXor::getName()
  {
    return "^=";
  }


  const std::string NodeBinaryOpEqualBitwiseXor::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpEqualBitwiseXor::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BitwiseXor" << NodeBinaryOpEqualBitwise::methodNameForCodeGen();
    return methodname.str();
  }


  UlamValue NodeBinaryOpEqualBitwiseXor::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata ^ rdata, len);
  }


  void NodeBinaryOpEqualBitwiseXor::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, ldata ^ rdata);
  }

} //end MFM
