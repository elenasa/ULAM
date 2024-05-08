#include <iomanip>
#include "SymbolClassNameTemplate.h"
#include "Resolver.h"
#include "CompilerState.h"


namespace MFM {

  SymbolClassNameTemplate::SymbolClassNameTemplate(const Token& id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClassName(id, utype, classblock, state) { }

  SymbolClassNameTemplate::~SymbolClassNameTemplate()
  {
    // possible stub instances that were never fully instantiated
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * sym = it->second;
	if(sym && sym->isStub())
	  {
	    delete sym;
	    it->second = NULL;
	  }
	it++;
      }
    m_scalarClassInstanceIdxToSymbolPtr.clear(); //many-to-1 (possible after fully instantiated)

    std::map<UTI, SymbolClass* >::iterator pit = m_scalarClassInstanceIdxToSymbolPtrTEMP.begin();
    while(pit != m_scalarClassInstanceIdxToSymbolPtrTEMP.end())
      {
	SymbolClass * sym = pit->second;
	if(sym && sym->isStub())
	  {
	    delete sym;
	    pit->second = NULL;
	  }
	pit++;
      }
    m_scalarClassInstanceIdxToSymbolPtrTEMP.clear(); //should be empty after each cloneInstance attempt

    // need to delete class instance symbols; ownership belongs here!
    std::map<std::string, SymbolClass* >::iterator mit = m_scalarClassArgStringsToSymbolPtr.begin();
    while(mit != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * msym = mit->second;
	delete msym;
	mit->second = NULL;
	mit++;
      }
    m_scalarClassArgStringsToSymbolPtr.clear();

    m_mapOfTemplateUTIToInstanceUTIPerClassInstance.clear();

    m_locStubsDeleted.clear();
  } //destructor

  void SymbolClassNameTemplate::getTargetDescriptorsForClassInstances(TargetMap& classtargets)
  {
    u32 scid = 0;
    Token scTok;
    if(getStructuredComment(scTok))
      scid = scTok.m_dataindex;

    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(m_state.okUTItoContinue(csym->getUlamTypeIdx()));
	csym->addTargetDescriptionMapEntry(classtargets, scid);
	it++;
      }
  } //getTargetDescriptorsForClassInstances

  void SymbolClassNameTemplate::getClassMemberDescriptionsForClassInstances(ClassMemberMap& classmembers)
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(m_state.okUTItoContinue(csym->getUlamTypeIdx()));
	csym->addClassMemberDescriptionsMapEntry(classmembers);
	it++;
      }
  } //getClassMemberDescriptionsForClassInstances

  u32 SymbolClassNameTemplate::getNumberOfParameters()
  {
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock); //fails if UNSEEN during parsing
    return templateclassblock->getNumberOfParameterNodes();
  }

  bool SymbolClassNameTemplate::parameterHasDefaultValue(u32 n)
  {
    // used while parsing, so c&l not called, and symbol hasDefault not yet set.
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock); //fails if UNSEEN during parsing
    Node * pnode = templateclassblock->getParameterNode(n);
    assert(pnode);
    return ((NodeConstantDef *) pnode)->hasConstantExpr(); //t3526
  } //parameterHasDefaultValue

  u32 SymbolClassNameTemplate::getTotalParametersWithDefaultValues()
  {
    u32 count = 0;
    u32 numparams = getNumberOfParameters();
    for(u32 i = 0; i < numparams; i++)
      {
	if(parameterHasDefaultValue(i))
	  count++;
      }
    return count;
  } //getTotalParametersWithDefaultValues

  u32 SymbolClassNameTemplate::getTotalParameterSlots()
  {
    u32 totalsizes = 0;
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock); //fails if UNSEEN during parsing

    u32 numparams = getNumberOfParameters();
    for(u32 i = 0; i < numparams; i++)
      {
	NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassblock->getParameterNode(i);
	assert(paramConstDef);
	UTI puti = paramConstDef->getTypeDescriptorGivenType();

	//types could be incomplete, sizes unknown for template
	totalsizes += m_state.slotsNeeded(puti);
      }
    return totalsizes;
  } //getTotalParameterSlots

  bool SymbolClassNameTemplate::isClassTemplate()
  {
    return true;
  }

  bool SymbolClassNameTemplate::isClassTemplate(UTI cuti)
  {
    if(cuti == getUlamTypeIdx())
      return true; // template definition

    SymbolClass * csym = NULL;
    return !findClassInstanceByUTI(cuti, csym);
  } //isClassTemplate

  void SymbolClassNameTemplate::setSuperClassForClassInstance(UTI superclass, UTI instance)
  {
    assert(instance != superclass);
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      csym->setBaseClass(superclass, 0); //Nouti is none, not a subclass.
    else if(instance == getUlamTypeIdx())
      SymbolClass::setBaseClass(superclass, 0); //instance is template definition
    else
      m_state.abortShouldntGetHere(); //not found???
  } //setSuperClassForClassInstance

  UTI SymbolClassNameTemplate::getSuperClassForClassInstance(UTI instance)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      return csym->getBaseClass(0); //Nouti is none, Hzy a stub
    else if(instance == getUlamTypeIdx())
      return SymbolClass::getBaseClass(0); //instance is template definition
    return false;
  } //getSuperClassForClassInstance

  void SymbolClassNameTemplate::appendBaseClassForClassInstance(UTI baseclass, UTI instance, bool sharedbase)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      csym->appendBaseClass(baseclass, sharedbase); //Nouti is none, not a subclass.
    else if(instance == getUlamTypeIdx())
      SymbolClass::appendBaseClass(baseclass, sharedbase); //instance is template definition
    else
      m_state.abortShouldntGetHere(); //not found???
  }

  u32 SymbolClassNameTemplate::getBaseClassCountForClassInstance(UTI instance)
  {
    u32 count = 0;
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      count = csym->getBaseClassCount(); //Nouti is none, not a subclass.
    else if(instance == getUlamTypeIdx())
      count = SymbolClass::getBaseClassCount(); //instance is template definition
    else
      m_state.abortShouldntGetHere(); //not found???

    return count;
  }

  UTI SymbolClassNameTemplate::getBaseClassForClassInstance(UTI instance, u32 item)
  {
    UTI baseuti = Nouti; // Avoid compiler warning on 16.04
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      baseuti = csym->getBaseClass(item); //Nouti is none, not a subclass.
    else if(instance == getUlamTypeIdx())
      SymbolClass::getBaseClass(item); //instance is template definition
    else
      m_state.abortShouldntGetHere(); //not found???
    return baseuti;
  }

  bool SymbolClassNameTemplate::updateBaseClassforClassInstance(UTI instance, UTI oldbase, UTI newbaseuti)
  {
    bool aok = false;
    s32 item = -1;
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      item = csym->isABaseClassItem(oldbase); //Nouti is none, not a subclass.
    else if(instance == getUlamTypeIdx())
      item = SymbolClass::isABaseClassItem(oldbase); //instance is template definition
    else
      m_state.abortShouldntGetHere(); //not found???

    if(item > 0) //excludes super
      {
	if(csym != NULL)
	  csym->updateBaseClass(oldbase, item, newbaseuti);
	else
	  SymbolClass::updateBaseClass(oldbase, item, newbaseuti);
	aok = true;
      }
    return aok;
  } //updateBaseClassforClassInstance

  void SymbolClassNameTemplate::initBaseClassListForAStubClassInstance(SymbolClass * newclassinstance)
  {
    assert(newclassinstance);
    //from template
    u32 basecount = SymbolClass::getBaseClassCount() + 1; //includes super
    for(u32 i = 0; i < basecount; i++)
      {
	UTI baseuti = SymbolClass::getBaseClass(i);
	bool sharedbase = SymbolClass::isDirectSharedBase(i);
	if(m_state.okUTItoContinue(baseuti))
	  {
	    if(m_state.isClassAStub(baseuti))
	      {
		//need a copy of the super stub, and its uti
		baseuti = Hzy; //wait until resolving loop.
	      }
	    else
	      {
		UTI balias = baseuti;
		if(m_state.findRootUTIAlias(baseuti, balias))
		  baseuti = balias;
	      }
	    newclassinstance->setBaseClass(baseuti, i, sharedbase);
	    //any superclass block links are handled during c&l
	  }
	else
	  {
	    ULAMCLASSTYPE tclasstype = getUlamClass();
	    if(tclasstype == UC_UNSEEN)
	      newclassinstance->setBaseClass(Hzy, i, sharedbase);
	  }
      }
  } //initBaseClassListForAStubClassInstance


  // (does not include template as an instance!)
  bool SymbolClassNameTemplate::findClassInstanceByUTI(UTI uti, SymbolClass * & symptrref)
  {
    UTI basicuti = m_state.getUlamTypeAsDeref(m_state.getUlamTypeAsScalar(uti));
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.find(basicuti);
    if(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	symptrref = it->second;
	//maybe be duplicates, same symbol, different UTIs (t3327)
	assert(it->first == basicuti); //cheap sanity check
	return true;
      }

    //in case in the middle of stub copy..(t3328)
    std::map<UTI, SymbolClass* >::iterator tit = m_scalarClassInstanceIdxToSymbolPtrTEMP.find(basicuti);
    if(tit != m_scalarClassInstanceIdxToSymbolPtrTEMP.end())
      {
	symptrref = tit->second;
	return true;
      }

    //in case there a mapped UTI (t41436)
    UTI mappedUTI = basicuti;
    if(hasMappedUTI(basicuti, mappedUTI))
      {
	std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.find(mappedUTI);
	if(it != m_scalarClassInstanceIdxToSymbolPtr.end())
	  {
	    symptrref = it->second;
	    //maybe be duplicates, same symbol, different UTIs (t3327)
	    assert(it->first == mappedUTI); //cheap sanity check
	    return true;
	  }
      }

    if(m_state.findRootUTIAlias(basicuti, mappedUTI)) //t3862
      {
	std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.find(mappedUTI);
	if(it != m_scalarClassInstanceIdxToSymbolPtr.end())
	  {
	    symptrref = it->second;
	    //maybe be duplicates, same symbol, different UTIs (t3327)
	    assert(it->first == mappedUTI); //cheap sanity check
	    return true;
	  }
      }

    return false;
  } //findClassInstanceByUTI

  bool SymbolClassNameTemplate::findClassInstanceByArgString(UTI cuti, SymbolClass *& csymptr)
  {
    bool found = false;
    std::string argstring = formatAnInstancesArgValuesAsAString(cuti);

    std::map<std::string, SymbolClass* >::iterator mit = m_scalarClassArgStringsToSymbolPtr.find(argstring);
    if(mit != m_scalarClassArgStringsToSymbolPtr.end())
      {
	csymptr = mit->second;
	found = true;
      }
    return found;
  } //findClassInstanceByArgString

  void SymbolClassNameTemplate::addClassInstanceUTI(UTI uti, SymbolClass * symptr)
  {
    assert(symptr && uti == symptr->getUlamTypeIdx()); //insane!!
    std::pair<std::map<UTI,SymbolClass *>::iterator,bool> ret;
    ret = m_scalarClassInstanceIdxToSymbolPtr.insert(std::pair<UTI,SymbolClass*> (uti,symptr)); //stub
    assert(ret.second); //false if already existed, i.e. not added
  }

  void SymbolClassNameTemplate::addClassInstanceByArgString(UTI uti, SymbolClass * symptr)
  {
    //new (full) entry, and owner of symbol class for class instances with args
    std::string argstring = formatAnInstancesArgValuesAsAString(uti);
    m_scalarClassArgStringsToSymbolPtr.insert(std::pair<std::string,SymbolClass*>(argstring,symptr));
  }

  bool SymbolClassNameTemplate::pendingClassArgumentsForStubClassInstance(UTI instance)
  {
    if(instance == getUlamTypeIdx())
      return false; //WHO KNOWS!!? t41440

    bool rtnpending = false;
    if((getNumberOfParameters() > 0) || (getUlamClass() == UC_UNSEEN))
      {
	SymbolClass * csym = NULL;
	AssertBool isDefined = findClassInstanceByUTI(instance, csym);
	assert(isDefined);
	assert(!csym->isStubCopy());
	rtnpending = csym->pendingClassArgumentsForClassInstance();
	assert(!rtnpending || csym->isStub());
      }
    return rtnpending;
  } //pendingClassArgumentsForStubClassInstance

  //Uses of Template classes. Note: makeAStub.. called in ONE place,
  // by the Parser.  Stubs contain template's member symbol table, and
  // member parse tree (named constants, typedefs, dm, and funcdecls),
  // as well as class argument nodes, but not functions. Goal: ablity
  // to use member named constants as default values of class
  // parameters.  Functions are added during full instantiation, when
  // no args are pending and all baseclasses are known.
  SymbolClass * SymbolClassNameTemplate::makeAStubClassInstance(const Token& typeTok, UTI stubcuti)
  {
    NodeBlockClass * templateclassblock = getClassBlockNode();
    if(templateclassblock == NULL)
      return NULL; //probably previous error caused this to happen (t41166).

    bool isCATemplate = ((UlamTypeClass *) m_state.getUlamTypeByIndex(getUlamTypeIdx()))->isCustomArray();

    //previous block is template's class block, and new NNO here!
    NodeBlockClass * newblockclass = new NodeBlockClass(NULL, m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(typeTok.m_locator); //for arg values
    newblockclass->setNodeType(stubcuti);
    newblockclass->resetNodeNo(templateclassblock->getNodeNo()); //keep NNO consistent (new)

    Token stubTok(TOK_TYPE_IDENTIFIER, typeTok.m_locator, getId());
    SymbolClass * newclassinstance = new SymbolClass(stubTok, stubcuti, newblockclass, this, m_state);
    assert(newclassinstance);
    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    if(SymbolClass::isConcreteClass())
      newclassinstance->setConcreteClass();  //t41664

    //inheritance: (multi-inheritance ulam-5), a difference btn seen/unseen templates
    initBaseClassListForAStubClassInstance(newclassinstance); //t3889

    if(isCATemplate)
      ((UlamTypeClass *) m_state.getUlamTypeByIndex(stubcuti))->setCustomArray();

    newclassinstance->mapUTItoUTI(getUlamTypeIdx(), stubcuti); //map template->instance, instead of fudging.

    addClassInstanceUTI(stubcuti, newclassinstance); //link here

    //before patching in data member symbols, like typedef "super".
    UTI compilingthis = m_state.getCompileThisIdx();
    if(flagpAsAStubForTemplate(compilingthis) || flagpAsAStubForTemplateMemberStub(compilingthis))
      {
        newclassinstance->setStubForTemplateType(compilingthis); //t41225, t3336?
        if(getUlamTypeIdx() == compilingthis)
          newclassinstance->setStubForTemplateTypeIncomplete(true);
      }

    //a difference between them t41442
    if(m_state.isASeenClass(getUlamTypeIdx()))
      {
	newclassinstance->partialInstantiationOfMemberNodesAndSymbols(*templateclassblock);
	cloneAnInstancesUTImap(this, newclassinstance); //t3384,t3565??

	//following Sergei's commit..20230311
	if(getUlamTypeIdx() == compilingthis)
          newclassinstance->setStubForTemplateTypeIncomplete(true); //t41648
      } //else wait if template is unseen

    return newclassinstance;
  } //makeAStubClassInstance


  //very minimal copy, since iteration might be in progress.
  //copying of Args done during upgradeStubCopyToAStub.. after merge, during c&l.
  //here, starts with  new class block and symbol, not clones;
  //note, template could still be unseen..
  SymbolClass * SymbolClassNameTemplate::copyAStubClassInstance(UTI instance, UTI newuti, UTI argvaluecontext, UTI argtypecontext, UTI valuecontextclassnametype, Locator newloc)
  {
    assert((getNumberOfParameters() > 0) || (getUlamClass() == UC_UNSEEN));
    assert(instance != newuti);

    UTI compilingthis = m_state.getThisClassForParsing(); //possibly no longer parsing
    compilingthis = compilingthis == Nouti ? m_state.getCompileThisIdx() : compilingthis;

    UTI stubcopyof = m_state.getStubCopyOf(instance);
    bool twasastubcopy = (stubcopyof != Nouti);
    assert(!twasastubcopy); //t41448, t41452

    SymbolClass * csym = NULL;
    AssertBool isDefined = findClassInstanceByUTI(instance, csym);
    assert(isDefined);

    assert(!csym->isStubCopy());
    assert(csym->isStub());
    assert(csym->pendingClassArgumentsForClassInstance()); //t41222

    NodeBlockClass * blockclass = csym->getClassBlockNode();
    assert(blockclass);
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock);
    bool isCATemplate = ((UlamTypeClass *) m_state.getUlamTypeByIndex(getUlamTypeIdx()))->isCustomArray();

    NodeBlockClass * newblockclass = new NodeBlockClass(NULL, m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(newloc); //questionable LOC? t41221

    //provides proper context for setting type (e.g. t3640)
    m_state.pushClassContext(newuti, newblockclass, newblockclass, false, NULL);

    newblockclass->setNodeType(newuti);
    newblockclass->resetNodeNo(templateclassblock->getNodeNo()); //keep NNO consistent (new)

    Token stubTok(TOK_TYPE_IDENTIFIER,csym->getLoc(), getId());

    SymbolClass * newclassinstance = new SymbolClass(stubTok, newuti, newblockclass, this, m_state);
    assert(newclassinstance);
    assert(newclassinstance->getUlamClass() == getUlamClass()); //t41436

    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    if(SymbolClass::isConcreteClass())
      newclassinstance->setConcreteClass();  //t41664

    if(isCATemplate)
      ((UlamTypeClass *) m_state.getUlamTypeByIndex(newuti))->setCustomArray(); //t41007

    if(valuecontextclassnametype != getUlamTypeIdx()) //no mapping if recursive t41645
      newclassinstance->mapUTItoUTI(instance, newuti); //map stub->stubcopy, instead of FUDGING. (t41224)
    newclassinstance->mapUTItoUTI(getUlamTypeIdx(), newuti); //map template->instance, instead of fudging

    //inheritance: (multi-inheritance ulam-5)
    initBaseClassListForAStubClassInstance(newclassinstance);

    //before patching in data member symbols, like typedef "super".
    if(flagpAsAStubForTemplate(compilingthis))
      {
	newclassinstance->setStubForTemplateType(compilingthis); //t41225? t41224, t41436
      }
    else if(flagpAsAStubForTemplateMemberStub(compilingthis)) //t41436?
      {
	UTI ttype = m_state.getMemberStubForTemplateType(compilingthis);
	newclassinstance->setStubForTemplateType(ttype);
      }

    //can't addClassInstanceUTI(newuti, newclassinstance) ITERATION IN PROGRESS!!!
    m_scalarClassInstanceIdxToSymbolPtrTEMP.insert(std::pair<UTI,SymbolClass*> (newuti,newclassinstance));

    newclassinstance->setContextForPendingArgValues(argvaluecontext);
    newclassinstance->setContextForPendingArgTypes(argtypecontext);

    newclassinstance->setStubCopy(); //effects getUlamTypeNameBriefByIndex..
    newclassinstance->setStubCopyOf(instance); //effects upgradeStubCopyToAStub..

    m_state.popClassContext(); //restore
    return newclassinstance;
  } //copyAStubClassInstance

  bool SymbolClassNameTemplate::flagpAsAStubForTemplate(UTI compilingthis)
  {
    if(m_state.isClassATemplate(compilingthis))
      {
	if((m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSINHERITANCE))
	  return true;
	if((m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSPARAMETER))
	  return true;
	if((m_state.m_parsingVariableSymbolTypeFlag == STF_MEMBERCONSTANT))
	  return true;
	if((m_state.m_parsingVariableSymbolTypeFlag == STF_MEMBERTYPEDEF))
	  return true;
      }
    return false;
  }

  bool SymbolClassNameTemplate::flagpAsAStubForTemplateMemberStub(UTI compilingthis)
  {
    if(m_state.isClassAMemberStubInATemplate(compilingthis))
      {
	//return true; //regardless
	if((m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSINHERITANCE))
	  return true;
	if((m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSPARAMETER))
	  return true;
	if((m_state.m_parsingVariableSymbolTypeFlag == STF_MEMBERCONSTANT))
	  return true;
	if((m_state.m_parsingVariableSymbolTypeFlag == STF_MEMBERTYPEDEF))
	  return true;
      }
    return false;
  }

  // called by parseThisClass, if wasIncomplete is parsed; temporary class arg names
  // are fixed to match the params, default arg values, and node type descriptors
  // for uses seen first! Parser::parseRestOfClassArguments() handles all this when
  // template seen before any uses.
  void SymbolClassNameTemplate::fixAnyUnseenClassInstances()
  {
    UTI cuti = getUlamTypeIdx();

    ULAMCLASSTYPE classtype = getUlamClass();
    assert(classtype != UC_UNSEEN);

    //furthermore, this must exist by now, or else this is the wrong time to be fixing
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock);
    bool isCATemplate = ((UlamTypeClass *) m_state.getUlamTypeByIndex(cuti))->isCustomArray();

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      return;

    u32 numparams = getNumberOfParameters();
    u32 numDefaultParams = getTotalParametersWithDefaultValues();
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(csym);
	UTI suti = csym->getUlamTypeIdx();
	if(!m_state.okUTItoContinue(suti))
	  {
	    it++;
	    continue;
	  }

	if(csym->getUlamClass() != UC_UNSEEN)
	  {
	    it++; //covers synonyms
	    continue;
	  }

	if(csym->isStubCopy())
	  {
	    it++; //wait for upgrade during c&l
	    continue;
	  }

	UlamKeyTypeSignature skey = m_state.getUlamTypeByIndex(suti)->getUlamKeyTypeSignature();
	AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(skey, Class, classtype, isCATemplate);
	assert(isReplaced);

	NodeBlockClass * cblock = csym->getClassBlockNode();
	assert(cblock);

	assert(cblock->getNodeNo() == templateclassblock->getNodeNo()); //keep NNOs consistent, sanity or set?

	//can have 0Holder symbols for possible typedefs seen from another class
	//which will increase the count of symbols; can only test for at least;
	u32 cargs = cblock->getNumberOfPotentialClassArgumentSymbols();
	if((cargs < numparams) && ((cargs + numDefaultParams) < numparams))
	  {
	    //number of arguments in class instance does not match the number of parameters
	    // including those with default values (u.1.2.1)
	    std::ostringstream msg;
	    msg << "Number of Arguments (" << cargs << ") in class instance '";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str(); //not a uti
	    msg << "' is insufficient for the required number of template parameters (";
	    msg << numparams << ")";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	    it++;
	    continue;
	  }

	if((cargs > numparams))
	  {
	    //number of arguments in class instance does not match the number of parameters
	    // including those with default values (u.1.2.1)
	    std::ostringstream msg;
	    msg << "Number of Arguments (" << cargs << ") in class instance '";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str(); //not a uti
	    msg << "' is beyond the required number of template parameters (";
	    msg << numparams << ")";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	    it++;
	    continue;
	  }

	UTI context = csym->getContextForPendingArgValues();
	if(m_state.isAClass(context) && m_state.getAClassBlock(context) == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Context " << m_state.getUlamTypeNameByIndex(context).c_str();
	    msg << " needed to fix any unseen class instances of template '";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str(); //not a uti
	    msg << "' is missing its class definition; Bailing..";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);  //t41432
	    it++;
	    continue;
	  }

	context = ((context == Nouti) ? suti : context); //t41446

	UTI typecontext = csym->getContextForPendingArgTypes(); //t41209, t41218
	typecontext = ((typecontext == Nouti) ? suti : typecontext);

	//this is what the Parser does had it seen the template first!
	m_state.pushClassOrLocalContextAndDontUseMemberBlock(context);
	m_state.pushCurrentBlock(cblock);

	//replace the temporary id with the official parameter name id;
	//update the class instance's ST, and argument list.
	u32 foundArgs = 0;
	s32 firstDefaultParamUsed = -1;
	s32 lastDefaultParamUsed = -1;
	for(s32 i = 0; i < (s32) numparams; i++)
	  {
	    Symbol * argsym = NULL;
	    std::ostringstream sname;
	    sname << "_" << i;
	    u32 sid = m_state.m_pool.getIndexForDataString(sname.str());

	    NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassblock->getParameterNode(i);
	    assert(paramConstDef);
	    u32 pid = paramConstDef->getSymbolId();

	    if(cblock->isIdInScope(sid,argsym))
	      {
		assert(argsym->isConstant());
		((SymbolConstantValue *) argsym)->changeConstantId(sid, pid);
		cblock->replaceIdInScope(sid, pid, argsym);

		//any type descriptors need to be copied (t41209,t41211);
		//including classes that might be holders (t41216)
		NodeConstantDef * stubConstDef = (NodeConstantDef *) cblock->getArgumentNode(i);
		assert(stubConstDef);

		//came from Parser parseRestOfClassArguments says null blocks likely (t41214)
		m_state.pushClassContext(suti, cblock, cblock, false, NULL);
		//if any and none (t41211, t41213, error/t41210, error/t41212)
		stubConstDef->cloneTypeDescriptorForPendingArgumentNode(paramConstDef);

		m_state.popClassContext();

		stubConstDef->fixPendingArgumentNode(); //name m_cid
		foundArgs++;
	      }
	    else
	      {
		if(firstDefaultParamUsed < 0) //first default seen
		  {
		    firstDefaultParamUsed = lastDefaultParamUsed = i;
		    m_state.pushClassContext(suti, cblock, cblock, false, NULL); //error/t41203,4
		  }

		if(i > (lastDefaultParamUsed + 1))
		  {
		    //error, must continue to be defaults after first one
		    std::ostringstream msg;
		    msg << "Arg " << i + 1 << " (of " << numparams;
		    msg << ") in class instance '";
		    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str(); //not a uti
		    msg << "' comes after the last default parameter value used (";
		    msg << lastDefaultParamUsed << ") to fix";
		    MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
		  }
		else
		  {
		    lastDefaultParamUsed = i;
		    if(!cblock->isIdInScope(pid,argsym))
		      {
			// and make a new symbol that's like the default param
			Symbol * clonesym = NULL;
			AssertBool gotclonesym = paramConstDef->cloneSymbol(clonesym);
			assert(gotclonesym);

			SymbolConstantValue * asym2 = (SymbolConstantValue *) clonesym;
			asym2->setClassArgAsDefaultValue(); //t41431
			cblock->addIdToScope(pid, asym2);

			// possible pending value for default param
			NodeConstantDef * argConstDef = (NodeConstantDef *) paramConstDef->instantiate();
			assert(argConstDef);
			//fold later; non ready expressions saved by UTI in m_nonreadyClassArgSubtrees (stub)
			argConstDef->setSymbolPtr(asym2); //since we have it handy
			csym->linkConstantExpressionForPendingArg(argConstDef);  //also adds argnode
		      }
		    foundArgs++;
		  }
	      }
	  } //all the parameters

	if(foundArgs != numparams)
	  {
	    //num arguments in class instance does not match number of parameters
	    std::ostringstream msg;
	    msg << "Number of Arguments (" << foundArgs << ") in class instance '";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str(); //not a uti
	    msg << "' did not match the required number of parameters (";
	    msg << numparams << ") to fix";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	  }

	if(firstDefaultParamUsed >= 0)
	  m_state.popClassContext(); //restore stub push for defaults

	m_state.popClassContext(); //restore current block push
	m_state.popClassContext(); //restore context push

	// class instance's prev classblock is linked to its template's when stub is made.
	// later, during c&l if a subclass, the super ptr gets the classblock of superclass
	initBaseClassListForAStubClassInstance(csym); //t41527
	//cblock->initBaseClassBlockList(); //wait for c&l when no longer a stub

	//makeAStub patches in data members after arguments fixed (t3895)
	csym->partialInstantiationOfMemberNodesAndSymbols(*templateclassblock);

	cloneAnInstancesUTImap(this, csym); //t3384,t3565??

	it++;
      } //while any more stubs
  } //fixAnyUnseenClassInstances

  //called during parsing, similar to fixAnyUnseenClassInstances for seen templates
  void SymbolClassNameTemplate::fixAClassStubsDefaultArgs(SymbolClass * stubcsym, u32 defaultstartidx)
  {
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock); //fails if UNSEEN during parsing

    assert(stubcsym);
    UTI suti = stubcsym->getUlamTypeIdx();
    NodeBlockClass * cblock = stubcsym->getClassBlockNode();
    assert(cblock);

    //BUT we need this stub as CompileThisIdx for any stub copies,
    // so, do the push and correct the resolver pendingArgs context later (t3891).
    //we don't want to push cblock context, because we want any new
    // Resolver for stubcsym to pick up the correct context.
    m_state.pushClassContext(suti, cblock, cblock, false, NULL);

    u32 numparams = getNumberOfParameters();
    assert(numparams - defaultstartidx <= getTotalParametersWithDefaultValues());

    //update the class instance's ST, and Resolver with defaults.
    for(u32 i = defaultstartidx; i < numparams; i++)
      {
	NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassblock->getParameterNode(i);
	assert(paramConstDef);
	u32 pid = paramConstDef->getSymbolId();

	SymbolConstantValue * asym2;
	Symbol * asym = NULL;
	bool hazyKin = false; //don't care
	if(!m_state.alreadyDefinedSymbol(pid, asym, hazyKin))
	  {
	    // and make a new symbol that's like the default param
	    AssertBool gotasym = paramConstDef->cloneSymbol(asym);
	    assert(gotasym);
	    asym2 = (SymbolConstantValue *) asym;
	    asym2->setBlockNoOfST(cblock->getNodeNo()); //stub NNO same as template, at this point
	    asym2->setClassArgAsDefaultValue();
	    cblock->addIdToScope(asym2->getId(), asym2);
	  }
	else
	  {
	    //already present from patched stub (t41440)
	    assert(asym->isConstant());
	    asym2 = (SymbolConstantValue *) asym;
	    asym2->setClassArgAsDefaultValue();
	  }

	// possible pending value for default param
	NodeConstantDef * argConstDef = (NodeConstantDef *) paramConstDef->instantiate();
	assert(argConstDef);
	//fold later; non ready expressions saved by UTI in m_nonreadyClassArgSubtrees (stub)
	argConstDef->setSymbolPtr(asym2); //since we have it handy, sets declnno
	stubcsym->linkConstantExpressionForPendingArg(argConstDef);
      }

    m_state.popClassContext(); //restore
    assert(stubcsym->getContextForPendingArgValues() != Nouti); //sanity or set??
  } //fixAClassStubsDefaultArgs

  void SymbolClassNameTemplate::fixAnyIncompleteClassInstances()
  {
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock);
    std::map<UTI, SymbolClass*>::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while (it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
        SymbolClass * sym = it->second;
        assert(sym);

        NodeBlockClass * cblock = sym->getClassBlockNode();
        assert(cblock->getNodeNo() == templateclassblock->getNodeNo());

        if(sym->isStubForTemplateTypeIncomplete())
          {
            // initBaseClassListForAStubClassInstance(sym);
            sym->partialInstantiationOfMemberNodesAndSymbols(*templateclassblock);
            cloneAnInstancesUTImap(this, sym);

            sym->setStubForTemplateTypeIncomplete(false);
          }

        it++;
      }
  } // fixAnyIncompleteClassInstances

  bool SymbolClassNameTemplate::statusNonreadyClassArgumentsInStubClassInstances()
  {
    //if(getUlamClass() == UC_UNSEEN) return false; //template not seen (e.g. typo) (t41435)

    bool aok = true;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI cuti = csym->getUlamTypeIdx();

	if(checkSFINAE(csym) && csym->isClassTemplate(cuti))
	  {
	    it++;
	    continue; //skip only templates (t41222)
	  }

	if(csym->isStubCopy())
	  {
	    it++;
	    continue;
	  }

	//push to Resolver to skip stubs that will never get resolved (e.g. t3787)
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);

	m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

	//pending args could depend on constants in ancestors (t3887)
	//ulam-5 supports multiple base classes; superclass optional
	u32 basecount = csym->getBaseClassCount() + 1; //include super
	u32 i = 0;
	while(i < basecount)
	  {
	    UTI baseuti = csym->getBaseClass(i);
	    if(m_state.okUTItoContinue(baseuti) && !classNode->isBaseClassBlockReady(cuti,baseuti))
	      {
		SymbolClass * basecsym = NULL;
		if(m_state.alreadyDefinedSymbolClass(baseuti, basecsym))
		  {
		    if(basecsym->isStub() || m_state.isHolder(baseuti)) //20210726 ish
		      aok = false;
		  }
	      }
	    else
	      aok = false; //? t41440 hazy
	    i++;
	  } //end while

	aok &= csym->statusNonreadyClassArguments(); //could bypass if fully instantiated

	m_state.popClassContext();
	it++;
      }
    return aok;
  } //statusNonreadyClassArgumentsInStubClassInstances

  std::string SymbolClassNameTemplate::formatAnInstancesArgValuesAsAString(UTI instance, bool dereftypes)
  {
    std::ostringstream args;

    UlamType * cut = m_state.getUlamTypeByIndex(instance);
    bool isARef = cut->isAltRefType();
    if(isARef && !dereftypes)
      args << "r"; //default is false for 2nd arg

    u32 numParams = getNumberOfParameters();
    args << ToLeximitedNumber(numParams);

    if(numParams == 0)
      return args.str();

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	std::ostringstream msg;
	msg << "Template '" << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << "' has no instances; Args format is number of parameters";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), DEBUG);
	return args.str(); //short-circuit when argument is template's UTI
      }

    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * templateclassNode = getClassBlockNode();
	assert(templateclassNode);

	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	//format values into stream
	for (u32 i = 0; i < numParams; i++)
	  {
	    bool isok = false;
	    NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassNode->getParameterNode(i);
	    assert(paramConstDef);
	    u32 pid = paramConstDef->getSymbolId();

	    Symbol * asym = NULL;
	    bool hazyKin = false; //don't care
	    AssertBool isDefined = m_state.alreadyDefinedSymbol(pid, asym, hazyKin);
	    assert(isDefined);
	    UTI auti = asym->getUlamTypeIdx();
	    if(dereftypes)
	      auti = m_state.getUlamTypeAsDeref(auti);

	    UlamType * aut = m_state.getUlamTypeByIndex(auti);

	    //append 'instance's arg (mangled) type; little 'c' for a class type parameter (t41209)
	    args << aut->UlamType::getUlamTypeMangledType().c_str();
	    assert(!aut->isAltRefType());
	    if(!aut->isScalar())
	      {
		std::string arrvalstr;
		if((isok = ((SymbolConstantValue *) asym)->getArrayValueAsString(arrvalstr)))
		  args << arrvalstr; //lex'd by array of u32's
	      }
	    else if(m_state.isAClass(auti))
	      {
		//check for this sooner! can't figure how to make a class chasing its tail.
		assert(UlamType::compare(auti, instance, m_state) != UTIC_SAME);

		std::string ccvalstr;
		if((isok = ((SymbolConstantValue *)asym)->getValueAsHexString(ccvalstr)))
		  args << ToLeximited(ccvalstr);
	      }
	    else
	      {
		//append 'instance's arg value (scalar)
		std::string argstr;
		if((isok = ((SymbolConstantValue *) asym)->getLexValue(argstr)))
		  args << argstr.c_str();
	      }

	    if(!isok)
	      {
		std::string astr = m_state.m_pool.getDataAsString(asym->getId());
		args << ToLeximited(astr);
	      }
	  } //next param

	m_state.popClassContext(); //restore
      }
    return args.str();
  } //formatAnInstancesArgValuesAsAString

  std::string SymbolClassNameTemplate::formatAnInstancesArgValuesAsCommaDelimitedString(UTI instance)
  {
    u32 numParams = getNumberOfParameters();
    if(numParams == 0)
      {
	return "";
      }

    std::ostringstream args;

    if(instance == getUlamTypeIdx())
      {
	if(m_scalarClassInstanceIdxToSymbolPtr.empty())
	  {
	    std::ostringstream msg;
	    msg << "Template '" << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	    msg << "' has no instances; Args format is number of parameters";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(), DEBUG);
	  }
	args << ToLeximitedNumber(numParams);
	return args.str(); //short-circuit when argument is template's UTI
      }

    args << "(";

    SymbolClass * csym = NULL;
    if(!findClassInstanceByUTI(instance, csym))
      {
	//check class instance idx in 'instance' UTI key;
	//templates don't always have the latest instance UTI as class parameter types;
	// (e.g. t3862 during printPostfix on Uq_Soo)
	UlamKeyTypeSignature ikey = m_state.getUlamKeyTypeSignatureByIndex(instance);
	UTI kuti = ikey.getUlamKeyTypeSignatureClassInstanceIdx();
	if(instance != kuti)
	  {
	    if(!findClassInstanceByUTI(kuti, csym))
	      {
		args << "failedtofindinstance)";
		return args.str(); //bail (t3644)
	      }
	    //else continue..
	  }
      }
    assert(csym);

    NodeBlockClass * templateclassNode = getClassBlockNode();
    assert(templateclassNode);

    NodeBlockClass * classNode = csym->getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);
    u32 n = 0;

    //format values into stream
    for (u32 i = 0; i < numParams; i++)
      {
	if(n++ > 0)
	  args << ",";

	NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassNode->getParameterNode(i);
	assert(paramConstDef);
	u32 pid = paramConstDef->getSymbolId();

	//get 'instance's value
	bool isok = false;
	Symbol * asym = NULL;
	bool hazyKin = false; //don't care
	AssertBool isDefined = m_state.alreadyDefinedSymbol(pid, asym, hazyKin);
	assert(isDefined);
	UTI auti = asym->getUlamTypeIdx();
	UlamType * aut = m_state.getUlamTypeByIndex(auti);
	if(aut->isComplete())
	  {
	    if(!aut->isScalar())
	      {
		std::string arrvalstr;
		if((isok = ((SymbolConstantValue *) asym)->getArrayValueAsString(arrvalstr)))
		  args << arrvalstr;  //lex'd array of u32's
	      }
	    else if(m_state.isAClass(auti))
	      {
		std::string ccvalstr;
		if((isok = ((SymbolConstantValue *)asym)->getValueAsHexString(ccvalstr)))
		  args << "0x" << ccvalstr; //t41209
	      }
	    else
	      {
		std::string valstr;
		if((isok = ((SymbolConstantValue *) asym)->getScalarValueAsString(valstr)))
		  args << valstr;    //pretty
	      } //isscalar
	  } //iscomplete

	if(!isok)
	  {
	    std::string astr = m_state.m_pool.getDataAsString(asym->getId());
	    args << astr.c_str();
	  }
      } //next param

    args << ")";

    m_state.popClassContext(); //restore
    return args.str();
  } //formatAnInstancesArgValuesAsCommaDelimitedString

  void SymbolClassNameTemplate::generatePrettyNameAndSignatureOfClassInstancesAsUserStrings()
  {
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI cuti = csym->getUlamTypeIdx();

	assert(cuti != getUlamTypeIdx()); //no template

	if(!csym->isStub())
	  {
	    std::string sigonly = generatePrettyNameOrSignature(cuti, true, false); //do instance signature only;
	    m_state.formatAndGetIndexForDataUserString(sigonly);

	    std::string fancysig = generatePrettyNameOrSignature(cuti, true, true); //do instance fancy signature
	    m_state.formatAndGetIndexForDataUserString(fancysig);

	    std::string simple = generatePrettyNameOrSignature(cuti, false, true); //do instance simple name
	    m_state.formatAndGetIndexForDataUserString(simple);

	    std::string plain = generatePrettyNameOrSignature(cuti, false, false); //do instance plain name
	    m_state.formatAndGetIndexForDataUserString(plain);

	  }
	else
	  {
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);

	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameByIndex(cuti).c_str();
	    msg << "' is still a stub; No pretty name error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
	it++;
      }
  } //generatePrettyNameAndSignatureOfClassInstancesAsUserStrings

  std::string SymbolClassNameTemplate::generatePrettyNameOrSignature(UTI instance, bool signa, bool argvals)
  {
    u32 pcnt = 0;
    std::ostringstream sig;

    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance,csym))
      {
	NodeBlockClass * templateclassNode = getClassBlockNode();
	assert(templateclassNode);

	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	sig << m_state.m_pool.getDataAsString(getId()).c_str(); //class name
	if(signa || argvals)
	  sig << "(";

	//format parameters type and name and value into stream
	u32 numparams = getNumberOfParameters();
	for (u32 i = 0; i < numparams; i++)
	  {
	    NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassNode->getParameterNode(i);
	    assert(paramConstDef);
	    u32 pid = paramConstDef->getSymbolId();

	    if((signa || argvals) && (pcnt > 0))
	      sig << ",";

	    //get 'instance's arg value
	    bool isok = false;
	    Symbol * asym = NULL;
	    bool hazyKin = false; //don't care
	    AssertBool isDefined = m_state.alreadyDefinedSymbol(pid, asym, hazyKin);
	    assert(isDefined);
	    UTI auti = asym->getUlamTypeIdx();
	    UlamType * aut = m_state.getUlamTypeByIndex(auti);

	    if(signa)
	      {
		sig << m_state.getUlamTypeNameBriefByIndex(asym->getUlamTypeIdx()).c_str();
		sig << " " << m_state.m_pool.getDataAsString(asym->getId()).c_str(); //param
	      }

	    if(argvals)
	      {
		if(signa)
		  sig << "=";

		if(aut->isComplete())
		  {
		    if(!aut->isScalar())
		      {
			std::string arrvalstr;
			if((isok = ((SymbolConstantValue *) asym)->getArrayValueAsString(arrvalstr)))
			  sig << arrvalstr;  //lex'd array of u32's
		      }
		    else if(m_state.isAClass(auti))
		      {
			std::string ccvalstr;
			if((isok = ((SymbolConstantValue *)asym)->getValueAsHexString(ccvalstr)))
			  sig << "0x" << ccvalstr; //t41209
		      }
		    else
		      {
			std::string valstr;
			if((isok = ((SymbolConstantValue *) asym)->getScalarValueAsString(valstr)))
			  sig << valstr;    //pretty
		      } //isscalar
		  } //iscomplete

		if(!isok)
		  {
		    sig << "BAD_VALUE";
		  }
	      } //argvals

	    pcnt++;
	  } //next param

	if(signa || argvals)
	  sig << ")";

	m_state.popClassContext(); //restore
      }
    return sig.str();
  } //generatePrettyNameOrSignature

  bool SymbolClassNameTemplate::hasInstanceMappedUTI(UTI instance, UTI auti, UTI& mappedUTI)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
	return csym->hasMappedUTI(auti, mappedUTI);
    else if(instance == getUlamTypeIdx())
      return SymbolClass::hasMappedUTI(auti, mappedUTI); // template definition
    return false;
  } //hasInstanceMappedUTI

  bool SymbolClassNameTemplate::mapInstanceUTI(UTI instance, UTI auti, UTI mappeduti)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      return csym->mapUTItoUTI(auti, mappeduti);
    return false; //excludes template definition as instance t3526
  } //mapInstanceUTI

  //new version...fully working...
  bool SymbolClassNameTemplate::fullyInstantiate()
  {
    bool aok = true; //all done

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      return true;

    UTI tuti = getUlamTypeIdx();
    NodeBlockClass * templatecblock = getClassBlockNode();
    if(!templatecblock)
      {
	std::ostringstream msg;
	msg << "Cannot fully instantiate a template class '";
	msg << m_state.getUlamTypeNameByIndex(tuti).c_str();
	msg << "' without a definition (maybe not a class at all)";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI cuti = csym->getUlamTypeIdx();

	if(checkSFINAE(csym) && csym->isClassTemplate(cuti))
	  {
	    it++;
	    continue; //skip templates only (t41222)
	  }

	if(!csym->isStub())
	  {
	    it++;
	    continue; //already done
	  }

	if(csym->isStubCopy())
	  {
	    //aok &= false;
	    it++;
	    continue; //have to wait (t3640)
	  }

	//ask stub class symbol..
	if(csym->pendingClassArgumentsForClassInstance())
	  {
	    aok &= false;
	    it++;
	    continue; //have to wait
	  }

	//have we seen these args before?
	SymbolClass * dupsym = NULL;
	if(findClassInstanceByArgString(cuti, dupsym))
	  {
	    assert(dupsym->getContextForPendingArgValues() == Nouti);
	    assert(dupsym->getStubForTemplateType() == Nouti);
	    UTI duputi = dupsym->getUlamTypeIdx();
	    u32 toomanycount = m_state.mergeClassUTI(cuti, duputi, dupsym->getLoc());
	    trashStub(duputi, csym);
	    it->second = dupsym; //duplicate! except different UTIs
	    if(toomanycount > 0)
	      {
		std::ostringstream msg;
		msg << "Circular reference or dependencies too complex: ";
		msg << toomanycount << " copies of ";
		msg << m_state.getUlamTypeNameBriefByIndex(duputi).c_str();
		msg << " (UTI " << duputi << "), so far.";
		MSG(m_state.getFullLocationAsString(dupsym->getLoc()).c_str(), msg.str().c_str(), ERR);
		outputLocationsOfTrashedStubs(toomanycount,duputi);
		aok &= false;  // error/t41455
	      }
	    it++;
	    continue;
	  }

	//check for any template ancestor issues; may create a stubcopy.
	if(!checkTemplateAncestorsAndUpdateStubBeforeAStubInstantiation(csym))
	  {
	    aok &= false;
	    it++;
	    continue; //have to wait
	  }

	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);

	classNode->setNodeLocation(this->getLoc()); //uses template's locals scope

	m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

	UTI whence = csym->getStubCopyOf();
	if(whence != Nouti)
	  {
	    assert(m_state.okUTItoContinue(whence));
	    SymbolClass * stubwhencecame = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(whence, stubwhencecame);
	    assert(isDefined);

	    assert(!stubwhencecame->isStubCopy()); //t3640
	    assert(stubwhencecame->getUlamClass() != UC_UNSEEN);

	    NodeBlockClass * whencecblock = stubwhencecame->getClassBlockNode();
	    assert(whencecblock);

	    csym->partialInstantiationOfMemberNodesAndSymbols(*whencecblock); //t41452,5 inf?
	  }

	classNode->setMemberFunctionsSymbolTable(cuti, *templatecblock);

	//set previous block pointer for function definition blocks, updating lineage
	// to this class block, and updates lineage across funcdefs
	classNode->updatePrevBlockPtrOfFuncSymbolsInTable();

	addClassInstanceByArgString(cuti, csym); //new entry, and owner of symbol class

	AssertBool verifd = verifySelfAndSuperTypedefs(cuti, csym);
        assert(verifd);

	csym->unsetStub();
	csym->clearStubForTemplate(); //useful, may not apply, as a duplicate class
	csym->setContextForPendingArgValues(Nouti);
	csym->setContextForPendingArgTypes(Nouti);

	m_state.popClassContext(); //restore
	it++;
      } //while

    return aok;
  } //fullyInstantiate

  bool SymbolClassNameTemplate::checkTemplateAncestorsAndUpdateStubBeforeAStubInstantiation(SymbolClass * stubcsym)
  {
    //original version does more than just check template ancestors, it updates stubcsym too!!
    assert(!stubcsym->isStubCopy() && !stubcsym->pendingClassArgumentsForClassInstance());

    bool rtnok = true;
    //if any of stub's base classes are still a stub..wait on full instantiation
    u32 basecount = stubcsym->getBaseClassCount() + 1; //include super
    for(u32 i = 0; i < basecount; i++)
      {
	UTI stubbaseuti = stubcsym->getBaseClass(i);
	UTI superbaseuti = SymbolClass::getBaseClass(i); //template's ancestor
	m_state.findRootUTIAlias(superbaseuti,superbaseuti); //t3567

	if((stubbaseuti == Nouti) || (stubbaseuti == Hzy))
	  {
	    //template was unseen at the time stub was made
	    if(superbaseuti == Nouti)
	      {
		stubcsym->updateBaseClass(stubbaseuti, i, Nouti); //no ancestor
		rtnok &= true;
	      }
	    else if(m_state.isUrSelf(superbaseuti))
	      {
		stubcsym->updateBaseClass(stubbaseuti, i, superbaseuti); //UrSelf is ancestor; not stub
		rtnok &= true;
	      }
	    else if(superbaseuti == Hzy) //only UNSEEN Templates
	      {
		rtnok &= false;
	      }
	    else if(!m_state.isASeenClass(superbaseuti))
	      {
		rtnok &= false;
	      }
	    else if(!m_state.isClassAStub(superbaseuti))
	      {
		stubcsym->updateBaseClass(stubbaseuti, i, superbaseuti); //not a stub; t3567, 3573, t3574, t3575
		rtnok &= true;
	      }
	    else
	      {
		UTI stubuti = stubcsym->getUlamTypeIdx();
		bool gotbase = false;
		if(i==0)
		  {
		    NodeBlockClass * stubcblock = stubcsym->getClassBlockNode();
		    assert(stubcblock);
		    m_state.pushClassContext(stubuti, stubcblock, stubcblock, false, NULL);

		    //try not to duplicate the baseuti for superclass (t41228?), special case.
		    u32 superid = m_state.m_pool.getIndexForDataString("Super");
		    UTI supertdef = Nouti;
		    UTI scalarsupertdef = Nouti;
		    if(m_state.getUlamTypeByTypedefName(superid, supertdef, scalarsupertdef) == TBOOL_TRUE)
		      {
			if(m_state.okUTItoContinue(supertdef)) //not if Hzy (t3642)
			  {
			    UTI alias = scalarsupertdef;
			    m_state.findRootUTIAlias(scalarsupertdef, alias); //t41228
			    stubcsym->updateBaseClass(stubbaseuti, i, alias); //stubcopy's superbase set here!!
			    rtnok &= true;
			    gotbase = true;
			  }
		      }
		    m_state.popClassContext(); //restore
		  }

		if(!gotbase)
		  {
		    //if superbaseuti is a stub of this template; possible un-ending (MAX_ITERATIONS)
		    // increase in size of m_scalarClassInstanceIdxToSymbolPtr each time we're called;
		    // never resolving; should be caught at parse time (t3901)
		    assert(m_state.getUlamTypeByIndex(superbaseuti)->getUlamTypeNameId() != getId());
		    UTI newstubbaseuti = m_state.addStubCopyToAncestorClassTemplate(superbaseuti, stubuti, stubuti, stubcsym->getLoc()); //t41431, t3640,1
		    stubcsym->updateBaseClass(stubbaseuti, i, newstubbaseuti); //stubcopy's type set here!!
		    rtnok &= false;
		  }
	      }
	  }
	else //neither nouti or hzy, waiting for fullinstiated/regular baseclasses
	  rtnok &= !m_state.isClassAStub(stubbaseuti);

      } //for loop of bases

    return rtnok;
  } //checkTemplateAncestorsAndUpdateStubBeforeAStubInstantiation

  bool SymbolClassNameTemplate::verifySelfAndSuperTypedefs(UTI cuti, SymbolClass * csym)
  {
    //internal processing, not caused by user; no error messages.
    bool aok = true;
    u32 selftypeid = m_state.m_pool.getIndexForDataString("Self");
    UTI selftdef = Nouti;
    UTI scalarselftdef = Nouti;
    aok = ((m_state.getUlamTypeByTypedefName(selftypeid, selftdef, scalarselftdef) == TBOOL_TRUE) && (selftdef == cuti));
    UTI superuti = csym->getBaseClass(0);
    u32 supertypeid = m_state.m_pool.getIndexForDataString("Super");
    UTI supertdef = Nouti;
    UTI scalarsupertdef = Nouti;
    aok &= ((m_state.getUlamTypeByTypedefName(supertypeid, supertdef, scalarsupertdef) == TBOOL_TRUE) && (supertdef == superuti));
    return aok;
  } //verifySelfAndSuperTypedefs

  void SymbolClassNameTemplate::mergeClassInstancesFromTEMP()
  {
    //quickly!! without making any new stubcopies. upgrade later during c&l for stubcopy.
    if(!m_scalarClassInstanceIdxToSymbolPtrTEMP.empty())
      {
	std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtrTEMP.begin();
	while(it != m_scalarClassInstanceIdxToSymbolPtrTEMP.end())
	  {
	    UTI cuti = it->first;
	    SymbolClass * csym = it->second;
	    addClassInstanceUTI(cuti, csym); //still stubcopy, but technically "defined"
	    it++;
	  }
	m_scalarClassInstanceIdxToSymbolPtrTEMP.clear();
      }
  }

  void SymbolClassNameTemplate::upgradeStubCopyToAStubClassInstance(UTI suti, SymbolClass * csym)
  {
    assert(csym->getId()==getId());

    //more stub copying possible during this upgrade process;
    //inserts into TEMP map may happen.
    //pushed context suti by c&l caller

    if(getUlamClass() == UC_UNSEEN)
      return; //wait since template is unseen..

    UTI cuti = getUlamTypeIdx();
    AssertBool unseenstubcopy = (csym->getUlamClass() == UC_UNSEEN);

    NodeBlockClass * templatecblock = getClassBlockNode();
    assert(templatecblock);
    NodeBlockClass * cblock = csym->getClassBlockNode();
    assert(cblock);

    if(unseenstubcopy)
      {
	bool isCATemplate = ((UlamTypeClass *) m_state.getUlamTypeByIndex(cuti))->isCustomArray();
	UlamKeyTypeSignature skey = m_state.getUlamTypeByIndex(suti)->getUlamKeyTypeSignature();
	AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(skey, Class, getUlamClass(), isCATemplate); //suti remains the same
	assert(isReplaced);
	assert(csym->getUlamClass() != UC_UNSEEN); //t41209

	//should this be done again? now that template is seen?
	initBaseClassListForAStubClassInstance(csym);
	//cblock->initBaseClassBlockList(); //wait for c&l when no longer a stub
      }

    //wait for merge to clone arg nodes (t41361?); and reassign argtypecontxt to self
    UTI argvaluecontext = csym->getContextForPendingArgValues();
    UTI argtypecontext = csym->getContextForPendingArgTypes();

    //what stub are we copying?? see copyAStubClassInstance..
    UTI whence = csym->getStubCopyOf();
    assert(m_state.okUTItoContinue(whence));
    SymbolClass * stubwhencecame = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(whence, stubwhencecame);
    assert(isDefined);

    assert(!stubwhencecame->isStubCopy()); //t3640
    assert(stubwhencecame->getUlamClass() != UC_UNSEEN);

    //let's patch in the symbols before attempting the arguments? t41228??
    NodeBlockClass * whencecblock = stubwhencecame->getClassBlockNode();
    assert(whencecblock);

    whencecblock->copyUlamTypeKeys(cblock); //t3895, maybe
    //cloneAnInstancesUTImap(stubwhencecame, csym); //nicer postfix answers (t3764,5,6..)
    stubwhencecame->cloneResolverUTImap(csym); //t3383,t3392,4,5..

    copyAnInstancesArgValues(stubwhencecame, csym);

    csym->cloneArgumentNodesForClassInstance(stubwhencecame, argvaluecontext, argtypecontext); //not in data member parse tree

    //upgrade only once: has arguments only, no members, no funcs;
    // but still a stub (w stubof)
    csym->unsetStubCopy();
  } //upgradeStubCopyToAStubClassInstance

  Node * SymbolClassNameTemplate::findNodeNoInAClassInstance(UTI instance, NNO n)
  {
    Node * foundNode = NULL;
    if(getUlamTypeIdx() == instance)
      {
	return SymbolClassName::findNodeNoInAClassInstance(instance, n);
      }

    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	classNode->findNodeNo(n, foundNode); //classblock node no IS the same across instances
	m_state.popClassContext(); //restore
      }
    return foundNode;
  } //findNodeNoInAClassInstance

  void SymbolClassNameTemplate::checkCustomArraysOfClassInstances()
  {
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	UTI cuti = csym->getUlamTypeIdx();
	if(!csym->isStub())
	  {
	    m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

	    classNode->checkCustomArrayTypeFunctions(cuti); //do each instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameByIndex(cuti).c_str();
	    msg << "' is still a stub; No check for custom arrays error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
	it++;
      }
  } //checkCustomArraysOfClassInstances()

  void SymbolClassNameTemplate::checkDuplicateFunctionsForClassInstances()
  {
    // only need to check the unique class instances that have been deeply copied
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	if(!csym->isStub())
	  {
	    m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	    classNode->checkDuplicateFunctionsInClassAndAncestors(); //do each instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameByIndex(csym->getUlamTypeIdx()).c_str();
	    msg << "' is still a stub; No check for duplication function error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
	it++;
      }
  } //checkDuplicateFunctionsForClassInstances

  void SymbolClassNameTemplate::calcMaxDepthOfFunctionsForClassInstances()
  {
    // only need to check the unique class instances that have been deeply copied
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI cuti = csym->getUlamTypeIdx();
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	if(!csym->isStub())
	  {
	    if(m_state.isComplete(cuti))
	      {
		m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

		classNode->calcMaxDepthOfFunctions(); //do each instance
		m_state.popClassContext(); //restore
	      }
	    else
	      {
		std::ostringstream msg;
		msg << " Class instance '";
		msg << m_state.getUlamTypeNameByIndex(cuti).c_str();
		msg << "' is still incomplete; No calc max depth function error";
		MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameByIndex(cuti).c_str();
	    msg << "' is still a stub; No calc max depth function, error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
	it++;
      }
  } //calcMaxDepthOfFunctionsForClassInstances


  bool SymbolClassNameTemplate::calcMaxIndexOfVirtualFunctionsForClassInstances()
  {
    bool aok = true;
    // only need to check the unique class instances that have been deeply copied
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	if(checkSFINAE(csym))
	  {
	    it++;
	    continue; //skip templates and template member stubs
	  }

	UTI cuti = csym->getUlamTypeIdx();
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	if(!csym->isStub())
	  {
	    if(m_state.isComplete(cuti))
	      {
		m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

		classNode->calcMaxIndexOfVirtualFunctions(); //do each instance
		m_state.popClassContext(); //restore
		aok &= (classNode->getVirtualMethodMaxIdx() != UNKNOWNSIZE);
	      }
	    else
	      {
		std::ostringstream msg;
		msg << " Class instance '";
		msg << m_state.getUlamTypeNameByIndex(cuti).c_str();
		msg << "' is still incomplete; No calc max index of virtual functions, error";
		MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		//aok = false;
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameByIndex(cuti).c_str();
	    msg << "' is still a stub; No calc max index of virtual functions, error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    //aok = false;
	  }
	it++;
      }
    return aok;
  } //calcMaxIndexOfVirtualFunctionsForClassInstances

  void SymbolClassNameTemplate::checkAbstractInstanceErrorsForClassInstances()
  {
    // only need to check the unique class instances that have been deeply copied
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	if(!csym->isStub())
	  {
	    csym->checkAbstractClassError(); //outputs error t41664 (missing)

	    m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	    classNode->checkAbstractInstanceErrors(); //do each instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameByIndex(csym->getUlamTypeIdx()).c_str();
	    msg << "' is still a stub; No checking of abstract instance errors ppossible";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
	it++;
      }
    return;
  } //checkAbstractInstanceErrorsForClassInstances()

  void SymbolClassNameTemplate::checkAndLabelClassInstances()
  {
    //template data members, constants, typedefs not c&l here (t41432)
    assert(getUlamClass() != UC_UNSEEN);

    std::map<UTI, SymbolClass* >::iterator stubit = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(stubit != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = stubit->second;
	if(csym->isStub() && !csym->isStubCopy())
	  {
	    UTI stubuti = csym->getUlamTypeIdx();

	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);

	    Locator saveStubLoc = classNode->getNodeLocation();
	    //temporarily change stub loc, in case of local filescope, incl arg/params
	    classNode->setNodeLocation(this->getLoc());

	    m_state.pushClassContext(stubuti, classNode, classNode, false, NULL);

	    //no need to stub checkArguments (errors: t3444,t3520,t41204,t41454) overkill!
	    classNode->checkAndLabelType(NULL); //do each stub instance

	    m_state.popClassContext(); //restore

	    classNode->setNodeLocation(saveStubLoc); //restore until fully instantiated
	  }
	stubit++;
      }

    // only need to c&l the unique class instances that have been deeply copied
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	UTI cuti = csym->getUlamTypeIdx();
	if(!csym->isStub())
	  {
	    m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

	    classNode->checkArgumentNodeTypes(); //checks unsupported types (t3894,t3895,t3898),t41324

	    classNode->checkAndLabelType(NULL); //do each instance


	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << "' is still a stub; Check and label error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	it++;
      }

    //LASTLY, upgrade stub copies to stub status..c&l next time round
    mergeClassInstancesFromTEMP(); //new here! (t41436, t41440?)

    std::map<UTI, SymbolClass* >::iterator stubcpit = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(stubcpit != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = stubcpit->second;
	if(csym->isStubCopy())
	  {
	    UTI stubuti = csym->getUlamTypeIdx();

	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);

	    m_state.pushClassContext(stubuti, classNode, classNode, false, NULL);

	    upgradeStubCopyToAStubClassInstance(stubuti, csym);

	    m_state.popClassContext(); //restore
	  }
	stubcpit++;
      }
  } //checkAndLabelClassInstances

  void SymbolClassNameTemplate::countNavNodesInClassInstances(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    // only full instances need to be counted, UNLESS there's an error situation
    // and we bailed out of the resolving loop, so do them all!
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();

    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	u32 navclasscnt = ncnt;
	u32 hzyclasscnt = hcnt;
	u32 unsetclasscnt = nocnt;

	SymbolClass * csym = it->second;
	//skip templates and template member stubs that will never get resolved (SFINAE)
	if(checkSFINAE(csym))
	  {
	    it++;
	    continue;
	  }

	if(csym->isStub()) //also stubcopies
	  {
	    it++;
	    continue;
	  }

	//check incomplete's too as they might have produced error msgs.
	UTI suti = csym->getUlamTypeIdx(); //this instance
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(suti, classNode, classNode, false, NULL);

	classNode->countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //do each instance
	if((ncnt - navclasscnt) > 0)
	  {
	    std::ostringstream msg;
	    msg << (ncnt - navclasscnt);
	    msg << " data member nodes with erroneous types remain in class instance '";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << "'";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
	  }

	if((hcnt - hzyclasscnt) > 0)
	  {
	    std::ostringstream msg;
	    msg << (hcnt - hzyclasscnt);
	    msg << " data member nodes with unresolved types remain in class instance '";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << "'";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
	  }

	if((nocnt - unsetclasscnt) > 0)
	  {
	    std::ostringstream msg;
	    msg << (nocnt - unsetclasscnt);
	    msg << " data member nodes with unset types remain in class instance '";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << "'";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
	  }
	m_state.popClassContext(); //restore
	it++;
      }

    //don't count the template since, it is no longer c&l after the first,
    //so those errors no longer count at the end of resolving loop
    return;
  } //countNavNodesInClassInstances

  bool SymbolClassNameTemplate::statusUnknownTypeInClassInstances(UTI huti)
  {
    bool aok = true;

    // only full instances need to be counted, UNLESS there's an error situation
    // and we bailed out of the resolving loop, so do them all!
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();

    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx();

	if(checkSFINAE(csym))
	  {
	    it++;
	    continue; //skips templates and template member stubs
	  }

	if(csym->isStubCopy())
	  {
	    it++;
	    continue; //wait until a stub..
	  }

	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(suti, classNode, classNode, false, NULL);

	aok &= csym->statusUnknownTypeInClass(huti);

	m_state.popClassContext(); //restore

	it++;
      }
    return aok;
  } //statusUnknownTypeInClassInstances

  bool SymbolClassNameTemplate::statusUnknownTypeNamesInClassInstances()
  {
    bool aok = true;
    bool aoktemplate = true;

    //use template results for remaining stubs (t3565, error t3444)
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    aoktemplate = SymbolClass::statusUnknownTypeNamesInClass();

    m_state.popClassContext(); //restore

    // only full instances need to be counted, UNLESS there's an error situation
    // and we bailed out of the resolving loop, so do them all!
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();

    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx();

	if(checkSFINAE(csym))
	  {
	    it++;
	    continue; //skip templates and template member stubs that will never get resolved
	  }

	if(csym->isStubCopy())
	  {
	    it++;
	    continue; //skip templates and stubcopies that will never get resolved
	  }

	if(csym->isStub())
	  aok &= aoktemplate; //use template
	else
	  {
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(suti, classNode, classNode, false, NULL);

	    aok &= csym->statusUnknownTypeNamesInClass();

	    m_state.popClassContext(); //restore
	  }
	it++;
      }
    return aok;
  } //statusUnknownTypeNamesInClassInstances

  u32 SymbolClassNameTemplate::reportUnknownTypeNamesInClassInstances()
  {
    // only full instances need to be reported, unless there are none.
    if(m_scalarClassArgStringsToSymbolPtr.empty())
      {
	return SymbolClass::reportUnknownTypeNamesInClass();
      }

    u32 totalcnt = 0;
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();

    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	totalcnt += csym->reportUnknownTypeNamesInClass();
	it++;
      }
    return totalcnt;
  } //reportUnknownTypeNamesInClassInstances

  u32 SymbolClassNameTemplate::reportClassInstanceNamesThatAreTooLong()
  {
    // only full instances need to be reported, unless there are none.
    if(m_scalarClassArgStringsToSymbolPtr.empty())
      {
	return 0;
      }

    u32 totalcnt = 0;
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();

    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(csym);
	if(csym->reportLongClassName())
	  totalcnt++;
	it++;
      }
    return totalcnt;
  } //reportClassInstanceNamesThatAreTooLong

  bool SymbolClassNameTemplate::setBitSizeOfClassInstances()
  {
    bool aok = true;
    std::vector<UTI> lostClasses;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();

    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI cuti = csym->getUlamTypeIdx(); //this instance
	UTI uti = it->first; //this instance entry; may not match Symbol class' uti
	s32 totalbits = 0;
	s32 sharedbits = UNKNOWNSIZE;
	s32 basebits = 0; //overstated, no sharing
	s32 mybits = 0; //main goal of trySetBitsize..

	if(checkSFINAE(csym))
	  {
	    it++;
	    continue; //skip templates and template member stubs
	  }

	if(m_state.isComplete(uti))
	  {
	    it++;
	    continue; //already set
	  }

	if(csym->isStubCopy())
	  {
	    it++;
	    continue;
	  }

	if(csym->pendingClassArgumentsForClassInstance() || csym->isStub())
	  {
	    aok = false;
	  }
	else
	  {
	    //push context for location in any error message e.g. t41418
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

	    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

	    if(cut->isComplete()) //uti != cuti
	      {
		totalbits = cut->getBitSize();
		mybits = m_state.getBaseClassBitSize(cuti); // Element is noop
	      }
	    else
	      {

		std::set<UTI> seenset;
		seenset.insert(cuti);
		aok = csym->trySetBitsizeWithUTIValues(basebits, mybits, seenset);

		if(aok)
		  {
		    s32 sharedbitssaved = UNKNOWNSIZE;
		    aok = csym->determineSharedBasesAndTotalBitsize(sharedbitssaved, sharedbits);
		    if(aok)
		      {
			assert(sharedbits >= 0);
			assert(sharedbitssaved >= sharedbits);
			totalbits = (mybits + sharedbits); //updates total here!!
		      }
		  }
	      }

	    if(aok)
	      {
		m_state.setUTIBitSize(uti, totalbits); //"scalar" Class bitsize  KEY ADJUSTED
		//after setBitSize so not to clobber it.
		m_state.setBaseClassBitSize(uti, mybits); //noop for elements
		m_state.updateUTIAliasForced(uti, cuti); //redo alias (t3652, t41384)
	      }
	    m_state.popClassContext(); //restore
	  }

	if(aok)
	  {
	    if(m_state.getBitSize(uti) != totalbits)
	      {
		std::ostringstream msg;
		msg << "CLASS INSTANCE '" << m_state.getUlamTypeNameBriefByIndex(uti).c_str();
		msg << "' SIZED " << totalbits << " FAILED";
		MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
		NodeBlockClass * classNode = csym->getClassBlockNode();
		assert(classNode);
		classNode->setNodeType(Nav); //avoid assert in resolving loop
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "CLASS INSTANCE '" << m_state.getUlamTypeNameByIndex(uti).c_str();
		msg << "' UTI" << uti << ", SIZED: " << totalbits;
		MSG(Symbol::getTokPtr(), msg.str().c_str(), DEBUG);
	      }
	  }
	else
	  lostClasses.push_back(uti); //track classes that fail to be sized.

	aok = true; //reset for next class
	it++;
      } //next class instance

    aok = lostClasses.empty();

    if(!aok)
      {
	std::ostringstream msg;
	msg << lostClasses.size() << " Class Instances without sizes";
	while(!lostClasses.empty())
	  {
	    UTI lcuti = lostClasses.back();
	    msg << ", " << m_state.getUlamTypeNameBriefByIndex(lcuti).c_str();
	    msg << " (UTI" << lcuti << ")";
	    lostClasses.pop_back();
	  }
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }
    else
      {
	std::ostringstream msg;
	msg << m_scalarClassInstanceIdxToSymbolPtr.size() << " Class Instance";
	msg << (m_scalarClassInstanceIdxToSymbolPtr.size() > 1 ? "s ALL " : " ");
	msg << "sized SUCCESSFULLY for template '";
	msg << m_state.m_pool.getDataAsString(getId()).c_str() << "'";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), DEBUG);
      }
    lostClasses.clear();
    return aok;
  } //setBitSizeOfClassInstances

  // separate pass...after labeling all classes is completed;
  void SymbolClassNameTemplate::printBitSizeOfClassInstances()
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->printBitSizeOfClass(); //this instance
	it++;
      }
  } //printBitSizeOfClassInstances

  void SymbolClassNameTemplate::packBitsForClassInstances()
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	if(m_state.isComplete(suti))
	  {
	    csym->packBitsForClassVariableDataMembers(); //returns TBOOL
	  }
	it++;
      }
  } //packBitsForClassInstances

  void SymbolClassNameTemplate::printUnresolvedVariablesForClassInstances()
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	if(m_state.isComplete(suti))
	  {
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(suti, classNode, classNode, false, NULL);
	    classNode->printUnresolvedVariableDataMembers(); //this instance
	    m_state.popClassContext();
	  }
	it++;
      }
  } //printUnresolvedVariablesForClassInstances

  void SymbolClassNameTemplate::buildDefaultValueForClassInstances()
  {
    BV8K dval; //tmp
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	if(m_state.isComplete(suti))
	  {
	    csym->getDefaultValue(dval); //this instance
	  }
	it++;
      }
  } //buildDefaultValueForClassInstances

  void SymbolClassNameTemplate::testForClassInstances(File * fp)
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(csym);
	csym->testThisClass(fp); //this instance
	it++;
      }
  } //testForClassInstances

  void SymbolClassNameTemplate::assignRegistrationNumberForClassInstances()
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(!csym->isStub());
	UTI suti = csym->getUlamTypeIdx();
	if(m_state.isComplete(suti))
	  {
	    u32 n = csym->getRegistryNumber();
	    assert(n != UNINITTED_REGISTRY_NUMBER); //sanity
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Class Instance '" << m_state.getUlamTypeNameByIndex(suti).c_str();
	    msg << "' is incomplete; Registry Number will not be assigned";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(), DEBUG);
	  }
	it++;
      }
  } //assignRegistrationNumberForClassInstances

  void SymbolClassNameTemplate::generateCodeForClassInstances(FileManager * fm)
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(!csym->isStub());
	UTI suti = csym->getUlamTypeIdx();
	if(m_state.isComplete(suti))
	  {
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(suti, classNode, classNode, false, NULL);

	    csym->generateCode(fm); //this instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Class Instance '" << m_state.getUlamTypeNameByIndex(suti).c_str();
	    msg << "' is incomplete; Code will not be generated";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(), DEBUG);
	  }
	it++;
      }
  } //generateCodeForClassInstances

  void SymbolClassNameTemplate::generateIncludesForClassInstances(File * fp)
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->generateAsOtherInclude(fp);
	it++;
      }
  } //generateIncludesForClassInstances

  void SymbolClassNameTemplate::generateAllIncludesForTestMainForClassInstances(File * fp)
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->generateAllIncludesForTestMain(fp);
	it++;
      }
  } //generateAllIncludesForTestMainForClassInstances

  void SymbolClassNameTemplate::generateForwardDefsForClassInstances(File * fp)
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->generateAsOtherForwardDef(fp);
	it++;
      }
  } //generateForwardDefsForClassInstances

  void SymbolClassNameTemplate::generateTestInstanceForClassInstances(File * fp, bool runtest)
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(csym);
	csym->generateTestInstance(fp, runtest);
	it++;
      }
  } //generateTestInstanceForClassInstances

  bool SymbolClassNameTemplate::takeAnInstancesArgValues(SymbolClass * fm, SymbolClass * to)
  {
    NodeBlockClass * fmclassblock = fm->getClassBlockNode();
    assert(fmclassblock);
    u32 cargs = fmclassblock->getNumberOfPotentialClassArgumentSymbols();
    u32 numparams = getNumberOfParameters();
    u32 numDefaultParams = getTotalParametersWithDefaultValues();
    if((cargs < numparams) && ((cargs + numDefaultParams) < numparams))
      {
	//error! number of arguments in stub does not match the number of parameters
	std::ostringstream msg;
	msg << "Number of Arguments (" << cargs << ") in class instance '";
	msg << m_state.m_pool.getDataAsString(fm->getId()).c_str(); //not a uti
	msg << "' is insufficient for the required number of template parameters (";
	msg << numparams << "); Not fully instantiated";
	MSG(fm->getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    if((cargs > numparams))
      {
	//number of arguments in class instance does not match the number of parameters
	// including those with default values (u.1.2.1)
	std::ostringstream msg;
	msg << "Number of Arguments (" << cargs << ") in class instance '";
	msg << m_state.m_pool.getDataAsString(fm->getId()).c_str(); //not a uti
	msg << "' is beyond the required number of template parameters (";
	msg << numparams << "); Not fully instantiated";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    NodeBlockClass * templateclassNode = getClassBlockNode();
    assert(templateclassNode);

    m_state.pushClassContext(fm->getUlamTypeIdx(), fmclassblock, fmclassblock, false, NULL);
    std::vector<SymbolConstantValue *> instancesArgs;

    //copy values from stub into temp list
    for (u32 i = 0; i < numparams; i++)
      {
	NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassNode->getParameterNode(i);
	assert(paramConstDef);
	u32 pid = paramConstDef->getSymbolId();

	//save 'instance's arg constant symbols in a temporary list
	Symbol * asym = NULL;
	m_state.takeSymbolFromCurrentScope(pid, asym); //ownership transferred to temp list; NULL if using default value
	instancesArgs.push_back((SymbolConstantValue *) asym);
      } //next param

    m_state.popClassContext(); //restore

    NodeBlockClass * toclassblock = to->getClassBlockNode();
    m_state.pushClassContext(to->getUlamTypeIdx(), toclassblock, toclassblock, false, NULL);

    //replace the clone's arg symbols
    for(u32 i = 0; i < numparams; i++)
      {
	SymbolConstantValue * asym = instancesArgs[i]; //asym might be null if default used

	NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassNode->getParameterNode(i);
	assert(paramConstDef);
	u32 aid = paramConstDef->getSymbolId();

	Symbol * clonesym = NULL;
	bool hazyKin = false; //don't care
	AssertBool isDefined = m_state.alreadyDefinedSymbol(aid, clonesym, hazyKin);
	assert(isDefined);
	if(asym)
	  m_state.replaceSymbolInCurrentScope(clonesym, asym); //deletes old, adds new
	//else already there! no need to add dup.
      } //next arg

    instancesArgs.clear(); //don't delete the symbols
    m_state.popClassContext(); //restore
    return true;
  } //takeAnInstancesArgValues

  bool SymbolClassNameTemplate::copyAnInstancesArgValues(SymbolClass * fm, SymbolClass * to)
  {
    NodeBlockClass * fmclassblock = fm->getClassBlockNode();
    assert(fmclassblock);
    // (don't care about inherited symbols for class args, so use NodeBlock)
    u32 cargs = fmclassblock->getNumberOfArgumentNodes();
    u32 numparams = getNumberOfParameters();
    u32 numDefaultParams = getTotalParametersWithDefaultValues();
    if((cargs < numparams) && ((cargs + numDefaultParams) < numparams))
      {
	//error! number of arguments in stub does not match the number of parameters
	std::ostringstream msg;
	msg << "Number of Arguments (" << cargs << ") in class instance stub '";
	msg << m_state.m_pool.getDataAsString(fm->getId()).c_str(); //not a uti
	msg << "' is insufficient for the required number of parameters (" << numparams;
	msg << ") to be copied from";
	MSG(fm->getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    m_state.pushClassContext(fm->getUlamTypeIdx(), fmclassblock, fmclassblock, false, NULL);
    std::vector<SymbolConstantValue *> instancesArgs;

    //copy values from stub into temp list;
    //can't use parametersymbol if template still unseen (t3895)
    bool ctUnseen = (getUlamClass() == UC_UNSEEN);
    for(u32 fmidx = 0; fmidx < cargs; fmidx++)
      {
	u32 snameid = 0;
	if(!ctUnseen)
	  {
	    NodeConstantDef * argNode = (NodeConstantDef *) (fmclassblock->getArgumentNode(fmidx)); //t3641??
	    assert(argNode);
	    snameid = argNode->getSymbolId();
	  }
	else
	  {
	    std::ostringstream sname;
	    sname << "_" << fmidx;
	    snameid = m_state.m_pool.getIndexForDataString(sname.str());
	  }

	//save 'instance's arg constant symbols in a temporary list
	Symbol * argSym = NULL;
	bool hazyKin = false; //don't care
	AssertBool isDefined = m_state.alreadyDefinedSymbol(snameid, argSym, hazyKin); //no ownership change;
	assert(isDefined);
	instancesArgs.push_back((SymbolConstantValue *) argSym); //for reference only
      }

    m_state.popClassContext(); //restore

    //copy the "ready" values from first the stub (fm)
    NodeBlockClass * toclassblock = to->getClassBlockNode();
    m_state.pushClassContext(to->getUlamTypeIdx(), toclassblock, toclassblock, false, NULL);

    //make replicas for the clone's arg symbols in its ST; change blockNo.
    for(u32 i = 0; i < cargs; i++)
      {
	SymbolConstantValue * asym = instancesArgs[i];
	SymbolConstantValue * asym2 = new SymbolConstantValue(*asym); //don't keep type!! (t41227)
	m_state.addSymbolToCurrentScope(asym2);
      } //next arg

    m_state.popClassContext(); //restore

    instancesArgs.clear(); //don't delete the symbols
    return true;
  } //copyAnInstancesArgValues

  // done promptly after the full instantiation
  void SymbolClassNameTemplate::cloneAnInstancesUTImap(SymbolClass * fm, SymbolClass * to)
  {
    fm->cloneResolverUTImap(to); //for non-classes only
    if(fm != this)
      fm->cloneUnknownTypesMapInClass(to); //from the stub to the clone.
    //any additional from the template? may have duplicates with stub (not added). t41481.
    //SymbolClass::cloneUnknownTypesMapInClass(to); //from the template; after UTImap.
  }

  bool SymbolClassNameTemplate::checkSFINAE(SymbolClass * sym)
  {
    //"Substitution Error Is Not A Failure"
    //bypass if template or with context of template (stub)
    // or "unseen" template (e.g. typo stub use) 20210328 ish 035039
    UTI suti = sym->getUlamTypeIdx();
    return ((suti == getUlamTypeIdx()) || (sym->isStub() && m_state.isClassAMemberStubInATemplate(suti)));
  }

  // No wait to safely delete the stub
  void SymbolClassNameTemplate::trashStub(UTI duputi, SymbolClass * symptr)
  {
    std::map<UTI, std::map<Locator, u32>, less_than_loc>::iterator it = m_locStubsDeleted.find(duputi);
    if(it != m_locStubsDeleted.end())
      {
	assert(duputi == it->first);
	std::map<Locator, u32> * locmap = &it->second;
	std::map<Locator, u32>::iterator mit = locmap->find(symptr->getLoc());
	if(mit != locmap->end())
	  {
	    u32 loccount = mit->second;
	    mit->second = loccount + 1;
	  }
	else
	  {
	   it->second.insert(std::pair<Locator, u32> (symptr->getLoc(), 1));
	  }
      }
    else
      {
	std::map<Locator, u32> locmap;
	locmap.insert(std::pair<Locator, u32>(symptr->getLoc(), 1));
	m_locStubsDeleted.insert(std::pair<UTI, std::map<Locator,u32> >(duputi, locmap));
      }
    delete symptr; //t41433
  }

  void SymbolClassNameTemplate::outputLocationsOfTrashedStubs(u32 toomany, UTI dupi)
  {
    u32 count = 0;
    std::ostringstream msg;
    //"Too many (" << toomany << ") copies of " <<  m_state.getUlamTypeNameBriefByIndex(dupi)
    std::map<UTI, std::map<Locator, u32>, less_than_loc>::iterator it = m_locStubsDeleted.find(dupi);
    if(it != m_locStubsDeleted.end())
      {
	assert(dupi == it->first);
	std::map<Locator,u32> locmap = it->second;
	msg << ".. " << locmap.size() << " locations responsible for the " << toomany << " copies: ";

	std::map<Locator,u32>::iterator sit = locmap.begin();
	while(sit != locmap.end())
	  {
	    Locator loc = sit->first;
	    u32 numloc = sit->second;
	    if(count > 0)
	      msg << ", ";
	    msg << m_state.getFullLocationAsString(loc).c_str();
	    msg << " (" << numloc << ")";
	    count += numloc;
	    sit++;
	  }
	MSG(Symbol::getTokPtr(), msg.str().c_str(), NOTE);
	assert(count + 1 == toomany); //t41455
      }
    //else dupi not found
  }

} //end MFM
