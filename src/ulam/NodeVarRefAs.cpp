#include <stdlib.h>
#include "NodeVarRefAs.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"

namespace MFM {

  NodeVarRefAs::NodeVarRefAs(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarRef(sym, nodetype, state) { }

  NodeVarRefAs::NodeVarRefAs(const NodeVarRefAs& ref) : NodeVarRef(ref) { }

  NodeVarRefAs::~NodeVarRefAs() { }

  Node * NodeVarRefAs::instantiate()
  {
    return new NodeVarRefAs(*this);
  }

  void NodeVarRefAs::updateLineage(NNO pno)
  {
    NodeVarRef::updateLineage(pno);
  } //updateLineage

  bool NodeVarRefAs::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeVarRef::findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  //see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarRefAs::printPostfix(File * fp)
  {
    NodeVarRef::printPostfix(fp);
  } //printPostfix

  const char * NodeVarRefAs::getName()
  {
    return NodeVarRef::getName(); //& ??
  }

  const std::string NodeVarRefAs::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeVarRefAs::safeToCastTo(UTI newType)
  {
    return CAST_CLEAR; //created automatically when 'as' is true
  }

  bool NodeVarRefAs::checkReferenceCompatibility(UTI uti)
  {
    assert(m_state.okUTItoContinue(uti));
    assert(m_state.getUlamTypeByIndex(uti)->isReference());
    return true; //ok
  } //checkReferenceCompatibility

  UTI NodeVarRefAs::checkAndLabelType()
  {
    UTI it = NodeVarRef::checkAndLabelType();
    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  TBOOL NodeVarRefAs::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_state.abortShouldntGetHere(); //refs can't be data members
    return TBOOL_FALSE;
  } //packBitsInOrderOfDeclaration

  void NodeVarRefAs::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    assert(m_varSymbol);
    return NodeVarRef::calcMaxDepth(depth, maxdepth, base);
  } //calcaMaxDepth

  void NodeVarRefAs::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeVarRef::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  EvalStatus NodeVarRefAs::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    assert(m_varSymbol->getUlamTypeIdx() == nuti);
    assert(!m_state.isAtom(nuti)); //rhs type of conditional as/has can't be an atom

    UlamValue pluv = m_state.m_currentAutoObjPtr;
    ((SymbolVariableStack *) m_varSymbol)->setAutoPtrForEval(pluv); //for future ident eval uses

    UTI luti = pluv.getPtrTargetType();
    assert(m_state.okUTItoContinue(luti));
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClassType();
    UTI autostgtype = m_state.m_currentAutoStorageType;
    if((UlamType::compare(autostgtype, UAtom, m_state) == UTIC_SAME) && (lclasstype == UC_ELEMENT))
       autostgtype = luti; //e.g. funccall expects a class, not an atom (t3636)

    ((SymbolVariableStack *) m_varSymbol)->setAutoStorageTypeForEval(autostgtype); //for future virtual function call eval uses

    //m_state.m_funcCallStack.storeUlamValueInSlot(pluv, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex()); //doesn't seem to matter..

    return NORMAL;
  } //eval

  EvalStatus  NodeVarRefAs::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  }

  // this is the auto local variable's node, created at parse time,
  // for Conditional-As case.
  void NodeVarRefAs::genCode(File * fp, UVPass & uvpass)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    // the uvpass comes from NodeControl, and still has the POS obtained
    // during the condition statement for As..unorthodox, but necessary.

    // before shadowing the lhs of the h/as-conditional variable with its auto,
    // let's load its storage from the currentSelfSymbol:
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMTYPE stgetyp = stgcosut->getUlamTypeEnum();
    ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClassType();

    assert((stgetyp == UAtom) || (stgetyp == Class)); //lhs

    if(stgcos->isSelf())
      return genCodeRefAsSelf(fp, uvpass);

    s32 tmpVarStg = m_state.getNextTmpVarNumber();

    // can't let Node::genCodeReadIntoTmpVar do this for us: need a ref.
    assert(m_state.m_currentObjSymbolsForCodeGen.size() == 1);
    m_state.indentUlamCode(fp);
    fp->write(stgcosut->getUlamTypeImmediateMangledName().c_str());
    fp->write("<EC> & "); //here it is!! brilliant
    fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());
    fp->write(" = ");
    fp->write(stgcos->getMangledName().c_str());
    fp->write("; //c++ reference to immediate"); GCNL;

    // now we have our pos in tmpVarPos, and our T in tmpVarStg
    // time to shadow 'self' with auto local variable:
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();

    m_state.indentUlamCode(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());

    if(stgetyp == UAtom)
      {
	fp->write(", 0u + T::ATOM_FIRST_STATE_BIT, "); //position as super quark (e.g. t3639, t3709, t3675, t3408, t3336); as element t3249, t3255, t3637; as atom ref t3908

	//note: needs effective self of the atom, not simply the RHS type.
	fp->write(m_state.getHiddenContextArgName());
	fp->write(".LookupUlamElementTypeFromContext(");
	fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str()); //t3636
	fp->write(".GetType())");
	if(vclasstype == UC_QUARK)
	  fp->write(", UlamRef<EC>::ELEMENTAL"); //becomes elemental, t3835
      }
    else if((stgclasstype == UC_ELEMENT))
      {
	if(stgcosut->isReference()) //not isAltRefType
	  {
	    fp->write(", 0u, "); //t3655
	    fp->write(stgcos->getMangledName().c_str()); //stg
	    fp->write(".GetEffectiveSelf()"); //Sat Jun 18 17:30:20 2016
	  }
	else
	  {
	    fp->write(", 0u + T::ATOM_FIRST_STATE_BIT, &"); //t3586, t3589, t3637
	    //must be same as look up for elements only Sat Jun 18 17:30:17 2016
	    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
	  }
	if(vclasstype == UC_QUARK)
	  fp->write(", UlamRef<EC>::ELEMENTAL"); //stays elemental
      }
    else if((stgclasstype == UC_TRANSIENT))
      {
	// transient can be another transient or a quark, not an element
	fp->write(", 0u, ");
	if(stgcosut->isReference()) //not isAltRefType
	  {
	    fp->write(stgcos->getMangledName().c_str()); //stg
	    fp->write(".GetEffectiveSelf()"); //t3824
	  }
	else
	  {
	    fp->write("&"); //t3822
	    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
	  }
	if(vclasstype == UC_QUARK)
	  fp->write(", UlamRef<EC>::CLASSIC"); //stays classic
      }
    else if((stgclasstype == UC_QUARK))
      {
	// quark can be another quark, not an element, nor transient
	fp->write(", 0u, ");
	if(stgcosut->isReference()) //not isAltRefType
	  {
	    fp->write(stgcos->getMangledName().c_str()); //stg
	    fp->write(".GetEffectiveSelf()"); //tt3829
	  }
	else
	  {
	    fp->write("&"); //t3830
	    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
	  }
	assert(vclasstype == UC_QUARK);
	fp->write(", UlamRef<EC>::CLASSIC"); //stays classic
      }
    else
      m_state.abortUndefinedUlamClassType(); //WHAT THEN???

    if(!stgcosut->isReference()) //not isAltRefType
      fp->write(", uc"); //t3249

    fp->write("); //shadows lhs of 'as'"); GCNL;

    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs ?
  } //genCode

  void NodeVarRefAs::genCodeRefAsSelf(File * fp, UVPass& uvpass)
  {
    //no tmpref needed since 'self' (i.e. ur) is already a C++ reference
    //t3821, t3815 (transient), t3828 (quark)
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indentUlamCode(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");
    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("(");
    fp->write(stgcos->getMangledName().c_str()); //stg
    fp->write(", 0u, ");
    fp->write(stgcos->getMangledName().c_str()); //stg
    fp->write(".GetEffectiveSelf()");
    fp->write("); //shadows lhs of 'as'"); GCNL;

    m_state.indentUlamCode(fp);
    fp->write("UlamRef<EC>& ur = ");
    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("; //shadows self"); GCNL;

    m_varSymbol->setIsSelf(); //nope

    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeRefAsSelf

} //end MFM
