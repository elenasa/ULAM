#include "SymbolTmpRef.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTmpRef::SymbolTmpRef(Token id, UTI utype, CompilerState & state) : Symbol(id, utype, state){}

  SymbolTmpRef::~SymbolTmpRef() {}

  Symbol * SymbolTmpRef::clone()
  {
    return new SymbolTmpRef(*this);
  }

  const std::string SymbolTmpRef::getMangledPrefix()
  {
    assert(0);
    return "UX";  //included in mangled name
  }

  const std::string SymbolTmpRef::getMangledName()
  {
    return m_state.m_pool.getDataAsString(getId());
  } //getMangledName

  u32 SymbolTmpRef::getPosOffset()
  {
    return 0; //included in its variable
  }

} //end MFM
