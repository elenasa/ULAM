#include "SymbolClassName.h"
#include "SymbolTable.h"
#include "CompilerState.h"


namespace MFM {

  SymbolClassName::SymbolClassName(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClass(id, utype, classblock, state), m_classInstanceST(state){}

  SymbolClassName::~SymbolClassName()
  {
    // symbols belong to  NodeBlockClass's ST; deleted there.
    m_parameterSymbols.clear();
  }

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


  bool SymbolClassName::isClassInstanceInTable(UTI uti, SymbolClass * & symptrref)
  {
    return m_classInstanceST.isInTable(uti, (Symbol * &) symptrref);
  }


  void SymbolClassName::addClassInstanceToTable(UTI uti, SymbolClass * symptr)
  {
    m_classInstanceST.addToTable(uti, symptr);
  }


} //end MFM
