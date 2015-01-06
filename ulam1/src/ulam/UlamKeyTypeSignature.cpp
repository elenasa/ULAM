#include <sstream>
#include <string.h>
#include <assert.h>
#include "UlamKeyTypeSignature.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamKeyTypeSignature::UlamKeyTypeSignature(){}

  UlamKeyTypeSignature::UlamKeyTypeSignature(u32 nameid, s32 bitsize, s32 arraysize ): m_typeNameId(nameid), m_bits(bitsize), m_arraySize(arraysize) {}

  UlamKeyTypeSignature::~UlamKeyTypeSignature(){}


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureName(CompilerState * state)
  {
    return state->m_pool.getDataAsString(m_typeNameId);
    //return m_typeName;
  }


  u32 UlamKeyTypeSignature::getUlamKeyTypeSignatureNameId()
  {
    return m_typeNameId;
  }


  s32 UlamKeyTypeSignature::getUlamKeyTypeSignatureBitSize()
  {
    return m_bits;
  }


  s32 UlamKeyTypeSignature::getUlamKeyTypeSignatureArraySize()
  {
    return m_arraySize;
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureNameAndBitSize(CompilerState * state)
  {
    std::ostringstream key;
    key << getUlamKeyTypeSignatureName(state) << "(" << m_bits << ")";
    return key.str();
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(CompilerState * state)
  {
    return UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(*this, state);
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(UlamKeyTypeSignature utk, CompilerState* state)
  {
    std::ostringstream key;
    //key << utk.getUlamKeyTypeSignatureName(state) << "." << utk.m_bits << "." << utk.m_arraySize;
    key << utk.getUlamKeyTypeSignatureName(state);
    if(utk.m_bits > 0)
      key << "(" << utk.m_bits << ")";
    if(utk.m_arraySize >= 0)
      key << "[" << utk.m_arraySize << "]";
    return key.str();
  }


  bool UlamKeyTypeSignature::operator<(const UlamKeyTypeSignature & key2)
  {
    //if(strcmp(m_typeName, key2.m_typeName) < 0) return true;
    //if(strcmp(m_typeName, key2.m_typeName) > 0) return false;
    if(m_typeNameId < key2.m_typeNameId) return true;  //?
    if(m_typeNameId > key2.m_typeNameId) return false; //?

    if(m_bits < key2.m_bits) return true;
    if(m_bits > key2.m_bits) return false;
    if(m_arraySize < key2.m_arraySize) return true;
    if(m_arraySize > key2.m_arraySize) return false;
    return false;
  }


  bool UlamKeyTypeSignature::operator==(const UlamKeyTypeSignature & key2)
  {
    //return ((strcmp(m_typeName, key2.m_typeName)== 0) && (m_bits == key2.m_bits) && (m_arraySize == key2.m_arraySize));
    return ((m_typeNameId == key2.m_typeNameId) && (m_bits == key2.m_bits) && (m_arraySize == key2.m_arraySize));
  }

} //end MFM
