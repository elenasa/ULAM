#include <iostream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeFloat.h"
#include "UlamValue.h"

namespace MFM {

  UlamTypeFloat::UlamTypeFloat(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


  void UlamTypeFloat::newValue(UlamValue & val)
  {
    assert((val.isArraySize() == m_key.m_arraySize) && (m_key.m_arraySize == 0));
    val.m_valFloat = 0.0;  //init to zero
  }


  void UlamTypeFloat::deleteValue(UlamValue * val)
  { }


   ULAMTYPE UlamTypeFloat::getUlamTypeEnum()
   {
     return Float;
   }


  const std::string UlamTypeFloat::getUlamTypeAsStringForC()
  {
    return "float";
  }


  bool UlamTypeFloat::cast(UlamValue & val)
    {
      UTI valtypidx = val.getUlamValueType()->getUlamTypeIndex();
      bool brtn = true;

      switch(valtypidx)
	{
	case Int:
	  val.init(this, (float) val.m_valInt);
	  break;
	case Bool:
	  val.init(this, (float) val.m_valBool);
	  break;
	case Float:
	  break;
	default:
	  //std::cerr << "UlamTypeFloat (cast) error! Value Type was: " << valtypidx << std::endl;
	  brtn = false;
	};
      
      return brtn;
    }

  
  void UlamTypeFloat::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    if(m_key.m_arraySize == 0)
      sprintf(valstr,"%f", val.m_valFloat);
    else
      {
	UlamValue ival = val.getValAt(0, state);
	char tmpstr[8];
	sprintf(valstr,"%f", ival.m_valFloat); 
	for(s32 i = 1; i < (s32) m_key.m_arraySize ; i++)
	  {
	    ival = val.getValAt(i, state);
	    sprintf(tmpstr,",%f", ival.m_valFloat); 
	    strcat(valstr,tmpstr);
	  }
      }
  }


  bool UlamTypeFloat::isZero(const UlamValue & val)
  {
    return (val.m_valFloat == 0.0);
  }

} //end MFM
