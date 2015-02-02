#include "SymbolClassName.h"
#include "CompilerState.h"

namespace MFM {

  //#define _DEBUG_DONT_MERGE

  SymbolClassName::SymbolClassName(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClass(id, utype, classblock, this, state){}

  SymbolClassName::~SymbolClassName()
  {
    // symbols belong to  NodeBlockClass's ST; deleted there.
    m_parameterSymbols.clear();

    // need to delete class instance symbols; ownership belongs here!
    for(std::size_t i = 0; i < m_scalarClassInstanceIdxToSymbolPtr.size(); i++)
      {
	delete m_scalarClassInstanceIdxToSymbolPtr[i];
      }
    m_scalarClassInstanceIdxToSymbolPtr.clear();
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
	rtnb = !isClassInstance(cuti, csym);
      }
    return rtnb;
  } //isClassTemplate

  bool SymbolClassName::isClassInstance(UTI uti, SymbolClass * & symptrref)
  {
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.find(uti);
    if(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	symptrref = it->second;
	assert( symptrref->getUlamTypeIdx() == uti);
	return true;
      }
    return false;
  } //isClassInstance

  void SymbolClassName::addClassInstance(UTI uti, SymbolClass * symptr)
  {
    m_scalarClassInstanceIdxToSymbolPtr.insert(std::pair<UTI,SymbolClass*> (uti,symptr));
  }

  SymbolClass * SymbolClassName::makeAShallowClassInstance(Token typeTok, UTI cuti)
  {
    NodeBlockClass * newblockclass = new NodeBlockClass(getClassBlockNode(), m_state);
    assert(newblockclass);
    newblockclass->setNodeLocation(typeTok.m_locator);
    newblockclass->setNodeType(cuti);
    newblockclass->setClassTemplateParent(getUlamTypeIdx()); //so it knows it's an instance with a template parent

    SymbolClass * newclassinstance = new SymbolClass(getId(), cuti, newblockclass, this, m_state);
    assert(newclassinstance);
    if(isQuarkUnion())
      newclassinstance->setQuarkUnion();

    addClassInstance(cuti, newclassinstance); //link here
    return newclassinstance;
  } //makeAShallowClassInstance

  //called by parseThisClass, if wasIncomplete is parsed; temporary class arg names
  // are fixed to match the params
  void SymbolClassName::fixAnyClassInstances()
  {
    ULAMCLASSTYPE classtype = getUlamClass();
    assert( classtype != UC_INCOMPLETE);

    //furthermore, this must exist by now, or else this is the wrong time to be fixing
    assert(getClassBlockNode());

    if(m_scalarClassInstanceIdxToSymbolPtr.size() > 0)
      {
	u32 numparams = getNumberOfParameters();
	std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
	while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
	  {
	    SymbolClass * csym = it->second;
	    assert(csym->getUlamClass() == UC_INCOMPLETE);
	    csym->setUlamClass(classtype);

	    NodeBlockClass * cblock = csym->getClassBlockNode();
	    assert(cblock);
	    u32 cargs = cblock->getNumberOfSymbolsInTable();
	    if(cargs != numparams)
	      {
		//error! number of arguments in class instance does not match the number of parameters
		std::ostringstream msg;
		msg << "number of arguments (" << cargs << ") in class instance: " << m_state.getUlamTypeNameByIndex(csym->getId()).c_str() << ", does not match the required number of parameters (" << numparams << ")";
		MSG("", msg.str().c_str(),ERR);
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

  std::string SymbolClassName::formatAnInstancesArgValuesAsAString(UTI instance)
  {
    u32 numParams = getNumberOfParameters();
    if(numParams == 0)
      {
	return "0";
      }
    std::ostringstream args;
    args << DigitCount(numParams, BASE10) << numParams;

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.find(instance);
    if(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	assert(it->first == instance);
	SymbolClass * csym = it->second;
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
	    switch(eutype)
	      {
	      case Int:
		{
		  s32 sval;
		  assert(((SymbolConstantValue *) asym)->getValue(sval));
		  args << DigitCount(sval, BASE10) << sval;
		  break;
		}
	      case Unsigned:
		{
		  u32 uval;
		  assert(((SymbolConstantValue *) asym)->getValue(uval));
		  args << DigitCount(uval, BASE10) << uval;
		  break;
		}
	      case Bool:
		{
		  bool bval;
		  assert(((SymbolConstantValue *) asym)->getValue(bval));
		  args << DigitCount(bval, BASE10) << bval;
		  break;
		}
	      default:
		assert(0);
	      };
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
	std::map<UTI, UTI> amap = it->second;
	amap.insert(std::pair<UTI, UTI>(auti, mappeduti));
      }
    else
      {
	std::map<UTI, UTI> amap;
	amap.insert(std::pair<UTI, UTI>(auti, mappeduti));
	m_mapOfTemplateUTIToInstanceUTIPerClassInstance.insert(std::pair <UTI, std::map<UTI, UTI> >(instance, amap));
      }
  } //mapInstanceUTI

  void SymbolClassName::cloneInstances()
  {
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      return;

    UTI saveTemplateUTI = getUlamTypeIdx();
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	m_state.m_compileThisIdx = csym->getUlamTypeIdx();
	m_utypeIdx =  csym->getUlamTypeIdx(); //fudge..
	SymbolClass * clone = new SymbolClass(*this);
	m_utypeIdx = saveTemplateUTI; //restore
	takeAnInstancesArgValues(csym, clone);
	delete csym;
	csym = NULL;
	it->second = clone;
	it++;
      } //while
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

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.find(instance);
    if(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	classNode->findNodeNo(n, foundNode);
      }

    m_state.m_classBlock = saveclassblock; //restore
    m_state.m_currentBlock = savecurrentblock;
    return foundNode;
  } //findNodeNoInAClassInstance

  void SymbolClassName::updateLineageOfClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	classNode->updateLineage(NULL);
	return;
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->getClassBlockNode()->updateLineage(NULL); //do each instance
	it++;
      }
  } //updateLineageOfClassInstances

  void SymbolClassName::checkCustomArraysOfClassInstances()
  {
    NodeBlockClass * saveClassNode = m_state.m_classBlock;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	classNode->checkCustomArrayTypeFunctions();
	m_state.m_classBlock = saveClassNode; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
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

	classNode->checkCustomArrayTypeFunctions(); //do each instance
	it++;
      }
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
  } //checkCustomArraysOfClassInstances()


  void SymbolClassName::checkAndLabelClassInstances()
  {
    NodeBlockClass * saveClassNode = m_state.m_classBlock;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	classNode->checkAndLabelType();
	m_state.m_classBlock = saveClassNode; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	return;
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	m_state.m_compileThisIdx = suti;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	classNode->checkAndLabelType(); //do each instance
	it++;
      }
    //restore
    m_state.m_compileThisIdx = m_utypeIdx;
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
  } //checkAndLabelClassInstances

  u32 SymbolClassName::countNavNodesInClassInstances()
  {
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    u32 navCounter = 0;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

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
	return navCounter;
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	u32 navclasscnt = 0;
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	m_state.m_compileThisIdx = suti;
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
	it++;
      }
    //restore
    m_state.m_compileThisIdx = m_utypeIdx;
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    return navCounter;
  } //countNavNodesInClassInstances

  bool SymbolClassName::setBitSizeOfClassInstances()
  {
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    bool aok = true;

    //check for class instances
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode); //infinite loop "Incomplete Class <> was never defined, fails sizing"
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
	s32 totalbits = 0;
	UTI cuti = getUlamTypeIdx();
	aok = SymbolClass::trySetBitsizeWithUTIValues(totalbits);
	if(aok)
	  m_state.setBitSize(cuti, totalbits);  //"scalar" Class bitsize  KEY ADJUSTED
	m_state.m_classBlock = saveClassNode; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	return aok;
      }

    std::vector<UTI> lostClasses;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance

	m_state.m_compileThisIdx = suti;
	m_state.m_classBlock = csym->getClassBlockNode();
	m_state.m_currentBlock = m_state.m_classBlock;

	s32 totalbits = 0;
	aok = csym->trySetBitsizeWithUTIValues(totalbits);

	//track classes that fail to be sized.
	if(aok)
	  {
	    m_state.setBitSize(suti, totalbits);  //"scalar" Class bitsize  KEY ADJUSTED
	  }
	else
	  lostClasses.push_back(suti);

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
	    UTI cuti = lostClasses.back();
	    msg << ", " << m_state.getUlamTypeNameByIndex(cuti).c_str();
	    lostClasses.pop_back();
	  }
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),DEBUG);
      }
    else
      {
	std::ostringstream msg;
	msg << m_scalarClassInstanceIdxToSymbolPtr.size() << " Class Instance" << (m_scalarClassInstanceIdxToSymbolPtr.size() > 1 ? "s ALL " : " ") << "sized SUCCESSFULLY";
	MSG("", msg.str().c_str(),INFO);
      }
    lostClasses.clear();

    //restore
    m_state.m_compileThisIdx = m_utypeIdx;
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    return aok;
  } //setBitSizeOfClassInstances()

  // separate pass...after labeling all classes is completed;
  void SymbolClassName::printBitSizeOfClassInstances()
  {
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	return SymbolClass::printBitSizeOfClass();
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	csym->printBitSizeOfClass(); //this instance
	it++;
      }
  } //printBitSizeOfClassInstances

  void SymbolClassName::packBitsForClassInstances()
  {
    NodeBlockClass * saveclassBlock = m_state.m_classBlock;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	NodeBlockClass * classNode = getClassBlockNode();
	m_state.m_classBlock = getClassBlockNode();
	m_state.m_currentBlock = m_state.m_classBlock;
	assert(classNode);
	classNode->packBitsForVariableDataMembers();
	m_state.m_classBlock = saveclassBlock; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
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

	UTI suti = csym->getUlamTypeIdx(); //this instance
	m_state.m_compileThisIdx = suti;

	classNode->packBitsForVariableDataMembers(); //this instance
	it++;
      }
    //restore
    m_state.m_compileThisIdx = m_utypeIdx;
    m_state.m_classBlock = saveclassBlock;
    m_state.m_currentBlock = m_state.m_classBlock;
  } //packBitsForClassInstances

  void SymbolClassName::testForClassInstances(File * fp)
  {
    NodeBlockClass * saveclassBlock = m_state.m_classBlock;
    m_state.m_classBlock = getClassBlockNode();
    m_state.m_currentBlock = m_state.m_classBlock;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	SymbolClass::testThisClass(fp);
	m_state.m_classBlock = saveclassBlock; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	return;
      }

#ifdef _DEBUG_DONT_MERGE
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
#else
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
#endif
      {
	SymbolClass * csym = it->second;
	assert(csym);
	csym->testThisClass(fp); //this instance
	it++;
      }
    //restore
    m_state.m_compileThisIdx = m_utypeIdx;
    m_state.m_classBlock = saveclassBlock; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
  } //testForClassInstances

  void SymbolClassName::mergeClassInstancesBeforeCodeGen()
  {
#ifdef _DEBUG_DONT_MERGE
    return;
#endif

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      return;

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI cuti = it->first;
	std::string argstring = formatAnInstancesArgValuesAsAString(cuti);

	std::map<std::string, SymbolClass* >::iterator mit = m_scalarClassArgStringsToSymbolPtr.find(argstring);
	if(mit == m_scalarClassArgStringsToSymbolPtr.end())
	  {
	    //new entry, not owner of symbol class
	    m_scalarClassArgStringsToSymbolPtr.insert(std::pair<std::string, SymbolClass*>(argstring, csym));
	  }
	else
	  {
	    //sanity check, keys should be identical except for the classInstanceIdx
	    SymbolClass * msym = mit->second;
	    UTI muti = msym->getUlamTypeIdx();
	    assert(csym->getId() == msym->getId() && m_state.getBitSize(cuti) == m_state.getBitSize(muti) && m_state.getArraySize(cuti) == m_state.getArraySize(muti));
	  }
	it++;
      }
  } //mergeClassInstancesBeforeCodeGen

  void SymbolClassName::generateCodeForClassInstances(FileManager * fm)
  {
    NodeBlockClass * saveclassBlock = m_state.m_classBlock;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	NodeBlockClass * classNode = getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	SymbolClass::generateCode(fm);
	m_state.m_classBlock = saveclassBlock; //restore
	m_state.m_currentBlock = m_state.m_classBlock;
	return;
      }

#ifdef _DEBUG_DONT_MERGE
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
#else
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
#endif
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	m_state.m_compileThisIdx = suti;
	m_state.m_classBlock = csym->getClassBlockNode();
	m_state.m_currentBlock = m_state.m_classBlock;
	csym->generateCode(fm);
	it++;
      }
    //restore
    m_state.m_compileThisIdx = m_utypeIdx;
    m_state.m_classBlock = saveclassBlock; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
  } //generateCodeForClassInstances

  void SymbolClassName::generateIncludesForClassInstances(File * fp)
  {
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	return SymbolClass::generateAsOtherInclude(fp);
      }

#ifdef _DEBUG_DONT_MERGE
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
#else
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
#endif
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

#ifdef _DEBUG_DONT_MERGE
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
#else
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
#endif
      {
	SymbolClass * csym = it->second;
	csym->generateAsOtherForwardDef(fp);
	it++;
      }
  } //generateForwardDefsForClassInstances

  void SymbolClassName::generateTestInstanceForClassInstances(File * fp)
  {
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	SymbolClass::generateTestInstance(fp);
	return;
      }

#ifdef _DEBUG_DONT_MERGE
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
#else
    std::map<std::string, SymbolClass* >::iterator it = m_scalarClassArgStringsToSymbolPtr.begin();
    while(it != m_scalarClassArgStringsToSymbolPtr.end())
#endif
      {
	SymbolClass * csym = it->second;
	assert(csym);
	csym->generateTestInstance(fp);
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
	assert(m_state.alreadyDefinedSymbol(psym->getId(), asym));
	instancesArgs.push_back((SymbolConstantValue *) asym);
	pit++;
      } //next param

    NodeBlockClass * toclassblock = to->getClassBlockNode();
    m_state.m_classBlock = toclassblock;
    m_state.m_currentBlock = m_state.m_classBlock;

    //replace the clone's arg symbols
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	SymbolConstantValue * asym = new SymbolConstantValue(*instancesArgs[i]); //copy it
	u32 aid = asym->getId();
	//get 'instance's value save in template's parameter list temporarily
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

} //end MFM
