#include <sstream>
#include <string.h>
#include <assert.h>
#include "UlamKeyTypeSignature.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamKeyTypeSignature::UlamKeyTypeSignature(): m_typeNameId(0), m_bits(UNKNOWNSIZE), m_arraySize(UNKNOWNSIZE), m_classInstanceIdx(Nav) {}

  UlamKeyTypeSignature::UlamKeyTypeSignature(u32 nameid, s32 bitsize, s32 arraysize ): m_typeNameId(nameid), m_bits(bitsize), m_arraySize(arraysize), m_classInstanceIdx(Nav) {}

  UlamKeyTypeSignature::UlamKeyTypeSignature(u32 nameid, s32 bitsize, s32 arraysize, UTI classinstanceidx) : m_typeNameId(nameid), m_bits(bitsize), m_arraySize(arraysize), m_classInstanceIdx(classinstanceidx) {}

  UlamKeyTypeSignature::~UlamKeyTypeSignature(){}

  void UlamKeyTypeSignature::append(UTI cuti)
  {
    m_classInstanceIdx = cuti;
  }

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


  UTI UlamKeyTypeSignature::getUlamKeyTypeSignatureClassInstanceIdx()
  {
    return m_classInstanceIdx;
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureNameAndSize(CompilerState * state)
  {
    std::ostringstream key;
    key << getUlamKeyTypeSignatureName(state);
    if(m_bits >= 0)
      key << "(" << m_bits << ")";
    else if(m_bits == UNKNOWNSIZE)
      key << "(" << "UNKNOWN" << ")";
    else if(m_bits == CYCLEFLAG)
      key << "(" << "CYCLE" << ")";
    else
      key << "(" << m_bits << "?)";

    //arraysize
    if(m_arraySize >= 0)
      key << "[" << m_arraySize << "]";
    else if(m_arraySize == UNKNOWNSIZE)
      key << "[" << "UNKNOWN" << "]";
    else if(m_arraySize != NONARRAYSIZE)
      key << "[" << m_arraySize << "?]";

    return key.str();
  } //getUlamKeyTypeSignatureNameAndBitSize


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(CompilerState * state)
  {
    return UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(*this, state);
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(UlamKeyTypeSignature utk, CompilerState* state)
  {
    std::ostringstream key;
    //key << utk.getUlamKeyTypeSignatureName(state) << "." << utk.m_bits << "." << utk.m_arraySize;
    key << utk.getUlamKeyTypeSignatureName(state);
    if(utk.m_bits >= 0)
      key << "(" << utk.m_bits << ")";
    else if(utk.m_bits == UNKNOWNSIZE)
      key << "(" << "UNKNOWN" << ")";
    else if(utk.m_bits == CYCLEFLAG)
      key << "(" << "CYCLE" << ")";
    else
      key << "(" << utk.m_bits << "?)";

    //arraysize
    if(utk.m_arraySize >= 0)
      key << "[" << utk.m_arraySize << "]";
    else if(utk.m_arraySize == UNKNOWNSIZE)
      key << "[" << "UNKNOWN" << "]";
    else if(utk.m_arraySize != NONARRAYSIZE)
      key << "[" << utk.m_arraySize << "?]";
    //key << "[" << "]";

    if(utk.m_classInstanceIdx != Nav)
      key << "<" << utk.m_classInstanceIdx << ">";

    return key.str();
  } //getUlamKeyTypeSignatureAsString


  bool UlamKeyTypeSignature::operator<(const UlamKeyTypeSignature & key2)
  {
    if(m_typeNameId < key2.m_typeNameId) return true;  //?
    if(m_typeNameId > key2.m_typeNameId) return false; //?

    if(m_bits < key2.m_bits) return true;
    if(m_bits > key2.m_bits) return false;
    if(m_arraySize < key2.m_arraySize) return true;
    if(m_arraySize > key2.m_arraySize) return false;

    if(m_classInstanceIdx < key2.m_classInstanceIdx) return true;
    if(m_classInstanceIdx > key2.m_classInstanceIdx) return false;
    return false;
  }


  bool UlamKeyTypeSignature::operator==(const UlamKeyTypeSignature & key2)
  {
    return ((m_typeNameId == key2.m_typeNameId) && (m_bits == key2.m_bits) && (m_arraySize == key2.m_arraySize) && m_classInstanceIdx == key2.m_classInstanceIdx);
  }

} //end MFM
