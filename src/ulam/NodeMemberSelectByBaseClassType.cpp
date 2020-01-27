#include "NodeMemberSelectByBaseClassType.h"
#include "CompilerState.h"

namespace MFM {

  NodeMemberSelectByBaseClassType::NodeMemberSelectByBaseClassType(Node * left, Node * right, Node * vtrn, CompilerState & state) : NodeMemberSelect(left,right,state), m_nodeVTclassrn(vtrn), m_tmpvarSymbolVTclassrn(NULL) { }

  NodeMemberSelectByBaseClassType::NodeMemberSelectByBaseClassType(const NodeMemberSelectByBaseClassType& ref) : NodeMemberSelect(ref), m_tmpvarSymbolVTclassrn(NULL)
  {
    if(ref.m_nodeVTclassrn)
      m_nodeVTclassrn = ref.m_nodeVTclassrn->instantiate();
    else
      m_nodeVTclassrn = NULL;
  }

  NodeMemberSelectByBaseClassType::~NodeMemberSelectByBaseClassType()
  {
    delete m_nodeVTclassrn;
    m_nodeVTclassrn = NULL;
    delete m_tmpvarSymbolVTclassrn;
    m_tmpvarSymbolVTclassrn = NULL;
  }

  void NodeMemberSelectByBaseClassType::updateLineage(NNO pno)
  {
    NodeBinaryOp::updateLineage(pno);
    if(m_nodeVTclassrn)
      m_nodeVTclassrn->updateLineage(getNodeNo());
  }

  bool NodeMemberSelectByBaseClassType::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeVTclassrn == oldnptr)
      {
	m_nodeVTclassrn = newnptr;
	return true;
      }
    return NodeBinaryOp::exchangeKids(oldnptr,newnptr);
  }

  bool NodeMemberSelectByBaseClassType::findNodeNo(NNO n, Node *& foundNode)
  {
    if(m_nodeVTclassrn && m_nodeVTclassrn->findNodeNo(n, foundNode))
      return true;
    return NodeBinaryOp::findNodeNo(n, foundNode);
  } //findNodeNo

  Node * NodeMemberSelectByBaseClassType::instantiate()
  {
    return new NodeMemberSelectByBaseClassType(*this);
  }

  const char * NodeMemberSelectByBaseClassType::getName()
  {
    if(m_nodeVTclassrn)
      return ".[]";
    return ".";
  }

  void NodeMemberSelectByBaseClassType::printPostfix(File * fp)
  {
    assert(m_nodeLeft);
    m_nodeLeft->printPostfix(fp);

    fp->write(" ");
    assert(m_nodeRight);
    m_nodeRight->printPostfix(fp);

    if(m_nodeVTclassrn)
      m_nodeVTclassrn->printPostfix(fp);

    printOp(fp); //operators last
  }

  const std::string NodeMemberSelectByBaseClassType::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeMemberSelectByBaseClassType::getStorageSymbolPtr(Symbol *& symptrref)
  {
    MSG(getNodeLocationAsString().c_str(), "No storage symbol", ERR);
    return false;
  }

  bool NodeMemberSelectByBaseClassType::hasASymbolDataMember()
  {
    return false;
  }

  FORECAST NodeMemberSelectByBaseClassType::safeToCastTo(UTI newType)
  {
    return m_nodeRight->safeToCastTo(newType);
  } //safeToCastTo

  UTI NodeMemberSelectByBaseClassType::checkAndLabelType()
  {
    UTI nuti = NodeMemberSelect::checkAndLabelType();
    if(m_state.okUTItoContinue(nuti))
      {
	UTI luti = m_nodeLeft->getNodeType();
	if(m_state.okUTItoContinue(luti) && (UlamType::compareForAssignment(luti, nuti, m_state) != UTIC_SAME) && !m_state.isClassASubclassOf(luti,nuti))
	  {
	    std::ostringstream msg;
	    msg << "Selected Base Class Type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();

	    msg << " is not a direct ancestor of ";
	    msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	    msg << " and cannot be used in this context";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    nuti = Nav; //t41308
	  }

	if(m_nodeVTclassrn)
	  {
	    UTI vtuti = m_nodeVTclassrn->checkAndLabelType(); //Unsigned
	    if(!m_state.okUTItoContinue(vtuti))
	      {
		std::ostringstream msg;
		msg << "Selected Base Class Registry Number for ";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();

		msg << " is not valid with ";
		msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
		msg << " and cannot be used in this context";
		if(vtuti == Hzy)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		if(nuti != Nav)
		  nuti = vtuti; //Hzy only
	      } //else
	  }

#if 0
	// right node is type descriptor, can't test here!! BUT WHERE??
	if(!m_nodeRight->isFunctionCall())
	  {
	    std::ostringstream msg;
	    msg << "Selected Base Class Type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " is not followed by a function call, ";
	    msg << "and cannot be used in this context";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    nuti = Nav;
	  }
#endif
	setNodeType(nuti); //right type
      }

    //based on lefthand side, since Types (on right) are not? t41307
    Node::setStoreIntoAble(m_nodeLeft->getStoreIntoAble());

    //base reference-ability on leftside
    Node::setReferenceAble(m_nodeLeft->getReferenceAble());

    return getNodeType();
  } //checkAndLabelType

  TBOOL NodeMemberSelectByBaseClassType::checkStoreIntoAble()
  {
    return TBOOL_TRUE; //don't stop progress
  }

  bool NodeMemberSelectByBaseClassType::isAConstructorFunctionCall()
  {
    return false;
  }

  bool NodeMemberSelectByBaseClassType::belongsToVOWN(UTI vown)
  {
    return false; //specific base class is based on eff self pos
  }

  bool NodeMemberSelectByBaseClassType::isAMemberSelectByRegNum()
  {
    return (m_nodeVTclassrn != NULL);
  }

  bool NodeMemberSelectByBaseClassType::isArrayItem()
  {
    return false;
  }

  EvalStatus NodeMemberSelectByBaseClassType::eval()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    if(m_nodeVTclassrn)
      return evalStatusReturnNoEpilog(UNEVALUABLE); //requires MFM Registry

    if(m_nodeLeft->isAConstant())
      {
	//probably need evaltostoreinto for rhs, since not DM.
	//m_state.abortNotImplementedYet(); //t41198, t41209, t41217
	//return UNEVALUABLE;
      }

    evalNodeProlog(0); //new current frame pointer on node eval stack

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************
    UlamValue saveCurrentSelfPtr = m_state.m_currentSelfPtr; //*************

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

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

    //assigns rhs to lhs UV pointer (handles arrays?);
    //also copy result UV to stack, -1 relative to current frame pointer
    if(!doBinaryOperation(1, 1, 1))
      return evalStatusReturn(ERROR); //skip restore now, ok?

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr
    m_state.m_currentSelfPtr = saveCurrentSelfPtr; //restore current self ptr

    if(evs != NORMAL) return evalStatusReturn(evs);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  //for eval, want the value of the lhs modified to the right
  bool NodeMemberSelectByBaseClassType::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);

    //the return ptr, adjusted to the base class
    UlamValue luv = m_state.m_currentObjPtr;

    UlamValue rtnUV;
    UTI ruti = getNodeType();
    PACKFIT packFit = m_state.determinePackable(ruti);

    if(m_state.isScalar(ruti) || WritePacked(packFit))
      {
	rtnUV = luv;
      }
    else
      {
	m_state.abortShouldntGetHere(); //???
	//make a ptr to an unpacked array, base[0] ? [pls test]
	rtnUV = UlamValue::makePtr(rslot, EVALRETURN, ruti, UNPACKED, m_state);
      }

    if((rtnUV.getUlamValueTypeIdx() == Nav) || (ruti == Nav))
      return false;

    if((rtnUV.getUlamValueTypeIdx() == Hzy) || (ruti == Hzy))
      return false;

    UTI subclass = luv.getPtrTargetType();
    UTI basetype = m_nodeRight->getNodeType();
    s32 baselen = m_state.getBaseClassBitSize(basetype);
    u32 basepos = UNRELIABLEPOS;
    if(m_state.getABaseClassRelativePositionInAClass(subclass, basetype, basepos))
      {
	u32 subpos = luv.getPtrPos();
	rtnUV.setPtrPos(subpos+basepos);
	rtnUV.setPtrLen(baselen);
	rtnUV.setPtrTargetType(basetype);
	rtnUV.setPtrNameId(0);
      }
    else
      {
	return false;
      }

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(rtnUV);
    return true;
  } //doBinaryOperation

  EvalStatus NodeMemberSelectByBaseClassType::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    if(m_nodeVTclassrn)
      return evalStatusReturnNoEpilog(UNEVALUABLE); //requires MFM Registry!!

    evalNodeProlog(0);

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    //UPDATE selected member (i.e. element or quark) before eval of rhs
    // (i.e. data member or func call)
    UlamValue newCurrentObjectPtr = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1); //e.g. Ptr to atom, or uvself
    UTI newobjtype = newCurrentObjectPtr.getUlamValueTypeIdx();
    if(!m_state.isPtr(newobjtype))
      {
	assert(m_nodeLeft->isFunctionCall());// must be the result of a function call;
	// copy anonymous class to "uc" hidden slot in STACK, then replace with a pointer to it.
	assert(m_state.isAClass(newobjtype));
	newCurrentObjectPtr = assignAnonymousClassReturnValueToStack(newCurrentObjectPtr); //t3913
      }

    UlamValue ruvPtr;
    ruvPtr = newCurrentObjectPtr;

    //works like NodeIdent makeUlamValue for "manufactured super"
    UTI subclass = ruvPtr.getPtrTargetType();
    assert(!m_state.isPtr(subclass)); //was != 11
    UTI basetype = m_nodeRight->getNodeType();
    s32 baselen = m_state.getBaseClassBitSize(basetype);
    u32 basepos = UNRELIABLEPOS;
    if(m_state.getABaseClassRelativePositionInAClass(subclass, basetype, basepos))
      {
	u32 subpos = newCurrentObjectPtr.getPtrPos();
	ruvPtr.setPtrPos(subpos+basepos);
	ruvPtr.setPtrTargetType(m_state.getUlamTypeAsDeref(basetype));
	ruvPtr.setPtrLen(baselen);
	ruvPtr.setPtrNameId(0);
      }
    else
      {
	evs = ERROR;
      }

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //was newCurrentObjectPtr;

    Node::assignReturnValuePtrToStack(ruvPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  void NodeMemberSelectByBaseClassType::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);

    //check the back (not front) to process multiple member selections
    m_nodeLeft->genCode(fp, uvpass);  //leave any array item as-is for gencode.
    uvpass.setPassApplyDelta(true);

    m_state.abortNeedsATest(); //XXXXXX
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************?
  } //genCode

  // presumably called by e.g. a binary op equal (lhs); caller saves
  // currentObjPass/Symbol, unlike genCode (rhs)
  void NodeMemberSelectByBaseClassType::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    UVPass luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //uvpass contains the member selected, or cos obj symbol?

    uvpass = luvpass;

    //build tmpvar symbol for virtual function ur (t41307)
    makeUVPassForCodeGen(uvpass);
    uvpass.setPassApplyDelta(true); //always
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol); //*********UPDATED GLOBAL;

    //second tmpvar for regid in [] after the base
    if(m_nodeVTclassrn)
      {
	std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
	m_state.clearCurrentObjSymbolsForCodeGen(); //*************

	UVPass vtuvpass;
	m_nodeVTclassrn->genCode(fp,vtuvpass); //clears stack

	m_tmpvarSymbolVTclassrn = Node::makeTmpVarSymbolForCodeGen(vtuvpass, NULL);
	m_tmpvarSymbolVTclassrn->setBaseClassRegNum();

	m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore first ****
	m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbolVTclassrn); //*********UPDATED GLOBAL;
      }
  } //genCodeToStoreInto

  bool NodeMemberSelectByBaseClassType::passalongUVPass()
  {
    return true; //pass along
  }

  void NodeMemberSelectByBaseClassType::makeUVPassForCodeGen(UVPass& uvpass)
  {
    UTI subclass = uvpass.getPassTargetType();
    UTI basetype = m_nodeRight->getNodeType();
    u32 subpos = uvpass.getPassPos();
    u32 basepos = UNRELIABLEPOS;

    //compile time basepos, assuming subclass is not a ref
    bool shared = m_state.getASharedBaseClassRelativePositionInAClass(subclass, basetype, basepos);
    if(!shared)
      if(!m_state.getABaseClassRelativePositionInAClass(subclass, basetype, basepos))
	m_state.abortShouldntGetHere();

    //continue on to build tmpvarsymbol and coordinating uvpass
    u32 newpos = subpos; //subpos+basepos;

    assert(basepos <= uvpass.getPassLen()); //t41351
    uvpass.setPassPosForced(newpos); //t41310

    newpos = uvpass.getPassPos(); //update for tmp symbol

    uvpass.setPassTargetType(basetype);

    s32 tmpturnum = m_state.getNextTmpVarNumber();
    std::string tmpvarname = m_state.getUlamRefTmpVarAsString(tmpturnum);
    Token tidTok(TOK_IDENTIFIER, Node::getNodeLocation(), m_state.m_pool.getIndexForDataString(tmpvarname));

    m_tmpvarSymbol = new SymbolTmpVar(tidTok, basetype, newpos, m_state);
    assert(m_tmpvarSymbol);
    m_tmpvarSymbol->setBaseClassRef();

    uvpass.setPassVarNum(tmpturnum);
    uvpass.setPassNameId(tidTok.m_dataindex);
    uvpass.setPassPosForced(0); //reset for any following data members
  } //makeUVPassForCodeGen

} //end MFM
