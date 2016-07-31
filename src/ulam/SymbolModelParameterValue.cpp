#include "SymbolModelParameterValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolModelParameterValue::SymbolModelParameterValue(const Token& id, UTI utype, CompilerState & state) : SymbolWithValue(id, utype, state)
  {
    setDataMemberClass(m_state.getCompileThisIdx());
  }

  SymbolModelParameterValue::SymbolModelParameterValue(const SymbolModelParameterValue & sref) : SymbolWithValue(sref) {}

  SymbolModelParameterValue::SymbolModelParameterValue(const SymbolModelParameterValue & sref, bool keepType) : SymbolWithValue(sref, keepType) {}

  SymbolModelParameterValue::~SymbolModelParameterValue()
  { }

  Symbol * SymbolModelParameterValue::clone()
  {
    return new SymbolModelParameterValue(*this);
  }

  Symbol * SymbolModelParameterValue::cloneKeepsType()
  {
    return new SymbolModelParameterValue(*this, true);
  }

  bool SymbolModelParameterValue::isConstant()
  {
    return false;
  }

  bool SymbolModelParameterValue::isModelParameter()
  {
    return true; //not class parameter!
  }

  const std::string SymbolModelParameterValue::getMangledPrefix()
  {
    return "Up_";
  }

  void SymbolModelParameterValue::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    UTI tuti = getUlamTypeIdx();

    fp->write(" parameter");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(tuti).c_str());
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(getId()).c_str());
    fp->write(" = ");

    SymbolWithValue::printPostfixValue(fp);
    fp->write("; ");
  } //printPostfixValuesOfVariableDeclarations

  void SymbolModelParameterValue::setStructuredComment()
  {
    Token scTok;
    if(m_state.getStructuredCommentToken(scTok)) //and clears it
      {
	m_structuredCommentToken = scTok;
	m_gotStructuredCommentToken = true;
      }
  } //setStructuredComment

} //end MFM
