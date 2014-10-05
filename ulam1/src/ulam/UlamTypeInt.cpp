#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeInt.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInt::UlamTypeInt(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {
    m_lengthBy32 = fitsIntoInts(getTotalBitSize());
  }


   ULAMTYPE UlamTypeInt::getUlamTypeEnum()
   {
     return Int;
   }


  const std::string UlamTypeInt::getUlamTypeVDAsStringForC()
  {
    return "VD::S32";
  }


  const std::string UlamTypeInt::getImmediateTypeAsString(CompilerState * state)
  {
    std::string ctype;
    s32 sizeByIntBits = getTotalSizeByInts();
    switch(sizeByIntBits)
      {
      case 32:
	ctype = "s32";
	break;
      case 64:
	ctype = "s64";
	break;
      default:
	{
	  assert(0);
	  //MSG(getNodeLocationAsString().c_str(), "Need UNPACKED ARRAY", INFO);
	}
	//error!
      };
    
    return ctype;
  } //getImmediateTypeAsString


  const std::string UlamTypeInt::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    if(needsImmediateType())
      return UlamType::getUlamTypeImmediateMangledName(state);

    return "s32";
  }


  bool UlamTypeInt::needsImmediateType()
  {
    return ((getBitSize() == ANYBITSIZECONSTANT || getBitSize() == MAXBITSPERINT) ? false : true);
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
	
    if(state.isConstant(typidx))
      {
	// avoid using out-of-band value as bitsize
	bitsize = state.getDefaultBitSize(typidx);
      }

    u32 data = val.getImmediateData(state);
    switch(valtypEnum)
      {
      case Int:
      case Unsigned:
      case Bits:
	// casting Int to Int to change bits size
	// casting Unsigned to Int to change type
	// casting Bits to Int to change type
	val = UlamValue::makeImmediate(typidx, data, state); //overwrite val
	break;
      case Unary:
	{
	  u32 count1s = PopCount(data);
	  val = UlamValue::makeImmediate(typidx, count1s, state); //overwrite val
	}
	break;
      case Bool:
	{
	  if(state.isConstant(valtypidx))  // bitsize is misleading
	    {
	      if(data != 0) //signed or unsigned
		val = UlamValue::makeImmediate(typidx, 1, state); //overwrite val
	      else
		val = UlamValue::makeImmediate(typidx, 0, state); //overwrite val
	    }
	  else
	    {
	      s32 count1s = PopCount(data);
	      if(count1s > (s32) (valbitsize - count1s))
		val = UlamValue::makeImmediate(typidx, 1, state); //overwrite val
	      else
		val = UlamValue::makeImmediate(typidx, 0, state); //overwrite val
	    }
	}
	break;
      case Void:
      default:
	//std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn;
  } //end cast


  const std::string UlamTypeInt::castMethodForCodeGen(UTI nodetype, CompilerState& state)
  {
    std::ostringstream rtnMethod;
    UlamType * nut = state.getUlamTypeByIndex(nodetype);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE nodetypEnum = nut->getUlamTypeEnum();
    s32 sizeByIntBits = nut->getTotalSizeByInts();
    switch(nodetypEnum)
      {
      case Unsigned:
	rtnMethod << 	"_SignExtend" << sizeByIntBits;
	break;
      case Int:
	rtnMethod << 	"_SignExtend" << sizeByIntBits;
	break;
      default:
	return UlamType::castMethodForCodeGen(nodetype, state); //standard '_NodeToNew32' format
	break;
      };
    return rtnMethod.str();
  } //end castmethod


  void UlamTypeInt::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%d", (s32) data);
    else
      sprintf(valstr,"%c%d", prefix, (s32) data);
  }
  
} //end MFM
