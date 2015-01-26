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

} //end MFM
