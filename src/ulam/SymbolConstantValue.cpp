#include "SymbolConstantValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolConstantValue::SymbolConstantValue(const Token& id, UTI utype, CompilerState & state) : SymbolWithValue(id, utype, state)
  {
    NodeBlockLocals * locals = m_state.findALocalScopeByNodeNo(this->getBlockNoOfST());
    if(locals != NULL)
      setLocalFilescopeDef(locals->getNodeType());
  }

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

  const std::string SymbolConstantValue::getMangledName()
  {
    if(m_state.isScalar(getUlamTypeIdx()))
      return Symbol::getMangledName();

    //local filescope constant arrays end with filescope name (e.g. _8Foo.ulam)
    std::ostringstream mangled;
    std::string nstr = m_state.getDataAsStringMangled(getId());
    mangled << getMangledPrefix() << nstr.c_str();

    if(isLocalFilescopeDef())
      {
	UTI locuti = getLocalFilescopeType();
	UlamType * locut = m_state.getUlamTypeByIndex(locuti);
	u32 classid = 0;
	AssertBool foundClassName = m_state.getClassNameFromFileName(locut->getUlamTypeNameOnly(), classid); //without trailing .ulam (no dots allowed)
	assert(foundClassName);
	mangled << "_" << m_state.getDataAsStringMangled(classid).c_str() << "4ulam"; //leximited
      }
    return mangled.str();
  } //getMangledName

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
    fp->write("; ");
  } //printPostfixValuesOfVariableDeclarations

  void SymbolConstantValue::setStructuredComment()
  {
    Token scTok;
    if(m_state.getStructuredCommentToken(scTok)) //and clears it
      {
	m_structuredCommentToken = scTok;
	m_gotStructuredCommentToken = true;
      }
  } //setStructuredComment

} //end MFM
