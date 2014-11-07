#include "NodeBinaryOpBitwiseOr.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseOr::NodeBinaryOpBitwiseOr(Node * left, Node * right, CompilerState & state) : NodeBinaryOpBitwise(left,right,state) {}

  NodeBinaryOpBitwiseOr::~NodeBinaryOpBitwiseOr(){}


  const char * NodeBinaryOpBitwiseOr::getName()
  {
    return "|";
  }


  const std::string NodeBinaryOpBitwiseOr::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpBitwiseOr::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BitwiseOr" << NodeBinaryOpBitwise::methodNameForCodeGen();
    return methodname.str();
  }


  UlamValue NodeBinaryOpBitwiseOr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata | rdata, len);
  }


  void NodeBinaryOpBitwiseOr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not used, though could be
    refUV.putData(pos, len, ldata | rdata);
  }

} //end MFM
