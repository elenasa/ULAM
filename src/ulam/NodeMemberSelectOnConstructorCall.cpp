#include "NodeMemberSelectOnConstructorCall.h"
#include "CompilerState.h"

namespace MFM {

  NodeMemberSelectOnConstructorCall::NodeMemberSelectOnConstructorCall(Node * left, Node * right, CompilerState & state) : NodeMemberSelect(left,right,state) { }

  NodeMemberSelectOnConstructorCall::NodeMemberSelectOnConstructorCall(const NodeMemberSelectOnConstructorCall& ref) : NodeMemberSelect(ref) { }

  NodeMemberSelectOnConstructorCall::~NodeMemberSelectOnConstructorCall() { }

  Node * NodeMemberSelectOnConstructorCall::instantiate()
  {
    return new NodeMemberSelectOnConstructorCall(*this);
  }

  const char * NodeMemberSelectOnConstructorCall::getName()
  {
    return ".";
  }

  const std::string NodeMemberSelectOnConstructorCall::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeMemberSelectOnConstructorCall::getStorageSymbolPtr(Symbol *& symptrref)
  {
    MSG(getNodeLocationAsString().c_str(), "No storage symbol", ERR);
    return false;
  }

  bool NodeMemberSelectOnConstructorCall::hasASymbolDataMember()
  {
    return false;
  }

  FORECAST NodeMemberSelectOnConstructorCall::safeToCastTo(UTI newType)
  {
    //constructed such that the function call returns void; while the member selected is its class
    return m_nodeLeft->safeToCastTo(newType);
  } //safeToCastTo

  UTI NodeMemberSelectOnConstructorCall::checkAndLabelType()
  {
    UTI nuti = NodeMemberSelect::checkAndLabelType();
    if(m_state.okUTItoContinue(nuti))
      {
	UTI luti = m_nodeLeft->getNodeType();
	setNodeType(luti);

	//based on lefthand side
	Node::setStoreIntoAble(m_nodeLeft->getStoreIntoAble());

	//base reference-ability on lefthand side (t41085)
	Node::setReferenceAble(m_nodeLeft->getReferenceAble());
      }
    return getNodeType();
  } //checkAndLabelType

  bool NodeMemberSelectOnConstructorCall::isAConstructorFunctionCall()
  {
    assert(m_nodeRight->isAConstructorFunctionCall());
    return true;
  }

  bool NodeMemberSelectOnConstructorCall::isArrayItem()
  {
    return false;
  }

  //for eval, want the value of the m_currentObjPtr
   bool NodeMemberSelectOnConstructorCall::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    UlamValue rtnUV;
    UTI ruti = getNodeType();
    PACKFIT packFit = m_state.determinePackable(ruti);

    if(m_state.isScalar(ruti) || WritePacked(packFit))
      {
	rtnUV = m_state.getPtrTarget(m_state.m_currentObjPtr);
      }
    else
      {
	rtnUV = m_state.m_currentObjPtr;
      }

    if((rtnUV.getUlamValueTypeIdx() == Nav) || (ruti == Nav))
      return false;

    if((rtnUV.getUlamValueTypeIdx() == Hzy) || (ruti == Hzy))
      return false;

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(rtnUV);
    return true;
  } //doBinaryOperation

  EvalStatus NodeMemberSelectOnConstructorCall::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0);

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    //UPDATE selected member (i.e. element or quark) before eval of rhs
    // (i.e. data member or func call)
    UlamValue newCurrentObjectPtr = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1); //e.g. Ptr to atom
    UTI newobjtype = newCurrentObjectPtr.getUlamValueTypeIdx();
    if(!m_state.isPtr(newobjtype))
      {
	assert(m_nodeLeft->isFunctionCall());// must be the result of a function call;
	// copy anonymous class to "uc" hidden slot in STACK, then replace with a pointer to it.
	assert(m_state.isAClass(newobjtype));
	newCurrentObjectPtr = assignAnonymousClassReturnValueToStack(newCurrentObjectPtr); //t3913
      }

    m_state.m_currentObjPtr = newCurrentObjectPtr;

    evs = m_nodeRight->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue ruvPtr = m_state.m_currentObjPtr;
    Node::assignReturnValuePtrToStack(ruvPtr);

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr **********

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  void NodeMemberSelectOnConstructorCall::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);

    // if parent is another MS, we might need to adjust pos first;
    // elements can be data members of transients, etc.
    UVPass luvpass;
    if(NodeMemberSelect::passalongUVPass())
      {
	luvpass = uvpass;
	Node::adjustUVPassForElements(luvpass);
      }

    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    if(passalongUVPass()) //true
      {
	uvpass = luvpass;
	Node::adjustUVPassForElements(uvpass);
      }

    //check the back (not front) to process multiple member selections
    m_nodeRight->genCode(fp, uvpass);  //leave any array item as-is for gencode.

    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************?
  } //genCode

  // presumably called by e.g. a binary op equal (lhs); caller saves
  // currentObjPass/Symbol, unlike genCode (rhs)
  void NodeMemberSelectOnConstructorCall::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);

    UVPass luvpass;
    if(NodeMemberSelect::passalongUVPass())
      {
	luvpass = uvpass;
	Node::adjustUVPassForElements(luvpass);
      }

    // if parent is another MS, we might need to adjust pos first
    // elements can be data members of transients, etc.
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    UVPass ruvpass;
    if(passalongUVPass()) //true
      {
	ruvpass = luvpass;
	Node::adjustUVPassForElements(ruvpass);
      }

    m_nodeRight->genCodeToStoreInto(fp, ruvpass); //uvpass contains the member selected, or cos obj symbol?

    uvpass = ruvpass;

    //undo any element adjustment now that we are returning an object, not a ref
    if(Node::needAdjustToStateBits(ruvpass.getPassTargetType()))
      {
	u32 rpos = ruvpass.getPassPos();
	uvpass.setPassPos(rpos - ATOMFIRSTSTATEBITPOS); //t41091
      }

    //tmp variable needed for any function call not returning a ref (constructors return Void).
    m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL); //dm to avoid leaks
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
  } //genCodeToStoreInto

  bool NodeMemberSelectOnConstructorCall::passalongUVPass()
  {
    return true; //pass along
  }

} //end MFM
