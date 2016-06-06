#include "MapDataMemberDesc.h"
#include "CompilerState.h"

namespace MFM {

  DataMemberDesc::DataMemberDesc(SymbolVariableDataMember * dmsym, UTI classtype, CompilerState & state) : ClassMemberDesc(dmsym, classtype, state), m_val(0)
  {
    if(dmsym->hasDefault())
      dmsym->getDefaultValue(m_val); //optional
  }

  DataMemberDesc::DataMemberDesc(const DataMemberDesc& dref) : ClassMemberDesc(dref), m_val(dref.m_val) {}

  DataMemberDesc::~DataMemberDesc(){}

  const ClassMemberDesc * DataMemberDesc::clone() const
  {
    return new DataMemberDesc(*this);
  }

  std::string DataMemberDesc::getMemberKind() const
  {
    return "DATAMEMBER";
  }

  bool DataMemberDesc::getValue(u64& vref) const
  {
    vref = m_val;
    return true;
  }

} //MFM
