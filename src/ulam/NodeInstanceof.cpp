#include <stdio.h>
#include "NodeInstanceof.h"
#include "CompilerState.h"

namespace MFM {

  NodeInstanceof::NodeInstanceof(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeStorageof(ofnode, nodetype, state), m_tmpvarSymbol(NULL) { }

  NodeInstanceof::NodeInstanceof(const NodeInstanceof& ref) : NodeStorageof(ref), m_tmpvarSymbol(NULL) { }

  NodeInstanceof::~NodeInstanceof()
  {
    delete m_tmpvarSymbol;
    m_tmpvarSymbol = NULL;
  }

  Node * NodeInstanceof::instantiate()
  {
    return new NodeInstanceof(*this);
  }

  const char * NodeInstanceof::getName()
  {
    return ".instanceof";
  }

  const std::string NodeInstanceof::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  u32 NodeInstanceof::getPositionOf()
  {
    return 0u;
  }

  FORECAST NodeInstanceof::safeToCastTo(UTI newType)
  {
    UTI nuti = getNodeType(); //UAtom for references; ow type of class/atom
    if(m_state.isAtom(nuti))
      return NodeStorageof::safeToCastTo(newType);
    return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);
  } //safeToCastTo

  UTI NodeInstanceof::checkAndLabelType(Node * thisparentnode)
  {
    NodeStorageof::checkAndLabelType(thisparentnode);

    UTI oftype = NodeStorageof::getOfType(); //deref'd
    if(m_state.okUTItoContinue(oftype))
      {
	//a virtual function (instanceof), behaves differently on refs vs object
	bool isself = m_nodeOf ? (m_nodeOf->hasASymbolSelf() && !m_nodeOf->isAMemberSelect()) : false; //t41092
	bool issuper = m_nodeOf ? (m_nodeOf->hasASymbolSuper()) : false;
	bool isaref = m_state.isAltRefType(oftype) || isself || issuper;

	if(isaref) //all refs
	  setNodeType(UAtom); //effective type known only at runtime (t3676)
	else
	  setNodeType(oftype);

	Node::setStoreIntoAble(TBOOL_TRUE); //why not? allows instanceof() w args (t41085)

	//if(!isaref)
	Node::setReferenceAble(TBOOL_FALSE); //t3660

      }
    return getNodeType();
  } //checkAndLabelType

  UlamValue NodeInstanceof::makeUlamValuePtr()
  {
    // (from NodeVarDecl's makeUlamValuePtr)
    UlamValue ptr;
    UlamValue atomuv;

    UTI auti = getNodeType(); //t41469
    UlamType * aut = m_state.getUlamTypeByIndex(auti);
    ULAMCLASSTYPE aclasstype = aut->getUlamClassType();

    u32 atop = 1;
    atop = m_state.m_funcCallStack.getAbsoluteStackIndexOfSlot(atop);
    if(m_state.isAtom(auti))
      {
	//ofnode is a ref, super or self; and uses effself regardless.
	UlamValue getatomtypeuv = NodeStorageof::evalAtomOfExpr();  //t3286
	UTI atomtype = getatomtypeuv.getUlamValueTypeIdx();
	bool atomisPtr = m_state.isPtr(atomtype);

	//use effself
	if(atomisPtr)
	  atomtype = getatomtypeuv.getPtrTargetEffSelfType();
	else
	  atomtype = getatomtypeuv.getUlamValueEffSelfTypeIdx(); //t3286

	//if no effself, use ulamvalue type, assuming same
	if(atomtype == Nouti)
	  {
	    if(atomisPtr)
	      atomtype = getatomtypeuv.getPtrTargetType();
	    else
	      getatomtypeuv.getUlamValueTypeIdx();
	  }

	assert(atomtype != Nouti); //sanity
	if(!m_state.isAtom(atomtype))
	  {
	    ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(atomtype)->getUlamClassType();
	    if(classtype == UC_ELEMENT)
	      {
		atomuv = UlamValue::makeDefaultAtom(atomtype, m_state); //t3747, effself is atomtype
		atomuv.setUlamValueTypeIdx(auti); //t3747, still an atom
	      }
	    else
	      auti = Nav; //error t3676
	  }
	else
	  auti = Nav; //error, atom type
      }
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

    if(!m_state.okUTItoContinue(auti))
      return atomuv; //error (t3676)

    m_state.m_funcCallStack.storeUlamValueAtStackIndex(atomuv, atop); //stackframeslotindex ?

    ptr = UlamValue::makePtr(atop, STACK, auti, m_state.determinePackable(auti), m_state, 0);
    if(m_state.isAtom(auti))
      {
	assert(!m_state.isPtr(atomuv.getUlamValueTypeIdx())); //sanity
	UTI atomuveffself = atomuv.getUlamValueEffSelfTypeIdx();
	ptr.setPtrTargetEffSelfType(atomuveffself, m_state); //t3747
      }
    else
      {
	ptr.setPtrTargetEffSelfType(auti, m_state); //t41318, t41384
      }

    ptr.setUlamValueTypeIdx(PtrAbs);

    return ptr;
  } //makeUlamValuePtr

  void NodeInstanceof::genCode(File * fp, UVPass& uvpass)
  {
    //generates a new instance of..
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    //starts out as its default type; references (UAtom) are updated:
    m_state.indentUlamCode(fp); //non-const
    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
    fp->write(";"); GCNL;

    // a reference (including 'self'), returns a UAtom of effective type;
    // SINCE effective self type is known only at runtime.
    if(m_nodeOf)
      {
	m_nodeOf->genCodeToStoreInto(fp, uvpass);
	assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	UTI cosuti = cos->getUlamTypeIdx();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

	bool isself = stgcos->isSelf();
	bool issuper = stgcos->isSuper();
	bool isaref = m_state.isAltRefType(cosuti) || isself || issuper;

	if(isaref)
	  {
	    u32 tmpuclass = m_state.getNextTmpVarNumber(); //only for this case
	    m_state.indentUlamCode(fp);
	    fp->write("const UlamClass<EC> * ");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(" = ");
	    fp->write(cos->getMangledName().c_str());
	    fp->write(".GetEffectiveSelf();"); GCNL;

	    //primitive FAILS
	    m_state.indentUlamCode(fp);
	    fp->write("if(");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(" == NULL) FAIL(ILLEGAL_ARGUMENT); //non-class"); GCNL;

	    //an immediate default quark FAILS
	    m_state.indentUlamCode(fp);
	    fp->write("if(");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write("->AsUlamQuark() != NULL) ");
	    fp->write("FAIL(NOT_AN_ELEMENT); //quark"); GCNL;

	    m_state.indentUlamCode(fp);
	    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	    fp->write(".WriteAtom(");
	    fp->write("((UlamElement<EC> *) ");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(")->GetDefaultAtom()); //instanceof default element"); GCNL;
	  }
	else if(m_state.isAtom(nuti))
	  {
	    u32 tmpuclass = m_state.getNextTmpVarNumber(); //only for this case
	    m_state.indentUlamCode(fp);
	    fp->write("const UlamClass<EC> * ");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(" = ");
	    fp->write("uc.LookupUlamElementTypeFromContext(");
	    fp->write(cos->getMangledName().c_str());
	    fp->write(".GetType()");
	    fp->write(");"); GCNL;

	    m_state.indentUlamCode(fp);
	    fp->write("if(");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(" == NULL) FAIL(ILLEGAL_ARGUMENT); //non-class"); GCNL;

	    m_state.indentUlamCode(fp);
	    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	    fp->write(".WriteAtom(");
	    fp->write("((UlamElement<EC> *) ");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(")->GetDefaultAtom()); //instanceof default element"); GCNL;
	  }
      }

    if(m_state.isAtom(nuti))
      {
	// THE READ:
	s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp to read into
	TMPSTORAGE rstor = nut->getTmpStorageTypeForTmpVar();

	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(nut->getTmpStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, rstor).c_str());
	fp->write(" = ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write(".");
	fp->write("read();"); GCNL;

	uvpass = UVPass::makePass(tmpVarNum2, rstor, nuti, nut->getPackable(), m_state, 0, cos ? cos->getId() : 0);
      }
    else //element and uvpass stays the same (a default immediate element).
      uvpass = UVPass::makePass(tmpVarNum, TMPBITVAL, nuti, nut->getPackable(), m_state, 0, cos ? cos->getId() : 0); //t3657

    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
  } //genCode

  void NodeInstanceof::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    //lhs
    //assert(getStoreIntoAble() == TBOOL_TRUE); //not so..why not? (t41503) constantof defaultvalue
    genCode(fp, uvpass); //t41085

    //tmp variable becomes the object of the constructor call (t41085)
    m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL);
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
  } //genCodeToStoreInto

} //end MFM
