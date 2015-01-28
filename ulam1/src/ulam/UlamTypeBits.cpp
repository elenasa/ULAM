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
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
  }


   ULAMTYPE UlamTypeBits::getUlamTypeEnum()
   {
     return Bits;
   }


  const std::string UlamTypeBits::getUlamTypeVDAsStringForC()
  {
    return "VD::BITS";
  }


  const char * UlamTypeBits::getUlamTypeAsSingleLowercaseLetter()
  {
    return "t";
  }


  bool UlamTypeBits::isMinMaxAllowed()
  {
    return false;
  }


  bool UlamTypeBits::cast(UlamValue & val, UTI typidx)
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

    //no changes to data, only type
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    u32 data = val.getImmediateData(m_state);
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
  } //end cast


  void UlamTypeBits::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", data);
    else
      sprintf(valstr,"%c%u", prefix, data);
  }

} //end MFM
