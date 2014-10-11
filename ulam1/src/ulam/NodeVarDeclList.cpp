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


  UTI NodeVarDeclList::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    m_nodeLeft->checkAndLabelType();  //for side-effect
    UTI newType = m_nodeRight->checkAndLabelType();  //for side-effect

    //let it be the type of the last var decl on the right.
    //Comma Node has no type since leaves can be different array sizes
    setNodeType(newType);
    
    setStoreIntoAble(false);
    return newType;
  }


  UTI NodeVarDeclList::calcNodeType(UTI lt, UTI rt)
  {
    assert(0);
    return Nav;
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
    assert(0);  //no way to get here!
    return ERROR;
  }


  UlamValue NodeVarDeclList::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    assert(0); //unused
    return UlamValue();
  }


  void NodeVarDeclList::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert(m_nodeLeft && m_nodeRight);
    //each VarDecl is output separately, not as a list.
    m_nodeLeft->packBitsInOrderOfDeclaration(offset); //updates offset first
    m_nodeRight->packBitsInOrderOfDeclaration(offset); //updates offset next
  }


  void NodeVarDeclList::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    //each VarDecl is output separately, not as a list.
    m_nodeLeft->genCode(fp, uvpass);
    m_nodeRight->genCode(fp, uvpass);
  }

} //end MFM
