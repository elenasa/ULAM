#include <iostream>
#include <stdio.h>
#include "UlamTypeNav.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeNav::UlamTypeNav(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypeNav::getUlamTypeEnum()
   {
     return Nav;
   }


  const std::string UlamTypeNav::getUlamTypeAsStringForC()
  {
    assert(0);
    return "nav";
  }


  bool UlamTypeNav::needsImmediateType()
  {
    return false;
  }


  const std::string UlamTypeNav::getImmediateStorageTypeAsString(CompilerState * state)
  {
    assert(0);
    return "nav";
  }


  const std::string UlamTypeNav::castMethodForCodeGen(UTI nodetype, CompilerState& state)
  {
    assert(0);
    return "nav";
  }

} //end MFM
