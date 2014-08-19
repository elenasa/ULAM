#include <sstream>
#include <string.h>
#include <assert.h>
#include "UlamKeyTypeSignature.h"
#include "util.h"

namespace MFM {

  UlamKeyTypeSignature::UlamKeyTypeSignature(){}


  UlamKeyTypeSignature::UlamKeyTypeSignature(const char * name, u32 bitsize, u32 arraysize) : m_bits(bitsize), m_arraySize(arraysize) 
  {
    strncpy(m_typeName, name, MAX_TYPENAME_LEN);
    m_typeName[MAX_TYPENAME_LEN] = '\0';
  }


  UlamKeyTypeSignature::~UlamKeyTypeSignature(){}


  char * UlamKeyTypeSignature::getUlamKeyTypeSignatureName()
  {
    return m_typeName;
  }


  u32 UlamKeyTypeSignature::getUlamKeyTypeSignatureBitSize()
  {
    return m_bits;
  }


  u32 UlamKeyTypeSignature::getUlamKeyTypeSignatureArraySize()
  {
    return m_arraySize;
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString()
  {
    std::ostringstream key;
    key << m_typeName << "." << m_bits << "." << m_arraySize << '\0';
    return key.str();
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(UlamKeyTypeSignature utk)
  {
    std::ostringstream key;
    key << utk.m_typeName << "." << utk.m_bits << "." << utk.m_arraySize;
    return key.str();
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureMangledName()
  {
    //Ut_18232Int  == Int[8]
    std::ostringstream mangled;
    
    mangled << "Ut_" << countDigits(m_arraySize) << m_arraySize << countDigits(m_bits) << m_bits << countDigits(strlen(m_typeName)) << strlen(m_typeName) << m_typeName;
    return mangled.str();
  }


  bool UlamKeyTypeSignature::operator<(const UlamKeyTypeSignature & key2)
  {    
    if(strcmp(m_typeName, key2.m_typeName) < 0) return true;
    if(strcmp(m_typeName, key2.m_typeName) > 0) return false;  
    if(m_bits < key2.m_bits) return true;
    if(m_bits > key2.m_bits) return false;
    if(m_arraySize < key2.m_arraySize) return true;
    if(m_arraySize > key2.m_arraySize) return false;
    return false; 
  }


  bool UlamKeyTypeSignature::operator==(const UlamKeyTypeSignature & key2)
  {    
    return ((strcmp(m_typeName, key2.m_typeName)== 0) && (m_bits == key2.m_bits) && (m_arraySize == key2.m_arraySize));
  }

} //end MFM
