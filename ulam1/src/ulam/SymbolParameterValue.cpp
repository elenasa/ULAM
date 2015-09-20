#include "SymbolParameterValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolParameterValue::SymbolParameterValue(Token id, UTI utype, CompilerState & state) : SymbolWithValue(id, utype, state), m_childOf(m_state.getCompileThisIdx()), m_gotStructuredCommentToken(false)
  {
    setDataMember();
  }

  SymbolParameterValue::SymbolParameterValue(const SymbolParameterValue & sref) : SymbolWithValue(sref), m_childOf(m_state.getCompileThisIdx()), m_structuredCommentToken(sref.m_structuredCommentToken), m_gotStructuredCommentToken(sref.m_gotStructuredCommentToken) {}

  SymbolParameterValue::SymbolParameterValue(const SymbolParameterValue & sref, bool keepType) : SymbolWithValue(sref, keepType), m_childOf(m_state.getCompileThisIdx()), m_structuredCommentToken(sref.m_structuredCommentToken), m_gotStructuredCommentToken(sref.m_gotStructuredCommentToken) {}

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

  bool SymbolParameterValue::getStructuredComment(Token& scTok)
  {
    if(m_gotStructuredCommentToken)
      {
	assert(m_structuredCommentToken.m_type == TOK_STRUCTURED_COMMENT);
	scTok = m_structuredCommentToken;
	return true;
      }
    return false;
  } //getStructuredComment

} //end MFM
