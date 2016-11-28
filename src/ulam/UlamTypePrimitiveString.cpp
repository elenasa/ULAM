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

  bool UlamTypePrimitiveString::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();

    if(UlamType::safeCast(valtypidx) != CAST_CLEAR) //bad|hazy
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

    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Int:
	// casting Int to Unsigned to change type
	data = _Int32ToUnsigned32(data, valbitsize, bitsize);
	break;
      case Unsigned:
	// casting UnsignedInt to UnsignedInt to change bits size
	data = _Unsigned32ToUnsigned32(data, valbitsize, bitsize);
	break;
      case Bits:
	// casting to Bits Unsigned to change type
	break;
      case Bool:
	  data = _Bool32ToUnsigned32(data, valbitsize, bitsize);
	break;
      case Unary:
	  data = _Unary32ToUnsigned32(data, valbitsize, bitsize);
	break;
      case String:
	break; //data is index, no cast req'd (t3951)
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
	// casting Int to Unsigned to change type
	data = _Int64ToUnsigned64(data, valbitsize, bitsize);
	break;
      case Unsigned:
	// casting UnsignedInt to UnsignedInt to change bits size
	data = _Unsigned64ToUnsigned64(data, valbitsize, bitsize);
	break;
      case Bits:
	// casting to Bits Unsigned to change type
	break;
      case Bool:
	data = _Bool64ToUnsigned64(data, valbitsize, bitsize);
	break;
      case Unary:
	data = _Unary64ToUnsigned64(data, valbitsize, bitsize);
	break;
      case String:
	// casting UnsignedInt to UnsignedInt to change bits size, test?
	data = _Unsigned64ToUnsigned64(data, valbitsize, bitsize);
	break;
      case Void:
      default:
	//std::cerr << "UlamTypePrimitiveString (cast) error! Value Type was: " << valtypidx << std::endl;
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

  FORECAST UlamTypePrimitiveString::safeCast(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    bool brtn = true;
    UlamType * vut = m_state.getUlamTypeByIndex(typidx);
    ULAMTYPE valtypEnum = vut->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case String:
	break;
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
    if(prefix == 'z')
      sprintf(valstr,"%s", m_state.m_upool.getDataAsFormattedString(data, &m_state).c_str());
    else
      sprintf(valstr,"%c%s", prefix, m_state.m_upool.getDataAsFormattedString(data, &m_state).c_str());
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

} //end MFM
