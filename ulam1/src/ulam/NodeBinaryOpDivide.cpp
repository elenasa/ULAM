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


  UlamType * NodeBinaryOpDivide::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UlamType * leftType = m_nodeLeft->checkAndLabelType();
    UlamType * rightType = m_nodeRight->checkAndLabelType();    
    UlamType * newType = calcNodeType(leftType, rightType);
    
    if((newType != m_state.getUlamTypeByIndex(Nav)) && (newType != m_state.getUlamTypeByIndex(Bool)))
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
    UlamType * nut = getNodeType();
    UlamType * scalartype = m_state.getUlamTypeAsScalar(nut);

    UTI typidx = scalartype->getUlamTypeIndex();    
    UlamValue rtnUV;  //immediate scalar

    for(u32 i = 0; i < slots; i++)
      {
	UlamValue luv = m_state.m_nodeEvalStack.getFrameSlotAt(lslot+i);
	UlamValue ruv = m_state.m_nodeEvalStack.getFrameSlotAt(rslot+i);

	//test for non-zero
	if(! ruv.isZero())
	  {
	    switch(typidx)
	      {
	      case Int:
		rtnUV.init(scalartype, (luv.m_valInt / ruv.m_valInt));
		break;
	      case Float:
		rtnUV.init(scalartype, (luv.m_valFloat / ruv.m_valFloat));
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
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", DEBUG);
	    rtnUV.init(m_state.getUlamTypeByIndex(Nav), 0);
	  }

	//copy result UV to stack, -1 (first array element deepest) relative to current frame pointer
	m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -slots + i);
      } //next slot    
  }


} //end MFM
