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

    //NodeIdent can't do it, because it doesn't know it's not a stand-alone element.
    // here, we know there's rhs of member select, which needs to adjust to state bits.
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

    //NodeIdent can't do it, because it doesn't know it's not a stand-alone element.
    // here, we know there's rhs of member select, which needs to adjust to state bits.
    //process multiple member selections (e.g. t3817)
    UVPass ruvpass;
    if(passalongUVPass()) //true
      {
	ruvpass = luvpass;
	Node::adjustUVPassForElements(ruvpass);
      }

    m_nodeRight->genCodeToStoreInto(fp, ruvpass); //uvpass contains the member selected, or cos obj symbol?

    uvpass = ruvpass;

    //tmp variable needed for any function call not returning a ref
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
