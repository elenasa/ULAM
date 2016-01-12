#include <iostream>
#include <stdio.h>
#include "UlamTypeHzy.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeHzy::UlamTypeHzy(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {}

  ULAMTYPE UlamTypeHzy::getUlamTypeEnum()
  {
    return Hzy;
  }

  ULAMCLASSTYPE UlamTypeHzy::getUlamClass()
  {
    assert(0);
    return UC_ERROR; //for compiler only
  }

  const std::string UlamTypeHzy::getUlamTypeAsStringForC(bool useref)
  {
    assert(0);
    return "hzy";
  }

  bool UlamTypeHzy::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypeHzy::getLocalStorageTypeAsString()
  {
    assert(0);
    return "hzy";
  }

  const std::string UlamTypeHzy::castMethodForCodeGen(UTI nodetype)
  {
    assert(0);
    return "hzy";
  }

  bool UlamTypeHzy::isComplete()
  {
    return false;
  }

  void UlamTypeHzy::genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp)
  {
    assert(0); //only primitive types
  }

} //end MFM
