#include "SymbolFunction.h"
#include "NodeBlockFunction.h"
#include "SymbolVariable.h"

namespace MFM {

  SymbolFunction::SymbolFunction(u32 id, UlamType * typetoreturn) : Symbol(id,typetoreturn), m_functionNode(NULL) 
  { 
    setDataMember(); // by definition all function definitions are data members
  }

  SymbolFunction::~SymbolFunction()
  {
    delete m_functionNode; 
    // symbols belong to  NodeBlockFunction's ST; deleted there.
    m_parameterSymbols.clear();
  }


  bool SymbolFunction::isFunction()
  {
    return true;
  }


  void SymbolFunction::addParameterSymbol(Symbol * sym)
  {
    m_parameterSymbols.push_back(sym);
  }


  u32 SymbolFunction::getNumberOfParameters()
  {
    return m_parameterSymbols.size();
  }


  u32 SymbolFunction::getTotalSizeOfParameters()
  {
    u32 totalsizes = 0;
    for(u32 i = 0; i < m_parameterSymbols.size(); i++)
      {
	Symbol * sym = m_parameterSymbols[i];
	u32 arraysize = sym->getUlamType()->getArraySize();
	totalsizes += (arraysize > 0 ? arraysize : 1);
      }
    return totalsizes; 
  }


  Symbol * SymbolFunction::getParameterSymbolPtr(u32 n)
  {
    assert(n < m_parameterSymbols.size());
    return m_parameterSymbols[n];
  }


  void SymbolFunction::setFunctionNode(NodeBlockFunction * func)
  {
    if(m_functionNode)
      delete m_functionNode;  //clean up any previous declarations

    m_functionNode = func;
  }


  NodeBlockFunction *  SymbolFunction::getFunctionNode()
  {
    return m_functionNode;
  }

} //end MFM
