#include <stdio.h>
#include <string.h>
#include <iostream>
#include "UlamTypeBool.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeBool::UlamTypeBool(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
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


  const std::string UlamTypeBool::getUlamTypeImmediateMangledName()
  {
    if(needsImmediateType())
      return UlamType::getUlamTypeImmediateMangledName();

    //return getImmediateStorageTypeAsString(state);  //"bool" inf loop
    //return getTmpStorageTypeAsString(state);
    return UlamType::getUlamTypeImmediateMangledName(); //? for constants
  }


  const char * UlamTypeBool::getUlamTypeAsSingleLowercaseLetter()
  {
    return "b";
  }


  bool UlamTypeBool::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    //UTI typidx = getUlamTypeIndex();
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();
    s32 arraysize = getArraySize();
    if(arraysize != m_state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: " << valtypidx << "," << m_state.getArraySize(valtypidx);
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	return false;
      }

    //change the size first of tobe, if necessary
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);

    if(bitsize == UNKNOWNSIZE || valbitsize == UNKNOWNSIZE)
      {
	std::ostringstream msg;
	msg << "Casting UNKNOWN sizes; " << bitsize << ", Value Type and size was: " << valtypidx << "," << valbitsize;
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	return false;
      }

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    //ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    u32 newdata = 0;
    u32 data = val.getImmediateData(m_state);
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
      val = UlamValue::makeImmediate(typidx, newdata, m_state); //overwrite val

    return brtn;
  } //end cast


  void UlamTypeBool::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    bool dataAsBool = false;
    if(!isComplete())
      {
	sprintf(valstr,"%s", "unknown");
      }
    else if(isConstant())
      {
	dataAsBool = (bool) data;
      }
    else
      {
	s32 bitsize = getBitSize();
	s32 count1s = PopCount(data);

	if(count1s > (s32) (bitsize - count1s))  // == when even number bits is ignored (warning at def)
	  dataAsBool = true;
      }

    if(prefix == 'z')
      sprintf(valstr,"%s", dataAsBool ? "true" : "false");
    else
      sprintf(valstr,"%c%s", prefix, dataAsBool ? "true" : "false");
  } //getDataAsString


  const std::string UlamTypeBool::getConvertToCboolMethod()
  {
    std::ostringstream rtnMethod;
    rtnMethod << "_Bool" << getTotalWordSize() << "ToCbool";
    return rtnMethod.str();
  } //getCovertToCBoolMethod

} //end MFM
