#include <sstream>
#include <string.h>
#include <assert.h>
#include "UlamKeyTypeSignature.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamKeyTypeSignature::UlamKeyTypeSignature(): m_typeNameId(0), m_bits(UNKNOWNSIZE), m_arraySize(UNKNOWNSIZE), m_classInstanceIdx(Nouti), m_referenceType(ALT_NOT) {}

  UlamKeyTypeSignature::UlamKeyTypeSignature(u32 nameid, s32 bitsize, s32 arraysize, ALT reftype): m_typeNameId(nameid), m_bits(bitsize), m_arraySize(arraysize), m_classInstanceIdx(Nouti), m_referenceType(reftype) {}

  UlamKeyTypeSignature::UlamKeyTypeSignature(u32 nameid, s32 bitsize, s32 arraysize, UTI classinstanceidx, ALT reftype) : m_typeNameId(nameid), m_bits(bitsize), m_arraySize(arraysize), m_classInstanceIdx(classinstanceidx), m_referenceType(reftype) {}

  UlamKeyTypeSignature::~UlamKeyTypeSignature(){}

  void UlamKeyTypeSignature::append(UTI cuti)
  {
    m_classInstanceIdx = cuti;
  }

  const std::string & UlamKeyTypeSignature::getUlamKeyTypeSignatureName(CompilerState * state)
  {
    assert(state);
    return state->m_pool.getDataAsString(m_typeNameId);
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

  ALT UlamKeyTypeSignature::getUlamKeyTypeSignatureReferenceType()
  {
    return m_referenceType;
  }

  const std::string & UlamKeyTypeSignature::getUlamKeyTypeSignatureNameAndBitSize(CompilerState * state)
  {
    std::ostringstream key;
    key << getUlamKeyTypeSignatureName(state);
    if(m_bits > 0)
      key << "(" << m_bits << ")";
    else if(m_bits == UNKNOWNSIZE)
      key << "(" << "UNKNOWN" << ")";
    else if(m_bits == CYCLEFLAG)
      key << "(" << "CYCLE" << ")";
    //else
    //  key << "(" << m_bits << "?)";
    //return key.str();
    u32 keyid = state->m_pool.getIndexForDataString(key.str());
    return state->m_pool.getDataAsString(keyid);
  } //getUlamKeyTypeSignatureNameAndBitSize

  const std::string & UlamKeyTypeSignature::getUlamKeyTypeSignatureNameAndSize(CompilerState * state)
  {
    std::ostringstream key;

    if(m_referenceType == ALT_CONSTREF)
      key << "constant ";

    key << getUlamKeyTypeSignatureNameAndBitSize(state);
    //arraysize
    if(m_arraySize >= 0)
      key << "[" << m_arraySize << "]";
    else if(m_arraySize == UNKNOWNSIZE)
      key << "[" << "UNKNOWN" << "]";
    else if(m_arraySize != NONARRAYSIZE)
      key << "[" << m_arraySize << "?]";

    if((m_referenceType == ALT_REF) || (m_referenceType == ALT_CONSTREF))
      key << "&"; //only when ulam programmer put in the &
    //return key.str();
    u32 keyid = state->m_pool.getIndexForDataString(key.str());
    return state->m_pool.getDataAsString(keyid);
  } //getUlamKeyTypeSignatureNameAndSize

  const std::string & UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(CompilerState * state)
  {
    return UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(*this, state);
  }

  const std::string & UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(UlamKeyTypeSignature utk, CompilerState* state)
  {
    std::ostringstream key;
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

    if(utk.m_classInstanceIdx != Nouti)
      key << "<" << utk.m_classInstanceIdx << ">";

    if(utk.m_referenceType != ALT_NOT)
      key << "<" << utk.m_referenceType << ">";

    //return key.str();
    u32 keyid = state->m_pool.getIndexForDataString(key.str());
    return state->m_pool.getDataAsString(keyid);
  } //getUlamKeyTypeSignatureAsString

  bool UlamKeyTypeSignature::operator<(const UlamKeyTypeSignature & key2) const
  {
    if(m_typeNameId < key2.m_typeNameId) return true;  //?
    if(m_typeNameId > key2.m_typeNameId) return false; //?

    if(m_bits < key2.m_bits) return true;
    if(m_bits > key2.m_bits) return false;

    if(m_arraySize < key2.m_arraySize) return true;
    if(m_arraySize > key2.m_arraySize) return false;

    if(m_classInstanceIdx < key2.m_classInstanceIdx) return true;
    if(m_classInstanceIdx > key2.m_classInstanceIdx) return false;

    if(m_referenceType < key2.m_referenceType) return true;
    if(m_referenceType > key2.m_referenceType) return false;

    return false;
  }

  bool UlamKeyTypeSignature::operator==(const UlamKeyTypeSignature & key2) const
  {
    return ((m_typeNameId == key2.m_typeNameId) && (m_bits == key2.m_bits) && (m_arraySize == key2.m_arraySize) && (m_classInstanceIdx == key2.m_classInstanceIdx) && (m_referenceType == key2.m_referenceType));
  }

} //end MFM
