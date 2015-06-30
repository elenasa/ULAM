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
    s32 arraysize = getArraySize();
    if(arraysize != m_state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: ";
	msg << valtypidx << "," << m_state.getArraySize(valtypidx);
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	return false;
      }

    //change the size first of tobe, if necessary
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);

    if(bitsize == UNKNOWNSIZE || valbitsize == UNKNOWNSIZE)
      {
	std::ostringstream msg;
	msg << "Casting UNKNOWN sizes; " << bitsize << ", Value Type and size was: ";
	msg << valtypidx << "," << valbitsize;
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	return false;
      }

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
	msg << valtypidx << "," << valbitsize;
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

    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(typidx);

    bool brtn = true;
    UlamType * vut = m_state.getUlamTypeByIndex(typidx);
    ULAMTYPE valtypEnum = vut->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Unsigned:
      case Unary:
      case Int:
      case Bool:
      case Bits:
	brtn = (bitsize >= valbitsize); //anything to void ok
	break;
      case Void:
      case UAtom:
	brtn = false;
	break;
      case Class:
	brtn = (vut->getUlamClass() == UC_QUARK && bitsize >= MAXBITSPERINT); //as Int
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
      sprintf(valstr,"%lu", data);
    else
      sprintf(valstr,"%c%lu", prefix, data);
  }

} //end MFM
