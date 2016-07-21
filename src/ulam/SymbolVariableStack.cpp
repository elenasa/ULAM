#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableStack::SymbolVariableStack(const Token& id, UTI utype, s32 slot, CompilerState& state) : SymbolVariable(id, utype, state), m_stackFrameSlotIndex(slot), m_autoStgTypeForEval(Nouti), m_declnno(0) {}

  SymbolVariableStack::SymbolVariableStack(const SymbolVariableStack& sref) : SymbolVariable(sref), m_stackFrameSlotIndex(sref.m_stackFrameSlotIndex), m_autoStgTypeForEval(sref.m_autoStgTypeForEval), m_declnno(sref.m_declnno) {}

  SymbolVariableStack::SymbolVariableStack(const SymbolVariableStack& sref, bool keepType) : SymbolVariable(sref, keepType), m_stackFrameSlotIndex(sref.m_stackFrameSlotIndex), m_autoStgTypeForEval(sref.m_autoStgTypeForEval), m_declnno(sref.m_declnno) {}

  SymbolVariableStack::~SymbolVariableStack() {}

  Symbol *  SymbolVariableStack::clone()
  {
    return new SymbolVariableStack(*this);
  }

  Symbol * SymbolVariableStack::cloneKeepsType()
  {
    return new SymbolVariableStack(*this, true);
  }

  s32 SymbolVariableStack::getStackFrameSlotIndex()
  {
    assert(!isDataMember());
    return m_stackFrameSlotIndex;
  }

  NNO SymbolVariableStack::getDeclNodeNo()
  {
    assert(!isDataMember());
    return m_declnno;
  }

  void SymbolVariableStack::setDeclNodeNo(NNO nno)
  {
    assert(!isDataMember());
    m_declnno = nno;
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
    if(getAutoLocalType() == ALT_REF)
      return "Ur_";
    return "Uv_";
  }

  void SymbolVariableStack::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    Symbol::printPostfixValuesOfVariableDeclarations(fp, slot, startpos, classtype); //pure in SymbolWithValue
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
