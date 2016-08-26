#include <stdio.h>
#include "NodeBlockInvisible.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockInvisible::NodeBlockInvisible(NodeBlock * prevBlockNode, CompilerState & state): NodeBlock(prevBlockNode, state, NULL) {}

  NodeBlockInvisible::NodeBlockInvisible(const NodeBlockInvisible& ref) : NodeBlock(ref) {}

  NodeBlockInvisible::~NodeBlockInvisible() {}

  Node * NodeBlockInvisible::instantiate()
  {
    return new NodeBlockInvisible(*this);
  }

  void NodeBlockInvisible::print(File * fp)
  {
    printNodeLocation(fp);
    fp->write("NodeBlockInvisible<NOTYPE>\n");
    fp->write("-----------------NodeBlockInvisible\n");
  }

  void NodeBlockInvisible::printPostfix(File * fp)
  {
    if(m_nodeNext)
      m_nodeNext->printPostfix(fp);
    else
      fp->write(" <INVISIBLEBLOCK>"); //not an error
  }

  const char * NodeBlockInvisible::getName()
  {
    return "";
  }

  const std::string NodeBlockInvisible::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeBlockInvisible::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeNext);
    m_state.m_currentIndentLevel++;
    m_nodeNext->genCode(fp, uvpass);
    m_state.m_currentIndentLevel--;
  }

} //end MFM
