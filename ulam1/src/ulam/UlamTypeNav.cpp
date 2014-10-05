#include <iostream>
#include <stdio.h>
#include "UlamTypeNav.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeNav::UlamTypeNav(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypeNav::getUlamTypeEnum()
   {
     return Nav;
   }


  bool UlamTypeNav::needsImmediateType()
  {
    return false;
  }

} //end MFM
