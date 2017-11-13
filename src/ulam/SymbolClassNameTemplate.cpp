#include <iomanip>
#include "SymbolClassNameTemplate.h"
#include "Resolver.h"
#include "CompilerState.h"


namespace MFM {

  SymbolClassNameTemplate::SymbolClassNameTemplate(const Token& id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClassName(id, utype, classblock, state)
  {
    //setParentClassTemplate(this);
  }

  SymbolClassNameTemplate::~SymbolClassNameTemplate()
  {
    // symbols belong to  NodeBlockClass's ST; deleted there.
    m_parameterSymbols.clear();

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

    //empty trash: delete any stubs that were replaced by full class instances
    std::map<UTI, SymbolClass* >::iterator dit = m_stubsToDelete.begin();
    while(dit != m_stubsToDelete.end())
      {
	SymbolClass * dsym = dit->second;
	delete dsym;
	dit->second = NULL;
	dit++;
      }
    m_stubsToDelete.clear();
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

  void SymbolClassNameTemplate::addParameterSymbol(SymbolConstantValue * sym)
  {
    m_parameterSymbols.push_back(sym); //just a pointer; classblock owns the constdef node (& symbol)
  }

  u32 SymbolClassNameTemplate::getNumberOfParameters()
  {
    return m_parameterSymbols.size();
  }

  bool SymbolClassNameTemplate::parameterHasDefaultValue(u32 n)
  {
    // used while parsing, so c&l not called, and symbol hasDefault not yet set.
    assert(n < m_parameterSymbols.size());
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock); //fails if UNSEEN during parsing
    Node * pnode = templateclassblock->getParameterNode(n);
    assert(pnode);
    return ((NodeConstantDef *) pnode)->hasConstantExpr(); //t3526
  } //parameterHasDefaultValue

  u32 SymbolClassNameTemplate::getTotalParametersWithDefaultValues()
  {
    u32 count = 0;
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	if(parameterHasDefaultValue(i))
	  count++;
      }
    return count;
  } //getTotalParametersWithDefaultValues

  u32 SymbolClassNameTemplate::getTotalParameterSlots()
  {
    u32 totalsizes = 0;
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	//types could be incomplete, sizes unknown for template
	totalsizes += m_state.slotsNeeded(sym->getUlamTypeIdx());
      }
    return totalsizes;
  } //getTotalParameterSlots

  SymbolConstantValue * SymbolClassNameTemplate::getParameterSymbolPtr(u32 n)
  {
    assert(n < m_parameterSymbols.size());
    return m_parameterSymbols[n];
  }

  SymbolConstantValue * SymbolClassNameTemplate::findParameterSymbolByNameId(u32 pnid)
  {
    SymbolConstantValue * rtnparamsymbol = NULL;
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	if(sym->getId() == pnid)
	  {
	    rtnparamsymbol = (SymbolConstantValue *) sym;
	    break;
	  }
      }
    return rtnparamsymbol;
  } //findParameterSymbolByNameId

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
      csym->setSuperClass(superclass); //Nouti is none, not a subclass.
    else if(instance == getUlamTypeIdx())
      SymbolClass::setSuperClass(superclass); //instance is template definition
    else
      m_state.abortShouldntGetHere(); //not found???
  } //setSuperClassForClassInstance

  UTI SymbolClassNameTemplate::getSuperClassForClassInstance(UTI instance)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      return csym->getSuperClass(); //Nouti is none, not a subclass.
    else if(instance == getUlamTypeIdx())
      return SymbolClass::getSuperClass(); //instance is template definition
    return false;
  } //getSuperClassForClassInstance

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
    bool rtnpending = false;
    if((getNumberOfParameters() > 0) || (getUlamClass() == UC_UNSEEN))
      {
	SymbolClass * csym = NULL;
	AssertBool isDefined = findClassInstanceByUTI(instance, csym);
	assert(isDefined);
	rtnpending = csym->pendingClassArgumentsForClassInstance();
	if(rtnpending) assert(csym->isStub());
      }
    return rtnpending;
  } //pendingClassArgumentsForStubClassInstance

  SymbolClass * SymbolClassNameTemplate::makeAStubClassInstance(const Token& typeTok, UTI stubcuti)
  {
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock);
    bool isCATemplate = ((UlamTypeClass *) m_state.getUlamTypeByIndex(getUlamTypeIdx()))->isCustomArray();

    //previous block is template's class block, and new NNO here!
    NodeBlockClass * newblockclass = new NodeBlockClass(templateclassblock, m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(typeTok.m_locator);
    newblockclass->setNodeType(stubcuti);
    newblockclass->resetNodeNo(templateclassblock->getNodeNo()); //keep NNO consistent (new)

    Token stubTok(TOK_IDENTIFIER, typeTok.m_locator, getId());
    SymbolClass * newclassinstance = new SymbolClass(stubTok, stubcuti, newblockclass, this, m_state);
    assert(newclassinstance);
    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    //inheritance:
    UTI superuti = getSuperClass();
    if(m_state.okUTItoContinue(superuti))
      {
	if(m_state.isClassAStub(superuti))
	  {
	    //need a copy of the super stub, and its uti
	    superuti = Hzy; //wait until resolving loop.
	  }
	newclassinstance->setSuperClass(superuti);
	//any superclass block links are handled during c&l
      }
    else
      {
	ULAMCLASSTYPE tclasstype = getUlamClass();
	if(tclasstype == UC_UNSEEN)
	  newclassinstance->setSuperClass(Hzy);
      }

    if(isCATemplate)
      ((UlamTypeClass *) m_state.getUlamTypeByIndex(stubcuti))->setCustomArray();

    addClassInstanceUTI(stubcuti, newclassinstance); //link here
    return newclassinstance;
  } //makeAStubClassInstance

  //instead of a copy, let's start new
  void SymbolClassNameTemplate::copyAStubClassInstance(UTI instance, UTI newuti, UTI context)
  {
    assert((getNumberOfParameters() > 0) || (getUlamClass() == UC_UNSEEN));
    assert(instance != newuti);

    SymbolClass * csym = NULL;
    AssertBool isDefined = findClassInstanceByUTI(instance, csym);
    assert(isDefined);

    assert(csym->pendingClassArgumentsForClassInstance());
    assert(csym->isStub());
    NodeBlockClass * blockclass = csym->getClassBlockNode();
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock);
    bool isCATemplate = ((UlamTypeClass *) m_state.getUlamTypeByIndex(getUlamTypeIdx()))->isCustomArray();

    //previous block is template's class block, and new NNO here!
    NodeBlockClass * newblockclass = new NodeBlockClass(templateclassblock, m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(blockclass->getNodeLocation());

    //provides proper context for setting type (e.g. t3640)
    m_state.pushClassContext(newuti, newblockclass, newblockclass, false, NULL);

    newblockclass->setNodeType(newuti);
    newblockclass->resetNodeNo(templateclassblock->getNodeNo()); //keep NNO consistent (new)
    newblockclass->setSuperBlockPointer(NULL); //wait for c&l when no longer a stub

    Token stubTok(TOK_IDENTIFIER,csym->getLoc(), getId());

    SymbolClass * newclassinstance = new SymbolClass(stubTok, newuti, newblockclass, this, m_state);
    assert(newclassinstance);

    assert(newclassinstance->getUlamClass() == getUlamClass());

    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    if(isCATemplate)
      ((UlamTypeClass *) m_state.getUlamTypeByIndex(newuti))->setCustomArray(); //t41007


    // we are in the middle of fully instantiating (context) or parsing;
    // with known args that we want to use to resolve, if possible, these pending args:
    if(copyAnInstancesArgValues(csym, newclassinstance))
      {
	//can't addClassInstanceUTI(newuti, newclassinstance) ITERATION IN PROGRESS!!!
	m_scalarClassInstanceIdxToSymbolPtrTEMP.insert(std::pair<UTI,SymbolClass*> (newuti,newclassinstance));

	newclassinstance->cloneArgumentNodesForClassInstance(csym, context, true);
	csym->cloneResolverUTImap(newclassinstance);
	blockclass->copyUlamTypeKeys(newblockclass); //t3895, maybe
      }
    else
      {
	delete newclassinstance; //failed e.g. wrong number of args
	newclassinstance = NULL;
      }

    m_state.popClassContext(); //restore
  } //copyAStubClassInstance

  //called by parseThisClass, if wasIncomplete is parsed; temporary class arg names
  // are fixed to match the params
  void SymbolClassNameTemplate::fixAnyClassInstances()
  {
    ULAMCLASSTYPE classtype = getUlamClass();
    assert(classtype != UC_UNSEEN);

    //furthermore, this must exist by now, or else this is the wrong time to be fixing
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock);
    bool isCATemplate = ((UlamTypeClass *) m_state.getUlamTypeByIndex(getUlamTypeIdx()))->isCustomArray();

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      return;

    u32 numparams = getNumberOfParameters();
    u32 numDefaultParams = getTotalParametersWithDefaultValues();
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	if(csym->getUlamClass() != UC_UNSEEN)
	  {
	    it++; //covers synonyms
	    continue;
	  }

	UTI suti = csym->getUlamTypeIdx();
	UlamKeyTypeSignature skey = m_state.getUlamTypeByIndex(suti)->getUlamKeyTypeSignature();
	AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(skey, Class, classtype, isCATemplate);
	assert(isReplaced);

	NodeBlockClass * cblock = csym->getClassBlockNode();
	assert(cblock);

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
	    MSG(Symbol::getTokPtr(), msg.str().c_str(),ERR);
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
	    MSG(Symbol::getTokPtr(), msg.str().c_str(),ERR);
	    it++;
	    continue;
	  }

	//provides proper context for any stub copies, e.g. default parameter values (t3895)
	m_state.pushClassContext(suti, cblock, cblock, false, NULL);

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
	    if(cblock->isIdInScope(sid,argsym))
	      {
		assert(argsym->isConstant());
		((SymbolConstantValue *) argsym)->changeConstantId(sid, m_parameterSymbols[i]->getId());
		argsym->resetUlamType(m_parameterSymbols[i]->getUlamTypeIdx()); //default was Int
		cblock->replaceIdInScope(sid, m_parameterSymbols[i]->getId(), argsym);
		foundArgs++;
	      }
	    else
	      {
		if(firstDefaultParamUsed < 0)
		  firstDefaultParamUsed = lastDefaultParamUsed = i;

		if(i > (lastDefaultParamUsed + 1))
		  {
		    //error, must continue to be defaults after first one
		    std::ostringstream msg;
		    msg << "Arg " << i + 1 << " (of " << numparams;
		    msg << ") in class instance '";
		    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str(); //not a uti
		    msg << "' comes after the last default parameter value used (";
		    msg << lastDefaultParamUsed << ") to fix";
		    MSG(Symbol::getTokPtr(), msg.str().c_str(),ERR);
		  }
		else
		  {
		    lastDefaultParamUsed = i;

		    SymbolConstantValue * psym = m_parameterSymbols[i];
		    u32 pid = psym->getId();

		    if(!cblock->isIdInScope(pid,argsym))
		      {
			// and make a new symbol that's like the default param
			SymbolConstantValue * asym2 = new SymbolConstantValue(*psym);
			assert(asym2);
			cblock->addIdToScope(pid, asym2);

			// possible pending value for default param
			NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassblock->getParameterNode(i);
			assert(paramConstDef);
			NodeConstantDef * argConstDef = (NodeConstantDef *) paramConstDef->instantiate();
			assert(argConstDef);
			//fold later; non ready expressions saved by UTI in m_nonreadyClassArgSubtrees (stub)
			argConstDef->setSymbolPtr(asym2); //since we have it handy
			csym->linkConstantExpressionForPendingArg(argConstDef);
		      }
		    foundArgs++;
		  }
	      }
	  }

	if(foundArgs != numparams)
	  {
	    //num arguments in class instance does not match number of parameters
	    std::ostringstream msg;
	    msg << "Number of Arguments (" << foundArgs << ") in class instance '";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str(); //not a uti
	    msg << "' did not match the required number of parameters (";
	    msg << numparams << ") to fix";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(),ERR);
	  }

	// class instance's prev classblock is linked to its template's when stub is made.
	// later, during c&l if a subclass, the super ptr gets the classblock of superclass
	cblock->setSuperBlockPointer(NULL); //wait for c&l when no longer a stub
	m_state.popClassContext(); //restore
	it++;
      } //while
  } //fixAnyClassInstances

  void SymbolClassNameTemplate::fixAClassStubsDefaultArgs(SymbolClass * stubcsym, u32 defaultstartidx)
  {
    NodeBlockClass * templateclassblock = getClassBlockNode();
    assert(templateclassblock); //fails if UNSEEN during parsing

    assert(stubcsym);
    NodeBlockClass * cblock = stubcsym->getClassBlockNode();
    assert(cblock);

    //BUT we need this stub as CompileThisIdx for any stub copies,
    // so, do the push and correct the resolver pendingArgs context later (t3891).
    //we don't want to push cblock context, because we want any new
    // Resolver for stubcsym to pick up the correct context.
    m_state.pushClassContext(stubcsym->getUlamTypeIdx(), cblock, cblock, false, NULL);

    u32 numparams = getNumberOfParameters();
    assert(numparams - defaultstartidx <= getTotalParametersWithDefaultValues());

    //update the class instance's ST, and Resolver with defaults.
    for(u32 i = defaultstartidx; i < numparams; i++)
      {
	SymbolConstantValue * paramSym = getParameterSymbolPtr(i);
	assert(paramSym);
	// and make a new symbol that's like the default param
	SymbolConstantValue * asym2 = new SymbolConstantValue(* paramSym);
	assert(asym2);
	asym2->setBlockNoOfST(cblock->getNodeNo()); //stub NNO same as template, at this point
	cblock->addIdToScope(asym2->getId(), asym2);

	// possible pending value for default param
	NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassblock->getParameterNode(i);
	assert(paramConstDef);
	NodeConstantDef * argConstDef = (NodeConstantDef *) paramConstDef->instantiate();
	assert(argConstDef);
	//fold later; non ready expressions saved by UTI in m_nonreadyClassArgSubtrees (stub)
	argConstDef->setSymbolPtr(asym2); //since we have it handy, sets declnno
	stubcsym->linkConstantExpressionForPendingArg(argConstDef);
      }
    m_state.popClassContext(); //restore
    if(stubcsym->getContextForPendingArgs() == Nouti)
      stubcsym->setContextForPendingArgs(m_state.getCompileThisIdx()); //reset
  } //fixAClassStubsDefaultsArgs

  bool SymbolClassNameTemplate::statusNonreadyClassArgumentsInStubClassInstances()
  {
    bool aok = true;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;

	if(checkSFINAE(csym))
	  {
	    it++;
	    continue; //skip templates Mon Jun 20 10:36:42 2016
	  }

	//push to Resolver to skip stubs that will never get resolved (e.g. t3787)
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	UTI cuti = csym->getUlamTypeIdx();

	m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

	//pending args could depend on constants in ancestors (t3887)
	UTI superuti = csym->getSuperClass();
	if(m_state.okUTItoContinue(superuti) && !classNode->isSuperClassLinkReady(cuti))
	  {
	    SymbolClass * supercsym = NULL;
	    if(m_state.alreadyDefinedSymbolClass(superuti, supercsym) && !supercsym->isStub())
	      {
		classNode->setSuperBlockPointer(supercsym->getClassBlockNode());
	      }
	  }

	aok &= csym->statusNonreadyClassArguments(); //could bypass if fully instantiated

	m_state.popClassContext();
	it++;
      }
    return aok;
  } //statusNonreadyClassArgumentsInStubClassInstances

  std::string SymbolClassNameTemplate::formatAnInstancesArgValuesAsAString(UTI instance)
  {
    std::ostringstream args;

    UlamType * cut = m_state.getUlamTypeByIndex(instance);
    bool isARef = cut->isReference();
    if(isARef)
      args << "r";

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
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	//format values into stream
	std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
	while(pit != m_parameterSymbols.end())
	  {
	    bool isok = false;
	    SymbolConstantValue * psym = *pit;
	    Symbol * asym = NULL;
	    bool hazyKin = false; //don't care
	    AssertBool isDefined = m_state.alreadyDefinedSymbol(psym->getId(), asym, hazyKin);
	    assert(isDefined);
	    UTI auti = asym->getUlamTypeIdx();
	    UlamType * aut = m_state.getUlamTypeByIndex(auti);

	    //append 'instance's arg (mangled) type
	    args << aut->getUlamTypeMangledType().c_str();
	    assert(!aut->isReference());
	    if(!aut->isScalar())
	      {
		std::string arrvalstr;
		if((isok = ((SymbolConstantValue *) asym)->getArrayValueAsString(arrvalstr)))
		  args << arrvalstr; //lex'd by array of u32's
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
	    pit++;
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
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);
	u32 n = 0;
	//format values into stream
	std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
	while(pit != m_parameterSymbols.end())
	  {
	    if(n++ > 0)
	      args << ",";

	    SymbolConstantValue * psym = *pit;
	    //get 'instance's value
	    bool isok = false;
	    Symbol * asym = NULL;
	    bool hazyKin = false; //don't care
	    AssertBool isDefined = m_state.alreadyDefinedSymbol(psym->getId(), asym, hazyKin);
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
		else
		  {
		    std::string valstr;
		    if((isok = ((SymbolConstantValue *) asym)->getScalarValueAsString(valstr)))
		      args << valstr;
		  } //isscalar
	      } //iscomplete

	    if(!isok)
	      {
		std::string astr = m_state.m_pool.getDataAsString(asym->getId());
		args << astr.c_str();
	      }
	    pit++;
	  } //next param
	args << ")";

	m_state.popClassContext(); //restore
      }
    return args.str();
  } //formatAnInstancesArgValuesAsCommaDelimitedString

  std::string SymbolClassNameTemplate::generateUlamClassSignature()
  {
    std::ostringstream sig;
    sig << m_state.m_pool.getDataAsString(getId()).c_str(); //class name

    u32 numparams = getNumberOfParameters();
    assert(numparams > 0);

    sig << "(";

    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);
    //format parameters type and name into stream
    for(u32 i = 0; i < numparams; i++)
      {
	if(i > 0)
	  sig << ", ";

	Node * pnode = classNode->getParameterNode(i);
	assert(pnode);
	sig << m_state.m_pool.getDataAsString(pnode->getTypeNameId()).c_str();
	sig << " " << pnode->getName(); //arg name
      }

    sig << ")";

    m_state.popClassContext(); //restore
    return sig.str();
  } //generateUlamClassSignature

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

  bool SymbolClassNameTemplate::fullyInstantiate()
  {
    bool aok = true; //all done

    // in case of leftovers from previous resolving loop;
    // could result from a different class' instantiation.
    mergeClassInstancesFromTEMP();

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      return true;

    NodeBlockClass * templatecblock = getClassBlockNode();
    if(!templatecblock)
      {
	std::ostringstream msg;
	msg << "Cannot fully instantiate a template class '";
	msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << "' without a definition (maybe not a class at all)";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    bool isCATemplate = ((UlamTypeClass *) m_state.getUlamTypeByIndex(getUlamTypeIdx()))->isCustomArray();

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI cuti = csym->getUlamTypeIdx();

	if(checkSFINAE(csym))
	  {
	    it++;
	    continue; //skip templates Mon Jun 20 10:36:42 2016
	  }

	if(!csym->isStub())
	  {
	    it++;
	    continue; //already done
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
	    UTI duti = dupsym->getUlamTypeIdx();
	    m_state.mergeClassUTI(cuti, duti);
	    trashStub(cuti, csym);
	    it->second = dupsym; //duplicate! except different UTIs
	    it++;
	    continue;
	  }

	//check for any ancestor stubs needed
	if(!checkTemplateAncestorBeforeAStubInstantiation(csym))
	  {
	    aok &= false;
	    it++;
	    continue; //have to wait
	  }

	// first time for this cuti, and ready args!
	m_state.pushClassContext(cuti, NULL, NULL, false, NULL);
	mapInstanceUTI(cuti, getUlamTypeIdx(), cuti); //map template->instance, instead of fudging.
	SymbolClass * clone = new SymbolClass(*this); // no longer a stub!

	//at this point we have a NodeBlockClass! update the context
	//keep the template's location (for targetmap)
	NodeBlockClass * classNode = clone->getClassBlockNode();
	assert(classNode);

	m_state.popClassContext();
	m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

	//set previous block pointer for function definition blocks, as updating lineage
	// to this class block
	classNode->updatePrevBlockPtrOfFuncSymbolsInTable();

	//set super block pointer to this class block during c&l
	classNode->setSuperBlockPointer(NULL); //clear in case of stubs

	//copy any constant strings from the stub
	clone->setUserStringPoolRef(csym->getUserStringPoolRef()); //t3962

	//copy any context, where stub used
	clone->setContextForPendingArgs(csym->getContextForPendingArgs()); //t3981

	if(!takeAnInstancesArgValues(csym, clone)) //instead of keeping template's unknown values
	  {
	    aok &= false;
	    delete clone;
	  }
	else
	  {
	    clone->cloneArgumentNodesForClassInstance(csym, csym->getContextForPendingArgs(), false);
	    cloneAnInstancesUTImap(csym, clone);
	    csym->getClassBlockNode()->copyUlamTypeKeys(classNode); //t3895

	    it->second = clone; //replace with the full copy
	    trashStub(cuti, csym);

	    addClassInstanceByArgString(cuti, clone); //new entry, and owner of symbol class

	    if(isCATemplate)
	      ((UlamTypeClass *) m_state.getUlamTypeByIndex(cuti))->setCustomArray();
	  }
	m_state.popClassContext(); //restore
	it++;
      } //while

    // done with iteration; go ahead and merge any entries into the non-temp map
    //mergeClassInstancesFromTEMP(); //try at end as well for inherited stubs.
    return aok;
  } //fullyInstantiate

  bool SymbolClassNameTemplate::checkTemplateAncestorBeforeAStubInstantiation(SymbolClass * stubcsym)
  {
    bool rtnok = false;
    //if stub's superclass is still a stub..wait on full instantiation
    UTI stubsuperuti = stubcsym->getSuperClass();
    UTI superuti = SymbolClass::getSuperClass(); //template's ancestor

    if((stubsuperuti == Nouti) || (stubsuperuti == Hzy))
      {
	//template was unseen at the time stub was made
	if(superuti == Nouti)
	  {
	    stubcsym->setSuperClass(Nouti); //no ancester
	    rtnok = true;
	  }
	else if(m_state.isUrSelf(superuti))
	  {
	    stubcsym->setSuperClass(superuti); //UrSelf is ancester; not a stub
	    rtnok = true;
	  }
	else if(superuti == Hzy) //only UNSEEN Templates
	  {
	    rtnok = false;
	  }
	else if(!m_state.isASeenClass(superuti))
	  {
	    rtnok = false;
	  }
	else if(!m_state.isClassAStub(superuti))
	  {
	    stubcsym->setSuperClass(superuti); //not a stub; t3567, 3573, t3574, t3575
	    rtnok = true;
	  }
	else
	  {
	    //if superuti is a stub of this template, we have a possible un-ending (MAX_ITERATIONS)
	    // increase in the size of m_scalarClassInstanceIdxToSymbolPtr each time we're called;
	    // never resolving; should be caught at parse time (t3901)
	    stubsuperuti = m_state.addStubCopyToAncestorClassTemplate(superuti, stubcsym->getUlamTypeIdx());
	    stubcsym->setSuperClass(stubsuperuti); //stubcopy's type set here!!
	    rtnok = false;
	  }
      }
    else //neither nouti or hzy
      rtnok = !m_state.isClassAStub(stubsuperuti);

    return rtnok;
  } //checkTemplateAncestorBeforeAStubInstantiation

  void SymbolClassNameTemplate::mergeClassInstancesFromTEMP()
  {
    if(!m_scalarClassInstanceIdxToSymbolPtrTEMP.empty())
      {
	std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtrTEMP.begin();
	while(it != m_scalarClassInstanceIdxToSymbolPtrTEMP.end())
	  {
	    UTI cuti = it->first;
	    SymbolClass * csym = it->second;
	    SymbolClassNameTemplate * ctsym = NULL;
	    //possibly a different template than the one currently being instantiated
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameTemplate(csym->getId(), ctsym);
	    assert(isDefined);
	    ctsym->addClassInstanceUTI(cuti, csym); //stub
	    it++;
	  }
	m_scalarClassInstanceIdxToSymbolPtrTEMP.clear();
      } //end temp stuff
  } //mergeClassInstancesFromTEMP

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

	    classNode->checkCustomArrayTypeFunctions(); //do each instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
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

	    classNode->checkDuplicateFunctions(); //do each instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameBriefByIndex(csym->getUlamTypeIdx()).c_str();
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
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	if(!csym->isStub())
	  {
	    m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	    classNode->calcMaxDepthOfFunctions(); //do each instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameBriefByIndex(csym->getUlamTypeIdx()).c_str();
	    msg << "' is still a stub; No calc max depth function error";
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
	    continue; //skip templates Mon Jun 20 10:36:42 2016
	  }

	UTI cuti = csym->getUlamTypeIdx();
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	if(!csym->isStub())
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
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << "' is still a stub; No calc max index of virtual functions error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
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
	    m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	    classNode->checkAbstractInstanceErrors(); //do each instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameBriefByIndex(csym->getUlamTypeIdx()).c_str();
	    msg << "' is still a stub; No checking of abstract instance errors ppossible";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
	it++;
      }
    return;
  } //checkAbstractInstanceErrorsForClassInstances()

  void SymbolClassNameTemplate::checkAndLabelClassInstances()
  {
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

	    classNode->checkAndLabelType(); //do each instance

	    classNode->checkArgumentNodeTypes(); //unsupported types (t3894,t3895,t3898)

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
	//skip templates and stubs that will never get resolved (SFINAE)
	if(checkSFINAE(csym))
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
    bool aoktemplate = true;

    //use template results for remaining stubs
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    aoktemplate = SymbolClass::statusUnknownTypeInClass(huti);

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
	    continue; //skip templates, stubs that will never get resolved
	  }

	if(csym->isStub())
	  aok &= aoktemplate; //use template
	else
	  {
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(suti, classNode, classNode, false, NULL);

	    aok &= csym->statusUnknownTypeInClass(huti);

	    m_state.popClassContext(); //restore
	  }
	it++;
      }
    return aok;
  } //statusUnknownTypeInClassInstances

  bool SymbolClassNameTemplate::statusUnknownTypeNamesInClassInstances()
  {
    bool aok = true;
    bool aoktemplate = true;

    //use template results for remaining stubs
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
	    continue; //skip templates and stubs that will never get resolved
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

	if(checkSFINAE(csym))
	  {
	    it++;
	    continue; //skip templates Mon Jun 20 10:36:42 2016
	  }

	if(m_state.isComplete(uti))
	  {
	    it++;
	    continue; //already set
	  }

	if(csym->pendingClassArgumentsForClassInstance() || csym->isStub())
	  {
	    aok = false;
	  }
	else
	  {
	    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	    if(cuti != uti && cut->isComplete())
	      {
		totalbits = cut->getBitSize();
		aok = true;
	      }
	    else
	      {
		NodeBlockClass * classNode = csym->getClassBlockNode();
		assert(classNode);
		m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

		aok = csym->trySetBitsizeWithUTIValues(totalbits);
		m_state.popClassContext(); //restore
	      }
	  }

	if(aok)
	  {
	    m_state.setBitSize(uti, totalbits); //"scalar" Class bitsize  KEY ADJUSTED
	    if(m_state.getBitSize(uti) != totalbits)
	      {
		std::ostringstream msg;
		msg << "CLASS INSTANCE '" << m_state.getUlamTypeNameByIndex(uti).c_str();
		msg << "' SIZED " << totalbits << " FAILED";
		MSG(Symbol::getTokPtr(), msg.str().c_str(),ERR);
		NodeBlockClass * classNode = csym->getClassBlockNode();
		assert(classNode);
		classNode->setNodeType(Nav); //avoid assert in resolving loop
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "CLASS INSTANCE '" << m_state.getUlamTypeNameBriefByIndex(uti).c_str();
		msg << "' UTI" << uti << ", SIZED: " << totalbits;
		MSG(Symbol::getTokPtr(), msg.str().c_str(), DEBUG);
	      }
	  }
	else
	  lostClasses.push_back(cuti); //track classes that fail to be sized.

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
	MSG(Symbol::getTokPtr(), msg.str().c_str(),DEBUG);
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
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(suti, classNode, classNode, false, NULL);
	    classNode->packBitsForVariableDataMembers(); //this instance
	    m_state.popClassContext();
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
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(suti, classNode, classNode, false, NULL);
	    csym->getDefaultValue(dval); //this instance
	    m_state.popClassContext();
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
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	csym->testThisClass(fp); //this instance
	m_state.popClassContext(); //restore
	it++;
      }
  } //testForClassInstances

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
	    msg << "Class Instance '" << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
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
	MSG(Symbol::getTokPtr(), msg.str().c_str(),ERR);
	return false;
      }

    m_state.pushClassContext(fm->getUlamTypeIdx(), fmclassblock, fmclassblock, false, NULL);
    std::vector<SymbolConstantValue *> instancesArgs;

    //copy values from stub into temp list
    std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
    while(pit != m_parameterSymbols.end())
      {
	SymbolConstantValue * psym = *pit;
	//save 'instance's arg constant symbols in a temporary list
	Symbol * asym = NULL;
	m_state.takeSymbolFromCurrentScope(psym->getId(), asym); //ownership transferred to temp list; NULL if using default value
	instancesArgs.push_back((SymbolConstantValue *) asym);
	pit++;
      } //next param

    m_state.popClassContext(); //restore

    NodeBlockClass * toclassblock = to->getClassBlockNode();
    m_state.pushClassContext(to->getUlamTypeIdx(), toclassblock, toclassblock, false, NULL);

    //replace the clone's arg symbols
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	SymbolConstantValue * asym = instancesArgs[i]; //asym might be null if default used
	u32 aid = m_parameterSymbols[i]->getId();
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
	    SymbolConstantValue * paramSym = getParameterSymbolPtr(fmidx);
	    assert(paramSym);
	    snameid = paramSym->getId();
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
	SymbolConstantValue * asym2 = new SymbolConstantValue(*asym);
	asym2->setBlockNoOfST(toclassblock->getNodeNo());
	m_state.addSymbolToCurrentScope(asym2);
      } //next arg

    m_state.popClassContext(); //restore

    instancesArgs.clear(); //don't delete the symbols
    return true;
  } //copyAnInstancesArgValues

  void SymbolClassNameTemplate::printClassTemplateArgsForPostfix(File * fp)
  {
    u32 pcnt = 0;

    fp->write("(");

    std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
    while(pit != m_parameterSymbols.end())
      {
	SymbolConstantValue * psym = *pit;
	assert(psym->isClassParameter());

	if(pcnt > 0)
	  fp->write(", ");

	fp->write(m_state.getUlamTypeNameBriefByIndex(psym->getUlamTypeIdx()).c_str());
	fp->write(" ");
	fp->write(m_state.m_pool.getDataAsString(psym->getId()).c_str());

	if(parameterHasDefaultValue(pcnt))
	  {
	    fp->write(" = ");
	    psym->printPostfixValue(fp);
	  }
	pcnt++;
	pit++;
      } //next param

    fp->write(")");
  } //printClassTemplateArgsForPostfix

  // done promptly after the full instantiation
  void SymbolClassNameTemplate::cloneAnInstancesUTImap(SymbolClass * fm, SymbolClass * to)
  {
    fm->cloneResolverUTImap(to);
    fm->cloneUnknownTypesMapInClass(to); //from the stub to the clone.
    //any additional from the template? may have duplicates with stub (not added).
    SymbolClass::cloneUnknownTypesMapInClass(to); //from the template; after UTImap.
  }

  bool SymbolClassNameTemplate::checkSFINAE(SymbolClass * sym)
  {
    //"Substitution Error Is Not A Failure"
    //bypass if template or with context of template (stub)
    return ((sym->getUlamTypeIdx() == getUlamTypeIdx()) || (sym->isStub() && m_state.isClassATemplate(sym->getContextForPendingArgs())));
  }

  // in case of a class stub was saved as a variable symbol in a NodeVarDecl plus ST;
  // wait to safely delete the stub even after it is fully instantiated (e.g. t3577 VG).
  void SymbolClassNameTemplate::trashStub(UTI uti, SymbolClass * symptr)
  {
    std::pair<std::map<UTI,SymbolClass *>::iterator,bool> ret;
    ret = m_stubsToDelete.insert(std::pair<UTI,SymbolClass*> (uti,symptr)); //stub
    assert(ret.second); //false if already existed, i.e. not added
  }

} //end MFM
