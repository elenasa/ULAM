#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypePrimitiveInt.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypePrimitiveInt::UlamTypePrimitiveInt(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypePrimitive(key, state)
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
	m_max = calcBitsizeSignedMax(bitsize);
	m_min = calcBitsizeSignedMin(bitsize);
      }
    else if(bitsize <= MAXBITSPERLONG)
      {
	m_wordLengthTotal = calcWordSize(getTotalBitSize());
	m_wordLengthItem = calcWordSize(bitsize);
	m_max = calcBitsizeSignedMaxLong(bitsize);
	m_min = calcBitsizeSignedMinLong(bitsize);
      }
    else
      m_state.abortGreaterThanMaxBitsPerLong();
  }

   ULAMTYPE UlamTypePrimitiveInt::getUlamTypeEnum()
   {
     return Int;
   }

  bool UlamTypePrimitiveInt::isNumericType()
  {
    return true;
  }

  bool UlamTypePrimitiveInt::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx(); //from type

    if(UlamTypePrimitive::safeCast(valtypidx) != CAST_CLEAR) //bad|hazy
      return false;

    u32 wordsize = getTotalWordSize();
    u32 valwordsize = m_state.getTotalWordSize(valtypidx);
    if(wordsize <= MAXBITSPERINT)
      {
	if(valwordsize <= MAXBITSPERINT)
	  brtn = castTo32(val, typidx);
	else if(valwordsize <= MAXBITSPERLONG)
	  brtn = castTo64(val, typidx); //downcast
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }
    else if(wordsize <= MAXBITSPERLONG)
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

  bool UlamTypePrimitiveInt::castTo32(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);

    u32 data = val.getImmediateData(m_state);
    u32 sdata = 0;
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Int:
	// casting Int to Int to change bits size
	sdata = _Int32ToInt32(data, valbitsize, bitsize);
	break;
      case Unsigned:
	// casting Unsigned to Int to change type
	sdata = _Unsigned32ToInt32(data, valbitsize, bitsize);
	break;
      case Bits:
	// casting Bits to Int to change type
	sdata = _Bits32ToInt32(data, valbitsize, bitsize);
	break;
      case Unary:
	sdata = _Unary32ToInt32(data, valbitsize, bitsize);
	break;
      case Bool:
	sdata = _Bool32ToInt32(data, valbitsize, bitsize);
	break;
      case Void:
      default:
	//std::cerr << "UlamTypePrimitiveInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      val = UlamValue::makeImmediate(typidx, sdata, m_state); //overwrite val
    return brtn;
  } //castTo32

  bool UlamTypePrimitiveInt::castTo64(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    u32 valwordsize = m_state.getTotalWordSize(valtypidx);
    u64 data;

    if(valwordsize <= MAXBITSPERINT)
      data = (u64) val.getImmediateData(m_state);
    else if(valwordsize <= MAXBITSPERLONG)
      data = val.getImmediateDataLong(m_state);
    else
      m_state.abortGreaterThanMaxBitsPerLong();

    u64 sdata = 0;
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Int:
	// casting Int to Int to change bits size
	sdata = _Int64ToInt64(data, valbitsize, bitsize);
	break;
      case Unsigned:
	// casting Unsigned to Int to change type
	sdata = _Unsigned64ToInt64(data, valbitsize, bitsize);
	break;
      case Bits:
	// casting Bits to Int to change type
	sdata = _Bits64ToInt64(data, valbitsize, bitsize);
	break;
      case Unary:
	sdata = _Unary64ToInt64(data, valbitsize, bitsize);
	break;
      case Bool:
	sdata = _Bool64ToInt64(data, valbitsize, bitsize);
	break;
      case Void:
      default:
	//std::cerr << "UlamTypePrimitiveInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      {
	u32 wordsize = getTotalWordSize(); //tobe
	if(wordsize <= MAXBITSPERINT) //downcast
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, m_state); //overwrite val
	else if(wordsize <= MAXBITSPERLONG)
	  val = UlamValue::makeImmediateLong(typidx, sdata, m_state); //overwrite val
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }
    return brtn;
  } //castTo64

  FORECAST UlamTypePrimitiveInt::safeCast(UTI typidx)
  {
    FORECAST scr = UlamTypePrimitive::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    bool aok = true;
    UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
    s32 fmbitsize = m_state.getBitSize(typidx);
    s32 tobitsize = getBitSize();

    ULAMTYPE fmtypEnum = fmut->getUlamTypeEnum();
    switch(fmtypEnum)
      {
      case Int:
	aok = (tobitsize >= fmbitsize);
	break;
      case Unsigned:
	aok = (tobitsize > fmbitsize);
	break;
      case Unary:
	aok = (tobitsize > (s32) _getLogBase2(fmbitsize) + 1);
	break;
      case Bool:
      case Bits:
      case Void:
      case UAtom:
      case String:
	aok = false;
	break;
      case Class:
	{
	  //must be Quark! treat as Int if it has a toInt method
	  if(fmut->isNumericType())
	    aok = (tobitsize >= MAXBITSPERINT);
	  else
	    aok = false; //t41131 called by matching args (no error msg please)
	}
	break;
      default:
	m_state.abortUndefinedUlamType();
	//std::cerr << "UlamTypePrimitiveInt (cast) error! Value Type was: " << valtypidx << std::endl;
	aok = false;
      };
    return aok ? CAST_CLEAR : CAST_BAD;
  } //safeCast

  void UlamTypePrimitiveInt::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    s32 sdata = getDataAsCs32(data);
    if(prefix == 'z')
      sprintf(valstr,"%d", sdata);
    else
      sprintf(valstr,"%c%d", prefix, sdata);
  }

  void UlamTypePrimitiveInt::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%s", ToSignedDecimal(data).c_str());
    else
      sprintf(valstr,"%c%s", prefix, ToSignedDecimal(data).c_str());
  }

  s32 UlamTypePrimitiveInt::getDataAsCs32(const u32 data)
  {
    return _Int32ToCs32(data, getBitSize());
  }

  u32 UlamTypePrimitiveInt::getDataAsCu32(const u32 data)
  {
    s32 bitsize = getBitSize();
    assert(bitsize > 0);
    u32 cdata = _Int32ToUnsigned32(data, (u32) bitsize, (u32) bitsize);
    return _Unsigned32ToCu32(cdata, (u32) bitsize);
  }

  s64 UlamTypePrimitiveInt::getDataAsCs64(const u64 data)
  {
    return _Int64ToCs64(data, getBitSize());
  }

  u64 UlamTypePrimitiveInt::getDataAsCu64(const u64 data)
  {
    s32 bitsize = getBitSize();
    assert(bitsize > 0);
    u64 cdata = _Int64ToUnsigned64(data, (u32) bitsize, (u32) bitsize);
    return _Unsigned64ToCu64(cdata, (u32) bitsize);
  }

  s32 UlamTypePrimitiveInt::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
  {
    s32 bitsize = getBitSize();
    s32 tobitsize = UNKNOWNSIZE;
    s32 wordsize = getTotalWordSize();
    switch(tobUT)
      {
      case Unsigned:
	tobitsize = bitsize; //unsafe
	break;
      case Unary:
	tobitsize = getMax();
	break;
      case Bool:
	tobitsize = 1;
	break;
      case Int:
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
	//std::cerr << "UlamTypePrimitiveInt (convertTo) error! " << tobUT << std::endl;
      };
    return (tobitsize > wordsize ? wordsize : tobitsize);
  } //bitsizeToConvertTypeTo

} //end MFM
