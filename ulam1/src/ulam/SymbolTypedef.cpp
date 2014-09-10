#include "SymbolTypedef.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTypedef::SymbolTypedef(u32 id, UTI utype, CompilerState & state) : Symbol(id, utype, state){}

  SymbolTypedef::~SymbolTypedef()
  {}

  bool SymbolTypedef::isTypedef()
  {
    return true;
  }


  const std::string SymbolTypedef::getMangledPrefix()
  {
    return "Ut_";  //?
  }

} //end MFM
