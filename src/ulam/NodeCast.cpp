#include <stdio.h>
#include "NodeCast.h"
#include "NodeConstant.h"
#include "CompilerState.h"
#include "SymbolVariableStack.h"

namespace MFM {

  NodeCast::NodeCast(Node * n, UTI typeToBe, NodeTypeDescriptor * nodetype, CompilerState & state): NodeUnaryOp(n, state), m_castToBe(typeToBe), m_explicit(false), m_nodeTypeDesc(nodetype)
  {
    setNodeType(typeToBe);
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeCast::NodeCast(const NodeCast& ref) : NodeUnaryOp(ref), m_castToBe(ref.m_castToBe), m_explicit(ref.m_explicit), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeCast::~NodeCast()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  Node * NodeCast::instantiate()
  {
    return new NodeCast(*this);
  }

  void NodeCast::updateLineage(NNO pno)
  {
    NodeUnaryOp::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  }

  bool NodeCast::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeUnaryOp::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeCast::checkAbstractInstanceErrors()
  {
    m_node->checkAbstractInstanceErrors();
  }

  const char * NodeCast::getName()
  {
    return "cast";
  }

  const std::string NodeCast::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  TBOOL NodeCast::getStoreIntoAble()
  {
    return Node::getStoreIntoAble(); //only ref casts are storeintoable
  }

  void NodeCast::setCastType(UTI tobe)
  {
    m_castToBe = tobe;
  }

  UTI NodeCast::getCastType()
  {
    return m_castToBe;
  }

  void NodeCast::setExplicitCast()
  {
    m_explicit = true;
  }

  bool NodeCast::isExplicitCast()
  {
    return m_explicit;
  }

  bool NodeCast::isAConstant()
  {
    return m_node->isAConstant();
  }

  bool NodeCast::isReadyConstant()
  {
    return m_node->isReadyConstant(); //needs constant folding
  }

  bool NodeCast::isNegativeConstant()
  {
    return m_node->isNegativeConstant();
  }

  bool NodeCast::isWordSizeConstant()
  {
    return m_node->isWordSizeConstant();
  }

  bool NodeCast::isFunctionCall()
  {
    return m_node->isFunctionCall();
  }

  bool NodeCast::isAConstructorFunctionCall()
  {
    return m_node->isAConstructorFunctionCall();
  }

  bool NodeCast::isArrayItem()
  {
    return m_node->isArrayItem();
  }

  bool NodeCast::isExplicitReferenceCast()
  {
    return isExplicitCast() && m_state.isAltRefType(getCastType());
  }

  FORECAST NodeCast::safeToCastTo(UTI newType)
  {
    //possible user error, deal with it.
    //assert(UlamType::compare(newType,getNodeType(), m_state) == UTIC_SAME);
    //ulamtype checks for complete, non array, and type specific rules
    return m_state.getUlamTypeByIndex(newType)->safeCast(getCastType());
  }

  UTI NodeCast::checkAndLabelType()
  {
    // unlike the other nodes, nodecast knows its type at construction time;
    // this is for checking for errors, before eval happens.
    u32 errorsFound = 0;
    u32 hazinessFound = 0;
    UTI tobeType = getCastType();
    UTI nodeType = m_node->checkAndLabelType();

    if(nodeType == Nav)
      {
	std::ostringstream msg;
	msg << "Cannot cast erroneous type" ;
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit
      }

    if(nodeType == Hzy)
      {
	std::ostringstream msg;
	msg << "Cannot cast a nonready type" ;
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	setNodeType(Hzy);
	m_state.setGoAgain(); //compiler counts
	return Hzy; //short-circuit
      }

    assert(nodeType != Nouti);

    if(m_nodeTypeDesc)
      {
	//might be a mapped uti for instantiated template class
	tobeType = m_nodeTypeDesc->checkAndLabelType();
	setCastType(tobeType); //overrides type set at parse time
	if(!m_nodeTypeDesc->isReadyType())
	  {
	    std::ostringstream msg;
	    msg << "Cannot cast to nonready type: " ;
	    msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    hazinessFound++; //goAgain set by nodetypedesc
	  }
      }

    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    UlamType * nut = m_state.getUlamTypeByIndex(nodeType);
    if(!tobe->isComplete())
      {
	std::ostringstream msg;
	msg << "Cannot cast to incomplete type: " ;
	msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	hazinessFound++;
      }
    else if(tobeType == Nav)
      {
	std::ostringstream msg;
	msg << "Cannot cast " << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
	msg << " to not-a-valid type";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	errorsFound++;
      }
    else
      {
	ULAMCLASSTYPE tclasstype = tobe->getUlamClassType();
	ULAMCLASSTYPE nclasstype = nut->getUlamClassType();

	if((tclasstype != UC_NOTACLASS) && (nclasstype == UC_NOTACLASS))
	  {
	    if(nodeType == Void)
	      {
		std::ostringstream msg;
		msg << "Cannot cast non-constructor ";
		msg << nut->getUlamTypeNameBrief().c_str(); //non-constructor void
		msg << " to " << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str(); //to a class
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;
	      }
	    else if(!m_state.isAtom(nodeType))
	      {
		std::ostringstream msg;
		msg << "Cannot cast ";
		msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str(); //non-atom
		msg << " to " << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str(); //to a class
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;
	      }
	    else if(tclasstype == UC_QUARK)
	      {
		// cast atom to quark:
		// it becomes an immediate, unless its a ref (t3631).
	      }
	    else if(tclasstype == UC_TRANSIENT)
	      {
		std::ostringstream msg;
		msg << "Cannot cast an atom to transient "; //an atom
		msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;
	      }
	  }
	else if(m_state.isAtomRef(tobeType) && (nclasstype == UC_QUARK) && !nut->isAltRefType())
	  {
	    std::ostringstream msg;
	    msg << "Cannot cast a non-reference quark, ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str(); //to atomref
	    msg << ", to an Atom &";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    errorsFound++;
	  }

	//and then check explicit casts
	if(isExplicitCast())
	  {
	    FORECAST scr = tobe->explicitlyCastable(nodeType);
	    if(scr != CAST_CLEAR)
	      {
		std::ostringstream msg;
		msg << "Cannot explicitly cast ";
		msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
		msg << " to type: " << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
		if((tobe->getUlamTypeEnum() == Bool) && !tobe->isAltRefType()) //t41197, t3962
		  msg << "; Consider using a comparison operation";
		else if(m_state.isAtom(tobeType) && (nclasstype == UC_QUARK))
		  msg << "; Consider using a reference (or self) with .atomof";
		if(scr == CAST_HAZY)
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		    hazinessFound++;
		  }
		else
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    errorsFound++;
		  }
	      }

	    if(m_state.isAltRefType(tobeType) && m_node->isAConstant())
	      {
		std::ostringstream msg;
		msg << "Cannot explicitly cast a constant, " << m_node->getName() << ", type ";
		msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
		msg << ", to a reference type, ";
		msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++; //t3962
	      }
	  }
      }

    //check for any array cast errors
    if(!tobe->isScalar())
      {
	if(!(m_state.isARefTypeOfUlamType(tobeType, nodeType)))
	  {
	    MSG(getNodeLocationAsString().c_str(),
		"Array casts currently not supported", ERR);
	    errorsFound++;
	  }

	if(nut->isScalar())
	  {
	    MSG(getNodeLocationAsString().c_str(),
		"Consider implementing array casts: Cannot cast scalar into array", ERR);
	    errorsFound++;
	  }
	else if(tobe->getArraySize() != nut->getArraySize())
	  {
	    MSG(getNodeLocationAsString().c_str(),
		"Consider implementing array casts: Array sizes differ", ERR);
	    errorsFound++;
	  }
      }
    else
      {
	//to be scalar type
	if(!nut->isScalar())
	  {
	    if(!(m_state.isARefTypeOfUlamType(tobeType, nodeType)))
	      {
		MSG(getNodeLocationAsString().c_str(),
		    "Consider implementing array casts: Cannot cast array into scalar", ERR);
		errorsFound++;
	      }
	  }
      } // end not scalar errors

	// needs commandline arg..lots of non-explicit warning.
	// reserve for user requested casts;
	////if(isExplicitCast())
	//Node::warnOfNarrowingCast(nodeType, tobeType);

    if((errorsFound == 0) && (hazinessFound == 0))
      {
	ULAMCLASSTYPE nodeClass = nut->getUlamClassType();
	ULAMCLASSTYPE tobeClass = tobe->getUlamClassType();
	//avoid assuming casting to a quark is numeric, might be a ref (t3650)
	if(nodeClass == UC_QUARK && tobe->isNumericType() && (tobeClass == UC_NOTACLASS))
	  {
	    // special case: user casting a quark to an Int;
	    if(!m_state.quarkHasAToIntMethod(nodeType))
	      {
		std::ostringstream msg;
		msg << "Cannot cast quark ";
		msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
		msg << " to numeric type " << m_state.getUlamTypeNameByIndex(tobeType).c_str();
		msg << " without a defined 'toInt' method";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;
	      }
	    // infinite loop!! using makeCastingNode. so don't.
	    else
	      {
		// a function call is not a valid lhs !!!
		// 'node' is a function call that returns a quark (it's not storeintoable);
		// build a toIntHelper function that takes the return value of 'node'
		// as its arg and returns toInt (NodeFunctionCall *)
		if(m_node->isFunctionCall())
		  {
		    if(Node::buildCastingFunctionCallNode((Node *) m_node, tobeType, m_node))
		      {
			//e.g. wrong size Int (t3412)
			if(!Node::makeCastingNode(m_node, tobeType, m_node))
			  errorsFound++;
		      }
		  }
		else
		  {
		    m_node = Node::buildToIntCastingNode(m_node);
		    assert(m_node);
		    m_node->setNodeLocation(getNodeLocation());
		    m_node->updateLineage(getNodeNo());
		    UTI chkintuti = m_node->checkAndLabelType();
		    if(!m_state.okUTItoContinue(chkintuti))
		      {
			std::ostringstream msg;
			msg << "Cannot cast quark ";
			msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
			msg << " to numeric type " << m_state.getUlamTypeNameByIndex(tobeType).c_str();
			msg << " without a defined 'toInt' method";
			if(chkintuti == Nav)
			  {
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			    errorsFound++;
			  }
			else
			  {
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			    hazinessFound++;
			  }
		      }
		  }
	      }
	  }
	//can't detect its a CaArray; already resolved by m_node ([]) to caarrayType
	else
	  {
	    //classes are not surprisingly unknown bit sizes at this point
	    if(nodeClass == UC_UNSEEN)
	      {
		std::ostringstream msg;
		msg << "Cannot cast unseen class ";
		msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
		msg << " to " << m_state.getUlamTypeNameByIndex(tobeType).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;
	      }
	  }
      }

    if(errorsFound)
      {
	setNodeType(Nav);
	return Nav; //consistent, at last!
      }

    if(hazinessFound)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
	return Hzy;
      }

    //storeintoable includes referenceAble
    if(m_state.getReferenceType(tobeType) == ALT_REF)
      Node::setStoreIntoAble(TBOOL_TRUE); //t3692, t41153
    else
      Node::setStoreIntoAble(TBOOL_FALSE); //TBOOL_HZY up until now.

    setNodeType(getCastType()); //since neither Hzy, nor Nav
    return getNodeType();
  } //checkAndLabelType

  UTI NodeCast::calcNodeType(UTI uti)
  {
    return uti; //noop
  }

  void NodeCast::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    m_node->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  EvalStatus NodeCast::eval()
  {
    assert(m_node); //has to be
    UTI nuti = getNodeType();
    UTI tobeType = getCastType();
    UTI nodeType = m_node->getNodeType(); //uv.getUlamValueType()

    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(nodeType);
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL) return evalStatusReturn(evs);

    if(nodeType == Void) return evalStatusReturn(UNEVALUABLE); //t41077, nothing to load

    //do we believe these to be scalars, only?
    //possibly an array that needs to be casted, per elenemt
    if(m_state.isScalar(tobeType))
      {
	if(!m_state.isScalar(nodeType))
	  {
	    std::ostringstream msg;
	    msg << "Cannot cast an array ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
	    msg << " to a scalar type " ;
	    msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	//both arrays the same dimensions
	if(m_state.getArraySize(tobeType) != m_state.getArraySize(nodeType))
	  {
	    MSG(getNodeLocationAsString().c_str(),
		"Considering implementing array casts!!!", ERR);
	  }
      }

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
    UTI vuti = uv.getUlamValueTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();
    assert(!m_state.isPtr(vuti));
    if(m_state.isAtom(nodeType) && m_state.isAtom(tobeType) && (vut->getUlamTypeEnum() == Class))
      {
	std::ostringstream msg;
	msg << "Cast question: Do not wipe out actual type for atom during eval! Value type ";
	msg << m_state.getUlamTypeNameBriefByIndex(vuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return evalStatusReturn(UNEVALUABLE);
      }
    if((m_state.isAtom(tobeType)) && (vclasstype == UC_QUARK))
      {
	assert(vut->isAltRefType()); //an immediate non-ref quark should be an error
	std::ostringstream msg;
	msg << "Cast question: actual type for quark ref during eval! Value type ";
	msg << m_state.getUlamTypeNameBriefByIndex(vuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return evalStatusReturn(UNEVALUABLE);
      }
    if((m_state.isAtom(tobeType)) && (vclasstype == UC_ELEMENT) && vut->isAltRefType())
      {
	std::ostringstream msg;
	msg << "Cast question: actual type for element ref during eval! Value type ";
	msg << m_state.getUlamTypeNameBriefByIndex(vuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return evalStatusReturn(UNEVALUABLE); //e.g. t3753
      }
    else if(UlamType::compare(nodeType, tobeType, m_state) != UTIC_SAME)
      {
	if(m_state.isARefTypeOfUlamType(nodeType, tobeType))
	  {
	    uv.setUlamValueTypeIdx(tobeType);
	  }
	else if(!(m_state.getUlamTypeByIndex(tobeType)->cast(uv, tobeType)))
	  {
	    std::ostringstream msg;
	    msg << "Cast problem during eval! Value type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(vuti).c_str();
	    msg << " failed to be cast as ";
	    msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    return evalStatusReturn(UNEVALUABLE);
	  }
      }
    //also copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeCast::evalToStoreInto()
  {
    assert(m_node); //has to be

    UTI nuti = getNodeType();
    UTI tobeType = getCastType();
    UTI nodeType = m_node->getNodeType(); //uv.getUlamValueType()

    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    bool isConstRefType = m_state.isConstantRefType(tobeType);

    if(!isConstRefType)
      {
	TBOOL stor = m_node->getStoreIntoAble();
	if(stor == TBOOL_FALSE) return evalErrorReturn();
	else if(stor == TBOOL_HAZY) evalStatusReturnNoEpilog(NOTREADY);
	//else
      }
    //else continue if constant ref type (t41238)

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_node->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    //then what? (see NodeMemberSelect)
    UlamValue ruvPtr = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1);
    if(!ruvPtr.isPtr()) return evalStatusReturn(UNEVALUABLE); //t41053

    if(UlamType::compare(nodeType, tobeType, m_state) != UTIC_SAME)
      {
	if(m_state.isARefTypeOfUlamType(nodeType, tobeType))
	  {
	    ruvPtr.setPtrTargetType(tobeType);
	  }
	else
	  {
	    UlamValue uvp = ruvPtr;
	    if(m_state.isAltRefType(uvp.getPtrTargetType()))
	      uvp = m_state.getPtrTarget(uvp);

	    UlamValue uv = uvp;
	    UTI ttype = uv.getUlamValueTypeIdx();
	    if(m_state.isPtr(ttype))
	      {
		uv = m_state.getPtrTarget(uvp);
		ttype = uv.getUlamValueTypeIdx();
	      }

	    if(m_state.isPtr(ttype) || !(m_state.getUlamTypeByIndex(tobeType)->cast(uv, tobeType)))
	      {
		std::ostringstream msg;
		msg << "Cast problem during evalToStoreInto! Value type ";
		msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
		msg << " failed to be cast as ";
		msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		return evalStatusReturn(UNEVALUABLE);
	      }
	    else
	      {
		UTI dereftobe = m_state.getUlamTypeAsDeref(tobeType);
		ruvPtr.setPtrTargetType(dereftobe); //t3754 case 1 & 3 (to element ref)

		//before the cast, so we don't lose the subclass ("effself") in
		//case of virtual func calls? (t41364)???
		if(m_state.isAClass(nodeType))
		  {
		    m_state.m_currentAutoObjPtr = ruvPtr;
		    m_state.m_currentAutoStorageType = nodeType;

		    u32 baserelpos = 0;
		    //use nodetype for data members (t41364)
		    if(m_state.getABaseClassRelativePositionInAClass(nodeType, dereftobe, baserelpos))
		      {
			ruvPtr.setPtrPos(ruvPtr.getPtrPos() + baserelpos); //t41319
			ruvPtr.setPtrLen(m_state.getBaseClassBitSize(dereftobe)); //t41364
		      }
		  } //else (not a class)
	      }
	  }
      }
    // fix absolute slot, here or at func call?
    Node::assignReturnValuePtrToStack(ruvPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeCast::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    m_state.abortShouldntGetHere(); // n/a
    return UlamValue();
  }

  UlamValue NodeCast::makeImmediateLongUnaryOp(UTI type, u64 data, u32 len)
  {
    m_state.abortShouldntGetHere(); // n/a
    return UlamValue();
  }

  void NodeCast::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    if(m_node)
      m_node->calcMaxDepth(depth, maxdepth, base); //funccall?
    return; //work done by NodeStatements and NodeBlock
  }

  void NodeCast::genCode(File * fp, UVPass& uvpass)
  {
    //m_node->genCode(fp, uvpass); //might be unused (e.g. classes, atoms)
    bool needsacast = needsACast();
    UTI tobeType = getCastType(); //tobe
    UTI nodeType = m_node->getNodeType();
    bool isternarynode = m_node->isTernaryExpression(); //t41071

    if((!m_state.isAPrimitiveType(tobeType) || !m_state.isAPrimitiveType(nodeType)) && needsacast && !isternarynode)
      m_node->genCodeToStoreInto(fp, uvpass); //(e.g. classes, atoms)
    else
      m_node->genCode(fp, uvpass); //might be unused (e.g. classes, atoms)

    if(needsacast) //(bypasses references)
      {
	genCodeReadIntoATmpVar(fp, uvpass); // cast.
      }
    else if(m_state.isAltRefType(tobeType)) //to ref type
      genCodeCastAsReference(fp, uvpass); //minimal casting
    else if(m_state.isAltRefType(nodeType)) //from ref type
      genCodeCastFromAReference(fp, uvpass);
  } //genCode

 void NodeCast::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    m_node->genCodeToStoreInto(fp, uvpass);
    if(needsACast())
      {
	genCodeReadIntoATmpVar(fp, uvpass); // cast.
      }
    //    else
    if(m_state.isAltRefType(getCastType())) //to ref type
      genCodeToStoreIntoCastAsReference(fp, uvpass); //noop
    else if(m_state.isAltRefType(m_node->getNodeType())) //from ref type
      genCodeToStoreIntoCastFromAReference(fp, uvpass); //noop
  } //genCodeToStoreInto

  void NodeCast::genCodeReadIntoATmpVar(File * fp, UVPass& uvpass)
  {
    // e.g. called by NodeFunctionCall on a NodeTerminal..
    if(!needsACast())
      {
	//needs a test..
	return m_node->genCodeReadIntoATmpVar(fp, uvpass);
      }

    UTI tobeType = getCastType(); //tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    s32 tmpVarNum =  uvpass.getPassVarNum();
    UTI vuti = uvpass.getPassTargetType(); //replace
    assert(m_state.okUTItoContinue(vuti));

    TMPSTORAGE vstor = uvpass.getPassStorage();
    bool isTerminal = (vstor == TERMINAL);

    if((tobeType == vuti) || (tobeType == Void)) //t3961
      {
	m_state.clearCurrentObjSymbolsForCodeGen(); //clear by now
	return; //nothing to do!
      }

   UlamType * vut = m_state.getUlamTypeByIndex(vuti); //after vuti replacement
   if(!tobe->isPrimitiveType() || !vut->isPrimitiveType())
     {
       return genCodeReadNonPrimitiveIntoATmpVar(fp, uvpass);
     }

   //Primitive types:
   s32 tmpVarCastNum = m_state.getNextTmpVarNumber();

   m_state.indentUlamCode(fp);
   fp->write("const ");
   fp->write(tobe->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64, etc.
   fp->write(" ");
   fp->write(m_state.getTmpVarAsString(tobeType, tmpVarCastNum, TMPREGISTER).c_str());
   fp->write(" = ");

   // write the cast method (e.g. _Unsigned32ToInt32, _Int32ToUnary32, etc..)
   fp->write(tobe->castMethodForCodeGen(vuti).c_str());
   fp->write("(");

   if(isTerminal)
     {
       fp->write(m_state.m_pool.getDataAsString(uvpass.getPassNameId()).c_str());
     }
   else
     {
       fp->write(m_state.getTmpVarAsString(tobeType, tmpVarNum, vstor).c_str());
       if(vstor == TMPBITVAL)
	 fp->write(".read()");
       if(vstor == TMPAUTOREF)
	 m_state.abortNeedsATest();
     }

   fp->write(", ");

   assert(!(m_state.isAtom(tobeType) || m_state.isAtom(vuti)));
   //LENGTH of node being casted (Uh_AP_mi::LENGTH ?)
   //fp->write(m_state.getBitVectorLengthAsStringForCodeGen(nodetype).c_str());
   fp->write_decimal(vut->getTotalBitSize()); //src length

   fp->write(", ");
   fp->write_decimal(tobe->getTotalBitSize()); //tobe length
   fp->write(");"); GCNL;

   //support constant function parameters using constant primitive types (t41240)
   UTI derefTobe = m_state.getUlamTypeAsDeref(tobeType);

   //PROBLEM is that funccall checks for 0 nameid to use the tmp var!
   // but then if we don't pass it along Node::genMemberNameForMethod fails..
   if(isTerminal)
     uvpass = UVPass::makePass(tmpVarCastNum, TMPREGISTER, derefTobe, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified.
   else
     uvpass = UVPass::makePass(tmpVarCastNum, tobe->getTmpStorageTypeForTmpVar(), derefTobe, m_state.determinePackable(tobeType), m_state, 0, uvpass.getPassApplyDelta(), uvpass.getPassNameId()); //POS 0 rightjustified; pass along name id and apply delta
   //   m_state.clearCurrentObjSymbolsForCodeGen(); //clear by now
  } //genCodeReadIntoATmpVar

  //handle element-atom and atom-element casting differently:
  // handle element->quark, atom->quark, not quark->atom
  // handle quark->quark for casting to ancestor
  // handle quarkref->element/elementref for casting from ancestor to sub
  void NodeCast::genCodeReadNonPrimitiveIntoATmpVar(File * fp, UVPass &uvpass)
  {
    UTI tobeType = getCastType(); //tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    ULAMCLASSTYPE tclasstype = tobe->getUlamClassType();

    UTI vuti = uvpass.getPassTargetType(); //replace
    UlamType * vut = m_state.getUlamTypeByIndex(vuti); //after vuti replacement
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();

    if((tclasstype == UC_QUARK) && (vclasstype == UC_QUARK))
      return genCodeCastDescendant(fp, uvpass); //rt check for super->sub

    if((tclasstype == UC_QUARK) && (vclasstype == UC_ELEMENT))
      return genCodeCastDescendant(fp, uvpass); //no rt check: sub->super

    // c&l insures quark is a ref.
    if((tclasstype == UC_ELEMENT) && (vclasstype == UC_QUARK))
      return genCodeCastAncestorQuarkAsSub(fp, uvpass); //rt check: super->sub

    if((tclasstype == UC_QUARK) && (vclasstype == UC_TRANSIENT))
      return genCodeCastDescendant(fp, uvpass); //rt check for super->sub

    if((tclasstype == UC_TRANSIENT) && (vclasstype == UC_TRANSIENT))
      return genCodeCastDescendant(fp, uvpass); //t3967 //rt check for super->sub

    // c&l insures quark is a ref. ???
    if((tclasstype == UC_TRANSIENT) && (vclasstype == UC_QUARK))
      return genCodeCastAncestorQuarkAsSub(fp, uvpass); //rt check: super->sub

    //only to be quark makes sense!!! check first, one might be element.
    //UNLESS quark ref -> atom, handled separately.
    if((tclasstype == UC_QUARK) || (vclasstype == UC_QUARK))
      return genCodeCastAtomAndQuark(fp, uvpass);

    if((tclasstype == UC_ELEMENT) || (vclasstype == UC_ELEMENT))
      return genCodeCastAtomAndElement(fp, uvpass);

    {
      std::ostringstream msg;
      msg << "Casting 'incomplete' types: ";
      msg << tobe->getUlamTypeName().c_str();
      msg << "(UTI" << tobeType << ") to be " << vut->getUlamTypeName().c_str();
      msg << "(UTI" << vuti << ")";
      if((tclasstype == UC_TRANSIENT) || (vclasstype == UC_TRANSIENT))
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      else
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      return;
    }
  } //genCodeReadNonPrimitiveIntoATmpVar

  void NodeCast::genCodeWriteFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass)
  {
    m_state.abortShouldntGetHere();
    genCodeWriteFromATmpVar(fp, luvpass, ruvpass);
  }

  void NodeCast::genCodeCastAtomAndElement(File * fp, UVPass & uvpass)
  {
    UTI tobeType = getCastType();
    UTI vuti = uvpass.getPassTargetType();  //replace
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    TMPSTORAGE vstor = uvpass.getPassStorage();

    bool usePassVal = m_state.m_currentObjSymbolsForCodeGen.empty() && ((vstor == TMPBITVAL) || (vstor == TMPAUTOREF));
    bool makeValFromPass = m_state.m_currentObjSymbolsForCodeGen.empty() && !usePassVal;

    s32 tmpTstg = m_state.getNextTmpVarNumber();
    if(makeValFromPass)
      {
	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members, tmp immediate from-type
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(vuti, tmpTstg, TMPBITVAL).c_str());
	fp->write(" = ");
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(";"); GCNL;
      }

      if(m_state.isAtom(vuti)) //from atom-to-element
	{
	  genCodeCastAtomToElement(fp, uvpass, makeValFromPass ? tmpTstg : 0, usePassVal);
	}
      else if(m_state.isAtom(tobeType)) //from element-to-atom
	{
	  genCodeCastElementToAtom(fp, uvpass, makeValFromPass ? tmpTstg : 0, usePassVal);
	}
      else //no atoms, explicit like-kind elements
	{
	  genCodeCastLikeKindElements(fp, uvpass, makeValFromPass ? tmpTstg : 0, usePassVal);
	}

      //uvpass updated to have the casted type
      m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs
      return;
  } //genCodeCastAtomAndElement

  void NodeCast::genCodeCastAtomToElement(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal)
  {
    UTI tobeType = getCastType();
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getPassTargetType();  //replace
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);
    UTI cosuti = cos->getUlamTypeIdx();

    bool makeValFromPass = (tmpvarstg > 0);

    // "downcast" might not be true; compare to be sure the atom is an element "Foo"
    m_state.indentUlamCode(fp);
    fp->write("if(!");
    fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
    fp->write(".");
    fp->write(m_state.getIsMangledFunctionName(tobeType));
    fp->write("(");
    if(usePassVal)
      {
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(".");
	fp->write(vut->readMethodForCodeGen().c_str());
	fp->write("()");
      }
    else if(makeValFromPass)
      {
	fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //t3277
      }
    else
      {
	assert(m_state.isAtom(cos->getUlamTypeIdx()));
	fp->write(cos->getMangledName().c_str()); //t3754, t3408
	fp->write(".read()");
      }
    fp->write("))"); GCNL;

    m_state.m_currentIndentLevel++;
    m_state.indentUlamCode(fp);
    fp->write("FAIL(BAD_CAST);"); GCNL;
    m_state.m_currentIndentLevel--;

    if(tobe->isReference()) //t3754 (not isAltRefType, could be ALT_AS t3835)
      {
	u32 idforuvpass = 0;
	s32 tmpeleref = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indentUlamCode(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpeleref, TMPAUTOREF).c_str());
	fp->write("(");

	if(usePassVal)
	  {
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(", ");
	    //must displace the Typefield for element ref
	    fp->write("0u + T::ATOM_FIRST_STATE_BIT, ");
	    if(m_state.isAtomRef(vuti)) //from atom/ref tmpvar
	      {
		fp->write(uvpass.getTmpVarAsString(m_state).c_str());
		fp->write(".GetEffectiveSelf()"); //maintains eff self
	      }
	    else
	      {
		fp->write("&");
		fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
		fp->write(", uc");
	      }
	    fp->write(");"); GCNL;
	  }
	else if(makeValFromPass) //fm atom non-ref
	  {
	    m_state.abortShouldntGetHere();
	  }
	else //fm atom/ref named var/dm
	  {
	    fp->write(cos->getMangledName().c_str()); //assumes only one!!!
	    fp->write(", ");
	    //must displace the Typefield for element ref
	    fp->write("0u + T::ATOM_FIRST_STATE_BIT, ");
	    if(m_state.isAtomRef(cosuti)) //t3754
	      {
		fp->write(cos->getMangledName().c_str()); //assumes only one!
		fp->write(".GetEffectiveSelf()"); //maintains eff self
	      }
	    else
	      {
		fp->write("&");
		fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
		fp->write(", uc"); //t3754
	      }
	    fp->write(");"); GCNL;
	    idforuvpass = cos->getId();
	  }
	uvpass = UVPass::makePass(tmpeleref, TMPAUTOREF, tobeType, UNPACKED, m_state, 0, idforuvpass); //POS moved left for type; pass along name id);
      }
    else
      {
	//tobe non-ref element
	if(m_state.isAtomRef(vuti)) //from atomref-to-element
	  {
	    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for atomref
	    m_state.indentUlamCode(fp);
	    fp->write("const ");
	    fp->write(vut->getTmpStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, TMPTATOM).c_str());
	    fp->write(" = ");

	    if(usePassVal)
	      {
		//read into a T (t41157), unless already a T from .atomof (t3756)
		fp->write(uvpass.getTmpVarAsString(m_state).c_str());
		fp->write(".read();"); GCNL;
	      }
	    else if(makeValFromPass)
	      {
		fp->write(m_state.getTmpVarAsString(vuti, tmpvarstg, TMPBITVAL).c_str());
		fp->write(".read();"); GCNL;
	      }
	    else //no need to re-read? atom-based element (e.g. t3410, 3277)
	      {
		fp->write(cos->getMangledName().c_str());
		fp->write(".read();"); GCNL;
	      }
	    uvpass = UVPass::makePass(tmpVarNum2, TMPTATOM, tobeType, UNPACKED, m_state, 0, uvpass.getPassNameId());
	  }
	else
	  {
	    Node::genCodeReadIntoATmpVar(fp,uvpass); //t3697,t41153
	    uvpass.setPassTargetType(tobeType); //same variable
	  }
      }
    //updated the uvpass to have the casted type
    return;
  } //genCodeCastAtomToElement (helper)

  void NodeCast::genCodeCastElementToAtom(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal)
  {
    UTI tobeType = getCastType();
    UTI vuti = uvpass.getPassTargetType();  //replace

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);
    UTI cosuti = cos->getUlamTypeIdx();

    bool makeValFromPass = (tmpvarstg > 0);

    //from element-to-atom
    if(m_state.isAtomRef(tobeType))
      {
	//element-ref to atom-ref is why we couldn't have packed elements!!! t3753
	u32 idforuvpass = 0;
	UlamType * atomut = m_state.getUlamTypeByIndex(UAtom);
	s32 tmpatomref = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indentUlamCode(fp);
	fp->write(atomut->getUlamTypeImmediateAutoMangledName().c_str()); //tobe as auto
	fp->write("<EC> ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpatomref, TMPAUTOREF).c_str());
	fp->write("(");

	if(usePassVal) //fm element/ele-ref
	  {
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(", ");
	    //must displace the Typefield if a ref
	    if(m_state.isAltRefType(vuti))
	      {
		fp->write("- T::ATOM_FIRST_STATE_BIT);"); GCNL; //t3753
	      }
	    else
	      {
		fp->write("0u, uc);"); GCNL;
	      }
	  }
	else if(makeValFromPass)
	  {
	    m_state.abortNotImplementedYet(); //??
	  }
	else
	  {
	    fp->write(cos->getMangledName().c_str()); //assumes only one!!!
	    fp->write(", ");
	    //must displace the Typefield if a ref
	    if(m_state.isAltRefType(cosuti))
	      {
		fp->write("- T::ATOM_FIRST_STATE_BIT);"); GCNL; //t3753
	      }
	    else
	      {
		fp->write("0u, uc);"); GCNL;
	      }
	    idforuvpass = cos->getId();
	  }
	uvpass = UVPass::makePass(tmpatomref, TMPAUTOREF, tobeType, UNPACKED, m_state, 0, idforuvpass); //POS moved left for type; pass along name id);
      }
    else if(m_state.isAtom(tobeType))
      {
	// to atom, from T (read NOW from element or element ref)
	//No longer, convert T to AtomBitStorage (e.g. t3697, t3637)
	Node::genCodeReadIntoATmpVar(fp,uvpass); //t3248,9,t3255,81,t3336,8
	uvpass.setPassTargetType(tobeType); //same variable, t3909
      }
  } //genCodeCastElementToAtom (helper)

  void NodeCast::genCodeCastLikeKindElements(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal)
  {
    //explicit cast from element-ref, to element or element-ref; or
    //explicit cast from element storage, to element or element-ref
    //explicit cast element to element ref (e.g. question colon, t41067)
    UTI tobeType = getCastType();
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    TMPSTORAGE tstor = tobe->getTmpStorageTypeForTmpVar();

    UTI vuti = uvpass.getPassTargetType();  //replace

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);
    UTI cosuti = cos->getUlamTypeIdx();

    u32 idforuvpass = 0;
    UTI reftobeType = m_state.getUlamTypeAsRef(tobeType);
    UlamType * reftobe = m_state.getUlamTypeByIndex(reftobeType);

    bool makeValFromPass = (tmpvarstg > 0);

    s32 tmpeleref = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write(reftobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(reftobeType, tmpeleref, TMPAUTOREF).c_str());
    fp->write("(");

    if(usePassVal)
      {
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	if(!m_state.isAltRefType(vuti))
	  fp->write(", uc");
	fp->write(");"); GCNL;
      }
    else if(makeValFromPass)
      {
	fp->write(m_state.getTmpVarAsString(vuti, tmpvarstg, TMPBITVAL).c_str());
	fp->write(", uc");
	fp->write(");"); GCNL;
      }
    else
      {
	//explicit cast fm element or element-ref, to element-ref(t41052)
	// ,including 'self' (t3583,t3912,3,t41065,89,t41139)
	fp->write(cos->getMangledName().c_str()); //assumes only one!!
	if(!m_state.isReference(cosuti)) //not AltRefType
	  {
	    fp->write(", T::ATOM_FIRST_STATE_BIT + 0, &"); //pos
	    fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str());
	    fp->write(", uc");
	    fp->write(");"); GCNL; //t41052
	  }
	else
	  {
	    fp->write(", 0, "); //pos
	    fp->write(cos->getMangledName().c_str()); //assumes only one!
	    fp->write(".GetEffectiveSelf()");
	    fp->write(");"); GCNL;
	  }

	idforuvpass = cos->getId();
      }
    if(tobe->isReference())
      {
	uvpass = UVPass::makePass(tmpeleref, TMPAUTOREF, reftobeType, UNPACKED, m_state, 0, idforuvpass); //POS moved left for type; pass along name id)
      }
    else
      {
	s32 tmpeleval = m_state.getNextTmpVarNumber();
	//convert element ref to its immediate, and read into
	//tmpstorage to return (t41067)
	m_state.indentUlamCode(fp);
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //BV, not const
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpeleval, tstor).c_str());
	fp->write(" = ");
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //anonymous immediate
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(reftobeType, tmpeleref, TMPAUTOREF).c_str());
	fp->write(").read();"); GCNL;

	//update the uvpass to have the casted value
	uvpass = UVPass::makePass(tmpeleval, tstor, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
  } //genCodeCastLikeKindElements (helper)

  void NodeCast::genCodeCastAtomAndQuark(File * fp, UVPass & uvpass)
  {
    // quark tobe, only if ancestor
    // also allow quark refs -> atom cast, if ancestor of atom type.
    // e.g. assignments (e.g. t3697, error/t3691).
    UTI  vuti = uvpass.getPassTargetType();
    assert(m_state.okUTItoContinue(vuti));
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    TMPSTORAGE vstor = uvpass.getPassStorage();

    bool usePassVal = m_state.m_currentObjSymbolsForCodeGen.empty() && ((vstor == TMPBITVAL) || (vstor == TMPAUTOREF));
    bool makeValFromPass = m_state.m_currentObjSymbolsForCodeGen.empty() && !usePassVal;

    s32 tmpStg = m_state.getNextTmpVarNumber();
    if(makeValFromPass)
      {
	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members, tmp immediate from-type
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(vuti, tmpStg, TMPBITVAL).c_str());
	fp->write("(");
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(");"); GCNL;
      }

    if(m_state.isAtom(vuti))
      {
	genCodeCastAtomToQuark(fp, uvpass, makeValFromPass ? tmpStg : 0, usePassVal);
      }
    else
      {
	genCodeCastQuarkRefToAtom(fp, uvpass, makeValFromPass ? tmpStg : 0, usePassVal);
      }
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastAtomAndQuark

  void NodeCast::genCodeCastAtomToQuark(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal)
  {
    // also allow quark refs -> atom cast, if ancestor of atom type.
    // e.g. assignments (e.g. t3697, error/t3691).
    UTI tobeType = getCastType();
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    TMPSTORAGE tstor = tobe->getTmpStorageTypeForTmpVar();

    UTI  vuti = uvpass.getPassTargetType();
    assert(m_state.okUTItoContinue(vuti));
    assert(m_state.isAtom(vuti)); //from atom/ref->quark/ref
    TMPSTORAGE vstor = uvpass.getPassStorage();

    //stack still empty when func call returns self (t41065, case foofunc())
    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    bool makeValFromPass = (tmpvarstg > 0);

    s32 tmpVarIs = m_state.getNextTmpVarNumber();

    // "downcast" might not be true; compare to be sure the atom is-a quark "Foo"
    // by inheritance (see t3631)

    //Type used twice, put in a tmp var
    s32 tmpVarType = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const s32 ");
    fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());;
    fp->write(" = ");

    //locals t3692,3,7,3701, t3756,7,t3837, t3986, t41005,6,7
    //e.g. carray (e.g. error/t3508), a data member
    if(usePassVal)
      {
	fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //t41143
	fp->write(".ReadAtom()");
      }
    else if(makeValFromPass) //a T
      {
	fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //t3692
      }
    else //no read for self
      {
	assert(!cos->isSelf());
	fp->write(cos->getMangledName().c_str()); //assumes only one!!!
      }
    fp->write(".GetType();"); GCNL;
    //got atom's Element Type in tmpvar.

    //now, check that Type is a defined Element type
    m_state.indentUlamCode(fp);
    fp->write("const bool ");
    fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs, TMPREGISTER).c_str());
    fp->write(" = ((");

    //when SUBATOMIC quark, dont know rel pos without asking (ulam-5)
    // immediate quark's storage cast as a quark? Terrible idea!!
    fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());
    fp->write(" == T::ATOM_UNDEFINED_TYPE) ? false : "); //subatomic type

    //internal method, takes uc, u32 and const char*, returns true
    // if same or ancester
    fp->write(m_state.getIsMangledFunctionName(vuti));
    fp->write("(");
    fp->write("uc, ");
    fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());
    fp->write(", &");
    fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
    fp->write("));"); GCNL;


    m_state.indentUlamCode(fp);
    fp->write("if(!");
    fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs, TMPREGISTER).c_str());
    fp->write(")\n");

    m_state.m_currentIndentLevel++;
    m_state.indentUlamCode(fp);
    fp->write("FAIL(BAD_CAST);"); GCNL;
    fp->write("\n");
    m_state.m_currentIndentLevel--;


    //Next pos of quark in atom/element..
    //offset of descendant NOT always 0 fm start of state bits (ulam-5)
    s32 tmpVarPos = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const s32 ");
    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
    fp->write(" = ");
    fp->write(m_state.getGetRelPosMangledFunctionName(vuti)); //UlamClass Method
    fp->write("(uc, ");
    fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str()); //element type
    fp->write(", &");
    fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str()); //baseclass ptr
    fp->write("); //relpos"); GCNL;

    //t3631, t3692, t3693, t3697, t3701, t3756, t3757, t3789, t3834, t3837
    // might be ancestor quark
    // can't let Node::genCodeReadIntoTmpVar do this for us (we need a ref!):
    //    if(!tobe->isReference()) // not isAltRefType, could be ALT_AS (t3835)

    // here, making a complete object quark from an atom/ref, need
    // that CLEVER trick i do believe (i.e. make a ref, then its
    // immediate) in order to gather possible scattered base
    // classes. Can't make a complete atom from a quark, unless
    // it's a quarkref.
    UTI reftobeType = m_state.getUlamTypeAsRef(tobeType);
    UlamType * reftobe = m_state.getUlamTypeByIndex(reftobeType);
    s32 tmpVarRef = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    fp->write(reftobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(reftobeType, tmpVarRef, TMPAUTOREF).c_str());
    fp->write("(");

    if(!m_state.isAtomRef(vuti)) //fm atom non-ref
      {
	fp->write(cos->getMangledName().c_str());
	fp->write(", ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str()); //pos +25
	fp->write(" + T::ATOM_FIRST_STATE_BIT, ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str()); //postoeff
	fp->write(", uc.LookupUlamElementTypeFromContext(");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());;
	fp->write("), uc"); //t3631,t3692,3
	fp->write(");"); GCNL;
      }
    else //from atomref
      {
	if(usePassVal)
	  {
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    if(vstor == TMPBITVAL)
	      {
		fp->write(",");
		fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
		fp->write(" + T::ATOM_FIRST_STATE_BIT, ");
		fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str()); //postoeff
		fp->write(", uc.LookupUlamElementTypeFromContext(");
		fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());;
		fp->write("), uc");
		fp->write(");"); GCNL;
	      }
	    else //tmpautoref
	      {
		fp->write(", ");
		fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
		fp->write(" + T::ATOM_FIRST_STATE_BIT, ");
		fp->write(uvpass.getTmpVarAsString(m_state).c_str());
		fp->write(".GetEffectiveSelf()");
		fp->write(");"); GCNL;
	      }
	  }
	else if(makeValFromPass)
	  {
	    fp->write(m_state.getTmpVarAsString(vuti, tmpvarstg, TMPBITVAL).c_str());
	    fp->write(",");
	    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
	    fp->write(" + T::ATOM_FIRST_STATE_BIT, ");
	    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str()); //postoeff
	    fp->write(", uc.LookupUlamElementTypeFromContext(");
	    fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());;
	    fp->write("), uc");
	    fp->write(");"); GCNL;
	  }
	else if(cosut->isReference()) //fm atom-ref t3986,t41005,6,7,t41143
	  {
	    fp->write(cos->getMangledName().c_str()); //reference
	    fp->write(", ");
	    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
	    fp->write(" + T::ATOM_FIRST_STATE_BIT, ");
	    fp->write(cos->getMangledName().c_str()); //ref
	    fp->write(".GetEffectiveSelf()");
	    fp->write(");"); GCNL;
	  }
      }

    if(tobe->isReference()) //to quark-ref, done.
      {
	uvpass = UVPass::makePass(tmpVarRef, TMPAUTOREF, reftobeType, m_state.determinePackable(reftobeType), m_state, 0, 0); //POS 0 rightjustified
      }
    else //tobe non-ref quark
      {
	//immediate cntr makes complete quark fm scattered bases
	//in its ref
	//t3697, error t3691, t41143 atom/ref->quark
	s32 tmpVarSuper = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //BV, not const
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarSuper, tstor).c_str());
	fp->write(" = ");
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //anonymous immediate
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(reftobeType, tmpVarRef, TMPAUTOREF).c_str());
	fp->write(").read();"); GCNL;

	//update the uvpass to have the casted quark value
	uvpass = UVPass::makePass(tmpVarSuper, tstor, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
  } //genCodeCastAtomToQuark (helper)

  void NodeCast::genCodeCastQuarkRefToAtom(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal)
  {
    UTI tobeType = getCastType();
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI  vuti = uvpass.getPassTargetType();
    assert(m_state.okUTItoContinue(vuti));
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    //stack still empty when func call returns self (t41065, case foofunc())
    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    bool makeValFromPass = (tmpvarstg > 0);

    s32 tmpVarIs = m_state.getNextTmpVarNumber();
    // "downcast" might not be true; compare to be sure the atom is-a quark "Foo"
    // by inheritance (see t3631)

    //Type used twice, put in a tmp var
    s32 tmpVarType = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const s32 ");
    fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());;
    fp->write(" = ");

    //From a quark, cast to atom..check load and check Type tmpvar
    //e.g. a quark here would fail, if not a superclass && ref
    if(usePassVal)
      {
	assert(vut->isAltRefType());
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
      }
    else if(makeValFromPass)
      {
	fp->write(m_state.getTmpVarAsString(vuti, tmpvarstg, TMPBITVAL).c_str());  //t3697, t41153
      }
    else
      {
	assert(cosut->isReference()); //t3697,t3834 (self is a ref,too)
	fp->write(cos->getMangledName().c_str());
      }

    fp->write(".");
    fp->write("GetType();"); GCNL;
    //got atom's Element Type in tmpvar.

    //insure the qref has a (MFM) type that's not UNDEFINED
    // don't use quark read into tmpvar in uvpass (u32); t41153,4,5)
    m_state.indentUlamCode(fp);
    fp->write("const bool ");
    fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs, TMPREGISTER).c_str());
    fp->write(" = (");
    fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());
    fp->write(" != T::ATOM_UNDEFINED_TYPE);"); GCNL; //subatomic type


    m_state.indentUlamCode(fp);
    fp->write("if(!");
    fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs, TMPREGISTER).c_str());
    fp->write(")\n");

    m_state.m_currentIndentLevel++;
    m_state.indentUlamCode(fp);
    fp->write("FAIL(BAD_CAST);"); GCNL;
    fp->write("\n");
    m_state.m_currentIndentLevel--;

    //Next pos of quark in atom/element..
    //offset of descendant NOT always 0 fm start of state bits (ulam-5)
    s32 tmpVarPos = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const s32 ");
    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());

    //sub-atomic vuti, tobetype is a defined atom (e.g. t41153)
    // subtraction gets pos of element's state bits from quark ref
    // try to be more like atomof
    fp->write(" = -(");
    if(usePassVal)
      fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    else if(makeValFromPass)
      m_state.abortShouldntGetHere();
    else
      fp->write(cos->getMangledName().c_str());
    fp->write(".GetPosToEffectiveSelf()"); //t3684,t3701,t41367
    fp->write("); //relpos back to state"); GCNL;


    //t3631, t3692, t3693, t3697, t3701, t3756, t3757, t3789, t3834, t3837
    // might be ancestor quark
    // can't let Node::genCodeReadIntoTmpVar do this for us (we need a ref!):
    //    if(!tobe->isReference()) // not isAltRefType, could be ALT_AS (t3835)

    // here, making a complete object quark from an atom/ref, need
    // that CLEVER trick i do believe (i.e. make a ref, then its
    // immediate) in order to gather possible scattered base
    // classes. Can't make a complete atom from a quark, unless
    // it's a quarkref.
    UTI reftobeType = m_state.getUlamTypeAsRef(tobeType);
    UlamType * reftobe = m_state.getUlamTypeByIndex(reftobeType);
    s32 tmpVarAtomRef = m_state.getNextTmpVarNumber();
    //from quark ref to atom/atomref (fail/t3691),t3697,t41153
    // (see also NodeAtomof)

    m_state.indentUlamCode(fp);
    fp->write(reftobe->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(reftobeType, tmpVarAtomRef, TMPAUTOREF).c_str());
    fp->write("(");
    if(usePassVal)
      fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    else if(makeValFromPass)
      {
	m_state.abortShouldntGetHere();
      }
    else
      fp->write(cos->getMangledName().c_str());
    fp->write(","); //entire storage
    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str()); //pos -25
    fp->write(" - T::ATOM_FIRST_STATE_BIT");
    fp->write(");"); GCNL;

    if(tobe->isReference())
      {
	uvpass = UVPass::makePass(tmpVarAtomRef, TMPAUTOREF, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    else
      {
	s32 tmpVarAtom = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarAtom, TMPBITVAL).c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(reftobeType, tmpVarAtomRef, TMPAUTOREF).c_str());
	fp->write(");"); GCNL;

	uvpass = UVPass::makePass(tmpVarAtom, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    //uvpass updated to have the casted value
  } //genCodeCastQuarkRefToAtom (helper)

  void NodeCast::genCodeCastDescendant(File * fp, UVPass & uvpass)
  {
    UTI tobeType = getCastType(); //quark tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    TMPSTORAGE tstor = tobe->getTmpStorageTypeForTmpVar();

    UTI vuti = uvpass.getPassTargetType(); //replace
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    //vuti is subclass of tobeType (or ref tobe), or visa versa (t3757);
    // reftypeof same ?: (t41069); or DM same type as ancestor (t41292)
    assert(m_state.isClassASubclassOf(vuti, tobeType) || m_state.isClassASubclassOf(tobeType, vuti) || (m_state.isARefTypeOfUlamType(tobeType, vuti) == UTIC_SAME));

    // "downcast" might not be true; compare to be sure the quark is-related to quark (t3758)
    if(m_state.isClassASubclassOf(tobeType, vuti)) //super (vuti) -> sub (tobe)
      {
	m_state.indentUlamCode(fp);
	fp->write("if(!");
	fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
	fp->write(".");
	fp->write(m_state.getIsMangledFunctionName(tobeType)); //UlamElement IsMethod
	fp->write("(");
	fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(vuti)); //efficiency
	fp->write("u) )");
	fp->write(" /* ");
	fp->write(m_state.getUlamTypeNameBriefByIndex(vuti).c_str());
	fp->write(" */ \n");

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(BAD_CAST);"); GCNL;
	m_state.m_currentIndentLevel--;
      }

    if(m_state.m_currentObjSymbolsForCodeGen.empty()) //t41301 (e.g. implicit safe cast of explicit cast)
      {
	if(UlamType::compare(tobeType, vuti, m_state) == UTIC_SAME)
	  {
	    uvpass.setPassTargetType(tobeType); //minimal casting
	    return; //done
	  } //else t3560
      }

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    UTI cosuti = cos->getUlamTypeIdx();
    UTI refcosuti = m_state.getUlamTypeAsRef(cosuti, ALT_REF);
    UlamType * refcosut = m_state.getUlamTypeByIndex(refcosuti);

    bool usePassVal = m_state.m_currentObjSymbolsForCodeGen.empty() && ((uvpass.getPassStorage() == TMPBITVAL) || (uvpass.getPassStorage() == TMPAUTOREF));
    bool makeValFromPass = m_state.m_currentObjSymbolsForCodeGen.empty() && !usePassVal;

    s32 tmpvarDM = 0;
    s32 posToDM = 0;
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	if((UlamType::compare(stgcosuti, vuti, m_state) != UTIC_SAME))
	  {
	    assert(cos->isDataMember()); //what else could it be?

	    //e.g. t41366 Bar& ref = eltref.t;
	    // where Bar is a base of quark t, a dm of element eltref;
	    posToDM = cos->getPosOffset();

	    // when stgcos is a ref, so that we can keep posToEff
	    //accurate, /we need tmp immediate ref to dm, when
	    //tobe is a baseclass type (t41366)
	    if(m_state.isReference(stgcosuti) && (UlamType::compare(m_state.getUlamTypeAsDeref(tobeType),cosuti,m_state) != UTIC_SAME))
	      {
		tmpvarDM = m_state.getNextTmpVarNumber();
		m_state.indentUlamCode(fp);
		fp->write(refcosut->getLocalStorageTypeAsString().c_str()); //tmp immediate ref element from-type
		fp->write(" ");
		fp->write(m_state.getTmpVarAsString(refcosuti, tmpvarDM, TMPAUTOREF).c_str());
		fp->write("(");
		fp->write(stgcos->getMangledName().c_str()); //ref
		fp->write(", ");
		if(Node::needAdjustToStateBits(cosuti))
		  fp->write("T::ATOM_FIRST_STATE_BIT + ");
		fp->write_decimal_unsigned(posToDM);
		fp->write("u, &");
		fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str()); //effSelf of dm
		fp->write(");"); GCNL;

		posToDM = 0; //done
	      }
	    else
	      {
		assert((u32) posToDM == uvpass.getPassPos()); //sanity (if true, don't need posToDM);
	      }
	  }
      }
    else //else using uvpass in some manner..
      {
	posToDM = uvpass.getPassPos(); //needed?
      }

    s32 tmpVarPos = m_state.getNextTmpVarNumber();
    genPositionOfBaseIntoATmpVar(fp, tmpVarPos, uvpass, stgcos, cos);

    s32 tmpStg = m_state.getNextTmpVarNumber();
    if(makeValFromPass)
      {
	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members, tmp immediate from-type
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(vuti, tmpStg, TMPBITVAL).c_str());
	fp->write("(");
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(");"); GCNL;
      }

    //e.g. a quark var here would be ok if a baseclass (ow runtime failure) (t3560, t3757)
    //use copy constructor taking its immediate ref (e.g. t41302)
    //start read fm pos var, for total length of quark/s in family tree
    UTI reftobeType = m_state.getUlamTypeAsRef(tobeType);
    UlamType * reftobe = m_state.getUlamTypeByIndex(reftobeType);
    s32 tmpVarRef = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    fp->write(reftobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(reftobeType,tmpVarRef,TMPAUTOREF).c_str());
    fp->write("(");

    if(usePassVal)
      {
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(", "); //ready for pos
      }
    else if(makeValFromPass)
      {
	fp->write(m_state.getTmpVarAsString(vuti,tmpStg,TMPBITVAL).c_str());
	fp->write(", "); //ready for pos
      }
    //self might be ALT_AS or ALT_REF (t41359,8)
    else if(stgcosut->isReference())
      {
	if(tmpvarDM > 0)
	  {
	    assert(cos->isDataMember());
	    fp->write(m_state.getTmpVarAsString(refcosuti, tmpvarDM, TMPAUTOREF).c_str());
	  }
	else
	  {
	    fp->write(stgcos->getMangledName().c_str()); //reference
	  }
	fp->write(", "); //ready for pos
      }
    else //stg is non-ref
      {
	//uses bit storage cntr; keep effSelf (t41357,5)
	fp->write(stgcos->getMangledName().c_str());
	fp->write(", ");
      }

    fp->write(m_state.getTmpVarAsString(Int,tmpVarPos,TMPREGISTER).c_str());
    if(posToDM > 0)
      {
	fp->write(" + ");
	fp->write_decimal_unsigned(posToDM);
	fp->write("u");
      }

    //delta fm pos to element state bits for non-ref stg (ulam-5)
    if(usePassVal)
      {
	if(Node::needAdjustToStateBits(vuti))
	  fp->write(" + T::ATOM_FIRST_STATE_BIT"); //non-ref element case
	fp->write(", ");

	if(m_state.isReference(vuti)) //t3735, t3562, t3621
	  {
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(".GetEffectiveSelf()");
	    fp->write(");"); GCNL;
	  }
	else //fm non-ref
	  {
	    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str()); //extra postoeff arg
	    fp->write(", &");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str()); //keep stg effSelf
	    fp->write(", uc");
	    fp->write(");"); GCNL;
	  }
      }
    else if(makeValFromPass) //i.e. non-ref
      {
	if(Node::needAdjustToStateBits(vuti))
	  fp->write(" + T::ATOM_FIRST_STATE_BIT");
	fp->write(", ");

	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
	fp->write(", &");
	fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str()); //keeps effSelf
	fp->write(", uc");
	fp->write(");"); GCNL;
      }
    else if(cos->isDataMember())
      {
	if(tmpvarDM == 0) //stgcos is non-ref, accepts delta arg
	  {
	    if(Node::needAdjustToStateBits(stgcosuti) || Node::needAdjustToStateBits(cosuti)) //t41292, t3735
	      fp->write(" + T::ATOM_FIRST_STATE_BIT");
	    fp->write(", ");

	    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str()); //delta
	    fp->write(", &");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str());
	    fp->write(", uc");
	    fp->write(");"); GCNL; //cos is data member,t41364
	  }
	else
	  {
	    fp->write(");"); GCNL; //cos is data member,t41366
	  }
      }
    else if(!stgcosut->isReference()) //not AltRefType, case ALT_AS (t41365)
      {
	if(Node::needAdjustToStateBits(stgcosuti))
	  fp->write(" + T::ATOM_FIRST_STATE_BIT");
	fp->write(", ");

	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str()); //delta
	fp->write(", &");
	fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
	fp->write(", uc");
	fp->write(");"); GCNL;
      }
    else //stg/cos is ref
      {
	fp->write(", ");
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".GetEffectiveSelf()");
	fp->write(");"); GCNL;
      }

    if(tobe->isAltRefType())
      {
	// to be ref, don't read! done. t3757
	uvpass = UVPass::makePass(tmpVarRef, TMPAUTOREF, tobeType, m_state.determinePackable(tobeType), m_state, uvpass.getPassPos(), 0); //POS 0 is justified; will name id help?
      }
    else
      {
	s32 tmpVarSuper = m_state.getNextTmpVarNumber();
	//immediate cntr makes complete quark fm scattered bases
	//in its ref (t3560,t3757,t41357)
	m_state.indentUlamCode(fp);
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //BV, not const
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarSuper, tstor).c_str());
	fp->write(" = ");
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //anonymous immediate
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(reftobeType, tmpVarRef, TMPAUTOREF).c_str());
	fp->write(").read();"); GCNL;

	//update the uvpass to have the casted quark value
	uvpass = UVPass::makePass(tmpVarSuper, tstor, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastDescendant

  void NodeCast::genCodeCastAncestorQuarkAsSub(File * fp, UVPass & uvpass)
  {
    // quark to sub-quark covered by CastDescendantQuark
    UTI tobeType = getCastType(); //related subclass tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    TMPSTORAGE tstor = tobe->getTmpStorageTypeForTmpVar();
    ULAMCLASSTYPE tclasstype = tobe->getUlamClassType();

    UTI vuti = uvpass.getPassTargetType(); //replace
    TMPSTORAGE vstor = uvpass.getPassStorage();

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);
    UTI stgcosuti = stgcos->getUlamTypeIdx();

    assert(m_state.isReference(stgcosuti)); //not AltRefType

    // "downcast" might not be true; compare to be sure the transient is-related to quark
    if(m_state.isClassASubclassOf(tobeType, vuti)) //super (vuti) -> sub (tobe)
      {
	m_state.indentUlamCode(fp);
	fp->write("if(!");
	fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
	fp->write(".");
	fp->write(m_state.getIsMangledFunctionName(tobeType)); //UlamElement IsMethod
	fp->write("("); //one arg
	fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(vuti)); //efficiency
	fp->write("u))");
	fp->write(" /* ");
	fp->write(m_state.getUlamTypeNameBriefByIndex(vuti).c_str());
	fp->write(" */ \n");

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(BAD_CAST);"); GCNL;
	m_state.m_currentIndentLevel--;
      }

    bool usePassVal = m_state.m_currentObjSymbolsForCodeGen.empty() && ((vstor == TMPBITVAL) || (vstor == TMPAUTOREF));
    bool makeValFromPass = m_state.m_currentObjSymbolsForCodeGen.empty() && !usePassVal;
    assert(!makeValFromPass); //can't make sub from immediate base

    //offset of descendant NOT always 0 fm start of state bits (ulam-5)
    s32 tmpVarPos = m_state.getNextTmpVarNumber();
    genPositionOfBaseIntoATmpVar(fp, tmpVarPos, uvpass, stgcos, cos);

    //use immediate copy cntr taking its immediate-ref (e.g. t41302)
    //start read fm pos var, for total length of all bases in family tree
    // -- no matter how they are scattered about (ulam-5).
    UTI reftobeType = m_state.getUlamTypeAsRef(tobeType);
    UlamType * reftobe = m_state.getUlamTypeByIndex(reftobeType);

    s32 tmpVarRef = m_state.getNextTmpVarNumber();

    //t3790,t3789 case 4: (fails to compile)
    //Incompatible class types TW& and Qbase& used to initialize reference 'twref'.
    m_state.indentUlamCode(fp);
    fp->write(reftobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(reftobeType, tmpVarRef, TMPAUTOREF).c_str());
    fp->write("(");
    if(usePassVal)
      {
	if(vstor == TMPAUTOREF)
	  {
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(", ");
	    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
	    fp->write(");"); GCNL;
	  }
	else //tmpbitval
	  m_state.abortShouldntGetHere();
      }
    else if(makeValFromPass)
      m_state.abortShouldntGetHere();
    else
      {
	fp->write(stgcos->getMangledName().c_str()); //a ref
	fp->write(", ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
	if(tclasstype == UC_ELEMENT)
	  {
	    fp->write(", ");
	    fp->write(stgcos->getMangledName().c_str()); //a ref
	    fp->write(".GetEffectiveSelf()"); //maintains eff self t3756
	  }
	fp->write(");"); GCNL;
      }

    if(tobe->isAltRefType())
      {
	uvpass = UVPass::makePass(tmpVarRef, TMPAUTOREF, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    else
      {
	//Compile-time error to downcast from a ref to a subclass instance. 20160616. WHY???
	//t3789 case 2 (init): TW tapple = (TW) qref;
	//t3789 case 3 (assign): tapple2 = (TW) qref;
	//m_state.abortShouldntGetHere(); //error caught already
	//immediate cntr makes complete transient fm ref's scattered
	// bases; read and return entire base in tmp storage
	s32 tmpVarVal = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //BV, not const
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarVal, tstor).c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(reftobeType, tmpVarRef, TMPAUTOREF).c_str());
	fp->write(".read());"); GCNL;

	//update the uvpass to have the casted quark value
	uvpass = UVPass::makePass(tmpVarVal, tstor, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastAncestorQuarkAsSub

  //helper: pos of tobe-type in fm-type/uvpass (maybe goes into Node.cpp)
  // used to make the tobe variable
  void NodeCast::genPositionOfBaseIntoATmpVar(File * fp, u32 tmpvarpos, UVPass & uvpass, Symbol * stgcos, Symbol * cos)
  {
    UTI tobeType = getCastType();
    UTI vuti = uvpass.getPassTargetType();
    bool usePassVal = m_state.m_currentObjSymbolsForCodeGen.empty() && ((uvpass.getPassStorage() == TMPBITVAL) || (uvpass.getPassStorage() == TMPAUTOREF));
    bool makeValFromPass = m_state.m_currentObjSymbolsForCodeGen.empty() && !usePassVal;

    m_state.indentUlamCode(fp);
    fp->write("const s32 ");
    fp->write(m_state.getTmpVarAsString(Int, tmpvarpos, TMPREGISTER).c_str()); //ulam-5
    fp->write(" = ");

    if(usePassVal)
      {
	if(uvpass.getPassStorage() == TMPAUTOREF)
	  {
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(".GetEffectiveSelf()->");
	    fp->write(m_state.getGetRelPosMangledFunctionName(vuti));
	    fp->write("(");
	    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(tobeType)); //efficiency
	    fp->write("u ");
	    fp->write("/* ");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(tobeType).c_str());
	    fp->write(" */");
	    fp->write(") - ");
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(".GetPosToEffectiveSelf();"); GCNL;
	  }
	else //tmpbitval
	  {
	    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	    fp->write(".");
	    fp->write(m_state.getGetRelPosMangledFunctionName(vuti)); //UlamElement GetRelPosMethod
	    fp->write("(");
	    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(tobeType)); //efficiency
	    fp->write("u); //relpos of ");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(tobeType).c_str());
	    GCNL;
	  }
      }
    else if(makeValFromPass) //an immediate complete obj
      {
	fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	fp->write(".");
	fp->write(m_state.getGetRelPosMangledFunctionName(vuti)); //UlamElement GetRelPosMethod
	fp->write("(");
	fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(tobeType)); //efficiency
	fp->write("u); //relpos of ");
	fp->write(m_state.getUlamTypeNameBriefByIndex(tobeType).c_str());
	GCNL;
      }
    else
      {
	assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
	UTI stgcosuti = cos->getUlamTypeIdx(); //t41141
	UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

	if(m_state.isClassASubclassOf(tobeType, vuti)) //base(vuti)->sub(tobe)
	  {
	    assert(m_state.isReference(vuti)); //sanity
	    //both refs (t3757)
	    if(stgcosut->isReference()) //not AltRefType
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetEffectiveSelf()->");
		fp->write(m_state.getGetRelPosMangledFunctionName(vuti));
		fp->write("(");
		fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(tobeType)); //efficiency
		fp->write("u ");
		fp->write("/* ");
		fp->write(m_state.getUlamTypeNameBriefByIndex(tobeType).c_str());
		fp->write(" */");
		fp->write(") - ");
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetPosToEffectiveSelf();"); GCNL;
	      }
	    else
	      {
		m_state.abortShouldntGetHere();
	      }
	  }
	else if(m_state.isClassASubclassOf(vuti, tobeType)) //sub(vuti)->base(tobe)
	  {
	    if(stgcosut->isReference()) //not AltRefType
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetEffectiveSelf()->");
		fp->write(m_state.getGetRelPosMangledFunctionName(vuti));
		fp->write("(");
		fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(tobeType)); //efficiency
		fp->write("u ");
		fp->write("/* ");
		fp->write(m_state.getUlamTypeNameBriefByIndex(tobeType).c_str());
		fp->write(" */");
		fp->write(") - ");
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetPosToEffectiveSelf();"); GCNL;
	      }
	    else
	      {
		//stg not a ref, so we can look up relpos at compile time
		u32 baserelpos = UNRELIABLEPOS;
		AssertBool gotbaserelpos = m_state.getABaseClassRelativePositionInAClass(vuti, tobeType, baserelpos);
		assert(gotbaserelpos);
		fp->write_decimal_unsigned(baserelpos);
		fp->write("; //relpos of ");
		fp->write(m_state.getUlamTypeNameBriefByIndex(tobeType).c_str());
		GCNL;
	      }
	  }
	else if(stgcos->isSelf() && (stgcos != cos))
	  {
	    assert(!(cos->isTmpVarSymbol() && ((SymbolTmpVar *) cos)->isBaseClassRef())); //sanity, pls
	    assert(cos->isDataMember()); //more sanity, no longer implicit self

	    fp->write_decimal(cos->getPosOffset()); //t41141, t41292
	    fp->write("); //dm"); GCNL;
	  }
	else
	  {
	    fp->write("0; //default"); GCNL;
	  }
      }
  } //genPositionOfBaseIntoATmpVar (helper)

  //SAME TYPES, to a reference, from either a reference or not
  void NodeCast::genCodeCastAsReference(File * fp, UVPass & uvpass)
  {
    UTI fromuti = uvpass.getPassTargetType();
    UTI tobeType = getCastType();
    if((m_state.isAtomRef(tobeType) && !m_state.isAltRefType(fromuti)))
      {
	return; //no casting, need to know for writing
      }
    uvpass.setPassTargetType(tobeType); //minimal casting
  } //genCodeCastAsReference

  //SAME TYPES, from a reference, to a non-reference
  void NodeCast::genCodeCastFromAReference(File * fp, UVPass & uvpass)
  {
    UTI tobeType = getCastType();
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    assert(!tobe->isAltRefType());
    assert(m_state.isAtom(tobeType) || m_state.isASeenElement(tobeType) || m_state.isAPrimitiveType(tobeType)); //sanity? terr/t3681,t3843; t3583,t3611
    uvpass.setPassTargetType(tobeType); //minimal casting; including atomref to atom(non-ref)
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastFromAReference

  //SAME TYPES, to a reference, from either a reference or not, or different ref type
  void NodeCast::genCodeToStoreIntoCastAsReference(File * fp, UVPass & uvpass)
  {
    UTI tobeType = getCastType();
    if(m_node->isAConstantClass() || m_node->isAConstant())
      {
	assert(m_state.isConstantRefType(tobeType)); //t41238-9,t41240,t41242,error/t41248,error/t41253
      }
    uvpass.setPassTargetType(tobeType); //minimal casting, t3812
    return;
  } //genCodeToStoreIntoCastAsReference

  void NodeCast::genCodeToStoreIntoCastFromAReference(File * fp, UVPass & uvpass)
  {
    UTI tobeType = getCastType();
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    assert(!tobe->isAltRefType());
    uvpass.setPassTargetType(tobeType); //minimal casting; including atomref to atom(non-ref)
    return;
  } //genCodeToStoreIntoCastFromAReference

  bool NodeCast::needsACast()
  {
    UTI tobeType = getCastType();
    UTI nodeType = m_node->getNodeType();

    ULAMTYPECOMPARERESULTS uticr = UlamType::compare(nodeType, tobeType, m_state);
    if(uticr == UTIC_DONTKNOW)
      {
	std::ostringstream msg;
	msg << "Casting 'incomplete' types: ";
	msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
	msg << "(UTI" << nodeType << ") to be ";
	msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	msg << "(UTI" << tobeType << ")";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	return false;
      }

    if(uticr == UTIC_SAME)
      return false; //short-circuit if same exact type

    // casting fm ref to complete obj of same type, needs casting, unless an element, since no telling where the bases might be otherwise; cast fm complete obj to its ref doesn't unless explicit. (ulam-5)
    // references are same sizes so no casting needed except to change the uti;
    // NOT including (?) mixing types (e.g. quark ref to atom, or quark ref to sub class)
    // NOT including explicit casts (e.g. t41067), or Strings (t3962),
    // including unnecessary atom/ref to atom/ref (t41139)
    if((m_state.isARefTypeOfUlamType(tobeType, nodeType) == UTIC_SAME) && (!m_state.isReference(nodeType) /*|| m_state.isASeenElement(nodeType)*/ ) && (!isExplicitCast() || m_state.isAStringType(tobeType) || m_state.isAtom(tobeType)))
      return false; //special case of casting

    UlamType * nodeut = m_state.getUlamTypeByIndex(nodeType);
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    ULAMTYPE typEnum = tobe->getUlamTypeEnum();
    ULAMTYPE nodetypEnum = nodeut->getUlamTypeEnum();

    //requires toInt
    if(nodeut->getUlamClassType() == UC_QUARK)
      {
	if(tobe->isNumericType())
	  {
	    assert(m_state.quarkHasAToIntMethod(nodeType)); //checked by c&l
	    return (tobeType == Int); //no need to cast
	  }
	return true;
      }
    // consider user requested first, then type and size independently;
    // size secondary when different classes, possibly related (e.g. t3779)
    // even constant may need casting (e.g. narrowing for saturation)
    // Bool constants require casts to generate "full" true UVPass (>1-bit).
    return( isExplicitCast() || (typEnum == Class) || (typEnum != nodetypEnum) || (m_state.getBitSize(tobeType) != m_state.getBitSize(nodeType)) || ( (nodetypEnum == Bool) && m_node->isAConstant() && (m_state.getBitSize(tobeType)>1)));
  } //needsACast

} //end MFM
