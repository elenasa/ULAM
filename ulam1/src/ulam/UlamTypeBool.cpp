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
    UTI valtypidx = val.getUlamValueTypeIdx();    
    s32 arraysize = getArraySize();
    if(arraysize != state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: " << valtypidx << "," << state.getArraySize(valtypidx);
	state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_ERR);
	return false;
      }
    
    //change the size first of tobe, if necessary
    s32 bitsize = getBitSize();
    s32 valbitsize = state.getBitSize(valtypidx);
    
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    if((bitsize != valbitsize) && (typEnum != valtypEnum))
      {
	//change to val's size, within the TOBE current type; 
	//get string index for TOBE enum type string
	u32 enumStrIdx = state.m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(typEnum));
	UlamKeyTypeSignature vkey1(enumStrIdx, valbitsize, arraysize);
	UTI vtype1 = state.makeUlamType(vkey1, typEnum); //may not exist yet, create  
	
	if(!(state.getUlamTypeByIndex(vtype1)->cast(val,state))) //val changes!!!
	  {
	    //error! 
	    return false;
	  }
	
	valtypidx = val.getUlamValueTypeIdx();  //reload
	valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
      }

    u32 newdata = 0;
    u32 data = val.getImmediateData(state);    
    switch(valtypEnum)
      {
      case Int:
      case Unary:
	{
	  if(data != 0) //signed or unsigned
	    newdata = _GetNOnes32(bitsize);  //all ones if true
	}
	break;
      case Bool:
	{
	  if(state.isConstant(valtypidx))
	    {
	      if(data != 0) //signed or unsigned
		newdata = _GetNOnes32(bitsize);  //all ones if true
	    }
	  else
	    {
	      // casting Bool to Bool could improve the bit count!
	      s32 count1s = PopCount(data);
	      if(count1s > (s32) (valbitsize - count1s))  // == when even number bits is ignored (warning at def)
		newdata = _GetNOnes32(bitsize);        //all ones if true
	    }
	}
	break;
      default:
	//std::cerr << "UlamTypeBool (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    
    if(brtn)
      val = UlamValue::makeImmediate(getUlamTypeIndex(), newdata, state); //overwrite val
    
    return brtn;
  } //end cast
  

  void UlamTypeBool::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%s", data ? "true" : "false");
    else
      sprintf(valstr,"%c%s", prefix, data ? "true" : "false");
  }

} //end MFM
