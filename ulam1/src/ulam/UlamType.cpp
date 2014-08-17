#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sstream>
#include "UlamType.h"
#include "CompilerState.h"

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


  const std::string UlamType::getUlamTypeMangledName()
  {
    return m_key.getUlamKeyTypeSignatureMangledName();
  }


  const char * UlamType::getUlamTypeAsSingleLowercaseLetter()
  {
    return "x";
  }
   
 
  void UlamType::genUlamTypeMangledDefinitionForC(File * fp, CompilerState& state)
  {
    state.m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeMangledName();	
    std::ostringstream  up;
    up << "Up_" << mangledName;
    std::string upstr = up.str();
    
    state.indent(fp);
    fp->write("#ifndef ");
    fp->write(upstr.c_str());
    fp->write("\n");
    
    state.indent(fp);
    fp->write("#define ");
    fp->write(upstr.c_str());
    fp->write("\n");
    
    state.indent(fp);
    fp->write("namespace MFM{\n");
    
    state.m_currentIndentLevel++;
    
    u32 arraysize = getArraySize();
    if(arraysize > 0)
      {
	state.indent(fp);
	fp->write("struct ");
	fp->write(mangledName.c_str());
	fp->write("\n");
	state.indent(fp);
	fp->write(" {\n");
    
	state.m_currentIndentLevel++;
	state.indent(fp);
	fp->write(getUlamTypeAsStringForC().c_str());
	fp->write(" ");  //getsinglelowercaseletterfortype
	fp->write(getUlamTypeAsSingleLowercaseLetter());
	
	u32 arraysize = getArraySize();
	if( arraysize > 0)
	  {
	    fp->write("[");
	    fp->write_decimal(arraysize);
	    fp->write("]");
	  }
	fp->write(";\n");

	state.m_currentIndentLevel--;
	state.indent(fp);
	fp->write("};\n");
      }
    else
      {
	state.indent(fp);
	fp->write("typedef ");
	fp->write(getUlamTypeAsStringForC().c_str());
	fp->write(" ");
	fp->write(mangledName.c_str());	
	fp->write(";\n");
      }

    state.m_currentIndentLevel--;
    state.indent(fp);
    fp->write("} //MFM\n");
    
    state.indent(fp);
    fp->write("#endif /*");
    fp->write(upstr.c_str());
    fp->write(" */\n\n");
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
