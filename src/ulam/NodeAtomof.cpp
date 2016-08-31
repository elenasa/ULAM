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
    //assert(m_token.m_type == TOK_IDENTIFIER); //caught at parse time (right?)
    assert(m_nodeOf); //Identifier, not a Type; caught at parse time (right?)
    UTI nuti = NodeStorageof::checkAndLabelType();
    if(m_state.okUTItoContinue(nuti))
      {
	//assert(m_varSymbol);
	//UTI vuti = m_varSymbol->getUlamTypeIdx();
	//bool isself = m_varSymbol->isSelf(); //a ref
	UTI vuti = m_nodeOf->getNodeType();
	bool isself = m_nodeOf->hasASymbolSelf();
	UTI oftype = NodeStorageof::getOfType();
	UlamType * ofut = m_state.getUlamTypeByIndex(oftype);

	//refs checked at runtime; non-refs here:
	if(!m_state.isReference(vuti) && !isself)
	  {
	    //only an element or atom have real storage (ie. not quarks)
	    if(ofut->getUlamClassType() == UC_QUARK)
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
#if 0
		else
		  {
		    // since can parse a.b.c.atomof; cuti no longer necessarily the storage;
		    // figure it out later..TBD.
		    UTI cuti = m_state.getCompileThisIdx();
		    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
		    std::ostringstream msg;
		    msg << "<" << m_nodeOf->getName();
		    msg << "> is a data member of ";
		    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		    if(cut->getUlamClassType() == UC_QUARK)
		      {
			msg << "; Quarks cannot be used with " << getName();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			setNodeType(Nav);
		      }
		    else if(cut->getUlamClassType() == UC_TRANSIENT)
		      {
			msg << "; Transients cannot be used with " << getName();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			setNodeType(Nav);
		      }
		    else
		      {
			msg << "; 'self' used with " << getName();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		      }
		  }
#endif
	      }
	    else if(ofut->getUlamClassType() == UC_TRANSIENT)
	      {
		std::ostringstream msg;
		msg << "<" << m_nodeOf->getName();
		msg << "> is a transient";
		msg << "; Transients cannot be used with " << getName();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav); //e.g. error/t3761
	      }
	  } //not a reference done at compile time.
	else if(ofut->getUlamClassType() == UC_TRANSIENT)
	  {
	    std::ostringstream msg;
	    msg << "<"  << m_nodeOf->getName();
	    msg << "> is a transient";
	    msg << "; Transients cannot be used with " << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav); //e.g. error/t3762
	  }
      }
    return getNodeType(); //UAtomRef (storeintoable)
  } //checkAndLabelType

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
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref

    //assert(m_varSymbol);
    //UTI vuti = m_varSymbol->getUlamTypeIdx();
    //bool isself = m_varSymbol->isSelf();
    //bool issuper = m_varSymbol->isSuper();
    //bool isaref = (m_state.isReference(vuti) || isself || issuper);
    //if var is a data member quark, then also isself
    assert(m_nodeOf);
    m_nodeOf->genCodeToStoreInto(fp, uvpass);
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
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
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //for C++ local vars
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPTATOM).c_str());
    fp->write(" = ");

    //data member's storage is self (not a ref)
    //if(m_varSymbol->isDataMember())
    if(cos->isDataMember())
      fp->write("ur");
    else
      //fp->write(m_varSymbol->getMangledName().c_str()); //element or atom, ur for self/super
      fp->write(cos->getMangledName().c_str()); //element or atom, ur for self/super

    fp->write(".ReadAtom(); //atomof"); GCNL; //can't be const

    //uvpass = UVPass::makePass(tmpVarNum, TMPTATOM, nuti, UNPACKED, m_state, 0, m_varSymbol ? m_varSymbol->getId() : 0);
    uvpass = UVPass::makePass(tmpVarNum, TMPTATOM, nuti, UNPACKED, m_state, 0, cos ? cos->getId() : 0);
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
  } //genCode

  void NodeAtomof::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    //lhs
    assert(getStoreIntoAble() == TBOOL_TRUE);
    UTI nuti = getNodeType(); //UAtomRef
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref

    //assert(m_varSymbol);
    //UTI vuti = m_varSymbol->getUlamTypeIdx();
    //bool isself = m_varSymbol->isSelf();
    //bool issuper = m_varSymbol->isSuper();
    //bool isaref = (m_state.isReference(vuti) || isself || issuper);
    //if var is a data member quark, then also isself (or issuper)
    assert(m_nodeOf);
    m_nodeOf->genCodeToStoreInto(fp, uvpass);
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
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
      fp->write("ur");
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
  } //genCodeToStoreInto

} //end MFM
