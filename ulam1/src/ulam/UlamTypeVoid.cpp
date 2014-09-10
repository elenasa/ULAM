#include <iostream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeVoid.h"
#include "UlamValue.h"

namespace MFM {

  UlamTypeVoid::UlamTypeVoid(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypeVoid::getUlamTypeEnum()
   {
     return Void;
   }


  const std::string UlamTypeVoid::getUlamTypeAsStringForC()
  {
    return "void";
  }


  const char * UlamTypeVoid::getUlamTypeAsSingleLowercaseLetter()
  {
    return "v";
  }


  //anything can be cast to a void  ???
  bool UlamTypeVoid::cast(UlamValue & val)
    {
      UTI valtypidx = val.getUlamValueTypeIdx();
      bool brtn = true;
#if 0
      u32 valarraysize = m_state.getArraySize(valtypidx);
      u32 myarraysize = getArraySize();

      if(valarraysize == 0 && myarraysize == 0)
	{
	  switch(valtypidx)
	    {
	    case Void:
	    case Int:
	    case Bool:
	      val.init(this, 0);
	      break;
	    default:
	      //std::cerr << "UlamTypeVoid (cast) error! Value Type was: " << valtypidx << std::endl;
	      brtn = false;
	    };
	}
      else
	{
	  assert(0);
	  brtn=false;
	}
#endif
      return brtn;
    }


  void UlamTypeVoid::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    assert(m_key.m_arraySize == 0);      
    sprintf(valstr,"0");
  }

} //end MFM
