#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sstream>
#include "UlamType.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

#define XX(a,b,c) b,

  static const char * utype_string[] = {
#include "UlamType.inc"
  };
  
#undef XX 



  UlamType::UlamType(const UlamKeyTypeSignature key, const UTI uti) : m_key(key), m_uti(uti), m_wordLength(0)
  {}
 

  UlamType * UlamType::getUlamType()
  {
    return this;
  }


  const std::string UlamType::getUlamTypeName(CompilerState * state)
  {
    return m_key.getUlamKeyTypeSignatureAsString(state);
    // REMINDER!! error due to disappearing string:
    //    return m_key.getUlamKeyTypeSignatureAsString().c_str();  
  }


  const std::string UlamType::getUlamTypeNameBrief(CompilerState * state)
  {
    //return m_key.getUlamKeyTypeSignatureName(state);
    return m_key.getUlamKeyTypeSignatureNameAndBitSize(state);
  }


  UTI UlamType::getUlamTypeIndex()
  {
    return m_uti;
  }


   UlamKeyTypeSignature UlamType::getUlamKeyTypeSignature()
  {
    return m_key;
  }    


  bool UlamType::cast(UlamValue & val, CompilerState& state)
  {
    assert(0);
    //std::cerr << "UlamTypeClass (cast) error! " << std::endl;
    return false;
  }


  void UlamType::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
    {
      sprintf(valstr,"%s", getUlamTypeName(&state).c_str());
    }


  ULAMCLASSTYPE UlamType::getUlamClass()
  {
    return UC_NOTACLASS;
  }


  const std::string UlamType::getUlamTypeAsStringForC()
  {
    assert(!isConstant());

    s32 len = getTotalBitSize(); // includes arrays
    assert(len >= 0);             
    //s32 roundUpSize = calcWordSize(len);
    s32 roundUpSize = getTotalWordSize();

    std::ostringstream ctype;
    ctype << "BitField<BitVector<" << roundUpSize << ">, ";

    if(isScalar())
      ctype << getUlamTypeVDAsStringForC() << ", ";
    else      
      ctype << "VD::BITS, ";  //use BITS for arrays

    ctype << len << ", " << roundUpSize - len << ">";
    return ctype.str();
  }


  const std::string UlamType::getUlamTypeVDAsStringForC()
  {
    assert(0);
    return "VD::NOTDEFINED";
  }


  const std::string UlamType::getUlamTypeMangledName(CompilerState * state)
  {
    std::ostringstream mangled;
    s32 bitsize = getBitSize();
    s32 arraysize = getArraySize();
    //    arraysize = (arraysize == NONARRAYSIZE ? 0 : arraysize); 

    mangled << getUlamTypeUPrefix().c_str();

    if(arraysize > 0)
      mangled << DigitCount(arraysize, BASE10) << arraysize;
    else
      mangled << 10;

    if(bitsize > 0)
      mangled << DigitCount(bitsize, BASE10) << bitsize;
    else
      mangled << 10;

    mangled << state->getDataAsStringMangled(m_key.getUlamKeyTypeSignatureNameId()).c_str();

    return mangled.str();
  }


  const std::string UlamType::getUlamTypeUPrefix()
  {
    return "Ut_";
  }


  const std::string UlamType::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    std::ostringstream imangled;
    imangled << "Ui_" << getUlamTypeMangledName(state) ;
    return  imangled.str();
  }


  bool UlamType::needsImmediateType()
  {
    return !isConstant();
    //return ( (getBitSize() == ANYBITSIZECONSTANT) ? false : true);  //skip constants
  }


  const std::string UlamType::getImmediateStorageTypeAsString(CompilerState * state)
  {
    std::ostringstream ctype;
    //ctype << "BitVector<" << getTotalWordSize() << ">";

    ctype << getUlamTypeImmediateMangledName(state);  // name of struct w typedef(bf) and storage(bv);

    return ctype.str();
  } //getImmediateStorageTypeAsString


  const std::string UlamType::getTmpStorageTypeAsString(CompilerState * state)
  {
    std::string ctype;
    //s32 sizeByIntBits = calcWordSize(getTotalBitSize());
    s32 sizeByIntBits = getTotalWordSize();
    switch(sizeByIntBits)
      {
      case 0:    //e.g. empty quarks
      case 32:
	ctype = "u32";
	break;
      case 64:
	ctype = "u64";
	break;
      default:
	{
	  assert(0);
	  //MSG(getNodeLocationAsString().c_str(), "Need UNPACKED ARRAY", INFO);
	}
	//error!
      };
    
    return ctype;
  } //getTmpStorageTypeAsString


  const char * UlamType::getUlamTypeAsSingleLowercaseLetter()
  {
    return "x";
  }
   

  void UlamType::genUlamTypeMangledDefinitionForC(File * fp, CompilerState * state)
  {
    state->m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName(state);	
    std::ostringstream  ud;
    ud << "Ud_" << mangledName;  //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    s32 sizeByIntBits = getTotalWordSize();
    
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

    state->indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    fp->write("\n");
    state->indent(fp);
    fp->write("{\n");
    
    //typedef bitfield inside struct
    state->m_currentIndentLevel++;
    state->indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeAsStringForC().c_str());  //e.g. BitField
    fp->write(" BF;\n");

    //storage here
    state->indent(fp);
    fp->write("BitVector<");
    fp->write_decimal(sizeByIntBits);
    fp->write(">");
    fp->write(" m_stg;\n");

    //default constructor (used by local vars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stg() { }\n");

    //constructor here (used by const tmpVars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write(" d) : m_stg(d) {}\n");

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


    //read BV method
    genUlamTypeReadDefinitionForC(fp, state);

    //write BV method
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


  void UlamType::genUlamTypeReadDefinitionForC(File * fp, CompilerState * state)
  {
    state->indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write(" read() const { return BF::");
    fp->write(readMethodForCodeGen().c_str());
    fp->write("(m_stg);}\n");
  } //genUlamTypeReadDefinitionForC


  void UlamType::genUlamTypeWriteDefinitionForC(File * fp, CompilerState * state)
  {
    state->indent(fp);
    fp->write("void write(");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write(" v) { BF::");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(m_stg, v); }\n");
  } //genUlamTypeWriteDefinitionForC


  void UlamType::genUlamTypeMangledImmediateDefinitionForC(File * fp, CompilerState * state)
  {
    const std::string mangledName = getUlamTypeImmediateMangledName(state);	
    std::ostringstream  up;

    state->indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeAsStringForC().c_str());  //e.g. BitVector
    fp->write(" ");
    fp->write(mangledName.c_str());	
    fp->write(";\n");
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


  bool UlamType::isConstant()
  {
    return m_key.getUlamKeyTypeSignatureBitSize() == ANYBITSIZECONSTANT;
  }


  bool UlamType::isScalar()
  {
    return (m_key.getUlamKeyTypeSignatureArraySize() == NONARRAYSIZE);
  }


  s32 UlamType::getArraySize()
  {
    return m_key.getUlamKeyTypeSignatureArraySize();
  }


  s32 UlamType::getBitSize()
  {
    if(isConstant())
      return ULAMTYPE_DEFAULTBITSIZE[getUlamTypeEnum()];

    return m_key.getUlamKeyTypeSignatureBitSize(); 
  }


  u32 UlamType::getTotalBitSize()
  {
    s32 arraysize = getArraySize();
    arraysize = (arraysize > NONARRAYSIZE ? arraysize : 1);
    
    s32 bitsize = getBitSize();
    return bitsize * arraysize;
  }


  u32 UlamType::getTotalWordSize()
  {
    return m_wordLength;  //e.g. 32, 64, 96
  }


  const std::string UlamType::readMethodForCodeGen()
  {
    std::string method;
    s32 sizeByIntBits = getTotalWordSize();
    if(isScalar())
      {     
	switch(sizeByIntBits)
	  {
	  case 0:    //e.g. empty quarks
	  case 32:
	    method = "ReadRaw";
	    break;
	  case 64:
	    method = "ReadLongRaw";
	    break;
	  default:
	    method = "ReadUnpackedRaw";  //TBD
	    //MSG(getNodeLocationAsString().c_str(), "Need UNPACKED ARRAY", INFO);
	    assert(0);
	  };
      }
    else
      {
	method = "ReadArrayRaw"; //TBD
      }
    return method;
  } //readMethodForCodeGen


  const std::string UlamType::writeMethodForCodeGen()
  {
    std::string method;
    s32 sizeByIntBits = getTotalWordSize();
    if(isScalar())
      {     
	switch(sizeByIntBits)
	  {
	  case 0:    //e.g. empty quarks
	  case 32:
	    method = "WriteRaw";
	    break;
	  case 64:
	    method = "WriteLongRaw";
	    break;
	  default:
	    method = "WriteUnpackedRaw";  //TBD
	    //MSG(getNodeLocationAsString().c_str(), "Need UNPACKED ARRAY", INFO);
	    assert(0);	 
	  };
      }
    else
      {
	method = "WriteArrayRaw"; //TBD
      }
    return method;
  } //writeMethodForCodeGen
  

  const std::string UlamType::castMethodForCodeGen(UTI nodetype, CompilerState& state)
  {
    std::ostringstream rtnMethod;
    UlamType * nut = state.getUlamTypeByIndex(nodetype);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE nodetypEnum = nut->getUlamTypeEnum();
    s32 sizeByIntBitsToBe = getTotalWordSize();
    s32 sizeByIntBits = nut->getTotalWordSize();

    assert(sizeByIntBitsToBe == sizeByIntBits);

    rtnMethod << "_" << UlamType::getUlamTypeEnumAsString(nodetypEnum) << sizeByIntBits << "To" << UlamType::getUlamTypeEnumAsString(typEnum) << sizeByIntBitsToBe;
    return rtnMethod.str();
  } //castMethodForCodeGen


  void UlamType::genCodeAfterReadingIntoATmpVar(File * fp, UlamValue & uvpass, CompilerState& state)
  {
    return; //nothing to do usually
  }

} //end MFM
