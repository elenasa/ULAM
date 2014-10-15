#include <sstream>
#include <stdio.h>
#include "UlamTypeAtom.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeAtom::UlamTypeAtom(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {
    m_wordLength = calcWordSize(getTotalBitSize());
  }


   ULAMTYPE UlamTypeAtom::getUlamTypeEnum()
   {
     return Atom;
   }


  const std::string UlamTypeAtom::getUlamTypeVDAsStringForC()
  {
    return "VD::ATOM";
  }


  const std::string UlamTypeAtom::getUlamTypeMangledName(CompilerState * state)
  {
    return "T";
  }


  const std::string UlamTypeAtom::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    return "T";
  }


  bool UlamTypeAtom::needsImmediateType()
  {
    return false;
  }


  const char * UlamTypeAtom::getUlamTypeAsSingleLowercaseLetter()
  {
    return "a";  //self ???
  }
 
} //end MFM
