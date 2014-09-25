#include "NodeBinaryOpBitwiseXorEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseXorEqual::NodeBinaryOpBitwiseXorEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualBitwise(left,right,state) {}

  NodeBinaryOpBitwiseXorEqual::~NodeBinaryOpBitwiseXorEqual(){}


  const char * NodeBinaryOpBitwiseXorEqual::getName()
  {
    return "^=";
  }


  const std::string NodeBinaryOpBitwiseXorEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamValue NodeBinaryOpBitwiseXorEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata ^ rdata, len);
  }


  void NodeBinaryOpBitwiseXorEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, ldata ^ rdata);
  }

} //end MFM
