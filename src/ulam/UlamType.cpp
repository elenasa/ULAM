#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sstream>
#include "UlamType.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

#define XY(a,b,c,d) b,

  static const char * utype_string[] = {
#include "UlamType.inc"
  };

#undef XY


#define XY(a,b,c,d) d,

  static const char * utype_primitivecode[] = {
#include "UlamType.inc"
  };

#undef XY


  UlamType::UlamType(const UlamKeyTypeSignature key, CompilerState & state) : m_key(key), m_state(state), m_wordLengthTotal(0), m_wordLengthItem(0)  {}

  UlamType * UlamType::getUlamType()
  {
    return this;
  }

  const std::string UlamType::getUlamTypeName()
  {
    return m_key.getUlamKeyTypeSignatureAsString(&m_state);
    // REMINDER!! error due to disappearing string:
    //    return m_key.getUlamKeyTypeSignatureAsString().c_str();
  }

  const std::string UlamType::getUlamTypeNameBrief()
  {
    return m_key.getUlamKeyTypeSignatureNameAndSize(&m_state);
  }

  const std::string UlamType::getUlamTypeClassNameBrief(UTI cuti)
  {
    m_state.abortUndefinedUlamClassType(); //only for classes
    return "iamnotaclass"; //for compiler
  }

  const std::string UlamType::getUlamTypeNameOnly()
  {
    return m_key.getUlamKeyTypeSignatureName(&m_state);
  }

  u32 UlamType::getUlamTypeNameId()
  {
    return m_key.getUlamKeyTypeSignatureNameId();
  } //helper

   UlamKeyTypeSignature UlamType::getUlamKeyTypeSignature()
  {
    return m_key;
  }

  bool UlamType::isNumericType()
  {
    return false;
  }

  bool UlamType::isPrimitiveType()
  {
    return false;
  }

  bool UlamType::cast(UlamValue & val, UTI typidx)
  {
    m_state.abortShouldntGetHere();
    //std::cerr << "UlamType (cast) error! " << std::endl;
    return false;
  }

  FORECAST UlamType::safeCast(UTI typidx)
  {
    // initial tests for completeness and scalars
    if(!isComplete() || !m_state.isComplete(typidx))
      {
	std::ostringstream msg;
	msg << "Casting UNKNOWN sizes; " << getBitSize();
	msg << ", Value Type and size was: " << typidx << "," << m_state.getBitSize(typidx);
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	return CAST_HAZY;  //includes Navs & Hzy's
      }

    //let packable arrays of same size pass...
    if(!checkArrayCast(typidx))
      return CAST_BAD;

    if((m_state.getReferenceType(typidx)==ALT_CONSTREF) && (getReferenceType()==ALT_REF))
      return CAST_BAD; //bad cast from const ref to non-const ref

    return CAST_CLEAR;
  } //safeCast

  FORECAST UlamType::explicitlyCastable(UTI typidx)
  {
    m_state.abortShouldntGetHere(); //must be overridden (pure virtual unhappy)
    return CAST_BAD;
  }

  bool UlamType::checkArrayCast(UTI typidx)
  {
    if(isScalar() && m_state.isScalar(typidx))
      return true; //not arrays, ok

    bool bOK = true;
    s32 arraysize = getArraySize();
    s32 varraysize = m_state.getArraySize(typidx);
    if(arraysize != varraysize)
      {
	std::ostringstream msg;
	msg << "Casting different Array sizes: ";
	msg << m_state.getUlamTypeNameBriefByIndex(typidx).c_str();
	msg << " TO " ;
	msg << getUlamTypeNameBrief().c_str();
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	bOK = false;
      }
    else
      {
	std::ostringstream msg;
	msg << "Casting (nonScalar) Array: ";
	msg << m_state.getUlamTypeNameBriefByIndex(typidx).c_str();
	msg << " TO " ;
	msg << getUlamTypeNameBrief().c_str();
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }
    return bOK;
  } //checkArrayCast

  bool UlamType::checkReferenceCast(UTI typidx)
  {
    // applies to ALT_REF or ALT_CONSTREF.
    //both complete; typidx is a reference
    UlamKeyTypeSignature key1 = getUlamKeyTypeSignature();
    UlamKeyTypeSignature key2 = m_state.getUlamKeyTypeSignatureByIndex(typidx);
    ALT alt1 = key1.getUlamKeyTypeSignatureReferenceType();
    ALT alt2 = key2.getUlamKeyTypeSignatureReferenceType();

    if(key1.getUlamKeyTypeSignatureNameId() != key2.getUlamKeyTypeSignatureNameId())
      return false;
    if(key1.getUlamKeyTypeSignatureArraySize() != key2.getUlamKeyTypeSignatureArraySize())
      return false;

    if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
      return false; //t3963

    //skip rest in the case of array item, continue with usual size fit;
    //must check (t3884, t3651, t3653, t3817, t41071,3 t41100)
    if(alt1 == ALT_ARRAYITEM || alt2 == ALT_ARRAYITEM)
      return true;

    if(key1.getUlamKeyTypeSignatureBitSize() != key2.getUlamKeyTypeSignatureBitSize())
	  return false;

    if((alt2 == ALT_CONSTREF) && (alt1 == ALT_REF))
      return false;

    return true; //keys the same, except for reference type
  } //checkReferenceCast

  void UlamType::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    sprintf(valstr,"%s", getUlamTypeName().c_str());
  }

  void UlamType::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    sprintf(valstr,"%s", getUlamTypeName().c_str());
  }

  s32 UlamType::getDataAsCs32(const u32 data)
  {
    m_state.abortShouldntGetHere();
    return (s32) data;
  }

  u32 UlamType::getDataAsCu32(const u32 data)
  {
    m_state.abortShouldntGetHere();
    return data;
  }

  s64 UlamType::getDataAsCs64(const u64 data)
  {
    m_state.abortShouldntGetHere();
    return (s64) data;
  }

  u64 UlamType::getDataAsCu64(const u64 data)
  {
    m_state.abortShouldntGetHere();
    return data;
  }

  s32 UlamType::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
  {
    return UNKNOWNSIZE; //atom, class, nav, ptr, holder
  }

  const std::string UlamType::getUlamTypeMangledType()
  {
    // e.g. parsing overloaded functions, may not be complete.
    std::ostringstream mangled;
    s32 bitsize = getBitSize();
    s32 arraysize = getArraySize();

    if(isReference()) //includes ALT_ARRAYITEM (t3147); not isAltRefType (t3114)
      mangled << "r"; //e.g. t3114

    if(arraysize > 0)
      mangled << ToLeximitedNumber(arraysize);
    //    else if(arraysize == 0)
    //  mangled << ToLeximitedNumber(-1); //distinct from scalar
    else
      mangled << 10; //scalar NONARRAYSIZE

    if(bitsize > 0)
      mangled << ToLeximitedNumber(bitsize);
    else
      mangled << 10;

    std::string ecode(UlamType::getUlamTypeEnumCodeChar(getUlamTypeEnum()));
    mangled << ToLeximited(ecode).c_str();

    return mangled.str();
  } //getUlamTypeMangledType

  const std::string UlamType::getUlamTypeMangledName()
  {
    // e.g. parsing overloaded functions, may not be complete.
    std::ostringstream mangled;
    mangled << getUlamTypeUPrefix().c_str();
    mangled << getUlamTypeMangledType();
    return mangled.str();
  } //getUlamTypeMangledName

  const std::string UlamType::getUlamTypeUPrefix()
  {

    return "Ut_";
  }

  bool UlamType::needsImmediateType()
  {
    return false; //isComplete();
  }

  const std::string UlamType::getUlamTypeImmediateMangledName()
  {
    std::ostringstream imangled;
    imangled << "Ui_" << getUlamTypeMangledName();
    return  imangled.str();
  }

  const std::string UlamType::getUlamTypeImmediateAutoMangledName()
  {
    assert(needsImmediateType());

    if(isAltRefType())
      {
	m_state.abortShouldntGetHere();
	return getUlamTypeImmediateMangledName();
      }

    //same as non-ref except for the 'r'
    std::ostringstream  automn;
    automn << "Ui_";
    automn << getUlamTypeUPrefix().c_str();
    automn << "r";
    automn << getUlamTypeMangledType();
    return automn.str();
  } //getUlamTypeImmediateAutoMangledName

  const std::string UlamType::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName();
    ctype << "<EC>"; //name of struct w typedef(bf) and storage(bv);
    return ctype.str();
  } //getLocalStorageTypeAsString

  const std::string UlamType::getImmediateModelParameterStorageTypeAsString()
  {
    m_state.abortShouldntGetHere();
    std::ostringstream mpimangled;
    //substitutes Up_ for Ut_ for model parameter immediate
    mpimangled << "Ui_Up_" << getUlamTypeMangledType();
    return mpimangled.str();
  } //getImmediateModelParameterStorageTypeAsString

  const std::string UlamType::getArrayItemTmpStorageTypeAsString()
  {
    return getTmpStorageTypeAsString(getItemWordSize());
  }

  const std::string UlamType::getTmpStorageTypeAsString()
  {
    return getTmpStorageTypeAsString(getTotalWordSize());
  }

  const std::string UlamType::getTmpStorageTypeAsString(s32 sizebyints)
  {
    std::string ctype;
    switch(sizebyints)
      {
      case 0: //e.g. empty quarks
      case 32:
	ctype = "u32";
	break;
      case 64:
	ctype = "u64";
	break;
      case 96:
	ctype = "BV96";
	break;
      default:
	{
	  assert(!isScalar());
	  //ctype = getTmpStorageTypeAsString(getItemWordSize()); //u32, u64 (inf loop)
	  std::ostringstream cstr;
	  if(sizebyints == (s32) getItemWordSize())
	    cstr << "BitVector<" << getBitSize() << ">";
	  else
	    cstr << "BitVector<" << getTotalBitSize() << ">"; //entire array
	  ctype = cstr.str();
	}
      };
    return ctype;
  } //getTmpStorageTypeAsString

  TMPSTORAGE UlamType::getTmpStorageTypeForTmpVar()
  {
    //references are read into the same underlying bitstorage as non-refs.
    TMPSTORAGE rtnStgType = TMPREGISTER;
    u32 sizebyints = getTotalWordSize();
    if(sizebyints > 64)
      rtnStgType = TMPTBV;
    return rtnStgType; //unpacked arrays reflected in tmp name.
  }

  const char * UlamType::getUlamTypeAsSingleLowercaseLetter()
  {
    return UlamType::getUlamTypeEnumCodeChar(getUlamTypeEnum());
  } //getUlamTypeAsSingleLowercaseLetter

  const std::string UlamType::castMethodForCodeGen(UTI nodetype)
  {
    std::ostringstream rtnMethod;
    UlamType * nut = m_state.getUlamTypeByIndex(nodetype);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    u32 sizeByIntBitsToBe = getTotalWordSize();
    u32 sizeByIntBits = nut->getTotalWordSize();

    if(sizeByIntBitsToBe != sizeByIntBits)
      {
	std::ostringstream msg;
	msg << "Casting different word sizes; " << sizeByIntBits;
	msg << ", Value Type and size was: " << nut->getUlamTypeName().c_str();
	msg << ", to be: " << sizeByIntBitsToBe << " for type: ";
	msg << getUlamTypeName().c_str();
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);

	//use the larger word size
	if(sizeByIntBitsToBe < sizeByIntBits) //downcast using larger
	  sizeByIntBitsToBe = sizeByIntBits;
	else
	  sizeByIntBits = sizeByIntBitsToBe;
      }
    rtnMethod << "_" << nut->getUlamTypeNameOnly().c_str() << sizeByIntBits << "To";
    rtnMethod << getUlamTypeNameOnly().c_str() << sizeByIntBitsToBe;
    return rtnMethod.str();
  } //castMethodForCodeGen

  void UlamType::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere(); //see UlamTypePrimitive
  }

  void UlamType::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamType::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  const char * UlamType::getUlamTypeEnumCodeChar(ULAMTYPE etype)
  {
    return utype_primitivecode[etype]; //static method
  }

  const char * UlamType::getUlamTypeEnumAsString(ULAMTYPE etype)
  {
    return utype_string[etype]; //static method
  }

  ULAMTYPE UlamType::getEnumFromUlamTypeString(const char * typestr)
  {
    ULAMTYPE rtnUT = Nav;
    for(u32 i = 0; i < LASTTYPE; i++)
      {
	if(strcmp(utype_string[i], typestr) == 0)
	  {
	    rtnUT = (ULAMTYPE) i;
	    break;
	  }
      }
    return rtnUT; //static method
  } //getEnumFromUlamTypeString

  ULAMCLASSTYPE UlamType::getUlamClassType()
  {
    return UC_ERROR;
  }

  bool UlamType::isScalar()
  {
    return (m_key.getUlamKeyTypeSignatureArraySize() == NONARRAYSIZE);
  }

  bool UlamType::isCustomArray()
  {
    return false;
  }

  s32 UlamType::getArraySize()
  {
    return m_key.getUlamKeyTypeSignatureArraySize(); //could be negative "uknown", or scalar
  }

  s32 UlamType::getBitSize()
  {
    return m_key.getUlamKeyTypeSignatureBitSize(); //could be negative "unknown"
  }

  u32 UlamType::getTotalBitSize()
  {
    s32 arraysize = getArraySize();
    arraysize = (arraysize > NONARRAYSIZE ? arraysize : 1);

    s32 bitsize = getBitSize();
    bitsize = (bitsize != UNKNOWNSIZE ? bitsize : 0);
    return bitsize * arraysize; // >= 0
  }

  u32 UlamType::getSizeofUlamType()
  {
    return getTotalBitSize(); //by default
  }

  s32 UlamType::getBitsizeAsBaseClass()
  {
    m_state.abortShouldntGetHere();
    return UNKNOWNSIZE;
  }

  void UlamType::setBitsizeAsBaseClass(s32 bs)
  {
    m_state.abortShouldntGetHere(); //only classes
  }

  ALT UlamType::getReferenceType()
  {
    return m_key.getUlamKeyTypeSignatureReferenceType();
  }

  bool UlamType::isReference()
  {
    //yes, all of these ALT types are treated as references in gencode.
    ALT alt = m_key.getUlamKeyTypeSignatureReferenceType();
    return (alt == ALT_AS) || (alt == ALT_REF) || (alt == ALT_ARRAYITEM) || (alt == ALT_CONSTREF);
  }

  bool UlamType::isAltRefType()
  {
    ALT alt = m_key.getUlamKeyTypeSignatureReferenceType();
    return (alt == ALT_REF) || (alt == ALT_CONSTREF);
  }

  bool UlamType::isHolder()
  {
    return false;
  }

  bool UlamType::isComplete()
  {
    return !(m_key.getUlamKeyTypeSignatureBitSize() <= UNKNOWNSIZE || getArraySize() == UNKNOWNSIZE);
  }

  ULAMTYPECOMPARERESULTS UlamType::compare(UTI u1, UTI u2, CompilerState& state)  //static
  {
    assert((u1 != Nouti) && (u2 != Nouti));

    if(u1 == u2) return UTIC_SAME; //short-circuit

    if((u1 == Nav) || (u2 == Nav)) return UTIC_NOTSAME;

    if((u1 == Hzy) || (u2 == Hzy)) return UTIC_DONTKNOW;

    UlamType * ut1 = state.getUlamTypeByIndex(u1);
    UlamType * ut2 = state.getUlamTypeByIndex(u2);
    ULAMCLASSTYPE ct1 = ut1->getUlamClassType();
    ULAMCLASSTYPE ct2 = ut2->getUlamClassType();
    UlamKeyTypeSignature key1 = ut1->getUlamKeyTypeSignature();
    UlamKeyTypeSignature key2 = ut2->getUlamKeyTypeSignature();

    // Given Class Arguments: we may end up with different sizes given different
    // argument values which may or may not be known while parsing!
    // t.f. classes are much more like the primitive ulamtypes now.
    // Was The Case: classes with unknown bitsizes are essentially as complete
    // as they can be during parse time; and will have the same UTIs.
    if(!ut1->isComplete())
      {
	if(ct1 == UC_NOTACLASS || ut1->getArraySize() == UNKNOWNSIZE)
	  return UTIC_DONTKNOW;

	//class with known arraysize(scalar or o.w.); no more Nav ids.
	if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
	  return UTIC_DONTKNOW;
      }

    if(!ut2->isComplete())
      {
	if(ct2 == UC_NOTACLASS || ut2->getArraySize() == UNKNOWNSIZE)
	  return UTIC_DONTKNOW;

	//class with known arraysize(scalar or o.w.); no more Nav ids.
	if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
	  return UTIC_DONTKNOW;
      }

    //both complete!
    //assert both key and ptr are either both equal or not equal; not different ('!^' eq '==')
    assert((key1 == key2) == (ut1 == ut2));
    return (ut1 == ut2) ? UTIC_SAME : UTIC_NOTSAME;
  } //compare (static)

  ULAMTYPECOMPARERESULTS UlamType::compareWithWildArrayItemALTKey(UTI u1, UTI u2, CompilerState& state)  //static
  {
    assert((u1 != Nouti) && (u2 != Nouti));

    if(u1 == u2) return UTIC_SAME; //short-circuit

    if((u1 == Nav) || (u2 == Nav)) return UTIC_NOTSAME;

    if((u1 == Hzy) || (u2 == Hzy)) return UTIC_DONTKNOW;

    UlamType * ut1 = state.getUlamTypeByIndex(u1);
    UlamType * ut2 = state.getUlamTypeByIndex(u2);
    ULAMCLASSTYPE ct1 = ut1->getUlamClassType();
    ULAMCLASSTYPE ct2 = ut2->getUlamClassType();
    UlamKeyTypeSignature key1 = ut1->getUlamKeyTypeSignature();
    UlamKeyTypeSignature key2 = ut2->getUlamKeyTypeSignature();

    // Given Class Arguments: we may end up with different sizes given different
    // argument values which may or may not be known while parsing!
    // t.f. classes are much more like the primitive ulamtypes now.
    // Was The Case: classes with unknown bitsizes are essentially as complete
    // as they can be during parse time; and will have the same UTIs.
    if(!ut1->isComplete())
      {
	if(ct1 == UC_NOTACLASS || ut1->getArraySize() == UNKNOWNSIZE)
	  return UTIC_DONTKNOW;

	//class with known arraysize(scalar or o.w.); no more Nav ids.
	if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
	  return UTIC_DONTKNOW;
      }

    if(!ut2->isComplete())
      {
	if(ct2 == UC_NOTACLASS || ut2->getArraySize() == UNKNOWNSIZE)
	  return UTIC_DONTKNOW;

	//class with known arraysize(scalar or o.w.); no more Nav ids.
	if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
	  return UTIC_DONTKNOW;
      }

    //both complete!
    //keys may have different reference types in the case of array items
    if(key1.getUlamKeyTypeSignatureNameId() != key2.getUlamKeyTypeSignatureNameId())
       return UTIC_NOTSAME;

    if(key1.getUlamKeyTypeSignatureArraySize() != key2.getUlamKeyTypeSignatureArraySize())
      return UTIC_NOTSAME;

    if(key1.getUlamKeyTypeSignatureBitSize() != key2.getUlamKeyTypeSignatureBitSize())
      return UTIC_NOTSAME;

    if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
      return UTIC_NOTSAME; //t3412

    ALT alt1 = key1.getUlamKeyTypeSignatureReferenceType();
    ALT alt2 = key2.getUlamKeyTypeSignatureReferenceType();
    if(alt1 != alt2)
      {
	if((alt1 == ALT_ARRAYITEM) || (alt2 == ALT_ARRAYITEM))
	{
	  if((alt1 == ALT_REF) || (alt2 == ALT_REF))
	    return UTIC_NOTSAME; //t3653
	  else if ((alt1 == ALT_CONSTREF) || (alt2 == ALT_CONSTREF))
	    return UTIC_NOTSAME; //like a ref
	  else if ((alt1 == ALT_AS) || (alt2 == ALT_AS))
	    return UTIC_NOTSAME; //like a ref
	  else
	    return UTIC_SAME; //matches ALT_NOT
	}
	else
	  return UTIC_NOTSAME;
      }
    return UTIC_SAME;
  } //compareWithWildArrayItemALTKey (static)

  ULAMTYPECOMPARERESULTS UlamType::compareWithWildALTKey(UTI u1, UTI u2, CompilerState& state)  //static
  {
    assert((u1 != Nouti) && (u2 != Nouti));

    if(u1 == u2) return UTIC_SAME; //short-circuit

    if((u1 == Nav) || (u2 == Nav)) return UTIC_NOTSAME;

    if((u1 == Hzy) || (u2 == Hzy)) return UTIC_DONTKNOW;

    UlamType * ut1 = state.getUlamTypeByIndex(u1);
    UlamType * ut2 = state.getUlamTypeByIndex(u2);
    ULAMCLASSTYPE ct1 = ut1->getUlamClassType();
    ULAMCLASSTYPE ct2 = ut2->getUlamClassType();
    UlamKeyTypeSignature key1 = ut1->getUlamKeyTypeSignature();
    UlamKeyTypeSignature key2 = ut2->getUlamKeyTypeSignature();

    // Given Class Arguments: we may end up with different sizes given different
    // argument values which may or may not be known while parsing!
    // t.f. classes are much more like the primitive ulamtypes now.
    // Was The Case: classes with unknown bitsizes are essentially as complete
    // as they can be during parse time; and will have the same UTIs.
    if(!ut1->isComplete())
      {
	if(ct1 == UC_NOTACLASS || ut1->getArraySize() == UNKNOWNSIZE)
	  return UTIC_DONTKNOW;

	//class with known arraysize(scalar or o.w.); no more Nav ids.
	if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
	  return UTIC_DONTKNOW;
      }

    if(!ut2->isComplete())
      {
	if(ct2 == UC_NOTACLASS || ut2->getArraySize() == UNKNOWNSIZE)
	  return UTIC_DONTKNOW;

	//class with known arraysize(scalar or o.w.); no more Nav ids.
	if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
	  return UTIC_DONTKNOW;
      }

    //both complete!
    //keys may have different reference types: (e.g. atomref = atom, atom = atomref)
    if(key1.getUlamKeyTypeSignatureNameId() != key2.getUlamKeyTypeSignatureNameId())
       return UTIC_NOTSAME;

    if(key1.getUlamKeyTypeSignatureArraySize() != key2.getUlamKeyTypeSignatureArraySize())
      return UTIC_NOTSAME;

    if(key1.getUlamKeyTypeSignatureBitSize() != key2.getUlamKeyTypeSignatureBitSize())
      return UTIC_NOTSAME;

    if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
      return UTIC_NOTSAME;

    ALT alt1 = key1.getUlamKeyTypeSignatureReferenceType();
    ALT alt2 = key2.getUlamKeyTypeSignatureReferenceType();
    if(alt1 != alt2)
      {
	return UTIC_SAME; //wild
      }
    return UTIC_SAME;
  } //compareWithWildALTKey (static)

  ULAMTYPECOMPARERESULTS UlamType::compareForArgumentMatching(UTI u1, UTI u2, CompilerState& state)  //static
  {
    return UlamType::compareWithWildArrayItemALTKey(u1, u2, state);
  } //compareForArgumentMatching

  ULAMTYPECOMPARERESULTS UlamType::compareForMakingCastingNode(UTI u1, UTI u2, CompilerState& state)  //static
  {
    return UlamType::compareWithWildArrayItemALTKey(u1, u2, state);
  } //compareForMakingCastingNode

  ULAMTYPECOMPARERESULTS UlamType::compareForUlamValueAssignment(UTI u1, UTI u2, CompilerState& state)  //static
  {
    return UlamType::compareWithWildALTKey(u1, u2, state);
  }

  ULAMTYPECOMPARERESULTS UlamType::compareForAssignment(UTI u1, UTI u2, CompilerState& state)  //static
  {
    return UlamType::compareWithWildALTKey(u1, u2, state);
  }

  ULAMTYPECOMPARERESULTS UlamType::compareForString(UTI u1, CompilerState& state)  //static
  {
    //bitsize always 32; wild reference type
    //arrays not treated as a String, per se (t3949, t3975, t3985, t3995)
    return UlamType::compareWithWildALTKey(u1, String, state);
  }

  ULAMTYPECOMPARERESULTS UlamType::compareForCustomArrayItem(UTI u1, UTI u2, CompilerState& state)  //static
  {
    return UlamType::compareWithWildALTKey(u1, u2, state);
  }

  u32 UlamType::getTotalWordSize()
  {
    assert(isComplete());
    return m_wordLengthTotal; //e.g. 0, 32, 64, 96
  }

  u32 UlamType::getItemWordSize()
  {
    assert(isComplete());
    return m_wordLengthItem; //e.g. 0, 32, 64, 96
  }

  void UlamType::setTotalWordSize(u32 tw)
  {
    m_wordLengthTotal = tw; //e.g. 32, 64, 96
  }

  void UlamType::setItemWordSize(u32 iw)
  {
    m_wordLengthItem = iw; //e.g. 32, 64, 96
  }

  u32 UlamType::getTotalNumberOfWords()
  {
    return getTotalWordSize()/MAXBITSPERINT; //wordsize was rounded up (no +31 needed)
  }

  bool UlamType::isMinMaxAllowed()
  {
    return false; //isScalar(); //minof/maxof allowed in ulam
  }

  u64 UlamType::getMax()
  {
    m_state.abortShouldntGetHere();
    return S64_MIN;
  }

  s64 UlamType::getMin()
  {
    m_state.abortShouldntGetHere();
    return S64_MAX;
  }

  u64 UlamType::getMax(UlamValue& rtnUV, UTI uti)
  {
    m_state.abortShouldntGetHere();
    return U64_MIN;
  }

  s64 UlamType::getMin(UlamValue& rtnUV, UTI uti)
  {
    m_state.abortShouldntGetHere();
    return S64_MAX;
  }

  PACKFIT UlamType::getPackable()
  {
    PACKFIT rtn = UNPACKED; //was false == 0
    u32 len = getTotalBitSize(); //could be 0, e.g. 'unknown'

    //scalars are considered packable (arraysize == NONARRAYSIZE); Atoms and Ptrs are NOT.
    if(len <= MAXBITSPERLONG)
      rtn = PACKEDLOADABLE;
    else
      if(len <= MAXSTATEBITS)
	rtn = PACKED;
    return rtn;
  } //getPackable

  const std::string UlamType::readMethodForCodeGen()
  {
    std::string method;
    u32 sizeByIntBits = getTotalWordSize();
    switch(sizeByIntBits)
      {
      case 0: //e.g. empty quarks
      case 32:
	method = "Read";
	break;
      case 64:
	method = "ReadLong";
	break;
      case 96:
	method = "ReadBig";
	break;
      default:
	method = "ReadBV"; //template arg deduced by gcc
      };
    return method;
  } //readMethodForCodeGen

  const std::string UlamType::writeMethodForCodeGen()
  {
    std::string method;
    u32 sizeByIntBits = getTotalWordSize();
    switch(sizeByIntBits)
      {
      case 0: //e.g. empty quarks
      case 32:
	method = "Write";
	break;
      case 64:
	method = "WriteLong";
	break;
      case 96:
	method = "WriteBig";
	break;
      default:
	method = "WriteBV"; //template arg deduced by gcc
      };
    return method;
  } //writeMethodForCodeGen

  const std::string UlamType::readArrayItemMethodForCodeGen()
  {
    std::string method;
    s32 sizeByIntBits = getItemWordSize();
    switch(sizeByIntBits)
      {
      case 0: //e.g. empty quarks
      case 32:
	method = "Read"; //ReadArray
	break;
      case 64:
	method = "ReadLong"; //ReadArrayLong
	break;
      case 96:
	method = "ReadBig";
	break;
      default:
	method = "ReadBV"; //template arg deduced by gcc
      };
    return method;
  } //readArrayItemMethodForCodeGen()

  const std::string UlamType::writeArrayItemMethodForCodeGen()
  {
    std::string method;
    s32 sizeByIntBits = getItemWordSize();
    switch(sizeByIntBits)
      {
      case 0: //e.g. empty quarks
      case 32:
	method = "Write"; //WriteArray
	break;
      case 64:
	method = "WriteLong"; //WriteArrayLong
	break;
      case 96:
	method = "WriteBig";
	break;
      default:
	method = "WriteBV"; //template arg deduced by gcc
      };
    return method;
  } //writeArrayItemMethodForCodeGen()

  //generates immediates with local storage
  void UlamType::genUlamTypeMangledDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamType::genUlamTypeReadDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamType::genUlamTypeWriteDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamType::genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamType::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamType::genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void UlamType::genStandardConfigTypedefTypenames(File * fp, CompilerState& state)
  {
    state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");
    fp->write("\n");
  } //(static)

  void UlamType::genGetUlamTypeMangledNameDefinitionForC(File * fp)
  {
    //for var args native funcs, non-refs, required of a BitStorage
    m_state.indent(fp);
    fp->write("virtual const char * GetUlamTypeMangledName() const { return \"");
    fp->write(getUlamTypeMangledName().c_str());
    fp->write("\"; }"); GCNL;
  }

} //end MFM
