#include <stdio.h>
#include "NodeStatementEmpty.h"
#include "CompilerState.h"

namespace MFM {

  NodeStatementEmpty::NodeStatementEmpty(CompilerState & state): Node(state) {}

  NodeStatementEmpty::NodeStatementEmpty(const NodeStatementEmpty& ref) : Node(ref) {}

  NodeStatementEmpty::~NodeStatementEmpty() {}

  Node * NodeStatementEmpty::instantiate()
  {
    return new NodeStatementEmpty(*this);
  }

  void NodeStatementEmpty::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
  } //updateLineage

  bool NodeStatementEmpty::findNodeNo(NNO n, Node *& foundNode)
  {
    return Node::findNodeNo(n, foundNode);
  } //findNodeNo

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

  UTI NodeStatementEmpty::checkAndLabelType()
  {
    setNodeType(Void);
    return getNodeType();
  }

  EvalStatus NodeStatementEmpty::eval()
  {
    return NORMAL;
  }

  void NodeStatementEmpty::genCode(File * fp, UlamValue& uvpass)
  {
    return;
  }

} //end MFM
