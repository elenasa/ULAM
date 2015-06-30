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
	msg << "Casting different Array sizes; " << arraysize;
	msg << ", Value Type and size was: ";
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

  FORECAST UlamTypeVoid::safeCast(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    bool brtn = true;
    UlamType * vut = m_state.getUlamTypeByIndex(typidx);
    ULAMTYPE valtypEnum = vut->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Void:
      case Unsigned:
      case Unary:
      case Int:
      case Bool:
      case Bits:
      case UAtom:
      case Class:
	brtn = true; //anything casts to void ok
	break;
      default:
	assert(0);
	//std::cerr << "UlamTypeVoid (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn ? CAST_CLEAR : CAST_BAD;
  } //safeCast

} //end MFM
