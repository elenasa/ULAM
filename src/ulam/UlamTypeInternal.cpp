#include <iostream>
#include <stdio.h>
#include "UlamTypeInternal.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInternal::UlamTypeInternal(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {}

  ULAMCLASSTYPE UlamTypeInternal::getUlamClassType()
  {
    m_state.abortShouldntGetHere();
    return UC_ERROR; //for compiler only
  }

  u32 UlamTypeInternal::getSizeofUlamType()
  {
    m_state.abortShouldntGetHere();
    return 0;
  }

  bool UlamTypeInternal::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypeInternal::getLocalStorageTypeAsString()
  {
    m_state.abortShouldntGetHere();
    return "internalerror";
  }

  const std::string UlamTypeInternal::castMethodForCodeGen(UTI nodetype)
  {
    m_state.abortShouldntGetHere();
    return "internalerror";
  }

  bool UlamTypeInternal::isComplete()
  {
    return false;
  }

  const std::string UlamTypeInternal::readMethodForCodeGen()
  {
    m_state.abortShouldntGetHere();
    return "internalreadmethoderror";
  }

  const std::string UlamTypeInternal::writeMethodForCodeGen()
  {
    m_state.abortShouldntGetHere();
    return "internalwritemethoderror";
  }

  const std::string UlamTypeInternal::readArrayItemMethodForCodeGen()
  {
    m_state.abortShouldntGetHere();
    return "internalreadarrayitemmethoderror";
  }

  const std::string UlamTypeInternal::writeArrayItemMethodForCodeGen()
  {
    m_state.abortShouldntGetHere();
    return "internalwritearrayitemmethoderror";
  }

} //end MFM
