#include <stdio.h>
#include "NodeUnaryOp.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOp::NodeUnaryOp(Node * n, CompilerState & state): Node(state), m_node(n) {}

  NodeUnaryOp::~NodeUnaryOp()
  { 
    delete m_node;
    m_node = NULL;
  }


  void NodeUnaryOp::print(File * fp)
  {
    printNodeLocation(fp);
    UlamType * myut = getNodeType();
    char id[255];
    if(!myut)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), myut->getUlamTypeName().c_str());
    fp->write(id);
    
    if(m_node)
      m_node->print(fp);
    else 
      fp->write("<NULL>\n");
    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }
  
  
  void NodeUnaryOp::printPostfix(File * fp)
  {
    if(m_node)
      m_node->printPostfix(fp);
    else 
      fp->write("<NULL>");
    
    printOp(fp);  //operators last
  }
  

  void NodeUnaryOp::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }
  

  UlamType * NodeUnaryOp::checkAndLabelType()
  { 
    assert(m_node);

    UlamType * ut = m_node->checkAndLabelType();
    
    if(ut == m_state.getUlamTypeByIndex(Bool))  //if array???
      {
	UlamType * iut = m_state.getUlamTypeByIndex(Int);
	m_node = new NodeCast(m_node, iut, m_state);  //except for Bang
	m_node->setNodeLocation(getNodeLocation());
	m_node->checkAndLabelType();
	setNodeType(iut);
      }
    else
      {
	if(ut->isScalar())
	  setNodeType(ut);  //stays the same
	else
	  {
	    std::ostringstream msg;
	    msg << "Incompatible (nonscalar) type: <" << ut->getUlamTypeName().c_str() << "> for unary operator" << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    
	    setNodeType(m_state.getUlamTypeByIndex(Nav));
	  }
      }

    setStoreIntoAble(false);

    return getNodeType(); 
  }


  EvalStatus NodeUnaryOp::eval()
  {
    assert(m_node);

    UlamType * nut = getNodeType();
    evalNodeProlog(0); //new current frame pointer
    u32 slots = makeRoomForNodeType(nut);
    EvalStatus evs = m_node->eval();
    
    if(evs == NORMAL)
      doUnaryOperation(1,slots);

    evalNodeEpilog();
    return evs;
  }

  void NodeUnaryOp::genCode(File * fp)
  {
    assert(m_node);
    printOp(fp); 
    m_node->genCode(fp);
  }

} //end MFM
