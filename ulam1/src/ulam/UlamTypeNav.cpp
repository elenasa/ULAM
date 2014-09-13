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


  void UlamTypeNav::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    sprintf(valstr,"%s", getUlamTypeName(&state).c_str());
  }

} //end MFM
