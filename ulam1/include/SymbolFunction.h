/* -*- c++ -*- */

#ifndef SYMBOLFUNCTION_H
#define SYMBOLFUNCTION_H

#include <vector>
#include "Symbol.h"

namespace MFM{

  class NodeBlockFunction;  //forward

  class SymbolFunction : public Symbol
  {
  public:
    SymbolFunction(u32 id, UlamType * typetoreturn);
    ~SymbolFunction();

    void addParameterSymbol(Symbol * argSym);
    u32 getNumberOfParameters();    
    u32 getTotalSizeOfParameters();

    Symbol * getParameterSymbolPtr(u32 n);

    virtual bool isFunction();
    void setFunctionNode(NodeBlockFunction * func);
    NodeBlockFunction *  getFunctionNode();

  protected:

  private:
    std::vector<Symbol *> m_parameterSymbols;  // variable or function can be an args
    NodeBlockFunction * m_functionNode;
  };

}

#endif //end SYMBOL_H
