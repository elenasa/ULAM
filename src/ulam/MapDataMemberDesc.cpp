#include "MapDataMemberDesc.h"
#include "SymbolWithValue.h"
#include "CompilerState.h"

namespace MFM {

  DataMemberDesc::DataMemberDesc(SymbolVariableDataMember * dmsym, UTI classtype, CompilerState & state) : ClassMemberDesc(dmsym, classtype, state), m_hasVal(false)
  {
    if(dmsym->hasInitValue())
      {
	dmsym->getInitValue(m_val); //optional
	m_hasVal = true;
      }
    m_len = state.getTotalBitSize(dmsym->getUlamTypeIdx());
  }

  DataMemberDesc::DataMemberDesc(const DataMemberDesc& dref) : ClassMemberDesc(dref), m_hasVal(dref.m_hasVal), m_len(dref.m_len)
  {
    if(dref.m_hasVal)
      m_val = dref.m_val;
  }

  DataMemberDesc::~DataMemberDesc(){}

  const ClassMemberDesc * DataMemberDesc::clone() const
  {
    return new DataMemberDesc(*this);
  }

  std::string DataMemberDesc::getMemberKind() const
  {
    return "DATAMEMBER";
  }

  bool DataMemberDesc::hasValue() const
  {
    //zero if uninitialized
    return true; //return m_hasVal;
  }

  std::string DataMemberDesc::getValueAsString() const
  {
    if(!m_hasVal)
      return "10"; //non-initialized -> zero

    std::string rtnstr;
    SymbolWithValue::getLexValueAsString(m_len, m_val, rtnstr);
    return rtnstr;
  }

} //MFM
