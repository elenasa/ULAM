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

  const std::string UlamType::getUlamTypeNameOnly()
  {
    return m_key.getUlamKeyTypeSignatureName(&m_state);
  }

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
    assert(0);
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

    return CAST_CLEAR;
  } //safeCast

  FORECAST UlamType::explicitlyCastable(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx); //default
    if(scr == CAST_CLEAR)
      {
	// primitives must be the same sizes when casting to a reference type
	if(isReference() && !checkReferenceCast(typidx))
	  scr = CAST_BAD;
      }
    return scr;
  } //explicitlyCastable

  bool UlamType::checkArrayCast(UTI typidx)
  {
    if(isScalar() && m_state.isScalar(typidx))
      return true; //not arrays, ok

    bool bOK = true;
    if((getPackable() != PACKEDLOADABLE) || (m_state.determinePackable(typidx) != PACKEDLOADABLE))
      {
	//for now, limited to refs of same class, not subclasses. XXX
	UTI anyUTI = Nouti;
	AssertBool anyDefined = m_state.anyDefinedUTI(m_key, anyUTI);
	assert(anyDefined);

	std::ostringstream msg;
	msg << "Casting requires UNPACKED array support: ";
	msg << m_state.getUlamTypeNameBriefByIndex(typidx).c_str();
	msg << " TO " ;
	msg << getUlamTypeNameBrief().c_str();

	if((m_state.isARefTypeOfUlamType(typidx, anyUTI) == UTIC_SAME))
	  MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	else
	  {
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    bOK = false;
	  }
      }
    else
      {
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
      }
    return bOK;
  } //checkArrayCast

  bool UlamType::checkReferenceCast(UTI typidx)
  {
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
      return false;

    //skip rest in the case of array item, continue with usual size fit
    if(alt1 == ALT_ARRAYITEM || alt2 == ALT_ARRAYITEM)
      return true;

    if(key1.getUlamKeyTypeSignatureBitSize() != key2.getUlamKeyTypeSignatureBitSize())
	  return false;
    //if(alt1 != ALT_NOT || alt2 == ALT_NOT)
    //  return false;

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
    assert(0);
    return (s32) data;
  }

  u32 UlamType::getDataAsCu32(const u32 data)
  {
    assert(0);
    return data;
  }

  s64 UlamType::getDataAsCs64(const u64 data)
  {
    assert(0);
    return (s64) data;
  }

  u64 UlamType::getDataAsCu64(const u64 data)
  {
    assert(0);
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

    if(isReference())
      {
	assert(0);
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
    assert(0);
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
	  //assert(0);
	  //MSG(getNodeLocationAsString().c_str(), "Need UNPACKED ARRAY", INFO);
	}
      };
    return ctype;
  } //getTmpStorageTypeAsString

  STORAGE UlamType::getTmpStorageTypeForTmpVar()
  {
    return TMPREGISTER;
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
    assert(0); //see UlamTypePrimitive
  } //genUlamTypeMangledAutoDefinitionForC

  void UlamType::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    assert(0);
  }

  void UlamType::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    assert(0);
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

  ALT UlamType::getReferenceType()
  {
    return m_key.getUlamKeyTypeSignatureReferenceType();
  }

  bool UlamType::isReference()
  {
    return m_key.getUlamKeyTypeSignatureReferenceType() != ALT_NOT;
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

  ULAMTYPECOMPARERESULTS UlamType::compareWithWildArrayItemReferenceType(UTI u1, UTI u2, CompilerState& state)  //static
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
      return UTIC_NOTSAME; //?

    ALT alt1 = key1.getUlamKeyTypeSignatureReferenceType();
    ALT alt2 = key2.getUlamKeyTypeSignatureReferenceType();
    if(alt1 != alt2)
      {
	if(alt1 == ALT_ARRAYITEM || alt2 == ALT_ARRAYITEM)
	  return UTIC_SAME;
	else
	  return UTIC_NOTSAME;
      }
    return UTIC_SAME;
  } //compareWithWildArrayItemReferenceType (static)

  ULAMTYPECOMPARERESULTS UlamType::compareWithWildReferenceType(UTI u1, UTI u2, CompilerState& state)  //static
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

    //if(key1.getUlamKeyTypeSignatureClassInstanceIdx() != key2.getUlamKeyTypeSignatureClassInstanceIdx())
    // return UTIC_NOTSAME; //?

    ALT alt1 = key1.getUlamKeyTypeSignatureReferenceType();
    ALT alt2 = key2.getUlamKeyTypeSignatureReferenceType();
    if(alt1 != alt2)
      {
	return UTIC_SAME; //wild
      }
    return UTIC_SAME;
  } //compareWithWildReferenceType (static)

  ULAMTYPECOMPARERESULTS UlamType::compareForArgumentMatching(UTI u1, UTI u2, CompilerState& state)  //static
  {
    return UlamType::compareWithWildArrayItemReferenceType(u1, u2, state);
  }

  ULAMTYPECOMPARERESULTS UlamType::compareForMakingCastingNode(UTI u1, UTI u2, CompilerState& state)  //static
  {
    return UlamType::compareWithWildArrayItemReferenceType(u1, u2, state);
  }

  ULAMTYPECOMPARERESULTS UlamType::compareForUlamValueAssignment(UTI u1, UTI u2, CompilerState& state)  //static
  {
    return UlamType::compareWithWildReferenceType(u1, u2, state);
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

  bool UlamType::isMinMaxAllowed()
  {
    return false; //isScalar(); //minof/maxof allowed in ulam
  }

  u64 UlamType::getMax()
  {
    assert(0);
  }

  s64 UlamType::getMin()
  {
    assert(0);
  }

  u64 UlamType::getMax(UlamValue& rtnUV, UTI uti)
  {
    assert(0);
  } //getMax (UlamValue)

  s64 UlamType::getMin(UlamValue& rtnUV, UTI uti)
  {
    assert(0);
  } //getMin (UlamValue)

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
	method = "ReadBV";
#if 0
	{
	  std::ostringstream mstr;
	  mstr << "ReadBV<" << getTotalBitSize() << ">";
	  method = mstr.str();
	}
#endif
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
	method = "WriteBV";
#if 0
	{
	  std::ostringstream mstr;
	  mstr << "WriteBV<" << getTotalBitSize() << ">";
	  method = mstr.str();
	}
#endif
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
	method = "ReadBV";
#if 0
	{
	  std::ostringstream mstr;
	  mstr << "ReadBV<" << getTotalBitSize() << ">";
	  method = mstr.str();
	}
#endif
	//TBD ReadArrayUnpacked
	//assert(0);
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
	method = "WriteBV"; //TBD WriteArrayUnpacked
      };
    return method;
  } //writeArrayItemMethodForCodeGen()

  //generates immediates with local storage
  void UlamType::genUlamTypeMangledDefinitionForC(File * fp)
  {
    assert(0);
  } //genUlamTypeMangledDefinitionForC

  void UlamType::genUlamTypeReadDefinitionForC(File * fp)
  {
    assert(0);
  }

  void UlamType::genUlamTypeWriteDefinitionForC(File * fp)
  {
    assert(0);
  }

  void UlamType::genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp)
  {
    assert(0);
  } //genUlamTypeMangledUnpackedArrayAutoDefinitionForC

  void UlamType::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    assert(0);
  } //genUlamTypeMangledUnpackedArrayDefinitionForC

  void UlamType::genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp)
  {
    assert(0);
  } //genUlamTypeMangledImmediateModelParameterDefinitionForC

  bool UlamType::genUlamTypeDefaultQuarkConstant(File * fp, u32& dqref)
  {
    assert(0);
    return false; //only true for quarks in UlamTypeClass
  }

} //end MFM
