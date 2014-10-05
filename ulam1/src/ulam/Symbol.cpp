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
    std::ostringstream pmangled;
    pmangled << "Up_" << getMangledName();
    return pmangled.str();
  }

  void Symbol::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
    {
      assert(0);
    }

} //end MFM
