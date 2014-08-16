#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeInt.h"
#include "UlamValue.h"

namespace MFM {

  UlamTypeInt::UlamTypeInt(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


  void UlamTypeInt::newValue(UlamValue & val)
  {
    assert((val.isArraySize() == m_key.m_arraySize) && (m_key.m_arraySize == 0));
    val.m_valInt = 0;  //init to zero
  }


  void UlamTypeInt::deleteValue(UlamValue * val){}


   ULAMTYPE UlamTypeInt::getUlamTypeEnum()
   {
     return Int;
   }


  const std::string UlamTypeInt::getUlamTypeAsStringForC()
  {
    std::ostringstream ctype;
    ctype <<  "s" << m_key.m_bits;
    return ctype.str();
  }


  bool UlamTypeInt::cast(UlamValue & val)
    {
      UTI valtypidx = val.getUlamValueType()->getUlamTypeIndex();
      bool brtn = true;

      u32 valarraysize = val.getUlamValueType()->getArraySize();
      u32 myarraysize = getArraySize();

      if(valarraysize == 0 && myarraysize == 0)
	{
	  switch(valtypidx)
	    {
	    case Float:
	      val.init(this, (s32) val.m_valFloat);
	      break;
	    case Bool:
	      val.init(this, (s32) val.m_valBool);
	      break;
	    case Int:
	      break;
	    default:
	      //std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	      brtn = false;
	    };
	}
      else
	{
	  if(myarraysize != valarraysize)
	    {
	      assert(0);
	      //std::cerr << "UlamTypeInt (cast) error! Different Array sizes; " << myarraysize << " Value Type and size was: " << valtypidx << "," << valarraysize << std::endl;
	      brtn=false;
	    }
	}

      return brtn;
    }


  void UlamTypeInt::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    if(m_key.m_arraySize == 0)
      {
	  sprintf(valstr,"%d", val.m_valInt);
      }
    else
      {
	UlamValue ival = val.getValAt(0, state);
	char tmpstr[8];
	sprintf(valstr,"%d", ival.m_valInt);
	for(s32 i = 1; i < (s32) m_key.m_arraySize ; i++)
	  {
	    ival = val.getValAt(i, state);
	    sprintf(tmpstr,",%d", ival.m_valInt); 
	    strcat(valstr,tmpstr);
	  }
      }
  }


  bool UlamTypeInt::isZero(const UlamValue & val)
  {
    return (val.m_valInt == 0);
  }

} //end MFM
