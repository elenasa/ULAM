#include <stdio.h>
#include "NodeConstantof.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstantof::NodeConstantof(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeInstanceof(ofnode, nodetype, state) { }

  NodeConstantof::NodeConstantof(const NodeConstantof& ref) : NodeInstanceof(ref) { }

  NodeConstantof::~NodeConstantof() { }

  Node * NodeConstantof::instantiate()
  {
    return new NodeConstantof(*this);
  }

  const char * NodeConstantof::getName()
  {
    return ".constantof";
  }

  const std::string NodeConstantof::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeConstantof::isAConstant()
  {
    return true;
  }

  bool NodeConstantof::isAConstantClass()
  {
    return true; //wrong node, though. confusing.
  }

  bool NodeConstantof::initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask)
  {
    UTI nuti = getNodeType();

    //bvref needs default value at pos 0 of our m_forClassUTI.(t41485)
    bool rtnok = m_state.getDefaultClassValue(nuti, bvref);
    if(rtnok)
      bvmask.SetBits(0, m_state.getUlamTypeByIndex(nuti)->getSizeofUlamType()); //t41229, t41234
    return rtnok;
  }

  UTI NodeConstantof::checkAndLabelType(Node * thisparentnode)
  {
    NodeInstanceof::checkAndLabelType(thisparentnode);

    Node::setStoreIntoAble(TBOOL_FALSE);
    return getNodeType();
  } //checkAndLabelType


} //end MFM
