#include <stdio.h>
#include "NodeBlockEmpty.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockEmpty::NodeBlockEmpty(NodeBlock * prevBlockNode, CompilerState & state): NodeBlock(prevBlockNode, state, NULL)
  {}

  NodeBlockEmpty::~NodeBlockEmpty()
  {}


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


  void NodeBlockEmpty::countNavNodes(u32& cnt)
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
