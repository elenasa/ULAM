#include "SymbolVariable.h"

namespace MFM {

  SymbolVariable::SymbolVariable(u32 id, UlamType * utype) : Symbol(id, utype), m_posOffset(0){}

  SymbolVariable::~SymbolVariable()
  {}


  s32 SymbolVariable::getStackFrameSlotIndex()
  {
    assert(0);
    return 0;  //== not on stack
  }


  u32 SymbolVariable::getDataMemberSlotIndex()
  {
    assert(0);
    return 0;  //== not a dta member
  }


  u32 SymbolVariable::getPosOffset()
  {
    return m_posOffset;
  }


  void SymbolVariable::setPosOffset(u32 offsetIntoAtom)
  {
    m_posOffset = offsetIntoAtom;
  }


} //end MFM
