#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sstream>
#include "UlamTypePrimitive.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypePrimitive::UlamTypePrimitive(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state), m_max(S32_MIN), m_min(S32_MAX) {}

  bool UlamTypePrimitive::isNumericType()
  {
    return false;
  }

  bool UlamTypePrimitive::isPrimitiveType()
  {
    return true;
  }

  bool UlamTypePrimitive::castTo32(UlamValue & val, UTI typidx)
  {
    assert(0);
    //std::cerr << "UlamType (cast) error! " << std::endl;
    return false;
  }

  bool UlamTypePrimitive::castTo64(UlamValue & val, UTI typidx)
  {
    assert(0);
    //std::cerr << "UlamType (cast) error! " << std::endl;
    return false;
  }

  s32 UlamTypePrimitive::getDataAsCs32(const u32 data)
  {
    assert(0);
    return (s32) data;
  }

  u32 UlamTypePrimitive::getDataAsCu32(const u32 data)
  {
    assert(0);
    return data;
  }

  s64 UlamTypePrimitive::getDataAsCs64(const u64 data)
  {
    assert(0);
    return (s64) data;
  }

  u64 UlamTypePrimitive::getDataAsCu64(const u64 data)
  {
    assert(0);
    return data;
  }

  s32 UlamTypePrimitive::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
  {
    return UNKNOWNSIZE; //atom, class, nav, ptr, holder
  }

  const std::string UlamTypePrimitive::getUlamTypeMangledType()
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

    std::string ecode(UlamTypePrimitive::getUlamTypeEnumCodeChar(getUlamTypeEnum()));
    mangled << ToLeximited(ecode).c_str();

    return mangled.str();
  } //getUlamTypeMangledType

  const std::string UlamTypePrimitive::getUlamTypeMangledName()
  {
    // e.g. parsing overloaded functions, may not be complete.
    std::ostringstream mangled;
    mangled << getUlamTypeUPrefix().c_str();
    mangled << getUlamTypeMangledType();
    return mangled.str();
  } //getUlamTypeMangledName

  const std::string UlamTypePrimitive::getUlamTypeUPrefix()
  {
    return "Ut_";
  }

  bool UlamTypePrimitive::needsImmediateType()
  {
    return isComplete();
  }

  const std::string UlamTypePrimitive::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName();
    ctype << "<EC>"; //name of struct w typedef(bf) and storage(bv);
    return ctype.str();
  } //getLocalStorageTypeAsString

  const std::string UlamTypePrimitive::getImmediateModelParameterStorageTypeAsString()
  {
    std::ostringstream mpimangled;
    //substitutes Up_ for Ut_ for model parameter immediate
    mpimangled << "Ui_Up_" << getUlamTypeMangledType();
    return mpimangled.str();
  } //getImmediateModelParameterStorageTypeAsString

  const std::string UlamTypePrimitive::getArrayItemTmpStorageTypeAsString()
  {
    return getTmpStorageTypeAsString(getItemWordSize());
  }

  const std::string UlamTypePrimitive::getTmpStorageTypeAsString()
  {
    return getTmpStorageTypeAsString(getTotalWordSize());
  }

  const std::string UlamTypePrimitive::getTmpStorageTypeAsString(s32 sizebyints)
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
	  //ctype = getTmpStorageTypeAsString(getItemWordSize()); //u32, u64 (inf loop)
	  ctype = "T";
	  //assert(0);
	  //MSG(getNodeLocationAsString().c_str(), "Need UNPACKED ARRAY", INFO);
	}
      };
    return ctype;
  } //getTmpStorageTypeAsString

  STORAGE UlamTypePrimitive::getTmpStorageTypeForTmpVar()
  {
    //immediate storage is TMPBITVAL for all UlamTypes.
    return TMPREGISTER;
  }

  const std::string UlamTypePrimitive::castMethodForCodeGen(UTI nodetype)
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

  void UlamTypePrimitive::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    s32 len = getTotalBitSize();

    //if(len > (BITSPERATOM - ATOMFIRSTSTATEBITPOS))
    //  return genUlamTypeMangledUnpackedArrayAutoDefinitionForC(fp);

    m_state.m_currentIndentLevel = 0;
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();
    std::ostringstream  ud;
    ud << "Ud_" << automangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

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
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(automangledName.c_str());
    fp->write(" : public UlamRef<EC>");
    fp->write("\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    m_state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");
    fp->write("\n");

    //constructor for ref (auto)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, u32 origin) : UlamRef<EC>(idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, origin, targ, NULL) { }\n"); //effself is null for primitives

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, u32 idx) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, NULL) { }\n"); //effself is null for primitives

    genUlamTypeAutoReadDefinitionForC(fp);

    genUlamTypeAutoWriteDefinitionForC(fp);

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
  } //genUlamTypeMangledAutoDefinitionForC

  void UlamTypePrimitive::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    //scalar and entire PACKEDLOADABLE array handled by base class read method
    if(!isScalar())
      {
	//reads an item of array;
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return ");
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //rel offset
	fp->write("itemlen, NULL)."); //itemlen, primitive effself
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(); }\n");
      }
  } //genUlamTypeAutoReadDefinitionForC

  void UlamTypePrimitive::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    //scalar and entire PACKEDLOADABLE array handled by base class write method
    if(!isScalar())
      {
	// writes an element of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" v, const u32 index, const u32 itemlen) { ");
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //rel offset
	fp->write("itemlen, NULL)."); //itemlen, primitive effself
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(v); }\n");
      }
  } //genUlamTypeAutoWriteDefinitionForC

  ULAMCLASSTYPE UlamTypePrimitive::getUlamClassType()
  {
    return UC_NOTACLASS;
  }

  bool UlamTypePrimitive::isMinMaxAllowed()
  {
    return isScalar(); //minof/maxof allowed in ulam
  }

  u64 UlamTypePrimitive::getMax()
  {
    return m_max;
  }

  s64 UlamTypePrimitive::getMin()
  {
    return m_min;
  }

  u64 UlamTypePrimitive::getMax(UlamValue& rtnUV, UTI uti)
  {
    u32 wordsize = getTotalWordSize();
    if(wordsize <= MAXBITSPERINT)
      rtnUV = UlamValue::makeImmediate(uti, (u32) m_max, m_state);
    else if(wordsize <= MAXBITSPERLONG)
      rtnUV = UlamValue::makeImmediateLong(uti, m_max, m_state);
    return m_max;
  } //getMax (UlamValue)

  s64 UlamTypePrimitive::getMin(UlamValue& rtnUV, UTI uti)
  {
    u32 wordsize = getTotalWordSize();
    if(wordsize <= MAXBITSPERINT)
      rtnUV = UlamValue::makeImmediate(uti, (s32) m_min, m_state);
    else if(wordsize <= MAXBITSPERLONG)
      rtnUV = UlamValue::makeImmediateLong(uti, (s64) m_min, m_state);
    return m_min;
  } //getMin (UlamValue)

  PACKFIT UlamTypePrimitive::getPackable()
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

  //generates immediates with local storage
  void UlamTypePrimitive::genUlamTypeMangledDefinitionForC(File * fp)
  {
    u32 len = getTotalBitSize(); //could be 0, includes arrays

    //if(len > (BITSPERATOM - ATOMFIRSTSTATEBITPOS))
    //  return genUlamTypeMangledUnpackedArrayDefinitionForC(fp); //no auto, just immediate

    m_state.m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName();
    std::ostringstream  ud;
    ud << "Ud_" << mangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

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
    fp->write("\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    fp->write(" : public ");
    fp->write("BitVectorStorage");
    fp->write("<EC, BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("u> >\n");

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    m_state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");
    m_state.indent(fp);
    fp->write("typedef BitVectorStorage<EC, BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("> > BVS;\n");
    fp->write("\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //u32
    fp->write(" d) { ");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(d); }\n");

    //copy constructor here (return by value)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str()); //u32
    fp->write("& other) { ");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(other.");
    fp->write(readMethodForCodeGen().c_str());
    fp->write("()); }\n ");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

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

  void UlamTypePrimitive::genUlamTypeReadDefinitionForC(File * fp)
  {
    if(isScalar() || (getPackable() == PACKEDLOADABLE))
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32 or u64
	fp->write(" ");
	fp->write(readMethodForCodeGen().c_str());
	fp->write("() const { return BVS::"); //or read()? ReadLong
	fp->write(readMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal_unsigned(getTotalBitSize());
	if(isScalar())
	  fp->write("u); }\n"); //done
	else
	  fp->write("u); } //reads entire array\n");
      }

    //scalar and entire PACKEDLOADABLE array handled by base class read method
    if(!isScalar())
      {
	//reads an item of array;
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return BVS::");
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, "); //rel offset
	fp->write("itemlen)"); //itemlen
	fp->write("; }\n");
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypePrimitive::genUlamTypeWriteDefinitionForC(File * fp)
  {
    if(isScalar() || (getPackable() == PACKEDLOADABLE))
      {
	m_state.indent(fp);
	fp->write("void ");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(const "); //or write? WriteLong
	fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32, s64 or u64
	fp->write(" v) { BVS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal_unsigned(getTotalBitSize());

	if(isScalar())
	  fp->write("u, v); }\n");
	else
	  fp->write("u, v); } //writes entire array\n");
      }

    //scalar and entire PACKEDLOADABLE array handled by base class write method
    if(!isScalar())
      {
	// writes an element of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" v, const u32 index, const u32 itemlen) { BVS::");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, "); //rel offset
	fp->write("itemlen, v)"); //itemlen, primitive effself
	fp->write("; }\n");
      }
  } //genUlamTypeWriteDefinitionForC

  //(redundant for primitives)
  void UlamTypePrimitive::genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp)
  {
    m_state.m_currentIndentLevel = 0;
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();
    const std::string mangledName = getUlamTypeImmediateMangledName();
    std::ostringstream  ud;
    ud << "Ud_" << automangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    u32 itemlen = getBitSize();
    u32 arraysize = getArraySize();

    UlamKeyTypeSignature baseKey(m_key.m_typeNameId, itemlen);
    UTI scalarUTI = m_state.makeUlamType(baseKey, getUlamTypeEnum(), UC_NOTACLASS);
    UlamType * scalarut = m_state.getUlamTypeByIndex(scalarUTI);

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
    fp->write("\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(mangledName.c_str());
    fp->write("; //forward \n\n");

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(automangledName.c_str());

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    m_state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");
    fp->write("\n");

    m_state.indent(fp);
    fp->write("typedef UlamRefFixed");
    fp->write("<EC, "); //BITSPERATOM
    fp->write_decimal(BITSPERATOM - ATOMFIRSTSTATEBITPOS - itemlen); //right-justified, relative
    fp->write(", ");
    fp->write_decimal(itemlen);
    fp->write("u> Up_Us;\n"); //per item

    //storage here (an array of T's)
    m_state.indent(fp);
    fp->write("T (& m_stgarrayref)[");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u];  //ref to BIG storage!\n\n");

    //copy constructor here
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("( ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r) : m_stgarrayref(r.m_stgarrayref) { }\n");

    //constructor here (used by tmpVars)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("( ");
    fp->write(mangledName.c_str());
    fp->write("<EC>& d) : m_stgarrayref(d.getStorageRef()) { }\n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(automangledName.c_str());
    fp->write("() {}\n");

    //Unpacked Read Array Item
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" readArrayItem(");
    fp->write("const u32 index, const u32 itemlen) { return ");
    fp->write("Up_Us(getRef(index), NULL).");
    fp->write(scalarut->readMethodForCodeGen().c_str());
    fp->write("(); }\n");

    //Unpacked Write Array Item
    m_state.indent(fp);
    fp->write("void writeArrayItem(const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" v, const u32 index, const u32 itemlen) { ");
    fp->write("Up_Us(getRef(index), NULL).");
    fp->write(scalarut->writeMethodForCodeGen().c_str());
    fp->write("(v);");
    fp->write(" }\n");

    //Unpacked, an item T
    m_state.indent(fp);
    fp->write("BitStorage<EC> ");
    fp->write("getBits(");
    fp->write("const u32 index) { return ");
    fp->write("m_stgarrayref[index]; }\n");

    //Unpacked, an item T const
    m_state.indent(fp);
    fp->write("const BitStorage<EC> ");
    fp->write("getBits(");
    fp->write("const u32 index) const { return ");
    fp->write("m_stgarrayref[index]; }\n");

    //Unpacked, an item T&
    m_state.indent(fp);
    fp->write("BitStorage<EC>& ");
    fp->write("getRef(");
    fp->write("const u32 index) { return ");
    fp->write("m_stgarrayref[index]; }\n");

    //Unpacked, position within whole
    m_state.indent(fp);
    fp->write("const u32 ");
    fp->write("getPosOffset(");
    fp->write("const u32 index) const { return ");
    fp->write("(BPA * index + BPA - T::ATOM_FIRST_STATE_BIT - ");
    fp->write_decimal_unsigned(itemlen); //right-justified, relative per item
    fp->write("u); }\n");

    //Unpacked, position within each item T
    m_state.indent(fp);
    fp->write("const u32 ");
    fp->write("getPosOffset(");
    fp->write(" ) const { return ");
    fp->write("(BPA - T::ATOM_FIRST_STATE_BIT - ");
    fp->write_decimal_unsigned(itemlen); //right-justified, relative per item
    fp->write("u); }\n");

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
  } //genUlamTypeMangledUnpackedArrayAutoDefinitionForC

  //(redundant for primitives)
  void UlamTypePrimitive::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    m_state.m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName();
    std::ostringstream  ud;
    ud << "Ud_" << mangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    u32 itemlen = getBitSize();
    u32 arraysize = getArraySize();

    UlamKeyTypeSignature baseKey(m_key.m_typeNameId, itemlen);
    UTI scalarUTI = m_state.makeUlamType(baseKey, getUlamTypeEnum(), UC_NOTACLASS);
    UlamType * scalarut = m_state.getUlamTypeByIndex(scalarUTI);

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
    fp->write("\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    m_state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");
    fp->write("\n");

    m_state.indent(fp);
    fp->write("typedef UlamRefFixed");
    fp->write("<EC, "); //BITSPERATOM
    fp->write_decimal(BITSPERATOM - ATOMFIRSTSTATEBITPOS - itemlen); //right-justified, relative
    fp->write(", ");
    fp->write_decimal(itemlen);
    fp->write("u> Up_Us;\n"); //one item

    //Unpacked, storage reference T (&) [N]
    m_state.indent(fp);
    fp->write("typedef BitVectorStorage<EC, BitVector<");
    fp->write_decimal_unsigned(itemlen);
    fp->write("u> > TARR[");
    fp->write_decimal_unsigned(arraysize);
    fp->write("];\n");

    //storage here (an array of T's)
    m_state.indent(fp);
    fp->write("TARR m_stgarr;  //BIG storage here!\n\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() {");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("m_stgarr[j].Write(0u, ");
    fp->write_decimal_unsigned(itemlen);
    fp->write("u, 0u);"); //default each item to zero
    fp->write(" }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //u32
    fp->write(" d) { ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) {");
    fp->write("m_stgarr[j].SetUndefinedImpl(); "); //T::ATOM_UNDEFINED_TYPE
    fp->write("writeArrayItem(d, j, ");
    fp->write_decimal_unsigned(itemlen); //right-justified per item
    fp->write("u); } }\n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

    //Unpacked Read Array Item
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" readArrayItem(");
    fp->write("const u32 index, const u32 itemlen) { return ");
    fp->write("Up_Us(getRef(index), NULL).");
    fp->write(scalarut->readMethodForCodeGen().c_str());
    fp->write("(); }\n");

    //Unpacked Write Array Item
    m_state.indent(fp);
    fp->write("void writeArrayItem(const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" v, const u32 index, const u32 itemlen) { ");
    fp->write("Up_Us(getRef(index), NULL).");
    fp->write(scalarut->writeMethodForCodeGen().c_str());
    fp->write("(v);");
    fp->write(" }\n");

    //Unpacked, an item T
    m_state.indent(fp);
    fp->write("BitVector<BPA>& ");
    fp->write("getBits(");
    fp->write("const u32 index) { return ");
    fp->write("m_stgarr[index].GetBits(); }\n");

    //Unpacked, an item T const
    m_state.indent(fp);
    fp->write("const BitVector<BPA>& ");
    fp->write("getBits(");
    fp->write("const u32 index) const { return ");
    fp->write("m_stgarr[index].GetBits(); }\n");

    //Unpacked, an item T&
    m_state.indent(fp);
    fp->write("T& ");
    fp->write("getRef(");
    fp->write("const u32 index) { return ");
    fp->write("m_stgarr[index]; }\n");

    //Unpacked, position within whole
    m_state.indent(fp);
    fp->write("const u32 ");
    fp->write("getPosOffset(");
    fp->write("const u32 index) const { return ");
    fp->write("(BPA * index + BPA - T::ATOM_FIRST_STATE_BIT - ");
    fp->write_decimal_unsigned(itemlen); //right-justified, relative per item
    fp->write("u); }\n");

    //Unpacked, position within each item T
    m_state.indent(fp);
    fp->write("const u32 ");
    fp->write("getPosOffset(");
    fp->write(" ) const { return ");
    fp->write("(BPA - T::ATOM_FIRST_STATE_BIT - ");
    fp->write_decimal_unsigned(itemlen); //right-justified, relative per item
    fp->write("u); }\n");

    m_state.indent(fp);
    fp->write("TARR& getStorageRef(");
    fp->write(") { return ");
    fp->write("m_stgarr; }\n");

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
  } //genUlamTypeMangledUnpackedArrayDefinitionForC

  void UlamTypePrimitive::genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp)
  {
    assert(isScalar());

    const std::string mangledName = getImmediateModelParameterStorageTypeAsString();

    std::ostringstream  ud;
    ud << "Ud_" << mangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    m_state.m_currentIndentLevel = 0;

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
    fp->write("\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    fp->write("\n");

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("// immediate model parameter definition:\n");

    //typedef atomic parameter type inside struct
    m_state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");
    fp->write("\n");

    s32 len = getTotalBitSize();

    m_state.indent(fp);
    fp->write("typedef UlamRefFixed");
    fp->write("<EC, "); //BITSPERATOM
    fp->write_decimal(BITSPERATOM - ATOMFIRSTSTATEBITPOS - len); //right-justified, relative
    fp->write(", ");
    fp->write_decimal(len);
    fp->write("u > Up_Us;\n");

    //reference to storage in atom
    m_state.indent(fp);
    fp->write("T* m_stgPtr;  //ptr to storage here!\n");

    // constructor with args
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stgPtr(NULL) { }\n");

    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" read() const { MFM_API_ASSERT_NONNULL(m_stgPtr); return Up_Us(*m_stgPtr, 0u, NULL)."); //origin 0u
    fp->write(readMethodForCodeGen().c_str());
    fp->write("(); }\n");

    m_state.indent(fp);
    fp->write("void init(T& realStg) { m_stgPtr = &realStg; }\n");

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
  } //genUlamTypeMangledImmediateModelParameterDefinitionForC

  bool UlamTypePrimitive::genUlamTypeDefaultQuarkConstant(File * fp, u32& dqref)
  {
    return false; //only true for quarks in UlamTypeClass
  }

} //end MFM
