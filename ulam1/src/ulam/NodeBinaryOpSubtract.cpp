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


  UlamType * NodeBinaryOpSubtract::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UlamType * leftType = m_nodeLeft->checkAndLabelType();
    UlamType * rightType = m_nodeRight->checkAndLabelType();	
    UlamType * newType = calcNodeType(leftType, rightType);

    if(newType !=  m_state.getUlamTypeByIndex(Nav))  //Nav
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
    UlamType * nut = getNodeType();
    UlamType * scalartype = m_state.getUlamTypeAsScalar(nut);

    UTI typidx = nut->getUlamTypeIndex();    
    UlamValue rtnUV;  //immediate scalar

    for(u32 i = 0; i < slots; i++)
      {
	UlamValue luv = m_state.m_nodeEvalStack.getFrameSlotAt(lslot+i);
	UlamValue ruv = m_state.m_nodeEvalStack.getFrameSlotAt(rslot+i);

	switch(typidx)
	  {
	  case Int:
	    rtnUV.init(scalartype, (luv.m_valInt - ruv.m_valInt));
	    break;
	  case Float:
	    rtnUV.init(scalartype, (luv.m_valFloat - ruv.m_valFloat));
	    break;
	  case Bool:
	  default:
	    {
	      std::ostringstream msg;
	      msg << "Invalid Type: <" << m_state.getUlamTypeNameByIndex(typidx) << "> used with binary operator" << getName();
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      rtnUV.init(m_state.getUlamTypeByIndex(Nav), 0);
	      assert(0);
	    }
	  };

	//copy result UV to stack, -1 (first array element deepest) relative to current frame pointer
	m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -slots + i);
      }
  }

} //end MFM
