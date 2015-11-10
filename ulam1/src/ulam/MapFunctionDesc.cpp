#include "MapFunctionDesc.h"
#include "CompilerState.h"

namespace MFM {

  FunctionDesc::FunctionDesc(SymbolFunction * fsym, UTI classtype, CompilerState & state) : ClassMemberDesc(fsym, classtype, state)
  {
    m_mangledMemberName = ""; //clears base init
    m_mangledMemberName = fsym->getMangledNameWithTypes();
  }

  FunctionDesc::~FunctionDesc() {}

  std::string FunctionDesc::getMemberKind()
  {
    return "FUNCTION";
  }


} //MFM
