#include <stdlib.h>
#include "NodeVarDecl.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"

namespace MFM {

  NodeVarDecl::NodeVarDecl(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_varSymbol(sym), m_vid(0), m_nodeInitExpr(NULL), m_currBlockNo(0), m_nodeTypeDesc(nodetype)
  {
    if(sym)
      {
	m_vid = sym->getId();
	m_currBlockNo = sym->getBlockNoOfST();
      }
  }

  NodeVarDecl::NodeVarDecl(const NodeVarDecl& ref) : Node(ref), m_varSymbol(NULL), m_vid(ref.m_vid), m_nodeInitExpr(NULL), m_currBlockNo(ref.m_currBlockNo), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();

    if(ref.m_nodeInitExpr)
      m_nodeInitExpr = (Node *) ref.m_nodeInitExpr->instantiate();
  }

  NodeVarDecl::~NodeVarDecl()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;

    delete m_nodeInitExpr;
    m_nodeInitExpr = NULL;
  }

  Node * NodeVarDecl::instantiate()
  {
    return new NodeVarDecl(*this);
  }

  void NodeVarDecl::updateLineage(NNO pno)
  {
    Node::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
    if(m_nodeInitExpr)
      m_nodeInitExpr->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeVarDecl::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeInitExpr == oldnptr)
      {
	m_nodeInitExpr = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeVarDecl::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    if(m_nodeInitExpr && m_nodeInitExpr->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeVarDecl::checkAbstractInstanceErrors()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    if(nut->getUlamTypeEnum() == Class)
      {
	UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(scalaruti, csym);
	assert(isDefined);
	if(csym->isAbstract())
	  {
	    std::ostringstream msg;
	    msg << "Instance of Abstract Class ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " used with variable symbol name '" << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	  }
      }
  } //checkAbstractInstanceErrors

  //see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarDecl::printPostfix(File * fp)
  {
    printTypeAndName(fp);
    if(m_nodeInitExpr)
      {
	fp->write(" =");
	m_nodeInitExpr->printPostfix(fp);
      }
    fp->write("; ");
  } //printPostfix

  void NodeVarDecl::printTypeAndName(File * fp)
  {
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(vuti);
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    fp->write(" ");
    if(vut->getUlamTypeEnum() != Class)
      fp->write(vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
    else
      fp->write(vut->getUlamTypeNameBrief().c_str());

    fp->write(" ");
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

  const char * NodeVarDecl::getName()
  {
    return m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str();
  }

  const std::string NodeVarDecl::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeVarDecl::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return true;
  }

  void NodeVarDecl::setInitExpr(Node * node)
  {
    assert(node);
    m_nodeInitExpr = node;
    m_nodeInitExpr->updateLineage(getNodeNo()); //for unknown subtrees
  }

  bool NodeVarDecl::hasInitExpr()
  {
    return (m_nodeInitExpr != NULL);
  }

  bool NodeVarDecl::foldInitExpression()
  {
    assert(0); //TBD;
  }

  FORECAST NodeVarDecl::safeToCastTo(UTI newType)
  {
    assert(m_nodeInitExpr);

    FORECAST rscr = CAST_CLEAR;
    UTI nuti = getNodeType();
    //cast RHS if necessary and safe
    if(UlamType::compare(nuti, newType, m_state) != UTIC_SAME)
      {
	rscr = m_nodeInitExpr->safeToCastTo(nuti); //but we're the new type!
	if(rscr != CAST_CLEAR)
	  {
	    std::ostringstream msg;
	    if(m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum() == Bool)
	      msg << "Use a comparison operator";
	    else
	      msg << "Use explicit cast";
	    msg << " to convert "; // the real converting-message
	    msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
	    msg << " to ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " for variable initialization";
	    if(rscr == CAST_BAD)
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
	else if(!Node::makeCastingNode(m_nodeInitExpr, nuti, m_nodeInitExpr))
	  rscr = CAST_BAD; //error
      } //safe cast
    else if(m_nodeInitExpr->isExplicitReferenceCast())
      {
	std::ostringstream msg;
	msg << "Explicit Reference cast for variable '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "' initialization is invalid";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rscr = CAST_BAD;
      }
    return rscr;
  } //safeToCastTo

  bool NodeVarDecl::checkSafeToCastTo(UTI fromType, UTI& newType)
  {
    bool rtnOK = true;
    FORECAST scr = safeToCastTo(fromType); //reversed
    if(scr == CAST_HAZY)
      {
	m_state.setGoAgain();
	newType = Hzy;
	rtnOK = false;
      }
    else if(scr == CAST_BAD)
      {
	newType = Nav;
	rtnOK = false;
      }
    return rtnOK;
  } //checkSafeToCastTo

  UTI NodeVarDecl::checkAndLabelType()
  {
    UTI it = Nav;

    // instantiate, look up in current block
    if(m_varSymbol == NULL)
      checkForSymbol();

    if(m_varSymbol)
      {
	it = m_varSymbol->getUlamTypeIdx(); //base type has arraysize

	UTI cuti = m_state.getCompileThisIdx();
	if(m_nodeTypeDesc)
	  {
	    UTI duti = m_nodeTypeDesc->checkAndLabelType(); //sets goagain
	    if(m_state.okUTItoContinue(duti) && (duti != it))
	      {
		std::ostringstream msg;
		msg << "REPLACING Symbol UTI" << it;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << " used with variable symbol name '" << getName();
		msg << "' with node type descriptor type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(duti).c_str();
		msg << " UTI" << duti << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		m_varSymbol->resetUlamType(duti); //consistent!
		m_state.mapTypesInCurrentClass(it, duti);
		it = duti;
	      }
	  }

	  if(!m_state.okUTItoContinue(it) || !m_state.isComplete(it))
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Variable Decl for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << " used with variable symbol name '" << getName();
	    msg << "' UTI" << it << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    if(m_state.okUTItoContinue(it) || (it == Hzy))
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		it = Hzy;
		m_state.setGoAgain(); //since not error
	      }
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      } //end var_symbol

    ULAMTYPE etyp = m_state.getUlamTypeByIndex(it)->getUlamTypeEnum();
    if(etyp == Void)
      {
	//void only valid use is as a func return type
	std::ostringstream msg;
	msg << "Invalid use of type ";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << " with variable symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	it = Nav;
      }

    if(m_nodeInitExpr)
      {
	UTI eit = m_nodeInitExpr->checkAndLabelType();
	if(eit == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Initial value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is invalid";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	if(eit == Hzy)
	  {
	    std::ostringstream msg;
	    msg << "Initial value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is not ready";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.setGoAgain(); //since not error
	    setNodeType(Hzy);
	    return Hzy; //short-circuit
	  }

	setNodeType(it); //needed before safeToCast
	checkSafeToCastTo(eit, it); //may side-effect 'it'
      }
    Node::setStoreIntoAble(TBOOL_TRUE);
    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  void NodeVarDecl::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_vid, asymptr, hazyKin) && !hazyKin)
      {
	if(!asymptr->isTypedef() && !asymptr->isConstant() && !asymptr->isModelParameter() && !asymptr->isFunction())
	  {
	    m_varSymbol = (SymbolVariable *) asymptr;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << "> is not a variable, and cannot be used as one";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) Variable <" << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "> is not defined, and cannot be used";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      } //alreadyDefined

    m_state.popClassContext(); //restore
  } //checkForSymbol

  NNO NodeVarDecl::getBlockNo()
  {
    return m_currBlockNo;
  }

  NodeBlock * NodeVarDecl::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  void NodeVarDecl::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
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

    if(m_nodeInitExpr)
      m_nodeInitExpr->calcMaxDepth(depth, maxdepth, base);
  } //calcMaxDepth

  void NodeVarDecl::printUnresolvedLocalVariables(u32 fid)
  {
    assert(m_varSymbol);
    UTI it = m_varSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(it))
      {
	// e.g. error/t3298 Int(Fu.sizeof)
	std::ostringstream msg;
	msg << "Unresolved type <";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << "> used with local variable symbol name '" << getName() << "'";
	msg << " in function: " << m_state.m_pool.getDataAsString(fid);
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //compiler counts
      } //not complete
  } //printUnresolvedLocalVariables

  void NodeVarDecl::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);

    if(m_nodeInitExpr)
      m_nodeInitExpr->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);

    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  EvalStatus NodeVarDecl::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    u32 len = nut->getTotalBitSize();

    if((classtype == UC_TRANSIENT) && (len > MAXSTATEBITS))
      return UNEVALUABLE;

    assert(m_varSymbol->getUlamTypeIdx() == nuti); //is it so? if so, some cleanup needed

    assert(!m_varSymbol->isAutoLocal()); //NodeVarRef::eval

    assert(!m_varSymbol->isDataMember()); //see NodeVarDeclDM

    u32 slots = Node::makeRoomForNodeType(nuti, STACK);

    if(m_state.isAtom(nuti))
      {
	UlamValue atomUV = UlamValue::makeAtom(m_varSymbol->getUlamTypeIdx());
	//scalar or UNPACKED array of atoms (t3709)
	u32 baseslot =  ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex();
	for(u32 j = 0; j < slots; j++)
	  m_state.m_funcCallStack.storeUlamValueInSlot(atomUV, baseslot + j);
      }
    else if(classtype == UC_NOTACLASS)
      {
	//local variable to a function;
	// t.f. must be SymbolVariableStack, not SymbolVariableDataMember
	UlamValue immUV;
	if(len <= MAXBITSPERINT)
	  immUV = UlamValue::makeImmediate(m_varSymbol->getUlamTypeIdx(), 0, m_state);
	else if(len <= MAXBITSPERLONG)
	  immUV = UlamValue::makeImmediateLong(m_varSymbol->getUlamTypeIdx(), 0, m_state);
	else
	  immUV = UlamValue::makePtr(((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex(), STACK, getNodeType(), m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId()); //array ptr

	m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
      }
    else if((classtype == UC_ELEMENT) || (classtype == UC_TRANSIENT))
      setupStackWithClassForEval(slots);
    else
      setupStackWithQuarkForEval(slots);

    if(m_nodeInitExpr)
      return evalInitExpr();

    return NORMAL;
  } //eval

  void NodeVarDecl::setupStackWithClassForEval(u32 slots)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    //for eval purposes, a transient must fit into atom state bits, like an element
    // any class may be a data member (see NodeVarDeclDM)
    if(nut->isScalar())
      {
	UlamValue atomUV = UlamValue::makeDefaultAtom(m_varSymbol->getUlamTypeIdx(), m_state);
	m_state.m_funcCallStack.storeUlamValueInSlot(atomUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
      }
    else
      {
	PACKFIT packFit = nut->getPackable();
	if(packFit == PACKEDLOADABLE)
	  {
	    u32 len = nut->getTotalBitSize();
	    u64 dval = 0;
	    AssertBool isPackLoadableScalar = m_state.getPackedDefaultClass(nuti, dval);
	    assert(isPackLoadableScalar);
	    u64 darrval = 0;
	    m_state.getDefaultAsPackedArray(nuti, dval, darrval); //3rd arg ref

	    if(len <= MAXBITSPERINT)
	      {
		UlamValue immUV = UlamValue::makeImmediateClass(nuti, (u32) darrval, len);
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
	      }
	    else if(len <= MAXBITSPERLONG) //t3710
	      {
		UlamValue immUV = UlamValue::makeImmediateLongClass(nuti, darrval, len);
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
	      }
	    else
	      assert(0); //not write load packable!
	  }
	else
	  {
	    //UNPACKED element array
	    UlamValue atomUV = UlamValue::makeDefaultAtom(m_varSymbol->getUlamTypeIdx(), m_state);
	    u32 baseslot =  ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex();
	    for(u32 j = 0; j < slots; j++)
	      {
		m_state.m_funcCallStack.storeUlamValueInSlot(atomUV, baseslot + j);
	      }
	  }
      }
  } //setupStackWithClassForEval

  void NodeVarDecl::setupStackWithQuarkForEval(u32 slots)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    u32 len = nut->getTotalBitSize();

    //must be a local quark! could be an array of them!!
    u32 dq = 0;
    AssertBool isDefinedQuark = m_state.getDefaultQuark(nuti, dq); //returns scalar dq
    assert(isDefinedQuark);
    if(nut->isScalar())
      {
	UlamValue immUV = UlamValue::makeImmediateClass(nuti, dq, len);
	m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
      }
    else
      {
	PACKFIT packFit = nut->getPackable();
	if(packFit == PACKEDLOADABLE)
	  {
	    u64 darrval = 0;
	    m_state.getDefaultAsPackedArray(nuti, (u64) dq, darrval); //3rd arg ref
	    if(len <= MAXBITSPERINT) //t3706
	      {
		UlamValue immUV = UlamValue::makeImmediateClass(nuti, (u32) darrval, len);
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
	      }
	    else if(len <= MAXBITSPERLONG) //t3708
	      {
		UlamValue immUV = UlamValue::makeImmediateLongClass(nuti, darrval, len);
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
	      }
	  }
	else
	  {
	    //UNPACKED array of quarks! t3649, t3707
	    UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	    UlamValue immUV = UlamValue::makeImmediateClass(scalaruti, dq, nut->getBitSize());
	    u32 baseslot =  ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex();
	    for(u32 j = 0; j < slots; j++)
	      {
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, baseslot + j);
	      }
	  }
      }
  } //setupStackWithQuarkForEval

  EvalStatus NodeVarDecl::evalInitExpr()
  {
    EvalStatus evs = NORMAL; //init
    // quark or nonclass data member;
    evalNodeProlog(0); //new current node eval frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr

    evs = evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1);

    u32 slots = makeRoomForNodeType(getNodeType());

    evs = m_nodeInitExpr->eval();

    if(evs == NORMAL)
      {
	if(slots == 1)
	  {
	    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slots+1); //immediate scalar
	    m_state.assignValue(pluv,ruv);

	    //also copy result UV to stack, -1 relative to current frame pointer
	    Node::assignReturnValueToStack(ruv); //not when unpacked? how come?
	  }
	else //unpacked
	  {
	    //t3704, t3706, t3707, t3709
	    UlamValue scalarPtr = UlamValue::makeScalarPtr(pluv, m_state);
	    for(u32 j = 0; j < slots; j++)
	      {
		UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1+1+j); //immediate scalar
		m_state.assignValue(scalarPtr,ruv);
		scalarPtr.incrementPtr(m_state); //by one.
	      }
	  }
      } //normal

    evalNodeEpilog();
    return evs;
  } //evalInitExpr

  EvalStatus NodeVarDecl::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    u32 len = nut->getTotalBitSize();

    if((classtype == UC_TRANSIENT) && (len > MAXSTATEBITS))
      return UNEVALUABLE;

    evalNodeProlog(0); //new current node eval frame pointer

    // return ptr to this local var (from NodeIdent's makeUlamValuePtr)
    UlamValue rtnUVPtr = makeUlamValuePtr();

    u32 absslot = m_state.m_funcCallStack.getAbsoluteStackIndexOfSlot(rtnUVPtr.getPtrSlotIndex());
    rtnUVPtr.setPtrSlotIndex(absslot);
    rtnUVPtr.setUlamValueTypeIdx(PtrAbs);

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeVarDecl::makeUlamValuePtr()
  {
    // (from NodeIdent's makeUlamValuePtr)
    UlamValue ptr;
    if(m_varSymbol->isSelf())
      {
	//when "self/atom" is a quark, we're inside a func called on a quark (e.g. dm or local)
	//'atom' gets entire atom/element containing this quark; including its type!
	//'self' gets type/pos/len of the quark from which 'atom' can be extracted
	UlamValue selfuvp = m_state.m_currentSelfPtr;
	UTI ttype = selfuvp.getPtrTargetType();
	assert(m_state.okUTItoContinue(ttype));
	if((m_state.getUlamTypeByIndex(ttype)->getUlamClassType() == UC_QUARK))
	  {
	    if(m_varSymbol->isSelf())
	      selfuvp = m_state.getAtomPtrFromSelfPtr();
	    //else
	  }
	return selfuvp;
      } //done

    assert(!m_varSymbol->isAutoLocal()); //nodevarref, not here!
    assert(!m_varSymbol->isDataMember());
    //local variable on the stack; could be array ptr!
    ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
    return ptr;
  } //makeUlamValuePtr

  // parse tree in order declared, unlike the ST.
  void NodeVarDecl::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_varSymbol);
    assert(m_state.isComplete(getNodeType()));

    assert(!m_varSymbol->isDataMember()); //NodeVarDeclDM::genCode
    assert(!m_varSymbol->isAutoLocal()); //NodeVarRef::genCode

    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();

    if(m_nodeInitExpr)
      {
	m_nodeInitExpr->genCode(fp, uvpass);

	m_state.indent(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write("("); // use constructor (not equals)
	fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPassVarNum(), uvpass.getPassStorage()).c_str()); //VALUE
	if(m_state.isAtomRef(vuti))
	  fp->write(", uc");
	fp->write(")");

	fp->write(";\n"); //func call args aren't NodeVarDecl's
	m_state.clearCurrentObjSymbolsForCodeGen();
	return; //done
      }

    //initialize T to default atom (might need "OurAtom" if data member ?)
    if(vclasstype == UC_ELEMENT)
      {
	m_state.indent(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str());
	if(vut->isScalar() && m_nodeInitExpr) //default constructor includes a GetDefaultAtom
	  {
	    fp->write("(");
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(vuti).c_str());
	    fp->write(".GetDefaultAtom())"); //returns object of type T
	  }
	//else
      }
    else if(vclasstype == UC_QUARK)
      {
	//left-justified. no initialization
	m_state.indent(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str());
      }
    else
      {
	m_state.indent(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str()); //default 0
      }
    fp->write(";\n"); //func call args aren't NodeVarDecl's
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCode

  void NodeVarDecl::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    assert(0); //see NodeVarDeclDM data members only
  } //generateUlamClassInfo

} //end MFM
