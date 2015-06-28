#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeUnsigned.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeUnsigned::UlamTypeUnsigned(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
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

   ULAMTYPE UlamTypeUnsigned::getUlamTypeEnum()
   {
     return Unsigned;
   }

  const std::string UlamTypeUnsigned::getUlamTypeVDAsStringForC()
  {
    return "VD::U32";
  }

  const std::string UlamTypeUnsigned::getUlamTypeImmediateMangledName()
  {
    if(needsImmediateType())
      return UlamType::getUlamTypeImmediateMangledName();

    //return getImmediateStorageTypeAsString(state); //"u32"; inf loop
    return UlamType::getUlamTypeImmediateMangledName(); //? for constants
  }

  bool UlamTypeUnsigned::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    //UTI typidx = getUlamTypeIndex();
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();
    s32 arraysize = getArraySize();
    if(arraysize != m_state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize;
	msg << ", Value Type and size was: ";
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
    if(wordsize == MAXBITSPERINT) //tobe
      {
	if(valwordsize == MAXBITSPERINT)
	  brtn = castTo32(val, typidx);
	else if(valwordsize == MAXBITSPERLONG)
	  brtn = castTo64(val, typidx); //downcast
	else
	  assert(0);
      }
    else if(wordsize == MAXBITSPERLONG) //tobe
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

  bool UlamTypeUnsigned::castTo32(UlamValue & val, UTI typidx)
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
      case Void:
      default:
	//std::cerr << "UlamTypeUnsigned (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
    return brtn;
  } //castTo32

  bool UlamTypeUnsigned::castTo64(UlamValue & val, UTI typidx)
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
      case Void:
      default:
	//std::cerr << "UlamTypeUnsigned (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      {
	u32 wordsize = getTotalWordSize(); //tobe
	if(wordsize == MAXBITSPERINT) //downcast
	  val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
	else if(wordsize == MAXBITSPERLONG)
	  val = UlamValue::makeImmediateLong(typidx, data, m_state); //overwrite val
	else
	  assert(0);
      }
    return brtn;
  } //castTo64

  CASTSTAT UlamTypeUnsigned::safeCast(UTI typidx)
  {
    CASTSTAT scr = UlamType::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(typidx);

    bool brtn = true;
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(typidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Unsigned:
	brtn = (bitsize >= valbitsize);
	break;
      case Unary:
	brtn = ((bitsize >= valbitsize - 1) && (bitsize > 0));
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
	//std::cerr << "UlamTypeUnsigned (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn ? CAST_CLEAR : CAST_BAD;
  } //safeCast

  void UlamTypeUnsigned::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", data);
    else
      sprintf(valstr,"%c%u", prefix, data);
  }

  void UlamTypeUnsigned::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%lu", data);
    else
      sprintf(valstr,"%c%lu", prefix, data);
  }

} //end MFM
