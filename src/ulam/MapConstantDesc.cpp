#include "MapConstantDesc.h"
#include "CompilerState.h"

namespace MFM {

  ConstantDesc::ConstantDesc(SymbolConstantValue * csym, UTI classtype, CompilerState & state) : ClassMemberDesc(csym, classtype, state)
  {
    csym->getValue(m_val); //must be ready!
  }

  ConstantDesc::ConstantDesc(const ConstantDesc& cref) : ClassMemberDesc(cref), m_val(cref.m_val) {}

  ConstantDesc::~ConstantDesc(){}

  ClassMemberDesc * ConstantDesc::clone() const
  {
    return new ConstantDesc(*this);
  }

  std::string ConstantDesc::getMemberKind() const
  {
    return "CONSTANT";
  }

  bool ConstantDesc::getValue(u64& vref) const
  {
    vref = m_val;
    return true;
  }

  bool ConstantDesc::hasValue() const
  {
    return true;
  }

  std::string ConstantDesc::getValueAsString() const
  {
    std::ostringstream dhex;
    dhex << " 0x" << std::hex << m_val;
    return dhex.str();
  }


} //MFM
