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

    //constructor for ref (auto); wo origin
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx) : UlamRef<EC>(idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, targ, NULL) { }\n"); //effself is null for primitives

    //copy constructor for autorefs
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg) : UlamRef<EC>(arg, arg.GetPos(), arg.GetLen(), NULL) { ");
    fp->write("MFM_API_ASSERT_ARG(arg.GetLen() == ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("); }\n"); //effself is null for primitives

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

    if(isScalar() || WritePacked(getPackable()))
      {
	// write must be scalar; ref param to avoid excessive copying
	//not an array
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write(" read() const { ");
	fp->write("return ");
	fp->write("UlamRef<EC>::");
	fp->write(readMethodForCodeGen().c_str()); //just the guts
	fp->write("(); /* entire */ }\n");
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
	fp->write("& v, const u32 index, const u32 itemlen) { ");
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //rel offset
	fp->write("itemlen, NULL)."); //itemlen, primitive effself
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(v); }\n");
      }

    if(isScalar() || WritePacked(getPackable()))
      {
	// write must be scalar; ref param to avoid excessive copying
	//not an array
	m_state.indent(fp);
	fp->write("void");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write("& targ) { UlamRef<EC>::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(targ); /* entire */ }\n");
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
    u32 bitsize = getBitSize();

    m_state.m_currentIndentLevel = 0;

    UTI anyuti = Nav;
    AssertBool anyDefined =  m_state.anyDefinedUTI(m_key, anyuti);
    assert(anyDefined);
    UTI scalaruti = m_state.getUlamTypeAsScalar(anyuti);
    UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
    const std::string scalarmangledName = scalarut->getUlamTypeMangledName();

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
    fp->write("BitVectorBitStorage");
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
    fp->write("typedef BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("> BV;\n");

    m_state.indent(fp);
    fp->write("typedef BitVectorBitStorage<EC, BV> BVS;\n");
    fp->write("\n");

    //put read/write methods before constructrtors that may use them.
    //read BV method
    genUlamTypeReadDefinitionForC(fp);

    //write BV method
    genUlamTypeWriteDefinitionForC(fp);

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(scalarut->getTmpStorageTypeAsString().c_str()); //u32, u64
    fp->write(" d) { ");
    if(isScalar())
      {
	//fp->write(writeMethodForCodeGen().c_str());
	fp->write("write");
	fp->write("(d); }\n");
      }
    else
      {
	fp->write("u32 n = ");
	fp->write_decimal(getArraySize());
	fp->write("u; while(n--) { ");
	fp->write("writeArrayItem(d, n, ");
	fp->write_decimal_unsigned(bitsize);
	fp->write("); }");
	fp->write(" }\n");
      }

    //copy constructor here (return by value)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str()); //u32
    fp->write("& other) { ");
    //fp->write(writeMethodForCodeGen().c_str());
    fp->write("write");
    fp->write("(other.");
    //fp->write(readMethodForCodeGen().c_str());
    fp->write("read");
    fp->write("()); }\n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

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
    //if(isScalar() || (getPackable() == PACKEDLOADABLE))
    if(isScalar() || WritePacked(getPackable()))
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32 or u64
	fp->write(" read");
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
    if(isScalar() || WritePacked(getPackable()))
      {
	m_state.indent(fp);
	fp->write("void write");
	fp->write("(const ");
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
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32, s64 or u64
	fp->write(" v, const u32 index, const u32 itemlen) { BVS::");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, "); //rel offset
	fp->write("itemlen, v)"); //itemlen, primitive effself
	fp->write("; }\n");
      }
  } //genUlamTypeWriteDefinitionForC

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
#if 0
    fp->write(" read() const { MFM_API_ASSERT_NONNULL(m_stgPtr); return Up_Us(*m_stgPtr, 0u, NULL)."); //origin 0u
#else
    fp->write(" read() const { MFM_API_ASSERT_NONNULL(m_stgPtr); return Up_Us(*m_stgPtr, NULL)."); //wo origin
#endif
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
