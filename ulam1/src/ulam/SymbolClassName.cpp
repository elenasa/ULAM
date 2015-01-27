#include "SymbolClassName.h"
#include "CompilerState.h"

namespace MFM {

  SymbolClassName::SymbolClassName(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClass(id, utype, classblock, state){}

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


  void SymbolClassName::checkAndLabelClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	classNode->checkAndLabelType();
	return;
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance

	takeAnInstancesArgValues(suti);

	classNode->checkAndLabelType(); //redo ???
	it++;
      }
    resetParameterValuesUnknown();
  } //checkAndLabelClassInstances


  bool SymbolClassName::setBitSizeOfClassInstances()
  {
    bool aok = true;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode); //infinite loop "Incomplete Class <> was never defined, fails sizing"
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;

    //check for class instances
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	s32 totalbits = 0;
	UTI cuti = getUlamTypeIdx();
	aok = trySetBitsizeWithUTIValues(cuti, totalbits);
	if(aok)
	  m_state.setBitSize(cuti, totalbits);  //"scalar" Class bitsize  KEY ADJUSTED
	return aok;
      }

    std::vector<UTI> lostClasses;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance

	//do we need to pretend to be the instance type too???
	takeAnInstancesArgValues(suti);
	s32 totalbits = 0;
	aok = trySetBitsizeWithUTIValues(suti, totalbits);

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
    resetParameterValuesUnknown();
    return aok;
  } //setBitSizeOfClassInstances()


  bool SymbolClassName::trySetBitsizeWithUTIValues(UTI suti, s32& totalbits)
  {
    NodeBlockClass * classNode = getClassBlockNode(); //main
    bool aok = true;
    if(isQuarkUnion())
      totalbits = classNode->getMaxBitSizeOfVariableSymbolsInTable(); //data members only
    else
      totalbits = classNode->getBitSizesOfVariableSymbolsInTable(); //data members only

    //check to avoid setting EMPTYSYMBOLTABLE instead of 0 for zero-sized classes
    if(totalbits == CYCLEFLAG)  // was < 0
      {
	std::ostringstream msg;
	msg << "cycle error!! " << m_state.getUlamTypeNameByIndex(suti).c_str();
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),DEBUG);
	    aok = false;
	  }
	else if(totalbits == EMPTYSYMBOLTABLE)
	  {
	    totalbits = 0;
	    aok = true;
	  }
	else if(totalbits != UNKNOWNSIZE)
	  aok = true;  //not UNKNOWN
    return aok;
  } //trySetBitSize


  // separate pass...after labeling all classes is completed;
  void SymbolClassName::printBitSizeOfClassInstances()
  {
    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	return printBitSizeOfClass(getUlamTypeIdx());
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance
	printBitSizeOfClass(suti);
	it++;
      }
  } //printBitSizeOfClassInstances

  void SymbolClassName::printBitSizeOfClass(UTI suti)
  {
    u32 total = m_state.getTotalBitSize(suti);
    UlamType * sut = m_state.getUlamTypeByIndex(suti);
    ULAMCLASSTYPE classtype = sut->getUlamClass();

    std::ostringstream msg;
    msg << "[UTBUA] Total bits used/available by " << (classtype == UC_ELEMENT ? "element <" : "quark <") << m_state.m_pool.getDataAsString(getId()).c_str() << ">: ";

    if(m_state.isComplete(suti))
      {
	s32 remaining = (classtype == UC_ELEMENT ? (MAXSTATEBITS - total) : (MAXBITSPERQUARK - total));
	msg << total << "/" << remaining;
      }
    else
      {
	total = UNKNOWNSIZE;
	s32 remaining = (classtype == UC_ELEMENT ? MAXSTATEBITS : MAXBITSPERQUARK);
	msg << "UNKNOWN" << "/" << remaining;
      }
    MSG("", msg.str().c_str(),INFO);
  } //printBitSizeOfClass


  void SymbolClassName::packBitsForClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;

    if(m_scalarClassInstanceIdxToSymbolPtr.empty())
      {
	return classNode->packBitsForVariableDataMembers();
      }

    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.begin();
    while(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	SymbolClass * csym = it->second;
	UTI suti = csym->getUlamTypeIdx(); //this instance

	takeAnInstancesArgValues(suti);

	classNode->packBitsForVariableDataMembers();
	it++;
      }
    resetParameterValuesUnknown();
  } //packBitsForClassInstances






  bool SymbolClassName::takeAnInstancesArgValues(UTI instance)
  {
    bool rtnb = false;
    std::map<UTI, SymbolClass* >::iterator it = m_scalarClassInstanceIdxToSymbolPtr.find(instance);
    if(it != m_scalarClassInstanceIdxToSymbolPtr.end())
      {
	assert(it->first == instance);
	SymbolClass * csym = it->second;
	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	//copy values into template's SymbolConstantValues
	std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
	while(pit != m_parameterSymbols.end())
	  {
	    SymbolConstantValue * psym = *pit;
	    //get 'instance's value
	    Symbol * asym = NULL;
	    assert(m_state.alreadyDefinedSymbol(psym->getId(), asym));
	    UTI auti = asym->getUlamTypeIdx();
	    if(m_state.getUlamTypeByIndex(auti)->getUlamTypeEnum() == Int)
	      {
		s32 sval;
		assert(((SymbolConstantValue *) asym)->getValue(sval));
		psym->setValue(sval);
	      }
	    else
	      {
		u32 uval;
		assert(((SymbolConstantValue *) asym)->getValue(uval));
		psym->setValue(uval);
	      }
	    pit++;
	  } //next param
	rtnb = true;
      }

    //restore
    m_state.m_classBlock = getClassBlockNode();
    m_state.m_currentBlock = m_state.m_classBlock;
    return rtnb;
  } //takeAnInstancesArgValues


  void SymbolClassName::resetParameterValuesUnknown()
  {
    std::vector<SymbolConstantValue *>::iterator pit = m_parameterSymbols.begin();
    while(pit != m_parameterSymbols.end())
      {
	SymbolConstantValue * psym = *pit;
	psym->setValue(UNKNOWNSIZE);
	pit++;
      } //next param
  } //resetParameterValues

} //end MFM
