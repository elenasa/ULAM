#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "UlamType.h"


namespace MFM {

#define XX(a,b) b,

  static const char * utype_string[] = {
#include "UlamType.inc"
  };
  
#undef XX 

  UlamType::UlamType(const UlamKeyTypeSignature key, const UTI uti) : m_key(key), m_index(uti)
  {}
  

  UlamType * UlamType::getUlamType()
  {
    return this;
  }


  const std::string UlamType::getUlamTypeName()
  {
    return m_key.getUlamKeyTypeSignatureAsString();
    // REMINDER!! error due to disappearing string:
    //    return m_key.getUlamKeyTypeSignatureAsString().c_str();  
  }


  const char * UlamType::getUlamTypeNameBrief()
  {
    return m_key.getUlamKeyTypeSignatureName();
  }


  UTI UlamType::getUlamTypeIndex()
  {
    return m_index;
  }


   UlamKeyTypeSignature UlamType::getUlamKeyTypeSignature()
  {
    return m_key;
  }    


  const std::string UlamType::getUlamTypeAsStringForC()
  {
    return "?";
  }


  const char * UlamType::getUlamTypeEnumAsString(ULAMTYPE etype)
  {
    return utype_string[etype];
  }


  ULAMTYPE UlamType::getEnumFromUlamTypeString(const char * typestr)
  {
    ULAMTYPE rtnUT = Nav;

    for(u32 i = 0; i < LASTTYPE; i++)
      {
	if(strcmp(utype_string[i], typestr) == 0)
	  {
	    rtnUT = (ULAMTYPE) i;
	    break;
	  }
      }
    return rtnUT;
  }

  bool UlamType::isScalar()
  {
    return (m_key.getUlamKeyTypeSignatureArraySize() == 0);
  }


  u32 UlamType::getArraySize()
  {
    return m_key.getUlamKeyTypeSignatureArraySize();
  }


  u32 UlamType::getBitSize()
  {
    return m_key.getUlamKeyTypeSignatureBitSize();
  }


} //end MFM
