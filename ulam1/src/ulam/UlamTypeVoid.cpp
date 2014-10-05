#include <iostream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeVoid.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeVoid::UlamTypeVoid(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypeVoid::getUlamTypeEnum()
   {
     return Void;
   }

  //anything can be cast to a void
  bool UlamTypeVoid::cast(UlamValue & val, CompilerState& state)
  {
    bool brtn = true;    
    UTI valtypidx = val.getUlamValueTypeIdx();    
    s32 arraysize = getArraySize();
    if(arraysize != state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: " << valtypidx << "," << state.getArraySize(valtypidx);
	state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_ERR);
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
	val = UlamValue::makeImmediate(getUlamTypeIndex(), 0, state); //overwrite val, no data
	break;
      default:
	//std::cerr << "UlamTypeVoid (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    return brtn;
  } //end cast    


  const std::string UlamTypeVoid::getUlamTypeAsStringForC()
  {
    return "void";
  }


  const char * UlamTypeVoid::getUlamTypeAsSingleLowercaseLetter()
  {
    return "v";
  }


  const std::string UlamTypeVoid::getUlamTypeMangledName(CompilerState * state)
  {
    return "void";
  }


  const std::string UlamTypeVoid::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    return "void";
  }


  bool UlamTypeVoid::needsImmediateType()
  {
    return false;
  }

} //end MFM
