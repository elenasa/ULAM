#include <iostream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeVoid.h"
#include "UlamValue.h"

namespace MFM {

  UlamTypeVoid::UlamTypeVoid(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


  void UlamTypeVoid::newValue(UlamValue & val)
  {
    assert((val.isArraySize() == m_key.m_arraySize) && (m_key.m_arraySize == 0));
    val.m_valInt = 0;  //init to zero
  }


  void UlamTypeVoid::deleteValue(UlamValue * val){}


   ULAMTYPE UlamTypeVoid::getUlamTypeEnum()
   {
     return Void;
   }


  const std::string UlamTypeVoid::getUlamTypeAsStringForC()
  {
    return "void";
  }


  //anything can be cast to a void
  bool UlamTypeVoid::cast(UlamValue & val)
    {
      UTI valtypidx = val.getUlamValueType()->getUlamTypeIndex();
      bool brtn = true;

      u32 valarraysize = val.getUlamValueType()->getArraySize();
      u32 myarraysize = getArraySize();

      if(valarraysize == 0 && myarraysize == 0)
	{
	  switch(valtypidx)
	    {
	    case Void:
	    case Int:
	    case Float:
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

      return brtn;
    }


  void UlamTypeVoid::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    assert(m_key.m_arraySize == 0);      
    sprintf(valstr,"0");
  }


  bool UlamTypeVoid::isZero(const UlamValue & val)
  {
    return true;
  }

} //end MFM
