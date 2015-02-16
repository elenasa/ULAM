#include "SymbolClassName.h"
#include "Resolver.h"
#include "CompilerState.h"

namespace MFM {

  SymbolClassName::SymbolClassName(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClass(id, utype, classblock, this, state)
  {
    setDeep();
  }

  SymbolClassName::~SymbolClassName()
  {
    // symbols belong to  NodeBlockClass's ST; deleted there.
    m_parameterSymbols.clear();

    // possible shallow instances that were never deeply instantiated
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * sym = it->second;
	if(sym && !sym->isDeep())
	  {
	    delete sym;
	    it->second = NULL;
	  }
	it++;
      }
    m_scalarClassInstanceIdxToSymbolPtr.clear(); //many-to-1 (possible after deep clone)
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

  void SymbolClassName::addParameterSymbol(SymbolConstantValue * sym)
  {
    m_parameterSymbols.push_back(sym);
  }

  u32 SymbolClassName::getNumberOfParameters()
  {
    return m_parameterSymbols.size();
  }

  u32 SymbolClassName::getTotalSizeOfParameters()
  {
    u32 totalsizes = 0;
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	//types could be incomplete, sizes unknown for template
	totalsizes += m_state.slotsNeeded(sym->getUlamTypeIdx());
      }
    return totalsizes;
  }

  Symbol * SymbolClassName::getParameterSymbolPtr(u32 n)
  {
    assert(n < m_parameterSymbols.size());
    return m_parameterSymbols[n];
  }

  bool SymbolClassName::isClassTemplate(UTI cuti)
  {
    bool rtnb = false;
    if(getNumberOfParameters() > 0)
      {
	SymbolClass * csym = NULL;
	rtnb = !findClassInstanceByUTI(cuti, csym);
      }
    return rtnb;
  } //isClassTemplate

  bool SymbolClassName::findClassInstanceByUTI(UTI uti, SymbolClass * & symptrref)
  {
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
  } //findClassInstanceByUTI

  bool SymbolClassName::findClassInstanceByArgString(UTI cuti, SymbolClass *& csymptr)
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

  void SymbolClassName::addClassInstanceUTI(UTI uti, SymbolClass * symptr)
  {
    m_scalarClassInstanceIdxToSymbolPtr.insert(std::pair<UTI,SymbolClass*> (uti,symptr)); //shallow
  }

  void SymbolClassName::addClassInstanceByArgString(UTI uti, SymbolClass * symptr)
  {
    //new (deep) entry, and owner of symbol class for class instances with args
    std::string argstring = formatAnInstancesArgValuesAsAString(uti);
    m_scalarClassArgStringsToSymbolPtr.insert(std::pair<std::string,SymbolClass*>(argstring,symptr));
  }

  bool SymbolClassName::pendingClassArgumentsForShallowClassInstance(UTI instance)
  {
    bool rtnpending = false;
    if(getNumberOfParameters() > 0)
      {
	SymbolClass * csym = NULL;
	assert(findClassInstanceByUTI(instance, csym));
	rtnpending = csym->pendingClassArgumentsForClassInstance();
	if(rtnpending) assert(!csym->isDeep());
      }
    return rtnpending;
  } //pendingClassArgumentsForShallowClassInstance

  SymbolClass * SymbolClassName::makeAShallowClassInstance(Token typeTok, UTI cuti)
  {
    //previous block is template's class block, and new NNO here!
    NodeBlockClass * newblockclass = new NodeBlockClass(getClassBlockNode(), m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(typeTok.m_locator);
    newblockclass->setNodeType(cuti);
    newblockclass->setClassTemplateParent(getUlamTypeIdx()); //so it knows it's an instance

    SymbolClass * newclassinstance = new SymbolClass(getId(), cuti, newblockclass, this, m_state);
    assert(newclassinstance);
    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    addClassInstanceUTI(cuti, newclassinstance); //link here
    return newclassinstance;
  } //makeAShallowClassInstance

  //instead of a copy, let's start new
  void SymbolClassName::copyAShallowClassInstance(UTI instance, UTI newuti, UTI context)
  {
    assert(getNumberOfParameters() > 0);
    assert(instance != newuti);

    SymbolClass * csym = NULL;
    assert(findClassInstanceByUTI(instance, csym));

    assert(csym->pendingClassArgumentsForClassInstance());
    assert(!csym->isDeep());
    NodeBlockClass * blockclass = csym->getClassBlockNode();

    //previous block is template's class block, and new NNO here!
    NodeBlockClass * newblockclass = new NodeBlockClass(getClassBlockNode(), m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(blockclass->getNodeLocation());
    newblockclass->setNodeType(newuti);
    newblockclass->setClassTemplateParent(getUlamTypeIdx()); //so it knows it's an instance

    SymbolClass * newclassinstance = new SymbolClass(getId(), newuti, newblockclass, this, m_state);
    assert(newclassinstance);
    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    //can't addClassInstanceUTI(newuti, newclassinstance) ITERATION IN PROGRESS!!!
    m_scalarClassInstanceIdxToSymbolPtrTEMP.insert(std::pair<UTI,SymbolClass*> (newuti,newclassinstance));

    // we are in the middle of fully instantiating (context); with known args that we want to use
    // to resolve, if possible, these pending args:
    copyAnInstancesArgValues(csym, newclassinstance);

    newclassinstance->cloneResolverForShallowClassInstance(csym, context);
  } //copyAShallowClassInstance

  //called by parseThisClass, if wasIncomplete is parsed; temporary class arg names
  // are fixed to match the params
  void SymbolClassName::fixAnyClassInstances()
  {
    ULAMCLASSTYPE classtype = getUlamClass();
    assert( classtype != UC_UNSEEN);

    //furthermore, this must exist by now, or else this is the wrong time to be fixing
    assert(getClassBlockNode());

    if(m_scalarClassInstanceIdxToSymbolPtr.size() > 0)
      {
	u32 numparams = getNumberOfParameters();
	std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
	while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
	  {
	    SymbolClass * csym = it->second;
	    assert(csym->getUlamClass() == UC_UNSEEN);
	    csym->setUlamClass(classtype);

	    NodeBlockClass * cblock = csym->getClassBlockNode();
	    assert(cblock);
	    u32 cargs = cblock->getNumberOfSymbolsInTable();
	    if(cargs != numparams)
	      {
		//error! number of arguments in class instance does not match the number of parameters
		std::ostringstream msg;
		msg << "number of arguments (" << cargs << ") in class instance: " << m_state.getUlamTypeNameByIndex(csym->getUlamTypeIdx()).c_str() << ", does not match the required number of parameters (" << numparams << ") to fix";
		MSG("", msg.str().c_str(),ERR);
		it++;
		continue;
	      }

	    //replace the temporary id with the official parameter name id; update the class instance's ST.
	    for(u32 i = 0; i < numparams; i++)
	      {
		Symbol * argsym = NULL;
		std::ostringstream sname;
		sname << "_" << i;
		u32 sid = m_state.m_pool.getIndexForDataString(sname.str());
		if(cblock->isIdInScope(sid,argsym))
		  {
		    assert(argsym->isConstant());
		    ((SymbolConstantValue *) argsym)->changeConstantId(sid, m_parameterSymbols[i]->getId());
		    cblock->replaceIdInScope(sid, m_parameterSymbols[i]->getId(), argsym);
		  }
	      }

	    //importantly, also link the class instance's class block to the classsymbolname's.
	    cblock->setPreviousBlockPointer(getClassBlockNode());
	    it++;
	  } //while
      } //any class instances
  } //fixAnyClassInstances

  void SymbolClassName::linkUnknownBitsizeConstantExpression(UTI auti, NodeTypeBitsize * ceNode)
  {
    SymbolClass::linkConstantExpression(auti, ceNode);
  }

  void SymbolClassName::linkUnknownBitsizeConstantExpression(UTI fromtype, UTI totype)
  {
    SymbolClass::cloneAndLinkConstantExpression(fromtype, totype);
  }

  void SymbolClassName::linkUnknownArraysizeConstantExpression(UTI auti, NodeSquareBracket * ceNode)
  {
    SymbolClass::linkConstantExpression(auti, ceNode);
  }

  void SymbolClassName::linkUnknownNamedConstantExpression(NodeConstantDef * ceNode)
  {
    SymbolClass::linkConstantExpression(ceNode);
  }

  bool SymbolClassName::statusUnknownConstantExpressionsInClassInstances()
  {
    bool aok = true; //all done
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	  {
	    std::ostringstream msg;
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; No status of unknown constant expressions";
	    MSG("", msg.str().c_str(), DEBUG);
	    return true;
	  }

	NodeBlockClass * classNode = getClassBlockNode();
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(getUlamTypeIdx());
	aok = SymbolClass::statusUnknownConstantExpressions();
	m_state.m_classBlock = saveClassNode; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(savecompilethisidx);
	return aok;
      }
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	m_state.setCompileThisIdx(suti);
	NodeBlockClass * classNode = csym->getClassBlockNode();
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	aok &= csym->statusUnknownConstantExpressions();
	it++;
      }
    //restore
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
    return aok;
  } //statusUnknownConstantExpressionsInClassInstances

  bool SymbolClassName::statusNonreadyClassArgumentsInShallowClassInstances()
  {
    bool aok = true;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	aok &= csym->statusNonreadyClassArguments(); //could bypass if deep
	it++;
      }
    return aok;
  }//statusNonreadyClassArgumentsInShallowClassInstances

  bool SymbolClassName::constantFoldClassArgumentsInAShallowClassInstance(UTI instance)
  {
    bool aok = true;
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      aok = csym->statusNonreadyClassArguments();
    return aok;
  }//constantFoldClassArgumentsInAShallowClassInstance

  std::string SymbolClassName::formatAnInstancesArgValuesAsAString(UTI instance)
  {
    u32 numParams = getNumberOfParameters();
    if(numParams == 0)
      {
	return "0";
      }
    std::ostringstream args;
    args << DigitCount(numParams, BASE10) << numParams;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	std::ostringstream msg;
	msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; args format is number of parameters";
	MSG("", msg.str().c_str(), DEBUG);
	return args.str(); //short-circuit when argument is template's UTI
      }

    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	//format values into stream
	std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
	while(pit != m_parameterSymbols.end())
	  {
	    SymbolConstantValue * psym = *pit;
	    //get 'instance's value
	    Symbol * asym = NULL;
	    assert(m_state.alreadyDefinedSymbol(psym->getId(), asym));
	    UTI auti = asym->getUlamTypeIdx();
	    ULAMTYPE eutype = m_state.getUlamTypeByIndex(auti)->getUlamTypeEnum();
	    args << DigitCount(eutype, BASE10) << eutype;

	    bool isok = false;
	    switch(eutype)
	      {
	      case Int:
		{
		  s32 sval;
		  if(((SymbolConstantValue *) asym)->getValue(sval))
		    {
		      args << DigitCount(sval, BASE10) << sval;
		      isok = true;
		    }
		  break;
		}
	      case Unsigned:
		{
		  u32 uval;
		  if(((SymbolConstantValue *) asym)->getValue(uval))
		    {
		      args << DigitCount(uval, BASE10) << uval;
		      isok = true;
		    }
		  break;
		}
	      case Bool:
		{
		  bool bval;
		  if(((SymbolConstantValue *) asym)->getValue(bval))
		    {
		      args << DigitCount(bval, BASE10) << bval;
		      isok = true;
		    }
		  break;
		}
	      default:
		assert(0);
	      };

	    if(!isok)
	      {
		std::string astr = m_state.m_pool.getDataAsString(asym->getId());
		args << DigitCount(astr.length(), BASE10) << astr.c_str();
	      }
	    pit++;
	  } //next param
      }
    //restore
    m_state.m_classBlock = getClassBlockNode();
    m_state.m_currentBlock = m_state.m_classBlock;
    return args.str();
  } //formatAnInstancesArgValuesAsAString

  bool SymbolClassName::hasInstanceMappedUTI(UTI instance, UTI auti, UTI& mappedUTI)
  {
    bool brtn = false;
    std::map<UTI, std::map<UTI,UTI> >::iterator it = m_mapOfTemplateUTIToInstanceUTIPerClassInstance.find(instance);
    if(it != m_mapOfTemplateUTIToInstanceUTIPerClassInstance.end())
      {
	assert(it->first == instance);
	std::map<UTI, UTI> amap = it->second;
	std::map<UTI, UTI>::iterator mit = amap.find(auti);
	if(mit != amap.end())
	  {
	    brtn = true;
	    assert(mit->first == auti);
	    mappedUTI = mit->second;
	  }
      }
    return brtn;
  } //hasInstanceMappedUTI

  void SymbolClassName::mapInstanceUTI(UTI instance, UTI auti, UTI mappeduti)
  {
    std::map<UTI, std::map<UTI,UTI> >::iterator it = m_mapOfTemplateUTIToInstanceUTIPerClassInstance.find(instance);
    if(it != m_mapOfTemplateUTIToInstanceUTIPerClassInstance.end())
      {
	assert(it->first == instance);
	it->second.insert(std::pair<UTI, UTI>(auti, mappeduti));
      }
    else
      {
	std::map<UTI, UTI> amap;
	amap.insert(std::pair<UTI, UTI>(auti, mappeduti));
	m_mapOfTemplateUTIToInstanceUTIPerClassInstance.insert(std::pair <UTI, std::map<UTI, UTI> >(instance, amap));
      }
    //sanity check please..
    UTI checkuti;
    assert(hasInstanceMappedUTI(instance,auti,checkuti));
    assert(checkuti == mappeduti);
  } //mapInstanceUTI

  bool SymbolClassName::cloneInstances()
  {
    bool aok = true; //all done

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(!isDeep())
	  {
	    updateLineageOfClassInstanceUTI(getUlamTypeIdx());
	    setDeep(); //i.e. seen
	  }
	return true;
      }

    UTI savecompilethisidx = m_state.m_compileThisIdx;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	if(csym->isDeep())
	  {
	    it++;
	    continue; //already done
	  }

	//ask shallow class symbol..
	if(csym->pendingClassArgumentsForClassInstance())
	  {
	    aok = false;
	    it++;
	    continue; //have to wait
	  }

	//have we seen these args before?
	UTI cuti = csym->getUlamTypeIdx();
	SymbolClass * dupsym = NULL;
	if(findClassInstanceByArgString(cuti, dupsym))
	  {
	    UTI duti = dupsym->getUlamTypeIdx();
	    m_state.mergeClassUTI(cuti,duti);
	    delete csym;
	    csym = NULL;
	    it->second = dupsym; //duplicate! except different UTIs
	    it++;
	    continue;
	  }

	// first time for this cuti, and ready args!
	m_state.setCompileThisIdx(cuti);
	mapInstanceUTI(cuti, getUlamTypeIdx(), cuti); // map template->instance instead of fudging.
	SymbolClass * clone = new SymbolClass(*this); //sets deep flag

	takeAnInstancesArgValues(csym, clone); //instead of keeping template's unknown values

	delete csym; //done with shallow copy
	csym = NULL;
	it->second = clone; //replace with the deep copy

	addClassInstanceByArgString(cuti, clone); //new entry, and owner of symbol class
	updateLineageOfClassInstanceUTI(cuti);
	cloneResolverForClassInstance(clone);
	it++;
      } //while

    // done with iteration; go ahead and merge these entries into the non-temp map
    // don't know which clone they are associated with, except by their resolvers perhaps
    // will their block's previous ptrs be updated properly?
    if(!m_scalarClassInstanceIdxToSymbolPtrTEMP.empty())
      {
	//m_scalarClassInstanceIdxToSymbolPtr.insert(m_scalarClassInstanceIdxToSymbolPtrTEMP.begin(), m_scalarClassInstanceIdxToSymbolPtrTEMP.end());
	//m_scalarClassInstanceIdxToSymbolPtrTEMP.erase(m_scalarClassInstanceIdxToSymbolPtrTEMP.begin(), m_scalarClassInstanceIdxToSymbolPtrTEMP.end());
	//m_scalarClassInstanceIdxToSymbolPtrTEMP.clear();
	std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtrTEMP.begin();
	while(it != m_scalarClassInstanceIdxToSymbolPtrTEMP.end())
	  {
	    UTI cuti = it->first;
	    SymbolClass * csym = it->second;
	    addClassInstanceUTI(cuti, csym); //shallow

	    //update the previous block ptr using the saved context in the resolver
	    UTI contextUTI = csym->getContextForPendingArgs();
	    SymbolClass * fromC = NULL;
	    assert(findClassInstanceByUTI(contextUTI, fromC));

	    NodeBlockClass * classblock = csym->getClassBlockNode();
	    classblock->setPreviousBlockPointer(fromC->getClassBlockNode());
	    it++;
	  }
	m_scalarClassInstanceIdxToSymbolPtrTEMP.clear();
      } //end temp stuff

    m_state.setCompileThisIdx(savecompilethisidx); //restore
    return aok;
  } //cloneInstances

  Node * SymbolClassName::findNodeNoInAClassInstance(UTI instance, NNO n)
  {
    Node * foundNode = NULL;
    NodeBlockClass * saveclassblock = m_state.m_classBlock;
    NodeBlock * savecurrentblock = m_state.m_currentBlock;

    if(getUlamTypeIdx() == instance)
      {
	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	classNode->findNodeNo(n, foundNode);
	m_state.m_classBlock = saveclassblock; //restore
	m_state.m_currentBlock = savecurrentblock;
	return foundNode;
      }

    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	// classblock node no is NOT the same across instances,
	// unlike ALL the other node blocks. sigh.
	if(n == getClassBlockNode()->getNodeNo())
	  foundNode = classNode;
	else
	  classNode->findNodeNo(n, foundNode);
      }

    m_state.m_classBlock = saveclassblock; //restore
    m_state.m_currentBlock = savecurrentblock;
    return foundNode;
  } //findNodeNoInAClassInstance

  void SymbolClassName::constantFoldIncompleteUTIOfClassInstance(UTI instance, UTI auti)
  {
    NodeBlockClass * saveclassnode = m_state.m_classBlock;
    NodeBlock * saveblocknode = m_state.m_currentBlock;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	  {
	    std::ostringstream msg;
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; Constant Fold of Incomplete UTI"
 << auti << " will not be tried";
	    MSG("", msg.str().c_str(), DEBUG);
	    return;
	  }

	assert(instance == getUlamTypeIdx());
	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	SymbolClass::constantFoldIncompleteUTI(auti);
	m_state.m_classBlock = saveclassnode; //restore
	m_state.m_currentBlock = saveblocknode;
	return;
      }

    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	csym->constantFoldIncompleteUTI(auti); //do this instance
      }
    m_state.m_classBlock = saveclassnode; //restore
    m_state.m_currentBlock = saveblocknode;
  } //constantFoldIncompleteUTIOfClassInstance

  void SymbolClassName::updateLineageOfClassInstanceUTI(UTI instance)
  {
    NodeBlockClass * saveclassnode = m_state.m_classBlock;
    NodeBlock * saveblocknode = m_state.m_currentBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    //if(m_scalarClassInstanceIdxToSymbolPtr.empty())
    if(getUlamTypeIdx() == instance)
      {
	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(getUlamTypeIdx());
	classNode->updateLineage(NULL);
	m_state.m_classBlock = saveclassnode; //restore
	m_state.m_currentBlock = saveblocknode;
	m_state.setCompileThisIdx(savecompilethisidx);
	return;
      }

    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(instance);
	classNode->updateLineage(NULL); //do this instance
      }
    m_state.m_classBlock = saveclassnode; //restore
    m_state.m_currentBlock = saveblocknode;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //updateLineageOfClassInstanceUTI

  void SymbolClassName::checkCustomArraysOfClassInstances()
  {
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	  {
	    std::ostringstream msg;
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; Custom Array check will not be done";
	    MSG("", msg.str().c_str(), DEBUG);
	    return;
	  }

	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(getUlamTypeIdx());
	classNode->checkCustomArrayTypeFunctions();
	m_state.m_classBlock = saveClassNode; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(savecompilethisidx);
	return;
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);

	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(csym->getUlamTypeIdx());
	classNode->checkCustomArrayTypeFunctions(); //do each instance
	it++;
      }
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //checkCustomArraysOfClassInstances()


  void SymbolClassName::checkAndLabelClassInstances()
  {
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	  {
	    std::ostringstream msg;
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; Check & Type Labelomg will not be done";
	    MSG("", msg.str().c_str(), DEBUG);
	    return;
	  }

	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(getUlamTypeIdx());
	classNode->checkAndLabelType();
	m_state.m_classBlock = saveClassNode; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(savecompilethisidx);
	return;
      }

    // only need to c&l the unique class instances that have been deeply copied
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	if(csym->isDeep())
	  {
	    m_state.m_classBlock = classNode;
	    m_state.m_currentBlock = m_state.m_classBlock;
	    m_state.setCompileThisIdx(csym->getUlamTypeIdx()); //this instance
	    classNode->checkAndLabelType(); //do each instance
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance: ";
	    msg << m_state.getUlamTypeNameByIndex(csym->getUlamTypeIdx()).c_str();
	    msg << " is still shallow, so check and label error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	it++;
      }
    //restore
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //checkAndLabelClassInstances

  u32 SymbolClassName::countNavNodesInClassInstances()
  {
    u32 navCounter = 0;
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	  {
	    std::ostringstream msg;
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; Nav nodes will not be counted";
	    MSG("", msg.str().c_str(), DEBUG);
	    return 0;
	  }

	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(getUlamTypeIdx());

	classNode->countNavNodes(navCounter);
	if(navCounter > 0)
	  {
	    std::ostringstream msg;
	    msg << navCounter << " data member nodes with illegal 'Nav' types remain in class <";
	    msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	    msg << ">";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }
	m_state.m_classBlock = saveClassNode; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(savecompilethisidx);
	return navCounter;
      }

    // only deep instances need to be counted
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	u32 navclasscnt = 0;
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	if(m_state.getUlamTypeByIndex(suti)->isComplete())
	  {
	    m_state.setCompileThisIdx(suti);
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    m_state.m_classBlock = classNode;
	    m_state.m_currentBlock = m_state.m_classBlock;
	    classNode->countNavNodes(navclasscnt); //do each instance
	    if(navclasscnt > 0)
	      {
		std::ostringstream msg;
		msg << navclasscnt << " data member nodes with illegal 'Nav' types remain in class instance <";
		msg << m_state.getUlamTypeNameByIndex(suti).c_str();
		msg << ">";
		MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
		navCounter += navclasscnt;
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Class Instance: " << m_state.getUlamTypeNameByIndex(suti).c_str() << ", is incomplete; Navs will not be counted";
	    MSG("", msg.str().c_str(), DEBUG);
	  }
	it++;
      }
    //restore
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
    return navCounter;
  } //countNavNodesInClassInstances

  bool SymbolClassName::setBitSizeOfClassInstances()
  {
    bool aok = true;
    NodeBlockClass * saveclassnode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    //check for class instances
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	  {
	    std::ostringstream msg;
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; Bit sizing will not be determined";
	    MSG("", msg.str().c_str(), DEBUG);
	    return true;
	  }

	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode); //infinite loop "Incomplete Class <> was never defined, fails sizing"
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(getUlamTypeIdx());
	s32 totalbits = 0;
	aok = SymbolClass::trySetBitsizeWithUTIValues(totalbits);
	if(aok)
	  {
	    UTI cuti = getUlamTypeIdx();
	    m_state.setBitSize(cuti, totalbits);  //"scalar" Class bitsize  KEY ADJUSTED
	    std::ostringstream msg;
	    msg << "CLASS (without instances): " << m_state.getUlamTypeNameByIndex(cuti).c_str() << " SIZED: " << totalbits;
	    MSG("", msg.str().c_str(),DEBUG);
	  }
	m_state.m_classBlock = saveclassnode; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(savecompilethisidx);
	return aok;
      }

    std::vector<UTI> lostClasses;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI cuti = csym->getUlamTypeIdx(); //this instance
	UTI uti = it->first; //this instance entry; may not match Symbol class' uti
	s32 totalbits = 0;

	if(m_state.getUlamTypeByIndex(uti)->isComplete())
	  {
	    it++;
	    continue; //already set
	  }

	if(csym->pendingClassArgumentsForClassInstance() || !csym->isDeep())
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
		m_state.setCompileThisIdx(cuti);
		m_state.m_classBlock = csym->getClassBlockNode();
		m_state.m_currentBlock = m_state.m_classBlock;

		aok = csym->trySetBitsizeWithUTIValues(totalbits);
	      }
	  }

	if(aok)
	  {
	    m_state.setBitSize(uti, totalbits);  //"scalar" Class bitsize  KEY ADJUSTED
	    std::ostringstream msg;
	    msg << "CLASS INSTANCE: " << m_state.getUlamTypeNameByIndex(uti).c_str() << " SIZED: " << totalbits;
	    MSG("", msg.str().c_str(),DEBUG);
	  }
	else
	  lostClasses.push_back(cuti); 	//track classes that fail to be sized.

	aok = true; //reset for next class
	m_state.m_classBlock = saveclassnode; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(savecompilethisidx);

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
	    msg << ", " << m_state.getUlamTypeNameByIndex(lcuti).c_str();
	    lostClasses.pop_back();
	  }
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),DEBUG);
      }
    else
      {
	std::ostringstream msg;
	msg << m_scalarClassInstanceIdxToSymbolPtr.size() << " Class Instance" << (m_scalarClassInstanceIdxToSymbolPtr.size() > 1 ? "s ALL " : " ") << "sized SUCCESSFULLY for template: " << m_state.m_pool.getDataAsString(getId()).c_str();
	MSG("", msg.str().c_str(),DEBUG);
      }
    lostClasses.clear();

    //restore
    m_state.m_classBlock = saveclassnode;
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
    return aok;
  } //setBitSizeOfClassInstances()

  // separate pass...after labeling all classes is completed;
  void SymbolClassName::printBitSizeOfClassInstances()
  {
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	    return; //skip templates

	SymbolClass::printBitSizeOfClass();
	return;
      }

    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->printBitSizeOfClass(); //this instance
	it++;
      }
  } //printBitSizeOfClassInstances

  void SymbolClassName::packBitsForClassInstances()
  {
    NodeBlockClass * saveclassBlock = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	  {
	    std::ostringstream msg;
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; Packed bits will not be done";
	    MSG("", msg.str().c_str(), DEBUG);
	    return;
	  }

	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = getClassBlockNode();
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(getUlamTypeIdx());

	classNode->packBitsForVariableDataMembers();
	m_state.m_classBlock = saveclassBlock; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(savecompilethisidx);
	return;
      }

    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	UTI suti = csym->getUlamTypeIdx(); //this instance
	m_state.setCompileThisIdx(suti);

	classNode->packBitsForVariableDataMembers(); //this instance
	it++;
      }
    //restore
    m_state.m_classBlock = saveclassBlock;
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //packBitsForClassInstances

  void SymbolClassName::testForClassInstances(File * fp)
  {
    NodeBlockClass * saveclassBlock = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	  {
	    std::ostringstream msg;
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; No test will be generated";
	    MSG("", msg.str().c_str(), DEBUG);
	    return;
	  }

	m_state.m_classBlock = getClassBlockNode();
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(getUlamTypeIdx());

	SymbolClass::testThisClass(fp);
	m_state.m_classBlock = saveclassBlock; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(savecompilethisidx);
	return;
      }
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(csym);
	m_state.m_classBlock = csym->getClassBlockNode();
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(csym->getUlamTypeIdx());
	csym->testThisClass(fp); //this instance
	it++;
      }
    //restore
    m_state.m_classBlock = saveclassBlock; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //testForClassInstances

  void SymbolClassName::generateCodeForClassInstances(FileManager * fm)
  {
    NodeBlockClass * saveclassBlock = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	if(getNumberOfParameters() > 0)
	  {
	    std::ostringstream msg;
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << ", has no instances; No C++ code will be generated";
	    MSG("", msg.str().c_str(), DEBUG);
	    return;
	  }
	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(getUlamTypeIdx());

	SymbolClass::generateCode(fm);
	m_state.m_classBlock = saveclassBlock; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	m_state.setCompileThisIdx(savecompilethisidx);
	return;
      }

    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(csym->isDeep());
	UTI suti = csym->getUlamTypeIdx();
	if(m_state.getUlamTypeByIndex(suti)->isComplete())
	  {
	    m_state.m_classBlock = csym->getClassBlockNode();
	    m_state.m_currentBlock = m_state.m_classBlock;
	    m_state.setCompileThisIdx(suti); //this instance

	    csym->generateCode(fm);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Class Instance: " << m_state.getUlamTypeNameByIndex(suti).c_str() << ", is incomplete; Code will not be generated";
	    MSG("", msg.str().c_str(), DEBUG);
	  }
	it++;
      }
    //restore
    m_state.m_classBlock = saveclassBlock; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //generateCodeForClassInstances

  void SymbolClassName::generateIncludesForClassInstances(File * fp)
  {
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	return SymbolClass::generateAsOtherInclude(fp);
      }

    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->generateAsOtherInclude(fp);
	it++;
      }
  } //generateIncludesForClassInstances

  void SymbolClassName::generateForwardDefsForClassInstances(File * fp)
  {
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	return SymbolClass::generateAsOtherForwardDef(fp);
      }

    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->generateAsOtherForwardDef(fp);
	it++;
      }
  } //generateForwardDefsForClassInstances

  void SymbolClassName::generateTestInstanceForClassInstances(File * fp, bool runtest)
  {
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	SymbolClass::generateTestInstance(fp, runtest);
	return;
      }

    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(csym);
	csym->generateTestInstance(fp, runtest);
	it++;
      }
  } //generateTestInstanceForClassInstances

  bool SymbolClassName::takeAnInstancesArgValues(SymbolClass * fm, SymbolClass * to)
  {
    NodeBlockClass * saveClassBlock = m_state.m_classBlock;
    NodeBlockClass * fmclassblock = fm->getClassBlockNode();
    assert(fmclassblock);
    u32 cargs = fmclassblock->getNumberOfSymbolsInTable();
    u32 numparams = getNumberOfParameters();
    if(cargs != numparams)
      //if(cargs < numparams)
      {
	//error! number of arguments in shallow class instance does not match the number of parameters
	std::ostringstream msg;
	msg << "number of arguments (" << cargs << ") in class instance: " << m_state.getUlamTypeNameByIndex(fm->getId()).c_str() << ", does not match the required number of parameters (" << numparams << ")";
	MSG("", msg.str().c_str(),ERR);
	return false;
      }

    m_state.m_classBlock = fmclassblock;
    m_state.m_currentBlock = m_state.m_classBlock;
    std::vector<SymbolConstantValue *> instancesArgs;

    //copy values from shallow instance into temp list
    std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
    while(pit != m_parameterSymbols.end())
      {
	SymbolConstantValue * psym = *pit;
	//save 'instance's arg constant symbols in a temporary list
	Symbol * asym = NULL;
	assert(m_state.takeSymbolFromCurrentScope(psym->getId(), asym)); //ownership transferred to temp list
	instancesArgs.push_back((SymbolConstantValue *) asym);
	pit++;
      } //next param

    NodeBlockClass * toclassblock = to->getClassBlockNode();
    m_state.m_classBlock = toclassblock;
    m_state.m_currentBlock = m_state.m_classBlock;

    //replace the clone's arg symbols
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	SymbolConstantValue * asym = instancesArgs[i];
	u32 aid = asym->getId();
	Symbol * clonesym = NULL;
	assert(m_state.alreadyDefinedSymbol(aid, clonesym));
	m_state.replaceSymbolInCurrentScope(clonesym, asym); //deletes old, adds new
      } //next arg

    instancesArgs.clear(); //don't delete the symbols
    //restore
    m_state.m_classBlock = saveClassBlock;
    m_state.m_currentBlock = m_state.m_classBlock;
    return true;
  } //takeAnInstancesArgValues

  bool SymbolClassName::copyAnInstancesArgValues(SymbolClass * fm, SymbolClass * to)
  {
    NodeBlockClass * saveClassBlock = m_state.m_classBlock;
    NodeBlockClass * fmclassblock = fm->getClassBlockNode();
    assert(fmclassblock);
    u32 cargs = fmclassblock->getNumberOfSymbolsInTable();
    u32 numparams = getNumberOfParameters();
    if(cargs != numparams)
      //if(cargs < numparams)
      {
	//error! number of arguments in shallow class instance does not match the number of parameters
	std::ostringstream msg;
	msg << "number of arguments (" << cargs << ") in class instance: " << m_state.getUlamTypeNameByIndex(fm->getId()).c_str() << ", does not match the required number of parameters (" << numparams << ")";
	MSG("", msg.str().c_str(),ERR);
	return false;
      }

    m_state.m_classBlock = fmclassblock;
    m_state.m_currentBlock = m_state.m_classBlock;
    std::vector<SymbolConstantValue *> instancesArgs;

    //copy values from shallow instance into temp list
    std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
    while(pit != m_parameterSymbols.end())
      {
	SymbolConstantValue * psym = *pit;
	//save 'instance's arg constant symbols in a temporary list
	Symbol * asym = NULL;
	assert(m_state.alreadyDefinedSymbol(psym->getId(), asym)); //no ownership change
	instancesArgs.push_back((SymbolConstantValue *) asym); //for reference only
	pit++;
      } //next param

    NodeBlockClass * toclassblock = to->getClassBlockNode();
    m_state.m_classBlock = toclassblock;
    m_state.m_currentBlock = m_state.m_classBlock;

    //make replicas for the clone's arg symbols in its ST; change blockNo.
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	SymbolConstantValue * asym = instancesArgs[i];
	SymbolConstantValue * asym2 = new SymbolConstantValue(*asym);
	asym2->setBlockNoOfST(toclassblock->getNodeNo());
	m_state.addSymbolToCurrentScope(asym2);
      } //next arg

    instancesArgs.clear(); //don't delete the symbols
    //restore
    m_state.m_classBlock = saveClassBlock;
    m_state.m_currentBlock = m_state.m_classBlock;
    return true;
  } //copyAnInstancesArgValues

  // done promptly after the deep clone
  void SymbolClassName::cloneResolverForClassInstance(SymbolClass * csym)
  {
    if(!m_resolver)
      return; //nothing to do

    UTI cuti = csym->getUlamTypeIdx();

    //populate empty resolver, for each unknown UTI
    std::map<UTI, std::map<UTI,UTI> >::iterator mit = m_mapOfTemplateUTIToInstanceUTIPerClassInstance.find(cuti);
    if(mit != m_mapOfTemplateUTIToInstanceUTIPerClassInstance.end())
      {
	assert(cuti == mit->first);
	std::map<UTI,UTI> utimap = mit->second;
	std::map<UTI,UTI>::iterator it = utimap.begin();

	while(it != utimap.end())
	  {
	    UTI olduti = it->first;
	    UTI newuti = it->second;
	    csym->cloneConstantExpressionSubtreesByUTI(olduti, newuti, *m_resolver);
	    it++;
	  }
      }
    else
      assert(0);
    //next, named constants separately
    csym->cloneNamedConstantExpressionSubtrees(*m_resolver);
  }//cloneResolverForClassInstance

} //end MFM
