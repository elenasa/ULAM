#include <iostream>
#include <stdio.h>
#include "UlamTypeInternalNav.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInternalNav::UlamTypeInternalNav(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeInternal(key, state)
  {}

  ULAMTYPE UlamTypeInternalNav::getUlamTypeEnum()
  {
    return Nav;
  }

} //end MFM
