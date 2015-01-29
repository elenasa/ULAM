#include <sstream>
#include <string.h>
#include "CompilerState.h"
#include "Symbol.h"
#include "Util.h"

namespace MFM {

  Symbol::Symbol(u32 id, UTI utype, CompilerState & state) : m_state(state), m_id(id), m_utypeIdx(utype), m_dataMember(false), m_elementParameter(false), m_autoLocal(false), m_isSelf(false) {}
  Symbol::Symbol(const Symbol & sref) : m_state(sref.m_state), m_id(sref.m_id), m_utypeIdx(sref.m_utypeIdx), m_dataMember(sref.m_dataMember), m_elementParameter(sref.m_elementParameter), m_autoLocal(sref.m_autoLocal), m_isSelf(sref.m_isSelf){}
  Symbol::~Symbol(){}

  u32 Symbol::getId()
  {
    return m_id;
  }


  UTI Symbol::getUlamTypeIdx()
  {
    return m_utypeIdx;
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
    assert(!m_elementParameter);  //maybe do it some other way XXX

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
    bool isaclass = (( classtype == UC_QUARK || classtype == UC_ELEMENT || classtype == UC_INCOMPLETE) && sut->isScalar());

    std::ostringstream pmangled;
    pmangled << Symbol::getParameterTypePrefix(isaclass).c_str() << getMangledName();
    return pmangled.str();
  } //getMangledNameForParameterType


  const std::string Symbol::getParameterTypePrefix(bool isaclass)  //static method
  {
    return (isaclass ? "Ut_" : "Up_");
    //    return "Up_";
  }


  void Symbol::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
    {
      assert(0);
    }

} //end MFM
