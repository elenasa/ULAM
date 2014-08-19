#include <iostream>
#include <stdio.h>
#include "UlamTypeQuark.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeQuark::UlamTypeQuark(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


  void UlamTypeQuark::newValue(UlamValue & val)
  {
    assert((val.isArraySize() == m_key.m_arraySize) && (m_key.m_arraySize == 0));
    val.m_baseArraySlotIndex = 0;  //tmp! newUlamValueQuark?
  }


  void UlamTypeQuark::deleteValue(UlamValue * val)
  { }


   ULAMTYPE UlamTypeQuark::getUlamTypeEnum()
   {
     return Quark;
   }


  bool UlamTypeQuark::cast(UlamValue & val)
  {
    assert(0);
    //std::cerr << "UlamTypeQuark (cast) error! " << std::endl;
    return false;
  }


  void UlamTypeQuark::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState * state)
  {
    sprintf(valstr,"%s", getUlamTypeName(state).c_str());
  }


  bool UlamTypeQuark::isZero(const UlamValue & val)
  {
    return true; 
  }


} //end MFM
