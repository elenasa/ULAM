#include "NodeBinaryOpDivide.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpDivide::NodeBinaryOpDivide(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state) {}

  NodeBinaryOpDivide::~NodeBinaryOpDivide(){}


  const char * NodeBinaryOpDivide::getName()
  {
    return "/";
  }


  const std::string NodeBinaryOpDivide::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeBinaryOpDivide::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();    
    UTI newType = calcNodeType(leftType, rightType);
    
    if((newType != Nav) && (newType != Bool))
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
    setStoreIntoAble(false);

    return newType;
  }
  

  void NodeBinaryOpDivide::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
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


  UlamValue NodeBinaryOpDivide::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    s32 quotient = 0;
    if( (s32) rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
      }
    else
      {
	quotient = (s32) ldata/ (s32) rdata;
      }

    return UlamValue::makeImmediate(type, quotient, len);
  }


  void NodeBinaryOpDivide::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    s32 quotient = 0;
    if( (s32) rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
      }
    else
      {
	quotient = (s32) ldata/ (s32) rdata;
      }

    refUV.putData(pos, len, quotient);
  }

} //end MFM
