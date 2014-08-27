#include "NodeBinaryOpEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqual::NodeBinaryOpEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state) {}

  NodeBinaryOpEqual::~NodeBinaryOpEqual(){}


  const char * NodeBinaryOpEqual::getName()
  {
    return "=";
  }


  const std::string NodeBinaryOpEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeBinaryOpEqual::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UlamType * newType = m_state.getUlamTypeByIndex(Nav); //init
    UlamType * leftType = m_nodeLeft->checkAndLabelType();
    UlamType * rightType = m_nodeRight->checkAndLabelType();
	
    //assert(m_nodeLeft->isStoreIntoAble());
    if(!m_nodeLeft->isStoreIntoAble())
      {
	std::ostringstream msg;
	msg << "Not storeIntoAble: <" << m_nodeLeft->getName() << ">, is type: <" << leftType->getUlamTypeName(&m_state).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(newType);
	setStoreIntoAble(false);
	return newType;  //nav
      }

    newType = leftType;

    //cast RHS if necessary
    if(newType != rightType)
      {
	m_nodeRight = new NodeCast(m_nodeRight, newType, m_state);
	m_nodeRight->setNodeLocation(getNodeLocation());
	m_nodeRight->checkAndLabelType();
      }

    setNodeType(newType);
    
    setStoreIntoAble(true);
    return newType;
  }


  EvalStatus NodeBinaryOpEqual::eval()
  {    
    assert(m_nodeLeft && m_nodeRight);
    evalNodeProlog(0); //new current frame pointer on node eval stack

    makeRoomForSlots(1); //always 1 slot for ptr

    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    u32 slot = makeRoomForNodeType(getNodeType());
    evs = m_nodeRight->eval();   //a Node Function Call here
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //assigns rhs to lhs UV pointer (handles arrays);  
    //also copy result UV to stack, -1 relative to current frame pointer    
    doBinaryOperation(1, 2, slot);

    evalNodeEpilog();
    return NORMAL;
  }


  EvalStatus NodeBinaryOpEqual::evalToStoreInto()
  {
    //assert(0);
    evalNodeProlog(0);

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();  
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue luvPtr(getNodeType(), 1, true, EVALRETURN);  //positive to current frame pointer

    assignReturnValuePtrToStack(luvPtr);
    
    evalNodeEpilog();
    return NORMAL;
  }


  void NodeBinaryOpEqual::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {    
    UlamValue pluv = m_state.m_nodeEvalStack.getFrameSlotAt(lslot);
    UlamValue ruvPtr(getNodeType(), rslot, true, EVALRETURN);  //positive to current frame pointer
    assignUlamValue(pluv,ruvPtr);

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(ruvPtr);
  }


  void NodeBinaryOpEqual::assignUlamValue(UlamValue pluv, UlamValue ruv)
  {
    if(pluv.m_storage == ATOM)
      {
	m_state.m_selectedAtom.assignUlamValue(pluv, ruv, m_state);
      }
    else if (pluv.m_storage == STACK)
      {
	m_state.m_funcCallStack.assignUlamValue(pluv, ruv, m_state);
      }
    else if (pluv.m_storage == EVALRETURN)
      {
	m_state.m_nodeEvalStack.assignUlamValue(pluv, ruv, m_state);
      }
    else
      {
	assert(0);
	// error! luv not storeintoable
      }
  }


} //end MFM
