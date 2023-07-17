#include <stdio.h>
#include "NodeAtomof.h"
#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  NodeAtomof::NodeAtomof(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeStorageof(ofnode, nodetype, state) { }

  NodeAtomof::NodeAtomof(const NodeAtomof& ref) : NodeStorageof(ref) { }

  NodeAtomof::~NodeAtomof() { }

  Node * NodeAtomof::instantiate()
  {
    return new NodeAtomof(*this);
  }

  const char * NodeAtomof::getName()
  {
    return ".atomof";
  }

  const std::string NodeAtomof::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeAtomof::checkAndLabelType(Node * thisparentnode)
  {
    assert(m_nodeOf); //Identifier, not a Type; caught at parse time (right?)
    UTI nuti = NodeStorageof::checkAndLabelType(thisparentnode);
    if(m_state.okUTItoContinue(nuti))
      {
	UTI vuti = m_nodeOf->getNodeType();
	bool isself = m_nodeOf->hasASymbolSelf();
	bool isaref = m_state.isReference(vuti); //t3706, t41046 (not isAltRefType)
	UTI oftype = NodeStorageof::getOfType();
	UlamType * ofut = m_state.getUlamTypeByIndex(oftype);
	assert(isself || UlamType::compare(m_state.getUlamTypeAsDeref(vuti), oftype, m_state) == UTIC_SAME); //sanity (e.g. t3905, t3701)
	ULAMCLASSTYPE ofclasstype = ofut->getUlamClassType();

	//refs checked at runtime; non-refs here:
	if(!isaref && !isself)
	  {
	    //only an element or atom have real storage (ie. not quarks)
	    if(ofclasstype == UC_QUARK)
	      {
		//only way to get storage for a quark is if its a DM
		// of an element;
		if(!m_nodeOf->hasASymbolDataMember())
		  {
		    std::ostringstream msg;
		    msg << "'" << m_nodeOf->getName();
		    msg << "' is a quark and cannot be used with ";
		    msg << getName() << "; try a reference or self";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		  }
		else
		  {
		    //quark dm, must find an element or ele-ref in m_nodeOf, o.w. error
		    // (can't be an atom dm, and transients are illegal for atomof, !isself)
		    // lose the quark. (t3906)
		    if(!trimToTheElement(NULL, m_nodeOf))
		      setNodeType(Nav);
		    else
		      setOfType(m_nodeOf->getNodeType());
		  }
	      }
	    else if(ofclasstype == UC_TRANSIENT)
	      {
		std::ostringstream msg;
		msg << "'" << m_nodeOf->getName();
		msg << "' is a transient";
		msg << "; Transients cannot be used with " << getName();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav); //e.g. error/t3761
	      }
	    //else element (or atom) ok!
	  } //neither a ref nor isself (done at compile time)
	else if(ofclasstype == UC_TRANSIENT)
	  {
	    std::ostringstream msg;
	    msg << "'"  << m_nodeOf->getName();
	    msg << "' is a transient reference";
	    msg << "; Transients cannot be used with " << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav); //e.g. error/t3762
	  }
	else if(ofclasstype == UC_QUARK)
	  {
	    //either a quark ref (runtime), or isself; if self, trim to self (t3905)
	    if(isself && trimToTheElement(NULL, m_nodeOf))
	      setOfType(m_nodeOf->getNodeType());
	  }
	//else element (or atom) is clear to go
      }
    return getNodeType(); //UAtomRef (storeintoable)
  } //checkAndLabelType

  bool NodeAtomof::trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr)
  {
    bool brtn = false;
    Node * tmpnodeof = NULL;
    if(m_nodeOf->trimToTheElement(NULL, tmpnodeof))
      {
	if(m_nodeOf != tmpnodeof)
	  {
	    delete m_nodeOf;
	    m_nodeOf = tmpnodeof;
	  }
	brtn = true;
      }
    else
      {
	std::ostringstream msg;
	msg << "No element node found to trim";
	msg << "; Quarks alone cannot be used with " << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return brtn;
  } //trimToTheElement

  UlamValue NodeAtomof::makeUlamValuePtr()
  {
    // (from NodeVarDecl's makeUlamValuePtr)
    UlamValue ptr;
    UlamValue atomuv;

    UTI nuti = getNodeType();
    UTI auti = getOfType(); //deref'ed

    assert(m_nodeOf);
    if(m_nodeOf->hasASymbolSelf())
      {
	//when "self/atom" is a quark, we're inside a func called on a quark (e.g. dm or local)
	//'atom' gets entire atom/element containing this quark; including its type!
	//'self' gets type/pos/len of the quark from which 'atom' can be extracted
	UlamValue selfuvp = m_state.m_currentSelfPtr;
	UTI selfttype = selfuvp.getPtrTargetType();
	assert(m_state.okUTItoContinue(selfttype));
	UTI effselfttype = selfuvp.getPtrTargetEffSelfType();
	if(effselfttype == Nouti) effselfttype = selfttype; //t3913

	if((m_state.getUlamTypeByIndex(selfttype)->getUlamClassType() == UC_QUARK) && (!m_state.isReference(auti) || (selfttype == effselfttype)))
	  {
	    selfuvp = atomuv; //bail for error (fail/t3682, error/t3681)
	  }
	else
	  {
	    if((UlamType::compareForUlamValueAssignment(selfttype, auti, m_state) == UTIC_NOTSAME))
	      {
		s32 selfpos = selfuvp.getPtrPos();
		if(selfttype != effselfttype)  //t3736, another baseclass
		  {
		    u32 relposofbase2 = 0;
		    AssertBool gotpos2 = m_state.getABaseClassRelativePositionInAClass(effselfttype, selfttype, relposofbase2);
		    assert(gotpos2);
		    selfpos -= relposofbase2;
		  }
		//else same, subclass or dm

		u32 relposofbase = 0;
		AssertBool gotpos = m_state.getABaseClassRelativePositionInAClass(effselfttype, auti, relposofbase);
		assert(gotpos);

		selfuvp.setPtrPos(selfpos + relposofbase);
		selfuvp.setPtrTargetType(m_state.getUlamTypeAsDeref(auti));
		selfuvp.setPtrTargetEffSelfType(effselfttype, m_state); //in case was empty
		selfuvp.setPtrLen(m_state.getBaseClassBitSize(auti));
	      }
	    //else same
	  } //end of else
	return selfuvp;
      } //done

    if(m_state.getReferenceType(auti) == ALT_AS)
      {
	m_state.abortShouldntGetHere();
	return m_nodeOf->getSymbolAutoPtrForEval(); //haha! we're done.
      }

    if(m_nodeOf->hasASymbolDataMember())
      {
	UTI cuti = m_state.m_currentObjPtr.getPtrTargetType();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	if(cut->getUlamClassType() == UC_QUARK)
	  ptr = atomuv; //bail
	else
	  {
	    // return ptr to the m_currentObjPtr that contains this data member within
	    ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), auti, m_state.determinePackable(getNodeType()), m_state, 0, 0); //id?
	    ptr.setPtrTargetEffSelfTypeFromAnotherUV(m_state.m_currentObjPtr, m_state);
	    ptr.checkForAbsolutePtr(m_state.m_currentObjPtr);
	  }
      }
    //else if(vsym != NULL)
    else if(m_nodeOf->hasASymbol())
      {
	UTI vuti = m_nodeOf->getNodeType(); //t3701,t3835,t3837
	UlamType * vut = m_state.getUlamTypeByIndex(vuti);

	if(vut->getUlamClassType() == UC_QUARK)
	  {
	    if(m_state.isReference(vuti)) //t3701,t3835,t3837
	      ptr = m_nodeOf->getSymbolAutoPtrForEval(); //t3835
	    else
	      {
		m_state.abortNeedsATest(); //this would be an error
		ptr = atomuv; //bail
	      }
	  }
	else
	  {
	    //local variable on the stack; could be array ptr! (t41531)
	    ptr = UlamValue::makePtr(m_nodeOf->getSymbolStackFrameSlotIndex(), STACK, auti, m_state.determinePackable(auti), m_state, 0, m_nodeOf->getNameId()); //id?
	    ptr.setPtrTargetEffSelfType(auti, m_state); //missing?
	  }
      }
    else
      {
	//no symbol based on Class Type instead, like NodeInstanceof, (t41531)
	u32 atop = 1;
	atop = m_state.m_funcCallStack.getAbsoluteStackIndexOfSlot(atop);
	ptr = UlamValue::makePtr(atop, STACK, nuti, m_state.determinePackable(nuti), m_state, 0);
	ptr.setPtrTargetEffSelfType(auti, m_state);
	ptr.setUlamValueTypeIdx(PtrAbs);
      }
    return ptr;
  } //makeUlamValuePtr

  void NodeAtomof::genCode(File * fp, UVPass& uvpass)
  {
    //lhs, no longer allowed with packed elements
    assert(getStoreIntoAble() == TBOOL_TRUE);

    UTI nuti = getNodeType(); //UAtomRef
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(m_nodeOf->hasASymbolReference() && (m_state.getUlamTypeByIndex(getOfType())->getUlamClassType() == UC_QUARK))
      {
	m_nodeOf->genCodeToStoreInto(fp, uvpass);

	Symbol * stgcos = NULL;
	Symbol * cos = NULL;
	Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);

	m_state.clearCurrentObjSymbolsForCodeGen(); //t3663

	m_state.indentUlamCode(fp);
	fp->write("if(");
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".GetType() == T::ATOM_UNDEFINED_TYPE)"); GCNL;

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(NOT_AN_ELEMENT);"); GCNL;
	m_state.m_currentIndentLevel--;

	s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref

	m_state.indentUlamCode(fp); //non-const
	fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write("(");

	fp->write(stgcos->getMangledName().c_str()); //ur for self
	fp->write(", - "); //is storage! can't be const (error/t3659), t3408

	fp->write(stgcos->getMangledName().c_str()); //ur for self, cant assume 0u pos
	fp->write(".GetPosToEffectiveSelf()"); //t3701

	fp->write(" - T::ATOM_FIRST_STATE_BIT"); //must be an effective element ref (e.g.t3684, t3663)
	fp->write("); //atomof"); GCNL;

	//read into a T
	s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for atomref
	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(nut->getTmpStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, TMPTATOM).c_str());
	fp->write(" = ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write(".read();"); GCNL;

	uvpass = UVPass::makePass(tmpVarNum2, TMPTATOM, nuti, UNPACKED, m_state, 0, uvpass.getPassNameId());
      }
    else
      {
	//get the element in a tmpvar; necessary for array item members selected (at runtime).
	m_nodeOf->genCode(fp, uvpass);
	assert(m_state.m_currentObjSymbolsForCodeGen.empty());

	if(uvpass.getPassStorage() == TMPBITVAL)
	  {
	    //read into a T (t41531)
	    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for atomref
	    m_state.indentUlamCode(fp);
	    fp->write("const ");
	    fp->write(nut->getTmpStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, TMPTATOM).c_str());
	    fp->write(" = ");
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(".read();"); GCNL;

	    uvpass = UVPass::makePass(tmpVarNum2, TMPTATOM, nuti, UNPACKED, m_state, uvpass.getPassPos(), uvpass.getPassNameId());
	  }
	else
	  {
	    assert(uvpass.getPassStorage() == TMPTATOM); //sanity
	    //e.g. 'return self.atomof;'
	    //(t3408,t3410,t3585,t3631,t3663,t41503,t41461,t41460,t41143)
	    uvpass = UVPass::makePass(uvpass.getPassVarNum(), TMPTATOM, nuti, UNPACKED, m_state, uvpass.getPassPos(), uvpass.getPassNameId());
	  }
      }
  } //genCode

  void NodeAtomof::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    //lhs, no longer allowed with packed elements
    assert(getStoreIntoAble() == TBOOL_TRUE);

    UTI nuti = getNodeType(); //UAtomRef

    if(m_nodeOf->hasASymbolReference() && (m_state.getUlamTypeByIndex(getOfType())->getUlamClassType() == UC_QUARK))
      {
	m_nodeOf->genCodeToStoreInto(fp, uvpass);

	Symbol * stgcos = NULL;
	Symbol * cos = NULL;
	Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);

	m_state.clearCurrentObjSymbolsForCodeGen(); //t3756..

	m_state.indentUlamCode(fp);
	fp->write("if(");
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".GetType() == T::ATOM_UNDEFINED_TYPE)"); GCNL;

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(NOT_AN_ELEMENT);"); GCNL;
	m_state.m_currentIndentLevel--;

	UlamType * nut = m_state.getUlamTypeByIndex(nuti);
	s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref

	m_state.indentUlamCode(fp); //non-const
	fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPAUTOREF).c_str());
	fp->write("(");

	fp->write(stgcos->getMangledName().c_str()); //ur for self
	fp->write(", -"); //is storage! can't be const (error/t3659)

	//(ulam-5) base class no longer at zero (t3735)
	fp->write(stgcos->getMangledName().c_str()); //ur for self
	fp->write(".GetPosToEffectiveSelf()"); //t3735
	fp->write(" - T::ATOM_FIRST_STATE_BIT"); //must be an effective element ref (e.g.t3684, t3663)
	fp->write("); //atomof"); GCNL;

	uvpass = UVPass::makePass(tmpVarNum, TMPAUTOREF, nuti, UNPACKED, m_state, uvpass.getPassPos(), uvpass.getPassNameId());
      }
    else
      {
	//lhs: t3223,t3684,t3907,8,9,t41033,42,43,46,51,62
	assert(getStoreIntoAble() == TBOOL_TRUE);
	assert(m_nodeOf);
	m_nodeOf->genCodeToStoreInto(fp, uvpass); //does it handle array item members selected?
	assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

	uvpass = UVPass::makePass(uvpass.getPassVarNum(), TMPAUTOREF /*TMPTATOM*/, getNodeType(), UNPACKED, m_state, uvpass.getPassPos(), uvpass.getPassNameId());
      }
  } //genCodeToStoreInto

} //end MFM
