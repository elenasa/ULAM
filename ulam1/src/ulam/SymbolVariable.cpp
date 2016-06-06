#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariable::SymbolVariable(Token id, UTI utype, CompilerState& state) : SymbolWithValue(id, utype, state), m_posOffset(0){}

  SymbolVariable::SymbolVariable(const SymbolVariable& sref) : SymbolWithValue(sref), m_posOffset(sref.m_posOffset){}

  SymbolVariable::SymbolVariable(const SymbolVariable& sref, bool keepType) : SymbolWithValue(sref, keepType), m_posOffset(sref.m_posOffset){}

  SymbolVariable::~SymbolVariable() {}

  bool SymbolVariable::isConstant()
  {
    return false;
  }

  s32 SymbolVariable::getStackFrameSlotIndex()
  {
    assert(0);
    return 0; //not on stack
  }

  void SymbolVariable::setStackFrameSlotIndex(s32 slot)
  {
    assert(0);
    return; //not on stack
  }

  u32 SymbolVariable::getDataMemberSlotIndex()
  {
    assert(0);
    return 0; //not a data member
  }

  //packed bit position of data members; relative to ATOMFIRSTSTATEBITPOS (or 0u).
  u32 SymbolVariable::getPosOffset()
  {
    return m_posOffset;
  }

  void SymbolVariable::setPosOffset(u32 offsetIntoAtom)
  {
    m_posOffset = offsetIntoAtom; //relative to first state bit
  }

} //end MFM
