#include <stdio.h>
#include <string.h>
#include <iostream>
#include "UlamTypePrimitiveBool.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypePrimitiveBool::UlamTypePrimitiveBool(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypePrimitive(key, state)
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

   ULAMTYPE UlamTypePrimitiveBool::getUlamTypeEnum()
   {
     return Bool;
   }

  bool UlamTypePrimitiveBool::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx(); //from typeg

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

  bool UlamTypePrimitiveBool::castTo32(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    u32 data = val.getImmediateData(m_state);

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    u32 newdata = 0;
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Int:
	newdata = _Int32ToBool32(data, valbitsize, bitsize);
	break;
      case Unsigned:
	newdata = _Unsigned32ToBool32(data, valbitsize, bitsize);
	break;
      case Unary:
	newdata = _Unary32ToBool32(data, valbitsize, bitsize); //all ones if true
	break;
      case Bool:
	newdata = _Bool32ToBool32(data, valbitsize, bitsize); //all ones if true
	break;
      case Bits:
	newdata = _Bits32ToBool32(data, valbitsize, bitsize);  //no change to Bits data
	break;
      case Void:
      default:
	//std::cerr << "UlamTypePrimitiveBool (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      val = UlamValue::makeImmediate(typidx, newdata, m_state); //overwrite val
    return brtn;
  } //castTo32

  bool UlamTypePrimitiveBool::castTo64(UlamValue & val, UTI typidx)
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

    u64 newdata = 0;
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    switch(valtypEnum)
      {
      case Int:
	newdata = _Int64ToBool64(data, valbitsize, bitsize);
	break;
      case Unsigned:
	newdata = _Unsigned64ToBool64(data, valbitsize, bitsize);
	break;
      case Unary:
	newdata = _Unary64ToBool64(data, valbitsize, bitsize); //all ones if true
	break;
      case Bool:
	newdata = _Bool64ToBool64(data, valbitsize, bitsize); //all ones if true
	break;
      case Bits:
	newdata = _Bits64ToBool64(data, valbitsize, bitsize);  //no change to Bits data
	break;
      case Void:
      default:
	//std::cerr << "UlamTypePrimitiveBool (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      {
	u32 wordsize = getTotalWordSize(); //tobe
	if(wordsize <= MAXBITSPERINT) //downcast
	  val = UlamValue::makeImmediate(typidx, newdata, m_state); //overwrite val
	else if(wordsize <= MAXBITSPERLONG)
	  val = UlamValue::makeImmediateLong(typidx, newdata, m_state); //overwrite val
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }
    return brtn;
  } //castTo64

  FORECAST UlamTypePrimitiveBool::safeCast(UTI typidx)
  {
    FORECAST scr = UlamTypePrimitive::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    bool brtn = true;
    UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
    s32 valbitsize = fmut->getBitSize();
    ULAMTYPE valtypEnum = fmut->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Bool:
	brtn = true; //any size ok
	break;
      case Unsigned:
      case Unary:
	brtn = (valbitsize == 1);
	break;
      case Int:
      case Bits:
      case Void:
      case UAtom:
      case Class:
      case String:
	brtn = false;
	break;
      default:
	m_state.abortUndefinedUlamType();
	//std::cerr << "UlamTypePrimitiveBool (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn ? CAST_CLEAR : CAST_BAD;
  } //safeCast

  FORECAST UlamTypePrimitiveBool::explicitlyCastable(UTI typidx)
  {
    ULAMTYPE vtypEnum = m_state.getUlamTypeByIndex(typidx)->getUlamTypeEnum();
    FORECAST scr = safeCast(typidx);
    if(((vtypEnum == Bits) && (UlamTypePrimitive::explicitlyCastable(typidx) == CAST_CLEAR)) || (scr == CAST_CLEAR))
      return CAST_CLEAR;
    return scr; //HAZY or UNSAFE
  } //explicitlyCastable

  void UlamTypePrimitiveBool::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(!isComplete())
      sprintf(valstr,"%s", "unknown");
    else
      {
	bool dataAsBool = _Bool32ToCbool(data, getBitSize());

	if(prefix == 'z')
	  sprintf(valstr,"%s", dataAsBool ? "true" : "false");
	else
	  sprintf(valstr,"%c%s", prefix, dataAsBool ? "true" : "false");
      }
  } //getDataAsString

  void UlamTypePrimitiveBool::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(!isComplete())
      sprintf(valstr,"%s", "unknown");
    else
      {
	bool dataAsBool = _Bool64ToCbool(data, getBitSize());

	if(prefix == 'z')
	  sprintf(valstr,"%s", dataAsBool ? "true" : "false");
	else
	  sprintf(valstr,"%c%s", prefix, dataAsBool ? "true" : "false");
      }
  } //getDataLongAsString

  s32 UlamTypePrimitiveBool::getDataAsCs32(const u32 data)
  {
    return _Bool32ToCs32(data, getBitSize());
  }

  u32 UlamTypePrimitiveBool::getDataAsCu32(const u32 data)
  {
    return _Bool32ToCu32(data, getBitSize());
  }

  s64 UlamTypePrimitiveBool::getDataAsCs64(const u64 data)
  {
    return _Bool64ToCs64(data, getBitSize());
  }

  u64 UlamTypePrimitiveBool::getDataAsCu64(const u64 data)
  {
    return _Bool64ToCu64(data, getBitSize());
  }

  const std::string UlamTypePrimitiveBool::getConvertToCboolMethod()
  {
    std::ostringstream rtnMethod;
    rtnMethod << "_Bool" << getTotalWordSize() << "ToCbool";
    return rtnMethod.str();
  } //getCovertToCBoolMethod

  s32 UlamTypePrimitiveBool::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
  {
    s32 bitsize = getBitSize();
    s32 tobitsize = UNKNOWNSIZE;
    s32 wordsize = getTotalWordSize();
    switch(tobUT)
      {
      case Bool:
      case Unsigned:
      case Unary:
	tobitsize = 1;
	break;
      case Int:
	tobitsize = 2;
	break;
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
	//std::cerr << "UlamTypeInt (convertTo) error! " << tobUT << std::endl;
      };
    return (tobitsize > wordsize ? wordsize : tobitsize);
  } //bitsizeToConvertTypeTo

} //end MFM
