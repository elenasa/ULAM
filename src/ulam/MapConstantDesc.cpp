#include "MapConstantDesc.h"
#include "CompilerState.h"

namespace MFM {

  ConstantDesc::ConstantDesc(SymbolConstantValue * csym, UTI classtype, CompilerState & state) : ClassMemberDesc(csym, classtype, state)
  {
    csym->getValue(m_val); //must be ready!
    m_len = state.getTotalBitSize(csym->getUlamTypeIdx());
  }

  ConstantDesc::ConstantDesc(const ConstantDesc& cref) : ClassMemberDesc(cref), m_len(cref.m_len), m_val(cref.m_val) {}

  ConstantDesc::~ConstantDesc(){}

  ClassMemberDesc * ConstantDesc::clone() const
  {
    return new ConstantDesc(*this);
  }

  std::string ConstantDesc::getMemberKind() const
  {
    return "CONSTANT";
  }

  bool ConstantDesc::hasValue() const
  {
    return true;
  }

  std::string ConstantDesc::getValueAsString() const
  {
    std::string rtnstr;
    SymbolWithValue::getLexValueAsString(m_len, m_val, rtnstr);
    return rtnstr;
  }


} //MFM
