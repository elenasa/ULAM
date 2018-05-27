#include <stdlib.h>
#include "NodeVarRef.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"

namespace MFM {

  NodeVarRef::NodeVarRef(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarDecl(sym, nodetype, state)
  {
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeVarRef::NodeVarRef(const NodeVarRef& ref) : NodeVarDecl(ref) { }

  NodeVarRef::~NodeVarRef() { }

  Node * NodeVarRef::instantiate()
  {
    return new NodeVarRef(*this);
  }

  void NodeVarRef::updateLineage(NNO pno)
  {
    NodeVarDecl::updateLineage(pno);
  }

  bool NodeVarRef::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeVarDecl::findNodeNo(n, foundNode))
      return true;
    return false;
  }

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
    NodeVarDecl::printPostfix(fp);
  }

  void NodeVarRef::printTypeAndName(File * fp)
  {
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(vuti);
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    if(m_state.isConstantRefType(vuti))
      fp->write(" constant"); //t41242,3

    fp->write(" ");
    if(vut->getUlamTypeEnum() != Class)
      {
	fp->write(vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
	fp->write("&"); //<--the only difference!!!
      }
    else
      fp->write(vut->getUlamTypeClassNameBrief(vuti).c_str()); //includes any &

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

  const char * NodeVarRef::getName()
  {
    return NodeVarDecl::getName(); //& ??
  }

  const std::string NodeVarRef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeVarRef::safeToCastTo(UTI newType)
  {
    assert(m_nodeInitExpr);
    UTI nuti = getNodeType();

    //cast RHS if necessary and safe
    //insure lval is same bitsize/arraysize
    // if classes, safe to cast a subclass to any of its superclasses
    FORECAST rscr = CAST_CLEAR;
    if(UlamType::compare(nuti, newType, m_state) != UTIC_SAME)
      {
	UlamType * nut = m_state.getUlamTypeByIndex(nuti);
	UlamType * newt = m_state.getUlamTypeByIndex(newType);
	rscr = m_nodeInitExpr->safeToCastTo(nuti);

	if((nut->getUlamTypeEnum() == Class))
	  {
	    if(rscr != CAST_CLEAR)
	      {
		//e.g. error/t3792, error/t3616
		std::ostringstream msg;
		msg << "Incompatible class types ";
		msg << nut->getUlamTypeClassNameBrief(nuti).c_str();
		msg << " and ";
		msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
		msg << " used to initialize reference '" << getName() <<"'";
		if(rscr == CAST_HAZY)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else if(m_state.isAtom(nuti))
	  {
	    //atoms init from a quark ref could use .atomof
	    // "clear" when complete, not the same as "bad".
	    if(rscr != CAST_CLEAR)
	      {
		std::ostringstream msg;
		msg << "Reference atom variable " << getName() << "'s type ";
		msg << nut->getUlamTypeNameBrief().c_str();
		msg << ", and its initial value type ";
		msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
		msg << ", are incompatible";
		if(newt->isAltRefType() && newt->getUlamClassType() == UC_QUARK)
		  msg << "; .atomof may help";
		if(rscr == CAST_HAZY)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    //primitives must be EXACTLY the same size (for initialization);
	    // "clear" when complete, not the same as "bad". (t3614, t3694)
	    if(rscr != CAST_CLEAR)
	      {
		std::ostringstream msg;
		msg << "Reference variable " << getName() << "'s type ";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		msg << ", and its initial value type ";
		msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
		msg << ", are incompatible";
		if(rscr == CAST_HAZY)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    else if(m_nodeInitExpr->isAConstant() && !m_state.isConstantRefType(nuti))
	      {
		std::ostringstream msg;
		msg << "Initial value of non-constant reference variable: " << getName();
		msg << " is constant";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //error/t41255
		rscr = CAST_BAD;
	      }
	  }
      }
    //okay to explicitly cast rhs to reference type, e.g. if(a is Foo) QW& qref = (Foo &) a;
    return rscr;
  } //safeToCastTo

  bool NodeVarRef::checkReferenceCompatibility(UTI uti)
  {
    assert(m_state.okUTItoContinue(uti));
    if((!m_state.getUlamTypeByIndex(uti)->isAltRefType()))
      {
	std::ostringstream msg;
	msg << "Variable reference '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "', is invalid";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }
    return true; //ok
  } //checkReferenceCompatibility

  UTI NodeVarRef::checkAndLabelType()
  {
    UTI it = NodeVarDecl::checkAndLabelType();
    u32 errCount= 0;
    u32 hazyCount = 0;
    if(!m_state.okUTItoContinue(it))
      {
	if(it == Nav)
	  errCount++;
	if(it == Hzy)
	  hazyCount++;
	assert(it != Nouti);
      }
    ////requires non-constant, non-funccall value
    //NOASSIGN REQUIRED (e.g. for function parameters) doesn't have to have this!
    if(hasInitExpr())
      {
	UTI eit = m_nodeInitExpr->getNodeType();
	if(eit == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", is invalid";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    errCount++;
	  }

	if(eit == Hzy)
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", is not ready";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //possibly still hazy
	    hazyCount++;
	  }

	if(eit == Nouti)
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", is not defined";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //possibly still hazy
	    hazyCount++;
	  }

	if(errCount)
	  {
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	if(hazyCount)
	  {
	    setNodeType(Hzy);
	    m_state.setGoAgain();
	    return Hzy; //short-circuit
	  }

	bool constref = m_state.isConstantRefType(it);

	//check isStoreIntoAble, before any casting (i.e. named constants, func calls, NOT).
	TBOOL istor = m_nodeInitExpr->getStoreIntoAble();
	if(constref)
	  istor = TBOOL_FALSE;
	Node::setStoreIntoAble(istor); //before setReferenceAble is set; //error/t41192

	TBOOL isrefable = m_nodeInitExpr->getReferenceAble();
	Node::setReferenceAble(isrefable); //custom arrays may have different stor/ref status

	if(((isrefable != TBOOL_TRUE) || (istor != TBOOL_TRUE)) && !constref)
	  {
	    //error tests: t3629, t3660, t3661, t3665, t3785
	    std::ostringstream msg;
	    msg << "Initialization for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", must be referenceable"; //e.g. constant or function call NOT so
	    if(istor == TBOOL_HAZY)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		it = Hzy;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav);
		return Nav; //short-circuit
	      }
	  }

	// Don't Go Beyond This Point when either it or eit are not ok.
	if(UlamType::compareForMakingCastingNode(eit, it, m_state) == UTIC_NOTSAME)
	  {
	    //must be safe to case (NodeVarDecl c&l) is different
	    if(!Node::makeCastingNode(m_nodeInitExpr, it, m_nodeInitExpr))
	      {
		setNodeType(Nav);
		return Nav; //short-circuit
	      }
	  }
      }
    else
      {
	//no node init expression
	if(it == Nav)
	  Node::setStoreIntoAble(TBOOL_FALSE);
	else if(it == Hzy)
	  Node::setStoreIntoAble(TBOOL_HAZY);
	if(m_varSymbol && m_varSymbol->isFunctionParameter() && ((SymbolVariableStack *) m_varSymbol)->isConstantFunctionParameter())
	  Node::setStoreIntoAble(TBOOL_FALSE); //t41186
	else
	  Node::setStoreIntoAble(TBOOL_TRUE);
      }

    setNodeType(it);
    if(it == Hzy)
      m_state.setGoAgain();
    return getNodeType();
  } //checkAndLabelType

  TBOOL NodeVarRef::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_state.abortShouldntGetHere(); //refs can't be data members
    return TBOOL_FALSE;
  }

  void NodeVarRef::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return NodeVarDecl::calcMaxDepth(depth, maxdepth, base);
  }

  void NodeVarRef::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeVarDecl::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  EvalStatus NodeVarRef::eval()
  {
    assert(m_varSymbol);

    assert(m_varSymbol->getAutoLocalType() != ALT_AS); //NodeVarRefAuto

    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    assert(m_varSymbol->getUlamTypeIdx() == nuti);
    assert(m_nodeInitExpr);

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr

    EvalStatus evs = m_nodeInitExpr->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue pluv = m_state.m_nodeEvalStack.popArg();
    assert(m_state.isPtr(pluv.getUlamValueTypeIdx()));
    ((SymbolVariableStack *) m_varSymbol)->setAutoPtrForEval(pluv); //for future ident eval uses
    ((SymbolVariableStack *) m_varSymbol)->setAutoStorageTypeForEval(m_nodeInitExpr->getNodeType()); //for future virtual function call eval uses

    m_state.m_funcCallStack.storeUlamValueInSlot(pluv, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex()); //doesn't seem to matter..

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeVarRef::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

    // (from NodeIdent's makeUlamValuePtr)
    // return ptr to this data member within the m_currentObjPtr
    // 'pos' modified by this data member symbol's packed bit position
  UlamValue NodeVarRef::makeUlamValuePtr()
  {
    u32 pos = 0;
    if(m_varSymbol->isDataMember())
      pos = m_varSymbol->getPosOffset();

    UlamValue ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + pos, m_varSymbol->getId());

    ptr.checkForAbsolutePtr(m_state.m_currentObjPtr);
    return ptr;
  } //makeUlamValuePtr

  void NodeVarRef::genCode(File * fp, UVPass & uvpass)
  {
    //reference always has initial value, unless func param
    assert(m_varSymbol->isAutoLocal());
    assert(m_varSymbol->getAutoLocalType() != ALT_AS);

    if(hasInitExpr())
      {
	// get the right-hand side, stgcos
	// can be same type (e.g. element, quark, or primitive),
	// or ancestor quark if a class.
	m_nodeInitExpr->genCodeToStoreInto(fp, uvpass);

	return Node::genCodeReferenceInitialization(fp, uvpass, m_varSymbol);
      }
  } //genCode

} //end MFM
