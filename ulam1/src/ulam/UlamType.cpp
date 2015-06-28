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


  UlamType::UlamType(const UlamKeyTypeSignature key, CompilerState & state) : m_key(key), m_state(state), m_wordLengthTotal(0), m_wordLengthItem(0), m_max(S32_MIN), m_min(S32_MAX)
  {}

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
    return m_key.getUlamKeyTypeSignatureNameAndBitSize(&m_state);
  }

  const std::string UlamType::getUlamTypeNameOnly()
  {
    return m_key.getUlamKeyTypeSignatureName(&m_state);
  }

   UlamKeyTypeSignature UlamType::getUlamKeyTypeSignature()
  {
    return m_key;
  }

  bool UlamType::cast(UlamValue & val, UTI typidx)
  {
    assert(0);
    //std::cerr << "UlamTypeClass (cast) error! " << std::endl;
    return false;
  }

  bool UlamType::castTo32(UlamValue & val, UTI typidx)
  {
    assert(0);
    //std::cerr << "UlamTypeClass (cast) error! " << std::endl;
    return false;
  }

  bool UlamType::castTo64(UlamValue & val, UTI typidx)
  {
    assert(0);
    //std::cerr << "UlamTypeClass (cast) error! " << std::endl;
    return false;
  }

  FORECAST UlamType::safeCast(UTI typidx)
  {
    // initial tests for completeness and scalars
    s32 bitsize = getBitSize();
    s32 valbitsize = m_state.getBitSize(typidx);

    if(!isComplete() || !m_state.isComplete(typidx))
      {
	std::ostringstream msg;
	msg << "Casting UNKNOWN sizes; " << bitsize;
	msg << ", Value Type and size was: " << typidx << "," << valbitsize;
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	return CAST_HAZY;
      }

    //let packable arrays of same size pass...
    return checkArrayCast(typidx) ? CAST_CLEAR : CAST_BAD;
  } //safeCast

  FORECAST UlamType::explicitlyCastable(UTI typidx)
  {
    return UlamType::safeCast(typidx); //default
  } //explicitlyCastable

  bool UlamType::checkArrayCast(UTI typidx)
  {
    if(isScalar() && m_state.isScalar(typidx))
      return true;

    bool bOK = true;
    if(getPackable() != PACKEDLOADABLE || m_state.determinePackable(typidx) != PACKEDLOADABLE)
      {
	std::ostringstream msg;
	msg << "Casting requires UNPACKED array support: ";
	msg << m_state.getUlamTypeNameByIndex(typidx).c_str();
	msg << " TO " ;
	msg << getUlamTypeName().c_str();
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);

	bOK = false;
      }
    else
      {
	s32 arraysize = getArraySize();
	s32 varraysize = m_state.getArraySize(typidx);
	if(arraysize != varraysize)
	  {
	    std::ostringstream msg;
	    msg << "Casting different Array sizes: " << arraysize;
	    msg << ", Value Type and size was: ";
	    msg << typidx << "," << varraysize;
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    bOK = false;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Casting nonScalar Array size: " << arraysize;
	    msg << ", Value Type and size was: ";
	    msg << typidx << "," << varraysize;
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	  }
      }
    return bOK;
  } //checkArrayCast

  void UlamType::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    sprintf(valstr,"%s", getUlamTypeName().c_str());
  }

  void UlamType::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    sprintf(valstr,"%s", getUlamTypeName().c_str());
  }

  ULAMCLASSTYPE UlamType::getUlamClass()
  {
    return UC_NOTACLASS;
  }

  const std::string UlamType::getUlamTypeAsStringForC()
  {
    assert(isComplete());

    s32 len = getTotalBitSize(); //includes arrays
    assert(len >= 0);
    u32 roundUpSize = getTotalWordSize();

    std::ostringstream ctype;
    ctype << "BitField<BitVector<" << roundUpSize << ">, ";

    if(isScalar())
      ctype << getUlamTypeVDAsStringForC() << ", ";
    else
      ctype << "VD::BITS, "; //use BITS for arrays

    if(!isScalar() && getPackable() != PACKEDLOADABLE)
      {
	s32 itemlen = getBitSize(); //per item
	ctype << itemlen << ", " << getItemWordSize() - itemlen << ">";
      }
    else
      ctype << len << ", " << roundUpSize - len << ">";

    return ctype.str();
  }

  const std::string UlamType::getUlamTypeVDAsStringForC()
  {
    assert(0);
    return "VD::NOTDEFINED";
  }

  const std::string UlamType::getUlamTypeMangledType()
  {
    // e.g. parsing overloaded functions, may not be complete.
    std::ostringstream mangled;
    s32 bitsize = getBitSize();
    s32 arraysize = getArraySize();

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

  const std::string UlamType::getUlamTypeImmediateMangledName()
  {
    std::ostringstream imangled;
    imangled << "Ui_" << getUlamTypeMangledName();
    return  imangled.str();
  }

  const std::string UlamType::getUlamTypeImmediateAutoMangledName()
  {
    assert(0); //only elements and quarks so far
  }

  bool UlamType::needsImmediateType()
  {
    return isComplete();
  }

  const std::string UlamType::getImmediateStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName(); //name of struct w typedef(bf) and storage(bv);
    return ctype.str();
  } //getImmediateStorageTypeAsString

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
      default:
	{
	  assert(0);
	  //MSG(getNodeLocationAsString().c_str(), "Need UNPACKED ARRAY", INFO);
	}
      };
    return ctype;
  } //getTmpStorageTypeAsString

  const char * UlamType::getUlamTypeAsSingleLowercaseLetter()
  {
    return UlamType::getUlamTypeEnumCodeChar(getUlamTypeEnum());
  } //getUlamTypeAsSingleLowercaseLetter

  void UlamType::genUlamTypeMangledDefinitionForC(File * fp)
  {
    m_state.m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName();
    std::ostringstream  ud;
    ud << "Ud_" << mangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    u32 sizeByIntBits = getTotalWordSize();

    m_state.indent(fp);
    fp->write("#ifndef ");
    fp->write(udstr.c_str());
    fp->write("\n");

    m_state.indent(fp);
    fp->write("#define ");
    fp->write(udstr.c_str());
    fp->write("\n");

    m_state.indent(fp);
    fp->write("namespace MFM{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    fp->write("\n");
    m_state.indent(fp);
    fp->write("{\n");

    //typedef bitfield inside struct
    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeAsStringForC().c_str()); //e.g. BitField
    fp->write(" BF;\n");

    //storage here
    m_state.indent(fp);
    fp->write("BitVector<");
    fp->write_decimal(sizeByIntBits);
    fp->write(">");
    fp->write(" m_stg;\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stg() { }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" d) : m_stg(d) {}\n");

    //read BV method
    genUlamTypeReadDefinitionForC(fp);

    //write BV method
    genUlamTypeWriteDefinitionForC(fp);

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("};\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //MFM\n");

    m_state.indent(fp);
    fp->write("#endif /*");
    fp->write(udstr.c_str());
    fp->write(" */\n\n");
  } //genUlamTypeMangledDefinitionForC

  void UlamType::genUlamTypeReadDefinitionForC(File * fp)
  {
    if(isScalar() || getPackable() == PACKEDLOADABLE)
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" read() const { return BF::");
	fp->write(readMethodForCodeGen().c_str());

	if(isScalar())
	  fp->write("(m_stg); }\n"); //done
	else
	  fp->write("(m_stg); }   //reads entire array\n");
      }

    if(!isScalar())
      {
	//reads an element of array
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 unitsize) const { return BF::");
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(m_stg, index, unitsize");
	fp->write("); }\n");
      }
  } //genUlamTypeReadDefinitionForC

  void UlamType::genUlamTypeWriteDefinitionForC(File * fp)
  {
    if(isScalar() || getPackable() == PACKEDLOADABLE)
      {
	m_state.indent(fp);
	fp->write("void write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" v) { BF::");
	fp->write(writeMethodForCodeGen().c_str());

	if(isScalar())
	  fp->write("(m_stg, v); }\n");
	else
	  fp->write("(m_stg, v); }   //writes entire array\n");
      }

    if(!isScalar())
      {
	// writes an element of array
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" v, const u32 index, const u32 unitsize) { BF::");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(m_stg, v, index, unitsize");
	fp->write("); }\n");
      }
  } //genUlamTypeWriteDefinitionForC

  void UlamType::genUlamTypeMangledImmediateDefinitionForC(File * fp)
  {
    const std::string mangledName = getUlamTypeImmediateMangledName();
    std::ostringstream  up;

    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeAsStringForC().c_str()); //e.g. BitVector
    fp->write(" ");
    fp->write(mangledName.c_str());
    fp->write(";\n");
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
    if(u1 == u2) return UTIC_SAME; //short-circuit

    if(u1 == Nav || u2 == Nav) return UTIC_NOTSAME;

    UlamType * ut1 = state.getUlamTypeByIndex(u1);
    UlamType * ut2 = state.getUlamTypeByIndex(u2);
    ULAMCLASSTYPE ct1 = ut1->getUlamClass();
    ULAMCLASSTYPE ct2 = ut2->getUlamClass();
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

  u32 UlamType::getTotalWordSize()
  {
    assert(isComplete());
    return m_wordLengthTotal; //e.g. 32, 64, 96
  }

  u32 UlamType::getItemWordSize()
  {
    assert(isComplete());
    return m_wordLengthItem; //e.g. 32, 64, 96
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
    return isScalar(); //minof/maxof allowed in ulam
  }

  u64 UlamType::getMax()
  {
    return m_max;
  }

  s64 UlamType::getMin()
  {
    return m_min;
  }

  PACKFIT UlamType::getPackable()
  {
    PACKFIT rtn = UNPACKED; //was false == 0
    u32 len = getTotalBitSize(); //could be 0, e.g. 'unknown'

    //scalars are considered packable (arraysize == NONARRAYSIZE); Atoms and Ptrs are NOT.
    //if(len <= MAXBITSPERINT)
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
      default:
	method = "ReadUnpacked"; //TBD
	//MSG(getNodeLocationAsString().c_str(), "Need UNPACKED ARRAY", INFO);
	assert(0);
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
      default:
	method = "WriteUnpacked"; //TBD
	//MSG(getNodeLocationAsString().c_str(), "Need UNPACKED ARRAY", INFO);
	assert(0);
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
	method = "ReadArray";
	break;
      case 64:
	method = "ReadArrayLong";
	break;
      default:
	method = "ReadArrayUnpacked"; //TBD
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
	method = "WriteArray";
	break;
      case 64:
	method = "WriteArrayLong";
	break;
      default:
	method = "WriteArrayUnpacked"; //TBD
      };
    return method;
  } //writeArrayItemMethodForCodeGen()

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

  void UlamType::genCodeAfterReadingIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    return; //nothing to do usually
  }

  void UlamType::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    assert(0); //only quarks and elements so far
  }

} //end MFM
