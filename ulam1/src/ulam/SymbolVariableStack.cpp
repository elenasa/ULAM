#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableStack::SymbolVariableStack(Token id, UTI utype, PACKFIT packed, s32 slot, CompilerState& state) : SymbolVariable(id, utype, packed, state), m_stackFrameSlotIndex(slot)
  {
    //if(getId() == m_state.m_pool.getIndexForDataString("self"))
    // setIsSelf();
  }

  SymbolVariableStack::SymbolVariableStack(const SymbolVariableStack& sref) : SymbolVariable(sref), m_stackFrameSlotIndex(sref.m_stackFrameSlotIndex) {}

  SymbolVariableStack::~SymbolVariableStack()
  {}

  Symbol *  SymbolVariableStack::clone()
  {
    return new SymbolVariableStack(*this);
  }

  s32 SymbolVariableStack::getStackFrameSlotIndex()
  {
    assert(!isDataMember());
    return m_stackFrameSlotIndex;
  }

  void SymbolVariableStack::setStackFrameSlotIndex(s32 slot)
  {
    assert(!isDataMember());
    m_stackFrameSlotIndex = slot;
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
