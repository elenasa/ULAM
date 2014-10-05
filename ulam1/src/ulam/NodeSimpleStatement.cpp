#include <stdio.h>
#include "NodeSimpleStatement.h"
#include "CompilerState.h"

namespace MFM {

  NodeSimpleStatement::NodeSimpleStatement(Node * s, CompilerState & state) : Node(state), m_node(s) {}

  NodeSimpleStatement::~NodeSimpleStatement()
  {
    delete m_node;
    m_node = NULL;
  }

  void NodeSimpleStatement::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    if(m_node) 
      m_node->print(fp);
    else 
      fp->write(" <EMPTYSTMT>\n");

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }


  void NodeSimpleStatement::printPostfix(File * fp)
  {
    assert(m_node);    //e.g. bad decl

    if(m_node)
      m_node->printPostfix(fp);
    else 
      fp->write(" <EMPTYSTMT>");
  }



  UTI NodeSimpleStatement::checkAndLabelType()
  {
    assert(m_node);

    m_node->checkAndLabelType();  //side-effect

    //statements don't have types 
    setNodeType(Void);
    return getNodeType();
  }


  const char * NodeSimpleStatement::getName()
  {
    return "simple";  //?
  }


  const std::string NodeSimpleStatement::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  EvalStatus NodeSimpleStatement::eval()
  {
    assert(m_node);
    evalNodeProlog(0);
    makeRoomForNodeType(m_node->getNodeType());
    EvalStatus evs = m_node->eval();

    evalNodeEpilog();
    return evs;
  }

  
  void NodeSimpleStatement::genCode(File * fp)
  {
    assert(m_node);
    m_state.indent(fp);
    m_node->genCode(fp);
    //fp->write(";\n");
  }

} //end MFM
