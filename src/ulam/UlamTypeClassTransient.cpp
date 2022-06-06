#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClassTransient.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClassTransient::UlamTypeClassTransient(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeClass(key, state) { }

  bool UlamTypeClassTransient::isNumericType()
  {
    return false;
  }

  ULAMCLASSTYPE UlamTypeClassTransient::getUlamClassType()
  {
    return UC_TRANSIENT;
  }

  bool UlamTypeClassTransient::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this); //tobe
    UTI valtypidx = val.getUlamValueTypeIdx();
    UlamType * fmut = m_state.getUlamTypeByIndex(valtypidx);
    assert(fmut->isScalar() && isScalar());
    // also allowing subclasses to be cast as their superclass (u1.2.2)
    if(isReference())
      {
	UTI dereftypidx = m_state.getUlamTypeAsDeref(typidx);
	if(UlamType::compare(valtypidx, dereftypidx, m_state) == UTIC_SAME)
	  {
	    val.setUlamValueTypeIdx(typidx);
	  }
	else if(m_state.isClassASubclassOf(valtypidx, dereftypidx))
	  {
	    val.setUlamValueTypeIdx(typidx);
	  }
	else
	  {
	    brtn = false;
	  }
      }
    else if(UlamType::compare(valtypidx, typidx, m_state) == UTIC_SAME)
      {
	//if same type nothing to do;
      }
    else if(m_state.isClassASubclassOf(valtypidx, typidx))
      {
	s32 len = getTotalBitSize();
	assert(len != UNKNOWNSIZE);
	if(len > MAXSTATEBITS)
	  m_state.abortNotSupported(); //for eval transients are limited to statebits
	UlamValue newval = UlamValue::makeAtom(typidx);
	m_state.extractTransientBaseFromSubclassForEval(val, typidx, newval);
	val = newval;
      }
    else if(fmut->getUlamTypeEnum() == Bits)
      {
	if(getTotalBitSize() == fmut->getTotalBitSize())
	  val.setUlamValueTypeIdx(typidx); //t41570
	else
	  brtn = false;
      }
    else
      brtn = false;
    return brtn;
  } //end cast

  FORECAST UlamTypeClassTransient::explicitlyCastable(UTI typidx)
  {
    FORECAST scr = UlamTypeClass::explicitlyCastable(typidx);
    if(scr == CAST_CLEAR)
      {
	//no casting from atom/atomref to transient, other classes may be fine
	if(m_state.isAtom(typidx))
	  scr = CAST_BAD;
      }
    return scr;
  } //explicitlyCastable

  const char * UlamTypeClassTransient::getUlamTypeAsSingleLowercaseLetter()
  {
    return "n";
  }

  const std::string UlamTypeClassTransient::getUlamTypeUPrefix()
  {
    //scalar or array Sat Sep 30 16:05:48 2017
    return "Un_";
  }

  const std::string UlamTypeClassTransient::readMethodForCodeGen()
  {
    if(getTotalBitSize() <= MAXBITSPERLONG)
      return UlamType::readMethodForCodeGen();
    return "ReadBV"; //UlamType::readMethodForCodeGen();
  }

  const std::string UlamTypeClassTransient::writeMethodForCodeGen()
  {
    if(getTotalBitSize() <= MAXBITSPERLONG)
      return UlamType::writeMethodForCodeGen();
    return "WriteBV"; //return UlamType::writeMethodForCodeGen();
  }

  const std::string UlamTypeClassTransient::readArrayItemMethodForCodeGen()
  {
    if(isCustomArray())
      return m_state.getCustomArrayGetMangledFunctionName();

    if(getBitSize() <= MAXBITSPERLONG)
      return UlamType::readArrayItemMethodForCodeGen();

    return "ReadBV";
  }

  const std::string UlamTypeClassTransient::writeArrayItemMethodForCodeGen()
  {
    if(isCustomArray())
      return m_state.getCustomArrayGetMangledFunctionName(); //returns a ref

    if(getBitSize() <= MAXBITSPERLONG)
      return UlamType::writeArrayItemMethodForCodeGen();

    return "WriteBV";
  }

  bool UlamTypeClassTransient::needsImmediateType()
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

  const std::string UlamTypeClassTransient::getTmpStorageTypeAsString()
  {
    if(getTotalWordSize() <= MAXBITSPERLONG)
      return UlamType::getTmpStorageTypeAsString();

    std::ostringstream stype;
    stype << "BitVector<" << getTotalBitSize() << ">";

    return stype.str();
  } //getTmpStorageTypeAsString

  const std::string UlamTypeClassTransient::getArrayItemTmpStorageTypeAsString()
  {
    if(!isScalar())
      {
	if(getItemWordSize() <= MAXBITSPERLONG)
	  return UlamType::getArrayItemTmpStorageTypeAsString();

	std::ostringstream stype;
	stype << "BitVector<" << getBitSize() << ">";
	return stype.str();
      }

    assert(isCustomArray());
    return m_state.getUlamTypeByIndex(getCustomArrayType())->getTmpStorageTypeAsString();
  } //getArrayItemTmpStorageTypeAsString

  const std::string UlamTypeClassTransient::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName();
    ctype << "<EC>";
    return ctype.str();
  } //getLocalStorageTypeAsString

  TMPSTORAGE UlamTypeClassTransient::getTmpStorageTypeForTmpVar()
  {
    return UlamType::getTmpStorageTypeForTmpVar();
    //return TMPTBV; //t41416
  } //getTmpStorageTypeForTmpVar

  const std::string UlamTypeClassTransient::castMethodForCodeGen(UTI nodetype)
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
	msg << ", Value Type and size was: ";
	msg << nut->getUlamTypeName().c_str() << ", to be: ";
	msg << sizeByIntBitsToBe << " for type: ";
	msg << getUlamTypeName().c_str();
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(),msg.str().c_str(), ERR);
      }

    return "_NoCast";
  } //castMethodForCodeGen

  void UlamTypeClassTransient::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    assert(getUlamClassType() == UC_TRANSIENT);
    s32 len = getTotalBitSize(); //could be 0, includes arrays
    s32 baselen = isScalar() ? getBitsizeAsBaseClass() : len; //could be 0, default when effself not self (ulam-5)
    assert(baselen >= 0);

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

    //forward declaration of class and its immediate (before struct!)
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
    fp->write(" : public UlamRef<EC>\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    UlamType::genStandardConfigTypedefTypenames(fp, m_state);

    //class typedef
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

    m_state.indent(fp);
    fp->write("typedef UlamRef"); //was atomicparametertype
    fp->write("<EC> Up_Us;"); GCNL;

    //read 'entire' method
    genUlamTypeAutoReadDefinitionForC(fp);

    //write 'entire' method
    genUlamTypeAutoWriteDefinitionForC(fp);

    //keep this one too?
    // constructor given storage
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const UlamContext<EC>& uc) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    if(!isScalar())
      fp->write_decimal_unsigned(len); //includes arraysize
    else if(len != baselen)
      {
	fp->write("&Us::THE_INSTANCE==effself ? ");
	fp->write_decimal_unsigned(len); //includes arraysize
	fp->write("u : ");
	fp->write_decimal_unsigned(baselen); //base class
      }
    else
	fp->write_decimal_unsigned(len);

    fp->write("u, targ, effself, ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::CLASSIC");
    fp->write(", uc) { }"); GCNL;

    //constructor given storage
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, u32 postoeff, const UlamClass<EC>* effself, const UlamContext<EC>& uc) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    if(!isScalar())
      fp->write_decimal_unsigned(len); //includes arraysize
    else if(len != baselen)
      {
	fp->write("&Us::THE_INSTANCE==effself ? ");
	fp->write_decimal_unsigned(len); //includes arraysize
	fp->write("u : ");
	fp->write_decimal_unsigned(baselen); //base class
      }
    else
	fp->write_decimal_unsigned(len);

    fp->write("u, postoeff, targ, effself, ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::CLASSIC");
    fp->write(", uc) { }"); GCNL;

    //short-hand from immediate to ref of same type
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write(automangledName.c_str());
	fp->write("(");
	fp->write(mangledName.c_str());
	fp->write("<EC>& targ, const UlamContext<EC> & uc) : UlamRef<EC>");
	fp->write("(0, "); //pos
	fp->write_decimal_unsigned(len);
	fp->write("u, targ, & Us::THE_INSTANCE, UlamRef<EC>::CLASSIC, uc)");
	fp->write(" { }"); GCNL;
      }

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 incr, const UlamClass<EC>* effself) : UlamRef<EC>(arg, incr, ");
    if(len != baselen)
      {
	fp->write("&Us::THE_INSTANCE==effself ? ");
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

    //(general) copy constructor here; pos relative to existing (i.e. same).
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef");
    fp->write("<EC>& r) : UlamRef<EC>(r,0,r.GetLen()) {}"); GCNL;

    //(general) copy constructor here; base pos relative to existing (t41355)
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


    //(exact) copy constructor (for compiler)
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

    //default destructor (intentionally left out)

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
  } //genUlamTypeMangledAutoDefinitionForC

  void UlamTypeClassTransient::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    u32 totbitsize = getSizeofUlamType();
    if(totbitsize <= MAXBITSPERLONG)
      {
	m_state.indent(fp);
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64
	fp->write(" read() const { ");
	fp->write("if(&Us::THE_INSTANCE==this->GetEffectiveSelfPointer()){ ");
	fp->write("return this->GetStorage().");
	fp->write(readMethodForCodeGen().c_str()); //just the guts
	fp->write("(this->GetPos(), this->GetLen()); } ");
	fp->write("else { return Usi(*this).read(); } }"); GCNL;
      }
    else
      {
	m_state.indent(fp);
	fp->write("void ");
	fp->write(" read(");
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write("& rtnbv) const { ");
	fp->write("if(&Us::THE_INSTANCE==this->GetEffectiveSelfPointer()){ ");
	fp->write("this->GetStorage().");
	fp->write(readMethodForCodeGen().c_str()); //just the guts
	fp->write("(this->GetPos(),rtnbv); /*entire transient*/}");
	fp->write("else { Usi(*this).read(rtnbv); } }"); GCNL;
      }

    //scalar and entire PACKEDLOADABLE array handled by read method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();

	//reads an item of array
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //u32,u64,BV
	fp->write(" readArrayItem(");
	fp->write("const u32 index,const u32 itemlen) const { return ");
	fp->write("UlamRef<EC>(");
	fp->write("*this,index * itemlen,"); //const ref, rel offset
	fp->write("itemlen,&");  //itemlen,
	fp->write(m_state.getTheInstanceMangledNameByIndex(scalaruti).c_str());
	fp->write(",UlamRef<EC>::CLASSIC).");
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("();}"); GCNL;
      }
  } //genUlamTypeAutoReadDefinitionForC

  void UlamTypeClassTransient::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    // write must be scalar; ref param to avoid excessive copying
    //not an array
    if(isScalar())
      {
	s32 len = getBitSize();
	//ref param to avoid excessive copying; not an array
	m_state.indent(fp);
	fp->write("void write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV
	fp->write("& targ){ ");
	if(len > 0)
	  {
	    fp->write("if(&Us::THE_INSTANCE==this->GetEffectiveSelfPointer()) ");
	    fp->write("UlamRef<EC>::");
	    fp->write(writeMethodForCodeGen().c_str()); //t41358
	    fp->write("(");
	    if(len > MAXBITSPERLONG)
	      fp->write("0,"); //t41512
	    fp->write("targ); /* entire transient */ ");
	    fp->write("else{ ");

	    if(len <= MAXBITSPERINT)
	      {
		fp->write("BitVector<");
		fp->write_decimal_unsigned(len);
		fp->write("u> tmpbv");
		fp->write("(targ);"); //copy cstr u32->bv
	      }
	    else if(len <= MAXBITSPERLONG)
	      {
		/* union trick to convert u64 to array of u32s */
		fp->write("union{u64 me64;u32 me32[2];} tmpU;");
		fp->write("tmpU.me64=targ;");

		fp->write("BitVector<");
		fp->write_decimal_unsigned(len);
		fp->write("u> tmpbv");
		fp->write("(tmpU.me32);"); //copy cstr u32[]->bv
	      }
	    //else //already a bitvector

	    //write the data members first
	    //here.. 'targ' BV is the complete, 'this' UlamRef
	    // may be different effSelf within larger context
	    s32 myblen = getBitsizeAsBaseClass();
	    assert(myblen >= 0);
	    if(myblen > 0)
	      {
		fp->write("/*data members first*/ ");
		fp->write("BitVector<");
		fp->write_decimal_unsigned(myblen);
		fp->write("u> tmpDM;");
		fp->write(len <= MAXBITSPERLONG ? "tmpbv" : "targ");
		fp->write(".ReadBV(0u,tmpDM);");
		fp->write("UlamRef<EC>(*this,0,");
		fp->write_decimal_unsigned(myblen);
		fp->write("u).WriteBV(0u,tmpDM);");
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
		    fp->write("BitVector<");
		    fp->write_decimal_unsigned(blen);
		    fp->write("u> tmpbv");
		    fp->write_decimal(j);
		    fp->write(len <= MAXBITSPERLONG ? "; tmpbv" : "; targ");
		    fp->write(".ReadBV(");
		    fp->write_decimal_unsigned(csym->getSharedBaseClassRelativePosition(j));
		    fp->write("u,tmpbv");
		    fp->write_decimal(j);
		    fp->write(");UlamRef<EC>(*this,this->GetEffectiveSelf()->");
		    fp->write(m_state.getGetRelPosMangledFunctionName(baseuti));
		    fp->write("(");
		    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(baseuti));
		    fp->write("u)- this->GetPosToEffectiveSelf(),");
		    fp->write_decimal_unsigned(blen);
		    fp->write("u).WriteBV(0u,tmpbv");
		    fp->write_decimal(j);
		    fp->write(");");

		    fp->write("/*");
		    fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str());
		    fp->write("*/ ");
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
    else
      {
	//array
	m_state.indent(fp);
	fp->write("void");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write("& targ) { this->GetStorage().");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(this->GetPos(), targ); /* entire transient */ }"); GCNL;

	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	// writes an item of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //BVi
	fp->write(" v, const u32 index, const u32 itemlen) { ");
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //rel offset
	fp->write("itemlen, &");  //itemlen,
	fp->write(m_state.getTheInstanceMangledNameByIndex(scalaruti).c_str());
	fp->write(", UlamRef<EC>::CLASSIC).");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(v); }"); GCNL;
      }
  } //genUlamTypeAutoWriteDefinitionForC

  void UlamTypeClassTransient::genUlamTypeMangledDefinitionForC(File * fp)
  {
    s32 len = getTotalBitSize(); //could be 0, includes arrays
    s32 bitsize = getBitSize();
    s32 arraysize = getArraySize();

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

    //forward declaration for immediate ref (before struct)
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
	//fp->write(getArrayItemTmpStorageTypeAsString().c_str());
	fp->write("BitVector<");
	fp->write_decimal_unsigned(getBitSize());
	fp->write("> BVI;"); GCNL;

	m_state.indent(fp);
	fp->write("typedef BitVectorBitStorage<EC, BVI > BVIS;"); GCNL;
	fp->write("\n");
      }

    //class typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;"); GCNL;
    fp->write("\n");

    //read/write methods before constructors in case used.
    genUlamTypeReadDefinitionForC(fp);

    genUlamTypeWriteDefinitionForC(fp);

    //default constructor uses the default value
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { ");
    if(isScalar())
      {
	fp->write("Us::THE_INSTANCE.getDefaultTransient(0u,*this); }"); GCNL;
      }
    else
      {
	fp->write("BVIS tmpbs; Us::THE_INSTANCE.getDefaultTransient(0u, tmpbs); ");
	if(bitsize <= MAXBITSPERLONG)
	  {
	    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //u32,u64
	    fp->write(" tmpbvi = tmpbs.");
	    fp->write(readMethodForCodeGen().c_str());
	    fp->write("(0u,");
	    fp->write_decimal_unsigned(bitsize);
	    fp->write("u);"); //t3810
	  }
	else
	  {
	    fp->write(" BVI tmpbvi; tmpbs.ReadBV(0u,tmpbvi); ");
	  }
	fp->write("for(u32 j = 0; j < ");
	fp->write_decimal_unsigned(arraysize);
	fp->write("u; j++) ");
	fp->write("writeArrayItem(tmpbvi,j,");
	fp->write_decimal_unsigned(bitsize);
	fp->write("); }"); GCNL;
      }

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, BVn
    fp->write("& d) { ");
    fp->write("write(d); }"); GCNL;

    // assignment copy constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str());
    fp->write("<EC> & arg) : BVS() { ");
    if(len <= MAXBITSPERLONG)
      {
	fp->write("write(arg.read()); }"); GCNL;
      }
    else
      {
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write(" tmpbv; arg.read(tmpbv); write(tmpbv); } "); GCNL;
      }

    //constructor from ref of same type
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& d) { ");
    if(isScalar())
      {
	fp->write("if(&Us::THE_INSTANCE==d.GetEffectiveSelfPointer()) ");
	if(len <= MAXBITSPERLONG)
	  {
	    fp->write("write(d.read()); ");
	  }
	else
	  {
	    fp->write("{ ");
	    fp->write(getTmpStorageTypeAsString().c_str()); //BV
	    fp->write(" tmpbv; d.read(tmpbv); write(tmpbv); } ");
	  }
	fp->write("else {");

	//write the data members first
	//here.. 'd' UlamRef is initially pointing to them.
	s32 myblen = getBitsizeAsBaseClass();
	assert(myblen >= 0);
	if(myblen > 0)
	  {
	    fp->write("/*data members first*/ ");
	    if(myblen <= MAXBITSPERINT)
	      {
		fp->write("BVS::Write(");
		fp->write("0u,");
		fp->write_decimal(myblen);
		fp->write("u,");
		fp->write("UlamRef<EC>(d,0,");
		fp->write_decimal(myblen);
		fp->write("u).Read()); ");
	      }
	    else
	      {
		fp->write("BitVector<");
		fp->write_decimal_unsigned(myblen);
		fp->write("u> tmpDM;");
		fp->write("UlamRef<EC>(d,0,");
		fp->write_decimal_unsigned(myblen);
		fp->write("u).ReadBV(0u,tmpDM);");
		fp->write("this->WriteBV(0u,tmpDM);");
	      }
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
		if(blen <= MAXBITSPERINT)
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
		    fp->write("u)-d.GetPosToEffectiveSelf(),");
		    fp->write_decimal_unsigned(blen);
		    fp->write("u).Read());");
		  }
		else
		  {
		    fp->write("BitVector<");
		    fp->write_decimal_unsigned(blen);
		    fp->write("u> tmpbv");
		    fp->write_decimal(j);
		    fp->write("; UlamRef<EC>(d, d.GetEffectiveSelf()->");
		    fp->write(m_state.getGetRelPosMangledFunctionName(baseuti));
		    fp->write("(");
		    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(baseuti));
		    fp->write("u)- d.GetPosToEffectiveSelf(),");
		    fp->write_decimal_unsigned(blen);
		    fp->write("u).ReadBV(0u,tmpbv");
		    fp->write_decimal(j);
		    fp->write("); this->WriteBV(");
		    fp->write_decimal(csym->getSharedBaseClassRelativePosition(j));
		    fp->write(", tmpbv");
		    fp->write_decimal(j);
		    fp->write(");");
		  }
		fp->write("/*");
		fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str());
		fp->write("*/ ");
	      } //next base
	    j++;
	  } //end while
	fp->write("} }"); GCNL;
      }
    else //array
      {
	if(len <= MAXBITSPERLONG)
	  {
	    fp->write("write(d.read()); }"); GCNL;
	  }
	else
	  {
	    fp->write(getTmpStorageTypeAsString().c_str()); //BV
	    fp->write(" tmpbv; d.read(tmpbv); write(tmpbv); }"); GCNL;
	  }
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

  void UlamTypeClassTransient::genUlamTypeReadDefinitionForC(File * fp)
  {
    u32 len = getTotalBitSize();
    if(len <= MAXBITSPERLONG)
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, BV
	fp->write(" read() const { ");
	fp->write("return this->BVS::");
	fp->write(UlamType::readMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal(len);
	fp->write("u); } //reads entire BV"); GCNL; //t41416
      }
    else
      {
	//UNPACKED
	m_state.indent(fp);
	fp->write("void ");
	fp->write(" read(");
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write("& rtnbv) const { ");
	fp->write("this->BVS::ReadBV(0u, rtnbv);");
	fp->write("} //reads entire BV"); GCNL;
      }

    //scalar and entire PACKEDLOADABLE array handled by read method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
	const std::string scalarmangledName = scalarut->getUlamTypeMangledName();
	u32 itemlen = getBitSize();
	//reads an item of array
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32, bv
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { "); //return BVS::"); //was const after )
	if(itemlen > MAXBITSPERLONG)
	  {
	    fp->write(scalarut->getTmpStorageTypeAsString().c_str()); //BV
	    fp->write("rtnunpbv; this->BVS::");
	    fp->write(scalarut->readMethodForCodeGen().c_str());
	    fp->write("(index * itemlen, rtnunpbv); return rtnunpbv; ");
	    fp->write("} //reads item of BV"); GCNL;
	  }
	else
	  {
	    fp->write("return this->BVS::");
	    fp->write(scalarut->readMethodForCodeGen().c_str());
	    fp->write("(index * itemlen,");
	    fp->write_decimal_unsigned(itemlen);
	    fp->write("u);");
	    fp->write("} //reads item of BV"); GCNL; //t3810
	  }
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypeClassTransient::genUlamTypeWriteDefinitionForC(File * fp)
  {
    s32 len = getTotalBitSize();
    if(len <= MAXBITSPERLONG)
      {
	m_state.indent(fp);
	fp->write("void ");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64
	fp->write("& d) { BVS::");
	fp->write(UlamType::writeMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal(len);
	fp->write("u, d); } //writes entire transient"); GCNL;
      }
    else
      {
	m_state.indent(fp);
	fp->write("void ");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //BV
	fp->write("& bv) { BVS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(0u, bv); ");
	fp->write("} //writes entire BV"); GCNL;
      }

    //scalar and entire PACKEDLOADABLE array handled by write method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	u32 itemlen = getBitSize();
	// writes an item of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write("& v, const u32 index, const u32 itemlen) { BVS::");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen,");
	if(itemlen <= MAXBITSPERLONG)
	  {
	    fp->write_decimal_unsigned(itemlen);
	    fp->write("u,");
	    fp->write("v); }"); GCNL;  //rel offset, len, val
	  }
	else
	  {
	    fp->write("v); }"); GCNL;  //rel offset, val
	  }
      }
  } //genUlamTypeWriteDefinitionForC

} //end MFM
