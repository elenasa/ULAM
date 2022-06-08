#include "SymbolTypedef.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTypedef::SymbolTypedef(const Token& id, UTI utype, UTI scalaruti, CompilerState & state) : Symbol(id, utype, state), m_scalarUTI(scalaruti), m_culamgenerated(false), m_culamgeneratedaliased(false) {}

  SymbolTypedef::SymbolTypedef(const SymbolTypedef& sref) : Symbol(sref), m_scalarUTI(Hzy), m_culamgenerated(sref.m_culamgenerated), m_culamgeneratedaliased(sref.m_culamgeneratedaliased)
  {
    if(isCulamGeneratedTypedef())
      {
	UTI sreftype = sref.getUlamTypeIdx();
	if(!isCulamGeneratedTypedefAliased())
	  {
	    assert(m_state.isHolder(sreftype));
	    UTI huti = m_state.makeUlamTypeHolder();
	    if(m_state.isAClass(sreftype))
	      {
		m_state.makeAnonymousClassFromHolder(huti, getLoc()); //t41013
	      }
	    resetUlamType(huti);
	  }
	else
	  resetUlamType(sreftype);
      }
  }

  SymbolTypedef::SymbolTypedef(const SymbolTypedef& sref, bool keeptype) : Symbol(sref, keeptype), m_scalarUTI(sref.m_scalarUTI), m_culamgenerated(sref.m_culamgenerated), m_culamgeneratedaliased(sref.m_culamgeneratedaliased) {}

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
    if(isCulamGeneratedTypedef()) return; //t41184

    UTI tuti = getUlamTypeIdx();
    UlamKeyTypeSignature tkey = m_state.getUlamKeyTypeSignatureByIndex(tuti);

    fp->write(" typedef ");

    //an array of refs as written, should be ref to an array.(t3666)
    fp->write(m_state.getUlamTypeNameBriefByIndex(tuti).c_str()); //includes &
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

  bool SymbolTypedef::isCulamGeneratedTypedef()
  {
    return m_culamgenerated;
  }

  void SymbolTypedef::setCulamGeneratedTypedef()
  {
    m_culamgenerated = true;
  }

  void SymbolTypedef::clearCulamGeneratedTypedef()
  {
    m_culamgenerated = false; //found user defined typedef
  }

  bool SymbolTypedef::isCulamGeneratedTypedefAliased()
  {
    return m_culamgeneratedaliased;
  }

  void SymbolTypedef::setCulamGeneratedTypedefAliased()
  {
    m_culamgeneratedaliased = true;
  }

} //end MFM
