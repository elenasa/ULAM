#include "NodeBinaryOpBitwiseOrEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseOrEqual::NodeBinaryOpBitwiseOrEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualBitwise(left,right,state) {}

  NodeBinaryOpBitwiseOrEqual::~NodeBinaryOpBitwiseOrEqual(){}


  const char * NodeBinaryOpBitwiseOrEqual::getName()
  {
    return "|=";
  }


  const std::string NodeBinaryOpBitwiseOrEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamValue NodeBinaryOpBitwiseOrEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata | rdata, len);
  }


  void NodeBinaryOpBitwiseOrEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, ldata | rdata);
  }

} //end MFM
