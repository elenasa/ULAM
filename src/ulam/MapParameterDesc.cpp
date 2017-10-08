#include "MapParameterDesc.h"
#include "CompilerState.h"

namespace MFM {

  ParameterDesc::ParameterDesc(SymbolModelParameterValue * psym, UTI classtype, CompilerState & state) : ClassMemberDesc(psym, classtype, state)
  {
    psym->getValue(m_val); //must be ready!
  }

  ParameterDesc::ParameterDesc(const ParameterDesc & pref) : ClassMemberDesc(pref), m_val(pref.m_val) {}

  ParameterDesc::~ParameterDesc(){}

  const ClassMemberDesc *  ParameterDesc::clone() const
  {
    return new ParameterDesc(*this);
  }

  std::string ParameterDesc::getMemberKind() const
  {
    return "PARAMETER";
  }

  bool ParameterDesc::getValue(u64& vref) const
  {
    vref = m_val;
    return true;
  }

  bool ParameterDesc::hasValue() const
  {
    return true;
  }

  std::string ParameterDesc::getValueAsString() const
  {
    std::ostringstream dhex;
    dhex << "0x" << std::hex << m_val;
    return dhex.str();
  }

} //MFM
