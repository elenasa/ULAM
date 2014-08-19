#include <sstream>
#include <string.h>
#include <assert.h>
#include "UlamKeyTypeSignature.h"
#include "util.h"
#include "CompilerState.h"

namespace MFM {

  UlamKeyTypeSignature::UlamKeyTypeSignature(){}

  UlamKeyTypeSignature::UlamKeyTypeSignature(u32 nameid, u32 bitsize, u32 arraysize ): m_typeNameId(nameid), m_bits(bitsize), m_arraySize(arraysize) {}

  UlamKeyTypeSignature::~UlamKeyTypeSignature(){}


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureName(CompilerState * state)
  {
    return state->m_pool.getDataAsString(m_typeNameId);
    //return m_typeName;
  }


  u32 UlamKeyTypeSignature::getUlamKeyTypeSignatureBitSize()
  {
    return m_bits;
  }


  u32 UlamKeyTypeSignature::getUlamKeyTypeSignatureArraySize()
  {
    return m_arraySize;
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(CompilerState * state)
  {
    std::ostringstream key;
    key << getUlamKeyTypeSignatureName(state) << "." << m_bits << "." << m_arraySize << '\0';
    return key.str();
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureAsString(UlamKeyTypeSignature utk, CompilerState* state)
  {
    std::ostringstream key;
    key << utk.getUlamKeyTypeSignatureName(state) << "." << utk.m_bits << "." << utk.m_arraySize;
    return key.str();
  }


  const std::string UlamKeyTypeSignature::getUlamKeyTypeSignatureMangledName(CompilerState * state)
  {
    //Ut_18232Int  == Int[8]
    std::ostringstream mangled;
    std::string nstr = state->m_pool.getDataAsString(m_typeNameId);
    u32 nstrlen = nstr.length();
    mangled << "Ut_" << countDigits(m_arraySize) << m_arraySize << countDigits(m_bits) << m_bits << countDigits(nstrlen) << nstrlen << nstr.c_str();
    return mangled.str();
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
