#include <iostream>
#include <stdio.h>
#include "UlamTypeAtom.h"
#include "UlamValue.h"

namespace MFM {

  UlamTypeAtom::UlamTypeAtom(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


  void UlamTypeAtom::newValue(UlamValue & val)
  {
    assert((val.isArraySize() == m_key.m_arraySize) && (m_key.m_arraySize == 0));
    val.m_baseArraySlotIndex = 0;  //tmp type!!! new UlamValueAtom?  what to do???
  }


  void UlamTypeAtom::deleteValue(UlamValue * val)
  {  }


   ULAMTYPE UlamTypeAtom::getUlamTypeEnum()
   {
     return Atom;
   }


  bool UlamTypeAtom::cast(UlamValue & val)
  {
    assert(0);
    //std::cerr << "UlamTypeAtom (cast) error! " << std::endl;
    return false;
  }


  void UlamTypeAtom::getUlamValueAsString(const UlamValue& val, char * valstr, CompilerState& state)
  {
    sprintf(valstr,"%s", getUlamTypeName().c_str());
  }


  bool UlamTypeAtom::isZero(const UlamValue & val)
  {
    return true; 
  }


} //end MFM
