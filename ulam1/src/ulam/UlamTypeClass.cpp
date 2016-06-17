#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClass.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClass::UlamTypeClass(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state), m_customArray(false)
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
  }

   ULAMTYPE UlamTypeClass::getUlamTypeEnum()
   {
     return Class;
   }

  bool UlamTypeClass::isNumericType()
  {
    return false; //quark may have toInt()
  } //isNumericType

  bool UlamTypeClass::isPrimitiveType()
  {
    return false;
  }

  bool UlamTypeClass::cast(UlamValue & val, UTI typidx)
  {
    assert(0); //neither element nor quark
    return false;
  } //end cast

  FORECAST UlamTypeClass::safeCast(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;
    UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
    if( fmut == this)
      return CAST_CLEAR; //same class, quark or element
    else
      {
	UTI fmderef = m_state.getUlamTypeAsDeref(typidx); //e.g. ALT_AS, ALT_ARRAYITEM
	u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx(); //our scalar "new"
	if(m_state.isClassASubclassOf(fmderef, cuti))
	  return CAST_CLEAR; //(up) casting to a super class
	else
	  {
	    //e.g. array item to a ref of same type (cuti)
	    ULAMTYPECOMPARERESULTS cmpr1 = UlamType::compare(fmderef, cuti, m_state);
	    if(cmpr1 == UTIC_SAME)
	      return CAST_CLEAR;
	    if(cmpr1 == UTIC_DONTKNOW)
	      return CAST_HAZY;

	    if(m_state.isClassASubclassOf(fmderef, cuti))
	      return CAST_CLEAR; //(up) casting ref to a super class (may also be ref)

	    if(m_state.isClassASubclassOf(cuti, fmderef))
	      return CAST_BAD; //(downcast) requires explicit cast

	    //ref of this class, applies to entire arrays too
	    UTI anyUTI = Nouti;
	    AssertBool anyDefined = m_state.anyDefinedUTI(m_key, anyUTI);
	   assert(anyDefined);

	    ULAMTYPECOMPARERESULTS cmpr2 = m_state.isARefTypeOfUlamType(typidx, anyUTI);
	    if(cmpr2 == UTIC_SAME)
	      return CAST_CLEAR;
	    else if(cmpr2 == UTIC_DONTKNOW)
	      return CAST_HAZY;
	  }
      }
    return CAST_BAD; //e.g. (typidx == UAtom)
  } //safeCast

  FORECAST UlamTypeClass::explicitlyCastable(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx); //default, arrays checked
    if(scr == CAST_CLEAR)
      {
	UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
	ULAMTYPE fetyp = fmut->getUlamTypeEnum();
	//no casting from primitive to class; but from atom/atomref to class may be fine (e.g. t3733)
	if((fetyp != Class) && (fetyp != UAtom))
	  return CAST_BAD;
	else if(m_state.isAtom(typidx))
	  return CAST_CLEAR;

	//check when casting from class to class
	bool isfmref = fmut->isReference();
	UTI fmderef = m_state.getUlamTypeAsDeref(typidx);
	u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx(); //our scalar as nonref "new"
	if(m_state.isClassASubclassOf(cuti, fmderef))
	  {
	    //even though it may fail at runtime:
	    //(down)casting fm super to sub..only if fm-ref && to-ref
	    if(!isfmref || !isReference())
	      scr = CAST_BAD; //t3756, t3757
	  }
	else if(m_state.isClassASubclassOf(fmderef, cuti))
	  {
	    // cast fm sub to super, ok!
	  }
	else if(UlamType::compare(fmderef, cuti, m_state) != UTIC_SAME)
	  scr = CAST_BAD;
	//else what if array???
      }
    return scr;
  } //explicitlyCastable

  const char * UlamTypeClass::getUlamTypeAsSingleLowercaseLetter()
  {
    assert(0); //UC_UNSEEN
    return UlamType::getUlamTypeEnumCodeChar(getUlamTypeEnum());
  } //getUlamTypeAsSingleLowercaseLetter()

  const std::string UlamTypeClass::getUlamTypeMangledType()
  {
    // e.g. parsing overloaded functions, may not be complete.
    std::ostringstream mangled;
    s32 bitsize = getBitSize();
    s32 arraysize = getArraySize();

    if(isReference())
      mangled << "r";

    if(arraysize > 0)
      mangled << ToLeximitedNumber(arraysize);
    else
      mangled << 10;

    if(bitsize > 0)
      mangled << ToLeximitedNumber(bitsize);
    else
      mangled << 10;

    mangled << m_state.getDataAsStringMangled(m_key.getUlamKeyTypeSignatureNameId()).c_str();
    //without types and values of args!!
    return mangled.str();
  } //getUlamTypeMangledType

  const std::string UlamTypeClass::getUlamTypeMangledName()
  {
    std::ostringstream mangledclassname;
    mangledclassname << UlamType::getUlamTypeMangledName(); //includes Uprefix

    //or numberOfParameters followed by each digi-encoded: mangled type and value
    u32 id = m_key.getUlamKeyTypeSignatureNameId();
    UTI cuti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(id);
    mangledclassname << cnsym->formatAnInstancesArgValuesAsAString(cuti);
    return mangledclassname.str();
  } //getUlamTypeMangledName

  const std::string UlamTypeClass::getUlamTypeUPrefix()
  {
    if(getArraySize() > 0)
      return "Ut_";

    return "U?_"; //UC_UNSEEN:
  } //getUlamTypeUPrefix

  const std::string UlamTypeClass::getUlamTypeNameBrief()
  {
    std::ostringstream namestr;

    namestr << m_key.getUlamKeyTypeSignatureName(&m_state).c_str();

    u32 id = m_key.getUlamKeyTypeSignatureNameId();
    u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(id);
    if(cnsym && cnsym->isClassTemplate())
      namestr << ((SymbolClassNameTemplate *) cnsym)->formatAnInstancesArgValuesAsCommaDelimitedString(cuti).c_str();

    if(getReferenceType() != ALT_NOT)
      namestr << "&";
    return namestr.str();
  } //getUlamTypeNameBrief

  //see SymbolVariableDataMember printPostfix for recursive output
  void UlamTypeClass::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%d", data);
    else
      sprintf(valstr,"%c%d", prefix, data);
  }

  void UlamTypeClass::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%s", ToSignedDecimal(data).c_str());
    else
      sprintf(valstr,"%c%s", prefix, ToSignedDecimal(data).c_str());
  }

  ULAMCLASSTYPE UlamTypeClass::getUlamClassType()
  {
    return UC_UNSEEN;
  }

  bool UlamTypeClass::isCustomArray()
  {
    return m_customArray; //canonical, ignores ancestors
  }

  void UlamTypeClass::setCustomArray()
  {
    m_customArray = true;
  }

  UTI UlamTypeClass::getCustomArrayType()
  {
    u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    return m_state.getAClassCustomArrayType(cuti);
  } //getCustomArrayType

  u32 UlamTypeClass::getCustomArrayIndexTypeFor(Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    return m_state.getAClassCustomArrayIndexType(cuti, rnode, idxuti, hasHazyArgs);
  } //getCustomArrayIndexTypeFor

  bool UlamTypeClass::isHolder()
  {
    std::string name = m_key.getUlamKeyTypeSignatureName(&m_state);
    s32 c = name.at(0);
    return (!isalpha(c)); //id same as UTI number, out-of-band
  }

  bool UlamTypeClass::isComplete()
  {
    if(isHolder())
      return false;

    if(getUlamClassType() == UC_UNSEEN)
      return false; //forgotten?

    return UlamType::isComplete();
  }

  const std::string UlamTypeClass::readMethodForCodeGen()
  {
    return "Illiterate";
  } //readMethodForCodeGen

  const std::string UlamTypeClass::writeMethodForCodeGen()
  {
    return "Illiterate";
  } //writeMethodForCodeGen

  bool UlamTypeClass::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypeClass::getUlamTypeImmediateMangledName()
  {
    if(needsImmediateType() || isReference())
      {
	return UlamType::getUlamTypeImmediateMangledName();
      }
    return "T";
  } //getUlamTypeImmediateMangledName

  const std::string UlamTypeClass::getUlamTypeImmediateAutoMangledName()
  {
    assert(needsImmediateType() || isReference());

    if(isReference())
      {
	assert(0); //use ImmediateMangledName
	return getUlamTypeImmediateMangledName();
      }

    //get name of a referenced UTI for this type;
    UTI myuti = Nav;
    AssertBool anyDefined = m_state.anyDefinedUTI(m_key, myuti);
    assert(anyDefined);

    UTI asRefType = m_state.getUlamTypeAsRef(myuti);
    UlamType * asRef = m_state.getUlamTypeByIndex(asRefType);
    return asRef->getUlamTypeImmediateMangledName();
  } //getUlamTypeImmediateAutoMangledName

  const std::string UlamTypeClass::getArrayItemTmpStorageTypeAsString()
  {
    if(!isScalar())
      return UlamType::getTmpStorageTypeAsString(getItemWordSize()); //return its scalar tmp storage type

    return m_state.getUlamTypeByIndex(getCustomArrayType())->getTmpStorageTypeAsString();
  } //getArrayItemTmpStorageTypeAsString

  const std::string UlamTypeClass::getLocalStorageTypeAsString()
  {
    assert(0);
    return "Bloco";
  } //getLocalStorageTypeAsString

  TMPSTORAGE UlamTypeClass::getTmpStorageTypeForTmpVar()
  {
    return UlamType::getTmpStorageTypeForTmpVar();
  } //getTmpStorageTypeForTmpVar

  const std::string UlamTypeClass::castMethodForCodeGen(UTI nodetype)
  {
    assert(0);
    return "_NoCast";
  } //castMethodForCodeGen

  void UlamTypeClass::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    assert(0);
  }

  void UlamTypeClass::genUlamTypeReadDefinitionForC(File * fp)
  {
    assert(0);
  }

  void UlamTypeClass::genUlamTypeWriteDefinitionForC(File * fp)
  {
    assert(0);
  }

  const std::string UlamTypeClass::readArrayItemMethodForCodeGen()
  {
    if(isCustomArray())
      return m_state.getCustomArrayGetMangledFunctionName();
    return UlamType::readArrayItemMethodForCodeGen();
  }

  const std::string UlamTypeClass::writeArrayItemMethodForCodeGen()
  {
    if(isCustomArray())
      return m_state.getCustomArraySetMangledFunctionName();
    return UlamType::writeArrayItemMethodForCodeGen();
  }

  void UlamTypeClass::genUlamTypeMangledDefinitionForC(File * fp)
  {
    assert(0);
  }

  void UlamTypeClass::genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp)
  {
    assert(0);
  }

  void UlamTypeClass::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    assert(0);
  }

} //end MFM
