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

  UTI NodeVarRefAs::checkAndLabelType()
  {
    UTI it = NodeVarRef::checkAndLabelType();
    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  void NodeVarRefAs::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert(0); //refs can't be data members
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
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    assert(m_varSymbol->getUlamTypeIdx() == nuti);
    assert(!m_state.isAtom(nuti)); //rhs type of conditional as/has can't be an atom

    UlamValue pluv = m_state.m_currentAutoObjPtr;
    ((SymbolVariableStack *) m_varSymbol)->setAutoPtrForEval(pluv); //for future ident eval uses

    UTI luti = pluv.getPtrTargetType();
    assert(m_state.okUTItoContinue(luti));
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClass();
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

    // (from NodeIdent's makeUlamValuePtr)
    // return ptr to this data member within the m_currentObjPtr
    // 'pos' modified by this data member symbol's packed bit position
    UlamValue rtnUVPtr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  }

  // this is the auto local variable's node, created at parse time,
  // for Conditional-As case.
  void NodeVarRefAs::genCode(File * fp, UlamValue & uvpass)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    // the uvpass comes from NodeControl, and still has the POS obtained
    // during the condition statement for As..unorthodox, but necessary.
    assert(m_state.isPtr(uvpass.getUlamValueTypeIdx()));

    // before shadowing the lhs of the h/as-conditional variable with its auto,
    // let's load its storage from the currentSelfSymbol:
    s32 tmpVarStg = m_state.getNextTmpVarNumber();
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMTYPE stgetype = stgut->getUlamTypeEnum();
    assert((stgetype == UAtom) || (stgut->getUlamClass() == UC_ELEMENT)); //not quark

    // can't let Node::genCodeReadIntoTmpVar do this for us: it's a ref.
    assert(m_state.m_currentObjSymbolsForCodeGen.size() == 1);
    m_state.indent(fp);
    fp->write(stgut->getTmpStorageTypeAsString().c_str());
    fp->write("& "); //here it is!!
    fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());
    fp->write(" = ");
    fp->write(stgcos->getMangledName().c_str());
    fp->write(".GetStorage()"); //non-const
    fp->write(";\n");

    // now we have our pos in tmpVarPos, and our T in tmpVarStg
    // time to shadow 'self' with auto local variable:
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    m_state.indent(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());

    if(stgetype == UAtom)
      {
	fp->write(", ");
	if(vclasstype == UC_QUARK) //t3639
	  fp->write("0u, ");
	fp->write(m_state.getHiddenContextArgName());
	fp->write(".LookupElementTypeFromContext(");
	fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str()); //t3636
	fp->write(".GetType())");
      }
    else
      {
	if(vclasstype == UC_QUARK)
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset()); //should be 0!
	    fp->write("u");
	  }
	fp->write(", &");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(stgcosuti).c_str());
      }
    fp->write("); //shadows lhs of 'as'\n");

    m_state.m_genCodingConditionalHas = false; // done
    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs ?
  } //genCode

} //end MFM
