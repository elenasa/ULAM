#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeInt.h"
#include "UlamValue.h"
#include "CompilerState.h"

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


  bool UlamTypeInt::cast(UlamValue & val, CompilerState& state)
    {
      bool brtn = true;

      UTI typidx = getUlamTypeIndex();
      UTI valtypidx = val.getUlamValueTypeIdx();

      u32 arraysize = getArraySize();
      if(arraysize != state.getArraySize(valtypidx))
	{
	  std::ostringstream msg;
	  msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: " << valtypidx << "," << state.getArraySize(valtypidx);
	  state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_ERR);
	  return false;
	}

      //base types e.g. Int, Bool, Unary, Foo, Bar..
      ULAMTYPE typEnum = getUlamTypeEnum();
      ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

      //change the size first, if necessary
      u32 bitsize = getBitSize();
      if(bitsize != state.getBitSize(valtypidx))
	{
	  if(typEnum == valtypEnum)
	    {
	      if(!castBitSize(val,state))
		{
		  //error!
		  return false;
		}
	    }
	  else
	    {
	      //change the size, within the val's current type
	      UlamKeyTypeSignature vkey1 = UlamKeyTypeSignature(valtypEnum, bitsize, arraysize);
	      UTI vtype1 = state.makeUlamType(vkey1, valtypEnum); //may not exist yet, create  

	      if(!(state.getUlamTypeByIndex(vtype1)->castBitSize(val,state)))
		{
		  //error! 
		  return false;
		}
	    }
	}

      // if same base type we're done, otherwise cast type
      if(typEnum != valtypEnum)
	{
	  //casting between types (e.g. Int->Bool, Bool->Int, Unary->Int)
	  // (same bit and array size)
	  switch(valtypidx)
	    {
	    case Bool:
	      {
		u32 data = val.getImmediateData(state);
		u32 count1s = PopCount(data);
		if(count1s > (bitsize - count1s))
		  {
		    val = UlamValue::makeImmediate(getUlamTypeIndex(), 1, state); //overwrite val
		  }
		else
		  {
		    val = UlamValue::makeImmediate(getUlamTypeIndex(), 0, state); //overwrite val
		  }
	      }
	      break;
	    case Unary:
	      {
		u32 data = val.getImmediateData(state);
		u32 count1s = PopCount(data);
		val = UlamValue::makeImmediate(getUlamTypeIndex(), count1s, state); //overwrite val
	      }
	      break;
	    case Int:
	      // nothing to do
	      break;
	    default:
	      //std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	      brtn = false;
	    };
	}
      return brtn;
    } //end cast


  bool UlamTypeInt::castBitSize(UlamValue & val, CompilerState& state)
  {
    UTI valtypidx = val.getUlamValueTypeIdx();
    assert(valtypidx == getUlamTypeIndex());

    u32 bitsize = getBitSize();
    u32 valbitsize = state.getBitSize(valtypidx);

    s32 data = val.getImmediateData(state);    
    if(bitsize < valbitsize)
      {
	//warning! may lose precision
      }
    else
      {
	data = _SignExtend32(data, bitsize);
      }

    val = UlamValue::makeImmediate(getUlamTypeIndex(), data, state); //overwrite val
    return true;
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
