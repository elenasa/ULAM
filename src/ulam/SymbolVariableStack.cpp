#include "SymbolVariableStack.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableStack::SymbolVariableStack(const Token& id, UTI utype, CompilerState& state) : SymbolVariable(id, utype, state), m_stackFrameSlotIndex(0), m_autoStgTypeForEval(Nouti), m_isFunctionParameter(false), m_isConstantFunctionParameter(false) {}

  SymbolVariableStack::SymbolVariableStack(const Token& id, UTI utype, s32 slot, CompilerState& state) : SymbolVariable(id, utype, state), m_stackFrameSlotIndex(slot), m_autoStgTypeForEval(Nouti), m_isFunctionParameter(false), m_isConstantFunctionParameter(false) {}

  SymbolVariableStack::SymbolVariableStack(const SymbolVariableStack& sref) : SymbolVariable(sref), m_stackFrameSlotIndex(sref.m_stackFrameSlotIndex), m_autoStgTypeForEval(sref.m_autoStgTypeForEval), m_isFunctionParameter(sref.m_isFunctionParameter), m_isConstantFunctionParameter(sref.m_isConstantFunctionParameter) {}

  SymbolVariableStack::SymbolVariableStack(const SymbolVariableStack& sref, bool keepType) : SymbolVariable(sref, keepType), m_stackFrameSlotIndex(sref.m_stackFrameSlotIndex), m_autoStgTypeForEval(sref.m_autoStgTypeForEval), m_isFunctionParameter(sref.m_isFunctionParameter), m_isConstantFunctionParameter(sref.m_isConstantFunctionParameter) {}

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
    assert(m_stackFrameSlotIndex != 0);
    return m_stackFrameSlotIndex;
  }

  void SymbolVariableStack::setStackFrameSlotIndex(s32 slot)
  {
    assert(!isDataMember());
    assert(slot != 0); //> 0 local var; < 0 func param
    //assert(slot != m_stackFrameSlotIndex);
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

  bool SymbolVariableStack::isFunctionParameter()
  {
    return m_isFunctionParameter;
  }

  void SymbolVariableStack::setFunctionParameter()
  {
    m_isFunctionParameter = true;
  }

  bool SymbolVariableStack::isConstantFunctionParameter()
  {
    assert(isFunctionParameter());
    return m_isConstantFunctionParameter;
  }

  void SymbolVariableStack::setConstantFunctionParameter()
  {
    m_isConstantFunctionParameter = true;
  }



} //end MFM
