#include <stdio.h>
#include "NodeBlockLocals.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockLocals::NodeBlockLocals(NodeBlock * prevBlockNode, CompilerState & state): NodeBlock(prevBlockNode, state, NULL), m_nodeEndingStmt(this) {}

  NodeBlockLocals::NodeBlockLocals(const NodeBlockLocals& ref) : NodeBlock(ref) {}

  NodeBlockLocals::~NodeBlockLocals() {}

  Node * NodeBlockLocals::instantiate()
  {
    return new NodeBlockLocals(*this);
  }

  void NodeBlockLocals::printPostfix(File * fp)
  {
    fp->write(" locals: ");
    NodeBlock::printPostfix(fp);
  }

  const char * NodeBlockLocals::getName()
  {
    return "locals";
  }

  const std::string NodeBlockLocals::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeBlockLocals::appendNextNode(Node * node)
  {
    assert(node);
    NodeStatements * nextNode = new NodeStatements(node, m_state);
    assert(nextNode);
    nextNode->setNodeLocation(node->getNodeLocation());
    assert(m_nodeEndingStmt);
    m_nodeEndingStmt->setNextNode(nextNode);
    m_nodeEndingStmt = nextNode;
  } //appendNextNode

  UTI NodeBlockLocals::checkAndLabelType()
  {
    return NodeBlock::checkAndLabelType();
  }

  void NodeBlockLocals::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return;  //overrides NodeBlock
  }

  void NodeBlockLocals::genCode(File * fp, UVPass& uvpass)
  {
    //noop, neither named constants nor typedefs require generated C++ code.
    assert(0);
  }

} //end MFM
