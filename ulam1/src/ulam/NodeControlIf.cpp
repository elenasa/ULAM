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
    return "if";
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


  EvalStatus  NodeControlIf::eval()
  {
    assert(m_nodeCondition && m_nodeBody);
    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(getNodeType());
    EvalStatus evs = m_nodeCondition->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue cuv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
	
    if(cuv.m_valBool == false)
      {
	if(m_nodeElse)  //not necessarily
	  {
	    makeRoomForNodeType(m_nodeElse->getNodeType());
	    evs = m_nodeElse->eval();
	  }
      }
    else
      {
	makeRoomForNodeType(m_nodeBody->getNodeType());
	evs = m_nodeBody->eval();  
      }

    // the type of this node is Bool for the condition since the
    // results may be of different types/sizes. Results need to be
    // stored in a variable somewhere.

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(cuv); //skip this for a break statement ???

    evalNodeEpilog();
    return evs;
  }


  void NodeControlIf::genCode(File * fp)
  {
    NodeControl::genCode(fp);

    if(m_nodeElse)
      {
	m_state.indent(fp);
	fp->write("else\n");

	m_state.m_currentIndentLevel++;
	m_nodeElse->genCode(fp);
	m_state.m_currentIndentLevel--;
      }
  }

} //end MFM
