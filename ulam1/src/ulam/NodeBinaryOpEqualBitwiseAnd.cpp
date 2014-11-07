#include "NodeBinaryOpEqualBitwiseAnd.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualBitwiseAnd::NodeBinaryOpEqualBitwiseAnd(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualBitwise(left,right,state) {}

  NodeBinaryOpEqualBitwiseAnd::~NodeBinaryOpEqualBitwiseAnd(){}


  const char * NodeBinaryOpEqualBitwiseAnd::getName()
  {
    return "&=";
  }


  const std::string NodeBinaryOpEqualBitwiseAnd::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpEqualBitwiseAnd::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BitwiseAnd" << NodeBinaryOpEqualBitwise::methodNameForCodeGen();
    return methodname.str();
  }


  UlamValue NodeBinaryOpEqualBitwiseAnd::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata & rdata, len);
  }


  void NodeBinaryOpEqualBitwiseAnd::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, ldata & rdata);
  }

} //end MFM
