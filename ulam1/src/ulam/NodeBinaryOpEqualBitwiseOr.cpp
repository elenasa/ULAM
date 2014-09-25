#include "NodeBinaryOpEqualBitwiseOr.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualBitwiseOr::NodeBinaryOpEqualBitwiseOr(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualBitwise(left,right,state) {}

  NodeBinaryOpEqualBitwiseOr::~NodeBinaryOpEqualBitwiseOr(){}


  const char * NodeBinaryOpEqualBitwiseOr::getName()
  {
    return "|=";
  }


  const std::string NodeBinaryOpEqualBitwiseOr::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamValue NodeBinaryOpEqualBitwiseOr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata | rdata, len);
  }


  void NodeBinaryOpEqualBitwiseOr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, ldata | rdata);
  }

} //end MFM
