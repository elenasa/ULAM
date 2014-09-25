#include "NodeBinaryOpBitwiseAnd.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseAnd::NodeBinaryOpBitwiseAnd(Node * left, Node * right, CompilerState & state) : NodeBinaryOpBitwise(left,right,state) {}

  NodeBinaryOpBitwiseAnd::~NodeBinaryOpBitwiseAnd(){}


  const char * NodeBinaryOpBitwiseAnd::getName()
  {
    return "&";
  }


  const std::string NodeBinaryOpBitwiseAnd::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }   
  

  UlamValue NodeBinaryOpBitwiseAnd::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata & rdata, len);
  }


  void NodeBinaryOpBitwiseAnd::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, ldata & rdata);
  }


} //end MFM
