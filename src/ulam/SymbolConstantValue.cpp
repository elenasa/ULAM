#include "SymbolConstantValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolConstantValue::SymbolConstantValue(const Token& id, UTI utype, CompilerState & state) : SymbolWithValue(id, utype, state), m_constantStackFrameAbsoluteSlotIndex(0)
  {
    NodeBlockLocals * localsblock = m_state.findALocalsScopeByNodeNo(this->getBlockNoOfST());
    if(localsblock != NULL)
      setLocalsFilescopeDef(localsblock->getNodeType());
  }

  SymbolConstantValue::SymbolConstantValue(const SymbolConstantValue & sref) : SymbolWithValue(sref), m_constantStackFrameAbsoluteSlotIndex(sref.m_constantStackFrameAbsoluteSlotIndex) {}

  SymbolConstantValue::SymbolConstantValue(const SymbolConstantValue & sref, bool keepType) : SymbolWithValue(sref, keepType), m_constantStackFrameAbsoluteSlotIndex(sref.m_constantStackFrameAbsoluteSlotIndex) {}

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

    std::ostringstream mangled;
    std::string nstr = m_state.getDataAsStringMangled(getId());
    mangled << getMangledPrefix() << nstr.c_str();
    return mangled.str();
  } //getMangledName

  const std::string SymbolConstantValue::getCompleteConstantMangledName()
  {
    if(m_state.isScalar(getUlamTypeIdx()))
      return getMangledName();

    std::ostringstream mangledfullname;
    if(isDataMember())
      {
	UTI dmclassuti = getDataMemberClass(); //t3881
	mangledfullname << m_state.getTheInstanceMangledNameByIndex(dmclassuti).c_str();
	mangledfullname << "." << getMangledName().c_str();
      }
    else if(isLocalsFilescopeDef())
      {
	//local filescope constant arrays end with filescope name (e.g. _3Foo4ulam)
	UTI locuti = getLocalsFilescopeType();
	u32 mangledclassid = m_state.getMangledClassNameIdForUlamLocalsFilescope(locuti);
	mangledfullname << m_state.m_pool.getDataAsString(mangledclassid).c_str();
	mangledfullname << "<EC>::THE_INSTANCE." << getMangledName().c_str();
      }
    else if(isClassArgument())
      {
	mangledfullname << m_state.getTheInstanceMangledNameByIndex(m_state.getCompileThisIdx()).c_str();
	mangledfullname << "." << getMangledName().c_str(); //t3894
      }
    else
      mangledfullname << getMangledName().c_str();
    return mangledfullname.str();
  } //getCompleteConstantMangledName

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

    if(m_state.isAClass(tuti))
      {
	std::string classhexstr;
	SymbolWithValue::getClassValueAsHexString(classhexstr); //t41277
	fp->write("{ ");
	fp->write(classhexstr.c_str());
	fp->write(" }");
      }
    else
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

  void SymbolConstantValue::setConstantStackFrameAbsoluteSlotIndex(u32 slot)
  {
    //    assert(!m_state.isScalar(getUlamTypeIdx()) || m_state.isAClass(getUlamTypeIdx()) );
    assert(slot > 0);
    m_constantStackFrameAbsoluteSlotIndex = slot;
  }

  u32 SymbolConstantValue::getConstantStackFrameAbsoluteSlotIndex()
  {
    // assert(!m_state.isScalar(getUlamTypeIdx()) || m_state.isAClass(getUlamTypeIdx()) );
    return m_constantStackFrameAbsoluteSlotIndex;
  }

} //end MFM
