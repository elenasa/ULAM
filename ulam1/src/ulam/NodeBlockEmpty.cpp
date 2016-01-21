#include <stdio.h>
#include "NodeBlockEmpty.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockEmpty::NodeBlockEmpty(NodeBlock * prevBlockNode, CompilerState & state): NodeBlock(prevBlockNode, state, NULL) {}

  NodeBlockEmpty::NodeBlockEmpty(const NodeBlockEmpty& ref) : NodeBlock(ref) {}

  NodeBlockEmpty::~NodeBlockEmpty() {}

  Node * NodeBlockEmpty::instantiate()
  {
    return new NodeBlockEmpty(*this);
  }

  void NodeBlockEmpty::print(File * fp)
  {
    printNodeLocation(fp);
    fp->write("NodeBlockEmpty<NOTYPE>\n");
    fp->write("-----------------NodeBlockEmpty\n");
  }

  void NodeBlockEmpty::printPostfix(File * fp)
  {
    fp->write(" {}");
  }

  const char * NodeBlockEmpty::getName()
  {
    return "{}";
  }

  const std::string NodeBlockEmpty::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBlockEmpty::checkAndLabelType()
  {
    setNodeType(Void);
    return getNodeType();
  }

  void NodeBlockEmpty::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return;  //overrides NodeBlock
  }

  void NodeBlockEmpty::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    return;
  }

  EvalStatus NodeBlockEmpty::eval()
  {
    return NORMAL;
  }

  void NodeBlockEmpty::genCode(File * fp, UlamValue& uvpass)
  {
    m_state.indent(fp);
    fp->write("{ /* empty */ }\n");
  }

} //end MFM
