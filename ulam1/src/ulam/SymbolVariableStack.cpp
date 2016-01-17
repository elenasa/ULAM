#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableStack::SymbolVariableStack(Token id, UTI utype, PACKFIT packed, s32 slot, CompilerState& state) : SymbolVariable(id, utype, packed, state), m_stackFrameSlotIndex(slot), m_autoStgTypeForEval(Nouti) {}

  SymbolVariableStack::SymbolVariableStack(const SymbolVariableStack& sref) : SymbolVariable(sref), m_stackFrameSlotIndex(sref.m_stackFrameSlotIndex), m_autoStgTypeForEval(sref.m_autoStgTypeForEval) {}

  SymbolVariableStack::~SymbolVariableStack() {}

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

  UlamValue SymbolVariableStack::getAutoPtrForEval()
  {
    return m_autoPtrForEval;
  }

  void SymbolVariableStack::setAutoPtrForEval(UlamValue ptr)
  {
    assert(isAutoLocal());
    m_autoPtrForEval = ptr;
  }

  void SymbolVariableStack::setAutoStorageTypeForEval(UTI uti)
  {
    assert(isAutoLocal());
    m_autoStgTypeForEval = uti;
  }

  UTI SymbolVariableStack::getAutoStorageTypeForEval()
  {
    return m_autoStgTypeForEval;
  }


} //end MFM
