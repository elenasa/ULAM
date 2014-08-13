#include <stdio.h>
#include "NodeStatementEmpty.h"
#include "CompilerState.h"

namespace MFM {

  NodeStatementEmpty::NodeStatementEmpty(CompilerState & state): Node(state)
  {}

  NodeStatementEmpty::~NodeStatementEmpty()
  {}


  void NodeStatementEmpty::print(File * fp)
  {
    printNodeLocation(fp);
    fp->write("NodeStatementEmpty<NOTYPE>\n");
    fp->write("-----------------NodeStatementEmpty\n");
  }


  void NodeStatementEmpty::printPostfix(File * fp)
  {
    fp->write(" ;");
  }


  void NodeStatementEmpty::printOp(File * fp)
  {}


  const char * NodeStatementEmpty::getName()
  {
    return ";";
  }


  const std::string NodeStatementEmpty::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeStatementEmpty::checkAndLabelType()
  {
    setNodeType(m_state.getUlamTypeByIndex(Void));
    return getNodeType();
  }


  EvalStatus NodeStatementEmpty::eval()
  {
    return NORMAL;
  }

} //end MFM
