#include <stdio.h>
#include "NodeControlIf.h"
#include "CompilerState.h"

namespace MFM {

  NodeControlIf::NodeControlIf(Node * condNode, Node * trueNode, Node * falseNode, CompilerState & state): NodeControl(condNode, trueNode, state), m_nodeElse(falseNode) {}


  NodeControlIf::~NodeControlIf()
  { 
    delete m_nodeElse;
    m_nodeElse = NULL;
  }


  void NodeControlIf::print(File * fp)
  {
    NodeControl::print(fp);

    fp->write("else:\n");
    if(m_nodeElse)
      m_nodeElse->print(fp);
    else
      fp->write("<NULLFALSE>\n");

    fp->write("-----------------NodeControlif\n");
  }


  void NodeControlIf::printPostfix(File * fp)
  {
    NodeControl::printPostfix(fp);

    if(m_nodeElse)
      {
	m_nodeElse->printPostfix(fp);
	fp->write(" else");  //operators last
      }
    //else 
    //  fp->write("<NULLFALSE>");
  }


  const char * NodeControlIf::getName()
  {
    return "ifthen";
  }


  const std::string NodeControlIf::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeControlIf::checkAndLabelType()
  { 
    NodeControl::checkAndLabelType();  //does condition and true

    if(m_nodeElse)
      {
	m_nodeElse->checkAndLabelType();
      }

    return getNodeType();  //Bool
  }


  void  NodeControlIf::eval()
  {
    assert(m_nodeCondition && m_nodeBody);
    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(getNodeType());
    m_nodeCondition->eval();
    UlamValue cuv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
	
    if(cuv.m_valBool == false)
      {
	if(m_nodeElse)  //not necessarily
	  {
	    makeRoomForNodeType(m_nodeElse->getNodeType());
	    m_nodeElse->eval();
	  }
      }
    else
      {
	makeRoomForNodeType(m_nodeBody->getNodeType());
	m_nodeBody->eval();  
      }

    // the type of this node is Bool for the condition since the
    // results may be of different types/sizes. Results need to be
    // stored in a variable somewhere.

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(cuv);

    evalNodeEpilog();
  }


} //end MFM
