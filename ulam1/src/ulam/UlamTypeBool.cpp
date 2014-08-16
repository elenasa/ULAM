#include <stdio.h>
#include <string.h>
#include <iostream>
#include "UlamTypeBool.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeBool::UlamTypeBool(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}

  void UlamTypeBool::newValue(UlamValue & val)
  {
    assert((val.isArraySize() == m_key.m_arraySize) && (m_key.m_arraySize == 0));
    val.m_valBool = false;  //init to zero
  }


  void UlamTypeBool::deleteValue(UlamValue * val)
  {}


   ULAMTYPE UlamTypeBool::getUlamTypeEnum()
   {
     return Bool;
   }


  const std::string UlamTypeBool::getUlamTypeAsStringForC()
  {
    return "bool";
  }


  bool UlamTypeBool::cast(UlamValue & val)
    {
      UTI valtypidx = val.getUlamValueType()->getUlamTypeIndex();
      bool brtn = true;

      switch(valtypidx)
	{
	case Int:
	  val.init(this, (bool) val.m_valInt);
	  break;
	case Float:
	  val.init(this, (bool) val.m_valFloat);
	  break;
	case Bool:
	  break;
	default:
	  //std::cerr << "UlamTypeBool (cast) error! Value Type was: " << valtypidx << std::endl;
	  brtn = false;
	};

      return brtn;
    }


  void UlamTypeBool::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    if(m_key.m_arraySize == 0)
      sprintf(valstr,"%s", val.m_valBool ? "true" : "false");
    else
      {
	UlamValue ival = val.getValAt(0, state);
	char tmpstr[8];
	sprintf(valstr,"%s", ival.m_valBool ? "true" : "false"); 
	for(s32 i = 1; i < (s32) m_key.m_arraySize ; i++)
	  {
	    ival = val.getValAt(i, state);
	    sprintf(tmpstr,",%s", ival.m_valBool ? "true" : "false"); 
	    strcat(valstr,tmpstr);
	  }
      }
  }


  bool UlamTypeBool::isZero(const UlamValue & val)
  {
    return (! val.m_valBool); 
  }

      
      
} //end MFM
