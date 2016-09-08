#include "SymbolTmpRef.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTmpRef::SymbolTmpRef(const Token& id, UTI utype, u32 offset, CompilerState & state) : Symbol(id, utype, state), m_posOffsetCopy(offset) {}

  SymbolTmpRef::~SymbolTmpRef() {}

  Symbol * SymbolTmpRef::clone()
  {
    return new SymbolTmpRef(*this);
  }

  bool SymbolTmpRef::isTmpRefSymbol()
  {
    return true;
  }

  const std::string SymbolTmpRef::getMangledPrefix()
  {
    m_state.abortShouldntGetHere();
    return "UX";  //included in mangled name
  }

  const std::string SymbolTmpRef::getMangledName()
  {
    return m_state.m_pool.getDataAsString(getId());
  } //getMangledName

  u32 SymbolTmpRef::getPosOffset()
  {
    return m_posOffsetCopy; //included in its variable
  }


} //end MFM
