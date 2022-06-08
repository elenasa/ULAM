#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypePrimitiveUnary.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypePrimitiveUnary::UlamTypePrimitiveUnary(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypePrimitive(key, state)
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
	m_max = _GetNOnes32((u32) bitsize);
	m_min = 0;
      }
    else if(bitsize <= MAXBITSPERLONG)
      {
	m_wordLengthTotal = calcWordSize(getTotalBitSize());
	m_wordLengthItem = calcWordSize(bitsize);
	m_max = _GetNOnes64((u64) bitsize);
	m_min = 0;
      }
    else
      m_state.abortGreaterThanMaxBitsPerLong();
  }

   ULAMTYPE UlamTypePrimitiveUnary::getUlamTypeEnum()
   {
     return Unary;
   }

  bool UlamTypePrimitiveUnary::isNumericType()
  {
    return true;
  }

  bool UlamTypePrimitiveUnary::cast(UlamValue & val, UTI typidx)
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

  bool UlamTypePrimitiveUnary::castTo32(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    u32 data = val.getImmediateData(m_state);

    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Int:
	// cast from Int->Unary, OR Bool->Unary (same as Bool->Int)
	data = _Int32ToUnary32(data, valbitsize, bitsize);
	break;
      case Unsigned:
	data = _Unsigned32ToUnary32(data, valbitsize, bitsize);
	break;
      case Bool:
	// Bool -> Unary is the same as Bool -> Int
	data = _Bool32ToUnary32(data, valbitsize, bitsize);
	break;
      case Unary:
	data = _Unary32ToUnary32(data, valbitsize, bitsize);
	break;
      case Bits:
	break;
      case Void:
      default:
	//std::cerr << "UlamTypePrimitiveUnary (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    if(brtn)
      val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val, same data
    return brtn;
  } //castTo32

  bool UlamTypePrimitiveUnary::castTo64(UlamValue & val, UTI typidx)
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

    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Int:
	// cast from Int->Unary, OR Bool->Unary (same as Bool->Int)
	data = _Int64ToUnary64(data, valbitsize, bitsize);
	break;
      case Unsigned:
	data = _Unsigned64ToUnary64(data, valbitsize, bitsize);
	break;
      case Bool:
	// Bool -> Unary is the same as Bool -> Int
	data = _Bool64ToUnary64(data, valbitsize, bitsize);
	break;
      case Unary:
	data = _Unary64ToUnary64(data, valbitsize, bitsize);
	break;
      case Bits:
	break;
      case Void:
      default:
	//std::cerr << "UlamTypePrimitiveUnary (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      {
	u32 wordsize = getTotalWordSize(); //tobe
	if(wordsize <= MAXBITSPERINT) //downcast
	  val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
	else if(wordsize <= MAXBITSPERLONG)
	  val = UlamValue::makeImmediateLong(typidx, data, m_state); //overwrite val
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }
    return brtn;
  } //castTo64

  FORECAST UlamTypePrimitiveUnary::safeCast(UTI typidx)
  {
    FORECAST scr = UlamTypePrimitive::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    bool brtn = true;
    UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
    s32 valbitsize = fmut->getBitSize();
    s32 bitsize = getBitSize();
    ULAMTYPE valtypEnum = fmut->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Unsigned:
	{
	  u32 vwordsize = fmut->getTotalWordSize();
	  if(vwordsize <= MAXBITSPERINT)
	    brtn = ((u32) bitsize >= (u32) fmut->getMax());
	  else
	    brtn = ((u64) bitsize >= fmut->getMax());
	}
	break;
      case Unary:
	brtn = (bitsize >= valbitsize);
	break;
      case Int:
      case Bool:
      case Bits:
      case Void:
      case UAtom:
      case Class:
      case String:
	brtn = false;
	break;
      default:
	m_state.abortUndefinedUlamType();
	//std::cerr << "UlamTypePrimitiveUnary (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn ? CAST_CLEAR : CAST_BAD;
  } //safeCast

  void UlamTypePrimitiveUnary::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", getDataAsCu32(data)); //converted to binary
    else
      sprintf(valstr,"%c%u", prefix, getDataAsCu32(data)); //converted to binary
  }

  void UlamTypePrimitiveUnary::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%s", ToUnsignedDecimal(getDataAsCu64(data)).c_str()); //converted to binary
    else
      sprintf(valstr,"%c%s", prefix, ToUnsignedDecimal(getDataAsCu64(data)).c_str()); //converted to binary
  }

  s32 UlamTypePrimitiveUnary::getDataAsCs32(const u32 data)
  {
    return _Unary32ToCs32(data, getBitSize());
  }

  u32 UlamTypePrimitiveUnary::getDataAsCu32(const u32 data)
  {
    return _Unary32ToCu32(data, getBitSize());
  }

  s64 UlamTypePrimitiveUnary::getDataAsCs64(const u64 data)
  {
    return _Unary64ToCs64(data, getBitSize());
  }

  u64 UlamTypePrimitiveUnary::getDataAsCu64(const u64 data)
  {
    return _Unary64ToCu64(data, getBitSize());
  }

  s32 UlamTypePrimitiveUnary::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
  {
    s32 wordsize = getTotalWordSize();
    s32 bitsize = getBitSize();
    s32 tobitsize = UNKNOWNSIZE;
    switch(tobUT)
      {
      case Unsigned:
	tobitsize = (s32) _getLogBase2(bitsize) + 1; //fits into unsigned
	break;
      case Int:
	tobitsize = (s32) _getLogBase2(bitsize) + 1 + 1;
	break;
      case Bool:
	tobitsize = 1;
	break;
      case Unary:
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
	//std::cerr << "UlamTypePrimitiveUnary convertTypeTo error! " << tobUT << std::endl;
      };
    return (tobitsize > wordsize ? wordsize : tobitsize);
  } //bitsizeToconvertTypeTo

} //end MFM
