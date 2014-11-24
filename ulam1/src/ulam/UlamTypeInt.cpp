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
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
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
  const std::string UlamTypeInt::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    if(needsImmediateType())
      return UlamType::getUlamTypeImmediateMangledName(state);

    //return getImmediateStorageTypeAsString(state); //BV<32>, not "s32" ? inf loop
    return UlamType::getUlamTypeImmediateMangledName(state); //? for constants
  }


  const std::string UlamTypeInt::getArrayItemTmpStorageTypeAsString(CompilerState * state)
  {
    return getTmpStorageTypeAsString(state, getItemWordSize());
  }

  const std::string UlamTypeInt::getTmpStorageTypeAsString(CompilerState * state)
  {
    return getTmpStorageTypeAsString(state, getTotalWordSize());
  }


  const std::string UlamTypeInt::getTmpStorageTypeAsString(CompilerState * state, s32 sizebyints)
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
	  std::ostringstream msg;
	  msg << "Need UNPACKED ARRAY for " << sizebyints << " bits; s32[" << getArraySize() << "]";
	  state->m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_INFO);
	  ctype = "s32*"; //array
	  assert(0);
	}
      };
    
    return ctype;
  } //getTmpStorageTypeAsString


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
    //ULAMTYPE typEnum = getUlamTypeEnum();
    ULAMTYPE valtypEnum = state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();

    u32 data = val.getImmediateData(state);

    switch(valtypEnum)
      {
      case Int:
	{
	  // casting Int to Int to change bits size
	  s32 sdata = _SignExtend32(data, valbitsize);
	  sdata = _Int32ToInt32(sdata, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, state); //overwrite val
	}
	break;
      case Unsigned:
	{
	  // casting Unsigned to Int to change type
	  s32 sdata = _Unsigned32ToInt32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, state); //overwrite val
	}
	break;
      case Bits:
	{
	  // casting Bits to Int to change type
	  s32 sdata = _Bits32ToInt32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, state); //overwrite val
	}
	break;
      case Unary:
	{
	  s32 sdata = _Unary32ToInt32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, state); //overwrite val
	}
	break;
      case Bool:
	{
	  s32 sdata = _Bool32ToInt32(data, valbitsize, bitsize);
	  val = UlamValue::makeImmediate(typidx, (u32) sdata, state); //overwrite val
	}
	break;
      case Void:
      default:
	//std::cerr << "UlamTypeInt (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn;
  } //end cast


  void UlamTypeInt::genCodeAfterReadingIntoATmpVar(File * fp, UlamValue & uvpass, CompilerState& state)
  {
    assert(uvpass.getUlamValueTypeIdx() == Ptr);

    //bypass sign extend when entire array is read
    if(!isScalar() && uvpass.getPtrLen() == (s32) getTotalBitSize())
      return;

    if(!isScalar())
      return genCodeAfterReadingArrayItemIntoATmpVar(fp, uvpass, state);

    UTI uti = getUlamTypeIndex();
    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    s32 tmpVarCastNum = state.getNextTmpVarNumber(); 
    s32 totWords = getTotalWordSize();
    
    state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString(&state).c_str()); //i.e. s32, s64
    fp->write(" ");
    fp->write(state.getTmpVarAsString(uti, tmpVarCastNum).c_str());
    fp->write(" = ");
    
    // write the cast method (e.g. _SignExtend32)
    //fp->write(castMethodForCodeGen(uti, state).c_str());
    fp->write("_SignExtend");
    fp->write_decimal(totWords);
    
    fp->write("(");
    fp->write(state.getTmpVarAsString(uti, tmpVarNum).c_str());
    fp->write(", ");
    fp->write_decimal(getTotalBitSize());
    fp->write(")");
    fp->write(";\n");
    
    UTI newuti = uti;
#if 0
    UTI newuti = Int;
    
    if(totWords > MAXBITSPERINT)
      {
	UlamKeyTypeSignature ikey(state.m_pool.getIndexForDataString("Int"), totWords);
	newuti = state.makeUlamType(ikey, Int);
      }
#endif
    
    uvpass = UlamValue::makePtr(tmpVarCastNum, TMPREGISTER, newuti, getPackable(), state, 0, uvpass.getPtrNameId()); //POS 0 rightjustified (atom-based); pass along name id

  } //genCodeAfterReadingIntoATmpVar
  

  // private helper
  void UlamTypeInt::genCodeAfterReadingArrayItemIntoATmpVar(File * fp, UlamValue & uvpass, CompilerState& state)
  {
    UTI uti = getUlamTypeIndex();
    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    s32 tmpVarCastNum = state.getNextTmpVarNumber(); 
    s32 itemWords = getItemWordSize();
    
    state.indent(fp);
    fp->write("const ");
    fp->write(getArrayItemTmpStorageTypeAsString(&state).c_str()); //i.e. s32, s64
    fp->write(" ");
    fp->write(state.getTmpVarAsString(uti, tmpVarCastNum).c_str());
    fp->write(" = ");
    
    // write the cast method (e.g. _SignExtend32)
    //fp->write(castMethodForCodeGen(uti, state).c_str());
    fp->write("_SignExtend");
    fp->write_decimal(itemWords);
    
    fp->write("(");
    fp->write(state.getTmpVarAsString(uti, tmpVarNum).c_str());
    fp->write(", ");
    fp->write_decimal(getBitSize());
    fp->write(")");
    fp->write(";\n");
    
    UTI newuti = uti;
#if 0
    UTI newuti = Int;
    
    if(itemWords > MAXBITSPERINT)
      {
	UlamKeyTypeSignature ikey(state.m_pool.getIndexForDataString("Int"), itemWords);
	newuti = state.makeUlamType(ikey, Int);
      }
#endif
    
    uvpass = UlamValue::makePtr(tmpVarCastNum, TMPREGISTER, newuti, getPackable(), state, 0, uvpass.getPtrNameId()); //POS 0 rightjustified (atom-based); pass along name id

  } //genCodeAfterReadingArrayItemIntoATmpVar


  void UlamTypeInt::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%d", (s32) data);
    else
      sprintf(valstr,"%c%d", prefix, (s32) data);
  }
  
} //end MFM
