#include <iostream>
#include <stdio.h>
#include "UlamTypeInternal.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInternal::UlamTypeInternal(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {}

  ULAMCLASSTYPE UlamTypeInternal::getUlamClassType()
  {
    assert(0);
    return UC_ERROR; //for compiler only
  }

  bool UlamTypeInternal::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypeInternal::getLocalStorageTypeAsString()
  {
    assert(0);
    return "internalerror";
  }

  const std::string UlamTypeInternal::castMethodForCodeGen(UTI nodetype)
  {
    assert(0);
    return "internalerror";
  }

  bool UlamTypeInternal::isComplete()
  {
    return false;
  }

} //end MFM
