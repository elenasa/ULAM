#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "UlamTypeInt.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeInt::UlamTypeInt(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    s32 bitsize = getBitSize();
    m_wordLengthItem = calcWordSize(bitsize);

    // consider s64 later...
    m_max = calcBitsizeSignedMax(bitsize);
    m_min = calcBitsizeSignedMin(bitsize);
  }

   ULAMTYPE UlamTypeInt::getUlamTypeEnum()
   {
     return Int;
   }

  const std::string UlamTypeInt::getUlamTypeVDAsStringForC()
  {
    return "VD::S32";
  }

  //As it stands, this can refer to base class code now.
  const std::string UlamTypeInt::getUlamTypeImmediateMangledName()
  {
    if(needsImmediateType())
      return UlamType::getUlamTypeImmediateMangledName();

    //return getImmediateStorageTypeAsString(state); //BV<32>, not "s32" ? inf loop
    return UlamType::getUlamTypeImmediateMangledName(); //? for constants
  }

  const std::string UlamTypeInt::getArrayItemTmpStorageTypeAsString()
  {
    return getTmpStorageTypeAsString(getItemWordSize());
  }

  const std::string UlamTypeInt::getTmpStorageTypeAsString()
  {
    return getTmpStorageTypeAsString(getTotalWordSize());
  }

  const std::string UlamTypeInt::getArrayItemUnsignedTmpStorageTypeAsString()
  {
    return UlamType::getTmpStorageTypeAsString(getItemWordSize());
  }

  const std::string UlamTypeInt::getUnsignedTmpStorageTypeAsString()
  {
    return UlamType::getTmpStorageTypeAsString(getTotalWordSize());
  }

  const std::string UlamTypeInt::getTmpStorageTypeAsString(s32 sizebyints)
  {
    std::string ctype;
    switch(sizebyints)
      {
      case 0:
      case 32:
	ctype = "s32";
	break;
      case 64:
	ctype = "s64";
	break;
      default:
	{
	  //std::ostringstream msg;
	  //msg << "Need UNPACKED ARRAY for " << sizebyints << " bits; s32[" << getArraySize() << "]";
	  //MSG3(state.getFullLocationAsString(state.m_locOfNextLineText).c_str(), msg.str().c_str(),INFO);
	  ctype = "s32"; //array item
	  //assert(0);
	}
      };
    return ctype;
  } //getTmpStorageTypeAsString

  const char * UlamTypeInt::getUlamTypeAsSingleLowercaseLetter()
  {
    return "i";
  }

  bool UlamTypeInt::cast(UlamValue & val, UTI typidx)
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
	msg << "Casting UNKNOWN sizes; " << bitsize << ", Value Type and size was: ";
	msg << valtypidx << "," << valbitsize;
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	return false;
      }

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    //ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    u32 data = val.getImmediateData(m_state);

    switch(valtypEnum)
      {
      case Int:
	{
	  // casting Int to Int to change bits size
	  s32 sdata = _SignExtend32(data, valbitsize);
	  sdata = _Int32ToInt32(sdata, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, m_state); //overwrite val
	}
	break;
      case Unsigned:
	{
	  // casting Unsigned to Int to change type
	  s32 sdata = _Unsigned32ToInt32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, m_state); //overwrite val
	}
	break;
      case Bits:
	{
	  // casting Bits to Int to change type
	  s32 sdata = _Bits32ToInt32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, m_state); //overwrite val
	}
	break;
      case Unary:
	{
	  s32 sdata = _Unary32ToInt32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, m_state); //overwrite val
	}
	break;
      case Bool:
	{
	  s32 sdata = _Bool32ToInt32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, m_state); //overwrite val
	}
	break;
      case Void:
      default:
	//std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn;
  } //end cast

  void UlamTypeInt::genCodeAfterReadingIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    assert(uvpass.getUlamValueTypeIdx() == Ptr);

    //bypass sign extend when entire array is read
    if(!isScalar() && uvpass.getPtrLen() == (s32) getTotalBitSize())
      return;

    if(!isScalar())
      return genCodeAfterReadingArrayItemIntoATmpVar(fp, uvpass);

    UTI uti = uvpass.getPtrTargetType(); // == getUlamTypeIndex()?
    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    s32 tmpVarCastNum = m_state.getNextTmpVarNumber();
    s32 totWords = getTotalWordSize();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //i.e. s32, s64
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(uti, tmpVarCastNum).c_str());
    fp->write(" = ");

    fp->write("_SignExtend");
    fp->write_decimal(totWords);

    fp->write("(");
    fp->write(m_state.getTmpVarAsString(uti, tmpVarNum).c_str());
    fp->write(", ");
    fp->write_decimal(getTotalBitSize());
    fp->write(")");
    fp->write(";\n");

    UTI newuti = uti;

    uvpass = UlamValue::makePtr(tmpVarCastNum, TMPREGISTER, newuti, getPackable(), m_state, 0, uvpass.getPtrNameId()); //POS 0 rightjustified (atom-based); pass along name id
  } //genCodeAfterReadingIntoATmpVar

  // private helper
  void UlamTypeInt::genCodeAfterReadingArrayItemIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI uti = uvpass.getPtrTargetType(); //getUlamTypeIndex();
    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    s32 tmpVarCastNum = m_state.getNextTmpVarNumber();
    s32 itemWords = getItemWordSize();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //i.e. s32, s64
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(uti, tmpVarCastNum).c_str());
    fp->write(" = ");

    fp->write("_SignExtend");
    fp->write_decimal(itemWords);

    fp->write("(");
    fp->write(m_state.getTmpVarAsString(uti, tmpVarNum).c_str());
    fp->write(", ");
    fp->write_decimal(getBitSize());
    fp->write(")");
    fp->write(";\n");

    UTI newuti = uti;
    uvpass = UlamValue::makePtr(tmpVarCastNum, TMPREGISTER, newuti, getPackable(), m_state, 0, uvpass.getPtrNameId()); //POS 0 rightjustified (atom-based); pass along name id
  } //genCodeAfterReadingArrayItemIntoATmpVar

  void UlamTypeInt::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%d", (s32) data);
    else
      sprintf(valstr,"%c%d", prefix, (s32) data);
  }

  void UlamTypeInt::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%ld", (s64) data);
    else
      sprintf(valstr,"%c%ld", prefix, (s64) data);
  }

} //end MFM
