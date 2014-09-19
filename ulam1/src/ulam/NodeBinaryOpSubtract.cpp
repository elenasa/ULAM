#include "NodeBinaryOpSubtract.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpSubtract::NodeBinaryOpSubtract(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state) {}

  NodeBinaryOpSubtract::~NodeBinaryOpSubtract(){}


  void NodeBinaryOpSubtract::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %sb", getName());
    fp->write(myname);
  }


  const char * NodeBinaryOpSubtract::getName()
  {
    return "-";
  }


  const std::string NodeBinaryOpSubtract::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeBinaryOpSubtract::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();	
    UTI newType = calcNodeType(leftType, rightType);

    if(newType != Nav) //init
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
    
    // 1 cases when binary op - IS storeIntoAble: left is storeIntoAble,
    // and the other node is an Int
    //if( (m_nodeLeft && m_nodeRight) && (m_nodeLeft->isStoreIntoAble() && m_nodeRight->getNodeType() == Int))
    //  //setStoreIntoAble(true);
    //  setStoreIntoAble(false);
    //else
    setStoreIntoAble(false);
      
    return newType;
  }

  void NodeBinaryOpSubtract::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	doBinaryOperationArray(lslot, rslot, slots);
      }
  } //end dobinaryop


  UlamValue NodeBinaryOpSubtract::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    //return UlamValue::makeImmediate(type, (s32) ldata - (s32) rdata, len);
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  s32 diffof1s = leftCount1s - rightCount1s;
	  if(diffof1s > 0)
	    rtnUV = UlamValue::makeImmediate(type, _GetNOnes32(diffof1s), len);
	  else
	    rtnUV = UlamValue::makeImmediate(type, 0, len);  //???
	}
	break;
      default:
	rtnUV = UlamValue::makeImmediate(type, (s32) ldata - (s32) rdata, len);
	break;
      };
    return rtnUV;

  }


  void NodeBinaryOpSubtract::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    //refUV.putData(pos, len, (s32) ldata - (s32) rdata);
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  s32 diffOf1s = leftCount1s - rightCount1s;
	  if(diffOf1s > 0)
	    refUV.putData(pos, len, _GetNOnes32(diffOf1s));
	  else
	    refUV.putData(pos, len, 0);
	}
	break;
      default:
	refUV.putData(pos, len, (s32) ldata - (s32) rdata);
	break;
      };
    return;
  }


} //end MFM
