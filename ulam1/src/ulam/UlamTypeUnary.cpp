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
    m_max = getBitSize();
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
    return "u";
  }


  bool UlamTypeUnary::cast(UlamValue & val, UTI typidx, CompilerState& state)
  {
    bool brtn = true;
    //    UTI typidx = getUlamTypeIndex();
    assert(state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();

    s32 arraysize = getArraySize();
    if(arraysize != state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: " << valtypidx << "," << state.getArraySize(valtypidx);
	MSG3(state.getFullLocationAsString(state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	return false;
      }

    //change the size first of tobe, if necessary
    s32 bitsize = getBitSize();
    s32 valbitsize = state.getBitSize(valtypidx);

    if(bitsize == UNKNOWNSIZE || valbitsize == UNKNOWNSIZE)
      {
	std::ostringstream msg;
	msg << "Casting UNKNOWN sizes; " << bitsize << ", Value Type and size was: " << valtypidx << "," << valbitsize;
	MSG3(state.getFullLocationAsString(state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	return false;
      }

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    //ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    u32 data = val.getImmediateData(state);
    switch(valtypEnum)
      {
      case Int:
	{
	  s32 sdata = _SignExtend32(data, valbitsize);
	  // cast from Int->Unary, OR Bool->Unary (same as Bool->Int)
	  sdata = _Int32ToUnary32(sdata, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, state); //overwrite val
	}
	break;
      case Unsigned:
	{
	  data = _Unsigned32ToUnary32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, state); //overwrite val
	}
	break;
      case Bool:
	{
	  // Bool -> Unary is the same as Bool -> Int
	  data = _Bool32ToUnary32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, state); //overwrite val
	}
	break;
      case Unary:
	{
	  data = _Unary32ToUnary32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, state); //overwrite val, same data
	}
	break;
      case Bits:
	val = UlamValue::makeImmediate(typidx, data, state); //overwrite val, same data
	break;
      case Void:
      default:
	//std::cerr << "UlamTypeUnary (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    return brtn;
  } //end cast


  void UlamTypeUnary::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", PopCount(data));            //converted to binary
    else
      sprintf(valstr,"%c%u", prefix, PopCount(data));  //converted to binary
  }

} //end MFM
