/* -*- c++ -*- */

#ifndef SYMBOLTYPEDEF_H
#define SYMBOLTYPEDEF_H

#include "UlamValue.h"
//#include "SymbolVariable.h"
#include "Symbol.h"

namespace MFM{

  //distinguish between Symbols
  //class SymbolTypedef : public SymbolVariable
  class SymbolTypedef : public Symbol
  {
  public:
    SymbolTypedef(u32 id, UlamType * utype);
    ~SymbolTypedef();

    virtual bool isTypedef();

  protected:

  private:

  };

}

#endif //end SYMBOLTYPEDEF_H
