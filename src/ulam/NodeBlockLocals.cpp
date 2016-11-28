#include <stdio.h>
#include "NodeBlockLocals.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockLocals::NodeBlockLocals(NodeBlock * prevBlockNode, CompilerState & state): NodeBlockContext(prevBlockNode, state) {}

  NodeBlockLocals::NodeBlockLocals(const NodeBlockLocals& ref) : NodeBlockContext(ref) {}

  NodeBlockLocals::~NodeBlockLocals() {}

  Node * NodeBlockLocals::instantiate()
  {
    return new NodeBlockLocals(*this);
  }

  void NodeBlockLocals::printPostfix(File * fp)
  {
    fp->write(" locals: ");
    if(m_nodeNext)
      NodeBlock::printPostfix(fp);
  }

  const char * NodeBlockLocals::getName()
  {
    return "localdefs";
  }

  const std::string NodeBlockLocals::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeBlockLocals::isAClassBlock()
  {
    return false;
  }

  UTI NodeBlockLocals::checkAndLabelType()
  {
    UTI savnuti = getNodeType();
    assert(savnuti != Nouti);

    //possibly empty (e.g. error/t3875)
    if(m_nodeNext)
      NodeBlock::checkAndLabelType();
    setNodeType(savnuti);
    return savnuti;
  }

  void NodeBlockLocals::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return;  //overrides NodeBlock
  }

  void NodeBlockLocals::genCode(File * fp, UVPass& uvpass)
  {
    //noop, neither named constants nor typedefs require generated C++ code.
    m_state.abortShouldntGetHere();
  }

  void NodeBlockLocals::cloneAndAppendNode(std::vector<Node *> & cloneVec)
  {
    if(m_nodeNext)
      m_nodeNext->cloneAndAppendNode(cloneVec);
  }

} //end MFM
