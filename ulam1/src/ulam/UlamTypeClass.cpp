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
    if(needsImmediateType())
      {
	return UlamType::getUlamTypeImmediateMangledName(state);
      }

    return "T";
  }


  bool UlamTypeClass::needsImmediateType()
  {
    // now allowing right-justified naked quarks
    // no "naked" quarks; all living within an atom somehwere.
    return (m_class == UC_ELEMENT || m_class == UC_INCOMPLETE || getBitSize() == 0 ? false : true);
  }


  const std::string UlamTypeClass::getTmpStorageTypeAsString(CompilerState * state)
  {
    if(m_class == UC_QUARK)
      {
	return UlamType::getTmpStorageTypeAsString(state); // u32 or u64
      }
    else if(m_class == UC_ELEMENT)
      {
 	return "T";
      }
    else
      assert(0);
    return ""; //error!
  }


  const std::string UlamTypeClass::getImmediateStorageTypeAsString(CompilerState * state)
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName(state);

    //if(m_class == UC_QUARK)
    if(m_class == UC_QUARK && isScalar())
      ctype << "<CC>";  

    return ctype.str();
  } //getImmediateStorageTypeAsString


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

#if 0
  // old way
  //prophylactic foward declaration of all classes (i.e. structs)
  void UlamTypeClass::GENULAMTYPEMANGLEDDEFINITIONFORC(File * fp, CompilerState * state)
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
#endif

  //old way: (arrays TBD!)
  void UlamTypeClass::genArrayMangledDefinitionForC(File * fp, CompilerState * state)
  {
    state->m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeMangledName(state);	
    std::ostringstream  ud;
    ud << "Ud_" << mangledName;
    std::string udstr = ud.str();
    
    state->indent(fp);
    fp->write("#ifndef ");
    fp->write(udstr.c_str());
    fp->write("\n");
    
    state->indent(fp);
    fp->write("#define ");
    fp->write(udstr.c_str());
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
    fp->write(udstr.c_str());
    fp->write(" */\n\n");
  } //genArrayMangledDefinitionForC


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


  bool UlamTypeClass::isConstant()
  {
    return false;   //e.g. zero-size quark is not a constant
  }


  s32 UlamTypeClass::getBitSize()
  {
    s32 bitsize = m_key.getUlamKeyTypeSignatureBitSize();
    return bitsize < 0 ? 0 : bitsize; //allow for empty quarks 
  }


  //quarks are right-justified in an atom space
  const std::string UlamTypeClass::getUlamTypeAsStringForC()
  {
    assert(m_class != UC_INCOMPLETE);
    if(m_class == UC_QUARK)
      {
	//	s32 len = getTotalBitSize();
	//std::ostringstream ctype;
	//ctype << "BitField<BitVector<" << BITSPERATOM << ">, ";
	//ctype << "VD::BITS, ";  //use BITS for quarks
	//ctype << len << ", " << BITSPERATOM - len << ">";
	//return ctype.str();
	return UlamType::getUlamTypeAsStringForC();
      }
   
    return "T"; //for elements
  }


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


  void UlamTypeClass::genUlamTypeMangledDefinitionForC(File * fp, CompilerState * state)
  {
    assert(m_class == UC_QUARK);

    // immediate quark arrays are treated like immediate primitives, not quarks
    if(!isScalar())
      return UlamType::genUlamTypeMangledDefinitionForC(fp, state);

    state->m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName(state);
    std::ostringstream  ud;
    ud << "Ud_" << mangledName;  //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    //s32 sizeByIntBits = getTotalWordSize();
    s32 len = getTotalBitSize();

    state->indent(fp);
    fp->write("#ifndef ");
    fp->write(udstr.c_str());
    fp->write("\n");
    
    state->indent(fp);
    fp->write("#define ");
    fp->write(udstr.c_str());
    fp->write("\n");
    
    state->indent(fp);
    fp->write("namespace MFM{\n");
    fp->write("\n");

    state->m_currentIndentLevel++;

    //forward declaration of quark (before struct!)
    state->indent(fp);
    fp->write("template<class CC, u32 POS> class ");
    fp->write(getUlamTypeMangledName(state).c_str());
    fp->write(";  //forward\n\n");

    state->indent(fp);
    fp->write("template<class CC>\n");

    state->indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    fp->write("\n");
    state->indent(fp);
    fp->write("{\n");

    state->m_currentIndentLevel++;
    
    //typedef atomic parameter type inside struct
    state->indent(fp);
    fp->write("typedef typename CC::ATOM_TYPE T;\n");
    state->indent(fp);
    fp->write("typedef typename CC::PARAM_CONFIG P;\n");
    state->indent(fp);
    fp->write("enum { BPA = P::BITS_PER_ATOM };\n");
    fp->write("\n");

    //quark typedef (right-justified)
    state->indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeMangledName(state).c_str());
    fp->write("<CC, ");
    fp->write_decimal(BITSPERATOM - len);
    fp->write("> Us;\n");

    state->indent(fp);
    fp->write("typedef AtomicParameterType<");
    fp->write("CC");  //BITSPERATOM
    fp->write(", ");
    fp->write(getUlamTypeVDAsStringForC().c_str());
    fp->write(", ");
    fp->write_decimal(len);   //include arraysize
    fp->write(", ");
    fp->write_decimal(BITSPERATOM - len); //right-justified    
    fp->write("> ");
    fp->write(" Up_Us;\n");
    fp->write("\n");

    //storage here in atom
    state->indent(fp);
    fp->write("T m_stg;\n");
    fp->write("\n");

    //default constructor (used by local vars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stg() { }\n");

    //constructor here (used by const tmpVars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
    //fp->write(" d) : m_t(d) {}\n");
    fp->write(" d) { write(d); }\n");

    //copy constructor here (used by func call return values)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getImmediateStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write("& d) : m_stg(d.m_stg) {}\n");

    //default destructor (for completeness)
    state->indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

    //read 'entire quark' method
    genUlamTypeReadDefinitionForC(fp, state);

    //write 'entire quark' method
    genUlamTypeWriteDefinitionForC(fp, state);

    // getBits method for scalar
    if(isScalar())
      {
	state->indent(fp);
	fp->write("BitVector<BPA>& getBits() { return m_stg.GetBits(); }\n");
      }

    state->m_currentIndentLevel--;
    state->indent(fp);
    fp->write("};\n");
    
    state->m_currentIndentLevel--;
    state->indent(fp);
    fp->write("} //MFM\n");
    
    state->indent(fp);
    fp->write("#endif /*");
    fp->write(udstr.c_str());
    fp->write(" */\n\n");
  } //genUlamTypeMangledDefinitionForC


  void UlamTypeClass::genUlamTypeReadDefinitionForC(File * fp, CompilerState * state)
  {
  // immediate quark arrays are treated like immediate primitives, not quarks
    if(!isScalar())
      return UlamType::genUlamTypeReadDefinitionForC(fp, state);

    state->indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write(" read() const { return Up_Us::");
    fp->write(readMethodForCodeGen().c_str());

    if(isScalar())
      fp->write("(m_stg.GetBits() ); }\n");
    else
      {
	//reads entire array
	s32 totbitsize = getTotalBitSize();

	fp->write("(m_stg.GetBits(), ");
	fp->write_decimal(totbitsize);
	fp->write(", ");
	fp->write_decimal(totbitsize);
	fp->write(", ");
	fp->write_decimal(BITSPERATOM - totbitsize);
	fp->write("); }   //reads entire array\n");

	state->indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
	fp->write(" readArray(");
	fp->write("u32 len, u32 pos) const { return Up_Us::");
	fp->write(readMethodForCodeGen().c_str());
	fp->write("(m_stg.GetBits(), ");
	fp->write_decimal(getTotalBitSize());
	fp->write("u, len, pos");
	fp->write("); }\n");
      }
  } //genUlamTypeReadDefinitionForC


  void UlamTypeClass::genUlamTypeWriteDefinitionForC(File * fp, CompilerState * state)
  {
    // immediate quark arrays are treated like immediate primitives, not quarks
    if(!isScalar())
      return UlamType::genUlamTypeWriteDefinitionForC(fp, state);

    state->indent(fp);
    fp->write("void write(");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write(" v) { Up_Us::");
    fp->write(writeMethodForCodeGen().c_str());

    if(isScalar())
      fp->write("(m_stg.GetBits(), v); }\n");
    else
      {
	//writes entire array
	s32 totbitsize = getTotalBitSize();
	fp->write("(m_stg.GetBits(), v, ");
	fp->write_decimal(totbitsize);
	fp->write("u, ");
	fp->write_decimal(totbitsize);
	fp->write("u, ");
	fp->write_decimal(BITSPERATOM - totbitsize);
	fp->write("u); }   //writes entire array\n");

	state->indent(fp);
	fp->write("void writeArray(");
	fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
	fp->write(" v, u32 len, u32 pos) { Up_Us::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(m_stg.GetBits(), v, ");
	fp->write_decimal(getTotalBitSize());
       	fp->write("u, len, pos");
	fp->write("); }\n");
      }
  } //genUlamTypeWriteDefinitionForC


} //end MFM
