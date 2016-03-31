#include <iostream>
#include <stdio.h>
#include "UlamTypeNav.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeNav::UlamTypeNav(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {}

  ULAMTYPE UlamTypeNav::getUlamTypeEnum()
  {
    return Nav;
  }

  ULAMCLASSTYPE UlamTypeNav::getUlamClassType()
  {
    assert(0);
    return UC_ERROR; //for compiler only
  }

  bool UlamTypeNav::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypeNav::getLocalStorageTypeAsString()
  {
    assert(0);
    return "nav";
  }

  const std::string UlamTypeNav::castMethodForCodeGen(UTI nodetype)
  {
    assert(0);
    return "nav";
  }

  bool UlamTypeNav::isComplete()
  {
    return false;
  }

} //end MFM
