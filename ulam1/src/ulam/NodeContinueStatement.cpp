#include <stdio.h>
#include "NodeContinueStatement.h"
#include "CompilerState.h"

namespace MFM {

  NodeContinueStatement::NodeContinueStatement(CompilerState & state) : Node(state) {}

  NodeContinueStatement::~NodeContinueStatement()
  {}

  void NodeContinueStatement::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);
  }


  void NodeContinueStatement::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }


  UTI NodeContinueStatement::checkAndLabelType()
  {
    UTI nodeType = Nav;
    setNodeType(nodeType);
    return nodeType;
  }


  const char * NodeContinueStatement::getName()
  {
    return "continue";
  }


  const std::string NodeContinueStatement::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  EvalStatus NodeContinueStatement::eval()
  {
    return CONTINUE;
  }
  

  void NodeContinueStatement::genCode(File * fp, UlamValue& uvpass)
  {
    m_state.indent(fp);
    fp->write("continue;\n");
  } //genCode

} //end MFM
