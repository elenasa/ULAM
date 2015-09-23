#include "MapConstantDesc.h"
#include "CompilerState.h"

namespace MFM {

  ConstantDesc::ConstantDesc(SymbolConstantValue * csym, UTI classtype, CompilerState & state) : ClassMemberDesc(csym, classtype, state)
  {
    csym->getValue(m_val); //must be ready!
  }

  ConstantDesc::~ConstantDesc(){}

  std::string ConstantDesc::getMemberKind()
  {
    return "CONSTANT";
  }

  bool ConstantDesc::getValue(u64& vref)
  {
    vref = m_val;
    return true;
  }


} //MFM
