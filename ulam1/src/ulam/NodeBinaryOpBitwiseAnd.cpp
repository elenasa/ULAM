#include "NodeBinaryOpBitwiseAnd.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseAnd::NodeBinaryOpBitwiseAnd(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state) {}

  NodeBinaryOpBitwiseAnd::~NodeBinaryOpBitwiseAnd(){}


  const char * NodeBinaryOpBitwiseAnd::getName()
  {
    return "&";
  }


  const std::string NodeBinaryOpBitwiseAnd::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeBinaryOpBitwiseAnd::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();	
    UTI newType = calcNodeTypeBitwise(leftType, rightType);

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
    setStoreIntoAble(false);
    return newType;
  }

   
  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  void NodeBinaryOpBitwiseAnd::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	// leverage case when both are packed, for logical bitwise operations
	if(m_state.determinePackable(nuti) == PACKEDLOADABLE)
	  {
	    doBinaryOperationImmediate(lslot, rslot, slots);
	  }
	else
	  { 
	    doBinaryOperationArray(lslot, rslot, slots);
	  }
      }
  } //end dobinaryop
  

  UlamValue NodeBinaryOpBitwiseAnd::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata & rdata, len);
  }


  void NodeBinaryOpBitwiseAnd::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    //assert(0); //not used, though could be
    refUV.putData(pos, len, ldata & rdata);
  }


} //end MFM
