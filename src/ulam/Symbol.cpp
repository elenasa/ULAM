#include <sstream>
#include <string.h>
#include "CompilerState.h"
#include "Symbol.h"
#include "Util.h"

namespace MFM {

  Symbol::Symbol(const Token& id, UTI utype, CompilerState & state) : m_state(state), m_gotStructuredCommentToken(false), m_idtok(id), m_uti(utype), m_dataMemberClass(Nouti), m_localsfilescopeType(Nouti), m_autoLocalType(ALT_NOT), m_isSelf(false), m_isSuper(false), m_stBlockNo(state.getCurrentBlockNo()){}

  Symbol::Symbol(const Symbol & sref) : m_state(sref.m_state), m_structuredCommentToken(sref.m_structuredCommentToken), m_gotStructuredCommentToken(sref.m_gotStructuredCommentToken), m_idtok(sref.m_idtok), m_uti(m_state.mapIncompleteUTIForCurrentClassInstance(sref.m_uti,sref.getLoc())), m_dataMemberClass(m_state.mapIncompleteUTIForCurrentClassInstance(sref.m_dataMemberClass,sref.getLoc())), m_localsfilescopeType(sref.m_localsfilescopeType), m_autoLocalType(sref.m_autoLocalType), m_isSelf(sref.m_isSelf), m_isSuper(sref.m_isSuper), m_stBlockNo(sref.m_stBlockNo) {}

  Symbol::Symbol(const Symbol& sref, bool keepType) : m_state(sref.m_state), m_structuredCommentToken(sref.m_structuredCommentToken), m_gotStructuredCommentToken(sref.m_gotStructuredCommentToken), m_idtok(sref.m_idtok), m_uti(sref.m_uti), m_dataMemberClass(sref.m_dataMemberClass), m_localsfilescopeType(sref.m_localsfilescopeType), m_autoLocalType(sref.m_autoLocalType), m_isSelf(sref.m_isSelf), m_isSuper(sref.m_isSuper), m_stBlockNo(sref.m_stBlockNo) {}

  Symbol::~Symbol(){}

  Symbol * Symbol::cloneKeepsType()
  {
    m_state.abortShouldntGetHere();
    return NULL;
  }

  void Symbol::setId(u32 newid)
  {
    m_idtok.m_dataindex = newid; //protected
  }

  void Symbol::resetIdToken(const Token& newtok)
  {
    m_idtok = newtok; //id & loc
  }

  u32 Symbol::getId()
  {
    return m_state.getTokenDataAsStringId(m_idtok);
  }

  Locator Symbol::getLoc()
  {
    return m_idtok.m_locator;
  }

  Locator Symbol::getLoc() const
  {
    return m_idtok.m_locator;
  }

  Token* Symbol::getTokPtr()
  {
    return &m_idtok;
  }

  void Symbol::resetUlamType(UTI newuti) //e.g. mappedUTI, fix _N class args
  {
    //assert((m_state.getReferenceType(m_uti) == m_state.getReferenceType(newuti)) || m_state.isHolder(m_uti)); Fri Jun 24 15:41:16 2016 (e.g. t3816)
    m_uti = newuti;
    setAutoLocalType(m_state.getReferenceType(newuti));
  }

  UTI Symbol::getUlamTypeIdx()
  {
    return m_uti;
  }

  u32 Symbol::getPosOffset()
  {
    m_state.abortShouldntGetHere();
    return 0; //data members only, incl.symbolparametervalue,tmprefsymbol
  }

  bool Symbol::isPosOffsetReliable()
  {
    m_state.abortShouldntGetHere();
    return false; //data members after packed bits.
  }

  bool Symbol::isFunction()
  {
    return false;
  }

  bool Symbol::isFunctionParameter()
  {
    return false;
  }

  bool Symbol::isTypedef()
  {
    return false;
  }

  bool Symbol::isConstant()
  {
    return false;
  }

  bool Symbol::isClass()
  {
    return false;
  }

  bool Symbol::isTmpVarSymbol()
  {
    return false;
  }

  void Symbol::setDataMemberClass(UTI cuti)
  {
    m_dataMemberClass = cuti;
  }

  UTI Symbol::getDataMemberClass()
  {
    return m_dataMemberClass;
  }

  bool Symbol::isDataMember()
  {
    return (m_dataMemberClass != Nouti);
  }

  void Symbol::setLocalsFilescopeDef(UTI locuti)
  {
    m_localsfilescopeType = locuti;
  }

  UTI Symbol::getLocalsFilescopeType()
  {
    assert(isLocalsFilescopeDef());
    return m_localsfilescopeType;
  }

  bool Symbol::isLocalsFilescopeDef()
  {
    return (m_localsfilescopeType != Nouti);
  }

  bool Symbol::isModelParameter()
  {
    return false;
  }

  void Symbol::setAutoLocalType(const Token& cTok)
  {
    if(cTok.m_type == TOK_KW_AS)
      setAutoLocalType(ALT_AS);
    else if(cTok.m_type == TOK_AMP)
      setAutoLocalType(ALT_REF);
    else
      setAutoLocalType(ALT_NOT);
  } //setAutoLocalType (helper)

  void Symbol::setAutoLocalType(ALT alt)
  {
    assert(m_state.getUlamTypeByIndex(getUlamTypeIdx())->getReferenceType() == alt);
    m_autoLocalType = alt;
  }

  ALT Symbol::getAutoLocalType()
  {
    return m_autoLocalType;
  }

  bool Symbol::isAutoLocal()
  {
    return (m_autoLocalType != ALT_NOT);
  }

  void Symbol::setIsSelf()
  {
    m_isSelf = true;
  }

  bool Symbol::isSelf()
  {
    return m_isSelf;
  }

  void Symbol::setIsSuper()
  {
    m_isSuper = true;
  }

  bool Symbol::isSuper()
  {
    return m_isSuper;
  }

  NNO Symbol::getBlockNoOfST()
  {
    return m_stBlockNo;
  }

  void Symbol::setBlockNoOfST(NNO n)
  {
    m_stBlockNo = n;
  }

  const std::string Symbol::getMangledName()
  {
    std::ostringstream mangled;
    //if(isSelf()) t41359
    if(isSelf() && (getAutoLocalType() != ALT_AS))
      {
	mangled << m_state.getHiddenArgName(); // ur
      }
    else if(isSuper())
      {
	mangled << m_state.getHiddenArgName(); // also ur
      }
    else
      {
	std::string nstr = m_state.getDataAsStringMangled(getId());
	mangled << getMangledPrefix() << nstr.c_str();
      }
    return mangled.str();
  } //getMangledName

  //atomic parameter type, not model parameter.
  const std::string Symbol::getMangledNameForParameterType()
  {
    assert(!isModelParameter());

    UlamType * sut = m_state.getUlamTypeByIndex(getUlamTypeIdx());
    //another way, like this?
    if(isModelParameter())
      {
	std::ostringstream epmangled;
	epmangled << sut->getImmediateModelParameterStorageTypeAsString();
	assert(sut->getUlamClassType() == UC_NOTACLASS);
	return epmangled.str();
      }

    // to distinguish btn an atomic parameter typedef and quark typedef;
    // use atomic parameter with array of classes
    bool isaclass = ((sut->getUlamTypeEnum() == Class) && sut->isScalar());

    std::ostringstream pmangled;
    pmangled << Symbol::getParameterTypePrefix(isaclass).c_str() << getMangledName();
    return pmangled.str();
  } //getMangledNameForParameterType

  const std::string Symbol::getParameterTypePrefix(bool isaclass)  //static method
  {
    return (isaclass ? "Ut_" : "Up_"); //atomic parameter p
  }

  void Symbol::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
    {
      m_state.abortShouldntGetHere();
    }

  void Symbol::setStructuredComment()
  {
    m_state.abortShouldntGetHere();
  } //setStructuredComment

  bool Symbol::getStructuredComment(Token& scTok)
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
