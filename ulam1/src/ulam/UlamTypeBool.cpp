#include <stdio.h>
#include <string.h>
#include <iostream>
#include "UlamTypeBool.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeBool::UlamTypeBool(const UlamKeyTypeSignature key, const UTI uti) : UlamType(key, uti)
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
    m_max = 1;
    m_min = 0;
  }


   ULAMTYPE UlamTypeBool::getUlamTypeEnum()
   {
     return Bool;
   }


  const std::string UlamTypeBool::getUlamTypeVDAsStringForC()
  {
    return "VD::BOOL";
  }


  const std::string UlamTypeBool::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    if(needsImmediateType())
      return UlamType::getUlamTypeImmediateMangledName(state);

    //return getImmediateStorageTypeAsString(state);  //"bool" inf loop
    //return getTmpStorageTypeAsString(state);
    return UlamType::getUlamTypeImmediateMangledName(state); //? for constants
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
    s32 arraysize = getArraySize();
    if(arraysize != state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: " << valtypidx << "," << state.getArraySize(valtypidx);
	MSG3(state.getFullLocationAsString(state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	return false;
      }

    //change the size first of tobe, if necessary
    s32 bitsize = getBitSize();
    s32 valbitsize = state.getBitSize(valtypidx);

    if(bitsize == UNKNOWNSIZE || valbitsize == UNKNOWNSIZE)
      {
	std::ostringstream msg;
	msg << "Casting UNKNOWN sizes; " << bitsize << ", Value Type and size was: " << valtypidx << "," << valbitsize;
	MSG3(state.getFullLocationAsString(state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	return false;
      }

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    //ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    u32 newdata = 0;
    u32 data = val.getImmediateData(state);
    switch(valtypEnum)
      {
      case Int:
	{
	  s32 sdata = _SignExtend32(data, valbitsize);
	  newdata = _Int32ToBool32(sdata, valbitsize, bitsize);
	}
	break;
      case Unsigned:
	newdata = _Unsigned32ToBool32(data, valbitsize, bitsize);
	break;
      case Unary:
	newdata = _Unary32ToBool32(data, valbitsize, bitsize); //all ones if true
	break;
      case Bool:
	newdata = _Bool32ToBool32(data, valbitsize, bitsize); //all ones if true
	break;
      case Bits:
	newdata = _Bits32ToBool32(data, valbitsize, bitsize);  //no change to Bits data
	break;
      case Void:
      default:
	//std::cerr << "UlamTypeBool (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      val = UlamValue::makeImmediate(typidx, newdata, state); //overwrite val

    return brtn;
  } //end cast


  void UlamTypeBool::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    UTI typidx = getUlamTypeIndex();
    bool dataAsBool = false;
    if(!isComplete())
      {
	sprintf(valstr,"%s", "unknown");
      }
    else if(state.isConstant(typidx))
      {
	dataAsBool = (bool) data;
      }
    else
      {
	s32 bitsize = state.getBitSize(typidx);
	s32 count1s = PopCount(data);

	if(count1s > (s32) (bitsize - count1s))  // == when even number bits is ignored (warning at def)
	  dataAsBool = true;
      }

    if(prefix == 'z')
      sprintf(valstr,"%s", dataAsBool ? "true" : "false");
    else
      sprintf(valstr,"%c%s", prefix, dataAsBool ? "true" : "false");
  }


  const std::string UlamTypeBool::getConvertToCboolMethod()
  {
    std::ostringstream rtnMethod;
    rtnMethod << "_Bool" << getTotalWordSize() << "ToCbool";
    return rtnMethod.str();
  } //getCovertToCBoolMethod

} //end MFM
