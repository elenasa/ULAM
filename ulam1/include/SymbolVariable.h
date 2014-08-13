/* -*- c++ -*- */

#ifndef SYMBOLVARIABLE_H
#define SYMBOLVARIABLE_H

#include "UlamValue.h"
#include "Symbol.h"

namespace MFM{

  class CompilerState;  //forward

  //distinguish between SymbolFunction
  class SymbolVariable : public Symbol
  {
  public:
    SymbolVariable(u32 id, UlamType * utype);
    ~SymbolVariable();

    virtual s32 getStackFrameSlotIndex();
    virtual u32 getDataMemberSlotIndex();

    virtual UlamValue getUlamValue(CompilerState & m_state) = 0;
    virtual UlamValue getUlamValueToStoreInto() = 0;
    virtual UlamValue getUlamValueAt(s32 idx, CompilerState& m_state) = 0;
    virtual UlamValue getUlamValueAtToStoreInto(s32 idx, CompilerState& state) = 0;
    virtual s32 getBaseArrayIndex() = 0;

  protected:

  private:
    virtual UlamValue makeUlamValuePtr() = 0;
    virtual UlamValue makeUlamValuePtrAt(u32 idx, CompilerState& state) = 0;
  };

}

#endif //end SYMBOLVARIABLE_H
