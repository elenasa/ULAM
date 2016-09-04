#include <stdio.h>
#include "NodeAtomof.h"
#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  //NodeAtomof::NodeAtomof(const Token& tokof, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeStorageof(tokof, nodetype, state) { }
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

  UTI NodeAtomof::checkAndLabelType()
  {
    assert(m_nodeOf); //Identifier, not a Type; caught at parse time (right?)
    UTI nuti = NodeStorageof::checkAndLabelType();
    if(m_state.okUTItoContinue(nuti))
      {
	//assert(m_varSymbol);
	//UTI vuti = m_varSymbol->getUlamTypeIdx();
	//bool isself = m_varSymbol->isSelf(); //a ref
	UTI vuti = m_nodeOf->getNodeType();
	bool isself = m_nodeOf->hasASymbolSelf();
	bool isaref = m_state.isReference(vuti);
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
		//if(!m_varSymbol->isDataMember())
		if(!m_nodeOf->hasASymbolDataMember())
		  {
		    std::ostringstream msg;
		    msg << "<" << m_nodeOf->getName();
		    msg << "> is a quark and cannot be used with ";
		    msg << getName() << "; try a reference or self";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		  }
		else
		  {
		    //quark dm, must find an element or ele-ref in m_nodeOf, o.w. error
		    // (can't be an atom dm, and transients are illegal for atomof, !isself)
		    // kill the quark. (t3906)
		    if(!trimToTheElement(NULL, m_nodeOf))
		      setNodeType(Nav);
		    else
		      setOfType(m_nodeOf->getNodeType());
		  }
	      }
	    else if(ofclasstype == UC_TRANSIENT)
	      {
		std::ostringstream msg;
		msg << "<" << m_nodeOf->getName();
		msg << "> is a transient";
		msg << "; Transients cannot be used with " << getName();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav); //e.g. error/t3761
	      }
	    //else element (or atom) ok!
	  } //neither a ref nor isself (done at compile time)
	else if(ofclasstype == UC_TRANSIENT)
	  {
	    std::ostringstream msg;
	    msg << "<"  << m_nodeOf->getName();
	    msg << "> is a transient reference";
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
    //assert(m_nodeOf);
    //return m_nodeOf->makeUlamValuePtr();
#if 1
    // (from NodeVarDecl's makeUlamValuePtr)
    UlamValue ptr;
    UlamValue atomuv;

    UTI auti = getOfType();
    assert(m_nodeOf);

    //if(m_varSymbol->isSelf())
    if(m_nodeOf->hasASymbolSelf())
      {
	//when "self/atom" is a quark, we're inside a func called on a quark (e.g. dm or local)
	//'atom' gets entire atom/element containing this quark; including its type!
	//'self' gets type/pos/len of the quark from which 'atom' can be extracted
	UlamValue selfuvp = m_state.m_currentSelfPtr;
	UTI ttype = selfuvp.getPtrTargetType();
	assert(m_state.okUTItoContinue(ttype));
	if((m_state.getUlamTypeByIndex(ttype)->getUlamClassType() == UC_QUARK))
	  {
	    selfuvp = atomuv; //bail for error
	  }
	return selfuvp;
      } //done

    //if(m_varSymbol->getAutoLocalType() == ALT_AS)
    if(m_state.getReferenceType(auti) == ALT_AS)
      {
	assert(0);
	Symbol * vsym = NULL;
	m_nodeOf->getSymbolPtr(vsym);
	return ((SymbolVariableStack *) vsym)->getAutoPtrForEval(); //haha! we're done.
      }

    //if(m_varSymbol->isDataMember())
    if(m_nodeOf->hasASymbolDataMember())
      {
	UTI cuti = m_state.m_currentObjPtr.getPtrTargetType();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	if(cut->getUlamClassType() == UC_QUARK)
	  ptr = atomuv; //bail
	else
	  {
	    // return ptr to the m_currentObjPtr that contains this data member within
	    //ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), auti, m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	    ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), auti, m_state.determinePackable(getNodeType()), m_state, 0, 0); //id?

	    ptr.checkForAbsolutePtr(m_state.m_currentObjPtr);
	  }
      }
    else
      {
	//UTI vuti = m_varSymbol->getUlamTypeIdx();
	UTI vuti = getOfType(); //m_nodeOf->getNodeType();
	UlamType * vut = m_state.getUlamTypeByIndex(vuti);
	if(vut->getUlamClassType() == UC_QUARK)
	  ptr = atomuv; //bail
	else
	  {
	  //local variable on the stack; could be array ptr!
	    Symbol * vsym = NULL;
	    m_nodeOf->getSymbolPtr(vsym);

	    //ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, auti, m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	    ptr = UlamValue::makePtr(((SymbolVariableStack *) vsym)->getStackFrameSlotIndex(), STACK, auti, m_state.determinePackable(vuti), m_state, 0, vsym->getId()); //id?
	  }
      }
    return ptr;
#endif
  } //makeUlamValuePtr

  void NodeAtomof::genCode(File * fp, UVPass& uvpass)
  {
    //lhs, no longer allowed with packed elements
    assert(getStoreIntoAble() == TBOOL_TRUE);

    UTI nuti = getNodeType(); //UAtomRef
    //UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    //s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref

    //assert(m_varSymbol);
    //UTI vuti = m_varSymbol->getUlamTypeIdx();
    //bool isself = m_varSymbol->isSelf();
    //bool issuper = m_varSymbol->isSuper();
    //bool isaref = (m_state.isReference(vuti) || isself || issuper);
    //if var is a data member quark, then also isself
    //assert(m_nodeOf);
    //m_nodeOf->genCodeToStoreInto(fp, uvpass);

    //assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    //Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    //UTI cosuti = cos->getUlamTypeIdx();
    //Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    //bool isself = stgcos->isSelf();
    //bool issuper = stgcos->isSuper();
    //bool isaref = m_state.isReference(cosuti) || isself || issuper;

    //get the element in a tmpvar
    m_nodeOf->genCode(fp, uvpass);

    //e.g. 'return self.atomof;' (e.g. t3408, t3410, t3585, t3631, t3663)
    uvpass = UVPass::makePass(uvpass.getPassVarNum(), TMPTATOM, nuti, UNPACKED, m_state, 0, uvpass.getPassNameId());

#if 0
    bool isself = m_nodeOf->hasASymbolSelf();

#if 0
    UTI vuti = m_nodeOf->getNodeType();
    UTI oftype = NodeStorageof::getOfType();
    UlamType * ofut = m_state.getUlamTypeByIndex(oftype);

    if(m_state.isReference(vuti))
      {
	m_state.indentUlamCode(fp);
	fp->write("if(");
	//fp->write(m_varSymbol->getMangledName().c_str());
	fp->write(cos->getMangledName().c_str());
	fp->write(".GetType() == T::ATOM_UNDEFINED_TYPE)"); GCNL;

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(NOT_AN_ELEMENT);"); GCNL;
	m_state.m_currentIndentLevel--;
      }
#endif

    m_state.indentUlamCode(fp); //non-const
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //for C++ local vars
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPTATOM).c_str());
    fp->write(" = ");

    //data member's storage is self (not a ref); or member selected (t3905)
    //if(m_varSymbol->isDataMember())
    //if(cos->isDataMember())
    if(m_nodeOf->hasASymbolDataMember())
      {
	if(!isself)
	  {
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());

#if 0
	    s32 elementidx = Node::isCurrentObjectsContainingAnElement();
	    assert(elementidx > -1);
	    s32 pos = Node::calcPosOfCurrentObjects(true, elementidx); //what if array item?
	    Symbol * elesym = m_state.m_currentObjSymbolsForCodeGen[elementidx];
	    UTI eleuti = elesym->getUlamTypeIdx();
	    UlamType * eleut = m_state.getUlamTypeByIndex(eleuti);

	    fp->write("UlamRef<EC>("); //wrapper for array item
	    if(eleut->isReference())
	      {
		fp->write(elesym->getMangledName().c_str()); //reference first
		fp->write(", ");
	      }
	    fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3832
	    fp->write_decimal_unsigned(pos); //rel offset of array base (e.g.t3147, t3148, t3153, t3177..)

	    if(eleut->isScalar())
	      fp->write("u, ");
	    else
	      {
		assert(0); //?????????????????
		fp->write("u + ");
		fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //INDEX??????????????
		fp->write(" * ");
		fp->write("T::BPA), "); //atom-based item length
	      }
	    fp->write("T::BPA - T::ATOM_FIRST_STATE_BIT, "); // item length

	    if(!eleut->isReference())
	      {
		fp->write(elesym->getMangledName().c_str()); //non-ref stg
		fp->write(", ");
	      }

	    fp->write("NULL, UlamRef<EC>::ELEMENTAL, uc)");
#endif
	  }
	else //isself
	  fp->write("ur");
      }
    else
      //fp->write(m_varSymbol->getMangledName().c_str()); //element or atom, ur for self/super
      //fp->write(cos->getMangledName().c_str()); //element or atom, ur for self/super

      fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //REDUNDANT, only 2 options

    fp->write(".ReadAtom(); //atomof"); GCNL; //can't be const

    //uvpass = UVPass::makePass(tmpVarNum, TMPTATOM, nuti, UNPACKED, m_state, 0, m_varSymbol ? m_varSymbol->getId() : 0);
    //uvpass = UVPass::makePass(tmpVarNum, TMPTATOM, nuti, UNPACKED, m_state, 0, cos ? cos->getId() : 0);
    uvpass = UVPass::makePass(tmpVarNum, TMPTATOM, nuti, UNPACKED, m_state, 0, uvpass.getPassNameId());

    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
#endif
  } //genCode

  void NodeAtomof::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    //lhs
    assert(getStoreIntoAble() == TBOOL_TRUE);
    //UTI nuti = getNodeType(); //UAtomRef
    //UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    //s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref

    //assert(m_varSymbol);
    //UTI vuti = m_varSymbol->getUlamTypeIdx();
    //bool isself = m_varSymbol->isSelf();
    //bool issuper = m_varSymbol->isSuper();
    //bool isaref = (m_state.isReference(vuti) || isself || issuper);
    //if var is a data member quark, then also isself (or issuper)
    assert(m_nodeOf);
    m_nodeOf->genCodeToStoreInto(fp, uvpass);
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

#if 0
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    bool isself = stgcos->isSelf();
    bool issuper = stgcos->isSuper();
    bool isaref = m_state.isReference(cosuti) || isself || issuper;

    if(isaref)
      {
	m_state.indentUlamCode(fp);
	fp->write("if(");
	//fp->write(m_varSymbol->getMangledName().c_str());
	fp->write(cos->getMangledName().c_str());
	fp->write(".GetType() == T::ATOM_UNDEFINED_TYPE)"); GCNL;

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(NOT_AN_ELEMENT);"); GCNL;
	m_state.m_currentIndentLevel--;
      }

    m_state.indentUlamCode(fp); //non-const
    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
    fp->write("(");

    //data member's storage is self (not a ref)
    //if(m_varSymbol->isDataMember())
    if(cos->isDataMember())
      {
	assert(0); //see genCode above
	fp->write("ur");
      }
    else
      //fp->write(m_varSymbol->getMangledName().c_str()); //element or atom (ur for self/super)
      fp->write(cos->getMangledName().c_str()); //element or atom (ur for self/super)

    fp->write(", "); //is storage! can't be const (error/t3659)

    if(isaref)
      fp->write(" - T::ATOM_FIRST_STATE_BIT"); //must be an effective element ref (e.g.t3684, t3663)
    else
      fp->write("0u");

    if(!isaref)
      {
	fp->write(", uc); //atomof"); GCNL;
      }
    else
      {
	fp->write("); //atomof"); GCNL; //t3684
      }

    //uvpass = UVPass::makePass(tmpVarNum, TMPBITVAL, nuti, UNPACKED, m_state, 0, m_varSymbol ? m_varSymbol->getId() : 0);
    uvpass = UVPass::makePass(tmpVarNum, TMPBITVAL, nuti, UNPACKED, m_state, 0, cos ? cos->getId() : 0);

    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
#endif
  } //genCodeToStoreInto

} //end MFM
