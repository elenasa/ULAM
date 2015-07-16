#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeBits.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeBits::UlamTypeBits(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
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
	m_wordLengthTotal = calcWordSizeLong(getTotalBitSize());
	m_wordLengthItem = calcWordSizeLong(bitsize);
	m_max = calcBitsizeUnsignedMaxLong(bitsize);
	m_min = 0;
      }
    else
      assert(0);
  }

   ULAMTYPE UlamTypeBits::getUlamTypeEnum()
   {
     return Bits;
   }

  bool UlamTypeBits::isPrimitiveType()
  {
    return true;
  }

  const std::string UlamTypeBits::getUlamTypeVDAsStringForC()
  {
    return "VD::BITS";
  }

  bool UlamTypeBits::isMinMaxAllowed()
  {
    return false;
  }

  bool UlamTypeBits::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();

    if(UlamType::safeCast(valtypidx) != CAST_CLEAR) //bad|hazy
      return false;

    u32 wordsize = getTotalWordSize();
    u32 valwordsize = m_state.getTotalWordSize(valtypidx);
    if(wordsize == MAXBITSPERINT)
      {
	if(valwordsize == MAXBITSPERINT)
	  brtn = castTo32(val, typidx);
	else if(valwordsize == MAXBITSPERLONG)
	  brtn = castTo64(val, typidx); //downcast
	else
	  assert(0);
      }
    else if(wordsize == MAXBITSPERLONG)
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

  bool UlamTypeBits::castTo32(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    u32 data = val.getImmediateData(m_state);

    //no changes to data, only type; unless theres a wordsize difference
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Void:
      case Int:
      case Unsigned:
      case Bool:
      case Unary:
      case Bits:
	val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
	break;
      default:
	//std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    return brtn;
  } //castTo32

  bool UlamTypeBits::castTo64(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    u32 valwordsize = m_state.getTotalWordSize(valtypidx);
    u64 data;

    if(valwordsize == MAXBITSPERINT)
      data = (u64) val.getImmediateData(m_state);
    else if(valwordsize == MAXBITSPERLONG)
      data = val.getImmediateDataLong(m_state);
    else
      assert(0);

    //no changes to data, only type
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Void:
      case Int:
      case Unsigned:
      case Bool:
      case Unary:
      case Bits:
	val = UlamValue::makeImmediateLong(typidx, data, m_state); //overwrite val
	break;
      default:
	//std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn;
  } //castTo64

  FORECAST UlamTypeBits::safeCast(UTI typidx)
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
      case Unary:
      case Int:
      case Bool:
      case Bits:
	brtn = (bitsize >= valbitsize); //downhill
	break;
      case Void:
      case UAtom:
	brtn = false;
	break;
      case Class:
	{
	  //must be Quark! treat as Int if it has a toInt method
	  if(vut->isNumericType())
	    brtn = (bitsize >= MAXBITSPERINT);
	  else
	    {
	      std::ostringstream msg;
	      msg << "Class: ";
	      msg << m_state.getUlamTypeNameBriefByIndex(typidx).c_str();
	      msg << " is not a numeric type and cannot be safely cast to Bits";
	      MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(),msg.str().c_str(), ERR);
	      brtn = false;
	    }
	}
	break;
      default:
	assert(0);
	//std::cerr << "UlamTypeBits (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn ? CAST_CLEAR : CAST_BAD;
  } //safeCast

  void UlamTypeBits::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", data);
    else
      sprintf(valstr,"%c%u", prefix, data);
  }

  void UlamTypeBits::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%s", ToUnsignedDecimal(data).c_str());
    else
      sprintf(valstr,"%c%s", prefix, ToUnsignedDecimal(data).c_str());
  }

  s32 UlamTypeBits::getDataAsCs32(const u32 data)
  {
    return _Bits32ToCs32(data, getBitSize());
  }

  u32 UlamTypeBits::getDataAsCu32(const u32 data)
  {
    return _Bits32ToCu32(data, getBitSize());
  }

  s64 UlamTypeBits::getDataAsCs64(const u64 data)
  {
    return _Bits64ToCs64(data, getBitSize());
  }

  u64 UlamTypeBits::getDataAsCu64(const u64 data)
  {
    return _Bits64ToCu64(data, getBitSize());
  }

  s32 UlamTypeBits::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
  {
    s32 bitsize = getBitSize();
    s32 tobitsize = UNKNOWNSIZE;
    s32 wordsize = getTotalWordSize();
    switch(tobUT)
      {
      case Bool:
      case Unsigned:
      case Unary:
      case Int:
      case Bits:
	tobitsize = bitsize;
	break;
      case Void:
	tobitsize = 0;
	break;
      case UAtom:
      case Class:
	break;
      default:
	assert(0);
	//std::cerr << "UlamTypeBits (convertTo) error! " << tobUT << std::endl;
      };
    return (tobitsize > wordsize ? wordsize : tobitsize);
  } //bitsizeToConvertTypeTo

} //end MFM
