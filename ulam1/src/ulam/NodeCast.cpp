#include <stdio.h>
#include "NodeCast.h"
#include "CompilerState.h"

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
  } //updateLineage

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
  } //checkAbstractInstanceErrors

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
    return m_node->getStoreIntoAble();
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

  bool NodeCast::isExplicitReferenceCast()
  {
    return isExplicitCast() && m_state.isReference(getCastType());
  }

  FORECAST NodeCast::safeToCastTo(UTI newType)
  {
    //possible user error, deal with it.
    //assert(UlamType::compare(newType,getNodeType(), m_state) == UTIC_SAME);
    //ulamtype checks for complete, non array, and type specific rules
    return m_state.getUlamTypeByIndex(newType)->safeCast(getCastType());
  } //safeToCastTo

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
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	setNodeType(Hzy);
	m_state.setGoAgain(); //compiler counts
	return Hzy; //short-circuit
      }

    if(nodeType == Nouti)
      assert(0);

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
	    msg << " (UTI" << tobeType << ")";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    hazinessFound++; //goAgain set by nodetypedesc
	  }
      }

    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    UlamType * nut = m_state.getUlamTypeByIndex(nodeType);
    if(!tobe->isComplete())
      {
	std::ostringstream msg;
	msg << "Cannot cast to incomplete type: " ;
	msg << tobe->getUlamTypeNameBrief().c_str();
	msg << " (UTI" << tobeType << ")";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	hazinessFound++;
      }
    else if(tobeType == Nav)
      {
	std::ostringstream msg;
	msg << "Cannot cast " << nut->getUlamTypeNameBrief().c_str();
	msg << " to not-a-valid type";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	errorsFound++;
      }
    else if((tobe->getUlamClass() != UC_NOTACLASS) && (nut->getUlamClass() == UC_NOTACLASS))
      {
	if(!m_state.isAtom(nodeType))
	  {
	    std::ostringstream msg;
	    msg << "Cannot cast ";
	    msg << nut->getUlamTypeNameBrief().c_str(); //non-atom
	    msg << " to " << tobe->getUlamTypeNameBrief().c_str(); //to a class
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    errorsFound++;
	  }
	else if(tobe->getUlamClass() == UC_QUARK)
	  {
#if 0
	    //it becomes an immediate, unless its a ref (t3631).
	    std::ostringstream msg;
	    msg << "Cannot cast an atom to quark "; //an atom
	    msg << tobe->getUlamTypeNameBrief().c_str(); //to quark, unless an ancestor
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    errorsFound++;
#endif
	  }
      }
    else if(isExplicitCast())
      {
	FORECAST scr = tobe->explicitlyCastable(nodeType);
	if(scr != CAST_CLEAR)
	  {
	    std::ostringstream msg;
	    msg << "Cannot explicitly cast ";
	    msg << nut->getUlamTypeNameBrief().c_str();
	    msg << " to type: " << tobe->getUlamTypeNameBrief().c_str();
	    if(tobe->getUlamTypeEnum() == Bool)
	      msg << "; Consider using a comparison operator";
	    else if(m_state.isAtom(tobeType) && (nut->getUlamClass() == UC_QUARK))
	      msg << "; Consider using a reference (or self) with .storageof";
	    if(scr == CAST_HAZY)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		hazinessFound++;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;
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
	ULAMCLASSTYPE nodeClass = nut->getUlamClass();
	ULAMCLASSTYPE tobeClass = tobe->getUlamClass();
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
	    //else if(!Node::makeCastingNode(m_node, tobeType, m_node, isExplicitCast()))
	    //	      errorsFound++; //and goagain set
	    else
	      {
		// a function call is not a valid lhs !!!
		// 'node' is a function call that returns a quark (it's not storeintoable);
		// build a toIntHelper function that takes the return value of 'node'
		// as its arg and returns toInt (NodeFunctionCall *)
		if(m_node->isFunctionCall())
		    m_node = (Node *) Node::buildCastingFunctionCallNode((Node *) m_node, tobeType);
		else
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

    if(tobe->isReference())
      Node::setStoreIntoAble(TBOOL_TRUE);

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

    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(nodeType);
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

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
    assert(!m_state.isPtr(vuti));
    if(m_state.isAtom(nodeType) && m_state.isAtom(tobeType) && (m_state.getUlamTypeByIndex(vuti)->getUlamTypeEnum() == Class))
      {
	std::ostringstream msg;
	msg << "Cast question: Do not wipe out actual type for atom during eval! Value type ";
	msg << m_state.getUlamTypeNameBriefByIndex(uv.getUlamValueTypeIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	evalNodeEpilog();
	return UNEVALUABLE;
      }
    if((m_state.isAtom(tobeType)) && (m_state.getUlamTypeByIndex(vuti)->getUlamClass() == UC_QUARK))
      {
	assert(m_state.isReference(vuti)); //an immediate non-ref quark should be an error
	std::ostringstream msg;
	msg << "Cast question: actual type for quark ref during eval! Value type ";
	msg << m_state.getUlamTypeNameBriefByIndex(uv.getUlamValueTypeIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	evalNodeEpilog();
	return UNEVALUABLE;
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
	    msg << m_state.getUlamTypeNameBriefByIndex(uv.getUlamValueTypeIdx()).c_str();
	    msg << " failed to be cast as ";
	    msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    evalNodeEpilog();
	    return UNEVALUABLE;
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

    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    TBOOL stor = m_node->getStoreIntoAble();
    if(stor == TBOOL_FALSE)
      return ERROR;
    else if(stor == TBOOL_HAZY)
      return NOTREADY;

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_node->evalToStoreInto();
    if(evs != NORMAL)
      {
    	evalNodeEpilog();
    	return evs;
      }

    //then what? (see NodeMemberSelect)
    UlamValue ruvPtr = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1);

    if(m_state.isARefTypeOfUlamType(nodeType, tobeType))
      {
	ruvPtr.setPtrTargetType(tobeType);
      }
    // fix absolute slot, here or at func call?
    Node::assignReturnValuePtrToStack(ruvPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeCast::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    assert(0); // n/a
    return UlamValue();
  }

  void NodeCast::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    if(m_node)
      m_node->calcMaxDepth(depth, maxdepth, base); //funccall?
    return; //work done by NodeStatements and NodeBlock
  }

  void NodeCast::genCode(File * fp, UlamValue& uvpass)
  {
    m_node->genCode(fp, uvpass); //might be unused (e.g. classes, atoms)
    if(needsACast()) //(bypasses references)
      {
	genCodeReadIntoATmpVar(fp, uvpass); // cast.
      }
    else if(m_state.isReference(getCastType())) //to ref type
      genCodeCastAsReference(fp, uvpass); //minimal casting
    else if(m_state.isReference(m_node->getNodeType())) //from ref type
      genCodeCastFromAReference(fp, uvpass);
  } //genCode

 void NodeCast::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    m_node->genCodeToStoreInto(fp, uvpass);
    if(needsACast())
      {
	m_node->genCodeReadIntoATmpVar(fp, uvpass);
	genCodeReadIntoATmpVar(fp, uvpass); // cast.
      }
    else if(m_state.isReference(getCastType())) //to ref type
      genCodeToStoreIntoCastAsReference(fp, uvpass); //noop
    else if(m_state.isReference(m_node->getNodeType())) //from ref type
      genCodeToStoreIntoCastFromAReference(fp, uvpass); //noop
  } //genCodeToStoreInto

  void NodeCast::genCodeReadIntoATmpVar(File * fp, UlamValue& uvpass)
  {
    // e.g. called by NodeFunctionCall on a NodeTerminal..
    if(!needsACast())
      {
	return m_node->genCodeReadIntoATmpVar(fp, uvpass);
      }

    UTI tobeType = getCastType(); //tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    bool isTerminal = false;
    s32 tmpVarNum = 0;
    UTI vuti = uvpass.getUlamValueTypeIdx();
    if(m_state.isPtr(vuti))
      {
	tmpVarNum = uvpass.getPtrSlotIndex();
	vuti = uvpass.getPtrTargetType(); //replace
	assert(m_state.okUTItoContinue(vuti));
      }
    else
      isTerminal = true; // an immediate terminal value

   if(tobeType == vuti)
     return; //nothing to do!

   UlamType * vut = m_state.getUlamTypeByIndex(vuti); //after vuti replacement
   //if(m_state.isAtom(tobeType) || m_state.isAtom(vuti) || (vclasstype == UC_ELEMENT) || (vclasstype == UC_QUARK))
   if(!tobe->isPrimitiveType() || !vut->isPrimitiveType())
     {
       return genCodeReadNonPrimitiveIntoATmpVar(fp, uvpass);
     }

   //Primitive types:
   s32 tmpVarCastNum = m_state.getNextTmpVarNumber();

   m_state.indent(fp);
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
       s32 len = m_state.getBitSize(vuti);
       assert(len != UNKNOWNSIZE);
       if(len <= MAXBITSPERINT)
	 {
	   u32 data = uvpass.getImmediateData(m_state);
	   char dstr[40];
	   vut->getDataAsString(data, dstr, 'z');
	   fp->write(dstr);
	 }
       else if(len <= MAXBITSPERLONG)
	 {
	   u64 data = uvpass.getImmediateDataLong(m_state);
	   char dstr[70];
	   vut->getDataLongAsString(data, dstr, 'z');
	   fp->write(dstr);
	 }
       else
	 assert(0);
     }
   else
     {
       fp->write(m_state.getTmpVarAsString(tobeType, tmpVarNum, uvpass.getPtrStorage()).c_str());
     }

   fp->write(", ");

   assert(!(m_state.isAtom(tobeType) || m_state.isAtom(vuti)));
   //LENGTH of node being casted (Uh_AP_mi::LENGTH ?)
   //fp->write(m_state.getBitVectorLengthAsStringForCodeGen(nodetype).c_str());
   fp->write_decimal(vut->getTotalBitSize()); //src length

   fp->write(", ");
   fp->write_decimal(tobe->getTotalBitSize()); //tobe length

   fp->write(")");
   fp->write(";\n");

   //PROBLEM is that funccall checks for 0 nameid to use the tmp var!
   // but then if we don't pass it along Node::genMemberNameForMethod fails..
   if(isTerminal)
     uvpass = UlamValue::makePtr(tmpVarCastNum, TMPREGISTER, tobeType, m_state.determinePackable(tobeType), m_state, 0); //POS 0 rightjustified.
   else
     uvpass = UlamValue::makePtr(tmpVarCastNum, tobe->getTmpStorageTypeForTmpVar(), tobeType, m_state.determinePackable(tobeType), m_state, 0, uvpass.getPtrNameId()); //POS 0 rightjustified; pass along name id
  } //genCodeReadIntoTmp

  //handle element-atom and atom-element casting differently:
  // handle element->quark, atom->quark, not quark->atom
  // handle quark->quark for casting to ancestor
  // handle quark->element for casting from ancestor to sub
  void NodeCast::genCodeReadNonPrimitiveIntoATmpVar(File * fp, UlamValue &uvpass)
  {
    UTI tobeType = getCastType(); //tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    ULAMCLASSTYPE tclasstype = tobe->getUlamClass();

    UTI vuti = uvpass.getPtrTargetType(); //replace
    UlamType * vut = m_state.getUlamTypeByIndex(vuti); //after vuti replacement
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    if((tclasstype == UC_QUARK) && (vclasstype == UC_QUARK))
      return genCodeCastDecendentQuark(fp, uvpass);

    if((tclasstype == UC_QUARK) && (vclasstype == UC_ELEMENT))
      return genCodeCastDecendentElement(fp, uvpass);

    // c&l insures quark is a ref.
    if((tclasstype == UC_ELEMENT) && (vclasstype == UC_QUARK))
      return genCodeCastAncestorQuarkAsSubElement(fp, uvpass);

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
      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      return;
    }
} //genCodeReadNonPrimitiveIntoATmpVar

  void NodeCast::genCodeWriteFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    assert(0);
    genCodeWriteFromATmpVar(fp, luvpass, ruvpass);
  }

  void NodeCast::genCodeCastAtomAndElement(File * fp, UlamValue & uvpass)
  {
    UTI tobeType = getCastType();
    UTI vuti = uvpass.getUlamValueTypeIdx();
    s32 tmpVarNum = 0;

    if(m_state.isPtr(vuti))
      {
	tmpVarNum = uvpass.getPtrSlotIndex();
	vuti = uvpass.getPtrTargetType();  //replace
      }
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    // "downcast" might not be true; compare to be sure the atom is an element "Foo"
    if(m_state.isAtom(vuti))
      {
	m_state.indent(fp);
	fp->write("if(!");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(tobeType).c_str());
	fp->write(".");
	fp->write(m_state.getIsMangledFunctionName(tobeType));
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, uvpass.getPtrStorage()).c_str());
	if(uvpass.getPtrStorage() == TMPBITVAL)
	  {
	    fp->write(".");
	    fp->write(vut->readMethodForCodeGen().c_str());
	    fp->write("()");
	  }
	fp->write("))\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);
	fp->write("FAIL(BAD_CAST);\n");
	m_state.m_currentIndentLevel--;
      }

    //don't read like ref's do!
    //update the uvpass to have the casted type
    uvpass = UlamValue::makePtr(tmpVarNum, uvpass.getPtrStorage(), tobeType, m_state.determinePackable(tobeType), m_state, 0, uvpass.getPtrNameId()); //POS 0 rightjustified; pass along name id
    return;
  } //genCodeCastAtomAndElement

  void NodeCast::genCodeCastAtomAndQuark(File * fp, UlamValue & uvpass)
  {
    // quark tobe, only if ancestor
    // also allow quark refs -> atom cast, if ancestor of atom type.
    // e.g. assignments (e.g. t3697, error/t3691).

    UTI tobeType = getCastType();
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getUlamValueTypeIdx();
    if(m_state.isPtr(vuti))
      vuti = uvpass.getPtrTargetType(); //replace
    assert(m_state.okUTItoContinue(vuti));

    s32 tmpVarNum = uvpass.getPtrSlotIndex();

    //when this is a custom array, the symbol is the "ew" for example,
    //not the atom (e.g. ew[idx]) that has no symbol
    m_node->genCodeToStoreInto(fp, uvpass); //No need to load lhs into tmp (T); symbol's in COS vector

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    s32 tmpVarIs = m_state.getNextTmpVarNumber();
    // "downcast" might not be true; compare to be sure the atom is-a quark "Foo"
    // by inheritance (see t3631)
    if(m_state.isAtom(vuti))
      {
	s32 tmpVarType = m_state.getNextTmpVarNumber();
	m_state.indent(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType).c_str());;
	fp->write(" = ");

	if(Node::isCurrentObjectALocalVariableOrArgument())
	  {
	    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, uvpass.getPtrStorage()).c_str()); //need atom type
	    fp->write(".");
	  }
	else
	  {
	    //e.g. carray (e.g. error/t3508), a data member
	    fp->write(stgcos->getMangledName().c_str()); //assumes only one!!!
	    if(!stgcos->isSelf())
	      {
		fp->write("."); //no read for self
		fp->write("ReadAtom");
		fp->write("().");
	      }
	  }
	fp->write("GetType();\n");

	m_state.indent(fp);
	fp->write("const bool ");
	fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs).c_str());;
	fp->write(" = ((");

	//when SUBATOMIC quark, uses first state bit position (0)
	// immediate quark's storage cast as a quark? Terrible idea!!
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType).c_str());;
	fp->write(" == T::ATOM_UNDEFINED_TYPE) ? false : "); //subatomic type

	//internal method, takes uc, u32 and const char*, returns true
	// if same or ancester
	fp->write(m_state.getIsMangledFunctionName(vuti));
	fp->write("(");
	fp->write("uc, ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType).c_str());;
	fp->write(", \"");
	if(tobe->isReference())
	  {
	    UTI dereftobe = m_state.getUlamTypeAsDeref(tobeType);
	    fp->write(m_state.getUlamTypeByIndex(dereftobe)->getUlamTypeMangledName().c_str());
	  }
	else
	  fp->write(tobe->getUlamTypeMangledName().c_str());
	fp->write("\"));\n");
      }
    else
      {
	//From a quark, cast to atom..
	//e.g. a quark here would fail, if not a superclass && ref
	UlamType* vut = m_state.getUlamTypeByIndex(vuti);
	assert(vut->isReference());

	//insure the qref has a (MFM) type that's not UNDEFINED
	// hopefully, uvpass is TMPBITVAL
	m_state.indent(fp);
	fp->write("const bool ");
	fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs).c_str());;
	fp->write(" = (");
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".");
	fp->write("GetType()");
	fp->write(" != T::ATOM_UNDEFINED_TYPE);\n"); //subatomic type
      }

    m_state.indent(fp);
    fp->write("if(!");
    fp->write(m_state.getTmpVarAsString(Bool, tmpVarIs).c_str());
    fp->write(")\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("FAIL(BAD_CAST);\n\n");
    m_state.m_currentIndentLevel--;


    UTI stguti = stgcos->getUlamTypeIdx();
    UlamType * stgut = m_state.getUlamTypeByIndex(stguti);
    bool isCustomArray = m_state.isClassACustomArray(stguti);
    if(isCustomArray)
      {
	stguti = m_state.getAClassCustomArrayType(stguti);
    	stgut = m_state.getUlamTypeByIndex(stguti);

	std::ostringstream msg;
	msg << "Cannot explicitly cast custom array type ";
	msg << stgut->getUlamTypeNameBrief().c_str();
	msg << " to type: " << tobe->getUlamTypeNameBrief().c_str();
	msg << "; Consider using a temporary variable";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    // might be ancestor quark
    // can't let Node::genCodeReadIntoTmpVar do this for us (we need a ref!):
    assert(m_state.m_currentObjSymbolsForCodeGen.size() == 1);

    if(!tobe->isReference())
      {
	// uses stgcos since there's no m_varSymbol in this situation.
	// don't forget the read!
	s32 tmpread = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indent(fp);
	fp->write("const ");
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //u32 or T
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpread, tobe->getTmpStorageTypeForTmpVar()).c_str());

	if(m_state.isAtom(vuti))
	  {
	    fp->write(" = UlamRef<EC>(");
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(", 0u, ");
	    fp->write_decimal_unsigned(tobe->getTotalBitSize());
	    fp->write("u, &"); //'is'
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(tobeType).c_str());
	    fp->write(").");
	  }
	else
	  {
	    fp->write(" = ");
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write("."); //entire storage
	  }

	fp->write(tobe->readMethodForCodeGen().c_str());
	fp->write("();\n");

	s32 tmpbv = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indent(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpbv, TMPBITVAL).c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpread, tobe->getTmpStorageTypeForTmpVar()).c_str());

	if(!m_state.isAtom(vuti))
	  fp->write(", uc"); //to atom

	fp->write(");\n");

	//update the uvpass to have the casted immediate quark
	uvpass = UlamValue::makePtr(tmpbv, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, uvpass.getPtrPos()); //POS 0 is justified; will name id help?
      }
    else
      {
	//reference tobe
	s32 tmpref = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indent(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpref, TMPBITVAL).c_str());
	fp->write("(");
	fp->write(stgcos->getMangledName().c_str());
	if(!stgcos->isSelf())
	  fp->write(".GetStorage()");

	if(m_state.isAtom(vuti))
	  {
	    //from atom, for known quark:
	    fp->write(", 0u, &"); //'is'
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(tobeType).c_str());
	  }
	else
	    fp->write(", uc");

	fp->write(");\n"); //like, shadow lhs of as

	//update the uvpass to have the casted immediate quark ref
	uvpass = UlamValue::makePtr(tmpref, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, uvpass.getPtrPos()); //POS 0 is justified; will name id help?
      }
    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs
  } //genCodeCastAtomAndQuark

  void NodeCast::genCodeCastDecendentElement(File * fp, UlamValue & uvpass)
  {
    UTI tobeType = getCastType(); //related quark tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getUlamValueTypeIdx();
    if(m_state.isPtr(vuti))
      vuti = uvpass.getPtrTargetType(); //replace

    //CHANGES uvpass
    m_node->genCodeToStoreInto(fp, uvpass); //No need to load lhs into tmp (T); symbol's in COS vector

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    // "downcast" might not be true; compare to be sure the element is-related to quark "Foo"
    m_state.indent(fp);
    fp->write("if(! ");
    fp->write(m_state.getEffectiveSelfMangledNameByIndex(vuti).c_str());
    fp->write(".");
    fp->write(m_state.getIsMangledFunctionName(vuti)); //UlamElement IsMethod
    fp->write("("); //one arg
    fp->write("\"");
    if(tobe->isReference())
      {
	UTI dereftobe = m_state.getUlamTypeAsDeref(tobeType);
	fp->write(m_state.getUlamTypeByIndex(dereftobe)->getUlamTypeMangledName().c_str());
      }
    else
      fp->write(tobe->getUlamTypeMangledName().c_str());
    fp->write("\"))\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("FAIL(BAD_CAST);\n");
    m_state.m_currentIndentLevel--;

    if(!tobe->isReference())
      {
	//read from pos 0, for length of quark
	s32 tmpVarVal = m_state.getNextTmpVarNumber();
	m_state.indent(fp);
	fp->write("const ");
	fp->write(tobe->getTmpStorageTypeAsString().c_str()); //u32
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpVarVal).c_str());
	fp->write(" = ");

	fp->write("UlamRef<EC>(");
	fp->write(stgcos->getMangledName().c_str());

	fp->write(", 0u, "); //offset of decendent is always 0
	fp->write_decimal_unsigned(tobe->getTotalBitSize()); //len
	fp->write("u, &");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(tobeType).c_str());
	fp->write(").");
	fp->write(tobe->readMethodForCodeGen().c_str());
	fp->write("();\n");

	//update the uvpass to have the casted quark value
	uvpass = UlamValue::makePtr(tmpVarVal, tobe->getTmpStorageTypeForTmpVar(), tobeType, m_state.determinePackable(tobeType), m_state, 0); //POS 0 rightjustified;

      }
    else
      {
	// to be ref, don't read!
	s32 tmpref = m_state.getNextTmpVarNumber(); //tmp since no variable name
	m_state.indent(fp);
	fp->write(tobe->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(tobeType, tmpref, TMPBITVAL).c_str());
	fp->write("(");
	fp->write(stgcos->getMangledName().c_str());
	if(!stgcos->isSelf())
	  fp->write(".GetStorage()");
	fp->write(", 0u, &"); //offset of decendent is always 0
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(vuti).c_str());
	fp->write(");\n");

	//update the uvpass to have the casted immediate quark
	uvpass = UlamValue::makePtr(tmpref, TMPBITVAL, tobeType, m_state.determinePackable(tobeType), m_state, uvpass.getPtrPos()); //POS 0 is justified; will name id help?
      }
    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs
  } //genCodeCastDecendentElement

  void NodeCast::genCodeCastAncestorQuarkAsSubElement(File * fp, UlamValue & uvpass)
  {
    // quark to sub-quark covered by CastDecendentQuark
    UTI tobeType = getCastType(); //related subclass tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getUlamValueTypeIdx();
    if(m_state.isPtr(vuti))
      vuti = uvpass.getPtrTargetType(); //replace

    assert(m_state.isReference(vuti)); //important!

    UTI derefvuti = m_state.getUlamTypeAsDeref(vuti);
    UlamType * derefvut = m_state.getUlamTypeByIndex(derefvuti);

    // CHANGES uvpass..and vuti, derefuti, etc.
    m_node->genCodeToStoreInto(fp, uvpass); //No need to load lhs into tmp (T); symbol's in COS vector

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    // "downcast" might not be true; compare to be sure the element is-related to quark "Foo"
    m_state.indent(fp);
    fp->write("if(! ");
    fp->write(m_state.getEffectiveSelfMangledNameByIndex(tobeType).c_str());
    fp->write(".");
    fp->write(m_state.getIsMangledFunctionName(tobeType)); //UlamElement IsMethod
    fp->write("("); //one arg
    fp->write("\"");
    fp->write(derefvut->getUlamTypeMangledName().c_str()); //related to quark
    fp->write("\"))\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("FAIL(BAD_CAST);\n");
    m_state.m_currentIndentLevel--;

    //read from pos 0, for length of quark
    s32 tmpVarVal = m_state.getNextTmpVarNumber();
    m_state.indent(fp);
    fp->write("const ");
    fp->write(tobe->getTmpStorageTypeAsString().c_str()); //T
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(tobeType, tmpVarVal, tobe->getTmpStorageTypeForTmpVar()).c_str());
    fp->write(" = ");

    fp->write(stgcos->getMangledName().c_str());
    fp->write(".");
    fp->write(tobe->readMethodForCodeGen().c_str()); //ReadAtom for entire quark stg
    fp->write("();\n");

    //update the uvpass to have the casted quark storage, including its subclass
    uvpass = UlamValue::makePtr(tmpVarVal, tobe->getTmpStorageTypeForTmpVar(), tobeType, m_state.determinePackable(tobeType), m_state, 0); //POS 0 rightjustified;

    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs
  } //genCodeCastAncestorQuarkAsSubElement

  void NodeCast::genCodeCastDecendentQuark(File * fp, UlamValue & uvpass)
  {
    UTI tobeType = getCastType(); //quark tobe
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);

    UTI vuti = uvpass.getUlamValueTypeIdx();
    if(m_state.isPtr(vuti))
      vuti = uvpass.getPtrTargetType(); //replace
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    s32 tmpVarNum = uvpass.getPtrSlotIndex();

    // CHANGES uvpass..and vuti, derefuti, etc.
    m_node->genCodeToStoreInto(fp, uvpass); //No need to load lhs into tmp (T); symbol's in COS vector; its value is in uvpass

    // more general approach, even when "cos" (back()) is a data member
    //    of "stgcos" ([0]), (e.g. an element);
    assert(m_state.isClassASubclassOf(m_state.getUlamTypeAsDeref(vuti), m_state.getUlamTypeAsDeref(tobeType))); //vuti is subclass of tobeType (or ref tobe)

    s32 tmpVarSuper = m_state.getNextTmpVarNumber();

    //e.g. a quark var here would be ok if a superclass
    m_state.indent(fp);
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
    fp->write(");\n");

    // new uvpass here..
    uvpass = UlamValue::makePtr(tmpVarSuper, tobe->getTmpStorageTypeForTmpVar(), tobeType, m_state.determinePackable(tobeType), m_state, 0); //POS 0 rightjustified.
    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs
  } //genCodeCastDecendentQuark

  //SAME TYPES, to a reference, from either a reference or not
  void NodeCast::genCodeCastAsReference(File * fp, UlamValue & uvpass)
  {
    assert(m_state.isPtr(uvpass.getUlamValueTypeIdx()));
    UTI fromuti = uvpass.getPtrTargetType();
    UTI tobeType = getCastType();
    if((m_state.isAtomRef(tobeType) && !m_state.isReference(fromuti)))
      {
	return; //no casting, need to know for writing
      }
    uvpass.setPtrTargetType(tobeType); //minimal casting
  } //genCodeCastAsReference

  //SAME TYPES, from a reference, to a non-reference
  void NodeCast::genCodeCastFromAReference(File * fp, UlamValue & uvpass)
  {
    assert(m_state.isPtr(uvpass.getUlamValueTypeIdx()));
    UTI tobeType = getCastType();
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    assert(!tobe->isReference());

    uvpass.setPtrTargetType(tobeType); //minimal casting; including atomref to atom(non-ref)
    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of lhs
  } //genCodeCastFromAReference

  void NodeCast::genCodeToStoreIntoCastAsReference(File * fp, UlamValue & uvpass)
  {
    return;
  } //genCodeToStoreIntoCastAsReference

  void NodeCast::genCodeToStoreIntoCastFromAReference(File * fp, UlamValue & uvpass)
  {
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
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return false;
      }

    if(uticr == UTIC_SAME)
      return false; //short-circuit if same exact type

    // references are same sizes so no casting needed except to change the uti;
    // NOT including (?) mixing types (e.g. quark ref to atom, or quark ref to sub class)
    if((m_state.isARefTypeOfUlamType(tobeType, nodeType) == UTIC_SAME))
      return false; //special case of casting

    UlamType * nodeut = m_state.getUlamTypeByIndex(nodeType);
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    ULAMTYPE typEnum = tobe->getUlamTypeEnum();
    ULAMTYPE nodetypEnum = nodeut->getUlamTypeEnum();

    //requires toInt
    if(m_state.getUlamTypeByIndex(nodeType)->getUlamClass() == UC_QUARK)
      {
	if(tobe->isNumericType())
	  {
	    assert(m_state.quarkHasAToIntMethod(nodeType)); //checked by c&l
	    return (tobeType == Int); //no need to cast
	  }
	return true;
      }
    // consider user requested first, then size independent;
    // even constant may need casting (e.g. narrowing for saturation)
    // Bool constants require casts to generate "full" true UlamValue (>1-bit).
    return( isExplicitCast() || (typEnum != nodetypEnum)  || (m_state.getBitSize(tobeType) != m_state.getBitSize(nodeType)) || ( (nodetypEnum == Bool) && m_node->isAConstant() && (m_state.getBitSize(tobeType)>1)));
  } //needsACast

} //end MFM
