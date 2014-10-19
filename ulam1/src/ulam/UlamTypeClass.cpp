#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClass.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClass::UlamTypeClass(const UlamKeyTypeSignature key, const UTI uti, ULAMCLASSTYPE type) : UlamType(key, uti), m_class(type)
  {
    m_wordLength = calcWordSize(getTotalBitSize());
  }


   ULAMTYPE UlamTypeClass::getUlamTypeEnum()
   {
     return Class;
   }


  const std::string UlamTypeClass::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    if(needsImmediateType())   //no longer does quark have an immediate type
      return UlamType::getUlamTypeImmediateMangledName(state);

    //XXX  TODO: needs to return a struct that has a T, not a bare T without storage
    return getImmediateStorageTypeAsString(state);  
  }


  bool UlamTypeClass::needsImmediateType()
  {
    // no "naked" quarks; all living within an atom somehwere.
    //return (m_class == UC_ELEMENT || m_class == UC_INCOMPLETE ? false : true);
    return false; 
  }


  const std::string UlamTypeClass::getTmpStorageTypeAsString(CompilerState * state)
  {
    return "T";
#if 0
    if(m_class == UC_QUARK)
      {
	//return UlamType::getUlamTypeImmediateMangledName(state); //no "naked" quarks
	return state->getUlamTypeByIndex(Atom)->getUlamTypeMangledName(state); // T
      }
    else if(m_class == UC_ELEMENT)
      {
	return state->getUlamTypeByIndex(Atom)->getUlamTypeMangledName(state); // T
      }
    else
      assert(0);
    return ""; //error!
#endif
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
	    genArrayMangledDefinitionForC(fp, state);  //???
	  }	
	else
	  {
	    state->indent(fp);
	    fp->write("namespace MFM{\n");
	    
	    state->m_currentIndentLevel++;
	    
	    if(m_class == UC_QUARK)
	      {
		state->indent(fp);
		fp->write("template<class C, u32 POS>\n");
	      }
	    else  //element
	      {
		state->indent(fp);
		fp->write("template<class CC>\n");
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


  //see SymbolVariableDataMember printPostfix for recursive output
  void UlamTypeClass::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%d", data);
    else
      sprintf(valstr,"%c%d", prefix, data);
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


  const std::string UlamTypeClass::getUlamTypeVDAsStringForC()
  {
    //assert(m_class == UC_QUARK); what if element???
    return "VD::BITS";  //for quark use bits
  }


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


  const std::string UlamTypeClass::getUlamTypeNameBrief(CompilerState * state)
  {
    return m_key.getUlamKeyTypeSignatureName(state);
  }


} //end MFM
