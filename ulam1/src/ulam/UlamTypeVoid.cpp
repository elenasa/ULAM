#include <iostream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeVoid.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeVoid::UlamTypeVoid(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {}

   ULAMTYPE UlamTypeVoid::getUlamTypeEnum()
   {
     return Void;
   }

  const std::string UlamTypeVoid::getUlamTypeAsStringForC()
  {
    return "void";
  }

  const std::string UlamTypeVoid::getUlamTypeMangledName()
  {
    return "void";
  }

  const std::string UlamTypeVoid::getUlamTypeImmediateMangledName()
  {
    return getImmediateStorageTypeAsString(); //"void";
  }

  bool UlamTypeVoid::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypeVoid::getImmediateStorageTypeAsString()
  {
    return "void";
  }

  const std::string UlamTypeVoid::getTmpStorageTypeAsString()
  {
    return "void";
  }

  bool UlamTypeVoid::isMinMaxAllowed()
  {
    return false;
  }

  //anything can be cast to a void (not the reverse)
  bool UlamTypeVoid::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
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

    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Void:
      case Int:
      case Unsigned:
      case Unary:
      case Bool:
      case Bits:
	val = UlamValue::makeImmediate(typidx, 0, m_state); //overwrite val, no data
	break;
      default:
	//std::cerr << "UlamTypeVoid (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    return brtn;
  } //end cast

} //end MFM
