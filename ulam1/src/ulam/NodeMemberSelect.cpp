#include "NodeMemberSelect.h"
#include "CompilerState.h"

namespace MFM {

  NodeMemberSelect::NodeMemberSelect(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state)
  {
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeMemberSelect::NodeMemberSelect(const NodeMemberSelect& ref) : NodeBinaryOpEqual(ref) {}

  NodeMemberSelect::~NodeMemberSelect(){}

  Node * NodeMemberSelect::instantiate()
  {
    return new NodeMemberSelect(*this);
  }

  void NodeMemberSelect::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }

  const char * NodeMemberSelect::getName()
  {
    return ".";
  }

  const std::string NodeMemberSelect::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeMemberSelect::methodNameForCodeGen()
  {
    return "_MemberSelect_Stub";
  }

  FORECAST NodeMemberSelect::safeToCastTo(UTI newType)
  {
    //ulamtype checks for complete, non array, and type specific rules
    return m_nodeRight->safeToCastTo(newType);
  } //safeToCastTo

  UTI NodeMemberSelect::checkAndLabelType()
  {
    assert(m_nodeLeft && m_nodeRight);

    UTI luti = m_nodeLeft->checkAndLabelType(); //side-effect
    TBOOL lstor = m_nodeLeft->getStoreIntoAble();
    if(lstor != TBOOL_TRUE)
    //if(m_nodeLeft->isFunctionCall())
      {
	//e.g. funcCall is not storeintoable even if its return value is.
	std::ostringstream msg;
	msg << "Member selected must be a valid lefthand side: '";
	msg << m_nodeLeft->getName();
	msg << "' requires a variable; may be a casted function call";
	if(lstor == TBOOL_HAZY)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    setNodeType(Hzy);
	    m_state.setGoAgain();
	    return Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav;
	  }
      } //done

    if(!m_state.isComplete(luti))
      {
	std::ostringstream msg;
	msg << "Member selected is incomplete class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	msg << ", check and label fails this time around";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

	setNodeType(Hzy);
	m_state.setGoAgain(); //since no error msg
	return getNodeType();
      } //done

    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE classtype = lut->getUlamClass();
    if((classtype == UC_NOTACLASS) && (lut->getUlamTypeEnum() != Holder))
      {
	// must be a 'Class' type, either quark or element
	// doesn't complete checkandlabel for rhs (e.g. funccall is NULL, no eval)
	std::ostringstream msg;
	msg << "Member selected must be either a quark or an element, not type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	if(m_state.isAtom(luti))
	  msg << "; suggest using a Conditional-As";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      } //done

    std::string className = m_state.getUlamTypeNameBriefByIndex(luti); //help me debug

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(luti, csym);
    assert(isDefined);

    NodeBlockClass * memberClassNode = csym->getClassBlockNode();
    assert(memberClassNode);  //e.g. forgot the closing brace on quark definition
    //set up compiler state to use the member class block for symbol searches
    m_state.pushClassContextUsingMemberClassBlock(memberClassNode);

    UTI rightType = m_nodeRight->checkAndLabelType();

    //clear up compiler state to no longer use the member class block for symbol searches
    m_state.popClassContext();

    setNodeType(rightType);

    //based on righthand side
    Node::setStoreIntoAble(m_nodeRight->getStoreIntoAble());
    return getNodeType();
  } //checkAndLabelType

  bool NodeMemberSelect::assignClassArgValueInStubCopy()
  {
    return true; //nothing to do
  }

  bool NodeMemberSelect::isFunctionCall()
  {
    return m_nodeRight->isFunctionCall(); //based like storeintoable, on right
  }

  EvalStatus NodeMemberSelect::eval()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    evalNodeProlog(0); //new current frame pointer on node eval stack

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************

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
    assert(m_state.isPtr(newCurrentObjectPtr.getUlamValueTypeIdx()));
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
    evalNodeEpilog();
    return evs;
  } //eval

  //for eval, want the value of the rhs
  bool NodeMemberSelect::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    //the return value of a function call, or value of a data member
    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot);

    UlamValue rtnUV;
    UTI ruti = getNodeType();
    PACKFIT packFit = m_state.determinePackable(ruti);

    if(m_state.isScalar(ruti) || WritePacked(packFit))
      {
	rtnUV = ruv;
      }
    else
      {
	//make a ptr to an unpacked array, base[0] ? [pls test]
	rtnUV = UlamValue::makePtr(rslot, EVALRETURN, ruti, UNPACKED, m_state);
      }

    if((rtnUV.getUlamValueTypeIdx() == Nav) || (ruti == Nav))
      return false;

    if((rtnUV.getUlamValueTypeIdx() == Hzy) || (ruti == Hzy))
      return false;

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(rtnUV);
    return true;
  } //doBinaryOperation

  EvalStatus NodeMemberSelect::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

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
    assert(m_state.isPtr(newCurrentObjectPtr.getUlamValueTypeIdx()));
    m_state.m_currentObjPtr = newCurrentObjectPtr;

    makeRoomForSlots(1); //always 1 slot for ptr
    evs = m_nodeRight->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue ruvPtr = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(2);

    Node::assignReturnValuePtrToStack(ruvPtr);

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr **********

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeMemberSelect::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    assert(0); //unused
    return UlamValue();
  }

  UlamValue NodeMemberSelect::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    assert(0); //unused
    return UlamValue();
  }

  void NodeMemberSelect::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //unused
  }

  bool NodeMemberSelect::getSymbolPtr(Symbol *& symptrref)
  {
    if(m_nodeLeft)
      return m_nodeLeft->getSymbolPtr(symptrref);

    MSG(getNodeLocationAsString().c_str(), "No symbol", ERR);
    return false;
  } //getSymbolPtr

  void NodeMemberSelect::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    //apparently not so: assert(m_state.m_currentObjSymbolsForCodeGen.empty());

    m_nodeLeft->genCodeToStoreInto(fp, uvpass);

    if(!m_state.m_currentObjSymbolsForCodeGen.empty() && !m_state.isScalar(m_state.m_currentObjSymbolsForCodeGen[0]->getUlamTypeIdx()))
      Node::genCodeConvertATmpVarIntoAutoRef(fp, uvpass); //uvpass becomes the autoref, and clears stack

    m_nodeRight->genCode(fp, uvpass);  // is this ok?

    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************?
  } //genCode

  // presumably called by e.g. a binary op equal (lhs); caller saves
  // currentObjPtr/Symbol, unlike genCode (rhs)
  void NodeMemberSelect::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    m_nodeLeft->genCodeToStoreInto(fp, uvpass);

    if(!m_state.m_currentObjSymbolsForCodeGen.empty() && !m_state.isScalar(m_state.m_currentObjSymbolsForCodeGen[0]->getUlamTypeIdx()))
      Node::genCodeConvertATmpVarIntoAutoRef(fp, uvpass); //uvpass becomes the autoref, and clears stack

    m_nodeRight->genCodeToStoreInto(fp, uvpass); //uvpass contains the member selected, or cos obj symbol?
  } //genCodeToStoreInto

} //end MFM
