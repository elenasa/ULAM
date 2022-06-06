#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypePrimitiveString.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypePrimitiveString::UlamTypePrimitiveString(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypePrimitive(key, state)
  {
    s32 bitsize = getBitSize();
    if(bitsize <= 0)
      {
	m_max = m_min = 0;
      }
    else if(bitsize <= MAXBITSPERINT)
      {
	m_wordLengthTotal = calcWordSize(getTotalBitSize());
	m_wordLengthItem = calcWordSize(bitsize);
	m_max = calcBitsizeUnsignedMax(bitsize);
	m_min = 0;
      }
    else if(bitsize <= MAXBITSPERLONG)
      {
	m_wordLengthTotal = calcWordSize(getTotalBitSize());
	m_wordLengthItem = calcWordSize(bitsize);
	m_max = calcBitsizeUnsignedMaxLong(bitsize);
	m_min = 0;
      }
    else
      m_state.abortGreaterThanMaxBitsPerLong();
  }

   ULAMTYPE UlamTypePrimitiveString::getUlamTypeEnum()
   {
     return String;
   }

  bool UlamTypePrimitiveString::isNumericType()
  {
    return false;
  }

  const std::string UlamTypePrimitiveString::getUlamTypeName()
  {
    std::ostringstream key;
    key << m_key.getUlamKeyTypeSignatureName(&m_state); //one size for all Strings

    if(!isScalar())
      {
	s32 arraysize = getArraySize(); //casting diff array sizes (t41429,t41324,t3976)
	if(arraysize >= 0)
	  key << "[" << arraysize << "]";
	else if(arraysize == UNKNOWNSIZE)
	  key << "[" << "UNKNOWN" << "]";
	else
	  key << "[" << arraysize << "?]";
      }

    if(UlamType::isAltRefType())
      key << "&"; //only when ulam programmer put in the &

    u32 keyid = m_state.m_pool.getIndexForDataString(key.str());
    return m_state.m_pool.getDataAsString(keyid);
  }

  const std::string UlamTypePrimitiveString::getUlamTypeNameBrief()
  {
    std::ostringstream key;
    key << m_key.getUlamKeyTypeSignatureName(&m_state); //one size for all Strings

    if(UlamType::isAltRefType())
      key << "&"; //only when ulam programmer put in the & (t3948)

    u32 keyid = m_state.m_pool.getIndexForDataString(key.str());
    return m_state.m_pool.getDataAsString(keyid);
  }

  const std::string UlamTypePrimitiveString::getUlamTypeImmediateMangledName()
  {
    if(needsImmediateType())
      return UlamType::getUlamTypeImmediateMangledName();

    return UlamType::getUlamTypeImmediateMangledName(); //? for constants
  }

  const std::string UlamTypePrimitiveString::castMethodForCodeGen(UTI nodetype)
  {
    //_Bits32ToString32 (and vis versa) defined in CastOps.h
    //return "invalidStringCastMethodForCodeGen";
    UlamType * nut = m_state.getUlamTypeByIndex(nodetype);
    //base type i.e. Bits only
    if(nut->getUlamTypeEnum() != Bits)
      m_state.abortShouldntGetHere();

    return UlamTypePrimitive::castMethodForCodeGen(nodetype);
  } //castMethodForCodeGen

  bool UlamTypePrimitiveString::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx(); //from type

    if(UlamTypePrimitive::safeCast(valtypidx) != CAST_CLEAR) //bad|hazy
      return false;

    u32 wordsize = getTotalWordSize();
    u32 valwordsize = m_state.getTotalWordSize(valtypidx);
    if(wordsize <= MAXBITSPERINT) //tobe
      {
	if(valwordsize <= MAXBITSPERINT)
	  brtn = castTo32(val, typidx);
	else if(valwordsize <= MAXBITSPERLONG)
	  brtn = castTo64(val, typidx); //downcast
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }
    else if(wordsize <= MAXBITSPERLONG) //tobe
      brtn = castTo64(val, typidx);
    else
      {
	std::ostringstream msg;
	msg << "Casting to an unsupported word size: " << wordsize;
	msg << ", Value Type and bit size was: ";
	msg << valtypidx << "," << m_state.getBitSize(valtypidx);
	msg << " TO: ";
	msg << typidx << "," << getBitSize();
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	brtn = false;
      }
    return brtn;
  } //cast

  bool UlamTypePrimitiveString::castTo32(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    u32 data = val.getImmediateData(m_state);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case String:
	break; //data is index, no cast req'd (t3951)
      case Int:
      case Unsigned:
      case Bits:
      case Bool:
      case Unary:
      case Void:
      default:
	//std::cerr << "UlamTypePrimitiveString (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
    return brtn;
  } //castTo32

  bool UlamTypePrimitiveString::castTo64(UlamValue & val, UTI typidx)
  {
    m_state.abortShouldntGetHere();
    return false;
  } //castTo64

  FORECAST UlamTypePrimitiveString::safeCast(UTI typidx)
  {
    FORECAST scr = UlamTypePrimitive::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    bool brtn = true;
    UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
    ULAMTYPE valtypEnum = fmut->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case String:
	break; //only safe cast!
      case Unsigned:
      case Unary:
      case Int:
      case Bool:
      case Bits:
      case Void:
      case UAtom:
      case Class:
	brtn = false;
	break;
      default:
	m_state.abortUndefinedUlamType();
	//std::cerr << "UlamTypePrimitiveString (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn ? CAST_CLEAR : CAST_BAD;
  } //safeCast

  FORECAST UlamTypePrimitiveString::explicitlyCastable(UTI typidx)
  {
    FORECAST scr = UlamTypePrimitive::explicitlyCastable(typidx);

    UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
    ULAMTYPE valtypEnum = fmut->getUlamTypeEnum();

    //allow Bits to be cast to consecutive String index (ulam-5);
    if(valtypEnum == Bits)
      scr = CAST_CLEAR; //t41422

    return scr;
  } //explicitlyCastable

  void UlamTypePrimitiveString::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    assert(data != 0);
    if(prefix == 'z')
      sprintf(valstr,"%s", m_state.getDataAsFormattedUserString(data).c_str());
    else
      sprintf(valstr,"%c%s", prefix, m_state.getDataAsFormattedUserString(data).c_str());
  }

  void UlamTypePrimitiveString::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    return getDataAsString((u32) data, valstr, prefix);
  }

  s32 UlamTypePrimitiveString::getDataAsCs32(const u32 data)
  {
    return _Unsigned32ToCs32(data, getBitSize());
  }

  u32 UlamTypePrimitiveString::getDataAsCu32(const u32 data)
  {
    return _Unsigned32ToCu32(data, getBitSize());
  }

  s64 UlamTypePrimitiveString::getDataAsCs64(const u64 data)
  {
    return _Unsigned64ToCs64(data, getBitSize());
  }

  u64 UlamTypePrimitiveString::getDataAsCu64(const u64 data)
  {
    return _Unsigned64ToCu64(data, getBitSize());
  }

  s32 UlamTypePrimitiveString::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
  {
    s32 bitsize = getBitSize();
    s32 tobitsize = UNKNOWNSIZE;
    s32 wordsize = getTotalWordSize();
    switch(tobUT)
      {
      case Unary:
	tobitsize = getMax();
	break;
      case Int:
	tobitsize = bitsize + 1;
	break;
      case Bool:
	tobitsize = 1;
	break;
      case Unsigned:
      case Bits:
	tobitsize = bitsize; //self
	break;
      case Void:
	tobitsize = 0;
	break;
      case UAtom:
      case Class:
	break;
      default:
	m_state.abortUndefinedUlamType();
	//std::cerr << "UlamTypePrimitiveString (convertTo) error! : " << tobUT << std::endl;
      };
    return (tobitsize > wordsize ? wordsize : tobitsize);
  } //bitsizeToConvertTypeTo

  const std::string UlamTypePrimitiveString::getTmpStorageTypeAsString()
  {
    return UlamType::getTmpStorageTypeAsString();
  }

  TMPSTORAGE UlamTypePrimitiveString::getTmpStorageTypeForTmpVar()
  {
    return UlamType::getTmpStorageTypeForTmpVar();
  }

  const std::string UlamTypePrimitiveString::readMethodForCodeGen()
  {
    return UlamType::readMethodForCodeGen();
  } //readMethodForCodeGen

  const std::string UlamTypePrimitiveString::writeMethodForCodeGen()
  {
    return UlamType::writeMethodForCodeGen();
  } //writeMethodForCodeGen

  void UlamTypePrimitiveString::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    UlamTypePrimitive::genUlamTypeAutoReadDefinitionForC(fp);
  }

  void UlamTypePrimitiveString::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    UlamTypePrimitive::genUlamTypeAutoWriteDefinitionForC(fp);
  }

  //generates immediates with local storage; registration class num and string index
  void UlamTypePrimitiveString::genUlamTypeMangledDefinitionForC(File * fp)
  {
    u32 len = getTotalBitSize(); //could be 0, includes arrays

    m_state.m_currentIndentLevel = 0;

    UTI anyuti = Nav;
    AssertBool anyDefined =  m_state.anyDefinedUTI(m_key, anyuti);
    assert(anyDefined);
    UTI scalaruti = m_state.getUlamTypeAsScalar(anyuti);
    UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
    const std::string scalarmangledName = scalarut->getUlamTypeMangledName();
    const std::string mangledName = getUlamTypeImmediateMangledName();
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();

    std::ostringstream  ud;
    ud << "Ud_" << mangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    m_state.indent(fp);
    fp->write("#ifndef ");
    fp->write(udstr.c_str());
    fp->write("\n");

    m_state.indent(fp);
    fp->write("#define ");
    fp->write(udstr.c_str());
    fp->write("\n");

    m_state.indent(fp);
    fp->write("namespace MFM{\n");
    fp->write("\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    fp->write(" : public ");
    fp->write("BitVectorBitStorage");
    fp->write("<EC, BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("u> >\n");

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    UlamType::genStandardConfigTypedefTypenames(fp, m_state);

    m_state.indent(fp);
    fp->write("typedef BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("> BV;"); GCNL;

    m_state.indent(fp);
    fp->write("typedef BitVectorBitStorage<EC, BV> BVS;"); GCNL;
    fp->write("\n");

    //put read/write methods before constructrtors that may use them.
    //read BV method
    genUlamTypeReadDefinitionForC(fp);

    //write BV method
    genUlamTypeWriteDefinitionForC(fp);

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { }"); GCNL;

    //constructor here (used by const tmpVars)
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write(mangledName.c_str());
	fp->write("(");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32
	//fp->write(" regnum, ");
	//fp->write(getTmpStorageTypeAsString().c_str()); //u32
	fp->write(" sidx) { ");
	//fp->write("setRegistrationNumber(regnum); setStringIndex(sidx); }"); GCNL;
	fp->write("setStringIndex(sidx); }"); GCNL;
      }
    else
      {
	s32 arraysize = getArraySize();

	//array initialization constructor here (used by const tmpVars);
	// in C, the array is just a pointer (since not within a struct);
	m_state.indent(fp);
	fp->write(mangledName.c_str());
	fp->write("(const u32");
	fp->write(" d[");
	fp->write_decimal_unsigned(UlamType::getTotalNumberOfWords());
	fp->write("]) { BV tmpbv; tmpbv.FromArray(d); ");
	if(len > MAXBITSPERLONG)
	  {
	    fp->write("this->write(tmpbv); }"); GCNL;
	  }
	else
	  {
	    fp->write(getTmpStorageTypeAsString().c_str()); //u32,u64
	    fp->write(" tmps = tmpbv.");
	    fp->write(readMethodForCodeGen().c_str());
	    fp->write("(0u,"); //startidx
	    fp->write_decimal_unsigned(len);
	    fp->write("u); ");
	    fp->write("this->write(tmps); }"); GCNL; //t41276
	  }

	//array initialization constructor here from BV for constant array (t41277)
	m_state.indent(fp);
	fp->write(mangledName.c_str());
	fp->write("(const BV& bvsarg) : BVS(bvsarg) { }"); GCNL;


	//array initialization constructor here from u64 for constant array[2] (t41277)
	if((len > MAXBITSPERINT) && (len <= MAXBITSPERLONG))
	  {
	    m_state.indent(fp);
	    fp->write(mangledName.c_str());
	    fp->write("(const u64 dl){const u32 mask=_GetNOnes31(20); for(u32 i=0;i<");
	    fp->write_decimal_unsigned(arraysize);
	    fp->write(";i++){");
	    fp->write("u32 d0=(u32)(dl>>20u*i);d0&=mask;writeArrayItem(d0,");
	    fp->write_decimal_unsigned(arraysize);
	    fp->write("-i-1,20u);}}"); GCNL; //reverse the order t41277
	  }
      }

    //copy constructor here (return by value)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str()); //u32
    fp->write("& other) : BVS() { "); //-Wextra
    if(len <= MAXBITSPERLONG)
      {
	fp->write("this->write");
	fp->write("(other.");
	fp->write("read()); }"); GCNL;
      }
    else
      {
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write(" tmpbv; other.read(tmpbv); write(tmpbv); } "); GCNL;
      }

    //constructor from ref of same type
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& d) { "); //uc consistent with atomref
    if(len <= MAXBITSPERLONG)
      {
	fp->write("this->write(d.read()); }"); GCNL;
      }
    else
      {
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write(" tmpbv; d.read(tmpbv); this->write(tmpbv); } "); GCNL;
      }

    //default destructor (intentionally left out)

    //for var args native funcs, non-refs, required of a BitStorage
    UlamType::genGetUlamTypeMangledNameDefinitionForC(fp);

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("};\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //MFM\n");

    m_state.indent(fp);
    fp->write("#endif /*");
    fp->write(udstr.c_str());
    fp->write(" */\n\n");
  } //genUlamTypeMangledDefinitionForC

  void UlamTypePrimitiveString::genUlamTypeReadDefinitionForC(File * fp)
  {
    u32 totbitsize = getTotalBitSize();
    if(totbitsize <= MAXBITSPERLONG)
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64
	fp->write(" read");
	fp->write("() const { return BVS::");
	fp->write(readMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal_unsigned(totbitsize);
	fp->write("u); ");
	if(isScalar())
	  {
	    fp->write("}"); GCNL; //done
	  }
	else
	  {
	    fp->write("} /* entire string array */ "); GCNL; //done
	  }
      }
    else
      {
	//array bit vector (t41276)
	m_state.indent(fp);
	fp->write("void read(BV & rtnbv) ");
	fp->write("const { ");
	fp->write("this->BVS::");
	fp->write("ReadBV(0u, rtnbv);");
	fp->write("} //reads entire BV"); GCNL;
      }

    if(!isScalar())
      {
	//reads an item of array;
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return ");
	fp->write("this->BVS::");
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, itemlen); } //reads BV array item"); GCNL;
      }
    else
      {
	//access string index; regnum gone (ulam-4)
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32
	fp->write(" getStringIndex");
	fp->write("() const { return BVS::Read(");
	fp->write_decimal_unsigned(REGNUMBITS); //=0
	fp->write("u, ");
	fp->write_decimal_unsigned(STRINGIDXBITS);
	fp->write("u); }"); GCNL; //done
	fp->write("\n");
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypePrimitiveString::genUlamTypeWriteDefinitionForC(File * fp)
  {
    u32 totbitsize = getTotalBitSize();
    if(totbitsize <= MAXBITSPERLONG)
      {
	m_state.indent(fp);
	fp->write("void write");
	fp->write("(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32, s64 or u64
	fp->write("& v) { BVS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal_unsigned(totbitsize);
	fp->write("u, v); }"); GCNL;
      }
    else
      {
	m_state.indent(fp);
	fp->write("void ");
	fp->write(" write(const BV");
	fp->write("& bv) { BVS::");
	fp->write("WriteBV(0u, bv); ");
	fp->write("} //writes entire BV"); GCNL;
      }

    if(!isScalar())
      {
	//reads an item of array;
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //u32
	fp->write("& v, const u32 index, const u32 itemlen) {");
	fp->write("this->BVS::");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, itemlen, v); } //writes BV array item"); GCNL;
      }
    else
      {
    	//set string index; regnum gone (ulam-4)
	m_state.indent(fp);
	fp->write("void setStringIndex");
	fp->write("(u32 sidx) { BVS::Write(");
	fp->write_decimal_unsigned(REGNUMBITS); //=0
	fp->write("u, ");
	fp->write_decimal_unsigned(STRINGIDXBITS);
	fp->write("u, sidx); }"); GCNL; //done
	fp->write("\n");
      }
  } //genUlamTypeWriteDefinitionForC

} //end MFM
