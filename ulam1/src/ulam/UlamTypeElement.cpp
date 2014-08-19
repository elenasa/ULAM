#include <iostream>
#include <stdio.h>
#include "UlamTypeElement.h"
#include "UlamValue.h"

namespace MFM {

  UlamTypeElement::UlamTypeElement(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


  void UlamTypeElement::newValue(UlamValue & val)
  {
    assert((val.isArraySize() == m_key.m_arraySize) && (m_key.m_arraySize == 0));
    val.m_baseArraySlotIndex = 0;  //tmp !!!  new UlamValueElement ?
  }


  void UlamTypeElement::deleteValue(UlamValue * val)
  {}


   ULAMTYPE UlamTypeElement::getUlamTypeEnum()
   {
     return Element;
   }


  bool UlamTypeElement::cast(UlamValue & val)
  {
    assert(0);
    //std::cerr << "UlamTypeElement (cast) error! " << std::endl;
    return false;
  }


  void UlamTypeElement::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState * state)
  {
    sprintf(valstr,"%s", getUlamTypeName(state).c_str());
  }


  bool UlamTypeElement::isZero(const UlamValue & val)
  {
    return true; 
  }


} //end MFM
