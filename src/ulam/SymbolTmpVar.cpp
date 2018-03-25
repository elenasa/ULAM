#include "SymbolTmpVar.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTmpVar::SymbolTmpVar(const Token& id, UTI utype, u32 offset, CompilerState & state) : Symbol(id, utype, state), m_posOffsetCopy(offset), m_divinedByConstantClass(false) {}

  SymbolTmpVar::~SymbolTmpVar() {}

  Symbol * SymbolTmpVar::clone()
  {
    m_state.abortShouldntGetHere();
    return new SymbolTmpVar(*this);
  }

  bool SymbolTmpVar::isTmpVarSymbol()
  {
    return true;
  }

  const std::string SymbolTmpVar::getMangledPrefix()
  {
    m_state.abortShouldntGetHere();
    return "UX";  //included in mangled name
  }

  const std::string SymbolTmpVar::getMangledName()
  {
    return m_state.m_pool.getDataAsString(getId());
  }

  u32 SymbolTmpVar::getPosOffset()
  {
    return m_posOffsetCopy; //included in its variable
  }

  bool SymbolTmpVar::isPosOffsetReliable()
  {
    return true; //??
  }

  void SymbolTmpVar::setDivinedByConstantClass()
  {
    m_divinedByConstantClass = true;
  }

  bool SymbolTmpVar::divinedByConstantClass()
  {
    //helps determine if an element data member of a transient TmpVar
    //needs its MFM Typefield fixed (t41267)
    return m_divinedByConstantClass;
  }

} //end MFM
