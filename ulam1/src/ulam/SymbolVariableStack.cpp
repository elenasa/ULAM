#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableStack::SymbolVariableStack(u32 id, UlamType * utype, s32 slot) : SymbolVariable(id, utype), m_stackFrameSlotIndex(slot){}


  SymbolVariableStack::~SymbolVariableStack()
  {}


  s32 SymbolVariableStack::getStackFrameSlotIndex()
  {
    assert(!isDataMember());
    return m_stackFrameSlotIndex;
  }


  UlamValue SymbolVariableStack::getUlamValue(CompilerState& m_state)
  {
    if(getUlamType()->isScalar())
      return m_state.m_funcCallStack.getFrameSlotAt(getStackFrameSlotIndex());

    return makeUlamValuePtr();
  }


  UlamValue SymbolVariableStack::getUlamValueToStoreInto()
  {
    return makeUlamValuePtr();
  }


  UlamValue SymbolVariableStack::getUlamValueAt(s32 idx, CompilerState& m_state)
  {
    assert(idx < (s32) getUlamType()->getArraySize() && idx >= 0);
    return m_state.m_funcCallStack.getFrameSlotAt(getStackFrameSlotIndex() + idx);
  }


  UlamValue SymbolVariableStack::getUlamValueAtToStoreInto(s32 idx, CompilerState& state)
  {
    assert(idx < (s32) getUlamType()->getArraySize() && idx >= 0);
    return makeUlamValuePtrAt(idx, state);
  }


  s32 SymbolVariableStack::getBaseArrayIndex()
  {
    return getStackFrameSlotIndex();
  }


  UlamValue SymbolVariableStack::makeUlamValuePtr()
  {
    return UlamValue(getUlamType(),getStackFrameSlotIndex(), true, STACK);
  }


  UlamValue SymbolVariableStack::makeUlamValuePtrAt(u32 idx, CompilerState& state)
  {
    assert(idx < getUlamType()->getArraySize() && idx >= 0);
    UlamValue tmpuv = getUlamValueAt(idx, state); //for scalar type
    
    return UlamValue(tmpuv.getUlamValueType(),getStackFrameSlotIndex()+idx, true, STACK);
  }


} //end MFM
