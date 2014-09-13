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


  bool UlamTypeBool::cast(UlamValue & val, CompilerState& state)
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
    
    // casting Bool to Bool could improve the bit count!
    // if same base type we're done, otherwise cast type
    // if(typEnum != valtypEnum)
    {
      u32 newdata = 0;
      u32 data = val.getImmediateData(state);
      
      switch(valtypidx)
	{
	case Int:
	case Unary:
	  {
	    if(data != 0) //signed or unsigned
	      {
		newdata = _GetNOnes32(bitsize);  //all ones if true
	      }
	  }
	  break;
	case Bool:
	  {
	    u32 count1s = PopCount(data);
	    if(count1s > (bitsize - count1s))
	      newdata = _GetNOnes32(bitsize);  //all ones if true
	  }
	  break;
	default:
	  //std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	  brtn = false;
	};
    
      if(brtn)
	val = UlamValue::makeImmediate(getUlamTypeIndex(), newdata, state); //overwrite val
    }

    return brtn;
  } //end cast
  

  bool UlamTypeBool::castBitSize(UlamValue & val, CompilerState& state)
  {
    UTI valtypidx = val.getUlamValueTypeIdx();
    assert(valtypidx == getUlamTypeIndex());

    u32 bitsize = getBitSize();
    u32 data = val.getImmediateData(state);
    u32 count1s = PopCount(data);
    if(count1s > (bitsize - count1s))
      data = _GetNOnes32(bitsize);  //all ones if true
    else
      data = 0;             //o.w. all zeros for false
    
    val = UlamValue::makeImmediate(getUlamTypeIndex(), data, state); //overwrite val
    return true;
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
