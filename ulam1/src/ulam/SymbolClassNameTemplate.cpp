#include "SymbolClassNameTemplate.h"
#include "Resolver.h"
#include "CompilerState.h"

namespace MFM {

  SymbolClassNameTemplate::SymbolClassNameTemplate(Token id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClassName(id, utype, classblock, state)
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
	csym->addTargetDescriptionMapEntry(classtargets, scid);
	it++;
      }
  } //getTargetDescriptorsForClassInstances()

  void SymbolClassNameTemplate::getClassMemberDescriptionsForClassInstances(ClassMemberMap& classmembers)
  {
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->addClassMemberDescriptionsMapEntry(classmembers);
	it++;
      }
  } //getClassMemberDescriptionsForClassInstances

  void SymbolClassNameTemplate::addParameterSymbol(SymbolConstantValue * sym)
  {
    m_parameterSymbols.push_back(sym);
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
    Node * pnode = templateclassblock->getParameterNode(n);
    assert(pnode);
    return ((NodeConstantDef *) pnode)->hasConstantExpr();
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
    SymbolClass * csym = NULL;
    return !findClassInstanceByUTI(cuti, csym);
  } //isClassTemplate

  void SymbolClassNameTemplate::setSuperClassForClassInstance(UTI superclass, UTI instance)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      csym->setSuperClass(superclass); //Nouti is none, not a subclass.
    else if(instance == getUlamTypeIdx())
      SymbolClass::setSuperClass(superclass); //instance is template definition
    else
      assert(0); //not found???
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

  bool SymbolClassNameTemplate::findClassInstanceByUTI(UTI uti, SymbolClass * & symptrref)
  {
#if 1
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.find(uti);
    if(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	symptrref = it->second;
	//expensive sanity check; isClassTemplate not so critical to check?
	//UTI suti = symptrref->getUlamTypeIdx();
	//assert( suti == uti || formatAnInstancesArgValuesAsAString(suti).compare(formatAnInstancesArgValuesAsAString(uti)) == 0);
	return true;
      }
    return false;
#else
    //debug version
    bool rtn = false;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	if(it->first == uti)
	  {
	    symptrref = it->second;
	    rtn = true;
	    break;
	  }
	it++;
      }
    return rtn;
#endif
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
    if(getNumberOfParameters() > 0)
      {
	SymbolClass * csym = NULL;
	AssertBool isDefined = findClassInstanceByUTI(instance, csym);
	assert(isDefined);
	rtnpending = csym->pendingClassArgumentsForClassInstance();
	if(rtnpending) assert(csym->isStub());
      }
    return rtnpending;
  } //pendingClassArgumentsForStubClassInstance

  SymbolClass * SymbolClassNameTemplate::makeAStubClassInstance(Token typeTok, UTI stubcuti)
  {
    NodeBlockClass * templateclassblock = getClassBlockNode();
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
	    //superuti = m_state.addStubCopyToAncestorClassTemplate(superuti, stubcuti);
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

    addClassInstanceUTI(stubcuti, newclassinstance); //link here
    return newclassinstance;
  } //makeAStubClassInstance

  // no stub available to copy, may need placeholder args too (unused)
  SymbolClass * SymbolClassNameTemplate::makeAStubClassInstanceHolder(Token typeTok, UTI suti)
  {
    NodeBlockClass * templateclassblock = getClassBlockNode();
    //previous block is template's class block, and new NNO here!
    NodeBlockClass * newblockclass = new NodeBlockClass(templateclassblock, m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(typeTok.m_locator);
    newblockclass->setNodeType(suti);
    newblockclass->resetNodeNo(templateclassblock->getNodeNo()); //keep NNO consistent (new)

    Token stubTok(TOK_IDENTIFIER, typeTok.m_locator, getId());
    SymbolClass * newclassinstance = new SymbolClass(stubTok, suti, newblockclass, this, m_state);
    assert(newclassinstance);
    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    addClassInstanceUTI(suti, newclassinstance); //link here

    //need place holder args too!
    std::vector<SymbolConstantValue *> argsForLater;
    m_state.pushClassContext(suti, newblockclass, newblockclass, false, NULL);

    u32 numparams = getNumberOfParameters();
    bool ctUnseen = (getUlamClass() == UC_UNSEEN);
    for(u32 parmIdx = 0; parmIdx < numparams; parmIdx++)
      {
	SymbolConstantValue * argSym = NULL;
	if(!ctUnseen)
	  {
	    SymbolConstantValue * paramSym = getParameterSymbolPtr(parmIdx);
	    assert(paramSym);
	    Token argTok(TOK_IDENTIFIER, typeTok.m_locator, paramSym->getId()); //use current locator
	    argSym = new SymbolConstantValue(argTok, paramSym->getUlamTypeIdx(), m_state); //like param, not copy
	  }
	else
	  {
	    std::ostringstream sname;
	    sname << "_" << parmIdx;
	    u32 snameid = m_state.m_pool.getIndexForDataString(sname.str());
	    Token argTok(TOK_IDENTIFIER, typeTok.m_locator, snameid); //use current locator
	    //stub id,  m_state.getUlamTypeOfConstant(Int) stub type, state
	    argSym = new SymbolConstantValue(argTok, Int, m_state);
	  }
	assert(argSym);
	m_state.addSymbolToCurrentScope(argSym); //scope updated to new class instance in parseClassArguments

	argsForLater.push_back(argSym);
      }

    m_state.popClassContext(); //restore before making NodeConstantDef, so current context

    //make Node with argument symbol wo trying to fold const expr;
    // add to list of unresolved for this uti
    // NULL node type descriptor, no token, yet know uti
    for(u32 argIdx = 0; argIdx < numparams; argIdx++)
      {
	NodeConstantDef * constNode = new NodeConstantDef(argsForLater[argIdx], NULL, m_state);
	assert(constNode);
	constNode->setNodeLocation(typeTok.m_locator);
	//constNode->setConstantExpr(exprNode);
	//fold later; non ready expressions saved by UTI in m_nonreadyClassArgSubtrees (stub)
	newclassinstance->linkConstantExpressionForPendingArg(constNode);
      }
     return newclassinstance;
  } //makeAStubClassInstanceHolder

  //instead of a copy, let's start new
  void SymbolClassNameTemplate::copyAStubClassInstance(UTI instance, UTI newuti, UTI context)
  {
    assert(getNumberOfParameters() > 0);
    assert(instance != newuti);

    SymbolClass * csym = NULL;
    AssertBool isDefined = findClassInstanceByUTI(instance, csym);
    assert(isDefined);

    assert(csym->pendingClassArgumentsForClassInstance());
    assert(csym->isStub());
    NodeBlockClass * blockclass = csym->getClassBlockNode();
    NodeBlockClass * templateclassblock = getClassBlockNode();

    //previous block is template's class block, and new NNO here!
    NodeBlockClass * newblockclass = new NodeBlockClass(templateclassblock, m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(blockclass->getNodeLocation());
    newblockclass->setNodeType(newuti);
    newblockclass->resetNodeNo(templateclassblock->getNodeNo()); //keep NNO consistent (new)
    newblockclass->setSuperBlockPointer(NULL); //wait for c&l when no longer a stub

    Token stubTok(TOK_IDENTIFIER,csym->getLoc(), getId());

    SymbolClass * newclassinstance = new SymbolClass(stubTok, newuti, newblockclass, this, m_state);
    assert(newclassinstance);

    newclassinstance->setUlamClass(getUlamClass());

    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    // we are in the middle of fully instantiating (context) or parsing;
    // with known args that we want to use to resolve, if possible, these pending args:
    if(copyAnInstancesArgValues(csym, newclassinstance))
      {
	//can't addClassInstanceUTI(newuti, newclassinstance) ITERATION IN PROGRESS!!!
	m_scalarClassInstanceIdxToSymbolPtrTEMP.insert(std::pair<UTI,SymbolClass*> (newuti,newclassinstance));

	newclassinstance->cloneResolverForStubClassInstance(csym, context);
	csym->cloneResolverUTImap(newclassinstance);
      }
    else
      {
	delete newclassinstance; //failed e.g. wrong number of args
	newclassinstance = NULL;
      }
  } //copyAStubClassInstance

  //called by parseThisClass, if wasIncomplete is parsed; temporary class arg names
  // are fixed to match the params
  void SymbolClassNameTemplate::fixAnyClassInstances()
  {
    ULAMCLASSTYPE classtype = getUlamClass();
    assert(classtype != UC_UNSEEN);

    //furthermore, this must exist by now, or else this is the wrong time to be fixing
    assert(getClassBlockNode());

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

	csym->setUlamClass(classtype);

	NodeBlockClass * cblock = csym->getClassBlockNode();
	assert(cblock);

	//can have 0Holder symbols for possible typedefs seen from another class
	//which will increase the count of symbols; can only test for at least;
	// (don't care about inherited symbols for class args, so use NodeBlock)
	//u32 cargs = cblock->NodeBlock::getNumberOfSymbolsInTable();
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

	//replace the temporary id with the official parameter name id;
	//update the class instance's ST.
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
		else if(i > (lastDefaultParamUsed + 1))
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
		    // and make a new symbol that's like the default param
		    SymbolConstantValue * asym2 = new SymbolConstantValue(* ((SymbolConstantValue * ) argsym));
		    assert(asym2);
		    asym2->setBlockNoOfST(cblock->getNodeNo());
		    m_state.addSymbolToCurrentScope(asym2);
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

	// the class instance's previous class block is linked to the template's when stub is made.
	// later, during c&l if a subclass, the super ptr has the class block of the superclass
	cblock->setSuperBlockPointer(NULL); //wait for c&l when no longer a stub
	it++;
      } //while
  } //fixAnyClassInstances

  bool SymbolClassNameTemplate::statusNonreadyClassArgumentsInStubClassInstances()
  {
    bool aok = true;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	UTI cuti = csym->getUlamTypeIdx();
	m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

	aok &= csym->statusNonreadyClassArguments(); //could bypass if fully instantiated
	m_state.popClassContext();
	it++;
      }
    return aok;
  } //statusNonreadyClassArgumentsInStubClassInstances

  bool SymbolClassNameTemplate::constantFoldClassArgumentsInAStubClassInstance(UTI instance)
  {
    bool aok = true;
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	UTI cuti = csym->getUlamTypeIdx();
	m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

	aok = csym->statusNonreadyClassArguments();
	m_state.popClassContext();
      }
    return aok;
  } //constantFoldClassArgumentsInAStubClassInstance

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
	    SymbolConstantValue * psym = *pit;
	    Symbol * asym = NULL;
	    bool hazyKin = false; //don't care
	    AssertBool isDefined = m_state.alreadyDefinedSymbol(psym->getId(), asym, hazyKin);
	    assert(isDefined);
	    UTI auti = asym->getUlamTypeIdx();
	    UlamType * aut = m_state.getUlamTypeByIndex(auti);

	    ULAMTYPE eutype = aut->getUlamTypeEnum();

	    //append 'instance's arg (mangled) type
	    args << aut->getUlamTypeMangledType().c_str();
	    assert(!aut->isReference());

	    //append 'instance's arg value
	    bool isok = false;
	    u64 uval;
	    if(((SymbolConstantValue *) asym)->getValue(uval))
	      {
		u32 awordsize = m_state.getTotalWordSize(auti); //must be complete!
		s32 abs = m_state.getBitSize(auti);

		switch(eutype)
		  {
		  case Int:
		    {
		      if(awordsize <= MAXBITSPERINT)
			args << ToLeximitedNumber(_SignExtend32((u32)uval, (u32) abs));
		      else if(awordsize <= MAXBITSPERLONG)
			args << ToLeximitedNumber64(_SignExtend64(uval, (u32) abs));
		      else
			assert(0);
		      isok = true;
		    }
		  break;
		  case Unsigned:
		  case Bits:
		    {
		      if(awordsize <= MAXBITSPERINT)
			args << ToLeximitedNumber((u32) uval);
		      else if(awordsize <= MAXBITSPERLONG)
			args << ToLeximitedNumber64(uval);
		      else
			assert(0);
		      isok = true;
		    }
		    break;
		  case Unary:
		    {
		      u32 pval = _Unary64ToUnsigned64(uval, abs, MAXBITSPERINT);
		      args << ToLeximitedNumber(pval);
		      isok = true;
		    }
		    break;
		  case Bool:
		    {
		      bool bval;
		      if(awordsize <= MAXBITSPERINT)
			bval = _Bool32ToCbool((u32) uval, m_state.getBitSize(auti));
		      else if(awordsize <= MAXBITSPERLONG)
			bval = _Bool64ToCbool(uval, m_state.getBitSize(auti));
		      else
			assert(0);

		      args << ToLeximitedNumber((u32) bval);
		      isok = true;
		    }
		    break;
		  default:
		    assert(0);
		  };
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
	    if(m_state.isComplete(auti))
	      {
		u32 awordsize = m_state.getTotalWordSize(auti); //requires completeness
		s32 abs = m_state.getBitSize(auti);
		ULAMTYPE eutype = m_state.getUlamTypeByIndex(auti)->getUlamTypeEnum();

		u64 uval;
		if(((SymbolConstantValue *) asym)->getValue(uval))
		  {
		    switch(eutype)
		      {
		      case Int:
			{
			  if(awordsize <= MAXBITSPERINT)
			    args << _SignExtend32((u32)uval, abs);
			  else if(awordsize <= MAXBITSPERLONG)
			    args << _SignExtend64(uval, abs);
			  else
			    assert(0);
			  isok = true;
			}
			break;
		      case Unsigned:
			{
			  args << uval << "u";
			  isok = true;
			}
			break;
		      case Unary:
			{
			  u32 pval = _Unary64ToUnsigned64(uval, abs, MAXBITSPERINT);
			  args << pval;
			  isok = true;
			}
			break;
		      case Bool:
			{
			  bool bval;
			  if(awordsize <= MAXBITSPERINT)
			    bval = _Bool32ToCbool((u32) uval, abs);
			  else if(awordsize <= MAXBITSPERLONG)
			    bval = _Bool64ToCbool(uval, abs);
			  else
			    assert(0);

			  args << (bval ? "true" : "false");
			  isok = true;
			}
			break;
		      case Bits:
			{
			  args << "0x " << std::hex << uval;  //as hex
			  isok = true;
			}
			break;
		      default:
			assert(0);
		      };
		  }
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

  bool SymbolClassNameTemplate::hasInstanceMappedUTI(UTI instance, UTI auti, UTI& mappedUTI)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	return csym->hasMappedUTI(auti, mappedUTI);
      }
    return false;
  } //hasInstanceMappedUTI

  bool SymbolClassNameTemplate::mapInstanceUTI(UTI instance, UTI auti, UTI mappeduti)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	return csym->mapUTItoUTI(auti, mappeduti);
      }
    return false;
  } //mapInstanceUTI

  bool SymbolClassNameTemplate::fullyInstantiate()
  {
    bool aok = true; //all done

    // in case of leftovers from previous resolving loop;
    // could result from a different class' instantiation.
    mergeClassInstancesFromTEMP();

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      return true;

    if(!getClassBlockNode())
      {
	std::ostringstream msg;
	msg << "Cannot fully instantiate a template class '";
	msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << "' without a definition (maybe not a class at all)";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return false;
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI cuti = csym->getUlamTypeIdx();

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
	    delete csym;
	    csym = NULL;
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

	if(!takeAnInstancesArgValues(csym, clone)) //instead of keeping template's unknown values
	  {
	    aok &= false;
	    delete clone;
	  }
	else
	  {
	    cloneAnInstancesUTImap(csym, clone);

	    it->second = clone; //replace with the full copy
	    delete csym; //done with stub
	    csym = NULL;

	    addClassInstanceByArgString(cuti, clone); //new entry, and owner of symbol class
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
	else if(superuti == Hzy) //only UNSEEN Templates
	  {
	    rtnok = false;
	  }
	else
	  {
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

	//if not in the tree, ask the resolver
	if(!foundNode)
	  csym->findNodeNoInResolver(n, foundNode);

	m_state.popClassContext(); //restore
      }
    return foundNode;
  } //findNodeNoInAClassInstance

  void SymbolClassNameTemplate::updateLineageOfClassInstanceUTI(UTI instance)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	assert(csym->getUlamTypeIdx() == instance);
	m_state.pushClassContext(instance, classNode, classNode, false, NULL);

	classNode->updateLineage(0); //do this instance
	m_state.popClassContext(); //restore
      }
  } //updateLineageOfClassInstanceUTI

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
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	if(!csym->isStub())
	  {
	    m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	    classNode->calcMaxIndexOfVirtualFunctions(); //do each instance
	    m_state.popClassContext(); //restore
	    aok &= (classNode->getVirtualMethodMaxIdx() != UNKNOWNSIZE);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance '";
	    msg << m_state.getUlamTypeNameBriefByIndex(csym->getUlamTypeIdx()).c_str();
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
    // only full instances need to be counted, unless there's an error situation
    // and we bailed out of the resolving loop.
    //    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    // while(it != m_scalarClassArgStringsToSymbolPtr.end())
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();

    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	u32 navclasscnt = ncnt;
	u32 hzyclasscnt = hcnt;
	u32 unsetclasscnt = nocnt;

	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance

	//check incomplete's too as they might have produced error msgs.
	//if(m_state.isComplete(suti))
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

	csym->countNavNodesInClassResolver(ncnt, hcnt, nocnt);

	m_state.popClassContext(); //restore

	it++;
      }

    //don't count the template since, it is no longer c&l after the first,
    //so those errors no longer count at the end of resolving loop
    return;
  } //countNavNodesInClassInstances

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
  } //setBitSizeOfClassInstances()

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

  void SymbolClassNameTemplate::buildDefaultQuarkForClassInstances()
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
	    u32 dqval = 0;
	    csym->getDefaultQuark(dqval); //this instance
	    m_state.popClassContext();
	  }
	it++;
      }
  } //buildDefaultQuarkForClassInstances

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
    // (don't care about inherited symbols for class args, so use NodeBlock)
    //u32 cargs = fmclassblock->NodeBlock::getNumberOfSymbolsInTable();
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
	else
	  m_state.addSymbolToCurrentScope(clonesym); //adds new
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
    u32 cargs = fmclassblock->NodeBlock::getNumberOfSymbolsInTable();
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

    //copy values from stub into temp list
    std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
    while(pit != m_parameterSymbols.end())
      {
	SymbolConstantValue * psym = *pit;
	//save 'instance's arg constant symbols in a temporary list
	Symbol * asym = NULL;
	bool hazyKin = false; //don't care
	AssertBool isDefined = m_state.alreadyDefinedSymbol(psym->getId(), asym, hazyKin); //no ownership change;
	assert(isDefined);
	instancesArgs.push_back((SymbolConstantValue *) asym); //for reference only
	pit++;
      } //next param

    m_state.popClassContext(); //restore

    //copy the "ready" values from first the stub (fm)
    NodeBlockClass * toclassblock = to->getClassBlockNode();
    m_state.pushClassContext(to->getUlamTypeIdx(), toclassblock, toclassblock, false, NULL);

    //make replicas for the clone's arg symbols in its ST; change blockNo.
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
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
	assert(psym->isParameter());

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
  }

} //end MFM
