#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeBits.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeBits::UlamTypeBits(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypeBits::getUlamTypeEnum()
   {
     return Bits;
   }


  const std::string UlamTypeBits::getUlamTypeAsStringForC()
  {
    return "unsigned int";
  }


  const char * UlamTypeBits::getUlamTypeAsSingleLowercaseLetter()
  {
    return "t";
  }


  bool UlamTypeBits::cast(UlamValue & val, CompilerState& state)
  {
    bool brtn = true;
    UTI typidx = getUlamTypeIndex();
    UTI valtypidx = val.getUlamValueTypeIdx();    
    s32 arraysize = getArraySize();
    if(arraysize != state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: " << valtypidx << "," << state.getArraySize(valtypidx);
	state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_ERR);
	return false;
      }
    
    //no changes to data, only type
    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    u32 data = val.getImmediateData(state);
    switch(valtypEnum)
      {
      case Bool:
      case Int:
      case Unsigned:
      case Bits:
	val = UlamValue::makeImmediate(typidx, data, state); //overwrite val
	break;
      default:
	assert(0);
	//std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    return brtn;
  } //end cast


  void UlamTypeBits::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%x", data);
    else
      sprintf(valstr,"%c%x", prefix, data);
  }
  
} //end MFM
