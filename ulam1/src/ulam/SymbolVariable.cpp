#include "SymbolVariable.h"

namespace MFM {

  SymbolVariable::SymbolVariable(u32 id, UlamType * utype) : Symbol(id, utype){}

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

} //end MFM
