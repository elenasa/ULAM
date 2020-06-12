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
    UlamType * fmut = m_state.getUlamTypeByIndex(valtypidx);
    assert(fmut->isScalar() && isScalar());
    ULAMTYPE vetyp = fmut->getUlamTypeEnum();

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
	s32 len = getTotalBitSize();
	assert(len != UNKNOWNSIZE);
	if(len > MAXBITSPERINT)
	  m_state.abortNotSupported(); //quarks are max 32 bits
	UlamValue newval = UlamValue::makeImmediateClass(typidx, 0, len);
	m_state.extractQuarkBaseFromSubclassForEval(val, typidx, newval);
	val = newval;
      }
    else
      brtn = false;

    return brtn;
  } //end cast

  FORECAST UlamTypeClassQuark::explicitlyCastable(UTI typidx)
  {
    FORECAST scr = UlamTypeClass::explicitlyCastable(typidx);
    // as of updated ulam-3: atom or atomref, possibly ok when inherited (runtime)
    return scr;
  } //explicitlyCastable

  const char * UlamTypeClassQuark::getUlamTypeAsSingleLowercaseLetter()
  {
    return "q";
  }

  const std::string UlamTypeClassQuark::getUlamTypeUPrefix()
  {
    return "Uq_"; //scalar or array Sat Sep 30 16:05:48 2017
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
    if(!isAltRefType())
      {
	rtnb = true;
	u32 id = getUlamTypeNameId();
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
    s32 baselen = isScalar() ? getBitsizeAsBaseClass() : len; //could be 0, default when effself not self (ulam-5)
    s32 bitsize = getBitSize();

    //class instance idx is always the scalar uti
    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
    const std::string scalarmangledName = scalarut->getUlamTypeMangledName();
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();
    const std::string mangledName = scalarut->getUlamTypeImmediateMangledName();
    m_state.m_currentIndentLevel = 0;

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

    //forward declaration of quark and its immediate (before struct!)
    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(scalarmangledName.c_str());
    fp->write(";  //forward"); GCNL;

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

    //immediate typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(mangledName.c_str());
    fp->write("<EC> Usi;"); GCNL;

    fp->write("\n");

    //read 'entire quark' method
    genUlamTypeAutoReadDefinitionForC(fp);

    //write 'entire quark' method
    genUlamTypeAutoWriteDefinitionForC(fp);

    //keep this one too?
    //constructor given storage
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const UlamContext<EC>& uc) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    if(len != baselen)
      {
	fp->write("&Us::THE_INSTANCE==effself ? ");
	fp->write_decimal_unsigned(len); //includes arraysize
	fp->write("u : ");
	fp->write_decimal_unsigned(baselen); //base class
      }
    else
      fp->write_decimal_unsigned(len); //includes arraysize

    fp->write("u, targ, effself, ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::CLASSIC");
    fp->write(", uc) { }"); GCNL;

    //constructor for conditional-as (auto); super/baseclass ref of
    //element (t3617, t3735);
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, u32 postoeff, const UlamClass<EC>* effself, const UlamContext<EC>& uc) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    fp->write_decimal_unsigned(baselen); //baseclass
    fp->write("u, postoeff, targ, effself, ");
    fp->write("UlamRef<EC>::CLASSIC");
    fp->write(", uc) { }"); GCNL;

    //short-hand from immediate to ref of same type
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write(automangledName.c_str());
	fp->write("(");
	fp->write(mangledName.c_str());
	fp->write("<EC>& qarg, const UlamContext<EC> & uc) : UlamRef<EC>");
	fp->write("(0, "); //pos
	fp->write_decimal_unsigned(len);
	fp->write("u, qarg, & Us::THE_INSTANCE, UlamRef<EC>::CLASSIC, uc)");
	fp->write(" { }"); GCNL;
      }

    //constructor for chain of refs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 incr, const UlamClass<EC>* effself) : UlamRef<EC>(arg, incr, ");
    if(len != baselen)
      {
	fp->write("& Us::THE_INSTANCE==effself ? ");
	fp->write_decimal_unsigned(len); //includes arraysize
	fp->write("u : ");
	fp->write_decimal_unsigned(baselen); //base class
      }
    else
      fp->write_decimal_unsigned(len);

    fp->write("u, effself, ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::CLASSIC");
    fp->write(") { }"); GCNL;

    //(general) copy constructor here; pos relative to existing (i.e. same). t3788, t41153
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef");
    fp->write("<EC>& r) : UlamRef<EC>(r,0,r.GetLen()) {}"); GCNL;

    //(general) copy constructor here; base pos relative to existing (t3697)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef");
    fp->write("<EC>& r, s32 incr) : UlamRef<EC>(r, incr, ");
    if(len != baselen)
      {
	fp->write("&Us::THE_INSTANCE==r.GetEffectiveSelfPointer() ? ");
	fp->write_decimal_unsigned(len); //includes arraysize
	fp->write("u : ");
	fp->write_decimal_unsigned(baselen); //base class
      }
    else
      fp->write_decimal_unsigned(len);

    fp->write("u, r.GetEffectiveSelf(), ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::CLASSIC");
    fp->write(") { }"); GCNL;

    //(exact) copy constructor; pos relative to existing (i.e. same).
    //t3617, t3631, t3668, t3669, t3672, t3689, t3692, t3693, t3697, t3746
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r) : UlamRef<EC>(r,0,");
    if(len != baselen)
      {
	fp->write("&Us::THE_INSTANCE==r.GetEffectiveSelfPointer() ? ");
	fp->write("r.GetLen() : ");
	fp->write_decimal_unsigned(baselen); //base class
	fp->write("u");
      }
    else
      fp->write("r.GetLen()");
    fp->write(") { }"); GCNL;

    //default destructor (intentially left out)

    //declare away operator=
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("& operator=(const ");
    fp->write(automangledName.c_str());
    fp->write("& rhs); //declare away"); GCNL;

    // aref calls generated inline for immediates.
    if(isCustomArray())
      {
	m_state.indent(fp);
	fp->write("/* a custom array ('Us' has aref method) */"); GCNL;
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
	//ref param to avoid excessive copying; (ulam-5) use deref copy cnstr
	// when a different effSelf (may be spread out).
	m_state.indent(fp);
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write(" read() const { ");
	fp->write("if(&Us::THE_INSTANCE==this->GetEffectiveSelfPointer()) ");
	fp->write("return UlamRef<EC>::");
	fp->write(readMethodForCodeGen().c_str()); //just the guts
	fp->write("();/* entire quark */");
	fp->write("else { return Usi(*this).read(); } }"); GCNL;
      }

    //scalar and entire PACKEDLOADABLE array handled by read method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	//reads an item of array;
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
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
    if(isScalar())
      {
	//ref param to avoid excessive copying; not an array
	m_state.indent(fp);
	fp->write("void");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write("& targ) { ");
	if(getBitSize() > 0)
	  {
	    fp->write("if(&Us::THE_INSTANCE==this->GetEffectiveSelfPointer()) ");
	    fp->write("UlamRef<EC>::");
	    fp->write(writeMethodForCodeGen().c_str());
	    fp->write("(targ); /* entire quark */ ");
	    fp->write("else { ");
	    fp->write("BitVector<QUARK_SIZE> tmpbv(targ); ");

	    //write the data members first
	    //here.. 'd' UlamRef is initially pointing to them.
	    u32 myblen = getBitsizeAsBaseClass();
	    if(myblen > 0)
	      {
		fp->write("/*data members first*/ ");
		fp->write("UlamRef<EC>(*this,0,"); //t3172
		fp->write_decimal_unsigned(myblen);
		fp->write("u).Write(");
		fp->write("tmpbv.Read(0u,");
		fp->write_decimal_unsigned(myblen);
		fp->write("u)); ");
	      }

	    //then, write each of its non-zero size (shared) base classes
	    //class instance idx is always the scalar uti
	    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	    SymbolClass * csym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(scalaruti, csym);
	    assert(isDefined);
	    u32 shbasecount = csym->getSharedBaseClassCount();
	    if(shbasecount > 0)
	      fp->write("/*nonzero base classes*/ ");
	    u32 j = 0;
	    while(j < shbasecount)
	      {
		UTI baseuti = csym->getSharedBaseClass(j);
		u32 blen = m_state.getBaseClassBitSize(baseuti);
		if(blen > 0)
		  {
		    fp->write("UlamRef<EC>(*this, this->GetEffectiveSelf()->");
		    fp->write(m_state.getGetRelPosMangledFunctionName(baseuti));
		    fp->write("(");
		    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(baseuti));
		    fp->write("u)- this->GetPosToEffectiveSelf(),");
		    fp->write_decimal_unsigned(blen);
		    fp->write("u).Write(tmpbv.Read("); //no true t41358
		    fp->write_decimal_unsigned(csym->getSharedBaseClassRelativePosition(j));
		    fp->write("u,");
		    fp->write_decimal_unsigned(blen);
		    fp->write("u)); /*");
		    fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str());
		    fp->write(" */ ");
		  }
		j++;
	      } //end while
	    fp->write("} }"); GCNL;
	  }
	else
	  {
	    fp->write("/* noop */ ");
	    fp->write("}"); GCNL;
	  }
      }
    else if(WritePacked(getPackable()))
      {
	//ref param to avoid excessive copying; an array (t3845)
	m_state.indent(fp);
	fp->write("void");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write("& targ) { ");
	fp->write("UlamRef<EC>::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(targ); /* entire quark */ ");
	fp->write("}"); GCNL;
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

    //forward declaration of quark and immediate ref (before struct)
    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(scalarmangledName.c_str());
    fp->write("; //forward"); GCNL;

    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(automangledName.c_str());
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
    bool hasDQ = m_state.getDefaultQuark(scalaruti, dqval);

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
    fp->write("<EC> & arg) : BVS() { "); //Wextra
    fp->write("write(arg.");
    fp->write("read()); }"); GCNL;

    //constructor for constants
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const u32 * const ");
    fp->write(" arg) : BVS(arg) { if(arg==NULL) FAIL(NULL_POINTER); }"); GCNL;

    //constructor from ref of same type (e.g. effself subclass)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& d) { ");
    if(isScalar())
      {
	fp->write("if(&Us::THE_INSTANCE==d.GetEffectiveSelfPointer()) ");
	fp->write("write(d.read()); ");
	fp->write("else {");
	//write the data members first
	//here.. 'd' UlamRef is initially pointing to them.
	u32 myblen = getBitsizeAsBaseClass();
	if(myblen > 0)
	  {
	    fp->write("/*data members first*/ ");
	    fp->write("BVS::Write(");
	    fp->write("0u,");
	    fp->write_decimal_unsigned(myblen);
	    fp->write("u,");
	    fp->write("UlamRef<EC>(d,0,"); //t3230
	    fp->write_decimal_unsigned(myblen);
	    fp->write("u).Read()); ");
	  }

	//then, write each of its non-zero size (shared) base classes
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(scalaruti, csym);
	assert(isDefined);
	u32 shbasecount = csym->getSharedBaseClassCount();
	if(shbasecount > 0)
	  fp->write("/*nonzero base classes*/ ");
	u32 j = 0;
	while(j < shbasecount)
	  {
	    UTI baseuti = csym->getSharedBaseClass(j);
	    u32 blen = m_state.getBaseClassBitSize(baseuti);
	    if(blen > 0)
	      {
		fp->write("BVS::Write(");
		fp->write_decimal_unsigned(csym->getSharedBaseClassRelativePosition(j));
		fp->write("u,");
		fp->write_decimal_unsigned(blen);
		fp->write("u,");
		fp->write("UlamRef<EC>(d,d.GetEffectiveSelf()->");
		fp->write(m_state.getGetRelPosMangledFunctionName(baseuti));
		fp->write("(");
		fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(baseuti));
		//	    fp->write("u)-d.GetPosToEffectiveSelf(),");
		fp->write("u),");
		fp->write_decimal_unsigned(blen);
		fp->write("u, UlamRef<EC>::CLASSIC, true).Read()); /*"); //(always true!)
		fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str());
		fp->write(" */ ");
	      }
	    j++;
	  } //end while
	fp->write("} }"); GCNL;
      }
    else //array (eg t3143)
      {
	fp->write("write(d.read()); }"); GCNL;
      }

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
    if(totbitsize <= BITSPERATOM) //Big 96bit array is unpacked, but.. (t3776,t3969)
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
    //    if(totbitsize <= BITSPERATOM) //Big 96bit array is unpacked, but.. (t3969)
    if(totbitsize <= MAXBITSPERLONG) //Big 96bit array is unpacked, but.. (t3969)
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
