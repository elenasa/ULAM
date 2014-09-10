#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeInt.h"
#include "UlamValue.h"

namespace MFM {

  UlamTypeInt::UlamTypeInt(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypeInt::getUlamTypeEnum()
   {
     return Int;
   }


  const std::string UlamTypeInt::getUlamTypeAsStringForC()
  {
    //std::ostringstream ctype;
    //ctype <<  "s" << m_key.getUlamKeyTypeSignatureBitSize(); 
    //return ctype.str();
    return "int";
  }


  const char * UlamTypeInt::getUlamTypeAsSingleLowercaseLetter()
  {
    return "i";
  }

  //NOT SURE HOW THESE WORK ANYMORE ???
  bool UlamTypeInt::cast(UlamValue & val)
    {
      UTI valtypidx = val.getUlamValueTypeIdx();
      bool brtn = true;
#if 0
      u32 valarraysize = state.getArraySize(valtypidx);
      u32 myarraysize = getArraySize();

      if(valarraysize == 0 && myarraysize == 0)
	{
	  switch(valtypidx)
	    {
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
#endif
      return brtn;
    }


   void UlamTypeInt::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
   {
     if(m_key.m_arraySize == 0)
       {
	 s32 idata = (s32) val.getImmediateData(state);
	 sprintf(valstr,"%d", idata);
       }
     else
       {
	 UlamValue ival = val.getValAt(0, state);
	 s32 idata = (s32) ival.getImmediateData(state);
	 char tmpstr[8];
	 sprintf(valstr,"%d", idata);
	 for(s32 i = 1; i < (s32) m_key.m_arraySize ; i++)
	   {
	     ival = val.getValAt(i, state);
	     idata = (s32) ival.getImmediateData(state);
	     sprintf(tmpstr,",%d", idata); 
	     strcat(valstr,tmpstr);
	   }
       }
   }

} //end MFM
