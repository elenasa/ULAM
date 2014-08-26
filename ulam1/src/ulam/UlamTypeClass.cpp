#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClass.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "util.h"

namespace MFM {

  UlamTypeClass::UlamTypeClass(const UlamKeyTypeSignature key, const UTI uti, ULAMCLASSTYPE type) : UlamType(key, uti), m_classType(type)
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
	if(getArraySize() > 0)
	  {
	    genArrayMangledDefinitionForC(fp, state);
	  }	
	else
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
  }


  void UlamTypeClass::genArrayMangledDefinitionForC(File * fp, CompilerState * state)
  {
    state->m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeMangledName(state);	
    std::ostringstream  up;
    up << "Up_" << mangledName;
    std::string upstr = up.str();
    
    state->indent(fp);
    fp->write("#ifndef ");
    fp->write(upstr.c_str());
    fp->write("\n");
    
    state->indent(fp);
    fp->write("#define ");
    fp->write(upstr.c_str());
    fp->write("\n");
    
    state->indent(fp);
    fp->write("namespace MFM{\n");
    
    state->m_currentIndentLevel++;
    
    SymbolClass * csym = NULL;
    assert(state->alreadyDefinedSymbolClass(getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym));
    UlamType * but = csym->getUlamType();
	
    if(but->getUlamClassType() == UC_QUARK)
      {
	state->indent(fp);
	fp->write("template <u32 POS>\n");
      }

    state->indent(fp);
    fp->write("struct ");   //might need template ???
    fp->write(mangledName.c_str());
    fp->write("\n");
    state->indent(fp);
    fp->write("{\n");
    
    state->m_currentIndentLevel++;
    state->indent(fp);
    fp->write(but->getUlamTypeMangledName(state).c_str()); //complete base class
    if(but->getUlamClassType() == UC_QUARK)
      {
	fp->write("<POS>");  
      }

    fp->write(" ");  
    fp->write(getUlamTypeAsSingleLowercaseLetter());    
    fp->write("[");
    fp->write_decimal(getArraySize());
    fp->write("]");
    fp->write(";\n");
    
    state->m_currentIndentLevel--;
    state->indent(fp);
    fp->write("};\n");
  
    state->m_currentIndentLevel--;
    state->indent(fp);
    fp->write("} //MFM\n");
    
    state->indent(fp);
    fp->write("#endif /*");
    fp->write(upstr.c_str());
    fp->write(" */\n\n");
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

#if 0
  const std::string UlamTypeClass::getUlamTypeAsStringForC()
  {
    //what i'd really like is for this to be the basetype for array structs
    // but that requires state.
    // o.w. this is a forward class declaration, not a C type, per se.
    assert(0);
    return getUlamTypeUPrefix(); //??? what is was before
  }
#endif

  const std::string UlamTypeClass::getUlamTypeUPrefix()
  {
    if(getArraySize() > 0)
      return "Ut_";

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
    return "xx";
  }


  // MAY replace the UlamType method for ALL!
  // unlike the other UlamTypes, (base) Class types do not need the bitsize/array in their mangled names,
  // only the key name and whether they are element or quark.
  const std::string UlamTypeClass::getUlamTypeMangledName(CompilerState * state)
  {
    assert(m_classType != UC_INCOMPLETE);

    if(getArraySize()== 0)
      {
	std::ostringstream mangled;
	std::string nstr = m_key.getUlamKeyTypeSignatureName(state);
	u32 nstrlen = nstr.length();

	mangled << getUlamTypeUPrefix().c_str();
	mangled << countDigits(getArraySize()) << getArraySize() << countDigits(getBitSize()) << getBitSize();
	mangled << countDigits(nstrlen) << nstrlen << nstr.c_str();
	return mangled.str();
      }
    return UlamType::getUlamTypeMangledName(state);  //e.g. Ut_071313Zot
  }


  //'Class' type calculates its size after type labeling
  s32 UlamTypeClass::getBitSize()  
  {
    return m_bitLength;
  }


  //'Class' type calculates its size after type labeling
  void UlamTypeClass::setBitSize(s32 bits)  
  {
    if(m_classType == UC_ELEMENT)
      assert(bits <= BITSPERATOM);

    if(m_classType == UC_QUARK)
      assert(bits <= 32);

    //-1 indicates we're trying to recursively calculate it
    m_bitLength = bits;
  }


  const std::string UlamTypeClass::getBitSizeTemplateString()
  {
    std::ostringstream mangled;
    if(m_classType == UC_QUARK)
      {
	mangled << "<" << UlamType::getTotalBitSize() << ">";
      }
    return mangled.str();
  }



} //end MFM
