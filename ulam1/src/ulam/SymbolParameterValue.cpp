#include "SymbolParameterValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolParameterValue::SymbolParameterValue(Token id, UTI utype, CompilerState & state) : SymbolWithValue(id, utype, state)
  {
    setDataMemberClass(m_state.getCompileThisIdx());
  }

  SymbolParameterValue::SymbolParameterValue(const SymbolParameterValue & sref) : SymbolWithValue(sref) {}

  SymbolParameterValue::SymbolParameterValue(const SymbolParameterValue & sref, bool keepType) : SymbolWithValue(sref, keepType) {}

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
    return "Up_";
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
    fp->write("; ");
  } //printPostfixValuesOfVariableDeclarations

  void SymbolParameterValue::setStructuredComment()
  {
    Token scTok;
    if(m_state.getStructuredCommentToken(scTok)) //and clears it
      {
	m_structuredCommentToken = scTok;
	m_gotStructuredCommentToken = true;
      }
  } //setStructuredComment

} //end MFM
