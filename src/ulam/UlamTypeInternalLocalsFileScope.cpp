#include <iostream>
#include <stdio.h>
#include "UlamTypeInternalLocalsFileScope.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInternalLocalsFileScope::UlamTypeInternalLocalsFileScope(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeInternal(key, state) {}


  ULAMTYPE UlamTypeInternalLocalsFileScope::getUlamTypeEnum()
  {
    return LocalsFileScope;
  }

  ULAMCLASSTYPE UlamTypeInternalLocalsFileScope::getUlamClassType()
  {
    return UC_NOTACLASS;
  }

} //end MFM
