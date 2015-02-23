#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariable::SymbolVariable(u32 id, UTI utype, PACKFIT packed, CompilerState& state) : Symbol(id, utype, state), m_posOffset(0), m_packed(packed){}
  SymbolVariable::SymbolVariable(const SymbolVariable& sref) : Symbol(sref), m_posOffset(sref.m_posOffset), m_packed(sref.m_packed) {}
  SymbolVariable::~SymbolVariable() {}

  s32 SymbolVariable::getStackFrameSlotIndex()
  {
    assert(0);
    return 0;  //not on stack
  }

  u32 SymbolVariable::getDataMemberSlotIndex()
  {
    assert(0);
    return 0;  //not a data member
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

  PACKFIT SymbolVariable::isPacked()
  {
    return m_packed;
  }

  void SymbolVariable::setPacked(PACKFIT p)
  {
    m_packed = p;
  }

} //end MFM
