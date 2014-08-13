/* -*- c++ -*- */

#ifndef ULAMKEYTYPESIGNATURE_H
#define ULAMKEYTYPESIGNATURE_H

#include <string>
#include "itype.h"


namespace MFM{
  
#define MAX_TYPENAME_LEN 15
  
  struct UlamKeyTypeSignature
  {
    char m_typeName[MAX_TYPENAME_LEN + 1];
    u32  m_bits;
    u32 m_arraySize;

    UlamKeyTypeSignature();
    UlamKeyTypeSignature(const char * name, u32 bitsize, u32 arraysize=0);
    ~UlamKeyTypeSignature();
    
    char * getUlamKeyTypeSignatureName();
    u32 getUlamKeyTypeSignatureBitSize();
    u32 getUlamKeyTypeSignatureArraySize();
    const std::string getUlamKeyTypeSignatureAsString();
    static const std::string getUlamKeyTypeSignatureAsString(UlamKeyTypeSignature utk);
    bool operator<(const UlamKeyTypeSignature & key2);
    bool operator==(const UlamKeyTypeSignature & key2);

  };
  
  
}

#endif //end ULAMKEYTYPESIGNATURE_H
