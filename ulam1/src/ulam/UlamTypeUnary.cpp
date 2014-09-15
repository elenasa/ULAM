#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeUnary.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeUnary::UlamTypeUnary(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {}


   ULAMTYPE UlamTypeUnary::getUlamTypeEnum()
   {
     return Int;
   }


  const std::string UlamTypeUnary::getUlamTypeAsStringForC()
  {
    //std::ostringstream ctype;
    //ctype <<  "s" << m_key.getUlamKeyTypeSignatureBitSize(); 
    //return ctype.str();
    return "int";
  }


  const char * UlamTypeUnary::getUlamTypeAsSingleLowercaseLetter()
  {
    return "i";
  }


  bool UlamTypeUnary::cast(UlamValue & val, CompilerState& state)
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
    
    //change the size first of tobe, if necessary
    u32 bitsize = getBitSize();
    u32 valbitsize = state.getBitSize(valtypidx);
    if(bitsize != valbitsize)
      {
	//base types e.g. Int, Bool, Unary, Foo, Bar..
	ULAMTYPE typEnum = getUlamTypeEnum();

	//change to val's size, within the TOBE current type; 
	//get string index for TOBE enum type string
	u32 enumStrIdx = state.m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(typEnum));
	UlamKeyTypeSignature vkey1 = UlamKeyTypeSignature(enumStrIdx, valbitsize, arraysize);
	UTI vtype1 = state.makeUlamType(vkey1, typEnum); //may not exist yet, create  
	
	if(!(state.getUlamTypeByIndex(vtype1)->cast(val,state))) //val changes!!!
	  {
	    //error! 
	    return false;
	  }
      }
	
    //might have changed, so reload
    valtypidx = val.getUlamValueTypeIdx();  
    valbitsize = state.getBitSize(valtypidx);
    //    assert(bitsize == valbitsize);

    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    // casting Int to Int to change bits
    u32 newdata = 0;
    u32 data = val.getImmediateData(state);
    
    switch(valtypEnum)
      {
      case Bool:
	{
	  s32 count1s = PopCount(data);
	  if(count1s > (s32) (bitsize - count1s))
	    {
	      val = UlamValue::makeImmediate(getUlamTypeIndex(), 1, state); //overwrite val
	    }
	  else
	    {
	      val = UlamValue::makeImmediate(getUlamTypeIndex(), 0, state); //overwrite val
	    }
	}
	break;
      case Int:
	{
	  // cast from Int to Unary
	  u32 count1s = PopCount(data);
	  val = UlamValue::makeImmediate(getUlamTypeIndex(), count1s, state); //overwrite val
	}
	break;
      case Unary:
	{
	  // size type change..
	  if(bitsize < valbitsize)
	    val = UlamValue::makeImmediate(getUlamTypeIndex(), data, state); //overwrite val, same data
	  else
	    {
	      u32 count1s = PopCount(data);
	      val = UlamValue::makeImmediate(getUlamTypeIndex(), _GetNOnes32(count1s), state); //overwrite val, max 1s
	    }
	}
	break;
      default:
	//std::cerr << "UlamTypeUnary (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
  } //end cast    
  

#if 0
  bool UlamTypeUnary::castBitSize(UlamValue & val, CompilerState& state)
  {
    bool rtnb = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    //assert(valtypidx == getUlamTypeIndex());
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    assert(typEnum == valtypEnum);
    
    
    u32 bitsize = getBitSize();
    u32 valbitsize = state.getBitSize(valtypidx);
    
    u32 data = val.getImmediateData(state);    
    u32 count1s = PopCount(data);
    if(bitsize < valbitsize)
      {
	if(count1s > bitsize)
	  rtnb = false;   //error! losing precision 
	data = _GetNOnes32(count1s);
      }
    
    val = UlamValue::makeImmediate(getUlamTypeIndex(), data, state); //overwrite val
    return rtnb;
  }
#endif


  void UlamTypeUnary::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", PopCount(data));  //converted to binary
    else
      sprintf(valstr,"%c%u", prefix, PopCount(data));  //converted to binary
  }


  void UlamTypeUnary::getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state)
  {
    if(m_key.m_arraySize == 0)
      {
	u32 idata = val.getImmediateData(state);
	sprintf(valstr,"%d", idata);
      }
    else
      {
	UlamValue ival = val.getValAt(0, state);
	u32 idata =  ival.getImmediateData(state);
	char tmpstr[8];
	sprintf(valstr,"%d", idata);
	for(s32 i = 1; i < (s32) m_key.m_arraySize ; i++)
	  {
	    ival = val.getValAt(i, state);
	    idata = ival.getImmediateData(state);
	    sprintf(tmpstr,",%d", idata); 
	    strcat(valstr,tmpstr);
	  }
      }
   }
  
} //end MFM
