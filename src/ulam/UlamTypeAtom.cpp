#include <sstream>
#include <stdio.h>
#include "UlamTypeAtom.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeAtom::UlamTypeAtom(const UlamKeyTypeSignature key, CompilerState & state) : UlamType(key, state)
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
  }

  ULAMTYPE UlamTypeAtom::getUlamTypeEnum()
  {
    return UAtom;
  }

  ULAMCLASSTYPE UlamTypeAtom::getUlamClassType()
  {
    //return UC_ATOM; //llok into this!!
    return UC_NOTACLASS;
  }

  bool UlamTypeAtom::needsImmediateType()
  {
    return isComplete(); //t3671
  }

  const std::string UlamTypeAtom::getArrayItemTmpStorageTypeAsString()
  {
    return "T";
  }

  const std::string UlamTypeAtom::getTmpStorageTypeAsString()
  {
    if(!isScalar())
      {
	std::ostringstream cstr;
	cstr << "BitVector<" << getTotalBitSize() << ">"; //may be zero if array[0]
	return cstr.str();
      }
    return "T";
  } //getTmpStorageTypeAsString

  const std::string UlamTypeAtom::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName() << "<EC>";
    return ctype.str();
  }

  TMPSTORAGE UlamTypeAtom::getTmpStorageTypeForTmpVar()
  {
    return TMPTATOM; //per array item/per atom
  }

  bool UlamTypeAtom::isMinMaxAllowed()
  {
    return false;
  }

  PACKFIT UlamTypeAtom::getPackable()
  {
    return UNPACKED;
  }

  bool UlamTypeAtom::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();
    UlamType * fmut = m_state.getUlamTypeByIndex(valtypidx);
    AssertBool isScalars = (fmut->isScalar() && isScalar());
    assert(isScalars);

    // what change is to be made ????
    // atom type vs. class type
    // how can it be both in an UlamValue?
    // what of its contents?
    // val = UlamValue::makeAtom(valtypidx);

    if((fmut->getUlamClassType() == UC_ELEMENT) || m_state.isAtom(valtypidx) || ((fmut->getUlamTypeEnum() == Class) && fmut->isAltRefType()))
      val.setUlamValueTypeIdx(typidx); //try this
    else
      brtn = false;
    return brtn;
  } //end cast

  FORECAST UlamTypeAtom::safeCast(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    UlamType * fmut = m_state.getUlamTypeByIndex(typidx);

    if(m_state.isAtom(typidx))
      return CAST_CLEAR; //atom/ref to atom/ref

    //casting from quark or transient to atom or atomref is bad
    ULAMCLASSTYPE fclasstype = fmut->getUlamClassType();

    // casting from quark ref needs .atomof (error t3696)
    // elements no longer packed, ok t3753
    return (fclasstype == UC_ELEMENT) ? CAST_CLEAR : CAST_BAD;
   } //safeCast

  FORECAST UlamTypeAtom::explicitlyCastable(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx); //default, arrays checked
    if(scr == CAST_CLEAR)
      {
	UlamType * fmut = m_state.getUlamTypeByIndex(typidx);
	ULAMCLASSTYPE fclasstype = fmut->getUlamClassType();
	if(fmut->isPrimitiveType())
	  scr = CAST_BAD;
	else if((fclasstype == UC_QUARK) && !fmut->isAltRefType())
	  //else if((vclasstype == UC_QUARK)) //WAIT UNTIL ULAM-4. to always use .atomof (t41153, 3697, t3834, t3691)
	  scr = CAST_BAD; //non-ref quark to atom is also bad (t3678, t41154)
	else if((fclasstype == UC_TRANSIENT))
	  scr = CAST_BAD; //transient to atom is also bad
	//else atom, element, element ref, quark ref (possibly), are acceptable
      }
    return scr;
  } //explicitlyCastable

  const std::string UlamTypeAtom::castMethodForCodeGen(UTI nodetype)
  {
    std::ostringstream rtnMethod;
    UlamType * nut = m_state.getUlamTypeByIndex(nodetype);

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    u32 sizeByIntBitsToBe = getTotalWordSize();
    u32 sizeByIntBits = nut->getTotalWordSize();

    assert(sizeByIntBitsToBe == sizeByIntBits);
    assert(nut->getUlamClassType() == UC_ELEMENT); //quarks only cast toInt

    rtnMethod << "_" << "Element"  << sizeByIntBits << "To";
    rtnMethod << getUlamTypeNameOnly().c_str() << sizeByIntBitsToBe;
    return rtnMethod.str();
  } //castMethodForCodeGen

  // version of Atom& based on an EventWindow with an array of AtomBitStorage
  void UlamTypeAtom::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    if(!isScalar())
      return genUlamTypeMangledUnpackedArrayAutoDefinitionForC(fp);

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

    //read 'entire atom' method
    genUlamTypeAutoReadDefinitionForC(fp);

    //write 'entire atom' method
    genUlamTypeAutoWriteDefinitionForC(fp);

    //constructor for ref(auto) (e.g. t3407, 3638, 3639, 3655, 3656, 3657, 3663, 3684, 3692)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 startidx, const UlamContext<EC>& uc) : UlamRef<EC>(startidx, BPA, targ, uc.LookupUlamElementTypeFromContext(targ.ReadAtom(startidx).GetType()), UlamRef<EC>::ATOMIC, uc) { }"); GCNL;

    //copy constructor for autoref (chain would be unpacked array,
    // e.g. 3812 requires NULL effself)
    // no extra uc, consistent with other types now.
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 idx) : UlamRef<EC>(arg, idx, BPA, NULL, UlamRef<EC>::ATOMIC) { }"); GCNL;

    //copy constructor (non-const), t3701, t3735, t3753,4,5,6,7,8,9
    // required by EventWindow aref method (ulamexports)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const "); //??
    fp->write(automangledName.c_str());
    fp->write("& arg) : UlamRef<EC>(arg, 0, arg.GetLen(), arg.GetEffectiveSelf(), UlamRef<EC>::ATOMIC) { }"); GCNL; //t3818, t3820, t3910 STALE_ATOM_REF

    //default destructor (intentionally left out)

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

  void UlamTypeAtom::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    assert(isScalar());
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write(" read() const { ");
    fp->write("return UlamRef<EC>::");
    fp->write(readMethodForCodeGen().c_str());
    fp->write("(); /* read entire atom */ }"); GCNL; //done
  } //genUlamTypeAutoReadDefinitionForC

  void UlamTypeAtom::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    assert(isScalar());
    //ref param to avoid excessive copying
    m_state.indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T or BV
    fp->write("& v) { ");
    fp->write("UlamRef<EC>::");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(v); /* write entire atom */ }"); GCNL; //done

    m_state.indent(fp);
    fp->write("void write(const ");
    fp->write("AtomRefBitStorage<EC>");
    fp->write("& v) { ");
    fp->write("UlamRef<EC>::");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(v.ReadAtom()); /* write entire atom */ }"); GCNL; //done
  } //genUlamTypeAutoWriteDefinitionForC

  //generates immediate for whole atom
  void UlamTypeAtom::genUlamTypeMangledDefinitionForC(File * fp)
  {
    if(!isScalar())
      return genUlamTypeMangledUnpackedArrayDefinitionForC(fp);

    m_state.m_currentIndentLevel = 0;
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
    fp->write("AtomBitStorage<EC>\n");

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    UlamType::genStandardConfigTypedefTypenames(fp, m_state);

    m_state.indent(fp);
    fp->write("typedef AtomRefBitStorage<EC> ABS;"); GCNL;
    fp->write("\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : AtomBitStorage<EC>");
    fp->write("(Element_Empty<EC>::THE_INSTANCE.GetDefaultAtom()) { }"); GCNL;

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write("& targ) : "); //uc consistent with atomref
    fp->write("AtomBitStorage<EC>");
    fp->write("(targ) { }"); GCNL;

    //copy constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write("AtomRefBitStorage<EC>");
    fp->write("& d) : "); //uc consistent with atomref
    fp->write("AtomBitStorage<EC>");
    fp->write("(d.ReadAtom()) { }"); GCNL;

    //copy constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str());
    fp->write("& d) : "); //uc consistent with atomref
    fp->write("AtomBitStorage<EC>");
    fp->write("(d.read()) { }"); GCNL;

    //constructor from ref of same type
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& d) : "); //uc consistent with atomref
    fp->write("AtomBitStorage<EC>");
    fp->write("(d.read()) { }"); GCNL;

    //default destructor (intentionally left out)

    //note: no operator= since base class has a T&

    genUlamTypeReadDefinitionForC(fp);

    genUlamTypeWriteDefinitionForC(fp);

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

  void UlamTypeAtom::genUlamTypeReadDefinitionForC(File * fp)
  {
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //T
	fp->write(" read() const { ");
	fp->write("return ABS::");
	fp->write(readMethodForCodeGen().c_str());
	fp->write("(); }"); GCNL; //done
      }
    else
      {
	//UNPACKED
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write(" read");
	fp->write("() const { ");
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write(" rtnunpbv; this->BVS::");
	fp->write(readMethodForCodeGen().c_str());
	fp->write("(0u, rtnunpbv); return rtnunpbv; ");
	fp->write("} //reads entire BV"); GCNL;
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypeAtom::genUlamTypeWriteDefinitionForC(File * fp)
  {
    //ref param to avoid excessive copying
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("void write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //T, BV96
	fp->write("& v) { ");
	fp->write("ABS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(v); }"); GCNL; //done

	m_state.indent(fp);
	fp->write("void write(const ");
	fp->write("AtomRefBitStorage<EC>");
	fp->write("& v) { ");
	fp->write("ABS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(v.ReadAtom()); }"); GCNL; //done
      }
    else
      {
	//UNPACKED
	m_state.indent(fp);
	fp->write("void ");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write("& bv) { BVS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(0u, bv); ");
	fp->write("} //writes entire BV"); GCNL;
      }
  } //genUlamTypeWriteDefinitionForC

  const std::string UlamTypeAtom::readMethodForCodeGen()
  {
    if(!isScalar())
      return "ReadBV";
    return "ReadAtom"; //GetStorage?
  }

  const std::string UlamTypeAtom::writeMethodForCodeGen()
  {
    if(!isScalar())
      return "WriteBV";
    return "WriteAtom";
  }

  const std::string UlamTypeAtom::readArrayItemMethodForCodeGen()
  {
    return "ReadAtom";
  }

  const std::string UlamTypeAtom::writeArrayItemMethodForCodeGen()
  {
    return "WriteAtom";
  }

  void UlamTypeAtom::genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp)
  {
    s32 len = getTotalBitSize(); //could be 0, includes arrays
    s32 arraysize = getArraySize();
    assert(arraysize >= 0); //zero-length array is legal to declare, but not access

    m_state.m_currentIndentLevel = 0;
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();
    const std::string mangledName = getUlamTypeImmediateMangledName();
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
    fp->write("\n");

    m_state.m_currentIndentLevel++;

    //forward declaration of atom array (before struct!)
    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(mangledName.c_str());
    fp->write(";  //forward"); GCNL;
    fp->write("\n");

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

    //constructor for conditional-as (auto)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, const UlamContext<EC>& uc) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, targ, NULL, UlamRef<EC>::ARRAY, uc) { }"); GCNL; //no effective self

    //constructor with exact same immediate storage, e.g. t3671
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("("); //non-const for ref
    fp->write(mangledName.c_str());
    fp->write("<EC>& s, u32 idx, const UlamContext<EC>& uc) : UlamRef<EC>(idx, ");//0u, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, s, NULL, UlamRef<EC>::ARRAY, uc) { }"); GCNL; //no effective self

    //copy constructor here (uc arg unused)
    //no uc for ref constructor, consistent with other types now
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r, u32 idx) : UlamRef<EC>(r, idx, r.GetLen(), r.GetEffectiveSelf(), UlamRef<EC>::ARRAY) { }"); GCNL;

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, effself, UlamRef<EC>::ARRAY) {}"); GCNL;

    //copy constructor
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("& arg) : UlamRef<EC>(arg, 0, arg.GetLen(), arg.GetEffectiveSelf(), UlamRef<EC>::ARRAY) { }"); GCNL;

    //default destructor (intentionally left out)

    //read 'entire atom' method
    genUlamTypeAutoReadArrayDefinitionForC(fp);

    //write 'entire atom' method
    genUlamTypeAutoWriteArrayDefinitionForC(fp);

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

  void UlamTypeAtom::genUlamTypeAutoReadArrayDefinitionForC(File * fp)
  {
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write(" read() const { ");
    fp->write("return UlamRef<EC>::");
    fp->write(readMethodForCodeGen().c_str());
    fp->write("(); /* read entire atom array */ }"); GCNL; //done

    //Unpacked Read Array Item
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
    fp->write(" readArrayItem(");
    fp->write("const u32 index, const u32 itemlen, const UlamContext<EC>& uc) const { return ");
    fp->write("UlamRef<EC>(");
    fp->write("*this, index * itemlen, "); //const ref, rel offset
    fp->write("itemlen, ");  //itemlen,
    fp->write("uc.LookupUlamElementTypeFromContext(this->GetType()), UlamRef<EC>::ATOMIC)."); //effself
    fp->write(readArrayItemMethodForCodeGen().c_str());
    fp->write("(); /* entire atom item */ }"); GCNL;
  } //genUlamTypeAutoReadArrayDefinitionForC

  void UlamTypeAtom::genUlamTypeAutoWriteArrayDefinitionForC(File * fp)
  {
    //ref param to avoid excessive copying
    m_state.indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T or BV
    fp->write("& v) { ");
    fp->write("UlamRef<EC>::");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(v); /* write entire atom array */ }"); GCNL; //done

    //Unpacked Write Array Item
    m_state.indent(fp);
    fp->write("void writeArrayItem(const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
    fp->write("& v, const u32 index, const u32 itemlen, const UlamContext<EC>& uc) { ");
    fp->write("UlamRef<EC>(");
    fp->write("*this, index * itemlen, "); //rel offset
    fp->write("itemlen, ");  //itemlen,
    fp->write("uc.LookupUlamElementTypeFromContext(v.GetType()), UlamRef<EC>::ATOMIC)."); //effself
    fp->write(writeArrayItemMethodForCodeGen().c_str());
    fp->write("(v); /* entire atom item */ }"); GCNL;
  } //genUlamTypeAutoWriteArrayDefinitionForC

  void UlamTypeAtom::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    s32 len = getTotalBitSize(); //could be 0, includes arrays
    s32 arraysize = getArraySize();

    assert(arraysize >= 0); //zero-length array is legal to declare, but not access

    m_state.m_currentIndentLevel = 0;

    //class instance idx is always the scalar uti
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
    fp->write("BitVectorBitStorage"); //storage here!
    fp->write("<EC, BitVector<"); //left-just pos
    fp->write_decimal_unsigned(len); //no round up
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

    //default constructor (used by local vars) new way!
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { ");
    fp->write("AtomBitStorage<EC> tmp(Element_Empty<EC>::THE_INSTANCE.GetDefaultAtom()); ");
    fp->write("BV96 tmpval = tmp.ReadBig(0u, BPA); ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("BVS::WriteBig(j * BPA, BPA, tmpval); }"); GCNL;

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write("T& targ) { ");
    fp->write("AtomBitStorage<EC> tmp(targ); ");
    fp->write("BV96 tmpval = tmp.ReadBig(0u, BPA); ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("BVS::WriteBig(j * BPA, BPA, tmpval); }"); GCNL;

    //the whole thing (e.g. t3709)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //BV
    fp->write("& d) { ");
    fp->write("write(d);");
    fp->write(" }"); GCNL;

    // assignment constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str());
    fp->write("<EC> & arg) { ");
    fp->write("this->m_stg = arg.m_stg; }"); GCNL;

    //assignment constructor, atom arg, for convenience
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const AtomRefBitStorage<EC> & arg) { ");
    fp->write("BV96 tmpval = arg.ReadBig(0u, BPA); ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("BVS::WriteBig(j * BPA, BPA, tmpval); }"); GCNL;

    //default destructor (intentionally left out)

    genUlamTypeReadDefinitionForC(fp);

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
  } //genUlamTypeMangledUnpackedArrayDefinitionForC

} //end MFM
