#include <iostream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeVoid.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeVoid::UlamTypeVoid(const UlamKeyTypeSignature key) : UlamType(key)
  {}


   ULAMTYPE UlamTypeVoid::getUlamTypeEnum()
   {
     return Void;
   }


  const std::string UlamTypeVoid::getUlamTypeAsStringForC()
  {
    return "void";
  }


  const std::string UlamTypeVoid::getUlamTypeMangledName(CompilerState * state)
  {
    return "void";
  }


  const std::string UlamTypeVoid::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    return getImmediateStorageTypeAsString(state); //"void";
  }


  bool UlamTypeVoid::needsImmediateType()
  {
    return false;
  }


  const std::string UlamTypeVoid::getImmediateStorageTypeAsString(CompilerState * state)
  {
    return "void";
  }


  const std::string UlamTypeVoid::getTmpStorageTypeAsString(CompilerState * state)
  {
    return "void";
  }


  const char * UlamTypeVoid::getUlamTypeAsSingleLowercaseLetter()
  {
    return "v";
  }

  bool UlamTypeVoid::isMinMaxAllowed()
  {
    return false;
  }


  //anything can be cast to a void (not the reverse)
  bool UlamTypeVoid::cast(UlamValue & val, UTI typidx, CompilerState& state)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    s32 arraysize = getArraySize();
    if(arraysize != state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: " << valtypidx << "," << state.getArraySize(valtypidx);
	MSG3(state.getFullLocationAsString(state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	return false;
      }

    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    switch(valtypEnum)
      {
      case Void:
      case Int:
      case Unsigned:
      case Unary:
      case Bool:
      case Bits:
	val = UlamValue::makeImmediate(typidx, 0, state); //overwrite val, no data
	break;
      default:
	//std::cerr << "UlamTypeVoid (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    return brtn;
  } //end cast

} //end MFM
