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
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
    // consider s64 later...
    m_max = calcBitsizeUnsignedMax(getBitSize());
    m_min = 0;
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


  const char * UlamTypeUnsigned::getUlamTypeAsSingleLowercaseLetter()
  {
    return "u";
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
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: " << valtypidx << "," << m_state.getArraySize(valtypidx);
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	return false;
      }

    //change the size first of tobe, if necessary
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);

    if(bitsize == UNKNOWNSIZE || valbitsize == UNKNOWNSIZE)
      {
	std::ostringstream msg;
	msg << "Casting UNKNOWN sizes; " << bitsize << ", Value Type and size was: " << valtypidx << "," << valbitsize;
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	return false;
      }

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    //ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    u32 data = val.getImmediateData(m_state);
    switch(valtypEnum)
      {
      case Int:
	{
	  // casting Int to Unsigned to change type
	  const s32 sdata = _SignExtend32(data,valbitsize);
	  data = _Int32ToUnsigned32(sdata, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
	}
	break;
      case Unsigned:
	{
	  // casting UnsignedInt to UnsignedInt to change bits size
	  data = _Unsigned32ToUnsigned32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
	}
	break;
      case Bits:
	// casting to Bits Unsigned to change type
	val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
	break;
      case Bool:
	{
	  data = _Bool32ToUnsigned32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
	}
	break;
      case Unary:
	{
	  data = _Unary32ToUnsigned32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, data, m_state); //overwrite val
	}
	break;
      case Void:
      default:
	//std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    return brtn;
  } //end cast


  void UlamTypeUnsigned::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", data);
    else
      sprintf(valstr,"%c%u", prefix, data);
  }

} //end MFM
