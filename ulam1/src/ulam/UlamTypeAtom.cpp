#include <sstream>
#include <stdio.h>
#include "UlamTypeAtom.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeAtom::UlamTypeAtom(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypeAtom::getUlamTypeEnum()
   {
     return Atom;
   }


  void UlamTypeAtom::getUlamValueAsString(const UlamValue& val, char * valstr, CompilerState& state)
  {
    sprintf(valstr,"%s", getUlamTypeName(&state).c_str());
  }


  const std::string UlamTypeAtom::getUlamTypeAsStringForC()
  {
    std::ostringstream ctype;
    ctype << "BitVector<" << BITSPERATOM << ">";
    return ctype.str();
  }


  const char * UlamTypeAtom::getUlamTypeAsSingleLowercaseLetter()
  {
    return "a";
  }


  void UlamTypeAtom::genUlamTypeMangledDefinitionForC(File * fp, CompilerState * state)
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
    
    state->indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    fp->write("\n");
    state->indent(fp);
    fp->write("{\n");
    
    state->m_currentIndentLevel++;
    state->indent(fp);
    fp->write(getUlamTypeAsStringForC().c_str());
    fp->write(" ");  
    fp->write(getUlamTypeAsSingleLowercaseLetter());	
    //fp->write("[");
    //fp->write_decimal(BITSPERATOM/32);  //BITSPERATOM/32
    //fp->write("]");
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

 
} //end MFM
