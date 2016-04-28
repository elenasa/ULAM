#include <stdio.h>
#include "NodeInstanceof.h"
#include "CompilerState.h"

namespace MFM {

  NodeInstanceof::NodeInstanceof(Token tokof, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeAtomof(tokof, nodetype, state) { }

  NodeInstanceof::NodeInstanceof(const NodeInstanceof& ref) : NodeAtomof(ref) { }

  NodeInstanceof::~NodeInstanceof() { }

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

  FORECAST NodeInstanceof::safeToCastTo(UTI newType)
  {
    UTI nuti = getNodeType(); //UAtom for references; ow type of class/atom
    if(m_state.isAtom(nuti))
      return NodeAtomof::safeToCastTo(newType);
    return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);
  } //safeToCastTo

  UTI NodeInstanceof::checkAndLabelType()
  {
    NodeAtomof::checkAndLabelType();

    UTI oftype = NodeAtomof::getOfType();
    if(m_state.okUTItoContinue(oftype))
      {
	bool isself = m_varSymbol ? m_varSymbol->isSelf() : false;
	if(m_state.isReference(oftype) || isself) //including 'self'
	  setNodeType(UAtom); //effective type known only at runtime
	else
	  setNodeType(oftype); //Type or variable
      }
    Node::setStoreIntoAble(TBOOL_FALSE);
    return getNodeType();
  } //checkAndLabelType

  UlamValue NodeInstanceof::makeUlamValuePtr()
  {
    // (from NodeVarDecl's makeUlamValuePtr)
    UlamValue ptr;
    UlamValue atomuv;

    UTI auti = getOfType();
    UlamType * aut = m_state.getUlamTypeByIndex(auti);
    ULAMCLASSTYPE aclasstype = aut->getUlamClassType();

    u32 atop = 1;
    atop = m_state.m_funcCallStack.getAbsoluteStackIndexOfSlot(atop);
    if(m_state.isAtom(auti))
      atomuv = UlamValue::makeAtom(auti);
    else if(aclasstype == UC_ELEMENT)
      atomuv = UlamValue::makeDefaultAtom(auti, m_state);
    else if(aclasstype == UC_QUARK)
      {
	u32 dq = 0;
	AssertBool isDefinedQuark = m_state.getDefaultQuark(auti, dq); //returns scalar dq
	assert(isDefinedQuark);
	atomuv = UlamValue::makeImmediateClass(auti, dq, aut->getTotalBitSize());
      }
    else if(aclasstype == UC_TRANSIENT)
      {
	assert(0);
      }
    else
      assert(0);

    m_state.m_funcCallStack.storeUlamValueAtStackIndex(atomuv, atop); //stackframeslotindex ?

    //ptr = UlamValue::makePtr(atop, STACK, auti, UNPACKED, m_state, 0); ??
    ptr = UlamValue::makePtr(atop, STACK, auti, m_state.determinePackable(auti), m_state, 0);
    ptr.setUlamValueTypeIdx(PtrAbs);
    return ptr;
  } //makeUlamValuePtr

  void NodeInstanceof::genCode(File * fp, UVPass& uvpass)
  {
    //generates a new instance of..
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref

    //starts out as its default type; references (UAtom) are updated:
    m_state.indent(fp); //non-const
    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
    fp->write(";\n");

    // a reference (including 'self'), returns a UAtom of effective type;
    // SINCE effective self type is known only at runtime.
    if((m_token.m_type == TOK_IDENTIFIER))
      {
	assert(m_varSymbol);
	UTI vuti = m_varSymbol->getUlamTypeIdx();
	bool isself = m_varSymbol->isSelf();

	if(m_state.isReference(vuti) || isself)
	  {
	    u32 tmpuclass = m_state.getNextTmpVarNumber(); //only for this case
	    m_state.indent(fp);
	    fp->write("const UlamClass<EC> * ");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(" = ");
	    fp->write(m_varSymbol->getMangledName().c_str());
	    fp->write(".GetEffectiveSelf();\n");

	    //primitive FAILS
	    m_state.indent(fp);
	    fp->write("if(");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(" == NULL) FAIL(ILLEGAL_ARGUMENT); //non-class\n");

	    //an immediate default quark FAILS
	    m_state.indent(fp);
	    fp->write("if(");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write("->AsUlamQuark() != NULL) ");
	    fp->write("FAIL(ILLEGAL_ARGUMENT); //quark\n");

	    m_state.indent(fp);
	    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	    fp->write(".WriteAtom(");
	    fp->write("((UlamElement<EC> *) ");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(")->GetDefaultAtom()); //instanceof default element\n");
	  }
	else if(m_state.isAtom(nuti))
	  {
	    u32 tmpuclass = m_state.getNextTmpVarNumber(); //only for this case
	    m_state.indent(fp);
	    fp->write("const UlamClass<EC> * ");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(" = ");
	    fp->write("uc.LookupElementTypeFromContext(");
	    fp->write(m_varSymbol->getMangledName().c_str());
	    fp->write(".GetType()");
	    fp->write(");\n");

	    m_state.indent(fp);
	    fp->write("if(");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(" == NULL) FAIL(ILLEGAL_ARGUMENT); //non-class\n");

	    m_state.indent(fp);
	    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	    fp->write(".WriteAtom(");
	    fp->write("((UlamElement<EC> *) ");
	    fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	    fp->write(")->GetDefaultAtom()); //instanceof default element\n");
	  }
      }

    if(m_state.isAtom(nuti))
      {
	// THE READ:
	s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp to read into
	TMPSTORAGE rstor = nut->getTmpStorageTypeForTmpVar();

	m_state.indent(fp);
	fp->write("const ");
	fp->write(nut->getTmpStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, rstor).c_str());
	fp->write(" = ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write(".");
	fp->write("read();\n");

	uvpass = UVPass::makePass(tmpVarNum2, rstor, nuti, nut->getPackable(), m_state, 0, m_varSymbol ? m_varSymbol->getId() : 0);
      }
    else //element and uvpass stays the same (a default immediate element).
      uvpass = UVPass::makePass(tmpVarNum, TMPBITVAL, nuti, nut->getPackable(), m_state, 0, m_varSymbol ? m_varSymbol->getId() : 0); //t3657

    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
  } //genCode

  void NodeInstanceof::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    //lhs
    assert(getStoreIntoAble() == TBOOL_TRUE); //not so.
  } //genCodeToStoreInto

} //end MFM
