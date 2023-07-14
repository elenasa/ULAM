#include <stdlib.h>
#include "NodeTypeDescriptor.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeDescriptor::NodeTypeDescriptor(const Token& tokarg, UTI auti, CompilerState & state) : Node(state), m_typeTok(tokarg), m_uti(auti), m_ready(false), m_unknownBitsizeSubtree(NULL), m_refType(ALT_NOT), m_referencedUTI(Nouti)
  {
    setNodeLocation(m_typeTok.m_locator);
  }

  // use by ALT_AS.
  NodeTypeDescriptor::NodeTypeDescriptor(const Token& tokarg, UTI auti, CompilerState & state, ALT refarg, UTI referencedUTIarg) : Node(state), m_typeTok(tokarg), m_uti(auti), m_ready(false), m_unknownBitsizeSubtree(NULL), m_refType(refarg), m_referencedUTI(referencedUTIarg)
  {
    setNodeLocation(m_typeTok.m_locator);
  }

  //since there's no assoc symbol, we map the m_uti here (e.g. S(x,y).sizeof nodeterminalproxy)
  NodeTypeDescriptor::NodeTypeDescriptor(const NodeTypeDescriptor& ref) : Node(ref), m_typeTok(ref.m_typeTok), m_uti(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_uti,ref.getNodeLocation())), m_ready(false), m_unknownBitsizeSubtree(NULL), m_refType(ref.m_refType), m_referencedUTI(Hzy /*m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_referencedUTI,ref.getNodeLocation())*/)
  {
    if(m_state.isHolder(m_uti))
      {
	m_state.addUnknownTypeTokenToAClassResolver(m_state.getCompileThisIdx(), m_typeTok, m_uti);  //t3765
      }

    if(ref.m_unknownBitsizeSubtree)
      {
	m_unknownBitsizeSubtree = new NodeTypeBitsize(*ref.m_unknownBitsizeSubtree); //mapped UTI?
      }
  }

  //clone class parameter for pending class argument; caller (NodeConstDef) corrects type (t41223)
  NodeTypeDescriptor::NodeTypeDescriptor(const NodeTypeDescriptor& ref, bool keepType) : Node(ref), m_typeTok(ref.m_typeTok), m_uti(ref.m_uti), m_ready(false), m_unknownBitsizeSubtree(NULL), m_refType(ref.m_refType), m_referencedUTI(ref.m_referencedUTI)
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
  }

  u32 NodeTypeDescriptor::getTypeNameId()
  {
    return m_state.getTokenDataAsStringId(m_typeTok); //e.g. t3924
  }

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

  UTI NodeTypeDescriptor::resetGivenUTI(UTI guti)
  {
    // use the root
    UTI galias = guti;
    m_state.findRootUTIAlias(guti, galias); //t3373
    if(!m_state.isComplete(galias) && m_state.isComplete(guti))
      {
	galias = guti; //t3666
      }
    else if(m_state.isHolder(galias) && !m_state.isHolder(guti))
      {
	galias = guti; //t3375, dont lose progress
      }

    if(m_state.isReference(galias))
      {
	m_uti = galias; //t3223
      }
    else if(getReferenceType() != ALT_NOT)
      {
	//m_uti = Hzy; //t3810, t41298, t41299
	//noop. don't replace givenUTI with a non-ref type galias (ulamexports:DebugUtils)
      }
    else if(m_uti != galias)
      {
	UTI aliasuti = m_uti;
	if(!m_state.findRootUTIAlias(m_uti, aliasuti))
	  {
	    if(m_state.isHolder(m_uti)) //not is stub (t41213),t41409 holder, t3375 reverse?
	      m_state.cleanupExistingHolder(m_uti, galias); //updates alias too
	  }
	else if(m_state.isClassAStub(aliasuti) && !m_state.isClassAStub(galias))
	  {
	    //t41469 (not complete but not a stub either); and, needs
	    //aliasuti "aliased" to galias, ow aliasuti remains unresolved.
	    assert(m_state.getUlamTypeNameIdByIndex(aliasuti) == m_state.getUlamTypeNameIdByIndex(galias)); //sanity check
	    m_state.updateUTIAliasForced(aliasuti, galias);
	  }
	else
	  {
	    assert((aliasuti == guti) || (aliasuti == galias) || (guti == galias) || (m_state.lookupUTIAlias(aliasuti)==galias) || m_state.isAPrimitiveType(m_uti)); // || m_state.isClassAStub(m_uti)); //t3384, t3373, t41438 (bitsizes differ), t3988 (typedef stub), ish 20230116 guti=galias
	  }
	m_uti = galias;
      }
    //else noop
    return m_uti;
  } //resetGivenUTI

  UTI NodeTypeDescriptor::getScalarType()
  {
    //An array typedef does not have NodeTypeDescriptorArray in use (t3847);
    //A class may use emptylist to initialize, disguises as an array to NodeVarDecl (t41201);
    UTI guti = givenUTI();
    if(m_state.isHolder(guti))
      return guti; //t3798
    assert(!m_state.isScalar(guti) || m_state.isAClass(guti)); //t3847, t41201
    if(m_state.isComplete(guti))
      return m_state.getUlamTypeAsScalar(guti);
    return Hzy; //ow assumes size of initialized list, wrong when arraysize 0 (t3847).
  }

  bool NodeTypeDescriptor::isEmptyArraysizeDecl()
  {
    return true;
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

  UTI NodeTypeDescriptor::checkAndLabelType(Node * thisparentnode)
  {
    if(isReadyType())
      return getNodeType();

    UTI it = givenUTI();
    if(resolveType(it)) //ref
      {
	it = resetGivenUTI(it); //may revert to its root
	setNodeType(it);
	m_ready = true; //set here!!!
	//assert(m_state.isComplete(it)); //true? not if baseclasssize still unknown (t41523)
      }
    else if(it == Hzy)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
      }
    else
      {
	it = resetGivenUTI(it);
	setNodeType(it); //though incomplete (t41213, t41469)
      }

    return getNodeType();
  } //checkAndLabelType

  bool NodeTypeDescriptor::resolveType(UTI& rtnuti)
  {
    bool rtnb = false;

    // not node select, we are the leaf Type: a typedef, class or primitive scalar.
    UTI nuti = givenUTI(); //start with given.
    bool isreferencetype = (getReferenceType() != ALT_NOT) || m_state.isAltRefType(nuti);

    if(isreferencetype) //t3668?
      {
	rtnb = resolveReferenceType(nuti); //may update nuti
	if(nuti == Nav)
	  {
	    rtnuti = Nav;
	    return false;
	  }
	rtnuti = nuti;
	//if(m_state.isAClass(nuti) && m_state.isScalar(nuti))
	//  return rtnb; //done? if a class, (t3172, t41444)
	//else still resolve if primitive, or array (t3668, t3653)
      }

    isreferencetype = (getReferenceType() != ALT_NOT) || m_state.isAltRefType(nuti); //refresh
    if(!m_state.isComplete(nuti) && !isreferencetype)
      {
	// if Nav, use token
	UTI mappedUTI = nuti;

	if(m_state.findRootUTIAlias(nuti, mappedUTI) && !m_state.isHolder(mappedUTI))
	  nuti = mappedUTI;

	//if no change, try context..
	UTI cuti = m_state.getCompileThisIdx();
	if(mappedUTI == nuti)
	  {
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
		assert(getReferenceType() == m_state.getReferenceType(mappedUTI)); //t41455
		nuti = mappedUTI;
	      }
	  }

	if(!m_state.isComplete(nuti)) //reloads to recheck for debug message
	  {
	    u32 nameid = m_state.getUlamTypeNameIdByIndex(nuti);
	    std::ostringstream msg;
	    msg << "Incomplete descriptor for type: ";
	    msg << m_state.m_pool.getDataAsString(nameid).c_str(); //t3125, t3298, t41361
	    msg << " (UTI " << nuti << ")"; //helpful for my debugging (e.g. t41209)
	    msg << ", while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << " (UTI " << cuti << ")"; //helpful for my debugging (e.g. t41440)
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //t3787 was WAIT
	  }
      }

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    if((etyp == Class))
      {
	rtnb = resolveClassType(nuti);
	if(rtnb && isreferencetype && !m_state.isReference(nuti))
	  {
	    //try again..
	    rtnb = resolveReferenceType(nuti); //20210726 ish
	  }
      }
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
	    if(m_state.statusUnknownClassTypeInThisLocalsScope(m_typeTok, nuti, kuti))
	      {
		kuti = resetGivenUTI(kuti); //even if not complete, no longer a holder
		if(!(rtnb = m_state.isComplete(kuti)))
		  kuti = Hzy;
	      }
	    nuti = kuti;
	  }

	//holder, culam-generated typedef, not resolveable as a holder. (t3565, t41536)
	//if((nuti == Hzy) || m_state.isHolder(nuti))
	if(nuti == Hzy)
	  {
	    UTI tduti = Nouti;
	    UTI tmpforscalaruti = Nouti;
	    u32 tokid = m_state.getTokenDataAsStringId(m_typeTok);
	    TBOOL isTypedef = TBOOL_FALSE;
	    if(!m_state.isThisLocalsFileScope())
	      isTypedef = m_state.getUlamTypeByTypedefNameInClassHierarchyThenLocalsScope(tokid, tduti, tmpforscalaruti);
	    else
	      isTypedef = m_state.getUlamTypeByTypedefNameInLocalsScope(tokid, tduti, tmpforscalaruti);

	    if(isTypedef == TBOOL_TRUE)
	      {
		if(m_state.okUTItoContinue(tduti) && !m_state.isHolder(tduti))
		  {
		    UTI mappedtd = tduti;
		    m_state.findRootUTIAlias(tduti, mappedtd);
		    nuti = mappedtd; //reset
		    rtnb = true;
		  }
	      }
	  }
      }
    else if(etyp == LocalsFileScope)
      {
	rtnb = true; //t41517 selected
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
    UTI cuti = m_state.getCompileThisIdx();
    UTI nuti = rtnuti; //preset to givenUTI(), or typedef before reset
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UTI derefuti = getReferencedUTI();

    assert((getReferenceType() != ALT_NOT) || (m_state.isAltRefType(nuti))); //t3668?

    if(m_state.isHolder(nuti) || m_state.isHolder(derefuti))
      {
	//switch places when ref UTI was not created at parse time (t41491)
	if(!m_state.isAltRefType(nuti) && m_state.isComplete(nuti))
	  {
	    derefuti = nuti; //t41491
	    nuti = givenUTI();
	    assert(nuti != derefuti);
	  }
	else if(derefuti == nuti)
	  {
	    UTI mappedUTI = nuti;
	    if(m_state.findRootUTIAlias(nuti, mappedUTI))
	      {
		derefuti = mappedUTI; //t41298,9 ?
	      }
	    else
	      return false;
	  }
	else
	  return false;
      }

    if(derefuti == Nouti)
      {
	//we didn't realize it was a reference at parse time; likely a typedef..(t3668)
	UTI tduti = Nouti;
	UTI tmpforscalaruti = Nouti;
	u32 tokid = m_state.getTokenDataAsStringId(m_typeTok);
	TBOOL isTypedef = m_state.getUlamTypeByTypedefNameInClassHierarchyThenLocalsScope(tokid, tduti, tmpforscalaruti);

	if(isTypedef == TBOOL_TRUE)
	  {
	    if(m_state.okUTItoContinue(tduti) && !m_state.isHolder(tduti))
	      {
		ALT altgiven = getReferenceType();
		ALT altd = m_state.getReferenceType(tduti);
		if((altgiven != ALT_NOT) && (altd == ALT_NOT))
		  {
		    //do nothing, fall thru..
		  }
		else
		  {
		    assert((altgiven==altd) || altd==m_state.getReferenceType(givenUTI())); //t3843
		    UTI mappedtd = tduti;
		    m_state.findRootUTIAlias(tduti, mappedtd);
		    rtnuti = mappedtd; //reset
		    return true;
		  }
	      }
	  }
      }

    //if reference is not complete, but its deref is, use its sizes to complete us.
    if(!m_state.okUTItoContinue(derefuti) && m_state.okUTItoContinue(nuti))
      {
	if(nut->getUlamTypeEnum() == Class)
	  derefuti = nut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureClassInstanceIdx();
	else
	  derefuti = m_state.getUlamTypeAsDeref(nuti); //t41449 both are Hzy..
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
	    if(m_state.mappedIncompleteUTI(cuti, derefuti, mappedUTI) && (derefuti != mappedUTI))
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

	    if((nuti == Hzy) || m_state.isHolder(nuti) || !m_state.isAltRefType(nuti)) //t41491
	      {
		nuti = m_state.getUlamTypeAsRef(derefuti, getReferenceType());
		rtnb = true;
	      }
	    else
	      {
		// we might have set the size of a holder ref. still a holder. darn.
		rtnb = m_state.completeAReferenceTypeWith(nuti, derefuti);
	      }

	    ALT altd = m_state.getReferenceType(nuti);
	    setReferenceType(altd, derefuti);
	  } //complete deref
	//else //deref not complete, t.f. nuti isn't changed (t41298,9)
	//rtnuti = Hzy; //t41153
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
	if(m_state.findRootUTIAlias(nuti, aliasuti))
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
	    TBOOL isTypedef = m_state.getUlamTypeByTypedefNameInClassHierarchyThenLocalsScope(m_typeTok.m_dataindex, tduti, tmpforscalaruti);

	    if(isTypedef == TBOOL_TRUE)
	      {
		//unseen typedef's appear like Class basetype, until seen
		//wait until complete to re-key..want arraysize if non-scalar
		std::ostringstream msg;
		if(m_state.isComplete(tduti))
		  {
		    UTI tdalias = tduti;
		    m_state.findRootUTIAlias(tduti, tdalias); //20210726 ish
		    m_state.cleanupExistingHolder(nuti, tdalias); //20210722 ish
		    rtnuti = tdalias; //reset
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
	    else //false or hazy
	      {
		bool stubcopy = false;
		bool isAnonymousClass = (nut->isHolder() || !m_state.isARootUTI(nuti));
		UTI auti = nuti;

		if(nut->isHolder())
		  {
		    if(m_state.isThisLocalsFileScope() || !m_state.statusUnknownTypeInThisClassResolver(nuti))
		      auti = Hzy;
		  }
		else if(!m_state.isARootUTI(nuti))
		  {
		    AssertBool isAlias = m_state.findRootUTIAlias(nuti, auti);
		    assert(isAlias);
		  }
		else if(m_state.isClassAStubCopy(nuti))
		  {
		    stubcopy = true;
		  }
		std::ostringstream msg;
		msg << "UNSEEN Class and incomplete descriptor for type: '";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str() << "'";
		if(isAnonymousClass) //t41413
		  {
		    msg << " (UTI" << nuti << ")";
		    msg << " replaced with type: (UTI" << auti << ")";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    rtnuti = auti; //was Hzy
		  }
		else if(stubcopy)
		  {
		    msg << " (UTI" << nuti << ")";
		    msg << " is a stubcopy";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //was WAIT
		    rtnuti = Hzy; //t41213
		  }
		else
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //was WAIT
		    rtnuti = Hzy; //t3407
		  }
	      }
	  }
	//else
	// rtnuti = Hzy;
      }
    else if(m_state.isClassAStub(nuti))
      {
	std::ostringstream msg;
	msg << "SEEN Class Stub and incomplete descriptor for type: '";
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str() << "'";
	msg << " (UTI " << nuti << ")";
	msg << " is a stub";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //was WAIT
	rtnuti = Hzy; //t41213, t3444
      }
    //else incomplete, non-stub, not Hzy (t41469)
    return rtnb;
  } //resolveClassType

  bool NodeTypeDescriptor::resolvePrimitiveOrArrayType(UTI& rtnuti)
  {
    bool rtnb = false;
    UTI nuti = rtnuti; //givenUTI(); //rtnuti; t3653, t3651
    //primitive, or array typedef
    if(!m_unknownBitsizeSubtree)
      {
	if(!m_state.okUTItoContinue(nuti))
	  {
	    ALT altd = getReferenceType();
	    //use given UTI, not the not-ok nuti here.. (t41288)
	    if(m_state.okUTItoContinue(givenUTI()) && !m_state.isHolder(givenUTI())) //t3378
	      {
		UlamType * nut = m_state.getUlamTypeByIndex(givenUTI());
		ULAMTYPE etyp = nut->getUlamTypeEnum();
		s32 arraysize = nut->getArraySize(); //NONARRAYSIZE for scalars
		ULAMCLASSTYPE classtype = nut->getUlamClassType(); //t3735,t3834,t41363,t41153
		//use default primitive bitsize;
		nuti = m_state.makeUlamType(m_typeTok, ULAMTYPE_DEFAULTBITSIZE[etyp], arraysize, getReferencedUTI(), altd, classtype);
		assert(m_state.okUTItoContinue(nuti)); //ish 20230116
		rtnb = true;
	      }
	    else if(getReferenceType() != ALT_NOT)
	      {
		//must be a reference type; use type token to make one!
		assert((altd == ALT_REF) || (altd == ALT_CONSTREF));
		ULAMTYPE etyp = m_state.getBaseTypeFromToken(m_typeTok);
		if((etyp != Class) && (etyp != Hzy))
		  {
		    //use default primitive bitsize; (assumes scalar)
		    nuti = m_state.makeUlamType(m_typeTok, ULAMTYPE_DEFAULTBITSIZE[etyp], NONARRAYSIZE, Nouti, altd, UC_NOTACLASS);
		    rtnb = true; //t3689,t3696,t3760,t3792,t3793
		  }
	      }
	    else
	      {
		nuti = Hzy; //t41288
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

    if(nuti == Hzy) //t41203, nav returned
      {
	u32 tokid = m_state.getTokenDataAsStringId(m_typeTok);
	UTI tduti = Nouti;
	UTI tmpforscalaruti = Nouti;
	bool isTypedef = ((m_typeTok.m_type == TOK_TYPE_IDENTIFIER) || (m_typeTok.m_type == TOK_KW_TYPE_SUPER) || (m_typeTok.m_type == TOK_KW_TYPE_SELF)) && (m_state.getUlamTypeByTypedefNameInClassHierarchyThenLocalsScope(tokid, tduti, tmpforscalaruti) == TBOOL_TRUE); //skip primitive types t41616,t41621

	if(isTypedef && !m_state.isHolder(tduti) && m_state.okUTItoContinue(tduti)) //t3765, t3384
	  {
	    UTI mappedtd = tduti;
	    m_state.findRootUTIAlias(tduti, mappedtd);
	    tduti = mappedtd;
	    UTI guti = givenUTI();
	    ALT givenreftype = getReferenceType();
	    if(m_state.isReference(tduti))
	      {
		ALT tdreftype = m_state.getReferenceType(tduti);
		UTI tdderef = m_state.getUlamTypeAsDeref(tduti);
		if(givenreftype == tdreftype)
		  {
		    setReferenceType(tdreftype, tdderef); //t3666 (no change to givenUTI)
		    m_state.updateUTIAliasForced(guti, tduti); //t3666
		  }
	      }
	    else if(givenreftype != ALT_NOT)
	      {
		UTI tdderef = tduti;
		setReferenceType(givenreftype, tdderef); //tduti might not be guti alias
		UTI tdasref = m_state.getUlamTypeAsRef(tduti, givenreftype);
		tduti = tdasref; //to continue (t41436)
	      }
	    //else alias handled during resetGivenUTI

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
    if(rtnuti == Nav)
      return false; //t3767, next time around..

    UTI auti = rtnuti;
    if(!m_state.okUTItoContinue(auti)) //Hzy not useful, t3653
      auti = givenUTI();

    UlamType * ut = m_state.getUlamTypeByIndex(auti);
    ULAMTYPE etyp = ut->getUlamTypeEnum();
    if(m_unknownBitsizeSubtree)
      {
	UTI sizetype = auti; //t3653,t3651
	s32 bs = UNKNOWNSIZE;
	//primitive with possible unknown bit sizes.
	bool rtnb = m_unknownBitsizeSubtree->getTypeBitSizeInParen(bs, etyp, sizetype); //eval
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
	    if(!m_state.setUTISizes(auti, bs, ut->getArraySize())) //update UlamType, outputs errs
	      {
		rtnuti = Nav;
		return false;
	      }
	    else
	      {
		rtnuti = auti;
	      }
	  }
	else
	  {
	    //??rtnuti = auti; //could be Hzy or Nav
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

  EvalStatus NodeTypeDescriptor::eval()
  {
    m_state.abortShouldntGetHere();  //not in parse tree; part of Node's type
    return NORMAL;
  } //eval

} //end MFM
