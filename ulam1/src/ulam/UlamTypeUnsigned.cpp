#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeUnsigned.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeUnsigned::UlamTypeUnsigned(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
  }


   ULAMTYPE UlamTypeUnsigned::getUlamTypeEnum()
   {
     return Unsigned;
   }


  const std::string UlamTypeUnsigned::getUlamTypeVDAsStringForC()
  {
    return "VD::U32";
  }


  const std::string UlamTypeUnsigned::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    if(needsImmediateType())
      return UlamType::getUlamTypeImmediateMangledName(state);

    //return getImmediateStorageTypeAsString(state); //"u32"; inf loop
    return UlamType::getUlamTypeImmediateMangledName(state); //? for constants
  }


  const char * UlamTypeUnsigned::getUlamTypeAsSingleLowercaseLetter()
  {
    return "u";
  }


  bool UlamTypeUnsigned::cast(UlamValue & val, CompilerState& state)
  {
    bool brtn = true;
    UTI typidx = getUlamTypeIndex();
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

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    //ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    u32 data = val.getImmediateData(state);
    switch(valtypEnum)
      {
      case Int:
	{
	  // casting Int to Unsigned to change type
	  const s32 sdata = _SignExtend32(data,valbitsize);
	  data = _Int32ToUnsigned32(sdata, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, state); //overwrite val
	}
	break;
      case Unsigned:
	{
	  // casting UnsignedInt to UnsignedInt to change bits size
	  data = _Unsigned32ToUnsigned32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, state); //overwrite val
	}
	break;
      case Bits:
	// casting to Bits Unsigned to change type
	val = UlamValue::makeImmediate(typidx, data, state); //overwrite val
	break;
      case Bool:
	{
	  data = _Bool32ToUnsigned32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, state); //overwrite val
	}
	break;
      case Unary:
	{
	  data = _Unary32ToUnsigned32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, state); //overwrite val
	}
	break;
      case Void:
      default:
	//std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    return brtn;
  } //end cast


  void UlamTypeUnsigned::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", data);
    else
      sprintf(valstr,"%c%u", prefix, data);
  }

} //end MFM
