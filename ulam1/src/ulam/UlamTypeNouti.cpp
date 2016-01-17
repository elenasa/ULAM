#include <iostream>
#include <stdio.h>
#include "UlamTypeNouti.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeNouti::UlamTypeNouti(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {}

  ULAMTYPE UlamTypeNouti::getUlamTypeEnum()
  {
    return Nouti;
  }

  ULAMCLASSTYPE UlamTypeNouti::getUlamClass()
  {
    assert(0);
    return UC_ERROR; //for compiler only
  }

  const std::string UlamTypeNouti::getUlamTypeAsStringForC(bool useref)
  {
    assert(0);
    return "nouti";
  }

  bool UlamTypeNouti::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypeNouti::getLocalStorageTypeAsString()
  {
    assert(0);
    return "nouti";
  }

  const std::string UlamTypeNouti::castMethodForCodeGen(UTI nodetype)
  {
    assert(0);
    return "nouti";
  }

  bool UlamTypeNouti::isComplete()
  {
    return false;
  }

  void UlamTypeNouti::genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp)
  {
    assert(0); //only primitive types
  }

} //end MFM
