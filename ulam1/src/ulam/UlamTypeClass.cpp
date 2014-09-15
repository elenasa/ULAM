#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClass.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClass::UlamTypeClass(const UlamKeyTypeSignature key, const UTI uti, ULAMCLASSTYPE type) : UlamType(key, uti), m_class(type)
  {}


   ULAMTYPE UlamTypeClass::getUlamTypeEnum()
   {
     return Class;
   }


  const char * UlamTypeClass::getUlamTypeAsSingleLowercaseLetter()
  {
    switch(m_class)
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
    if(m_class == UC_ELEMENT || m_class == UC_QUARK) //skip primitive class (incomplete) 
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
	    
	    if(m_class == UC_QUARK)
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
    UlamType * but = state->getUlamTypeByIndex(csym->getUlamTypeIdx());

    if(but->getUlamClass() == UC_QUARK)
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
    if(but->getUlamClass() == UC_QUARK)
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


  void UlamTypeClass::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%d", (s32) data);
    else
      sprintf(valstr,"%c%d", prefix, (s32) data);
  }


  ULAMCLASSTYPE UlamTypeClass::getUlamClass()
  {
    return m_class;
  }


  void UlamTypeClass::setUlamClass(ULAMCLASSTYPE type)
  {
    m_class = type;
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

    switch(m_class)
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
    assert(m_class != UC_INCOMPLETE);

    if(getArraySize()== 0)
      {
	std::ostringstream mangled;
	std::string nstr = m_key.getUlamKeyTypeSignatureName(state);
	u32 nstrlen = nstr.length();

	mangled << getUlamTypeUPrefix().c_str();
	mangled << DigitCount(getArraySize(), BASE10) << getArraySize() << DigitCount(getBitSize(), BASE10) << getBitSize();
	mangled << DigitCount(nstrlen, BASE10) << nstrlen << nstr.c_str();
	return mangled.str();
      }
    return UlamType::getUlamTypeMangledName(state);  //e.g. Ut_071313Zot
  }


  const std::string UlamTypeClass::getBitSizeTemplateString()
  {
    std::ostringstream mangled;
    if(m_class == UC_QUARK)
      {
	mangled << "<" << UlamType::getTotalBitSize() << ">";
      }
    return mangled.str();
  }



} //end MFM
