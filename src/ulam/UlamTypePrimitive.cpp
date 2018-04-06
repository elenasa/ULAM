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

  FORECAST UlamTypePrimitive::safeCast(UTI typidx)
  {
    //common to all primitive types:
    FORECAST scr = UlamType::safeCast(typidx); //default
    if(scr == CAST_CLEAR)
      {
	// primitives must be the same sizes when casting to a reference type
	if(isAltRefType() && !UlamType::checkReferenceCast(typidx))
	  scr = CAST_BAD;
      }
    //the rest is primitive-specific..
    return scr;
  }

  FORECAST UlamTypePrimitive::explicitlyCastable(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx); //default
    if(scr == CAST_CLEAR)
      {
	// primitives must be the same sizes when casting to a reference type
	if(isAltRefType() && !UlamType::checkReferenceCast(typidx))
	  scr = CAST_BAD;

	// strings cannot be cast explicitly to other primitive types, except Void (t3961)
	UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
	ULAMTYPE valtypEnum = fmut->getUlamTypeEnum();
	if((getUlamTypeEnum() != Void) && ((valtypEnum == String) ^ (getUlamTypeEnum() == String)))
	  scr = CAST_BAD;

	//only quarks may be cast to Ints, explicitly or not; requires toInt method (t3996)
	if(valtypEnum == Class)
	  {
	    ULAMCLASSTYPE vclasstype = fmut->getUlamClassType();
	    if(vclasstype != UC_QUARK)
	      scr = CAST_BAD;
	  }
      }
    return scr;
  } //explicitlyCastable

  bool UlamTypePrimitive::castTo32(UlamValue & val, UTI typidx)
  {
    m_state.abortShouldntGetHere();
    //std::cerr << "UlamType (cast) error! " << std::endl;
    return false;
  }

  bool UlamTypePrimitive::castTo64(UlamValue & val, UTI typidx)
  {
    m_state.abortShouldntGetHere();
    //std::cerr << "UlamType (cast) error! " << std::endl;
    return false;
  }

  s32 UlamTypePrimitive::getDataAsCs32(const u32 data)
  {
    m_state.abortShouldntGetHere();
    return (s32) data;
  }

  u32 UlamTypePrimitive::getDataAsCu32(const u32 data)
  {
    m_state.abortShouldntGetHere();
    return data;
  }

  s64 UlamTypePrimitive::getDataAsCs64(const u64 data)
  {
    m_state.abortShouldntGetHere();
    return (s64) data;
  }

  u64 UlamTypePrimitive::getDataAsCu64(const u64 data)
  {
    m_state.abortShouldntGetHere();
    return data;
  }

  s32 UlamTypePrimitive::bitsizeToConvertTypeTo(ULAMTYPE tobUT)
  {
    return UNKNOWNSIZE; //atom, class, nav, ptr, holder
  }

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

  TMPSTORAGE UlamTypePrimitive::getTmpStorageTypeForTmpVar()
  {
    //immediate storage is TMPBITVAL for all UlamTypes.
    return UlamType::getTmpStorageTypeForTmpVar(); //TMPREGISTER
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
    fp->write(" : public UlamRef<EC>\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    UlamType::genStandardConfigTypedefTypenames(fp, m_state);

    genUlamTypeAutoReadDefinitionForC(fp);

    genUlamTypeAutoWriteDefinitionForC(fp);

    //constructor for ref (auto); wo origin
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, const UlamContext<EC>& uc) : UlamRef<EC>(idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, targ, NULL, "); //effself is null for primitives
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::PRIMITIVE");
    fp->write(", uc) { }"); GCNL;

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 idx) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, NULL, "); //effself is null for primitives
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::PRIMITIVE");
    fp->write(") { }"); GCNL;

    //copy constructor
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str()); //(was UlamRef)
    fp->write("<EC>& arg) : UlamRef<EC>(arg, 0, arg.GetLen(), NULL, ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::PRIMITIVE");
    fp->write(") { ");
    fp->write("MFM_API_ASSERT_ARG(arg.GetLen() == ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("); }"); GCNL; //effself is null for primitives

    //default destructor (intentially left out)

    //declare away operator=
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("& operator=(const ");
    fp->write(automangledName.c_str());
    fp->write("& rhs); //declare away"); GCNL;

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
	fp->write("itemlen, NULL, UlamRef<EC>::PRIMITIVE)."); //itemlen, primitive effself
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(); }"); GCNL;
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
	fp->write("(); /* entire */ }"); GCNL;
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
	fp->write("itemlen, NULL, UlamRef<EC>::PRIMITIVE)."); //itemlen, primitive effself
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(v); }"); GCNL;
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
	fp->write("(targ); /* entire */ }"); GCNL;
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
    else
      m_state.abortGreaterThanMaxBitsPerLong();
    return m_max;
  } //getMax (UlamValue)

  s64 UlamTypePrimitive::getMin(UlamValue& rtnUV, UTI uti)
  {
    u32 wordsize = getTotalWordSize();
    if(wordsize <= MAXBITSPERINT)
      rtnUV = UlamValue::makeImmediate(uti, (s32) m_min, m_state);
    else if(wordsize <= MAXBITSPERLONG)
      rtnUV = UlamValue::makeImmediateLong(uti, (s64) m_min, m_state);
    else
      m_state.abortGreaterThanMaxBitsPerLong();
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

    m_state.m_currentIndentLevel = 0;

    UTI anyuti = Nav;
    AssertBool anyDefined =  m_state.anyDefinedUTI(m_key, anyuti);
    assert(anyDefined);
    UTI scalaruti = m_state.getUlamTypeAsScalar(anyuti);
    UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
    const std::string scalarmangledName = scalarut->getUlamTypeMangledName();
    const std::string mangledName = getUlamTypeImmediateMangledName();
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();

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
    UlamType::genStandardConfigTypedefTypenames(fp, m_state);

    m_state.indent(fp);
    fp->write("typedef BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("> BV;"); GCNL;

    m_state.indent(fp);
    fp->write("typedef BitVectorBitStorage<EC, BV> BVS;"); GCNL;
    fp->write("\n");

    //put read/write methods before constructrtors that may use them.
    //read BV method
    genUlamTypeReadDefinitionForC(fp);

    //write BV method
    genUlamTypeWriteDefinitionForC(fp);

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { }"); GCNL;

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64
    fp->write(" d) { ");
    fp->write("write(d); }"); GCNL;

    //array initialization constructor here (used by const tmpVars);
    // in C, the array is just a pointer (since not within a struct);
    if(!isScalar())
      {
	m_state.indent(fp);
	fp->write(mangledName.c_str());
	fp->write("(const u32");
	fp->write(" d[");
	fp->write_decimal_unsigned(UlamType::getTotalNumberOfWords());
	fp->write("]) : BVS(d) { }"); GCNL;
      }

    //copy constructor here (return by value)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str()); //u32
    fp->write("& other) { ");
    fp->write("this->write");
    fp->write("(other.");
    fp->write("read");
    fp->write("()); }"); GCNL;

    //constructor from ref of same type
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& d) { "); //uc consistent with atomref
    fp->write("this->write(");
    fp->write("d.read()); }"); GCNL;

    //default destructor (intentionally left out

    //for var args native funcs, non-refs, required of a BitStorage
    UlamType::genGetUlamTypeMangledNameDefinitionForC(fp);

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
    u32 totbitsize = getTotalBitSize();
    if(totbitsize <= BITSPERATOM) //Big 96bit array is unpacked, but.. (t3969)
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32 or u64
	fp->write(" read");
	fp->write("() const { return BVS::");
	fp->write(readMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal_unsigned(totbitsize); //incl ReadBig
	if(isScalar())
	  {
	    fp->write("u); }"); GCNL; //done
	  }
	else
	  {
	    fp->write("u); } //reads entire array"); GCNL;
	  }
      }
    else
      {
	//UNPACKED (e.g. t3975)
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write(" read");
	fp->write("() const { ");
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write(" rtnunpbv; this->BVS::");
	fp->write("ReadBV(0u, rtnunpbv); return rtnunpbv; ");
	fp->write("} //reads entire BV"); GCNL;
      }

    if(!isScalar())
      {
	//reads an item of array;
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return ");
	fp->write("this->BVS::");
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, itemlen); } //reads BV array item"); GCNL;
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypePrimitive::genUlamTypeWriteDefinitionForC(File * fp)
  {
    u32 totbitsize = getTotalBitSize();
    if(totbitsize <= BITSPERATOM) //Big 96bit array is unpacked, but.. (t3969)
      {
	m_state.indent(fp);
	fp->write("void write");
	fp->write("(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32, s64 or u64
	fp->write("& v) { BVS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal_unsigned(totbitsize); //incl WriteBig
	if(isScalar())
	  {
	    fp->write("u, v); }"); GCNL;
	  }
	else
	  {
	    fp->write("u, v); } //writes entire array"); GCNL;
	  }
      }
    else
      {
	//UNPACKED
	m_state.indent(fp);
	fp->write("void ");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write("& bv) { BVS::");
	fp->write("WriteBV(0u, bv); ");
	fp->write("} //writes entire BV"); GCNL;
      }

    if(!isScalar())
      {
	//reads an item of array;
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write("& v, const u32 index, const u32 itemlen) {");
	fp->write("this->BVS::");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, itemlen, v); } //writes BV array item"); GCNL;
      }
  } //genUlamTypeWriteDefinitionForC

  void UlamTypePrimitive::genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp)
  {
    assert(isScalar()); //since arrays cannot be initialized

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
    UlamType::genStandardConfigTypedefTypenames(fp, m_state);

    s32 len = getTotalBitSize();

    //reference to storage in atom
    m_state.indent(fp);
    fp->write("T* m_stgPtr;  //ptr to storage here!"); GCNL;

    // default constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stgPtr(NULL) { }"); GCNL;

    m_state.indent(fp);
    fp->write("void init(T& realStg) { m_stgPtr = &realStg; }"); GCNL;

    //read method: MFM model parameters are right justified in the atom
    // NO write method for MPs in ulam. t3259
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32, s64 or u64
    fp->write(" read(const UlamContext<EC>& uc) const { MFM_API_ASSERT_NONNULL(m_stgPtr); AtomRefBitStorage<EC> mpfoo(*m_stgPtr); return UlamRef<EC>(BPA - ");
    fp->write_decimal(len); //by convention at the end of the atom
    fp->write("u, ");
    fp->write_decimal(len);
    fp->write("u, mpfoo, NULL, UlamRef<EC>::PRIMITIVE, uc)."); //origin 0u
    fp->write(readMethodForCodeGen().c_str());
    fp->write("(); }"); GCNL;

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

} //end MFM
