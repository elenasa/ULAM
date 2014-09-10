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


  bool UlamTypePtr::cast(UlamValue & val)
  {
    assert(0);
    //std::cerr << "UlamTypePtr (cast) error! " << std::endl;
    return false;
  }


  void UlamTypePtr::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    sprintf(valstr,"%s", getUlamTypeName(&state).c_str());
  }

} //end MFM
