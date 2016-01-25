#include <sstream>
#include <string.h>
#include "CompilerState.h"
#include "Symbol.h"
#include "Util.h"

namespace MFM {

  Symbol::Symbol(Token id, UTI utype, CompilerState & state) : m_state(state), m_gotStructuredCommentToken(false), m_idtok(id), m_uti(utype), m_dataMemberClass(Nouti), m_autoLocalType(ALT_NOT), m_isSelf(false), m_stBlockNo(state.getCurrentBlockNo()){}

  Symbol::Symbol(const Symbol & sref) : m_state(sref.m_state), m_structuredCommentToken(sref.m_structuredCommentToken), m_gotStructuredCommentToken(sref.m_gotStructuredCommentToken), m_idtok(sref.m_idtok), m_uti(m_state.mapIncompleteUTIForCurrentClassInstance(sref.m_uti)), m_dataMemberClass(m_state.mapIncompleteUTIForCurrentClassInstance(sref.m_dataMemberClass)), m_autoLocalType(sref.m_autoLocalType), m_isSelf(sref.m_isSelf), m_stBlockNo(sref.m_stBlockNo) {}

  Symbol::Symbol(const Symbol& sref, bool keepType) : m_state(sref.m_state), m_structuredCommentToken(sref.m_structuredCommentToken), m_gotStructuredCommentToken(sref.m_gotStructuredCommentToken), m_idtok(sref.m_idtok), m_uti(sref.m_uti), m_dataMemberClass(sref.m_dataMemberClass), m_autoLocalType(sref.m_autoLocalType), m_isSelf(sref.m_isSelf), m_stBlockNo(sref.m_stBlockNo) {}

  Symbol::~Symbol(){}

  Symbol * Symbol::cloneKeepsType()
  {
    assert(0);
    return NULL;
  }

  void Symbol::setId(u32 newid)
  {
    m_idtok.m_dataindex = newid; //protected
  }

  void Symbol::resetIdToken(Token newtok)
  {
    m_idtok = newtok; //id & loc
  }

  u32 Symbol::getId()
  {
    return m_idtok.m_dataindex;
  }

  Locator Symbol::getLoc()
  {
    return m_idtok.m_locator;
  }

  Token* Symbol::getTokPtr()
  {
    return &m_idtok;
  }

  void Symbol::resetUlamType(UTI newuti) //e.g. mappedUTI, fix _N class args
  {
    assert(m_state.getReferenceType(m_uti) == m_state.getReferenceType(newuti));
    m_uti = newuti;
  }

  UTI Symbol::getUlamTypeIdx()
  {
    return m_uti;
  }

  u32 Symbol::getPosOffset()
  {
    assert(0);
    return 0; //data members only, incl. symbolparametervalue
  }

  bool Symbol::isFunction()
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

  bool Symbol::isModelParameter()
  {
    return false;
  }

  void Symbol::setAutoLocalType(Token cTok)
  {
    if(cTok.m_type == TOK_KW_AS)
      setAutoLocalType(ALT_AS);
    else if(cTok.m_type == TOK_KW_HAS)
      //  setAutoLocalType(ALT_HAS);
      assert(0); //deprecated
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
       std::string nstr = m_state.getDataAsStringMangled(getId());

       mangled << getMangledPrefix() << nstr.c_str();
       return mangled.str();
  }

  //atomic parameter type, not model parameter.
  const std::string Symbol::getMangledNameForParameterType()
  {
    assert(!isModelParameter());

    UlamType * sut = m_state.getUlamTypeByIndex(getUlamTypeIdx());
    ULAMCLASSTYPE classtype = sut->getUlamClass();

    //another way, like this?
    if(isModelParameter())
      {
	std::ostringstream epmangled;
	epmangled << sut->getImmediateModelParameterStorageTypeAsString();
	//if(classtype == UC_QUARK)
	//  epmangled << "::Us";
	assert(classtype == UC_NOTACLASS);
	return epmangled.str();
      }

    // to distinguish btn an atomic parameter typedef and quark typedef;
    // use atomic parameter with array of classes
    bool isaclass = (( classtype == UC_QUARK || classtype == UC_ELEMENT || classtype == UC_UNSEEN) && sut->isScalar());

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
      assert(0);
    }

  void Symbol::setStructuredComment()
  {
    assert(0);
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
