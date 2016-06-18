#include "MapDataMemberDesc.h"
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
    return m_hasVal;
  }

  std::string DataMemberDesc::getValueAsString() const
  {
    u32 uvals[ARRAY_LEN8K];
    m_val.ToArray(uvals);

    u32 nwords = (m_len + 31)/MAXBITSPERINT;

    //short-circuit if all zeros
    bool isZero = true;
    for(u32 x = 0; x < nwords; x++)
      {
	if(uvals[x] != 0)
	  {
	    isZero = false;
	    break;
	  }
      }

    if(isZero)
      return "0x0"; //nothing to do

    std::ostringstream dhex;
    dhex << "0x";
    for(u32 w = 0; w < nwords; w++)
      {
	dhex << std::hex << uvals[w];
      }
    return dhex.str();
  } //getValueAsString

} //MFM
