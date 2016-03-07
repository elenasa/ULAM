#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariable::SymbolVariable(Token id, UTI utype, CompilerState& state) : Symbol(id, utype, state), m_posOffset(0){}

  SymbolVariable::SymbolVariable(const SymbolVariable& sref) : Symbol(sref), m_posOffset(sref.m_posOffset) {}

  SymbolVariable::~SymbolVariable() {}

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

  //packed bit position of data members; relative to ATOMFIRSTSTATEBITPOS
  u32 SymbolVariable::getPosOffset()
  {
    return m_posOffset;
  }

  void SymbolVariable::setPosOffset(u32 offsetIntoAtom)
  {
    m_posOffset = offsetIntoAtom;
  }

} //end MFM
