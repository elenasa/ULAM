#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClassQuark.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClassQuark::UlamTypeClassQuark(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeClass(key, state) { }

  bool UlamTypeClassQuark::isNumericType()
  {
    u32 quti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    return (m_state.quarkHasAToIntMethod(quti));
  } //isNumericType

  ULAMCLASSTYPE UlamTypeClassQuark::getUlamClassType()
  {
    return UC_QUARK;
  }

  bool UlamTypeClassQuark::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this); //tobe
    UTI valtypidx = val.getUlamValueTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(valtypidx);
    assert(vut->isScalar() && isScalar());
    ULAMTYPE vetyp = vut->getUlamTypeEnum();
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();

    //now allowing atoms to be cast as quarks, as well as elements;
    // also allowing subclasses to be cast as their superclass (u1.2.2)
    if(vetyp == UAtom)
      brtn = false; //cast atom to a quark ref (in eval)?
    else if(UlamType::compare(valtypidx, typidx, m_state) == UTIC_SAME)
      {
	//if same type nothing to do; if atom, shows as element in eval-land.
	//val.setAtomElementTypeIdx(typidx); //?
      }
    else if(m_state.isClassASubclassOf(valtypidx, typidx))
      {
	//2 quarks, or element (val), or transient, inherits from this quark
	if(vclasstype == UC_ELEMENT)
	  {
	    s32 pos = ATOMFIRSTSTATEBITPOS; //ancestors start at first state bit pos
	    s32 len = getTotalBitSize();
	    assert(len != UNKNOWNSIZE);
	    if(len <= MAXBITSPERINT)
	      {
		u32 qdata = val.getDataFromAtom(pos, len);
		val = UlamValue::makeImmediateClass(typidx, qdata, len);
	      }
	    else if(len <= MAXBITSPERLONG)
	      {
		m_state.abortNotSupported(); //quarks are max 32 bits
		u64 qdata = val.getDataLongFromAtom(pos, len);
		val = UlamValue::makeImmediateLongClass(typidx, qdata, len);
	      }
	    else
	      m_state.abortGreaterThanMaxBitsPerLong();
	  }
	else if(vclasstype == UC_TRANSIENT) //t3998, t3999, t41000, t41001, t41069
	  {
	    s32 pos = 0; //ancestors start at first bit pos
	    s32 len = getTotalBitSize();
	    assert(len != UNKNOWNSIZE);
	    if(len <= MAXBITSPERINT)
	      {
		u32 qdata = val.getDataFromAtom(pos, len);
		val = UlamValue::makeImmediateClass(typidx, qdata, len);
	      }
	    else if(len <= MAXBITSPERLONG)
	      {
		m_state.abortNotSupported(); //quarks are max 32 bits
		u64 qdata = val.getDataLongFromAtom(pos, len);
		val = UlamValue::makeImmediateLongClass(typidx, qdata, len);
	      }
	    else
	      m_state.abortGreaterThanMaxBitsPerLong();
	  }
	else
	  {
	    // both left-justified immediate quarks
	    // Coo c = (Coo) f.su; where su is a Soo : Coo
	    s32 vlen = vut->getTotalBitSize();
	    s32 len = getTotalBitSize();
	    u32 vdata = val.getImmediateClassData(vlen); //not from element
	    assert((vlen - len) >= 0); //sanity check
	    u32 qdata = vdata >> (vlen - len); //stays left-justified
	    val = UlamValue::makeImmediateClass(typidx, qdata, len);
	  }
      }
    else
      brtn = false;

    return brtn;
  } //end cast

  FORECAST UlamTypeClassQuark::explicitlyCastable(UTI typidx)
  {
    FORECAST scr = UlamTypeClass::explicitlyCastable(typidx);
    // as of updated ulam-3: atom or atomref, possibly ok when inherited (runtime)
    //if(scr == CAST_CLEAR)
    //check from atom or atomref, possibly ok for quark ref only (runtime)
    //if(m_state.isAtom(typidx) && !isReference()) scr = CAST_BAD;
    return scr;
  } //explicitlyCastable

  const char * UlamTypeClassQuark::getUlamTypeAsSingleLowercaseLetter()
  {
    return "q";
  }

  const std::string UlamTypeClassQuark::getUlamTypeUPrefix()
  {
    //scalar or array Sat Sep 30 16:05:48 2017
    return "Uq_";
  } //getUlamTypeUPrefix

  const std::string UlamTypeClassQuark::readMethodForCodeGen()
  {
    if(!isScalar())
      return UlamType::readMethodForCodeGen();
    return "Read"; //quarks only 32 bits
  }

  const std::string UlamTypeClassQuark::writeMethodForCodeGen()
  {
    if(!isScalar())
      return UlamType::writeMethodForCodeGen();
    return "Write"; //quarks only 32 bits
  }

  bool UlamTypeClassQuark::needsImmediateType()
  {
    if(!isComplete())
      return false;

    bool rtnb = false;
    if(!isReference())
      {
	rtnb = true;
	u32 id = m_key.getUlamKeyTypeSignatureNameId();
	u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(id);
	if(cnsym->isClassTemplate() && ((SymbolClassNameTemplate *) cnsym)->isClassTemplate(cuti))
	  rtnb = false; //the "template" has no immediate, only instances
      }
    return rtnb;
  } //needsImmediateType

  const std::string UlamTypeClassQuark::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName();
    ctype << "<EC>"; //default local quarks
    return ctype.str();
  } //getLocalStorageTypeAsString

  TMPSTORAGE UlamTypeClassQuark::getTmpStorageTypeForTmpVar()
  {
    return UlamTypeClass::getTmpStorageTypeForTmpVar();
  }

  const std::string UlamTypeClassQuark::castMethodForCodeGen(UTI nodetype)
  {
    std::ostringstream msg;
    msg << "Quarks only cast 'toInt': value type and size was: ";
    msg << m_state.getUlamTypeByIndex(nodetype)->getUlamTypeName().c_str();
    msg << ", to be: " << getUlamTypeName().c_str();
    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(),msg.str().c_str(), ERR);
    return "invalidQuarkCastMethodForCodeGen";
  } //castMethodForCodeGen

  void UlamTypeClassQuark::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    s32 len = getTotalBitSize(); //could be 0, includes arrays
    s32 bitsize = getBitSize();

    //class instance idx is always the scalar uti
    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();

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
    fp->write("\n");

    m_state.m_currentIndentLevel++;

    //forward declaration of quark (before struct!)
    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(scalarmangledName.c_str());
    fp->write(";  //forward"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(automangledName.c_str());
    fp->write(" : public UlamRef<EC>"); GCNL;
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    UlamType::genStandardConfigTypedefTypenames(fp, m_state);

    m_state.indent(fp);
    fp->write("enum { QUARK_SIZE = ");
    fp->write_decimal_unsigned(bitsize);
    fp->write("};"); GCNL;
    fp->write("\n");

    //quark typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;"); GCNL;

    //read 'entire quark' method
    genUlamTypeAutoReadDefinitionForC(fp);

    //write 'entire quark' method
    genUlamTypeAutoWriteDefinitionForC(fp);

    //constructor given storage
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const UlamContext<EC>& uc) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, targ, effself, ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::CLASSIC");
    fp->write(", uc) { }"); GCNL;

    //constructor for chain of refs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, effself, ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::CLASSIC");
    fp->write(") { }"); GCNL;

    //constructor for conditional-as (auto); superclass ref of element (t3617);
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage, const UlamContext<EC>& uc) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, targ, effself, ");
    fp->write("usage"); //controlled by caller
    fp->write(", uc) { }"); GCNL;

    //constructor for chain of autorefs
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, effself, ");
    fp->write("usage"); //controlled by caller
    fp->write(") { }"); GCNL;

    //(general) copy constructor here; pos relative to exisiting (i.e. same). t3788, t41153
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef");
    fp->write("<EC>& r) : UlamRef<EC>(r, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u) { }"); GCNL;

    //(exact) copy constructor; pos relative to exisiting (i.e. same).
    //t3617, t3631, t3668, t3669, t3672, t3689, t3692, t3693, t3697, t3746
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r) : UlamRef<EC>(r, r.GetLen()");
    fp->write(") { }"); GCNL;

    //default destructor (intentially left out)

    //declare away operator=
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("& operator=(const ");
    fp->write(automangledName.c_str());
    fp->write("& rhs); //declare away"); GCNL;

    // aref/aset calls generated inline for immediates.
    if(isCustomArray())
      {
	m_state.indent(fp);
	fp->write("/* a custom array ('Us' has aref, aset methods) */"); GCNL;
      }

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
  } //genUlamTypeQuarkMangledAutoDefinitionForC

  void UlamTypeClassQuark::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    if(isScalar() || WritePacked(getPackable()))
      {
	// ref param to avoid excessive copying
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write(" read() const { ");
	fp->write("return ");
	fp->write("UlamRef<EC>::");
	fp->write(readMethodForCodeGen().c_str()); //just the guts
	fp->write("(); /* entire quark */ }"); GCNL;
      }

    //scalar and entire PACKEDLOADABLE array handled by read method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	//reads an item of array
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return "); //was const after )
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //const ref, rel offset
	fp->write("itemlen, &");  //itemlen,
	fp->write(m_state.getTheInstanceMangledNameByIndex(scalaruti).c_str());
	fp->write(", UlamRef<EC>::CLASSIC).");
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(); }"); GCNL;
      }
  } //genUlamTypeAutoReadDefinitionForC

  void UlamTypeClassQuark::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    if(isScalar() || WritePacked(getPackable()))
      {
	//ref param to avoid excessive copying; not an array
	m_state.indent(fp);
	fp->write("void");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write("& targ) { UlamRef<EC>::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(targ); /* entire quark */ }"); GCNL;
      }

    //scalar and entire PACKEDLOADABLE array handled by write method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	// writes an item of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write("& v, const u32 index, const u32 itemlen) { ");
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //rel offset
	fp->write("itemlen, &");  //itemlen,
	fp->write(m_state.getTheInstanceMangledNameByIndex(scalaruti).c_str());
	fp->write(", UlamRef<EC>::CLASSIC).");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(v); }"); GCNL;
      }
  } //genUlamTypeAutoWriteDefinitionForC

  //builds the immediate definition
  void UlamTypeClassQuark::genUlamTypeMangledDefinitionForC(File * fp)
  {
    s32 len = getTotalBitSize(); //could be 0, includes arrays
    u32 bitsize = getBitSize();

    //class instance idx is always the scalar uti
    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
    const std::string mangledName = getUlamTypeImmediateMangledName();
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();

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
    fp->write("template<class EC> class ");
    fp->write(scalarmangledName.c_str());
    fp->write("; //forward"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    fp->write(" : public ");
    fp->write("BitVectorBitStorage"); //storage here!
    fp->write("<EC, BitVector<"); //left-just pos
    fp->write_decimal_unsigned(len); //no round up
    fp->write("> >\n");

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    UlamType::genStandardConfigTypedefTypenames(fp, m_state);

    m_state.indent(fp);
    fp->write("enum { QUARK_SIZE = ");
    fp->write_decimal_unsigned(bitsize);
    fp->write("};"); GCNL;

    u32 dqval = 0;
    bool hasDQ = m_state.getDefaultQuark(m_key.getUlamKeyTypeSignatureClassInstanceIdx(), dqval);

    m_state.indent(fp);
    fp->write("typedef BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("> BV;"); GCNL;

    m_state.indent(fp);
    fp->write("typedef BitVectorBitStorage<EC, BV> BVS;"); GCNL;
    fp->write("\n");

    if(!isScalar())
      {
	m_state.indent(fp);
	fp->write("typedef ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str());
	fp->write(" BVI;"); GCNL;

	m_state.indent(fp);
	fp->write("typedef BitVectorBitStorage<EC, BVI > BVIS;"); GCNL;
	fp->write("\n");
      }

    //quark typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;"); GCNL;
    fp->write("\n");

    //read/write methods before constructors in case used.
    genUlamTypeReadDefinitionForC(fp);

    genUlamTypeWriteDefinitionForC(fp);

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { ");
    if(hasDQ)
      {
	if(isScalar())
	  {
	    fp->write("write(Us::THE_INSTANCE.getDefaultQuark()); }");//t3776
	    GCNL;
	  }
	else
	  {
	    u32 arraysize = getArraySize();
	    fp->write("u32 tmpdq = Us::THE_INSTANCE.getDefaultQuark(); ");
	    fp->write("for(u32 j = 0; j < ");
	    fp->write_decimal_unsigned(arraysize);
	    fp->write("u; j++) ");
	    fp->write("writeArrayItem(tmpdq, j, ");
	    fp->write_decimal_unsigned(bitsize);
	    fp->write("); }"); GCNL;
	  }
      } //hasDQ
    else
      {
	fp->write(" }"); GCNL;
      }

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" d) { ");
    fp->write("write(d);"); //e.g. t3649
    fp->write(" }"); GCNL;

    // assignment copy constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str());
    fp->write("<EC> & arg) { ");
    fp->write("write(arg.");
    fp->write("read()); }"); GCNL;

    //constructor from ref of same type
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& d) { ");
    fp->write("write(d.read()); }"); GCNL;

    //default destructor (intentionally left out)

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

  void UlamTypeClassQuark::genUlamTypeReadDefinitionForC(File * fp)
  {
    u32 totbitsize = getTotalBitSize();
    if(totbitsize <= BITSPERATOM) //Big 96bit array is unpacked, but.. (t3969)
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32 or u64
	fp->write(" read() const { return BVS::");
	fp->write(readMethodForCodeGen().c_str());
	fp->write("(0u, ");
	if(isScalar())
	  {
	    fp->write("QUARK_SIZE); }"); GCNL; //done
	  }
	else
	  {
	    fp->write_decimal_unsigned(totbitsize); //incl ReadBig
	    fp->write("u); } //reads entire array"); GCNL;
	  }
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

    //scalar and entire PACKEDLOADABLE array handled by base class read method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	//reads an item of array
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return BVS::"); //was const after )
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, "); //const ref, rel offset
	fp->write("itemlen); }"); GCNL;  //itemlen,
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypeClassQuark::genUlamTypeWriteDefinitionForC(File * fp)
  {
    u32 totbitsize = getTotalBitSize();
    if(totbitsize <= BITSPERATOM) //Big 96bit array is unpacked, but.. (t3969)
      {
	m_state.indent(fp);
	fp->write("void ");
	fp->write("write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32, s64 or u64
	fp->write(" v) { BVS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(0u, ");

	if(isScalar())
	  {
	    fp->write("QUARK_SIZE, v); }"); GCNL;
	  }
	else
	  {
	    fp->write_decimal_unsigned(totbitsize); //incl WriteBig
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
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(0u, bv); ");
	fp->write("} //writes entire BV"); GCNL;
      }

    //scalar and entire PACKEDLOADABLE array handled by base class write method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	// writes an item of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write("& v, const u32 index, const u32 itemlen) { BVS::");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, "); //rel offset
	fp->write("itemlen, v); }"); GCNL; //itemlen, val
      }
  } //genUlamTypeQuarkWriteDefinitionForC

} //end MFM
