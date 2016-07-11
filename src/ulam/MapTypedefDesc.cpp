#include "MapTypedefDesc.h"
#include "CompilerState.h"

namespace MFM {

  TypedefDesc::TypedefDesc(SymbolTypedef * tdsym, UTI classtype, CompilerState & state) : ClassMemberDesc(tdsym, classtype, state) {}

  TypedefDesc::TypedefDesc(const TypedefDesc & tref) : ClassMemberDesc(tref) {}

  TypedefDesc::~TypedefDesc(){}

  const ClassMemberDesc *  TypedefDesc::clone() const
  {
    return new TypedefDesc(*this);
  }

  std::string TypedefDesc::getMemberKind() const
  {
    return "TYPEDEF";
  }

} //MFM
