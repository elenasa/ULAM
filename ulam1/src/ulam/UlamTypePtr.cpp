#include <iostream>
#include <stdio.h>
#include "UlamTypePtr.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypePtr::UlamTypePtr(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {}


   ULAMTYPE UlamTypePtr::getUlamTypeEnum()
   {
     return Ptr;
   }

  ULAMCLASSTYPE UlamTypePtr::getUlamClassType()
  {
    assert(0);
    return UC_ERROR;
  }

  bool UlamTypePtr::needsImmediateType()
  {
    return false;
  }


  bool UlamTypePtr::isMinMaxAllowed()
  {
    return false;
  }


  PACKFIT UlamTypePtr::getPackable()
  {
    return UNPACKED;
  }

} //end MFM
