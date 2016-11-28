#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariable::SymbolVariable(const Token& id, UTI utype, CompilerState& state) : SymbolWithValue(id, utype, state){ }

  SymbolVariable::SymbolVariable(const SymbolVariable& sref) : SymbolWithValue(sref){ }

  SymbolVariable::SymbolVariable(const SymbolVariable& sref, bool keepType) : SymbolWithValue(sref, keepType){ }

  SymbolVariable::~SymbolVariable() {}

  bool SymbolVariable::isConstant()
  {
    return false;
  }

  s32 SymbolVariable::getStackFrameSlotIndex()
  {
    m_state.abortShouldntGetHere();
    return 0; //not on stack
  }

  void SymbolVariable::setStackFrameSlotIndex(s32 slot)
  {
    m_state.abortShouldntGetHere();
    return; //not on stack
  }

  u32 SymbolVariable::getDataMemberSlotIndex()
  {
    m_state.abortShouldntGetHere();
    return 0; //not a data member
  }

  u32 SymbolVariable::getPosOffset()
  {
    m_state.abortShouldntGetHere();
    return 0; //only data members
  }

} //end MFM
