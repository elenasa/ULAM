#include <stdio.h>
#include "NodeBlockSwitch.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockSwitch::NodeBlockSwitch(NodeBlock * prevBlockNode, u32 switchnum, CompilerState & state): NodeBlock(prevBlockNode, state, NULL), m_switchnum(switchnum), m_defaultcaseNodeNo(0) {}

  NodeBlockSwitch::NodeBlockSwitch(const NodeBlockSwitch& ref) : NodeBlock(ref), m_switchnum(ref.m_switchnum), m_defaultcaseNodeNo(ref.m_defaultcaseNodeNo) {}

  NodeBlockSwitch::~NodeBlockSwitch() {}

  Node * NodeBlockSwitch::instantiate()
  {
    return new NodeBlockSwitch(*this);
  }

  const std::string NodeBlockSwitch::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBlockSwitch::checkAndLabelType(Node * thisparentnode)
  {
    return NodeBlock::checkAndLabelType(thisparentnode);
    //maybe something else to help determine the conditional variable type? (t41019)
  }

  void NodeBlockSwitch::genCode(File * fp, UVPass& uvpass)
  {
    m_state.pushCurrentBlock(this);

    NodeBlock::genCode(fp, uvpass);

    m_state.popClassContext(); //restore
  }

  bool NodeBlockSwitch::isASwitchBlock()
  {
    return true;
  }

  u32 NodeBlockSwitch::getSwitchNumber()
  {
    return m_switchnum;
  }

  NNO NodeBlockSwitch::getDefaultCaseNodeNo()
  {
    return m_defaultcaseNodeNo;
  }

  void NodeBlockSwitch::setDefaultCaseNodeNo(NNO dnno)
  {
    m_defaultcaseNodeNo = dnno;
  }

} //end MFM
