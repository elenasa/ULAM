#include <stdlib.h>
#include "NodeTypeDescriptor.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeDescriptor::NodeTypeDescriptor(const Token& tokarg, UTI auti, CompilerState & state) : Node(state), m_typeTok(tokarg), m_uti(auti), m_ready(false), m_contextForPendingArgType(Nouti), m_unknownBitsizeSubtree(NULL), m_refType(ALT_NOT), m_referencedUTI(Nouti)
  {
    setNodeLocation(m_typeTok.m_locator);
  }

  // use by ALT_AS.
  NodeTypeDescriptor::NodeTypeDescriptor(const Token& tokarg, UTI auti, CompilerState & state, ALT refarg, UTI referencedUTIarg) : Node(state), m_typeTok(tokarg), m_uti(auti), m_ready(false), m_contextForPendingArgType(Nouti), m_unknownBitsizeSubtree(NULL), m_refType(refarg), m_referencedUTI(referencedUTIarg)
  {
    setNodeLocation(m_typeTok.m_locator);
  }

  //since there's no assoc symbol, we map the m_uti here (e.g. S(x,y).sizeof nodeterminalproxy)
  NodeTypeDescriptor::NodeTypeDescriptor(const NodeTypeDescriptor& ref) : Node(ref), m_typeTok(ref.m_typeTok), m_uti(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_uti,ref.getNodeLocation())), m_ready(false), m_contextForPendingArgType(Nouti), m_unknownBitsizeSubtree(NULL), m_refType(ref.m_refType), m_referencedUTI(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_referencedUTI,ref.getNodeLocation()))
  {
    if(ref.m_unknownBitsizeSubtree)
      m_unknownBitsizeSubtree = new NodeTypeBitsize(*ref.m_unknownBitsizeSubtree); //mapped UTI?
  }

  //clone class parameter for pending class argument; caller (NodeConstDef) corrects type (t41223)
  NodeTypeDescriptor::NodeTypeDescriptor(const NodeTypeDescriptor& ref, bool keepType) : Node(ref), m_typeTok(ref.m_typeTok), m_uti(ref.m_uti), m_ready(false), m_contextForPendingArgType(Nouti), m_unknownBitsizeSubtree(NULL), m_refType(ref.m_refType), m_referencedUTI(ref.m_referencedUTI)
  {
    if(ref.m_unknownBitsizeSubtree)
      m_unknownBitsizeSubtree = new NodeTypeBitsize(*ref.m_unknownBitsizeSubtree); //mapped UTI?
  }

  NodeTypeDescriptor::~NodeTypeDescriptor()
  {
    delete m_unknownBitsizeSubtree;
    m_unknownBitsizeSubtree = NULL;
  } //destructor

  Node * NodeTypeDescriptor::instantiate()
  {
    return new NodeTypeDescriptor(*this);
  }

  void NodeTypeDescriptor::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_unknownBitsizeSubtree)
      m_unknownBitsizeSubtree->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeTypeDescriptor::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_unknownBitsizeSubtree && m_unknownBitsizeSubtree->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypeDescriptor::printPostfix(File * fp)
  {
    fp->write(getName());
  }

  const char * NodeTypeDescriptor::getName()
  {
    return m_state.getTokenDataAsString(m_typeTok).c_str();
  } //getName

  u32 NodeTypeDescriptor::getTypeNameId()
  {
    bool isTypedefOrClass = (m_typeTok.m_type == TOK_TYPE_IDENTIFIER);
    ULAMTYPE bUT = m_state.getBaseTypeFromToken(m_typeTok); //could be Hzy?
    //t3343 Hzy if typedef from another class; not a 'Class', ok to continue..
    if(isTypedefOrClass && (bUT != Class))
      return m_typeTok.m_dataindex; //typedef

    UTI nuti = getNodeType();
    assert(m_state.okUTItoContinue(nuti));
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    //skip bitsize if default size
    if(nut->getBitSize() == ULAMTYPE_DEFAULTBITSIZE[bUT])
      return m_state.m_pool.getIndexForDataString(nut->getUlamTypeNameOnly());
    return m_state.m_pool.getIndexForDataString(m_state.getUlamTypeNameBriefByIndex(nuti));
  } //getTypeNameId

  const std::string NodeTypeDescriptor::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeTypeDescriptor::linkConstantExpressionBitsize(NodeTypeBitsize * ceForBitSize)
  {
    assert(!m_unknownBitsizeSubtree);
    m_unknownBitsizeSubtree = ceForBitSize; //tfr owner
  } //linkConstantExpressionBitsize

  bool NodeTypeDescriptor::isReadyType()
  {
    return m_ready;
  }

  UTI NodeTypeDescriptor::givenUTI()
  {
    return m_uti;
  }

  void NodeTypeDescriptor::resetGivenUTI(UTI guti)
  {
    m_uti = guti;  //invariant?
  }

  UTI NodeTypeDescriptor::getReferencedUTI()
  {
    return m_referencedUTI;
  }

  ALT NodeTypeDescriptor::getReferenceType()
  {
    return m_refType;
  }

  void NodeTypeDescriptor::setReferenceType(ALT refarg, UTI referencedUTI)
  {
    m_refType = refarg;
    m_referencedUTI = referencedUTI;
  }

  void NodeTypeDescriptor::setReferenceType(ALT refarg, UTI referencedUTI, UTI refUTI)
  {
    setReferenceType(refarg, referencedUTI);
    resetGivenUTI(refUTI); //new given as ref UTI Sat Jul  2 15:10:29 2016
  }

  UTI NodeTypeDescriptor::getContextForPendingArgType()
  {
    return m_contextForPendingArgType;
  }

  void NodeTypeDescriptor::setContextForPendingArgType(UTI context)
  {
    m_contextForPendingArgType = context;
  }

  UTI NodeTypeDescriptor::checkAndLabelType()
  {
    if(isReadyType())
      return getNodeType();

    UTI it = givenUTI();
    if(resolveType(it)) //ref
      {
	setNodeType(it);
	resetGivenUTI(it); //new given reset here!!! Mon Aug  1 12:02:52 2016
	m_ready = true; //set here!!!
      }
    else if(it == Hzy)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
      }
    else
      setNodeType(it);

    return getNodeType();
  } //checkAndLabelType

  bool NodeTypeDescriptor::resolveType(UTI& rtnuti)
  {
    bool rtnb = false;
    if(isReadyType())
      {
	rtnuti = getNodeType();
	return true;
      }

    // not node select, we are the leaf Type: a typedef, class or primitive scalar.
    UTI nuti = givenUTI(); //start with given.

    //if(getReferenceType() != ALT_NOT)
    if((getReferenceType() != ALT_NOT) || m_state.isAltRefType(nuti)) //t3668?
      {
	rtnb = resolveReferenceType(nuti); //may update nuti
	if(nuti == Nav)
	  {
	    rtnuti = Nav;
	    return false;
	  }
      }

    if(!m_state.isComplete(nuti))
      {
	// if Nav, use token
	UTI mappedUTI = nuti;

	// first search current block, often same as context;
	UTI cbuti = m_state.getCurrentBlock()->getNodeType();
	if(m_state.isAClass(cbuti))
	  {
	    if(m_state.mappedIncompleteUTI(cbuti, nuti, mappedUTI))
	      {
		std::ostringstream msg;
		msg << "Substituting Mapped UTI" << mappedUTI;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
		msg << " for incomplete descriptor type: '";
		msg << m_state.getUlamTypeNameByIndex(nuti).c_str();
		msg << "' UTI" << nuti << " while labeling current block: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cbuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		nuti = mappedUTI;
	      }
	  }

	//if no change, try context..
	if(mappedUTI == nuti)
	  {
	    UTI cuti = m_state.getCompileThisIdx();
	    // the symbol associated with this type, was mapped during instantiation
	    // since we're call AFTER that (not during), we can look up our
	    // new UTI and pass that on up the line of NodeType Selects, if any.
	    if(m_state.mappedIncompleteUTI(cuti, nuti, mappedUTI))
	      {
		std::ostringstream msg;
		msg << "Substituting Mapped UTI" << mappedUTI;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
		msg << " for incomplete descriptor type: '";
		msg << m_state.getUlamTypeNameByIndex(nuti).c_str();
		msg << "' UTI" << nuti << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		nuti = mappedUTI;
	      }
	  }

	if(!m_state.isComplete(nuti)) //reloads to recheck for debug message
	  {
	    std::ostringstream msg;
	    msg << "Incomplete descriptor for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str(); //t3125, t3298
	    msg << " (UTI " << nuti << ")"; //helpful for my debugging (e.g. t41209)
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //t3787
	  }
      }

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    if(etyp == Class)
      {
	rtnb = resolveClassType(nuti);
      }
    //else if((etyp == Holder) && !m_state.isThisLocalsFileScope())
    else if((etyp == Holder))
      {
	if(!m_state.isThisLocalsFileScope())
	  {
	    //non-class reference, handled in resolveReferenceType
	    if(getReferenceType() == ALT_NOT)
	      {
		// non-class, non-ref, non-localdef scope
		if(!(rtnb = m_state.statusUnknownTypeInThisClassResolver(nuti)))
		  nuti = Hzy;
	      }
	  }
	else
	  {
	    //islocalsscope t41232
	    UTI kuti = nuti; //kuti changes
	    rtnb = m_state.statusUnknownClassTypeInThisLocalsScope(m_typeTok, nuti, kuti);
	    nuti = kuti;
	  }
      }
    else
      {
	//primitive, or array typedef
	rtnb = resolvePrimitiveOrArrayType(nuti);
      }

    rtnuti = nuti;
    return rtnb;
  } //resolveType

  bool NodeTypeDescriptor::resolveReferenceType(UTI& rtnuti)
  {
    bool rtnb = false;

    UTI nuti = givenUTI();
    UTI cuti = m_state.getCompileThisIdx();

    //assert(getReferenceType() != ALT_NOT);
    assert((getReferenceType() != ALT_NOT) || (m_state.isAltRefType(nuti))); //t3668?

    //if reference is not complete, but its deref is, use its sizes to complete us.
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UTI derefuti = getReferencedUTI();

    if(!m_state.okUTItoContinue(derefuti))
      {
	if(nut->getUlamTypeEnum() == Class)
	  derefuti = nut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureClassInstanceIdx();
	else
	  derefuti = m_state.getUlamTypeAsDeref(nuti);
      }

    if(m_state.okUTItoContinue(derefuti))
      {
	if(!m_state.isComplete(derefuti))
	  {
	    // if Nav, use token
	    UTI mappedUTI = derefuti;

	    // the symbol associated with this type, was mapped during instantiation
	    // since we're call AFTER that (not during), we can look up our
	    // new UTI and pass that on up the line of NodeType Selects, if any.
	    if(m_state.mappedIncompleteUTI(cuti, derefuti, mappedUTI))
	      {
		std::ostringstream msg;
		msg << "Substituting Mapped UTI" << mappedUTI;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
		msg << " for incomplete de-ref descriptor type: ";
		msg << m_state.getUlamTypeNameByIndex(derefuti).c_str();
		msg << "' UTI" << derefuti << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		derefuti = mappedUTI;
	      }
	  } //incomplete deref

	UlamType * derefut = m_state.getUlamTypeByIndex(derefuti);
	if(m_state.isComplete(derefuti))
	  {
	    //move to before known
	    if(derefut->getReferenceType() != ALT_NOT)
	      {
		std::ostringstream msg;
		msg << "Referencing a reference type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		rtnuti = Nav;
		return false;
	      }

	    // we might have set the size of a holder ref. still a holder. darn.
	    rtnb = m_state.completeAReferenceTypeWith(nuti, derefuti);

	    ALT altd = m_state.getReferenceType(nuti);
	    setReferenceType(altd, derefuti);
	  } //complete deref
	//else deref not complete, t.f. nuti isn't changed
      } //else not ok to continue

    if(rtnb)
      rtnuti = nuti;
    return rtnb;
  } //resolveReferenceType

  bool NodeTypeDescriptor::resolveClassType(UTI& rtnuti)
  {
    bool rtnb = false;
    UTI nuti = rtnuti; //not givenUTI!!
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    assert(etyp == Class);

    ULAMCLASSTYPE nclasstype = nut->getUlamClassType();
    if(nut->isComplete())
      {
	UTI aliasuti;
	if(m_state.findaUTIAlias(nuti, aliasuti))
	  rtnuti = aliasuti; //t3921
	else
	  rtnuti = nuti;
	rtnb = true;
      } //else we're not ready!!
    else if((nclasstype == UC_UNSEEN))
      {
	if(nut->isReference())
	  {
	    UTI derefuti = m_state.getUlamTypeAsDeref(nuti);
	    if(m_state.okUTItoContinue(derefuti))
	      {
		UlamType * derefut = m_state.getUlamTypeByIndex(derefuti);
		if(m_state.correctAReferenceTypeWith(nuti, derefuti))
		  {
		    AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(nut->getUlamKeyTypeSignature(), Class, derefut->getUlamClassType(), derefut->isCustomArray());
		    assert(isReplaced);
		    nut = m_state.getUlamTypeByIndex(nuti);
		    nclasstype = nut->getUlamClassType();
		  }
	      }
	  }

	if(nclasstype == UC_UNSEEN) //still unseen?
	  {
	    UTI tduti = Nouti;
	    UTI tmpforscalaruti = Nouti;
	    bool isTypedef = m_state.getUlamTypeByTypedefName(m_typeTok.m_dataindex, tduti, tmpforscalaruti);

	    if(isTypedef)
	      {
		//unseen typedef's appear like Class basetype, until seen
		//wait until complete to re-key..want arraysize if non-scalar
		std::ostringstream msg;
		if(m_state.isComplete(tduti))
		  {
		    UlamType * tut = m_state.getUlamTypeByIndex(tduti);
		    UlamKeyTypeSignature newkey(tut->getUlamTypeNameId(), tut->getBitSize(), tut->getArraySize(), tduti, tut->getReferenceType());
		    m_state.makeUlamTypeFromHolder(newkey, tut->getUlamTypeEnum(), nuti, tut->getUlamClassType());
		    rtnuti = tduti; //reset
		    rtnb = true;
		    msg << "RESET ";
		  }
		else
		  {
		    rtnuti = Hzy;
		    msg << "Hazy ";
		  }
		msg << "Unseen Class (UTI" << nuti << ") was a typedef for: '";
		msg << m_state.getUlamTypeNameByIndex(tduti).c_str();
		msg << "' (UTI" << tduti << ") while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	    else
	      {
		bool isAnonymousClass = (nut->isHolder() || !m_state.isARootUTI(nuti));
		UTI auti = nuti;

		if(nut->isHolder())
		  {
		    if(m_state.isThisLocalsFileScope() || !m_state.statusUnknownTypeInThisClassResolver(nuti))
		      auti = Hzy;
		  }
		else if(!m_state.isARootUTI(nuti))
		  {
		    AssertBool isAlias = m_state.findaUTIAlias(nuti, auti);
		    assert(isAlias);
		  }

		std::ostringstream msg;
		msg << "UNSEEN Class and incomplete descriptor for type: '";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str() << "'";
		if(isAnonymousClass)
		  {
		    msg << " (UTI" << nuti << ")";
		    msg << " replaced with type: (UTI" << auti << ")";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    rtnuti = auti; //was Hzy
		  }
		else
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		    rtnuti = Hzy; //t3407
		  }
	      }
	  }
	//else
	// rtnuti = Hzy;
      }
    return rtnb;
  } //resolveClassType

  bool NodeTypeDescriptor::resolvePrimitiveOrArrayType(UTI& rtnuti)
  {
    bool rtnb = false;
    UTI nuti = rtnuti;
    //primitive, or array typedef
    if(!m_unknownBitsizeSubtree)
      {
	if(!m_state.okUTItoContinue(nuti))
	  {
	    //use given UTI, not the not-ok nuti here..
	    assert(m_state.okUTItoContinue(givenUTI()) || (getReferenceType() != ALT_NOT));
	    if(m_state.okUTItoContinue(givenUTI()))
	      {
		UlamType * nut = m_state.getUlamTypeByIndex(givenUTI());
		ULAMTYPE etyp = nut->getUlamTypeEnum();
		s32 arraysize = nut->getArraySize(); //NONARRAYSIZE for scalars
		//use default primitive bitsize; (assumes scalar)
		nuti = m_state.makeUlamType(m_typeTok, ULAMTYPE_DEFAULTBITSIZE[etyp], arraysize, Nouti);
		rtnb = true;
	      }
	    else
	      {
		ALT altd = getReferenceType();
		//must be a reference type; use type token to make one!
		assert((altd == ALT_REF) || (altd == ALT_CONSTREF));
		ULAMTYPE etyp = m_state.getBaseTypeFromToken(m_typeTok);
		if(etyp != Class)
		  {
		    //use default primitive bitsize; (assumes scalar)
		    nuti = m_state.makeUlamType(m_typeTok, ULAMTYPE_DEFAULTBITSIZE[etyp], NONARRAYSIZE, Nouti, altd, UC_NOTACLASS);
		    rtnb = true; //t3689, t3696, t3760, t3792. t3793
		  }
	      }
	  }
	else if(!(rtnb = m_state.isComplete(nuti)))
	  {
	    nuti = Hzy;
	  }
	//else mapped?
      }
    else
      {
	//primitive with possible unknown bit size
	rtnb = resolveTypeBitsize(nuti);
      }

    if(!m_state.okUTItoContinue(nuti))
      {
	UTI tduti = Nouti;
	UTI tmpforscalaruti = Nouti;
	bool isTypedef = m_state.getUlamTypeByTypedefName(m_typeTok.m_dataindex, tduti, tmpforscalaruti);

	if(isTypedef)
	  {
	    m_state.updateUTIAliasForced(givenUTI(), tduti); //t3898
	    nuti = tduti;
	    if(!(rtnb = m_state.isComplete(nuti)))
	      nuti = Hzy;
	  }
      }
    rtnuti = nuti;
    return rtnb;
  } //resolvePrimitiveOrArrayType

  bool NodeTypeDescriptor::resolveTypeBitsize(UTI& rtnuti)
  {
    UTI auti = rtnuti;
    UlamType * ut = m_state.getUlamTypeByIndex(auti);
    ULAMTYPE etyp = ut->getUlamTypeEnum();
    if(m_unknownBitsizeSubtree)
      {
	s32 bs = UNKNOWNSIZE;
	//primitive with possible unknown bit sizes.
	bool rtnb = m_unknownBitsizeSubtree->getTypeBitSizeInParen(bs, etyp, auti); //eval
	if(rtnb)
	  {
	    if(bs < 0)
	      {
		std::ostringstream msg;
		msg << "Type Bitsize specifier for " << UlamType::getUlamTypeEnumAsString(etyp);
		msg << " type, within (), is a negative numeric constant expression: ";
		msg << bs;
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		rtnuti = Nav;
		return false;
	      }

	    // keep m_unknownBitsizeSubtree in case of template (don't delete)
	    if(!m_state.setUTISizes(rtnuti, bs, ut->getArraySize())) //update UlamType, outputs errs
	      {
		rtnuti = Nav;
		return false;
	      }
	  }
	else
	  {
	    rtnuti = auti; //could be Hzy or Nav
	    return false;
	  }
      }

    bool rtnb = m_state.isComplete(rtnuti);  //repeat if bitsize is still unknown
    if(!rtnb)
      rtnuti = Hzy;
    return rtnb;
  } //resolveTypeBitsize

  void NodeTypeDescriptor::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if(m_unknownBitsizeSubtree)
      m_unknownBitsizeSubtree->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  bool NodeTypeDescriptor::assignClassArgValueInStubCopy()
  {
    return true;
  }

  EvalStatus NodeTypeDescriptor::eval()
  {
    m_state.abortShouldntGetHere();  //not in parse tree; part of Node's type
    return NORMAL;
  } //eval

} //end MFM
