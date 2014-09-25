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
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
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


  UTI NodeUnaryOp::checkAndLabelType()
  { 
    assert(m_node);
    UTI ut = m_node->checkAndLabelType();
    UTI newType = ut;         // init to stay the same
    
    if(!m_state.isScalar(ut)) //array unsupported at this time
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) type: <" << m_state.getUlamTypeNameByIndex(ut).c_str() << "> for unary operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);	
	newType = Nav;
      }
    else
      {
	ULAMTYPE eut = m_state.getUlamTypeByIndex(ut)->getUlamTypeEnum();
	if(eut == Bool)
	  {
	    newType = Int;
	    m_node = makeCastingNode(m_node, newType);  //insert node/s
	  }
      }

    setNodeType(newType);

    setStoreIntoAble(false);

    return newType; 
  } //checkAndLabelType

  
#if 0
  UTI NodeUnaryOp::CHECKANDLABELTYPE()
  { 
    assert(m_node);

    UTI ut = m_node->checkAndLabelType();
    
    if(ut == Bool)  //if array?, unsupported at this time
      {
	UTI iut = Int;
	m_node = new NodeCast(m_node, iut, m_state);  //except for Bang
	m_node->setNodeLocation(getNodeLocation());
	m_node->checkAndLabelType();
	setNodeType(iut);
      }
    else
      {
	if(m_state.isScalar(ut))
	  setNodeType(ut);  //stays the same
	else
	  {
	    std::ostringstream msg;
	    msg << "Incompatible (nonscalar) type: <" << m_state.getUlamTypeNameByIndex(ut).c_str() << "> for unary operator" << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    
	    setNodeType(Nav);
	  }
      }

    setStoreIntoAble(false);

    return getNodeType(); 
  }
#endif


  EvalStatus NodeUnaryOp::eval()
  {
    assert(m_node);

    UTI nut = getNodeType();
    evalNodeProlog(0); //new current frame pointer
    u32 slots = makeRoomForNodeType(nut);
    EvalStatus evs = m_node->eval();
    
    if(evs == NORMAL)
      doUnaryOperation(1,slots);

    evalNodeEpilog();
    return evs;
  }


  void NodeUnaryOp::doUnaryOperation(s32 slot, u32 nslots)
  {
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doUnaryOperationImmediate(slot, nslots);
      }
    else
      { //arrays not supported at this time
	assert(0); 
      }
  } //end dobinaryop


  void NodeUnaryOp::doUnaryOperationImmediate(s32 slot, u32 nslots)
  {
    assert(nslots == 1);
    UTI nuti = getNodeType();
    u32 len = m_state.getTotalBitSize(nuti);

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slot); //immediate value                  
    u32 data = uv.getImmediateData(len);
    UlamValue rtnUV = makeImmediateUnaryOp(nuti, data, len);
    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);
  } //end dounaryopImmediate


  void NodeUnaryOp::genCode(File * fp)
  {
    assert(m_node);
    printOp(fp); 
    m_node->genCode(fp);
  }

} //end MFM
