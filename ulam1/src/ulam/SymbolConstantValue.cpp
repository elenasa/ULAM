#include "SymbolConstantValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolConstantValue::SymbolConstantValue(Token id, UTI utype, CompilerState & state) : SymbolWithValue(id, utype, state) {}

  SymbolConstantValue::SymbolConstantValue(const SymbolConstantValue & sref) : SymbolWithValue(sref) {}

  SymbolConstantValue::SymbolConstantValue(const SymbolConstantValue & sref, bool keepType) : SymbolWithValue(sref, keepType) {}

  SymbolConstantValue::~SymbolConstantValue()
  { }

  Symbol * SymbolConstantValue::clone()
  {
    return new SymbolConstantValue(*this);
  }

  Symbol * SymbolConstantValue::cloneKeepsType()
  {
    return new SymbolConstantValue(*this, true);
  }

  bool SymbolConstantValue::isConstant()
  {
    return true;
  }

  const std::string SymbolConstantValue::getMangledPrefix()
  {
    return "Uc_";
  }

  // replaces NodeConstantValue:printPostfix to learn the values of Class' storage in center site
  void SymbolConstantValue::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    UTI tuti = getUlamTypeIdx();

    fp->write(" constant");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(tuti).c_str());
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(getId()).c_str());
    fp->write(" = ");

    SymbolWithValue::printPostfixValue(fp);
  } //printPostfixValuesOfVariableDeclarations

} //end MFM
