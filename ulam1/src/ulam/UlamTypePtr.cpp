#include <iostream>
#include <stdio.h>
#include "UlamTypePtr.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypePtr::UlamTypePtr(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypePtr::getUlamTypeEnum()
   {
     return Ptr;
   }

} //end MFM
