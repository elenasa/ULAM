#include <stdio.h>
#include "NodeStatements.h"
#include "CompilerState.h"

namespace MFM {

  NodeStatements::NodeStatements(Node * s, CompilerState & state) : Node(state), m_node(s), m_nextNode(NULL) {}

  NodeStatements::~NodeStatements()
  {
    delete m_nextNode;
    m_nextNode = NULL;

    delete m_node;
    m_node = NULL;
  }


  void NodeStatements::print(File * fp)
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

    if(m_nextNode) 
      m_nextNode->print(fp);
    else 
      fp->write(" <NONEXTSTMT>\n");
    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }


  void NodeStatements::printPostfix(File * fp)
  {
    assert(m_node);    //e.g. bad decl

    if(m_node)
      m_node->printPostfix(fp);
    else 
      fp->write(" <EMPTYSTMT>");

    if(m_nextNode)
      m_nextNode->printPostfix(fp);
  }



  UlamType * NodeStatements::checkAndLabelType()
  {
    assert(m_node);

    //unlike statements, blocks don't have an m_node
    m_node->checkAndLabelType();       //side-effect

    if(m_nextNode)
      m_nextNode->checkAndLabelType(); //side-effect

    //statements don't have types 
    setNodeType(m_state.getUlamTypeByIndex(Void));
    return getNodeType();
  }


  const char * NodeStatements::getName()
  {
    return "Stmts";  //?
  }


  const std::string NodeStatements::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  EvalStatus NodeStatements::eval()
  {
    assert(m_node); 

    evalNodeProlog(0);
    makeRoomForNodeType(m_node->getNodeType());
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //not the last one, so thrown out results and continue
    if(m_nextNode)
      {
	evalNodeEpilog();
	evs = m_nextNode->eval();
	if(evs != NORMAL)
	  {
	    ////evalNodeEpilog();
	    return evs;
	  }
      }
    else
      {
	//end, so copy to -1
	//	UlamValue rtnPtr(getNodeType(), 1, true, EVALRETURN);  //positive to current frame pointer
	//assignReturnValueToStack(rtnPtr);
	evalNodeEpilog();
      }

    return NORMAL;
  }



  void NodeStatements::setNextNode(NodeStatements * s)
  {
    m_nextNode = s;
  }


  void NodeStatements::genCode(File * fp)
  {
    m_node->genCode(fp);

    if(m_nextNode)
      m_nextNode->genCode(fp);
  }

} //end MFM
