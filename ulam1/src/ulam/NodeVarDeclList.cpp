#include "NodeVarDeclList.h"
#include "NodeTerminalIdent.h"
#include "CompilerState.h"

namespace MFM {

  NodeVarDeclList::NodeVarDeclList(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state) {}

  NodeVarDeclList::~NodeVarDeclList(){}


  void NodeVarDeclList::printOp(File * fp)
  {
    //fp->write(", ");
  }


  const char * NodeVarDeclList::getName()
  {
    return ",";
  }


  const std::string NodeVarDeclList::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeVarDeclList::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    m_nodeLeft->checkAndLabelType();  //for side-effect
    UlamType * newType = m_nodeRight->checkAndLabelType();  //for side-effect

    //let it be the type of the last var decl on the right.
    //Comma Node has no type since leaves can be different array sizes
    setNodeType(newType);
    
    setStoreIntoAble(false);
    return newType;
  }


  EvalStatus NodeVarDeclList::eval()
  {   
    assert(m_nodeLeft && m_nodeRight);
    //evalNodeProlog(0); //new current frame pointer
    //
    //makeRoomForNodeType(m_nodeLeft->getNodeType()); 
    //m_nodeLeft->eval();
    //m_state.m_nodeEvalStack.popArg();
    //
    //makeRoomForNodeType(getNodeType()); 
    //m_nodeRight->eval();  
    //UlamValue ruvPtr(getNodeType(), 1, true, EVALRETURN); //positive to current frame pointer

    //copy result UV to stack, -1 relative to current frame pointer
    ////Node::assignReturnValueToStack(ruvPtr);  //convention is to return the right
    
    //evalNodeEpilog();
    return NORMAL;
  }


  EvalStatus NodeVarDeclList::evalToStoreInto()
  {
    assert(m_nodeLeft && m_nodeRight);

    evalNodeProlog(0); //new current frame pointer

    u32 slots = makeRoomForSlots(1); 
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    makeRoomForSlots(1);
    evs = m_nodeRight->evalToStoreInto();  
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue ruvPtr(getNodeType(), slots + 1, true, EVALRETURN); //positive to current frame pointer

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(ruvPtr);  //convention is to return the right
    
    evalNodeEpilog();
    return NORMAL;
  }


  void NodeVarDeclList::genCode(File * fp)
  {
    assert(m_nodeLeft && m_nodeRight);
    //each VarDecl is output separately, not as a list.
    m_nodeLeft->genCode(fp);
    m_nodeRight->genCode(fp);
  }

} //end MFM
