#include <stdio.h>
#include "NodeBreakStatement.h"
#include "CompilerState.h"

namespace MFM {

  NodeBreakStatement::NodeBreakStatement(CompilerState & state) : Node(state) {}

  NodeBreakStatement::NodeBreakStatement(const NodeBreakStatement& ref) : Node(ref) {}

  NodeBreakStatement::~NodeBreakStatement() {}

  Node * NodeBreakStatement::instantiate()
  {
    return new NodeBreakStatement(*this);
  }

  void NodeBreakStatement::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);
  } //print

  void NodeBreakStatement::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }

  const char * NodeBreakStatement::getName()
  {
    return "break";
  }

  const std::string NodeBreakStatement::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBreakStatement::checkAndLabelType()
  {
    UTI nodeType = Void;
    setNodeType(nodeType);
    return nodeType;
  }

  EvalStatus NodeBreakStatement::eval()
  {
    return BREAK;
  }

  void NodeBreakStatement::genCode(File * fp, UlamValue& uvpass)
  {
    m_state.indent(fp);
    fp->write("break;\n");
  } //genCode

} //end MFM
