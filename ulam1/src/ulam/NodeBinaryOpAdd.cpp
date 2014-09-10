#include "NodeBinaryOpAdd.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpAdd::NodeBinaryOpAdd(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state) {}

  NodeBinaryOpAdd::~NodeBinaryOpAdd(){}


  void NodeBinaryOpAdd::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %sb", getName());
    fp->write(myname);
  }


  const char * NodeBinaryOpAdd::getName()
  {
    return "+";
  }


  const std::string NodeBinaryOpAdd::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeBinaryOpAdd::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();	
    UTI newType = calcNodeType(leftType, rightType);
    
    if(newType != Nav)
      {
	if(newType != leftType)
	  {
	    m_nodeLeft = new NodeCast(m_nodeLeft, newType, m_state);
	    m_nodeLeft->setNodeLocation(getNodeLocation());
	    m_nodeLeft->checkAndLabelType();
	  }
	
	if(newType != rightType)
	  {
	    m_nodeRight = new NodeCast(m_nodeRight, newType, m_state);
	    m_nodeRight->setNodeLocation(getNodeLocation());
	    m_nodeRight->checkAndLabelType();
	  }
      }
    
    setNodeType(newType);
    
    // 2 cases when binary op + IS storeIntoAble: either right or left is storeIntoAble,
    // and the other node is an Int
    //    if( (m_nodeLeft && m_nodeRight) && ((m_nodeLeft->isStoreIntoAble() && m_nodeRight->getNodeType() == Int) || (m_nodeRight->isStoreIntoAble() && m_nodeLeft->getNodeType() == Int)))
    //  //setStoreIntoAble(true);
    //  setStoreIntoAble(false);
    //else
    setStoreIntoAble(false);

    return newType;
  }


  void NodeBinaryOpAdd::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	doBinaryOperationArray(lslot, rslot, slots);
      }
  } //end dobinaryop


  UlamValue NodeBinaryOpAdd::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, (s32) ldata + (s32) rdata, len);
  }


  void NodeBinaryOpAdd::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, (s32) ldata + (s32) rdata);
  }
  
} //end MFM
