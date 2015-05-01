#include <iostream>
#include <stdio.h>
#include "UlamTypeHolder.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeHolder::UlamTypeHolder(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state) {}

  ULAMTYPE UlamTypeHolder::getUlamTypeEnum()
  {
    return Holder;
  }

  const std::string UlamTypeHolder::getUlamTypeAsStringForC()
  {
    assert(0);
    return "holder";
  }

  bool UlamTypeHolder::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypeHolder::getImmediateStorageTypeAsString()
  {
    assert(0);
    return "holder";
  }

  const std::string UlamTypeHolder::castMethodForCodeGen(UTI nodetype)
  {
    assert(0);
    return "holder";
  }

  bool UlamTypeHolder::isHolder()
  {
    return true;
  }

  bool UlamTypeHolder::isComplete()
  {
    return false;
  }

} //end MFM
