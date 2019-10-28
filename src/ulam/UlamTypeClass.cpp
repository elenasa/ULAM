#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClass.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClass::UlamTypeClass(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state), m_customArray(false), m_bitsizeAsBaseClass(UNKNOWNSIZE)
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
  }

  bool UlamTypeClass::isPrimitiveType()
  {
    return false;
  }

  s32 UlamTypeClass::getBitsizeAsBaseClass()
  {
    assert(isScalar());
    assert(!isReference()); //t41366
    //assert(m_bitsizeAsBaseClass >= 0); //t3318
    return m_bitsizeAsBaseClass;
  }

  void UlamTypeClass::setBitsizeAsBaseClass(s32 bs)
  {
    assert(isScalar());
    m_bitsizeAsBaseClass = bs;
  }

  bool UlamTypeClass::cast(UlamValue & val, UTI typidx)
  {
    m_state.abortShouldntGetHere(); //neither element nor quark
    return false;
  }

  FORECAST UlamTypeClass::safeCast(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;
    UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
    if( fmut == this)
      return CAST_CLEAR; //same class, quark or element
    UTI fmderef = m_state.getUlamTypeAsDeref(typidx); //e.g. ALT_ARRAYITEM
    u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx(); //our scalar "new"
    if(m_state.isClassASubclassOf(fmderef, cuti))
      return CAST_CLEAR; //(up) casting to a super class

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
	bool isfmref = fmut->isAltRefType();
	UTI fmderef = m_state.getUlamTypeAsDeref(typidx);
	u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx(); //our scalar as nonref "new"
	if(m_state.isClassASubclassOf(cuti, fmderef))
	  {
	    //even though it may fail at runtime:
	    //(down)casting fm super to sub..only if fm-ref && to-ref
	    //	    if(!isfmref || !isAltRefType())
	    if(!isfmref) //only if fm-ref???
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
   if(getUlamClassType() == UC_UNSEEN)
      {
	m_state.abortShouldntGetHere(); //UC_UNSEEN
      }
   return UlamType::getUlamTypeEnumCodeChar(getUlamTypeEnum());
  }

  const std::string UlamTypeClass::getUlamTypeMangledType()
  {
    // e.g. parsing overloaded functions, may not be complete.
    std::ostringstream mangled;
    s32 bitsize = getBitSize();
    s32 arraysize = getArraySize();

    if(isReference()) //not isAltRefType
      mangled << "r";

    if(arraysize > 0)
      mangled << ToLeximitedNumber(arraysize);
    else if(arraysize == 0)
      mangled << ToLeximitedNumber(-1); //distinct from scalar
    else
      mangled << 10;

    if(bitsize > 0)
      mangled << ToLeximitedNumber(bitsize);
    else
      mangled << 10;

    mangled << m_state.getDataAsStringMangled(getUlamTypeNameId()).c_str();
    //without types and values of args!!
    return mangled.str();
  } //getUlamTypeMangledType

  const std::string UlamTypeClass::getUlamTypeMangledName()
  {
    std::ostringstream mangledclassname;
    mangledclassname << UlamType::getUlamTypeMangledName(); //includes Uprefix

    //or numberOfParameters followed by each digi-encoded: mangled type and value
    u32 id = getUlamTypeNameId();
    UTI cuti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(id);
    mangledclassname << cnsym->formatAnInstancesArgValuesAsAString(cuti);
    return mangledclassname.str();
  } //getUlamTypeMangledName

  const std::string UlamTypeClass::getUlamTypeUPrefix()
  {
    //scalar or array Sat Sep 30 16:05:48 2017
    return "U?_"; //UC_UNSEEN:
  } //getUlamTypeUPrefixx

  const std::string UlamTypeClass::getUlamTypeNameBrief()
  {
    u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    //except for UlamType::checkArrayCast error message (e.g. t3668, t3814)
    return getUlamTypeClassNameBrief(cuti); //when we don't know the uti (ok for non templated)
  } //getUlamTypeNameBrief

  const std::string UlamTypeClass::getUlamTypeClassNameBrief(UTI cuti)
  {
    //note: any "[arraysize]" comes with variable name, not class type (like C decl).
    bool isref = (getReferenceType() != ALT_NOT);
    UTI kuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    UTI uti = isref ? kuti : cuti;

#if 0
    //for DEBUGG ONLY!!
    UTI aliasuti;
    if(m_state.findaUTIAlias(cuti, aliasuti))
      {
	//when array or ref, the kuti is the scalar/deref uti, the aliasuti is same as cuti;
	//stubs get here via printPostfix on template classes;
	//sanity:t3363(stub),t3757,t3806 (stub),3814 (array)
	assert(isref || !isScalar() || m_state.isClassAStub(cuti) || (aliasuti == kuti));
      }
    else
      {
	//debug:t3862,t41209,t3143,t3327
	assert(isref || !isScalar() || !isComplete() || (cuti == kuti));
      }
#endif

    std::ostringstream namestr;
    namestr << m_key.getUlamKeyTypeSignatureName(&m_state).c_str();

    u32 id = getUlamTypeNameId();
    SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(id);
    if(cnsym && cnsym->isClassTemplate())
      namestr << ((SymbolClassNameTemplate *) cnsym)->formatAnInstancesArgValuesAsCommaDelimitedString(uti).c_str();

     //note: any "[arraysize]" comes with variable name, not class type (like C decl).
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

  PACKFIT UlamTypeClass::getPackable()
  {
    //if(isCustomArray())
    //  return m_state.getUlamTypeByIndex(getCustomArrayType())->getPackable(); //t41143
    return UlamType::getPackable();
  } //getPackable

  const std::string UlamTypeClass::readMethodForCodeGen()
  {
    return "Illiterate";
  }

  const std::string UlamTypeClass::writeMethodForCodeGen()
  {
    return "Illiterate";
  }

  bool UlamTypeClass::needsImmediateType()
  {
    return false;
  }

  const std::string UlamTypeClass::getUlamTypeImmediateMangledName()
  {
    if(needsImmediateType() || isAltRefType())
      {
	return UlamType::getUlamTypeImmediateMangledName();
      }
    return "T";
  } //getUlamTypeImmediateMangledName

  const std::string UlamTypeClass::getUlamTypeImmediateAutoMangledName()
  {
    assert(needsImmediateType() || isAltRefType());

    if(isAltRefType())
      {
	m_state.abortShouldntGetHere(); //use ImmediateMangledName
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
  }

  const std::string UlamTypeClass::getLocalStorageTypeAsString()
  {
    m_state.abortShouldntGetHere();
    return "Bloco";
  }

  TMPSTORAGE UlamTypeClass::getTmpStorageTypeForTmpVar()
  {
    return UlamType::getTmpStorageTypeForTmpVar();
  }

  const std::string UlamTypeClass::castMethodForCodeGen(UTI nodetype)
  {
    m_state.abortShouldntGetHere();
    return "_NoCast";
  } //castMethodForCodeGen

  void UlamTypeClass::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamTypeClass::genUlamTypeReadDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamTypeClass::genUlamTypeWriteDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
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
      return m_state.getCustomArrayGetMangledFunctionName(); //return a ref
    return UlamType::writeArrayItemMethodForCodeGen();
  }

  void UlamTypeClass::genUlamTypeMangledDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamTypeClass::genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamTypeClass::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

} //end MFM
