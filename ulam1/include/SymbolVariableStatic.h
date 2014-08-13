/* -*- c++ -*- */

#ifndef SYMBOLVARIABLESTATIC_H
#define SYMBOLVARIABLESTATIC_H

#include "UlamValue.h"
#include "SymbolVariable.h"

namespace MFM{

  class SymbolVariableStatic : public SymbolVariable
  {
  public:
    SymbolVariableStatic(u32 id, UlamType * utype, u32 slot);
    ~SymbolVariableStatic();

    virtual u32 getDataMemberSlotIndex();
    virtual UlamValue getUlamValue(CompilerState & m_state);
    virtual UlamValue getUlamValueToStoreInto();
    virtual UlamValue getUlamValueAt(s32 idx, CompilerState& m_state);
    virtual UlamValue getUlamValueAtToStoreInto(s32 idx, CompilerState& state);

    virtual s32 getBaseArrayIndex();

  protected:

  private:
    u32 m_dataMemberSlotIndex;
    virtual UlamValue makeUlamValuePtr();
    virtual UlamValue makeUlamValuePtrAt(u32 idx, CompilerState& state);

  };

}

#endif //end SYMBOLVARIABLESTATIC_H
