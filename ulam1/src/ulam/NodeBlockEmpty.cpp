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


  UlamType * NodeBlockEmpty::checkAndLabelType()
  {
    setNodeType(m_state.getUlamTypeByIndex(Void));
    return getNodeType();
  }


  EvalStatus NodeBlockEmpty::eval()
  {
    return NORMAL;
  }

} //end MFM
