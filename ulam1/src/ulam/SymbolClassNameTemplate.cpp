#include "SymbolClassNameTemplate.h"
#include "Resolver.h"
#include "CompilerState.h"

namespace MFM {

  SymbolClassNameTemplate::SymbolClassNameTemplate(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClassName(id, utype, classblock, state)
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
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->addTargetDescriptionMapEntry(classtargets);
	it++;
      }
  } //getTargetDescriptorsForClassInstances()

  void SymbolClassNameTemplate::addParameterSymbol(SymbolConstantValue * sym)
  {
    m_parameterSymbols.push_back(sym);
  }

  u32 SymbolClassNameTemplate::getNumberOfParameters()
  {
    return m_parameterSymbols.size();
  }

  u32 SymbolClassNameTemplate::getTotalSizeOfParameters()
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

  Symbol * SymbolClassNameTemplate::getParameterSymbolPtr(u32 n)
  {
    assert(n < m_parameterSymbols.size());
    return m_parameterSymbols[n];
  }

  bool SymbolClassNameTemplate::isClassTemplate()
  {
    return true;
  }

  bool SymbolClassNameTemplate::isClassTemplate(UTI cuti)
  {
    SymbolClass * csym = NULL;
    return !findClassInstanceByUTI(cuti, csym);
  } //isClassTemplate

  bool SymbolClassNameTemplate::findClassInstanceByUTI(UTI uti, SymbolClass * & symptrref)
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
	assert(findClassInstanceByUTI(instance, csym));
	rtnpending = csym->pendingClassArgumentsForClassInstance();
	if(rtnpending) assert(csym->isStub());
      }
    return rtnpending;
  } //pendingClassArgumentsForStubClassInstance

  SymbolClass * SymbolClassNameTemplate::makeAStubClassInstance(Token typeTok, UTI cuti)
  {
    NodeBlockClass * templateclassblock = getClassBlockNode();
    //previous block is template's class block, and new NNO here!
    NodeBlockClass * newblockclass = new NodeBlockClass(templateclassblock, m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(typeTok.m_locator);
    newblockclass->setNodeType(cuti);
    newblockclass->resetNodeNo(templateclassblock->getNodeNo()); //keep NNO consistent (new)

    SymbolClass * newclassinstance = new SymbolClass(getId(), cuti, newblockclass, this, m_state);
    assert(newclassinstance);
    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    addClassInstanceUTI(cuti, newclassinstance); //link here
    return newclassinstance;
  } //makeAStubClassInstance

  //instead of a copy, let's start new
  void SymbolClassNameTemplate::copyAStubClassInstance(UTI instance, UTI newuti, UTI context)
  {
    assert(getNumberOfParameters() > 0);
    assert(instance != newuti);

    SymbolClass * csym = NULL;
    assert(findClassInstanceByUTI(instance, csym));

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

    SymbolClass * newclassinstance = new SymbolClass(getId(), newuti, newblockclass, this, m_state);
    assert(newclassinstance);
    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    // we are in the middle of fully instantiating (context); with known args that we want to use
    // to resolve, if possible, these pending args:
    if(copyAnInstancesArgValues(csym, newclassinstance))
      {
	//can't addClassInstanceUTI(newuti, newclassinstance) ITERATION IN PROGRESS!!!
	m_scalarClassInstanceIdxToSymbolPtrTEMP.insert(std::pair<UTI,SymbolClass*> (newuti,newclassinstance));

	newclassinstance->cloneResolverForStubClassInstance(csym, context);
	csym->cloneResolverUTImap(newclassinstance);
      }
    else
      delete newclassinstance; //failed e.g. wrong number of args
  } //copyAStubClassInstance

  //called by parseThisClass, if wasIncomplete is parsed; temporary class arg names
  // are fixed to match the params
  void SymbolClassNameTemplate::fixAnyClassInstances()
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
		msg << "Number of Arguments (" << cargs << ") in class instance: ";
		msg << m_state.m_pool.getDataAsString(csym->getId()).c_str(); //not a uti
		msg << ", does not match the required number of parameters (";
		msg << numparams << ") to fix";
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
		    argsym->resetUlamType(m_parameterSymbols[i]->getUlamTypeIdx()); //default was Int
		    cblock->replaceIdInScope(sid, m_parameterSymbols[i]->getId(), argsym);
		  }
	      }

	    //importantly, also link the class instance's class block to the classsymbolname's.
	    cblock->setPreviousBlockPointer(getClassBlockNode());
	    it++;
	  } //while
      } //any class instances
  } //fixAnyClassInstances

  bool SymbolClassNameTemplate::statusUnknownConstantExpressionsInClassInstances()
  {
    bool aok = true; //all done
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	assert(csym);
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	aok &= csym->statusUnknownConstantExpressions();
	m_state.popClassContext(); //restore
	it++;
      }
    return aok;
  } //statusUnknownConstantExpressionsInClassInstances

  bool SymbolClassNameTemplate::statusNonreadyClassArgumentsInStubClassInstances()
  {
    bool aok = true;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	aok &= csym->statusNonreadyClassArguments(); //could bypass if fully instantiated
	m_state.popClassContext();
	it++;
      }
    return aok;
  }//statusNonreadyClassArgumentsInStubClassInstances

  bool SymbolClassNameTemplate::constantFoldClassArgumentsInAStubClassInstance(UTI instance)
  {
    bool aok = true;
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	aok = csym->statusNonreadyClassArguments();
	m_state.popClassContext();
      }
    return aok;
  }//constantFoldClassArgumentsInAStubClassInstance

  std::string SymbolClassNameTemplate::formatAnInstancesArgValuesAsAString(UTI instance)
  {
    u32 numParams = getNumberOfParameters();
    if(numParams == 0)
      {
	return "0";
      }

    std::ostringstream args;
    args << ToLeximitedNumber(numParams);

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	std::ostringstream msg;
	msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << ", has no instances; args format is number of parameters";
	MSG("", msg.str().c_str(), DEBUG);
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
	    //get 'instance's value
	    Symbol * asym = NULL;
	    assert(m_state.alreadyDefinedSymbol(psym->getId(), asym));
	    UTI auti = asym->getUlamTypeIdx();
	    UlamType * aut = m_state.getUlamTypeByIndex(auti);
	    ULAMTYPE eutype = aut->getUlamTypeEnum();

	    args << aut->getUlamTypeMangledType().c_str();

	    bool isok = false;
	    switch(eutype)
	      {
	      case Int:
		{
		  s32 sval;
		  if(((SymbolConstantValue *) asym)->getValue(sval))
		    {
		      args << ToLeximitedNumber(sval);
		      isok = true;
		    }
		  break;
		}
	      case Unsigned:
		{
		  u32 uval;
		  if(((SymbolConstantValue *) asym)->getValue(uval))
		    {
		      args << ToLeximitedNumber(uval);
		      isok = true;
		    }
		  break;
		}
	      case Bool:
		{
		  bool bval;
		  if(((SymbolConstantValue *) asym)->getValue(bval))
		    {
		      args << ToLeximitedNumber((u32) bval);
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
	    msg << "Template: " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	    msg << ", has no instances; args format is number of parameters";
	    MSG("", msg.str().c_str(), DEBUG);
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
	    Symbol * asym = NULL;
	    assert(m_state.alreadyDefinedSymbol(psym->getId(), asym));
	    UTI auti = asym->getUlamTypeIdx();
	    ULAMTYPE eutype = m_state.getUlamTypeByIndex(auti)->getUlamTypeEnum();

	    bool isok = false;
	    switch(eutype)
	      {
	      case Int:
		{
		  s32 sval;
		  if(((SymbolConstantValue *) asym)->getValue(sval))
		    {
		      args << sval;
		      isok = true;
		    }
		  break;
		}
	      case Unsigned:
		{
		  u32 uval;
		  if(((SymbolConstantValue *) asym)->getValue(uval))
		    {
		      args << uval << "u";
		      isok = true;
		    }
		  break;
		}
	      case Bool:
		{
		  bool bval;
		  if(((SymbolConstantValue *) asym)->getValue(bval))
		    {
		      args << (bval ? "true" : "false");
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

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      return true;

    if(!getClassBlockNode())
      {
	std::ostringstream msg;
	msg << "Cannot fully instantiate a template class <";
	msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << "> without a definition, maybe not a class at all";
	MSG("", msg.str().c_str(), ERR);
	return false;
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
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
	m_state.pushClassContext(cuti, NULL, NULL, false, NULL);
	mapInstanceUTI(cuti, getUlamTypeIdx(), cuti); //map template->instance, instead of fudging.
	SymbolClass * clone = new SymbolClass(*this);

	//at this point we have a NodeBlockClass! update the context
	//keep the template's location (for targetmap)
	NodeBlockClass * classNode = clone->getClassBlockNode();
	assert(classNode);

	m_state.popClassContext();
	m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

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
	    //updateLineageOfClassInstanceUTI(cuti); NNO-based now
	    cloneTemplateResolverForClassInstance(clone);
	  }
	m_state.popClassContext(); //restore
	it++;
      } //while

    // done with iteration; go ahead and merge these entries into the non-temp map
    if(!m_scalarClassInstanceIdxToSymbolPtrTEMP.empty())
      {
	std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtrTEMP.begin();
	while(it != m_scalarClassInstanceIdxToSymbolPtrTEMP.end())
	  {
	    UTI cuti = it->first;
	    SymbolClass * csym = it->second;
	    SymbolClassNameTemplate * ctsym = NULL;
	    //possibly a different template than the one currently being instantiated
	    assert(m_state.alreadyDefinedSymbolClassNameTemplate(csym->getId(), ctsym));
	    ctsym->addClassInstanceUTI(cuti, csym); //stub
	    it++;
	  }
	m_scalarClassInstanceIdxToSymbolPtrTEMP.clear();
      } //end temp stuff
    return aok;
  } //fullyInstantiate

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

	// classblock node no is NOT the same across instances, NOT TRUE anymore!!!
	// unlike ALL the other node blocks. sigh.
	//if(n == getClassBlockNode()->getNodeNo())
	//  foundNode = classNode; //slight-of-hand magic
	//else
	  {
	    classNode->findNodeNo(n, foundNode);
	    //if not in the tree, ask the resolver
	    if(!foundNode)
	      {
		csym->findNodeNoInResolver(n, foundNode);
	      }
	  }
	m_state.popClassContext(); //restore
      }
    return foundNode;
  } //findNodeNoInAClassInstance

  void SymbolClassNameTemplate::constantFoldIncompleteUTIOfClassInstance(UTI instance, UTI auti)
  {
    SymbolClass * csym = NULL;
    if(findClassInstanceByUTI(instance, csym))
      {
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	csym->constantFoldIncompleteUTI(auti); //do this instance
	m_state.popClassContext(); //restore
      }
  } //constantFoldIncompleteUTIOfClassInstance

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
	if(!csym->isStub())
	  {
	    m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	    classNode->checkCustomArrayTypeFunctions(); //do each instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(csym->getUlamTypeIdx()).c_str();
	    msg << " is still a stub, so no check for custom arrays error";
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
	    msg << " Class instance: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(csym->getUlamTypeIdx()).c_str();
	    msg << " is still a stub, so no check for duplication function error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
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
	    msg << " Class instance: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(csym->getUlamTypeIdx()).c_str();
	    msg << " is still a stub, so no calc max depth function error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }
	it++;
      }
  } //calcMaxDepthOfFunctionsForClassInstances

  void SymbolClassNameTemplate::checkAndLabelClassInstances()
  {
    // only need to c&l the unique class instances that have been deeply copied
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	if(!csym->isStub())
	  {
	    m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	    classNode->checkAndLabelType(); //do each instance
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << " Class instance: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(csym->getUlamTypeIdx()).c_str();
	    msg << " is still a stub, so check and label error";
	    MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	it++;
      }
  } //checkAndLabelClassInstances

  u32 SymbolClassNameTemplate::countNavNodesInClassInstances()
  {
    u32 navCounter = 0;

    // only full instances need to be counted
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
      {
	u32 navclasscnt = 0;
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	if(m_state.getUlamTypeByIndex(suti)->isComplete())
	  {
	    NodeBlockClass * classNode = csym->getClassBlockNode();
	    assert(classNode);
	    m_state.pushClassContext(suti, classNode, classNode, false, NULL);

	    classNode->countNavNodes(navclasscnt); //do each instance
	    if(navclasscnt > 0)
	      {
		std::ostringstream msg;
		msg << navclasscnt;
		msg << " data member nodes with illegal 'Nav' types remain in class instance <";
		msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
		msg << ">";
		MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
		navCounter += navclasscnt;
	      }
	    m_state.popClassContext(); //restore
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Class Instance: " << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << ", is incomplete; Navs will not be counted";
	    MSG("", msg.str().c_str(), DEBUG);
	  }
	it++;
      }
    return navCounter;
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

	if(m_state.getUlamTypeByIndex(uti)->isComplete())
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
	    m_state.setBitSize(uti, totalbits);  //"scalar" Class bitsize  KEY ADJUSTED
	    std::ostringstream msg;
	    msg << "CLASS INSTANCE: " << m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	    msg << " UTI" << uti << ", SIZED: " << totalbits;
	    MSG("", msg.str().c_str(), DEBUG);
	  }
	else
	  lostClasses.push_back(cuti); 	//track classes that fail to be sized.

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
	    lostClasses.pop_back();
	  }
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }
    else
      {
	std::ostringstream msg;
	msg << m_scalarClassInstanceIdxToSymbolPtr.size() << " Class Instance";
	msg << (m_scalarClassInstanceIdxToSymbolPtr.size() > 1 ? "s ALL " : " ");
	msg << "sized SUCCESSFULLY for template: " << m_state.m_pool.getDataAsString(getId()).c_str();
	MSG("", msg.str().c_str(),DEBUG);
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
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.pushClassContext(csym->getUlamTypeIdx(), classNode, classNode, false, NULL);

	classNode->packBitsForVariableDataMembers(); //this instance
	m_state.popClassContext();
	it++;
      }
  } //packBitsForClassInstances

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
	if(m_state.getUlamTypeByIndex(suti)->isComplete())
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
	    msg << "Class Instance: " << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << ", is incomplete; Code will not be generated";
	    MSG("", msg.str().c_str(), DEBUG);
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
    u32 cargs = fmclassblock->getNumberOfSymbolsInTable();
    u32 numparams = getNumberOfParameters();
    if(cargs != numparams)
      {
	//error! number of arguments in stub does not match the number of parameters
	std::ostringstream msg;
	msg << "Number of Arguments (" << cargs << ") in class instance: ";
	msg << m_state.m_pool.getDataAsString(fm->getId()).c_str(); //not a uti
	msg << ", does not match the required number of parameters (" << numparams << ")";
	MSG("", msg.str().c_str(), ERR);
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
	assert(m_state.takeSymbolFromCurrentScope(psym->getId(), asym)); //ownership transferred to temp list
	instancesArgs.push_back((SymbolConstantValue *) asym);
	pit++;
      } //next param

    m_state.popClassContext(); //restore

    NodeBlockClass * toclassblock = to->getClassBlockNode();
    m_state.pushClassContext(to->getUlamTypeIdx(), toclassblock, toclassblock, false, NULL);

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
    m_state.popClassContext(); //restore
    return true;
  } //takeAnInstancesArgValues

  bool SymbolClassNameTemplate::copyAnInstancesArgValues(SymbolClass * fm, SymbolClass * to)
  {
    NodeBlockClass * fmclassblock = fm->getClassBlockNode();
    assert(fmclassblock);
    u32 cargs = fmclassblock->getNumberOfSymbolsInTable();
    u32 numparams = getNumberOfParameters();
    if(cargs != numparams)
      {
	//error! number of arguments in stub does not match the number of parameters
	std::ostringstream msg;
	msg << "Number of Arguments (" << cargs << ") in class instance stub: ";
	msg << m_state.m_pool.getDataAsString(fm->getId()).c_str(); //not a uti
	msg << ", does not match the required number of parameters (" << numparams;
	msg << ") to copy from";
	MSG("", msg.str().c_str(),ERR);
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
	assert(m_state.alreadyDefinedSymbol(psym->getId(), asym)); //no ownership change
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

  // done promptly after the full instantiation
  void SymbolClassNameTemplate::cloneAnInstancesUTImap(SymbolClass * fm, SymbolClass * to)
  {
    fm->cloneResolverUTImap(to);
  }

  // done promptly after the full instantiation; after cloneAnInstancesUTImap
  void SymbolClassNameTemplate::cloneTemplateResolverForClassInstance(SymbolClass * csym)
  {
    if(!m_resolver)
      return; //nothing to do

    m_resolver->cloneTemplateResolver(csym);
  }//cloneResolverForClassInstance



} //end MFM
