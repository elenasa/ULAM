#include "NodeBinaryOpBitwiseAndEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseAndEqual::NodeBinaryOpBitwiseAndEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualBitwise(left,right,state) {}

  NodeBinaryOpBitwiseAndEqual::~NodeBinaryOpBitwiseAndEqual(){}


  const char * NodeBinaryOpBitwiseAndEqual::getName()
  {
    return "&=";
  }


  const std::string NodeBinaryOpBitwiseAndEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamValue NodeBinaryOpBitwiseAndEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata & rdata, len);
  }


  void NodeBinaryOpBitwiseAndEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, ldata & rdata);
  }

} //end MFM
