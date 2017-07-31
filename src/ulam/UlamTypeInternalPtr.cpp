#include <iostream>
#include <stdio.h>
#include "UlamTypeInternalPtr.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInternalPtr::UlamTypeInternalPtr(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeInternal(key, state)
  {}


   ULAMTYPE UlamTypeInternalPtr::getUlamTypeEnum()
   {
     return Ptr;
   }

  PACKFIT UlamTypeInternalPtr::getPackable()
  {
    return UNPACKED;
  }

} //end MFM
