#include <stdio.h>
#include "NodeBlockSwitch.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockSwitch::NodeBlockSwitch(NodeBlock * prevBlockNode, u32 switchnum, CompilerState & state): NodeBlock(prevBlockNode, state, NULL), m_switchnum(switchnum) {}

  NodeBlockSwitch::NodeBlockSwitch(const NodeBlockSwitch& ref) : NodeBlock(ref), m_switchnum(ref.m_switchnum) {}

  NodeBlockSwitch::~NodeBlockSwitch() {}

  Node * NodeBlockSwitch::instantiate()
  {
    return new NodeBlockSwitch(*this);
  }

  const std::string NodeBlockSwitch::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBlockSwitch::checkAndLabelType()
  {
    return NodeBlock::checkAndLabelType();
    //maybe something else to help determine the conditional variable type? (t41019)
  }

  bool NodeBlockSwitch::isASwitchBlock()
  {
    return true;
  }

  u32 NodeBlockSwitch::getSwitchNumber()
  {
    return m_switchnum;
  }

} //end MFM
