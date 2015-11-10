#include "MapTypedefDesc.h"
#include "CompilerState.h"

namespace MFM {

  TypedefDesc::TypedefDesc(SymbolTypedef * tdsym, UTI classtype, CompilerState & state) : ClassMemberDesc(tdsym, classtype, state) {}

  TypedefDesc::~TypedefDesc(){}

  std::string TypedefDesc::getMemberKind()
  {
    return "TYPEDEF";
  }

} //MFM
