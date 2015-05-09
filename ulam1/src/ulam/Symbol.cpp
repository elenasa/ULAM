#include <sstream>
#include <string.h>
#include "CompilerState.h"
#include "Symbol.h"
#include "Util.h"

namespace MFM {

  Symbol::Symbol(Token id, UTI utype, CompilerState & state) : m_state(state), m_idtok(id), m_uti(utype), m_dataMember(false), m_elementParameter(false), m_autoLocal(false), m_isSelf(false), m_stBlockNo(state.getCurrentBlockNo()) {}

  Symbol::Symbol(const Symbol & sref) : m_state(sref.m_state), m_idtok(sref.m_idtok), m_uti(m_state.mapIncompleteUTIForCurrentClassInstance(sref.m_uti)), m_dataMember(sref.m_dataMember), m_elementParameter(sref.m_elementParameter), m_autoLocal(sref.m_autoLocal), m_isSelf(sref.m_isSelf), m_stBlockNo(sref.m_stBlockNo) {}

  Symbol::Symbol(const Symbol& sref, bool keepType) : m_state(sref.m_state), m_idtok(sref.m_idtok), m_uti(sref.m_uti), m_dataMember(sref.m_dataMember), m_elementParameter(sref.m_elementParameter), m_autoLocal(sref.m_autoLocal), m_isSelf(sref.m_isSelf), m_stBlockNo(sref.m_stBlockNo) {}

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
    m_uti = newuti;
  }

  UTI Symbol::getUlamTypeIdx()
  {
    return m_uti;
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

  void Symbol::setDataMember()
  {
    m_dataMember = true;
    if(m_state.m_parsingElementParameterVariable)
      m_elementParameter = true;
  }

  bool Symbol::isDataMember()
  {
    return m_dataMember;
  }

  void Symbol::setElementParameter()
  {
      m_elementParameter = true;
  }

  bool Symbol::isElementParameter()
  {
    return m_elementParameter;
  }

  void Symbol::setAutoLocal()
  {
      m_autoLocal = true;
  }

  bool Symbol::isAutoLocal()
  {
    return m_autoLocal;
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

  //atomic parameter type, not element parameter.
  const std::string Symbol::getMangledNameForParameterType()
  {
    assert(!m_elementParameter); //maybe do it some other way XXX

    UlamType * sut = m_state.getUlamTypeByIndex(getUlamTypeIdx());
    ULAMCLASSTYPE classtype = sut->getUlamClass();

    //another way, like this?
    if(m_elementParameter)
      {
	std::ostringstream epmangled;
	epmangled << sut->getImmediateStorageTypeAsString();
	if(classtype == UC_QUARK)
	  epmangled << "::Us";
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
    return (isaclass ? "Ut_" : "Up_");
  }

  void Symbol::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
    {
      assert(0);
    }

} //end MFM
