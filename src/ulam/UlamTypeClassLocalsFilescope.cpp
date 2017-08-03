#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClassLocalsFilescope.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClassLocalsFilescope::UlamTypeClassLocalsFilescope(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeClass(key, state) { }


  ULAMCLASSTYPE UlamTypeClassLocalsFilescope::getUlamClassType()
  {
    return UC_LOCALSFILESCOPE;
  }

  const char * UlamTypeClassLocalsFilescope::getUlamTypeAsSingleLowercaseLetter()
  {
    return "l";
  }

  const std::string UlamTypeClassLocalsFilescope::getUlamTypeUPrefix()
  {
    return "Ul_";
  }

} //end MFM
