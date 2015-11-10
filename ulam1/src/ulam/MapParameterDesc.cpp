#include "MapParameterDesc.h"
#include "CompilerState.h"

namespace MFM {

  ParameterDesc::ParameterDesc(SymbolParameterValue * psym, UTI classtype, CompilerState & state) : ClassMemberDesc(psym, classtype, state)
  {
    psym->getValue(m_val); //must be ready!
  }

  ParameterDesc::~ParameterDesc(){}

  std::string ParameterDesc::getMemberKind()
  {
    return "PARAMETER";
  }

  bool ParameterDesc::getValue(u64& vref)
  {
    vref = m_val;
    return true;
  }


} //MFM
