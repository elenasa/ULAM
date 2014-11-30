#include <sstream>
#include <stdio.h>
#include "UlamTypeAtom.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeAtom::UlamTypeAtom(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
  }


   ULAMTYPE UlamTypeAtom::getUlamTypeEnum()
   {
     return Atom;
   }


  const std::string UlamTypeAtom::getUlamTypeVDAsStringForC()
  {
    return "VD::ATOM";
  }

#if 0
  const std::string UlamTypeAtom::getUlamTypeMangledName(CompilerState * state)
  {
    //return "T";
  }
#endif

#if 0
  const std::string UlamTypeAtom::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    if(needsImmediateType())
      {
	return UlamType::getUlamTypeImmediateMangledName(state);
      }
    
    return "T";
  }
#endif

  bool UlamTypeAtom::needsImmediateType()
  {
    //return false;
    return true;
  }


  const std::string UlamTypeAtom::getTmpStorageTypeAsString(CompilerState * state)
  {
    return "T";
  }

  const std::string UlamTypeAtom::getImmediateStorageTypeAsString(CompilerState * state)
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName(state) << "<CC>";
    return ctype.str();
  } //getImmediateStorageTypeAsString


  const char * UlamTypeAtom::getUlamTypeAsSingleLowercaseLetter()
  {
    return "a";  //self ???
  }


  //whole atom
  void UlamTypeAtom::genUlamTypeMangledDefinitionForC(File * fp, CompilerState * state)
  {
    assert(isScalar());

    state->m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName(state);
    std::ostringstream  ud;
    ud << "Ud_" << mangledName;  //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    //s32 len = getTotalBitSize();  //BITSPERATOM

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

    //storage here in atom
    state->indent(fp);
    fp->write("T m_stg;  //storage here!\n");

    //default constructor (used by local vars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stg() { }\n");

    //constructor here (used by const tmpVars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //T
    fp->write(" d) : m_stg(d) {}\n");
    
    //copy constructor here (used by func call return values)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getImmediateStorageTypeAsString(state).c_str());
    fp->write("& d) : m_stg(d.m_stg) {}\n");

    //default destructor (for completeness)
    state->indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

    //read 'entire atom' method
    genUlamTypeReadDefinitionForC(fp, state);

    //write 'entire atom' method
    genUlamTypeWriteDefinitionForC(fp, state);


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


  void UlamTypeAtom::genUlamTypeReadDefinitionForC(File * fp, CompilerState * state)
  {
    // arrays are handled separately
    assert(isScalar());

    //not an array
    state->indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //T
    fp->write(" read() const { return m_stg; }\n");
  } //genUlamTypeReadDefinitionForC


  void UlamTypeAtom::genUlamTypeWriteDefinitionForC(File * fp, CompilerState * state)
  {
    // arrays are handled separately
    assert(isScalar());

    // here, must be scalar
    state->indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //T
    fp->write(" v) { m_stg = v; }\n");
  } //genUlamTypeWriteDefinitionForC


  PACKFIT UlamTypeAtom::getPackable()
  {
    return UNPACKED;
  }

} //end MFM
