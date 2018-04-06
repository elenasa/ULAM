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

  const std::string UlamTypePrimitiveString::getUlamTypeImmediateMangledName()
  {
    if(needsImmediateType())
      return UlamType::getUlamTypeImmediateMangledName();

    return UlamType::getUlamTypeImmediateMangledName(); //? for constants
  }

  const std::string UlamTypePrimitiveString::castMethodForCodeGen(UTI nodetype)
  {
    //_String32ToString32 undefined in CastOps.h
    m_state.abortShouldntGetHere();
    return "invalidStringCastMethodForCodeGen";
  }

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

  void UlamTypePrimitiveString::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    UlamTypePrimitive::genUlamTypeAutoReadDefinitionForC(fp);

    //access regnum and string index, separately
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32 or u64
	fp->write(" getRegistrationNumber");
	fp->write("() const { return UlamRef<EC>(*this, 0u, ");
	fp->write_decimal_unsigned(REGNUMBITS);
	fp->write("u, NULL, UlamRef<EC>::PRIMITIVE).Read(); }"); GCNL; //done

	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32 or u64
	fp->write(" getStringIndex");
	fp->write("() const { return UlamRef<EC>(*this, ");
	fp->write_decimal_unsigned(REGNUMBITS);
	fp->write("u, ");
	fp->write_decimal_unsigned(STRINGIDXBITS);
	fp->write("u, NULL, UlamRef<EC>::PRIMITIVE).Read(); }"); GCNL; //done
	fp->write("\n");
      }
  }

  void UlamTypePrimitiveString::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    UlamTypePrimitive::genUlamTypeAutoWriteDefinitionForC(fp);

    //access regnum and string index, separately
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("void setRegistrationNumber");
	fp->write("(u32 regnum) { UlamRef<EC>(*this, 0u, ");
	fp->write_decimal_unsigned(REGNUMBITS);
	fp->write("u, NULL, UlamRef<EC>::PRIMITIVE).Write(regnum); }"); GCNL; //done

	m_state.indent(fp);
	fp->write("void setStringIndex");
	fp->write("(u32 sidx) { UlamRef<EC>(*this, ");
	fp->write_decimal_unsigned(REGNUMBITS);
	fp->write("u, ");
	fp->write_decimal_unsigned(STRINGIDXBITS);
	fp->write("u, NULL, UlamRef<EC>::PRIMITIVE).Write(sidx); }"); GCNL; //done
	fp->write("\n");
      }
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

    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("enum { REG_NUM_BITS = ");
	fp->write_decimal_unsigned(REGNUMBITS);
	fp->write(", STR_IDX_BITS = ");
	fp->write_decimal_unsigned(STRINGIDXBITS);
	fp->write(", STR_IDX_MASK = ");
	fp->write_decimal_unsigned(STRINGIDXMASK);
	fp->write("};"); GCNL;

	//helper methods
	m_state.indent(fp);
	fp->write("static u32 getRegNum(u32 combinedidx) { return (combinedidx >> REG_NUM_BITS); }");
	GCNL;
	m_state.indent(fp);
	fp->write("static u32 getStrIdx(u32 combinedidx) { return (combinedidx & STR_IDX_MASK); }");
	GCNL;
	m_state.indent(fp);
	fp->write("static u32 makeCombinedIdx(u32 regnum, u32 stridx) { return ((regnum << REG_NUM_BITS) | (stridx & STR_IDX_MASK)); }");
	GCNL;
      }

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
	fp->write(" regnum, ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32
	fp->write(" sidx) { ");
	fp->write("setRegistrationNumber(regnum); setStringIndex(sidx); }"); GCNL;
      }
    else
      {
	//array initialization constructor here (used by const tmpVars);
	// in C, the array is just a pointer (since not within a struct);
	m_state.indent(fp);
	fp->write(mangledName.c_str());
	fp->write("(const u32");
	fp->write(" d[");
	fp->write_decimal_unsigned(UlamType::getTotalNumberOfWords());
	fp->write("]) : BVS(d) { }"); GCNL;

	//array initialization constructor here from BV for constant array (t41277)
	m_state.indent(fp);
	fp->write(mangledName.c_str());
	fp->write("(const BVS& bvsarg) : BVS(bvsarg) { }"); GCNL;

	//array initialization constructor here from u64 for constant array[2] (t41277)
	if((len > MAXBITSPERINT) && (len <= MAXBITSPERLONG))
	  {
	    m_state.indent(fp);
	    fp->write(mangledName.c_str());
	    fp->write("(const u64 dl) { u32 d0=(u32)(dl>>32u); BVS::Write(0,32,d0); u32 d1=(u32)dl; BVS::Write(32,32,d1);}"); GCNL;
	  }
      }

    //copy constructor here (return by value)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str()); //u32
    fp->write("& other) { ");
    fp->write("this->write");
    fp->write("(other.");
    fp->write("read");
    fp->write("()); }"); GCNL;

    //constructor from ref of same type
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& d) { "); //uc consistent with atomref
    fp->write("this->write(");
    fp->write("d.read()); }"); GCNL;

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
    UlamTypePrimitive::genUlamTypeReadDefinitionForC(fp);

    //access regnum and string index, separately
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32 or u64
	fp->write(" getRegistrationNumber");
	fp->write("() const { return BVS::Read(0u, ");
	fp->write_decimal_unsigned(REGNUMBITS);
	fp->write("u); }"); GCNL; //done

	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32 or u64
	fp->write(" getStringIndex");
	fp->write("() const { return BVS::Read(");
	fp->write_decimal_unsigned(REGNUMBITS);
	fp->write("u, ");
	fp->write_decimal_unsigned(STRINGIDXBITS);
	fp->write("u); }"); GCNL; //done
	fp->write("\n");
      }
  }

  void UlamTypePrimitiveString::genUlamTypeWriteDefinitionForC(File * fp)
  {
    UlamTypePrimitive::genUlamTypeWriteDefinitionForC(fp);

    //access regnum and string index, separately
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("void setRegistrationNumber");
	fp->write("(u32 regnum) { BVS::Write(0u, ");
	fp->write_decimal_unsigned(REGNUMBITS);
	fp->write("u, regnum); }"); GCNL; //done

	m_state.indent(fp);
	fp->write("void setStringIndex");
	fp->write("(u32 sidx) { BVS::Write(");
	fp->write_decimal_unsigned(REGNUMBITS);
	fp->write("u, ");
	fp->write_decimal_unsigned(STRINGIDXBITS);
	fp->write("u, sidx); }"); GCNL; //done
	fp->write("\n");
      }
  }


} //end MFM
