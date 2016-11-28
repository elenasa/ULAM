#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClassLocalFilescopes.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClassLocalFilescopes::UlamTypeClassLocalFilescopes(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeClass(key, state) { }


  ULAMCLASSTYPE UlamTypeClassLocalFilescopes::getUlamClassType()
  {
    return UC_LOCALFILESCOPES;
  }

  const char * UlamTypeClassLocalFilescopes::getUlamTypeAsSingleLowercaseLetter()
  {
    return "l";
  }

  const std::string UlamTypeClassLocalFilescopes::getUlamTypeUPrefix()
  {
    return "Ul_";
  }

} //end MFM
