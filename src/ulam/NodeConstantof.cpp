#include <stdio.h>
#include "NodeConstantof.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstantof::NodeConstantof(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeInstanceof(ofnode, nodetype, state) { }

  NodeConstantof::NodeConstantof(const NodeConstantof& ref) : NodeInstanceof(ref) { }

  NodeConstantof::~NodeConstantof() { }

  Node * NodeConstantof::instantiate()
  {
    return new NodeConstantof(*this);
  }

  const char * NodeConstantof::getName()
  {
    return ".constantof";
  }

  const std::string NodeConstantof::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeConstantof::isAConstant()
  {
    return true;
  }

  bool NodeConstantof::isAConstantClass()
  {
    return true;
  }

  bool NodeConstantof::getConstantValue(BV8K& bval)
  {
    UTI nuti = getNodeType();
    assert(m_state.okUTItoContinue(nuti));
    bool rtnok = m_state.getDefaultClassValue(nuti, bval);
    return rtnok;
  }

  void NodeConstantof::setClassType(UTI cuti)
  {
    NodeStorageof::setOfType(cuti);
  }

  bool NodeConstantof::initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask)
  {
    UTI nuti = getNodeType();

    //bvref needs default value at pos 0 of our m_forClassUTI.(t41485)
    bool rtnok = m_state.getDefaultClassValue(nuti, bvref);
    if(rtnok)
      bvmask.SetBits(0, m_state.getUlamTypeByIndex(nuti)->getSizeofUlamType()); //t41229, t41234
    return rtnok;
  }

  UTI NodeConstantof::checkAndLabelType(Node * thisparentnode)
  {
    NodeInstanceof::checkAndLabelType(thisparentnode);

    Node::setStoreIntoAble(TBOOL_FALSE);

    return getNodeType();
  } //checkAndLabelType


  UlamValue NodeConstantof::makeUlamValuePtr()
  {
    UlamValue ptr;
    UlamValue atomuv;

    UTI auti = getOfType();
    UlamType * aut = m_state.getUlamTypeByIndex(auti);
    ULAMCLASSTYPE aclasstype = aut->getUlamClassType();

    u32 atop = 1;
    atop = m_state.m_constantStack.getAbsoluteTopOfStackIndexOfNextSlot();
    u32 slotsneeded = m_state.slotsNeeded(auti);
    Node::makeRoomForSlots(slotsneeded, CNSTSTACK); //before store (t41507)

    if(m_state.isAtom(auti))
      atomuv = NodeStorageof::evalAtomOfExpr();  //t3286
    else if(aclasstype == UC_ELEMENT)
      atomuv = UlamValue::makeDefaultAtom(auti, m_state);
    else if(aclasstype == UC_QUARK)
      {
	u64 dq = 0;
	AssertBool isDefinedQuark = m_state.getDefaultQuark(auti, dq); //returns scalar dq
	assert(isDefinedQuark);
	atomuv = UlamValue::makeImmediateLongClass(auti, dq, aut->getTotalBitSize());
      }
    else if(aclasstype == UC_TRANSIENT)
      atomuv = UlamValue::makeDefaultAtom(auti, m_state); //size limited to atom for eval
    else
      m_state.abortUndefinedUlamClassType();

    m_state.m_constantStack.storeUlamValueInSlot(atomuv, atop);

    ptr = UlamValue::makePtr(atop, CNSTSTACK, auti, m_state.determinePackable(auti), m_state, 0);
    ptr.setPtrTargetEffSelfType(auti);
    ptr.setUlamValueTypeIdx(PtrAbs);
    return ptr;
  } //makeUlamValuePtr

} //end MFM
