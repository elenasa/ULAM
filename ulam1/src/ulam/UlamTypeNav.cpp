#include <iostream>
#include <stdio.h>
#include "UlamTypeNav.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeNav::UlamTypeNav(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


  void UlamTypeNav::newValue(UlamValue & val){}


  void UlamTypeNav::deleteValue(UlamValue * val){}


   ULAMTYPE UlamTypeNav::getUlamTypeEnum()
   {
     return Nav;
   }


  bool UlamTypeNav::cast(UlamValue & val)
  {
    assert(0);
    //std::cerr << "UlamTypeNav (cast) error! " << std::endl;
    return false;
  }


  void UlamTypeNav::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    sprintf(valstr,"%s", getUlamTypeName().c_str());
  }


  bool UlamTypeNav::isZero(const UlamValue & val)
  {
    return true; 
  }


} //end MFM
