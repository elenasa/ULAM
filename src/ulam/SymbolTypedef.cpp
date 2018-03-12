#include "SymbolTypedef.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTypedef::SymbolTypedef(const Token& id, UTI utype, UTI scalaruti, CompilerState & state) : Symbol(id, utype, state), m_scalarUTI(scalaruti) {}

  SymbolTypedef::SymbolTypedef(const SymbolTypedef& sref) : Symbol(sref), m_scalarUTI(m_state.mapIncompleteUTIForCurrentClassInstance(sref.m_scalarUTI,sref.getLoc())) {}

  SymbolTypedef::SymbolTypedef(const SymbolTypedef& sref, bool keeptype) : Symbol(sref, keeptype), m_scalarUTI(sref.m_scalarUTI) {}

  SymbolTypedef::~SymbolTypedef() {}

  Symbol * SymbolTypedef::clone()
  {
    return new SymbolTypedef(*this);
  }

  Symbol * SymbolTypedef::cloneKeepsType()
  {
    return new SymbolTypedef(*this, true);
  }

  bool SymbolTypedef::isTypedef()
  {
    return true;
  }

  UTI SymbolTypedef::getScalarUTI()
  {
    return m_scalarUTI;
  }

  const std::string SymbolTypedef::getMangledPrefix()
  {
    return "Ut_";  //?
  }

  // replaces NodeTypedef:printPostfix to learn the values of Class' storage in center site
  void SymbolTypedef::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    if(getId() == m_state.m_pool.getIndexForDataString("Self")) return;
    if(getId() == m_state.m_pool.getIndexForDataString("Super")) return;

    UTI tuti = getUlamTypeIdx();
    UlamKeyTypeSignature tkey = m_state.getUlamKeyTypeSignatureByIndex(tuti);
    UlamType * tut = m_state.getUlamTypeByIndex(tuti);

    fp->write(" typedef");

    fp->write(" ");
    if(tut->getUlamTypeEnum() != Class)
      {
      fp->write(tkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
      if(tut->isAltRefType())
	fp->write(" &"); //an array of refs as written, should be ref to an array.
      }
    else
      fp->write(tut->getUlamTypeClassNameBrief(tuti).c_str());

    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(getId()).c_str());

    s32 arraysize = m_state.getArraySize(tuti);
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }

    fp->write("; ");
  } //printPostfixValuesOfVariableDeclarations

  void SymbolTypedef::setStructuredComment()
  {
    Token scTok;
    if(m_state.getStructuredCommentToken(scTok)) //and clears it
      {
	m_structuredCommentToken = scTok;
	m_gotStructuredCommentToken = true;
      }
  } //setStructuredComment

} //end MFM
