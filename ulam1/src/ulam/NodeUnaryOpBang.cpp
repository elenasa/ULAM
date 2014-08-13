#include "NodeUnaryOpBang.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpBang::NodeUnaryOpBang(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpBang::~NodeUnaryOpBang(){}


  const char * NodeUnaryOpBang::getName()
  {
    return "!";
  }


  const std::string NodeUnaryOpBang::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeUnaryOpBang::checkAndLabelType()
  { 
    assert(m_node);

    UlamType * but = m_state.getUlamTypeByIndex(Bool);
    UlamType * ut  = m_node->checkAndLabelType(); 
    
    assert(ut->isScalar());

    if(ut != but)
      {
	m_node = new NodeCast(m_node, but, m_state);
	m_node->setNodeLocation(getNodeLocation());
	m_node->checkAndLabelType();
      }
    
    setNodeType(but);
    setStoreIntoAble(false);

    return getNodeType();
  }


  void NodeUnaryOpBang::doUnaryOperation(u32 slot, u32 nslots)
  {
    UlamType * nut = getNodeType();
    
    for(u32 i = 0; i < nslots; i++)
      {
	UlamValue uv = m_state.m_nodeEvalStack.getFrameSlotAt(slot+i); //immediate scalar
	
	//assumes bool; invert uv.
	uv.init(nut, !uv.m_valBool);
	
	//copy result UV to stack, -1 (first array element deepest) relative to current frame pointer
	m_state.m_nodeEvalStack.storeUlamValueInSlot(uv, -nslots + i);	
      }
  }
  
} //end MFM
