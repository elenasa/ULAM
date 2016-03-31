#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeUnary.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeUnary::UlamTypeUnary(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypePrimitive(key, state)
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
	m_wordLengthTotal = calcWordSizeLong(getTotalBitSize());
	m_wordLengthItem = calcWordSizeLong(bitsize);
	m_max = _GetNOnes64((u64) bitsize);
	m_min = 0;
      }
    else
      assert(0);
  }

   ULAMTYPE UlamTypeUnary::getUlamTypeEnum()
   {
     return Unary;
   }

  bool UlamTypeUnary::isNumericType()
  {
    return true;
  }

  bool UlamTypeUnary::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();

    if(UlamType::safeCast(valtypidx) != CAST_CLEAR) //bad|hazy
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
	  assert(0);
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

  bool UlamTypeUnary::castTo32(UlamValue & val, UTI typidx)
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
	//std::cerr << "UlamTypeUnary (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    if(brtn)
      val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val, same data
    return brtn;
  } //castTo32

  bool UlamTypeUnary::castTo64(UlamValue & val, UTI typidx)
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
      assert(0);

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
	//std::cerr << "UlamTypeUnary (cast) error! Value Type was: " << valtypidx << std::endl;
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
	  assert(0);
      }
    return brtn;
  } //castTo64

  FORECAST UlamTypeUnary::safeCast(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    bool brtn = true;
    UlamType * vut = m_state.getUlamTypeByIndex(typidx);
    s32 valbitsize = vut->getBitSize();
    s32 bitsize = getBitSize();
    ULAMTYPE valtypEnum = vut->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Unsigned:
	{
	  u32 vwordsize = vut->getTotalWordSize();
	  if(vwordsize <= MAXBITSPERINT)
	    brtn = ((u32) bitsize >= (u32) vut->getMax());
	  else
	    brtn = ((u64) bitsize >= vut->getMax());
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
	brtn = false;
	break;
      default:
	assert(0);
	//std::cerr << "UlamTypeUnary (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn ? CAST_CLEAR : CAST_BAD;
  } //safeCast

  void UlamTypeUnary::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", getDataAsCu32(data)); //converted to binary
    else
      sprintf(valstr,"%c%u", prefix, getDataAsCu32(data)); //converted to binary
  }

  void UlamTypeUnary::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%s", ToUnsignedDecimal(getDataAsCu64(data)).c_str()); //converted to binary
    else
      sprintf(valstr,"%c%s", prefix, ToUnsignedDecimal(getDataAsCu64(data)).c_str()); //converted to binary
  }

  s32 UlamTypeUnary::getDataAsCs32(const u32 data)
  {
    return _Unary32ToCs32(data, getBitSize());
  }

  u32 UlamTypeUnary::getDataAsCu32(const u32 data)
  {
    return _Unary32ToCu32(data, getBitSize());
  }

  s64 UlamTypeUnary::getDataAsCs64(const u64 data)
  {
    return _Unary64ToCs64(data, getBitSize());
  }

  u64 UlamTypeUnary::getDataAsCu64(const u64 data)
  {
    return _Unary64ToCu64(data, getBitSize());
  }

  s32 UlamTypeUnary::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
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
	assert(0);
	//std::cerr << "UlamTypeUnary convertTypeTo error! " << tobUT << std::endl;
      };
    return (tobitsize > wordsize ? wordsize : tobitsize);
  } //bitsizeToconvertTypeTo

} //end MFM
