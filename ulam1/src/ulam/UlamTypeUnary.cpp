#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeUnary.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeUnary::UlamTypeUnary(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
    m_max = (getBitSize() <= 0 ? 0 : _GetNOnes32((u32) getBitSize()));
    m_min = 0;
  }

   ULAMTYPE UlamTypeUnary::getUlamTypeEnum()
   {
     return Unary;
   }

  const std::string UlamTypeUnary::getUlamTypeVDAsStringForC()
  {
    return "VD::UNARY";
  }

  const char * UlamTypeUnary::getUlamTypeAsSingleLowercaseLetter()
  {
    return "y";
  }

  bool UlamTypeUnary::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
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
	{
	  s32 sdata = _SignExtend32(data, valbitsize);
	  // cast from Int->Unary, OR Bool->Unary (same as Bool->Int)
	  data = _Int32ToUnary32(sdata, valbitsize, bitsize);
	}
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
	{
	  s64 sdata = _SignExtend64(data, valbitsize);
	  // cast from Int->Unary, OR Bool->Unary (same as Bool->Int)
	  data = _Int64ToUnary64(sdata, valbitsize, bitsize);
	}
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
	if(wordsize == MAXBITSPERINT) //downcast
	  val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
	else if(wordsize == MAXBITSPERLONG)
	  val = UlamValue::makeImmediateLong(typidx, data, m_state); //overwrite val
	else
	  assert(0);
      }
    return brtn;
  } //castTo64

  void UlamTypeUnary::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", PopCount(data)); //converted to binary
    else
      sprintf(valstr,"%c%u", prefix, PopCount(data)); //converted to binary
  }

  void UlamTypeUnary::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", PopCount(data)); //converted to binary
    else
      sprintf(valstr,"%c%u", prefix, PopCount(data)); //converted to binary
  }

} //end MFM
