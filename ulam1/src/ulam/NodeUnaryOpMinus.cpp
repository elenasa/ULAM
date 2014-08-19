#include "NodeUnaryOpMinus.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpMinus::NodeUnaryOpMinus(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpMinus::~NodeUnaryOpMinus(){}


  const char * NodeUnaryOpMinus::getName()
  {
    return "-";
  }


  const std::string NodeUnaryOpMinus::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }



  void NodeUnaryOpMinus::doUnaryOperation(u32 slot, u32 nslots)
  {
    UlamType * nut = getNodeType();
    UlamType * scalartype = m_state.getUlamTypeAsScalar(nut);
    UTI typidx = m_state.getUlamTypeIndex(scalartype);
    
    for(u32 i = 0; i < nslots; i++)
      {
	UlamValue uv = m_state.m_nodeEvalStack.getFrameSlotAt(slot+i); //immediate scalar
	
	switch(typidx)
	  {
	  case Int:
	    uv.m_valInt = - uv.m_valInt;
	    break;
	  case Float:
	    uv.m_valFloat = - uv.m_valFloat;
	    break;
	  case Bool:
	    // cast to an Int.
	  default:
	    break;
	  };
	
	//copy result UV to stack, -1 (first array element deepest) relative to current frame pointer
	m_state.m_nodeEvalStack.storeUlamValueInSlot(uv, -nslots + i);	
      }
  }


} //end MFM
