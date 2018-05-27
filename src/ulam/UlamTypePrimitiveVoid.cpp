#include <iostream>
#include <stdio.h>
#include <string.h>
#include "UlamTypePrimitiveVoid.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypePrimitiveVoid::UlamTypePrimitiveVoid(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypePrimitive(key, state)  {}

   ULAMTYPE UlamTypePrimitiveVoid::getUlamTypeEnum()
   {
     return Void;
   }

  const std::string UlamTypePrimitiveVoid::getUlamTypeMangledName()
  {
    return "void";
  }

  const std::string UlamTypePrimitiveVoid::getUlamTypeImmediateMangledName()
  {
    return getLocalStorageTypeAsString(); //"void";
  }

  bool UlamTypePrimitiveVoid::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypePrimitiveVoid::getLocalStorageTypeAsString()
  {
    return "void";
  }

  const std::string UlamTypePrimitiveVoid::getTmpStorageTypeAsString()
  {
    return "void";
  }

  bool UlamTypePrimitiveVoid::isMinMaxAllowed()
  {
    return false;
  }

  void UlamTypePrimitiveVoid::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    m_state.abortShouldntGetHere();
    sprintf(valstr,"%s", getUlamTypeName().c_str());
  }

  void UlamTypePrimitiveVoid::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    m_state.abortShouldntGetHere();
    sprintf(valstr,"%s", getUlamTypeName().c_str());
  }

  s32 UlamTypePrimitiveVoid::getDataAsCs32(const u32 data)
  {
    m_state.abortShouldntGetHere();
    return (s32) data;
  }

  u32 UlamTypePrimitiveVoid::getDataAsCu32(const u32 data)
  {
    m_state.abortShouldntGetHere();
    return data;
  }

  s64 UlamTypePrimitiveVoid::getDataAsCs64(const u64 data)
  {
    m_state.abortShouldntGetHere();
    return (s64) data;
  }

  u64 UlamTypePrimitiveVoid::getDataAsCu64(const u64 data)
  {
    m_state.abortShouldntGetHere();
    return data;
  }

  bool UlamTypePrimitiveVoid::castTo32(UlamValue & val, UTI typidx)
  {
    return false;
  } //castTo32

  bool UlamTypePrimitiveVoid::castTo64(UlamValue & val, UTI typidx)
  {
    return false;
  }

  //anything can be cast to a void (not the reverse)
  bool UlamTypePrimitiveVoid::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    UTI valtypidx = val.getUlamValueTypeIdx(); //from type

    if(UlamTypePrimitive::safeCast(valtypidx) != CAST_CLEAR) //bad|hazy
      return false;

    ULAMTYPE valtypEnum = m_state.getUlamTypeByIndex(valtypidx)->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Void:
      case Int:
      case Unsigned:
      case Unary:
      case Bool:
      case Bits:
      case UAtom:
      case Class:
      case String:
	val = UlamValue::makeImmediate(typidx, 0, m_state); //overwrite val, no data
	break;
      default:
	//std::cerr << "UlamTypePrimitiveVoid (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };

    return brtn;
  } //end cast

  FORECAST UlamTypePrimitiveVoid::safeCast(UTI typidx)
  {
    FORECAST scr = UlamTypePrimitive::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    bool brtn = true;
    UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
    ULAMTYPE valtypEnum = fmut->getUlamTypeEnum();
    switch(valtypEnum)
      {
      case Void:
      case Unsigned:
      case Unary:
      case Int:
      case Bool:
      case Bits:
      case UAtom:
      case Class:
      case String:
	brtn = true; //anything casts to void ok
	break;
      default:
	m_state.abortUndefinedUlamType();
	//std::cerr << "UlamTypePrimitiveVoid (cast) error! Value Type was: " << valtypidx << std::endl;
	brtn = false;
      };
    return brtn ? CAST_CLEAR : CAST_BAD;
  } //safeCast

  s32 UlamTypePrimitiveVoid::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
  {
    u32 tobitsize = UNKNOWNSIZE;
    switch(tobUT)
      {
      case Bool:
      case Unsigned:
      case Unary:
      case Int:
      case Bits:
      case String:
	break;
      case Void:
	tobitsize = 0;
	break;
      case UAtom:
      case Class:
	break;
      default:
	m_state.abortUndefinedUlamType();
	//std::cerr << "UlamTypePrimitiveVoid (convertTo) error! " << tobUT << std::endl;
      };
    return tobitsize;
  } //bitsizeToConvertTypeTo

  void UlamTypePrimitiveVoid::genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp)
  {
    return;
  } //genUlamTypeMangledImmediateModelParameterDefinitionForC

  void UlamTypePrimitiveVoid::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
    return;
  }

 void UlamTypePrimitiveVoid::genUlamTypeReadDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
    return;
  }

 void UlamTypePrimitiveVoid::genUlamTypeWriteDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
    return;
  }

  const std::string UlamTypePrimitiveVoid::readArrayItemMethodForCodeGen()
  {
    return "voidreadarrayitemerror";
  }

  const std::string UlamTypePrimitiveVoid::writeArrayItemMethodForCodeGen()
  {
    return "voidwritearrayitemerror";
  }

 void UlamTypePrimitiveVoid::genUlamTypeMangledDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
    return;
  }

 void UlamTypePrimitiveVoid::genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
    return;
  }

 void UlamTypePrimitiveVoid::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
    return;
  }
} //end MFM
