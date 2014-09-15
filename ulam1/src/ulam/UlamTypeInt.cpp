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

	
    valtypidx = val.getUlamValueTypeIdx();  //might have changed, so reload
    valbitsize = state.getBitSize(valtypidx);
    //assert(bitsize == valbitsize);

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
	    val = UlamValue::makeImmediate(getUlamTypeIndex(), 1, state); //overwrite val
	  else
	    val = UlamValue::makeImmediate(getUlamTypeIndex(), 0, state); //overwrite val
	}
	break;
      case Unary:
	{
	  u32 count1s = PopCount(data);
	  val = UlamValue::makeImmediate(getUlamTypeIndex(), count1s, state); //overwrite val
	  }
	break;
      case Int:
	val = UlamValue::makeImmediate(getUlamTypeIndex(), data, state); //overwrite val to change bitsize
	break;
      default:
	//std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn;
  } //end cast
  
  
#if 0
  bool UlamTypeInt::castBitSize(UlamValue & val, CompilerState& state)
  {
    UTI valtypidx = val.getUlamValueTypeIdx();
    //assert(valtypidx == getUlamTypeIndex());
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    assert(typEnum == valtypEnum);

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
#endif


  void UlamTypeInt::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%d", (s32) data);
    else
      sprintf(valstr,"%c%d", prefix, (s32) data);
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
