#include "NodeUnaryOpPlus.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpPlus::NodeUnaryOpPlus(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpPlus::~NodeUnaryOpPlus(){}


  const char * NodeUnaryOpPlus::getName()
  {
    return "+";
  }


  const std::string NodeUnaryOpPlus::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamValue NodeUnaryOpPlus::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    return UlamValue::makeImmediate(type, data, len); //no change
  }

} //end MFM
