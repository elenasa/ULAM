#include "NodeBinaryOpMultiplyEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpMultiplyEqual::NodeBinaryOpMultiplyEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpMultiplyEqual::~NodeBinaryOpMultiplyEqual(){}


  const char * NodeBinaryOpMultiplyEqual::getName()
  {
    return "*=";
  }


  const std::string NodeBinaryOpMultiplyEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  void NodeBinaryOpMultiplyEqual::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {    
    s32 plslot = lslot;
    UlamValue pluv = m_state.m_nodeEvalStack.getFrameSlotAt(plslot);

    UlamType * nut = getNodeType();    

    UlamValue ruvPtr(nut, rslot, true, EVALRETURN);  //positive to current frame pointer

    //to avoid knowing lhs storage place, just push on the node eval stack
    makeRoomForNodeType(nut);
    lslot = 1 + slots + 1; 
    m_nodeLeft->eval();  
    UlamValue luvPtr(nut, lslot, true, EVALRETURN);  //positive to current frame pointer


    //continue with the binary operation

    UlamType * scalartype = m_state.getUlamTypeAsScalar(nut);
    UTI typidx = scalartype->getUlamTypeIndex();    
    UlamValue rtnUV;  //immediate scalar, tmp

    for(u32 i = 0; i < slots; i++)
      {
	UlamValue luv = m_state.m_nodeEvalStack.getFrameSlotAt(lslot+i);
	UlamValue ruv = m_state.m_nodeEvalStack.getFrameSlotAt(rslot+i);

	switch(typidx)
	  {
	  case Int:
	    rtnUV.init(scalartype, (luv.m_valInt * ruv.m_valInt));
	    break;
	  case Float:
	    rtnUV.init(scalartype, (luv.m_valFloat * ruv.m_valFloat));
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

	//overwrite lhs copy with result UV 
	// (could possibly avoid this step if we used luv instead of rtnUV ?)
	m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, lslot + i); 
      }

    assignUlamValue(pluv,luvPtr);

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(luvPtr);
  }

} //end MFM
