#include "NodeMemberSelect.h"
#include "SymbolVariableDataMember.h"
#include "CompilerState.h"

namespace MFM {

  NodeMemberSelect::NodeMemberSelect(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state), m_tmpvarSymbol(NULL)
  {
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeMemberSelect::NodeMemberSelect(const NodeMemberSelect& ref) : NodeBinaryOpEqual(ref), m_tmpvarSymbol(NULL) {}

  NodeMemberSelect::~NodeMemberSelect()
  {
    delete m_tmpvarSymbol;
    m_tmpvarSymbol = NULL;
  }

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

  bool NodeMemberSelect::getSymbolPtr(Symbol *& symptrref)
  {
    if(m_nodeRight)
      return m_nodeRight->getSymbolPtr(symptrref);

    MSG(getNodeLocationAsString().c_str(), "No symbol", ERR);
    return false;
  }

  bool NodeMemberSelect::getStorageSymbolPtr(Symbol *& symptrref)
  {
    if(m_nodeLeft)
      return m_nodeLeft->getSymbolPtr(symptrref); //includes quarks, transients

    MSG(getNodeLocationAsString().c_str(), "No storage symbol", ERR);
    return false;
  }

  bool NodeMemberSelect::hasASymbolDataMember()
  {
    return true;
  }

  bool NodeMemberSelect::hasASymbolSuper()
  {
    assert(m_nodeLeft);
    return m_nodeLeft->hasASymbolSuper();
  }

  bool NodeMemberSelect::hasASymbolSelf()
  {
    assert(m_nodeLeft);
    return m_nodeLeft->hasASymbolSelf();
  }

  bool NodeMemberSelect::hasASymbolReference()
  {
    assert(m_nodeLeft);
    return m_nodeLeft->hasASymbolReference();
  }

  bool NodeMemberSelect::hasASymbolReferenceConstant()
  {
    assert(hasASymbolReference());
    return m_nodeLeft->hasASymbolReferenceConstant();
  }

  bool NodeMemberSelect::belongsToVOWN(UTI vown)
  {
    assert(m_nodeLeft && m_nodeRight);
    if(m_nodeLeft->hasASymbolSelf())
      return m_nodeRight->belongsToVOWN(vown); //determine
    return false;
  }

  bool NodeMemberSelect::isAConstant()
  {
    return m_nodeLeft->isAConstant(); //constant classes possible
  }

  bool NodeMemberSelect::isAMemberSelect()
  {
    return true;
  }

  bool NodeMemberSelect::isAMemberSelectByRegNum()
  {
    assert(isAMemberSelect());
    return false;
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
    UTI nuti = getNodeType();
    UTI luti = m_nodeLeft->checkAndLabelType(); //side-effect

    if(!m_state.isComplete(luti))
      {
	std::ostringstream msg;
	msg << "Member selected is incomplete class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	msg << ", check and label fails this time around";
	if(luti == Nav)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    nuti = Nav; //error/t3460
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    if(nuti != Nav)
	      nuti = Hzy; //avoid clobbering Nav
	  }
	setNodeType(nuti);
	if(nuti == Hzy) m_state.setGoAgain();
	return getNodeType();
      } //done

    TBOOL stor = checkStoreIntoAble(); //given lhs, this node set later
    if(m_nodeRight->isFunctionCall())
      {
	if(stor == TBOOL_FALSE)
	  nuti = Nav;
	else if(stor == TBOOL_HAZY)
	  nuti = Hzy; //t3607
      }
    else
      {
	//data members cannot be shadowed by relatives, t.f. selection
	//by classId of related subclasses doesn't make sense!
	if(m_nodeLeft->isAMemberSelect() && ((NodeMemberSelect *) m_nodeLeft)->isAMemberSelectByRegNum())
	  {
	    std::ostringstream msg;
	    msg << "Member selected by classId must be a virtual function, ";
	    msg << "not data member '" << m_nodeRight->getName() << "'";
	    msg << "; data members cannot be shadowed by related subclasses";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //t41388
	  } //done
      }

    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClassType();
    if(((lclasstype == UC_NOTACLASS) && (lut->getUlamTypeEnum() != Holder)) || !lut->isScalar())
      {
	// must be a scalar 'Class' type, (e.g. error/t3815)
	// doesn't complete checkandlabel for rhs (e.g. funccall is NULL, no eval)
	std::ostringstream msg;
	msg << "Member selected must be a Class, not type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	if(lclasstype != UC_NOTACLASS)
	  msg << "[" << lut->getArraySize() << "]";
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

    UTI leftblockuti = memberClassNode->getNodeType();
    if(!m_state.okUTItoContinue(leftblockuti))
      {
	std::ostringstream msg;
	msg << "Member selected is not ready: ";
	msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	if(leftblockuti == Nav)
	  {
	    m_state.abortShouldntGetHere(); //luti is complete! (t41363)
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setNodeType(Hzy);
	    m_state.setGoAgain();
	    return Hzy; //t41222, inheritance
	  }
      }
    //else //t41010, t41145

   //set up compiler state to use the member class block for symbol searches
    m_state.pushClassContextUsingMemberClassBlock(memberClassNode);

    UTI rightType = m_nodeRight->checkAndLabelType();

    //clear up compiler state to no longer use the member class block for symbol searches
    m_state.popClassContext();

    setNodeType(rightType); //set Nav if error (t3513)

    if(m_state.okUTItoContinue(rightType))
      {
	setStoreIntoAbleAndReferenceAble();
	if(m_nodeRight->isFunctionCall())
	  {
	    if(m_nodeLeft->isAMemberSelect() && ((NodeMemberSelect *) m_nodeLeft)->isAMemberSelectByRegNum())
	      {
		Symbol * fsymptr = NULL;
		AssertBool gotfunc = m_nodeRight->getSymbolPtr(fsymptr);
		assert(gotfunc);
		assert(fsymptr->isFunction());
		if(!((SymbolFunction *) fsymptr)->isVirtualFunction())
		  {
		    std::ostringstream msg;
		    msg << "Member selected by classId must be a VIRTUAL function, ";
		    msg << "not '" << m_nodeRight->getName() << "'";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		    return Nav; //t41388
		  }
	      }
	  }
      }
    return getNodeType();
  } //checkAndLabelType

  bool NodeMemberSelect::getConstantMemberValue(BV8K& bvmsel)
  {
    bool rtnok = false;
    //vs t41232, t41263
    //righthand member of constant class (t41273); left is complete, we know.
    assert(m_nodeLeft->isAConstant());
    UTI leftType = m_nodeLeft->getNodeType();
    assert(m_state.isAClass(leftType));
    assert(m_state.isComplete(leftType));
    UlamType * lut = m_state.getUlamTypeByIndex(leftType);
    ULAMCLASSTYPE lclasstype = lut->getUlamClassType();
    UTI rightType = m_nodeRight->getNodeType();
    UlamType * rut = m_state.getUlamTypeByIndex(rightType);

    //rhs could be a class/array, primitive/array; whatever it is a constant!
    //replace rhs with a constant node version of it, using the value found in lhs.
    assert(!m_nodeRight->isAList());
    assert(!m_nodeRight->isFunctionCall());
    if(!m_nodeRight->isAConstant())
      {
	Symbol * sym = NULL;
	if(m_nodeRight->getSymbolPtr(sym))
	  {
	    assert(sym->isDataMember());
	    SymbolVariableDataMember * dmsym = (SymbolVariableDataMember *) sym;
	    u32 rpos = 9999;
	    if(!dmsym->isPosOffsetReliable())
	      {
		TBOOL packed = m_state.tryToPackAClass(leftType);
		if(packed == TBOOL_TRUE)
		  {
		    m_nodeRight->getSymbolPtr(sym); //refresh
		    dmsym = (SymbolVariableDataMember *) sym;
		    rpos = dmsym->getPosOffset();
		  }
		//else cannot pack yet
	      }
	    else
	      rpos = dmsym->getPosOffset();

	    //okay to fold (possible refactor TODO); Element Types and Strings still un-fixed.
	    if(rpos != 9999)
	      {
		if(lclasstype == UC_ELEMENT)
		  rpos += ATOMFIRSTSTATEBITPOS;

		BV8K bvcctmp;
		bool gotVal = m_nodeLeft->getConstantValue(bvcctmp);

		if(gotVal)
		  {
		    u32 rlen = rut->getSizeofUlamType();
		    bvcctmp.CopyBV(rpos, 0, rlen, bvmsel);
		    rtnok = true;
		  } //no left class value
	      } //rpos not reliable
	  } //no right symbol ptr
      }
    else
      {
	//right is a constant (t41278)
	rtnok = m_nodeRight->getConstantValue(bvmsel);
      }
    return rtnok;
  } //getConstantMemberValue

  bool NodeMemberSelect::getConstantValue(BV8K& bval)
  {
    return getConstantMemberValue(bval);
  }

  TBOOL NodeMemberSelect::checkStoreIntoAble()
  {
    TBOOL lstor = m_nodeLeft->getStoreIntoAble();
    if(m_nodeRight->isFunctionCall())
      {
	if(lstor != TBOOL_TRUE)
	  {
	    //e.g. funcCall is not storeintoable even if its return value is.
	    std::ostringstream msg;
	    msg << "Member selected must be a modifiable lefthand side: '";
	    msg << m_nodeLeft->getName();
	    msg << "' requires a variable";
	    if(m_nodeLeft->isFunctionCall())
	      msg << "; may be a function call";
	    else if(hasASymbolReference() && hasASymbolReferenceConstant())
	      msg << "; may be a constant function parameter";
	    if(lstor == TBOOL_HAZY)
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  } //done
      }
    return lstor;
  } //checkStoreIntoAble

  void NodeMemberSelect::setStoreIntoAbleAndReferenceAble()
  {
    TBOOL  lstor = m_nodeLeft->getStoreIntoAble(); //default ok
    TBOOL  rstor = m_nodeRight->getStoreIntoAble(); //default ok
    TBOOL stor = Node::minTBOOL(lstor, rstor); //min of lhs and rhs
    setStoreIntoAble(stor);

    TBOOL  lrefer = m_nodeLeft->getReferenceAble(); //default ok
    TBOOL  rrefer = m_nodeRight->getReferenceAble(); //default ok
    TBOOL refer = Node::minTBOOL(lrefer, rrefer); //min of lhs and rhs
    setReferenceAble(refer);
  }

  bool NodeMemberSelect::trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr)
  {
    Node * tmpnptr = NULL;
    //right is a leaf (NodeIdent)
    if(m_nodeRight->trimToTheElement(NULL, tmpnptr))
      {
	rtnnodeptr = this; //keep ms node;
	if(fromleftnode)
	  *fromleftnode = NULL; //clear for future deletion
	return true;
      }

    //left-assoc tree..
    if(m_nodeLeft->trimToTheElement(&m_nodeLeft, tmpnptr))
      {
	rtnnodeptr = tmpnptr;
	return true;
      }
    return false;
  } //trimToTheElement

  bool NodeMemberSelect::assignClassArgValueInStubCopy()
  {
    return true; //nothing to do
  }

  bool NodeMemberSelect::isFunctionCall()
  {
    return m_nodeRight->isFunctionCall(); //based like storeintoable, on right
  }

  bool NodeMemberSelect::isAConstructorFunctionCall()
  {
    //see NodeMemberSelectOnConstructorCall
    return false; //based like storeintoable, on right
  }

  bool NodeMemberSelect::isArrayItem()
  {
    return m_nodeRight->isArrayItem(); //based like storeintoable, on right
  }

  EvalStatus NodeMemberSelect::eval()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

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

    u32 slot = makeRoomForNodeType(nuti);
    evs = m_nodeRight->eval(); //a Node Function Call here, or data member eval
    if(evs != NORMAL) return evalStatusReturn(evs);

    //assigns rhs (next slot e.g. t3704) to lhs UV pointer (handles arrays);
    //also copy result UV to stack, -1 relative to current frame pointer
    if(slot) //avoid Void's
      if(!doBinaryOperation(1, 1+1, slot))
	return evalStatusReturn(ERROR); //skip restore now, ok???

    m_state.m_currentObjPtr = saveCurrentObjectPtr; //restore current object ptr
    m_state.m_currentSelfPtr = saveCurrentSelfPtr; //restore current self ptr

    if(evs != NORMAL) return evalStatusReturn(evs);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  //for eval, want the value of the rhs
   bool NodeMemberSelect::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);

    UlamValue rtnUV;
    UTI ruti = getNodeType();

    if(Node::returnValueOnStackNeededForEval(ruti)) //t3704
      {
	//the return value of a function call, or value of a data member
	UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot);
	PACKFIT packFit = m_state.determinePackable(ruti);

	if(m_state.isScalar(ruti) || WritePacked(packFit))
	  {
	    rtnUV = ruv;
	  }
	else
	  m_state.abortNotImplementedYet(); //or repeat the next else ????
      }
    else
      {
	//make a ptr to an unpacked array, base[0] ? //t3704,
	//t3381,t3853,t3995 (PACKEDLOADABLE, not UNPACKED);
	rtnUV = UlamValue::makePtr(rslot, EVALRETURN, ruti, m_state.determinePackable(ruti), m_state);
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

    makeRoomForSlots(1); //always 1 slot for ptr
    evs = m_nodeRight->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

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
  } //evalToStoreInto

  UlamValue NodeMemberSelect::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    m_state.abortShouldntGetHere(); //unused
    return UlamValue();
  }

  UlamValue NodeMemberSelect::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    m_state.abortShouldntGetHere(); //unused
    return UlamValue();
  }

  void NodeMemberSelect::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    m_state.abortShouldntGetHere(); //unused
  }

  void NodeMemberSelect::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);

    // if parent is another MS, we might need to adjust pos first;
    // elements can be data members of transients, etc. (t3968)
    UVPass luvpass;
    if(passalongUVPass())
      {
	luvpass = uvpass;
      }

    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    //NodeIdent can't do it, because it doesn't know it's not a stand-alone element.
    // here, we know there's rhs of member select, which needs to adjust to state bits.
    if(passalongUVPass())
      {
	uvpass = luvpass;
      }

    //check the back (not front) to process multiple member selections (e.g. t3818)
    m_nodeRight->genCode(fp, uvpass);  //leave any array item as-is for gencode.

    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************?
  } //genCode

  // presumably called by e.g. a binary op equal (lhs); caller saves
  // currentObjPass/Symbol, unlike genCode (rhs)
  void NodeMemberSelect::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);

    UVPass luvpass;
    if(passalongUVPass())
      {
	luvpass = uvpass; //t3584, t3803
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
	ruvpass = luvpass;  //t3615, t3803
      }

    m_nodeRight->genCodeToStoreInto(fp, ruvpass); //uvpass contains the member selected, or cos obj symbol?

    uvpass = ruvpass;

    //tmp variable needed for any function call not returning a ref (t41006), including 'aref'(t41005); func calls returning a ref already made tmpvar.
    // uvpass not necessarily returning a reference type (t3913,4,5,7);
    // t41035 returns a primitive ref; t3946, t3948
    if(m_nodeRight->isFunctionCall() && !m_state.isStringATmpVar(uvpass.getPassNameId()))
      {
	m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL); //dm to avoid leaks
	m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
      }
  } //genCodeToStoreInto

  bool NodeMemberSelect::passalongUVPass()
  {
    bool rtnb = false; //don't pass along
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	Symbol * cossym = m_state.m_currentObjSymbolsForCodeGen.back();
	UTI cosuti = cossym->getUlamTypeIdx();
	UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
	//t3913, t3915 tmpref may not be a ref, but may need adjusting (i.e. anonymous element returned)
	// t3706 not isAltRefType; t41307,9,10 isBaseClassRef (ulam-5); t41314 pass if self;
	rtnb = cossym->isSelf() || (!cosut->isReference() && (!cossym->isTmpVarSymbol() || Node::needAdjustToStateBits(cosuti) || ((SymbolTmpVar *) cossym)->isBaseClassRef())) || (cosut->getReferenceType() == ALT_AS) /* AS, but not ARRAYITEM (t41396, t3706) */  ;
      }
    return rtnb;
  }

} //end MFM
