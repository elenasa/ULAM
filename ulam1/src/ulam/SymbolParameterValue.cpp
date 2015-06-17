#include "SymbolParameterValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolParameterValue::SymbolParameterValue(Token id, UTI utype, CompilerState & state) : SymbolWithValue(id, utype, state), m_childOf(m_state.getCompileThisIdx())
  {
    setDataMember();
  }

  SymbolParameterValue::SymbolParameterValue(const SymbolParameterValue & sref) : SymbolWithValue(sref), m_childOf(m_state.getCompileThisIdx()) {}

  SymbolParameterValue::SymbolParameterValue(const SymbolParameterValue & sref, bool keepType) : SymbolWithValue(sref, keepType), m_childOf(m_state.getCompileThisIdx()) {}

  SymbolParameterValue::~SymbolParameterValue()
  { }

  Symbol * SymbolParameterValue::clone()
  {
    return new SymbolParameterValue(*this);
  }

  Symbol * SymbolParameterValue::cloneKeepsType()
  {
    return new SymbolParameterValue(*this, true);
  }

  bool SymbolParameterValue::isConstant()
  {
    return false;
  }

  bool SymbolParameterValue::isModelParameter()
  {
    return true;
  }

  const std::string SymbolParameterValue::getMangledPrefix()
  {
    //mangled "prefix" includes Uc_ + parent class' mangled name +
    // its prefix before its mangled name; this keeps Model Parameters
    // unique for each distinct Ulam Class, but the same for each
    // C++ usage.
    std::ostringstream mangled;
    mangled << "Uc_";
    mangled << m_state.getUlamTypeByIndex(m_childOf)->getUlamTypeMangledName().c_str();
    mangled << "_Up_";
    return mangled.str();
  }

  void SymbolParameterValue::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    UTI tuti = getUlamTypeIdx();

    fp->write(" parameter");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(tuti).c_str());
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(getId()).c_str());
    fp->write(" = ");

    SymbolWithValue::printPostfixValue(fp);
  } //printPostfixValuesOfVariableDeclarations

} //end MFM
