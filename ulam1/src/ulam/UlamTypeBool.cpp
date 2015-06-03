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
    m_max = (getBitSize() <= 0 ? 0 : _GetNOnes32((u32) getBitSize())); // was = 1;
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
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();
    s32 arraysize = getArraySize();
    if(arraysize != m_state.getArraySize(valtypidx))
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes; " << arraysize << ", Value Type and size was: ";
	msg << valtypidx << "," << m_state.getArraySize(valtypidx);
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	return false;
      }

    //change the size first of tobe, if necessary
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);

    if(bitsize == UNKNOWNSIZE || valbitsize == UNKNOWNSIZE)
      {
	std::ostringstream msg;
	msg << "Casting UNKNOWN sizes; " << bitsize << ", Value Type and size was: " << valtypidx << "," << valbitsize;
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	return false;
      }

    u32 wordsize = getTotalWordSize();
    u32 valwordsize = m_state.getTotalWordSize(valtypidx);
    if(wordsize == MAXBITSPERINT)
      {
	if(valwordsize == MAXBITSPERINT)
	  brtn = castTo32(val, typidx);
	else if(valwordsize == MAXBITSPERLONG)
	  brtn = castTo64(val, typidx); //downcast
	else
	  assert(0);
      }
    else if(wordsize == MAXBITSPERLONG)
      brtn = castTo64(val, typidx);
    else
      {
	std::ostringstream msg;
	msg << "Casting to an unsupported word size: " << wordsize;
	msg << ", Value Type and bit size was: ";
	msg << valtypidx << "," << valbitsize;
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	brtn = false;
      }
    return brtn;
  } //cast

  bool UlamTypeBool::castTo32(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    u32 data = val.getImmediateData(m_state);

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    u32 newdata = 0;
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
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
  } //castTo32

  bool UlamTypeBool::castTo64(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx();
    u32 valwordsize = m_state.getTotalWordSize(valtypidx);
    u64 data;

    if(valwordsize == MAXBITSPERINT)
      data = (u64) val.getImmediateData(m_state);
    else if(valwordsize == MAXBITSPERLONG)
      data = val.getImmediateDataLong(m_state);
    else
      assert(0);

    u64 newdata = 0;
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(valtypidx);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    switch(valtypEnum)
      {
      case Int:
	{
	  s64 sdata = _SignExtend64(data, valbitsize);
	  newdata = _Int64ToBool64(sdata, valbitsize, bitsize);
	}
	break;
      case Unsigned:
	newdata = _Unsigned64ToBool64(data, valbitsize, bitsize);
	break;
      case Unary:
	newdata = _Unary64ToBool64(data, valbitsize, bitsize); //all ones if true
	break;
      case Bool:
	newdata = _Bool64ToBool64(data, valbitsize, bitsize); //all ones if true
	break;
      case Bits:
	newdata = _Bits64ToBool64(data, valbitsize, bitsize);  //no change to Bits data
	break;
      case Void:
      default:
	//std::cerr << "UlamTypeBool (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    if(brtn)
      {
	u32 wordsize = getTotalWordSize(); //tobe
	if(wordsize == MAXBITSPERINT) //downcast
	  val = UlamValue::makeImmediate(typidx, newdata, m_state); //overwrite val
	else if(wordsize == MAXBITSPERLONG)
	  val = UlamValue::makeImmediateLong(typidx, newdata, m_state); //overwrite val
	else
	  assert(0);
      }
    return brtn;
  } //castTo64

  void UlamTypeBool::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    bool dataAsBool = false;
    if(!isComplete())
      {
	sprintf(valstr,"%s", "unknown");
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

  void UlamTypeBool::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    bool dataAsBool = false;
    if(!isComplete())
      {
	sprintf(valstr,"%s", "unknown");
      }
    else
      {
	s32 bitsize = getBitSize();
	s32 count1s = PopCount(data);

	if(count1s > (s64) (bitsize - count1s))  // == when even number bits is ignored (warning at def)
	  dataAsBool = true;
      }

    if(prefix == 'z')
      sprintf(valstr,"%s", dataAsBool ? "true" : "false");
    else
      sprintf(valstr,"%c%s", prefix, dataAsBool ? "true" : "false");
  } //getDataLongAsString

  const std::string UlamTypeBool::getConvertToCboolMethod()
  {
    std::ostringstream rtnMethod;
    rtnMethod << "_Bool" << getTotalWordSize() << "ToCbool";
    return rtnMethod.str();
  } //getCovertToCBoolMethod

} //end MFM
