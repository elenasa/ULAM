#include <iostream>
#include <stdio.h>
#include "UlamTypeInternalHolder.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInternalHolder::UlamTypeInternalHolder(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeInternal(key, state) {}


  ULAMTYPE UlamTypeInternalHolder::getUlamTypeEnum()
  {
    return Holder;
  }

  ULAMCLASSTYPE UlamTypeInternalHolder::getUlamClassType()
  {
    return UC_NOTACLASS;
  }

  bool UlamTypeInternalHolder::isHolder()
  {
    return true;
  }

  bool UlamTypeInternalHolder::isMinMaxAllowed()
  {
    return isScalar(); //minof/maxof allowed in ulam
  }

} //end MFM
