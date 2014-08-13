/* -*- c++ -*- */

#ifndef SYMBOLVARIABLESTACK_H
#define SYMBOLVARIABLESTACK_H

#include "UlamValue.h"
#include "SymbolVariable.h"

namespace MFM{

  class SymbolVariableStack : public SymbolVariable
  {
  public:
    SymbolVariableStack(u32 id, UlamType * utype, s32 slot);
    ~SymbolVariableStack();

    virtual s32 getStackFrameSlotIndex();
    virtual UlamValue getUlamValue(CompilerState & m_state);
    virtual UlamValue getUlamValueToStoreInto();
    virtual UlamValue getUlamValueAt(s32 idx, CompilerState& m_state);
    virtual UlamValue getUlamValueAtToStoreInto(s32 idx, CompilerState& state);

    virtual s32 getBaseArrayIndex();

  protected:

  private:
    s32 m_stackFrameSlotIndex;
    virtual UlamValue makeUlamValuePtr();
    virtual UlamValue makeUlamValuePtrAt(u32 idx, CompilerState& state);
  };

}

#endif //end SYMBOLVARIABLESTACK_H
