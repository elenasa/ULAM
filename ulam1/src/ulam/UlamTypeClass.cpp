#include <iostream>
#include <stdio.h>
#include "UlamTypeClass.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "util.h"

namespace MFM {

  UlamTypeClass::UlamTypeClass(const UlamKeyTypeSignature key, const UTI uti, ULAMCLASSTYPE type) : UlamType(key, uti), m_classType(type), m_bitLength(0)
  {}


  void UlamTypeClass::newValue(UlamValue & val)
  {
    assert((val.isArraySize() == m_key.m_arraySize) && (m_key.m_arraySize == 0));
    val.m_baseArraySlotIndex = 0;  //tmp! newUlamValueClass?
  }


  void UlamTypeClass::deleteValue(UlamValue * val)
  { }


   ULAMTYPE UlamTypeClass::getUlamTypeEnum()
   {
     return Class;
   }


  bool UlamTypeClass::cast(UlamValue & val)
  {
    assert(0);
    //std::cerr << "UlamTypeClass (cast) error! " << std::endl;
    return false;
  }


  const char * UlamTypeClass::getUlamTypeAsSingleLowercaseLetter()
  {
    switch(m_classType)
      {
      case UC_ELEMENT:
	return "e";
      case UC_QUARK:
	return "q";
      case UC_INCOMPLETE:
      default:
	assert(0);
      };
    return "?";
  }


  //prophylactic foward declaration of all classes (i.e. structs)
  void UlamTypeClass::genUlamTypeMangledDefinitionForC(File * fp, CompilerState * state)
  {
    //keep forward's in the MFM namespace
    if(m_classType == UC_ELEMENT || m_classType == UC_QUARK) //skip primitive class (incomplete) 
      {
	state->indent(fp);
	fp->write("namespace MFM{\n");
	
	state->m_currentIndentLevel++;
	
	if(m_classType == UC_QUARK)
	  {
	    state->indent(fp);
	    fp->write("template<u32 POS>\n");
	  }
	
	state->indent(fp);
	fp->write("class ");
	fp->write(getUlamTypeMangledName(state).c_str());
	fp->write(";   //forward \n");
	
	state->m_currentIndentLevel--;
	
	state->indent(fp);
	fp->write("} //MFM\n\n");
      }
  }


  void UlamTypeClass::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState * state)
  {
    sprintf(valstr,"%s", getUlamTypeName(state).c_str());
  }


  bool UlamTypeClass::isZero(const UlamValue & val)
  {
    return true; 
  }


  ULAMCLASSTYPE UlamTypeClass::getUlamClassType()
  {
    return m_classType;
  }


  void UlamTypeClass::setUlamClassType(ULAMCLASSTYPE type)
  {
    m_classType = type;
  }


  const std::string UlamTypeClass::getUlamTypeAsStringForC()
  {
    switch(m_classType)
      {
      case UC_ELEMENT:
	return "Ue_";
      case UC_QUARK:
	return "Uq_";
      case UC_INCOMPLETE:
	return "U?_";
     default:
	assert(0);
      };
  }


  //unlike the other UlamTypes, (base) Class types do not need the bitsize/array in their mangled names,
  // only the key name and whether they are element or quark.
  const std::string UlamTypeClass::getUlamTypeMangledName(CompilerState * state)
  {
    assert(m_classType != UC_INCOMPLETE);

    if(getArraySize()== 0)
      {
	std::ostringstream mangled;
	std::string nstr = m_key.getUlamKeyTypeSignatureName(state);
	u32 nstrlen = nstr.length();
	mangled << getUlamTypeAsStringForC() << countDigits(nstrlen) << nstrlen << nstr.c_str();
	return mangled.str();
      }

    return UlamType::getUlamTypeMangledName(state);  //e.g. Ut_0723213Zot
  }


  const std::string UlamTypeClass::getBitSizeTemplateString()
  {
    std::ostringstream mangled;
    if(m_classType == UC_QUARK)
      {
	mangled << "<" << getBitSize() << ">";
      }
    return mangled.str();
  }

  //'Class' type calculates its size after type labeling
  u32 UlamTypeClass::getBitSize()  
  {
    if(m_classType == UC_ELEMENT)
      assert(m_bitLength <= BITSPERATOM);

    if(m_classType == UC_QUARK)
      assert(m_bitLength <= 32);

    return m_bitLength;
  }


  //'Class' type calculates its size after type labeling
  void UlamTypeClass::setBitSize(u32 bits)  
  {
    if(m_classType == UC_ELEMENT)
      assert(bits <= BITSPERATOM);

    if(m_classType == UC_QUARK)
      assert(bits <= 32);

    m_bitLength = bits;
  }
} //end MFM
