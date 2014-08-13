/* -*- c++ -*- */

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <map>
#include "Symbol.h"
#include "itype.h"

namespace MFM{

  class SymbolTable
  {
  public:

    SymbolTable();
    ~SymbolTable();

    bool isInTable(u32 id, Symbol * & symptrref);
    void addToTable(u32 id, Symbol * s);

    Symbol * getSymbolPtr(u32 id);

    void labelTableOfFunctions();

    u32 getTableSize();

    u32 getTotalSymbolSize();

  protected:
    std::map<u32, Symbol* > m_idToSymbolPtr;

  private:



  };

}

#endif //end SYMBOL_H
