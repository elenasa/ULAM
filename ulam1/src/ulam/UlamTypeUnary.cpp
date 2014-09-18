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
     return Unary;
   }


  const std::string UlamTypeUnary::getUlamTypeAsStringForC()
  {
    //std::ostringstream ctype;
    //ctype <<  "s" << m_key.getUlamKeyTypeSignatureBitSize(); 
    //return ctype.str();
    return "unsigned int";
  }


  const char * UlamTypeUnary::getUlamTypeAsSingleLowercaseLetter()
  {
    return "u";
  }


  bool UlamTypeUnary::cast(UlamValue & val, CompilerState& state)
  {
    bool brtn = true;
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

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    if((bitsize != valbitsize) && (typEnum != valtypEnum))
      {
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
	
	valtypidx = val.getUlamValueTypeIdx();  //reload
	valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
      }
	
    u32 data = val.getImmediateData(state);    
    switch(valtypEnum)
      {
      case Bool:
	//	{
	//  s32 count1s = PopCount(data);
	//  if(count1s > (s32) (bitsize - count1s))
	//    val = UlamValue::makeImmediate(getUlamTypeIndex(), 1, state); //overwrite val
	//  else
	//    val = UlamValue::makeImmediate(getUlamTypeIndex(), 0, state); //overwrite val
	//}
	//break;
      case Int:
	{
	  // cast from Int->Unary, OR Bool->Unary
	  u32 count1s = PopCount(data);
	  val = UlamValue::makeImmediate(getUlamTypeIndex(), _GetNOnes32(count1s), state); //overwrite val
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
  
    return brtn;
  } //end cast    
  

  void UlamTypeUnary::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%u", PopCount(data));            //converted to binary
    else
      sprintf(valstr,"%c%u", prefix, PopCount(data));  //converted to binary
  }

} //end MFM
