#include <iostream>
#include <stdio.h>
#include "UlamTypeInternalHzy.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInternalHzy::UlamTypeInternalHzy(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeInternal(key, state)
  {}

  ULAMTYPE UlamTypeInternalHzy::getUlamTypeEnum()
  {
    return Hzy;
  }

} //end MFM
