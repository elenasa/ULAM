#include <stdio.h>
#include "NodeReturn.h"
#include "CompilerState.h"

namespace MFM {

  NodeReturn::NodeReturn(Node * s, CompilerState & state) : Node(state), m_node(s) {}

  NodeReturn::~NodeReturn()
  {
    delete m_node;
    m_node = NULL;
  }

  void NodeReturn::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UlamType * myut = getNodeType();
    char id[255];
    if(!myut)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), myut->getUlamTypeName().c_str());
    fp->write(id);

    if(m_node) 
      m_node->print(fp);
    else 
      fp->write(" <EMPTYSTMT>\n");

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }


  void NodeReturn::printPostfix(File * fp)
  {
    assert(m_node);    //e.g. bad decl

    if(m_node)
      m_node->printPostfix(fp);
    else 
      fp->write(" <EMPTYSTMT>");

    fp->write(" ");
    fp->write(getName());
  }



  UlamType * NodeReturn::checkAndLabelType()
  {
    assert(m_node);

    UlamType * nodeType = m_node->checkAndLabelType();

    if(nodeType != m_state.m_currentFunctionReturnType)
      {
	m_node = new NodeCast(m_node, m_state.m_currentFunctionReturnType, m_state);
	m_node->setNodeLocation(getNodeLocation());
	nodeType = m_node->checkAndLabelType(); //update for return node type
      }

    setNodeType(nodeType); //return take type of their node

    m_state.m_currentFunctionReturnNodes.push_back(this); //check later against defined function return type
    return nodeType;
  }


  const char * NodeReturn::getName()
  {
    return "return";
  }


  const std::string NodeReturn::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  EvalStatus NodeReturn::eval()
  {
    assert(m_node); 

    evalNodeProlog(0);
    makeRoomForNodeType(getNodeType());
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	assert(evs != CONTINUE && evs != BREAK);
	evalNodeEpilog();
	return evs;
      }

    //end, so copy to -1
    UlamValue rtnPtr(getNodeType(), 1, true, EVALRETURN);  //positive to current frame pointer
    
    assignReturnValueToStack(rtnPtr, STACK); //uses STACK, unlike all the other nodes
    evalNodeEpilog();
    return RETURN;
  }
  
  void NodeReturn::genCode(File * fp)
  {
    m_state.indent(fp);
    fp->write("return ");
    if(m_node)
      {
	fp->write("(");
	m_node->genCode(fp);
	fp->write(")");
      }
    fp->write(";\n");
  }

} //end MFM
