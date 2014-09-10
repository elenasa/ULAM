#include <stdio.h>
#include "NodeControl.h"
#include "CompilerState.h"

namespace MFM {

  NodeControl::NodeControl(Node * condNode, Node * trueNode, CompilerState & state): Node(state), m_nodeCondition(condNode), m_nodeBody(trueNode) {}

  NodeControl::~NodeControl()
  { 
    delete m_nodeCondition;
    m_nodeCondition = NULL;
    delete m_nodeBody;
    m_nodeBody = NULL;
  }


  void NodeControl::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    fp->write("condition:\n");
    if(m_nodeCondition)
      m_nodeCondition->print(fp);
    else
      fp->write("<NULLCOND>\n");

    sprintf(id,"%s:\n",getName());
    fp->write(id);

    if(m_nodeBody)
      m_nodeBody->print(fp);
    else
      fp->write("<NULLTRUE>\n");
  }


  void NodeControl::printPostfix(File * fp)
  {
    if(m_nodeCondition)
      m_nodeCondition->printPostfix(fp);
    else 
      fp->write("<NULLCONDITION>");

    fp->write(" cond");

    if(m_nodeBody)
      m_nodeBody->printPostfix(fp);
    else 
      fp->write("<NULLTRUE>");

    printOp(fp);  //operators last
  }


  void NodeControl::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }


  UTI NodeControl::checkAndLabelType()
  { 
    assert(m_nodeCondition && m_nodeBody);
    UTI newType = Bool;

    // should be bool, cast
    UTI cut = m_nodeCondition->checkAndLabelType();
    assert(m_state.isScalar(cut));

    if(cut != newType)
      {
	//std::ostringstream msg;
	//msg << " condition is not a bool, it's: " << cut->getUlamTypeName().c_str() << ", must cast!";
	//m_state->m_err.warnMsg(getNodeLocationAsString().c_str(), msg.str().c_str(), "NodeControl::checkAndLabelType");
	m_nodeCondition = new NodeCast(m_nodeCondition, newType, m_state);
	m_nodeCondition->setNodeLocation(getNodeLocation());
	m_nodeCondition->checkAndLabelType();
      }
    
    m_nodeBody->checkAndLabelType(); //side-effect
    
    setNodeType(newType);  //stays the same
    
    setStoreIntoAble(false);
    return getNodeType(); 
  }


  void NodeControl::genCode(File * fp)
  {
    assert(m_nodeCondition && m_nodeBody);
    
    m_state.indent(fp);
    fp->write(getName());
    fp->write("(");
    m_nodeCondition->genCode(fp);
    fp->write(")\n");

    m_state.m_currentIndentLevel++;
    m_nodeBody->genCode(fp);
    m_state.m_currentIndentLevel--;

  }


} //end MFM
