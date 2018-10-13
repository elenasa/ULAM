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
    m_node->genCode(fp, uvpass); //might be unused (e.g. classes, atoms)
    if(needsACast()) //(bypasses references)
      {
	genCodeReadIntoATmpVar(fp, uvpass); // cast.
      }
    else if(m_state.isAltRefType(getCastType())) //to ref type
      genCodeCastAsReference(fp, uvpass); //minimal casting
    else if(m_state.isAltRefType(m_node->getNodeType())) //from ref type
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
     return; //nothing to do!

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
     uvpass = UVPass::makePass(tmpVarCastNum, tobe->getTmpStorageTypeForTmpVar(), derefTobe, m_state.determinePackable(tobeType), m_state, 0, uvpass.getPassNameId()); //POS 0 rightjustified; pass along name id
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
      return genCodeCastDescendantQuark(fp, uvpass); //rt check for super->sub

    if((tclasstype == UC_QUARK) && (vclasstype == UC_ELEMENT))
      return genCodeCastDescendantElement(fp, uvpass); //no rt check: sub->super

    // c&l insures quark is a ref.
    if((tclasstype == UC_ELEMENT) && (vclasstype == UC_QUARK))
      return genCodeCastAncestorQuarkAsSubElement(fp, uvpass); //rt check: super->sub

    if((tclasstype == UC_QUARK) && (vclasstype == UC_TRANSIENT))
      return genCodeCastDescendantTransient(fp, uvpass); //no rt check: sub->super

    if((tclasstype == UC_TRANSIENT) && (vclasstype == UC_TRANSIENT))
      return genCodeCastDescendantTransient(fp, uvpass); //t3967 //rt check for super->sub

    // c&l insures quark is a ref. ???
    if((tclasstype == UC_TRANSIENT) && (vclasstype == UC_QUARK))
      return genCodeCastAncestorQuarkAsSubTransient(fp, uvpass); //rt check: super->sub

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
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getPassTargetType();  //replace
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    Symbol * stgcos = NULL;

    if(tobe->isReference()) //not isAltRefType, could be ALT_AS (t3835)
      {
	// don't repeat genCodeToStoreInto when m_node is a function call that returns an atomref
	if(m_state.m_currentObjSymbolsForCodeGen.empty() && !m_node->isFunctionCall())
	  {
	    //safe to call genCodeToStoreInto since nodeType must be storeintoable;
	    //except when assignment of Foo.instanceof (t3818)
	    UVPass ruvpass;
	    m_node->genCodeToStoreInto(fp, ruvpass); //No need to load lhs into tmp (T); symbol's in COS vector
	  }
	//else (t41051) ILLEGAL_ARGUMENT

	assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
	stgcos = m_state.m_currentObjSymbolsForCodeGen.back();
      }

      // "downcast" might not be true; compare to be sure the atom is an element "Foo"
      if(m_state.isAtom(vuti)) //from atom-to-element
	{
	  m_state.indentUlamCode(fp);
	  fp->write("if(!");
	  fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
	  fp->write(".");
	  fp->write(m_state.getIsMangledFunctionName(tobeType));
	  fp->write("(");
	  if(stgcos)
	    {
	      assert(m_state.isAtom(stgcos->getUlamTypeIdx()));
	      fp->write(stgcos->getMangledName().c_str()); //t3754, t3408 Tue Mar 21 12:01:17 2017
	      fp->write(".read()");
	    }
	  else
	    {
	      fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	      if(uvpass.getPassStorage() == TMPBITVAL)
		{
		  fp->write(".");
		  fp->write(vut->readMethodForCodeGen().c_str());
		  fp->write("()");
		}
	    }

	  fp->write("))"); GCNL;

	  m_state.m_currentIndentLevel++;
	  m_state.indentUlamCode(fp);
	  fp->write("FAIL(BAD_CAST);"); GCNL;
	  m_state.m_currentIndentLevel--;

	  if(tobe->isReference()) //t3754 (not isAltRefType, , could be ALT_AS t3835)
	  {
	    assert(stgcos);
	    UTI stgcosuti = stgcos->getUlamTypeIdx();
 	    s32 tmpeleref = m_state.getNextTmpVarNumber(); //tmp since no variable name
	    m_state.indentUlamCode(fp);
	    fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	    fp->write(" ");
	    fp->write(m_state.getTmpVarAsString(tobeType, tmpeleref, TMPBITVAL).c_str());
	    fp->write("(");
	    fp->write(stgcos->getMangledName().c_str()); //assumes only one!!!
	    fp->write(", ");
	    //must displace the Typefield for element ref
	    fp->write("+ T::ATOM_FIRST_STATE_BIT, ");
	    if(m_state.isAtomRef(stgcosuti)) //t3754
	      {
		fp->write(stgcos->getMangledName().c_str()); //assumes only one!!!
		fp->write(".GetEffectiveSelf()"); //maintains eff self
	      }
	    else
	      {
		fp->write("&");
		fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
		fp->write(", uc"); //t3754
	      }
	    fp->write(");"); GCNL;
	    uvpass = UVPass::makePass(tmpeleref, TMPBITVAL, tobeType, UNPACKED, m_state, 0, stgcos->getId()); //POS moved left for type; pass along name id);
	  }
	  else
	    {
	      TMPSTORAGE vstor = uvpass.getPassStorage();
	      if(m_state.isAtomRef(vuti) && (vstor != TMPTATOM)) //from atomref-to-element
		{
		  //read into a T (t41157), unless already a T from .atomof (t3756)
		  s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for atomref
		  m_state.indentUlamCode(fp);
		  fp->write("const ");
		  fp->write(vut->getTmpStorageTypeAsString().c_str()); //for C++ local vars
		  fp->write(" ");
		  fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, TMPTATOM).c_str());
		  fp->write(" = ");
		  fp->write(uvpass.getTmpVarAsString(m_state).c_str());
		  fp->write(".read();"); GCNL;

		  uvpass = UVPass::makePass(tmpVarNum2, TMPTATOM, tobeType, UNPACKED, m_state, 0, uvpass.getPassNameId());
		}
	      else //no need to read atom-based element (e.g. t3410, 3277)
		uvpass.setPassTargetType(tobeType); //same variable
	    }
	}
      else if(m_state.isAtomRef(tobeType))
	{
	  //from element-to-atom
	  //element-ref to atom-ref is why we couldn't have packed elements!!! t3753
	    assert(stgcos);
	    UTI stgcosuti = stgcos->getUlamTypeIdx();
	    UlamType * atomut = m_state.getUlamTypeByIndex(UAtom);
	    s32 tmpatomref = m_state.getNextTmpVarNumber(); //tmp since no variable name
	    m_state.indentUlamCode(fp);
	    fp->write(atomut->getUlamTypeImmediateAutoMangledName().c_str()); //tobe as auto
	    fp->write("<EC> ");
	    fp->write(m_state.getTmpVarAsString(tobeType, tmpatomref, TMPBITVAL).c_str());
	    fp->write("(");
	    fp->write(stgcos->getMangledName().c_str()); //assumes only one!!!
	    fp->write(", ");
	    //must displace the Typefield if a ref
	    if(m_state.isAltRefType(stgcosuti))
	      {
		fp->write("- T::ATOM_FIRST_STATE_BIT);"); GCNL; //t3753
	      }
	    else
	      {
		fp->write("0u, uc);"); GCNL;
	      }
	    uvpass = UVPass::makePass(tmpatomref, TMPBITVAL, tobeType, UNPACKED, m_state, 0, stgcos->getId()); //POS moved left for type; pass along name id);
	  }
      else if(m_state.isAtom(tobeType))
	{
	  // to atom, from T (read from element or element ref)
	  //No longer, convert T to AtomBitStorage (e.g. t3697, t3637)
	  uvpass.setPassTargetType(tobeType); //same variable
	}
      else //no atoms, explicit like-kind elements
	{
	  //explicit cast from element-ref, to element or element-ref; or
	  //explicit cast from element storage, to element or element-ref
	  assert(stgcos);
	  UTI stgcosuti = stgcos->getUlamTypeIdx();
	  s32 tmpeleref = m_state.getNextTmpVarNumber();
	  if(m_state.isAltRefType(stgcosuti))
	    {
	      //explicit cast from element-ref, to element or element-ref (t41052)
	      m_state.indentUlamCode(fp);
	      fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	      fp->write(" ");
	      fp->write(m_state.getTmpVarAsString(tobeType, tmpeleref, TMPBITVAL).c_str());
	      fp->write("(");
	      fp->write(stgcos->getMangledName().c_str()); //assumes only one!!!
	      if(m_state.isAltRefType(tobeType))
		{
		  fp->write(", 0u, &");
		  fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
		}
	      fp->write(");"); GCNL;
	      uvpass = UVPass::makePass(tmpeleref, TMPBITVAL, tobeType, UNPACKED, m_state, 0, stgcos->getId()); //POS moved left for type; pass along name id);
	    }
	  else
	    {
	      //from element (storage), to element or element-ref
	      //explicit cast element to element ref (e.g. question colon, t41067)
	      m_state.indentUlamCode(fp);
	      fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	      fp->write(" ");
	      fp->write(m_state.getTmpVarAsString(tobeType, tmpeleref, TMPBITVAL).c_str());
	      fp->write("(");
	      fp->write(stgcos->getMangledName().c_str()); //assumes only one!!!
	      if(m_state.isAltRefType(tobeType))
		{
		  fp->write(", ");
		  fp->write("+ T::ATOM_FIRST_STATE_BIT, &"); //displace Typefield of element
		  fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
		  fp->write(", uc"); //non-ref to ref
		}
	      fp->write(");"); GCNL;
	      uvpass = UVPass::makePass(tmpeleref, TMPBITVAL, tobeType, UNPACKED, m_state, 0, stgcos->getId()); //POS moved left for type; pass along name id);
	    }
	}
      //don't read like ref's do!
      //update the uvpass to have the casted type
      m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs
      return;
  } //genCodeCastAtomAndElement

  void NodeCast::genCodeCastAtomAndQuark(File * fp, UVPass & uvpass)
  {
    // quark tobe, only if ancestor
    // also allow quark refs -> atom cast, if ancestor of atom type.
    // e.g. assignments (e.g. t3697, error/t3691).

    UTI tobeType = getCastType();
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI  vuti = uvpass.getPassTargetType();
    assert(m_state.okUTItoContinue(vuti));
    TMPSTORAGE vstor = uvpass.getPassStorage();

    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	// don't repeat genCodeToStoreInto when m_node is a function call that returns an atomref
	//when this is a custom array, the symbol is the "ew" for example,
	//not the atom (e.g. ew[idx]) that has no symbol
	if(!m_node->isFunctionCall())
	  {
	    UVPass ruvpass;
	    m_node->genCodeToStoreInto(fp, ruvpass); //No need to load lhs into tmp (T); symbol's in COS vector
	  }
	else
	  {
	    //(t41051, case 2) ILLEGAL_ARGUMENT
	    //stgcos,cos will be loaded as self (ur).
	    assert(vstor == TMPBITVAL); //t41143 same as isSelf(stguti), when stack is empty
	  }
      }

    //stack still empty when func call returns self (t41065, case foofunc())
    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);
    stgcos = cos; //use from back

    UTI stguti = stgcos->getUlamTypeIdx();
    UlamType * stgut = m_state.getUlamTypeByIndex(stguti);

    s32 tmpVarIs = m_state.getNextTmpVarNumber();
    // "downcast" might not be true; compare to be sure the atom is-a quark "Foo"
    // by inheritance (see t3631)
    if(m_state.isAtom(vuti))
      {
	//Type used twice, put in a tmp var
	s32 tmpVarType = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());;
	fp->write(" = ");

	//locals t3692,3,7,3701, t3756,7,t3837, t3986, t41005,6,7
	//e.g. carray (e.g. error/t3508), a data member
	if(!stgcos->isSelf())
	  fp->write(stgcos->getMangledName().c_str()); //assumes only one!!!
	else
	  fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //t41143
	if(vstor == TMPBITVAL) //stgcos->isSelf())
	  {
	    fp->write("."); //no read for self
	    fp->write("ReadAtom");
	    fp->write("()");
	  }
	else
	  {
	    assert(vstor == TMPTATOM); //t3692
	  }
	fp->write(".GetType();"); GCNL;

	m_state.indentUlamCode(fp);
	fp->write("const bool ");
	fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs, TMPREGISTER).c_str());;
	fp->write(" = ((");

	//when SUBATOMIC quark, uses first state bit position (0)
	// immediate quark's storage cast as a quark? Terrible idea!!
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());;
	fp->write(" == T::ATOM_UNDEFINED_TYPE) ? false : "); //subatomic type

	//internal method, takes uc, u32 and const char*, returns true
	// if same or ancester
	fp->write(m_state.getIsMangledFunctionName(vuti));
	fp->write("(");
	fp->write("uc, ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());;
	fp->write(", &");
	fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
	fp->write("));"); GCNL;
      }
    else
      {
	//From a quark, cast to atom..
	//e.g. a quark here would fail, if not a superclass && ref
	assert(stgut->isAltRefType()); //t3697, t3834 (self is a ref, too!)

	//insure the qref has a (MFM) type that's not UNDEFINED
	// don't use quark read into tmpvar in uvpass (u32); t41153,4,5)
	m_state.indentUlamCode(fp);
	fp->write("const bool ");
	fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs, TMPREGISTER).c_str());;
	fp->write(" = (");
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".");
	fp->write("GetType()");
	fp->write(" != T::ATOM_UNDEFINED_TYPE);"); GCNL; //subatomic type
      }

    m_state.indentUlamCode(fp);
    fp->write("if(!");
    fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs, TMPREGISTER).c_str());
    fp->write(")\n");

    m_state.m_currentIndentLevel++;
    m_state.indentUlamCode(fp);
    fp->write("FAIL(BAD_CAST);"); GCNL;
    fp->write("\n");
    m_state.m_currentIndentLevel--;

    //t3631, t3692, t3693, t3697, t3701, t3756, t3757, t3789, t3834, t3837
    // might be ancestor quark
    // can't let Node::genCodeReadIntoTmpVar do this for us (we need a ref!):
    if(!tobe->isReference()) // not isAltRefType, could be ALT_AS (t3835)
      {
	//t3697, error t3691, t41143 atomref->quark (can this be legal?)
	// uses stgcos since there's no m_varSymbol in this situation.
	// don't forget the read!
	s32 tmpread = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //u32 or T
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpread, tobe->getTmpStorageTypeForTmpVar()).c_str());

	if(m_state.isAtom(vuti)) //from atom/ref->quark
	  {
	    fp->write(" = UlamRef<EC>(");
	    if(m_state.isAtomRef(vuti))
	      {
		if(vstor == TMPBITVAL) //stgcos->isSelf())
		  fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //t41143
		else
		  fp->write(stgcos->getMangledName().c_str()); //ref ??
		fp->write(", ");
	      }
	    fp->write("0u + T::ATOM_FIRST_STATE_BIT, ");
	    fp->write_decimal_unsigned(tobe->getTotalBitSize());
	    fp->write("u, ");
	    if(!m_state.isAtomRef(vuti))
	      {
		if(vstor == TMPBITVAL) //stgcos->isSelf())
		  fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //t41143
		else
		  fp->write(stgcos->getMangledName().c_str());
		fp->write(", &"); //'is' storage
		fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
		fp->write(", UlamRef<EC>::ELEMENTAL, uc).");
	      }
	    else
	      {
		if(vstor == TMPBITVAL) //stgcos->isSelf())
		  fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //t41143
		else
		  fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetEffectiveSelf()"); //maintains eff self
		fp->write(", UlamRef<EC>::ELEMENTAL).");
	      }
	    fp->write(tobe->readMethodForCodeGen().c_str());
	    fp->write("();"); GCNL;
	  }
	else
	  {
	    //from quark to atom/ref
	    fp->write(" = ");
	    if(vstor == TMPBITVAL)
	      fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //?
	    else
	      fp->write(stgcos->getMangledName().c_str()); //?
	    fp->write("."); //entire storage
	    fp->write(tobe->readMethodForCodeGen().c_str());
	    fp->write("();"); GCNL;
	  }

	s32 tmpbv = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indentUlamCode(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpbv, TMPBITVAL).c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpread, tobe->getTmpStorageTypeForTmpVar()).c_str());
	fp->write(");"); GCNL;

	//update the uvpass to have the casted immediate quark
	uvpass = UVPass::makePass(tmpbv, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, uvpass.getPassPos(), 0); //POS 0 is justified; will name id help?
      }
    else
      {
	//reference tobe
	s32 tmpref = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indentUlamCode(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpref, TMPBITVAL).c_str());
	fp->write("(");
	fp->write(stgcos->getMangledName().c_str());
	//t3692 undefined .GetStorage, it is a storage!

	if(m_state.isAtom(vuti))
	  {
	    //from atom/ref, for known quark ref: t3631, t3632, t3633, t3986
	    fp->write(", 0u + T::ATOM_FIRST_STATE_BIT, "); //'is' &
	    fp->write("NULL"); //look up effself t3837
	    fp->write(", UlamRef<EC>::ELEMENTAL"); //becomes elemental t3837
	    if(!m_state.isAtomRef(stguti))
	      fp->write(", uc"); //t3986
	  }
	else
	  {
	    //from quarkref to atomref; maintains everything but length (2nd arg)
	    assert(stgut->isAltRefType()); //t3697, t3834
	    fp->write(", - T::ATOM_FIRST_STATE_BIT"); //'is' t3834
	  }
	fp->write(");"); GCNL; //like, shadow lhs of as

	//update the uvpass to have the casted immediate quark ref
	uvpass = UVPass::makePass(tmpref, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, uvpass.getPassPos(), 0); //POS 0 is justified; will name id help?
      }
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastAtomAndQuark

  void NodeCast::genCodeCastDescendantTransient(File * fp, UVPass & uvpass)
  {
    //similar to genCodeCastDescendantElement, except for +25
    UTI tobeType = getCastType(); //related quark tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getPassTargetType();

    //CHANGES uvpass
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	// don't repeat genCodeToStoreInto when m_node is a function call that returns a ref
	if(!m_node->isFunctionCall())
	  {
	    UVPass ruvpass;
	    m_node->genCodeToStoreInto(fp, ruvpass); //No need to load lhs into tmp (T); symbol's in COS vector
	  }
	else //(t41053, case 1)
	  m_state.abortShouldntGetHere();
      }

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    // "downcast" might not be true; compare to be sure the transient is-related to quark
    if(m_state.isClassASubclassOf(tobeType, vuti)) //super (vuti) -> sub (tobe)
      {
	m_state.indentUlamCode(fp);
	fp->write("if(! ");
	fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
	fp->write(".");
	fp->write(m_state.getIsMangledFunctionName(tobeType)); //IsMethod
	fp->write("(&"); //one arg
	fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	fp->write("))\n");

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(BAD_CAST);"); GCNL;
	m_state.m_currentIndentLevel--;
      }

    if(!tobe->isAltRefType())
      {
	//read from pos 0, for length of quark
	s32 tmpVarVal = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //u32
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarVal, TMPREGISTER).c_str());
	fp->write(" = ");

	fp->write("UlamRef<EC>(");
	if(stgcosut->isAltRefType())
	  {
	    fp->write(stgcos->getMangledName().c_str()); //reference
	    fp->write(", ");
	  }
	fp->write("0u, "); //offset of decendent is always 0
	fp->write_decimal_unsigned(tobe->getTotalBitSize()); //len
	fp->write("u, ");

	if(!stgcosut->isAltRefType())
	  {
	    if(stgcos->isDataMember())
	      fp->write(m_state.getHiddenArgName());
	    else
	      fp->write(stgcos->getMangledName().c_str()); //storage
	    fp->write(", &");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
	  }
	else
	  {
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".GetEffectiveSelf()"); //maintains eff self
	  }

	fp->write(", UlamRef<EC>::CLASSIC");
	if(!stgcosut->isAltRefType())
	  fp->write(", uc");
	fp->write(").");
	fp->write(tobe->readMethodForCodeGen().c_str());
	fp->write("();"); GCNL;

	//update the uvpass to have the casted quark value
	uvpass = UVPass::makePass(tmpVarVal, tobe->getTmpStorageTypeForTmpVar(), tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    else
      {
	// to be ref, don't read! //t3789: case 1 "Qbase& qref = tw;"
	s32 tmpref = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indentUlamCode(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpref, TMPBITVAL).c_str());
	fp->write("(");

	if(stgcos->isDataMember())
	  fp->write(m_state.getHiddenArgName()); //ur first arg (t3967)
	else
	  fp->write(stgcos->getMangledName().c_str());

	fp->write(", "); //offset of decendent is always 0
	fp->write_decimal_unsigned(uvpass.getPassPos());
	if(!stgcosut->isAltRefType())
	  {
	    fp->write("u, &"); //transients stg at pos , state of super quark at 0 //t3789, case 1: Qbase& qref = tw;
	    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	    if(!stgcos->isDataMember()) //t3967
	      fp->write(", uc");
	  }
	else
	  {
	    fp->write("u, ");
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".GetEffectiveSelf()"); //maintains eff self
	  }
	fp->write(");"); GCNL;

	//update the uvpass to have the casted immediate quark
	uvpass = UVPass::makePass(tmpref, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, uvpass.getPassPos(), 0); //POS 0 is justified; will name id help?
      }
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastDescendantTransient

  void NodeCast::genCodeCastAncestorQuarkAsSubTransient(File * fp, UVPass & uvpass)
  {
    // quark to sub-quark covered by CastDescendantQuark
    UTI tobeType = getCastType(); //related subclass tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getPassTargetType(); //replace

    // CHANGES uvpass..and vuti, derefuti, etc.
    if(tobe->isAltRefType())
      {
	// don't repeat genCodeToStoreInto when m_node is a function call that returns a ref
	if(m_state.m_currentObjSymbolsForCodeGen.empty() && !m_node->isFunctionCall())
	  {
	    UVPass ruvpass;
	    m_node->genCodeToStoreInto(fp, ruvpass); //No need to load lhs into tmp (u32); symbol's in COS vector
	    assert(m_state.isAltRefType(ruvpass.getPassTargetType())); //t3789, t3790
	  }
	//else (t41054, case 2, and t3790)
      }
    else
      m_state.abortShouldntGetHere(); //error caught already


    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();

    assert(m_state.isAltRefType(stgcosuti));

    // "downcast" might not be true; compare to be sure the transient is-related to quark
    if(m_state.isClassASubclassOf(tobeType, vuti)) //super (vuti) -> sub (tobe)
      {
	m_state.indentUlamCode(fp);
	fp->write("if(! ");
	fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
	fp->write(".");
	fp->write(m_state.getIsMangledFunctionName(tobeType)); //UlamElement IsMethod
	fp->write("(&"); //one arg
	fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	fp->write("))\n");

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(BAD_CAST);"); GCNL;
	m_state.m_currentIndentLevel--;
      }

    //read from pos 0, for length of quark
    s32 tmpVarVal = m_state.getNextTmpVarNumber();

    if(tobe->isAltRefType())
      {
	//t3789 case 4: (fails to compile)
	//Incompatible class types TW& and Qbase& used to initialize reference 'twref'.
	m_state.indentUlamCode(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarVal, TMPBITVAL).c_str());
	fp->write("(");
	fp->write(stgcos->getMangledName().c_str()); //a ref
	fp->write(");"); GCNL;
	uvpass = UVPass::makePass(tmpVarVal, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    else
      {
	//Compile-time error to downcast from a ref to a subclass instance. 20160616.
	//t3789 case 2 (init): TW tapple = (TW) qref;
	//t3789 case 3 (assign): tapple2 = (TW) qref;
	m_state.abortShouldntGetHere(); //error caught already
      }
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastAncestorQuarkAsSubTransient

  void NodeCast::genCodeCastDescendantElement(File * fp, UVPass & uvpass)
  {
    UTI tobeType = getCastType(); //related quark tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getPassTargetType();

    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	// don't repeat genCodeToStoreInto when m_node is a function call that returns an atomref
	//CHANGES uvpass
	if(!m_node->isFunctionCall())
	  {
	    UVPass ruvpass;
	    m_node->genCodeToStoreInto(fp, ruvpass); //No need to load lhs into tmp (T); symbol's in COS vector
	  }
	//else  (t41052, case 1)
      }

    //still empty when func call returns self (t41065, case foofunc())
    //assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    Node::loadStorageAndCurrentObjectSymbols(stgcos, cos);
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    bool useSelf = (stgcos->isSelf() && (uvpass.getPassStorage() != TMPBITVAL)); //t41065

    assert(m_state.isClassASubclassOf(vuti, tobeType) || (m_state.isARefTypeOfUlamType(tobeType, vuti) == UTIC_SAME)); //not a downcast; going sub->super

    if(!tobe->isAltRefType())
      {
	//read from pos 0, for length of quark
	s32 tmpVarVal = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //u32
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarVal, TMPREGISTER).c_str());
	fp->write(" = ");

	//make a temporary reference to a super-quark in an element for reading
	fp->write("UlamRef<EC>(");
	if(stgcos->isSelf() && !useSelf)
	  {
	    fp->write("T::ATOM_FIRST_STATE_BIT + "); //elements stg at 0 , state of quark at 25
	  }
	else if(stgcosut->isAltRefType())
	  {
	    fp->write(stgcos->getMangledName().c_str()); //reference
	    fp->write(", ");
	  }
	else
	  fp->write("T::ATOM_FIRST_STATE_BIT + "); //elements stg at 0 , state of quark at 25

	fp->write("0u, "); //offset of decendent is always 0 from start of state bits

	fp->write_decimal_unsigned(tobe->getTotalBitSize()); //len
	fp->write("u, ");

	if(stgcos->isSelf() && !useSelf)
	  {
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(", &");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	  }
	else if(!stgcosut->isAltRefType())
	  {
	    fp->write(stgcos->getMangledName().c_str()); //storage
	    fp->write(", &");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
	  }
	else
	  {
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".GetEffectiveSelf()"); //maintains eff self
	  }

	fp->write(", UlamRef<EC>::ELEMENTAL");
	if(!stgcosut->isAltRefType()) //self is a reference
	  fp->write(", uc");
	else if(stgcos->isSelf() && !useSelf)
	  fp->write(", uc"); //t41065, m-case;
	//else t41065 j-case;

	fp->write(").");
	fp->write(tobe->readMethodForCodeGen().c_str());
	fp->write("();"); GCNL;

	//update the uvpass to have the casted quark value
	uvpass = UVPass::makePass(tmpVarVal, tobe->getTmpStorageTypeForTmpVar(), tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    else
      {
	// to be ref, don't read! //t3756 case 1: CastAsDescendantElement
	s32 tmpref = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indentUlamCode(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpref, TMPBITVAL).c_str());
	fp->write("(");
	if(stgcos->isSelf() && !useSelf)
	  fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //?
	else if(stgcos->isDataMember())
	  {
	    fp->write_decimal(stgcos->getPosOffset()); //like t41141, test?
	    fp->write("u");
	  }
	else
	  fp->write(stgcos->getMangledName().c_str());

	if(!stgcosut->isAltRefType() || (stgcos->isSelf() && !useSelf))
	  {
	    fp->write(", "); //offset of decendent is always 0 +25
	    fp->write_decimal_unsigned(uvpass.getPassPos()); //t3735 (was ruvapss)
	    fp->write("u + T::ATOM_FIRST_STATE_BIT, &"); //elements stg at 0 , state of quark at 25
	    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	    fp->write(", UlamRef<EC>::ELEMENTAL"); //stays elemental
	    fp->write(", uc"); //t3617
	    fp->write(");"); GCNL;
	  }
	else
	  {
	    fp->write("); //decendent element cast"); GCNL;  //3697
	  }

	//update the uvpass to have the casted immediate quark
	uvpass = UVPass::makePass(tmpref, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, uvpass.getPassPos(), 0); //POS 0 is justified; will name id help?
      }
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastDescendantElement

  void NodeCast::genCodeCastAncestorQuarkAsSubElement(File * fp, UVPass & uvpass)
  {
    // quark to sub-quark covered by CastDescendantQuark
    UTI tobeType = getCastType(); //related subclass tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getPassTargetType(); //replace

    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	// don't repeat genCodeToStoreInto when m_node is a function call that returns an atomref
	// CHANGES uvpass..and vuti, derefuti, etc.
	if(!m_node->isFunctionCall())
	  {
	    UVPass ruvpass;
	    m_node->genCodeToStoreInto(fp, ruvpass); //No need to load lhs into tmp (T); symbol's in COS vector
	    assert(m_state.isAltRefType(ruvpass.getPassTargetType()));
	  }
	//else (t41052, case 2)
      }

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();

    assert(m_state.isAltRefType(stgcosuti));

    // "downcast" might not be true; compare to be sure the element is-related to quark
    m_state.indentUlamCode(fp);
    fp->write("if(! ");
    fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
    fp->write(".");
    fp->write(m_state.getIsMangledFunctionName(tobeType)); //UlamElement IsMethod
    fp->write("(&"); //one arg
    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
    fp->write("))\n");

    m_state.m_currentIndentLevel++;
    m_state.indentUlamCode(fp);
    fp->write("FAIL(BAD_CAST);"); GCNL;
    m_state.m_currentIndentLevel--;

    //read from pos 0, for length of quark
    s32 tmpVarVal = m_state.getNextTmpVarNumber();

    if(tobe->isAltRefType()) //t3756: case 5: 'A& appleref = (A&) qref;'
      {
	m_state.indentUlamCode(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarVal, TMPBITVAL).c_str());
	fp->write("(");
	if(stgcos->isDataMember())
	  {
	    fp->write_decimal(stgcos->getPosOffset()); //like t41141, test?
	    fp->write("u");
	  }
	else
	  fp->write(stgcos->getMangledName().c_str()); //a ref
	fp->write(", 0u, ");
	fp->write(stgcos->getMangledName().c_str()); //a ref
	fp->write(".GetEffectiveSelf()"); //maintains eff self
	fp->write(");"); GCNL;
	uvpass = UVPass::makePass(tmpVarVal, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    else
      {
	//Compile-time error to downcast from a ref to a subclass instance. 20160616.
	//t3756 case 2: (init) 'A apple = (A) qref;'
	//t3756 case 3: (assign) 'apple2 = (A) qref;'
	m_state.abortShouldntGetHere(); //error caught already
      }
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastAncestorQuarkAsSubElement

  void NodeCast::genCodeCastDescendantQuark(File * fp, UVPass & uvpass)
  {
    UTI tobeType = getCastType(); //quark tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getPassTargetType(); //replace
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    s32 tmpVarNum = uvpass.getPassVarNum();

    assert(m_state.isClassASubclassOf(vuti, tobeType) || m_state.isClassASubclassOf(tobeType, vuti) || (m_state.isARefTypeOfUlamType(tobeType, vuti) == UTIC_SAME)); //vuti is subclass of tobeType (or ref tobe), or visa versa (t3757); reftypeof same ?: (t41069)

    // "downcast" might not be true; compare to be sure the quark is-related to quark
    if(m_state.isClassASubclassOf(tobeType, vuti)) //super (vuti) -> sub (tobe)
      {
	m_state.indentUlamCode(fp);
	fp->write("if(! ");
	fp->write(m_state.getTheInstanceMangledNameByIndex(tobeType).c_str());
	fp->write(".");
	fp->write(m_state.getIsMangledFunctionName(tobeType)); //IsMethod
	fp->write("(&"); //one arg
	fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	fp->write("))\n");

	m_state.m_currentIndentLevel++;
	m_state.indentUlamCode(fp);
	fp->write("FAIL(BAD_CAST);"); GCNL;
	m_state.m_currentIndentLevel--;
      }

    s32 tmpVarSuper = m_state.getNextTmpVarNumber();

    if(tobe->isAltRefType()) //t3757
      {
	// CHANGES uvpass..and vuti, derefuti, etc.
	if(m_state.m_currentObjSymbolsForCodeGen.empty() && !m_node->isFunctionCall())
	  {
	    UVPass ruvpass;
	    m_node->genCodeToStoreInto(fp, ruvpass); //No need to load lhs into tmp (T); symbol's in COS vector; its value is in uvpass
	  }
	//else (t41052, case 3)

	assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
	Symbol * stgcos = NULL;
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0]; //ref can't be a dm
	UTI stgcosuti = stgcos->getUlamTypeIdx();

	m_state.indentUlamCode(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarSuper, TMPBITVAL).c_str());
	fp->write("(");
	if(stgcos->isDataMember())
	  {
	    fp->write_decimal(stgcos->getPosOffset()); //t41141
	    fp->write("u");
	  }
	else
	  fp->write(stgcos->getMangledName().c_str()); //a ref or stg
	if(!m_state.isAltRefType(stgcosuti))
	  {
	    fp->write(", 0u, ");
	    fp->write("&"); //e.g. quark-sub to quark-super-ref (t3758)
	    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
	    fp->write(", uc");
	  }
	fp->write(");"); GCNL;
	uvpass = UVPass::makePass(tmpVarSuper, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified;
      }
    else
      {
	//e.g. a quark var here would be ok if a superclass
	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //u32
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarSuper, tobe->getTmpStorageTypeForTmpVar()).c_str());;

	fp->write(" = _ShiftOpRightInt32(");
	// right shift the bitlen of vuti - the bitlen of tobeType
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, vut->getTmpStorageTypeForTmpVar()).c_str());
	fp->write(", ");
	s32 nlen = tobe->getBitSize();
	s32 shiftlen = m_state.getBitSize(vuti) - nlen;
	fp->write_decimal(shiftlen);
	fp->write(", ");
	fp->write_decimal(nlen);
	fp->write(");"); GCNL;

	// new uvpass here..
	uvpass = UVPass::makePass(tmpVarSuper, tobe->getTmpStorageTypeForTmpVar(), tobeType, m_state.determinePackable(tobeType), m_state, 0, 0); //POS 0 rightjustified.
      }
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeCastDescendantQuark

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

    // references are same sizes so no casting needed except to change the uti;
    // NOT including (?) mixing types (e.g. quark ref to atom, or quark ref to sub class)
    // NOT including explicit casts (e.g. t41067), or Strings (t3962),
    // including unnecessary atom/ref to atom/ref (t41139)
    if((m_state.isARefTypeOfUlamType(tobeType, nodeType) == UTIC_SAME) && (!isExplicitCast() || (UlamType::compareForString(tobeType, m_state) == UTIC_SAME) || m_state.isAtom(tobeType)))
      return false; //special case of casting

    UlamType * nodeut = m_state.getUlamTypeByIndex(nodeType);
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    ULAMTYPE typEnum = tobe->getUlamTypeEnum();
    ULAMTYPE nodetypEnum = nodeut->getUlamTypeEnum();

    //requires toInt
    if(m_state.getUlamTypeByIndex(nodeType)->getUlamClassType() == UC_QUARK)
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
