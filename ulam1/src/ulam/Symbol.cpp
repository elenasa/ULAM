#include <sstream>
#include <string.h>
#include "CompilerState.h"
#include "Symbol.h"
#include "Util.h"

namespace MFM {

  Symbol::Symbol(u32 id, UTI utype, CompilerState & state) : m_state(state), m_id(id), m_utypeIdx(utype), m_dataMember(false) {}
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


  bool Symbol::isClass()
  {
    return false;
  }


  void Symbol::setDataMember()
  {
    m_dataMember = true;
  }


  bool Symbol::isDataMember()
  {
    return m_dataMember;
  }


  const std::string Symbol::getMangledName()
  {
       std::ostringstream mangled;
       std::string nstr = m_state.getDataAsStringMangled(getId());

       mangled << getMangledPrefix() << nstr.c_str();
       return mangled.str();
  }


  const std::string Symbol::getMangledNameForParameterType()
  {
    // to distinguish btn an atomic parameter typedef and quark typedef
    ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(getUlamTypeIdx())->getUlamClass();
    bool isaclass = (classtype == UC_QUARK || classtype == UC_ELEMENT);

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
