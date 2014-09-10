#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableStack::SymbolVariableStack(u32 id, UTI utype, bool packed, s32 slot, CompilerState& state) : SymbolVariable(id, utype, packed, state), m_stackFrameSlotIndex(slot){}


  SymbolVariableStack::~SymbolVariableStack()
  {}


  s32 SymbolVariableStack::getStackFrameSlotIndex()
  {
    assert(!isDataMember());
    return m_stackFrameSlotIndex;
  }


  s32 SymbolVariableStack::getBaseArrayIndex()
  {
    return getStackFrameSlotIndex();
  }


  const std::string SymbolVariableStack::getMangledPrefix()
  {
    return "Uv_"; 
  }


  void SymbolVariableStack::generateCodedVariableDeclarations(File * fp, ULAMCLASSTYPE classtype)
  {
    assert(0);
    //not sure what this should do for local variables, if anything, 
  }

} //end MFM
