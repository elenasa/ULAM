#include <stdio.h>
#include <string.h>
#include <iostream>
#include "UlamTypeBool.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeBool::UlamTypeBool(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypeBool::getUlamTypeEnum()
   {
     return Bool;
   }


  const std::string UlamTypeBool::getUlamTypeAsStringForC()
  {
    return "bool";
  }


  const char * UlamTypeBool::getUlamTypeAsSingleLowercaseLetter()
  {
    return "b";
  }


  bool UlamTypeBool::cast(UlamValue & val)
    {
      UTI valtypidx = val.getUlamValueTypeIdx();
      return(valtypidx == Int || valtypidx == Bool);
    }


  void UlamTypeBool::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    if(m_key.m_arraySize == 0)
      {
	bool bdata = (bool) val.getImmediateData(state);
	sprintf(valstr,"%s", bdata ? "true" : "false");
      }
    else
      {
	assert(val.getUlamValueTypeIdx() == Ptr);
	UlamValue ival = val.getValAt(0, state);
	bool bdata = (bool) ival.getImmediateData(state);
	char tmpstr[8];
	sprintf(valstr,"%s", bdata ? "true" : "false"); 
	for(s32 i = 1; i < (s32) m_key.m_arraySize ; i++)
	  {
	    ival = val.getValAt(i, state);
	    bdata = (bool) ival.getImmediateData(state);
	    sprintf(tmpstr,",%s", bdata ? "true" : "false"); 
	    strcat(valstr,tmpstr);
	  }
      }
  }      
      
} //end MFM
