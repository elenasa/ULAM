#include <iostream>
#include <stdio.h>
#include "UlamTypeInternalNouti.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInternalNouti::UlamTypeInternalNouti(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeInternal(key, state)
  {}

  ULAMTYPE UlamTypeInternalNouti::getUlamTypeEnum()
  {
    return Nouti;
  }

} //end MFM
