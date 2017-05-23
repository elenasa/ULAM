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
    assert(m_nodeRight->isAConstructorFunctionCall()); //based like storeintoable, on right
    return true;
  }

  bool NodeMemberSelectOnConstructorCall::isArrayItem()
  {
    return false;
  }

  EvalStatus NodeMemberSelectOnConstructorCall::eval()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    return UNEVALUABLE;

#if 0
    //maybe some day
    evalNodeProlog(0); //new current frame pointer on node eval stack

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************
    UlamValue saveCurrentSelfPtr = m_state.m_currentSelfPtr; //*************

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //UPDATE selected member (i.e. element or quark) before eval of rhs
    //(i.e. data member or func call); e.g. Ptr to atom
    UlamValue newCurrentObjectPtr = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1);
    UTI newobjtype = newCurrentObjectPtr.getUlamValueTypeIdx();
    if(!m_state.isPtr(newobjtype))
      {
	assert(m_nodeLeft->isFunctionCall()); //must be the result of a function call
	// copy anonymous class to "uc" hidden slot in STACK, then replace with a pointer to it.
	assert(m_state.isAClass(newobjtype));
	newCurrentObjectPtr = assignAnonymousClassReturnValueToStack(newCurrentObjectPtr); //t3912
      }

    u32 superid = m_state.m_pool.getIndexForDataString("super");
    if(newCurrentObjectPtr.getPtrNameId() == superid)
      {
	if(!m_nodeRight->isFunctionCall())
	  newCurrentObjectPtr = m_state.m_currentSelfPtr; //(t3749)
	else
	  m_state.m_currentSelfPtr = newCurrentObjectPtr; //changes self ********* (t3743, t3745)
      }

    m_state.m_currentObjPtr = newCurrentObjectPtr;

    u32 slot = makeRoomForNodeType(nuti);
    evs = m_nodeRight->eval(); //a Node Function Call here, or data member eval
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //assigns rhs to lhs UV pointer (handles arrays);
    //also copy result UV to stack, -1 relative to current frame pointer
    if(slot) //avoid Void's
      if(!doBinaryOperation(1, 1+slot, slot))
	evs = ERROR;

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr
    m_state.m_currentSelfPtr = saveCurrentSelfPtr; //restore current self ptr

    evalNodeEpilog();
    return evs;
#endif
  } //eval

  //for eval, want the value of the rhs
   bool NodeMemberSelectOnConstructorCall::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    return false;
  } //doBinaryOperation

  EvalStatus NodeMemberSelectOnConstructorCall::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    return UNEVALUABLE;
#if 0
    evalNodeProlog(0);

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

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

    makeRoomForSlots(1); //always 1 slot for ptr
    evs = m_nodeRight->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue ruvPtr = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(2);

    UTI robjtype = ruvPtr.getUlamValueTypeIdx(); //t3913
    if(!m_state.isPtr(robjtype))
      {
	// must be the result of a function call;
	// copy anonymous class to "uc" hidden slot in STACK, then replace with a pointer to it.
	assert(m_state.isAClass(robjtype));
	ruvPtr = assignAnonymousClassReturnValueToStack(ruvPtr);
      }

    Node::assignReturnValuePtrToStack(ruvPtr);

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr **********

    evalNodeEpilog();
    return NORMAL;
#endif
  } //evalToStoreInto


  void NodeMemberSelectOnConstructorCall::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);

    // if parent is another MS, we might need to adjust pos first;
    // elements can be data members of transients, etc. (t3968)
    UVPass luvpass;
    if(NodeMemberSelect::passalongUVPass())
      {
	luvpass = uvpass;
	Node::adjustUVPassForElements(luvpass); //t3803?
      }

    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    //NodeIdent can't do it, because it doesn't know it's not a stand-alone element.
    // here, we know there's rhs of member select, which needs to adjust to state bits.
    if(passalongUVPass()) //true
      {
	uvpass = luvpass;
	Node::adjustUVPassForElements(uvpass); //t3803?
      }

    //check the back (not front) to process multiple member selections (e.g. t3818)
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
	luvpass = uvpass; //t3584
	Node::adjustUVPassForElements(luvpass); //t3803 ?
      }

    // if parent is another MS, we might need to adjust pos first
    // elements can be data members of transients, etc.

    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    //NodeIdent can't do it, because it doesn't know it's not a stand-alone element.
    // here, we know there's rhs of member select, which needs to adjust to state bits.
    //process multiple member selections (e.g. t3817)
    UVPass ruvpass;
    if(passalongUVPass())
      {
	ruvpass = luvpass;  //t3615 ?
	Node::adjustUVPassForElements(ruvpass); //t3803
      }

    m_nodeRight->genCodeToStoreInto(fp, ruvpass); //uvpass contains the member selected, or cos obj symbol?

    uvpass = ruvpass;

    //tmp variable needed for any function call not returning a ref (t41006), including 'aref'(t41005); func calls returning a ref already made tmpvar.
    // uvpass not necessarily returning a reference type (t3913,4,5,7);
    // t41035 returns a primitive ref; t3946, t3948
    if(!m_state.isReference(uvpass.getPassTargetType()))
      {
	m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL); //dm to avoid leaks
	m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
      }
  } //genCodeToStoreInto

  bool NodeMemberSelectOnConstructorCall::passalongUVPass()
  {
    return true; //pass along
  }

} //end MFM
