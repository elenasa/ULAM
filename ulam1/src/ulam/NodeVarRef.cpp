#include <stdlib.h>
#include "NodeVarRef.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"

namespace MFM {

  NodeVarRef::NodeVarRef(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarDecl(sym, nodetype, state), m_storageExpr(NULL) { }

  NodeVarRef::NodeVarRef(const NodeVarRef& ref) : NodeVarDecl(ref), m_storageExpr(NULL)
  {
    if(ref.m_storageExpr)
      m_storageExpr = ref.m_storageExpr->instantiate();
  }

  NodeVarRef::~NodeVarRef()
  {
    delete m_storageExpr;
    m_storageExpr = NULL;
  }

  Node * NodeVarRef::instantiate()
  {
    return new NodeVarRef(*this);
  }

  void NodeVarRef::updateLineage(NNO pno)
  {
    NodeVarDecl::updateLineage(pno);
    if(m_storageExpr)
      m_storageExpr->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeVarRef::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeVarDecl::findNodeNo(n, foundNode))
      return true;
    if(m_storageExpr && m_storageExpr->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeVarRef::checkAbstractInstanceErrors()
  {
    //unlike NodeVarDecl, an abstract class can be a reference!
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    if(nut->getUlamTypeEnum() == Class)
      {
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
	assert(isDefined);
	if(csym->isAbstract())
	  {
	    std::ostringstream msg;
	    msg << "Instance of Abstract Class ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " used with reference variable symbol name '";
	    msg << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
	  }
      }
  } //checkAbstractInstanceErrors

  //see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarRef::printPostfix(File * fp)
  {
    printTypeAndName(fp);
    if(m_storageExpr)
      {
	fp->write(" =");
	m_storageExpr->printPostfix(fp);
      }
    fp->write("; ");
  } //printPostfix

  void NodeVarRef::printTypeAndName(File * fp)
  {
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(vuti);
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    fp->write(" ");
    if(vut->getUlamTypeEnum() != Class)
      fp->write(vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
    else
      fp->write(vut->getUlamTypeNameBrief().c_str());

    fp->write("& ");
    fp->write(getName());

    s32 arraysize = m_state.getArraySize(vuti);
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }
  } //printTypeAndName

  const char * NodeVarRef::getName()
  {
    return NodeVarDecl::getName(); //& ??
  }

  const std::string NodeVarRef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeVarRef::setStorageExpr(Node * node)
  {
    m_storageExpr = node;
    m_storageExpr->updateLineage(getNodeNo()); //for unknown subtrees
  }

  bool NodeVarRef::hasStorageExpr()
  {
    return (m_storageExpr != NULL);
  }

  bool NodeVarRef::foldStorageExpression()
  {
    assert(0); //TBD;
  }

  UTI NodeVarRef::checkAndLabelType()
  {
    UTI it = NodeVarDecl::checkAndLabelType();

    ////requires non-constant, non-funccall value
    //NOASSIGN REQUIRED (e.g. for function parameters) doesn't have to have this!
    if(m_storageExpr)
      {
	it = m_storageExpr->checkAndLabelType();
	if(it == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", is invalid";
	    if(!m_storageExpr->isStoreIntoAble())
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //possibly still hazy
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	//if(m_storageExpr->isStoreIntoAble())
	Symbol * storsym = NULL;
	if(m_storageExpr->getSymbolPtr(storsym) && (storsym->isConstant() || storsym->isFunction()))
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", must be storeintoable";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }
      }
    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  void NodeVarRef::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert(0); //refs can't be data members
  } //packBitsInOrderOfDeclaration

  void NodeVarRef::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    assert(m_varSymbol);
    s32 newslot = depth + base;
    s32 oldslot = ((SymbolVariable *) m_varSymbol)->getStackFrameSlotIndex();
    if(oldslot != newslot)
      {
	std::ostringstream msg;
	msg << "'" << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "' was at slot: " << oldslot << ", new slot is " << newslot;
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	((SymbolVariable *) m_varSymbol)->setStackFrameSlotIndex(newslot);
      }
    depth += m_state.slotsNeeded(getNodeType());
  } //calcMaxDepth

  void NodeVarRef::countNavNodes(u32& cnt)
  {
    if(m_storageExpr)
      m_storageExpr->countNavNodes(cnt);

    Node::countNavNodes(cnt);
  } //countNavNodes

  EvalStatus NodeVarRef::eval()
  {
    assert(m_varSymbol);

    if(m_varSymbol->getAutoLocalType() == ALT_AS)
      return evalAutoLocal();

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    assert(m_varSymbol->getUlamTypeIdx() == nuti);
    assert(m_storageExpr);

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr

    EvalStatus evs = m_storageExpr->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.popArg();
    ((SymbolVariableStack *) m_varSymbol)->setAutoPtrForEval(pluv); //for future ident eval uses
    ((SymbolVariableStack *) m_varSymbol)->setAutoStorageTypeForEval(m_storageExpr->getNodeType()); //for future virtual function call eval uses

    m_state.m_funcCallStack.storeUlamValueInSlot(pluv, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex()); //doesn't seem to matter..

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeVarRef::evalAutoLocal()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    assert(m_varSymbol->getUlamTypeIdx() == nuti);
    assert(nuti != UAtom); //rhs type of conditional as/has can't be an atom

    UlamValue pluv = m_state.m_currentAutoObjPtr;
    ((SymbolVariableStack *) m_varSymbol)->setAutoPtrForEval(pluv); //for future ident eval uses
    ((SymbolVariableStack *) m_varSymbol)->setAutoStorageTypeForEval(m_state.m_currentAutoStorageType); //for future virtual function call eval uses

    m_state.m_funcCallStack.storeUlamValueInSlot(pluv, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex()); //doesn't seem to matter..

    return NORMAL;
  } //evalAutoLocal

  EvalStatus  NodeVarRef::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    // (from NodeIdent's makeUlamValuePtr)
    // return ptr to this data member within the m_currentObjPtr
    // 'pos' modified by this data member symbol's packed bit position
    UlamValue rtnUVPtr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  }

  void NodeVarRef::genCode(File * fp, UlamValue & uvpass)
  {
    if(m_varSymbol->getAutoLocalType() == ALT_AS)
      return genCodedAutoLocal(fp, uvpass);

    //reference
    assert(m_varSymbol->isAutoLocal());
    if(m_storageExpr)
      {
	m_storageExpr->genCodeToStoreInto(fp, uvpass);

	assert(m_state.m_currentObjSymbolsForCodeGen.size() == 1);
	Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen.back();

	UTI vuti = m_varSymbol->getUlamTypeIdx();
	UlamType * vut = m_state.getUlamTypeByIndex(vuti);
	ULAMCLASSTYPE vclasstype = vut->getUlamClass();

	assert(vuti == stgcos->getUlamTypeIdx());

	m_state.indent(fp);
	fp->write(vut->getUlamTypeImmediateAutoMangledName().c_str()); //for C++ local vars, ie non-data members
	if(vclasstype == UC_ELEMENT)
	  fp->write("<EC> ");
	else if(vclasstype == UC_QUARK)
	  {
	    fp->write("<EC, ");
	    fp->write_decimal_unsigned(uvpass.getPtrPos());
	    fp->write("u> ");
	  }
	else //primitive
	  {
	    fp->write("<EC, ");
	    if(stgcos->isDataMember())
	      fp->write("T::ATOM_FIRST_STATE_BIT + ");

	    fp->write_decimal_unsigned(uvpass.getPtrPos());
	    fp->write("u> ");
	  }

	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write("("); //pass ref in constructor (ref's not assigned with =)
	if(stgcos->isDataMember())
	  {
	    fp->write("Uv_4atom, ");
	    fp->write_decimal_unsigned(stgcos->getPosOffset());
	  }
	else
	  {
	    fp->write(stgcos->getMangledName().c_str());
	    if(stgcos->getId() != m_state.m_pool.getIndexForDataString("atom")) //not isSelf check; was "self"
	      fp->write(".getRef()");
	    fp->write(", ");
	    if(vclasstype == UC_NOTACLASS)
	      fp->write_decimal_unsigned(BITSPERATOM - vut->getTotalBitSize()); //right-justified
	    else
	      fp->write(", 0"); //left-justified
	  }
	fp->write("u);\n"); //func call parameters aren't NodeVarDecl's
      } //storage

    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of rhs ?
  } //genCode

  // this is the auto local variable's node, created at parse time,
  // for Conditional-As case.
  void NodeVarRef::genCodedAutoLocal(File * fp, UlamValue & uvpass)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    // the uvpass comes from NodeControl, and still has the POS obtained
    // during the condition statement for As..unorthodox, but necessary.
    assert(uvpass.getUlamValueTypeIdx() == Ptr);
    s32 tmpVarPos = uvpass.getPtrSlotIndex();

    // before shadowing the lhs of the h/as-conditional variable with its auto,
    // let's load its storage from the currentSelfSymbol:
    s32 tmpVarStg = m_state.getNextTmpVarNumber();
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stguti = stgcos->getUlamTypeIdx();
    UlamType * stgut = m_state.getUlamTypeByIndex(stguti);
    assert(stguti == UAtom || stgut->getUlamClass() == UC_ELEMENT); //not quark

    // can't let Node::genCodeReadIntoTmpVar do this for us: it's a ref.
    assert(m_state.m_currentObjSymbolsForCodeGen.size() == 1);
    m_state.indent(fp);
    fp->write(stgut->getTmpStorageTypeAsString().c_str());
    fp->write("& "); //here it is!!
    fp->write(m_state.getTmpVarAsString(stguti, tmpVarStg, TMPBITVAL).c_str());
    fp->write(" = ");
    fp->write(m_state.m_currentObjSymbolsForCodeGen[0]->getMangledName().c_str());

    if(m_varSymbol->getId() != m_state.m_pool.getIndexForDataString("atom")) //not isSelf check; was "self"
      fp->write(".getRef()");
    fp->write(";\n");

    // now we have our pos in tmpVarPos, and our T in tmpVarStg
    // time to shadow 'self' with auto local variable:
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    m_state.indent(fp);
    fp->write(vut->getUlamTypeImmediateAutoMangledName().c_str()); //for C++ local vars, ie non-data members
    if(vclasstype == UC_ELEMENT)
      fp->write("<EC> ");
    else //QUARK
      {
	fp->write("<EC, ");
	fp->write_decimal_unsigned(m_varSymbol->getPosOffset()); //POS should be 0+25 for inheritance
	fp->write("u + T::ATOM_FIRST_STATE_BIT> ");
      }

    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(stguti, tmpVarStg, TMPBITVAL).c_str());

    if(vclasstype == UC_QUARK)
      {
	fp->write(", ");
	if(m_state.m_genCodingConditionalHas) //not sure this is posoffset, and not true/false?
	  fp->write(m_state.getTmpVarAsString(uvpass.getPtrTargetType(), tmpVarPos).c_str());
	else
	  {
	    assert(m_varSymbol->getPosOffset() == 0);
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset()); //should be 0!
	  }
      }
    else if(vclasstype == UC_ELEMENT)
      {
	//fp->write(", true"); //invokes 'badass' constructor
      }
    else
      assert(0);

    fp->write(");   //shadows lhs of 'h/as'\n");

    m_state.indent(fp);

    //special ulamcontext for autos based on its (lhs) storage
    fp->write("const UlamContext<EC> ");
    fp->write(m_state.getAutoHiddenContextArgName()); // _ucauto

    if(stgcos->isAutoLocal())
      {
	//shadows previous _ucAuto
	fp->write("(");
	fp->write(m_state.getAutoHiddenContextArgName()); // _ucauto
	fp->write("(, ");
	fp->write(m_state.getAutoHiddenContextArgName()); // _ucauto
	fp->write(".LookupElementTypeFromContext(");
      }
    else
      fp->write("(uc, uc.LookupElementTypeFromContext(");

    fp->write(m_varSymbol->getMangledName().c_str()); //auto's name
    fp->write(".getType()));\n");

    m_state.m_genCodingConditionalHas = false; // done
    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs ?
  } //genCodedAutoLocal

} //end MFM
