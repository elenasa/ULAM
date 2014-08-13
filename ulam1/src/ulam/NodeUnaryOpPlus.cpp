#include "NodeUnaryOpPlus.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpPlus::NodeUnaryOpPlus(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpPlus::~NodeUnaryOpPlus(){}


  const char * NodeUnaryOpPlus::getName()
  {
    return "+";
  }


  const std::string NodeUnaryOpPlus::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  void NodeUnaryOpPlus::doUnaryOperation(u32 slot, u32 nslots)
  {
    for(u32 i = 0; i < nslots; i++)
      {
	UlamValue uv = m_state.m_nodeEvalStack.getFrameSlotAt(slot+i); //immediate scalar
	
	//no change to uv.

	//copy result UV to stack, -1 (first array element deepest) relative to current frame pointer
	m_state.m_nodeEvalStack.storeUlamValueInSlot(uv, -nslots + i);	
      }
  }

} //end MFM
