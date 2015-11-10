#include "MapDataMemberDesc.h"
#include "CompilerState.h"

namespace MFM {

  DataMemberDesc::DataMemberDesc(SymbolVariableDataMember * dmsym, UTI classtype, CompilerState & state) : ClassMemberDesc(dmsym, classtype, state), m_val(0)
  {
    if(dmsym->hasInitValue())
      dmsym->getInitValue(m_val); //optional
  }

  DataMemberDesc::~DataMemberDesc(){}

  std::string DataMemberDesc::getMemberKind()
  {
    return "DATAMEMBER";
  }

  bool DataMemberDesc::getValue(u64& vref)
  {
    vref = m_val;
    return true;
  }

} //MFM
