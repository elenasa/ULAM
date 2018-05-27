#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClassElement.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClassElement::UlamTypeClassElement(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeClass(key, state) { }

  ULAMCLASSTYPE UlamTypeClassElement::getUlamClassType()
  {
    return UC_ELEMENT;
  }

  bool UlamTypeClassElement::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this); //tobe
    UTI valtypidx = val.getUlamValueTypeIdx();
    UlamType * fmut = m_state.getUlamTypeByIndex(valtypidx);
    assert(fmut->isScalar() && isScalar());
    ULAMTYPE vetype = fmut->getUlamTypeEnum();
    //now allowing atoms to be cast as quarks, as well as elements;
    // also allowing subclasses to be cast as their superclass (u1.2.2)
    if(!(vetype == UAtom || UlamType::compare(valtypidx, typidx, m_state) == UTIC_SAME))
      brtn = false; //elements inherit from only quarks.
    else if(vetype == UAtom)
      {
	UTI dereftypidx = m_state.getUlamTypeAsDeref(typidx);
	val.setAtomElementTypeIdx(dereftypidx); //for testing purposes, assume ok (e.g. t3754)
      }
    //else true
    return brtn;
  } //end cast

  FORECAST UlamTypeClassElement::explicitlyCastable(UTI typidx)
  {
    FORECAST scr = UlamTypeClass::explicitlyCastable(typidx); //default, arrays checked
    if(scr == CAST_BAD)
      {
	//check from atom or atomref, possible ok for (non-packed) elements (runtime)
	if(m_state.isAtom(typidx))
	  scr = CAST_CLEAR;
      }
    return scr;
  } //explicitlyCastable

  const char * UlamTypeClassElement::getUlamTypeAsSingleLowercaseLetter()
  {
    return "e";
  }

  const std::string UlamTypeClassElement::getUlamTypeUPrefix()
  {
    //scalar or array Sat Sep 30 16:05:48 2017
    return "Ue_";
  } //getUlamTypeUPrefix

  PACKFIT UlamTypeClassElement::getPackable()
  {
    //atom-based, no longer PACKED
    return UNPACKED; //UlamType::getPackable();
  }

  u32 UlamTypeClassElement::getSizeofUlamType()
  {
    //atom-based sizeof for both scalar and arrays of elements
    s32 arraysize = getArraySize();
    arraysize = (arraysize == NONARRAYSIZE) ? 1 : arraysize;
    s32 len = BITSPERATOM * arraysize; //atom-based; could be 0 bits.
    //BitVector<0> handled by BitVector.h
    return len; //t3841, t3400
  } //getSizeofUlamType

  const std::string UlamTypeClassElement::readMethodForCodeGen()
  {
    if(!isScalar())
      return "ReadBV";
    return "ReadAtom";
  }

  const std::string UlamTypeClassElement::writeMethodForCodeGen()
  {
    if(!isScalar())
      return "WriteBV";
    return "WriteAtom";
  }

  bool UlamTypeClassElement::needsImmediateType()
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

  const std::string UlamTypeClassElement::getArrayItemTmpStorageTypeAsString()
  {
    return "T";
  }

  const std::string UlamTypeClassElement::getTmpStorageTypeAsString()
  {
    if(!isScalar())
      {
	std::ostringstream cstr;
	cstr << "BitVector<" << getSizeofUlamType() << ">";
	return cstr.str();
      }
    return "T";
  } //getTmpStorageTypeAsString

  const std::string UlamTypeClassElement::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName();
    ctype << "<EC>";
    return ctype.str();
  } //getLocalStorageTypeAsString

  TMPSTORAGE UlamTypeClassElement::getTmpStorageTypeForTmpVar()
  {
    return TMPTATOM; //per array item/per atom-based element
  }

  const std::string UlamTypeClassElement::castMethodForCodeGen(UTI nodetype)
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

    //e.g. casting element-to-element, redundant and not supported: Element96ToElement96?
    if(!m_state.isAtom(nodetype))
      {
	std::ostringstream msg;
	msg << "Attempting to illegally cast a non-atom type to an element: ";
	msg << "value type and size was: ";
	msg << nut->getUlamTypeName().c_str() << ", to be: " << getUlamTypeName().c_str();
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(),msg.str().c_str(), ERR);
      }
    rtnMethod << "_" << nut->getUlamTypeNameOnly().c_str() << sizeByIntBits;
    rtnMethod << "ToElement" << sizeByIntBitsToBe;
    return rtnMethod.str();
  } //castMethodForCodeGen

  void UlamTypeClassElement::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    // handles both scalar and arrays (unpacked or packed)
    // scalar, NOT 96: (e.g. 3249, t3255, t3259, t3407,8, t3615, t3636,7,8,9,t3655,6, t3670,75, t3706,9,10
    // t3751,2,3,4,5,6,7,9, t3788, t3795, t3817,18,20, t3831,32, t3841)
    s32 len = getTotalBitSize(); //NOT (BITSPERATOM - ATOMFIRSTSTATEBITPOS), NOT 96.
    if(!isScalar())
      {
    	assert(getArraySize() >= 0); //zero-length array is legal to declare, but not access
    	len = getSizeofUlamType(); //could be 0, includes arrays
      }

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

    //forward declaration of element (before struct!)
    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(scalarmangledName.c_str());
    fp->write(";  //forward\n"); GCNL;

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

    //element typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;"); GCNL;
    fp->write("\n");

    //read 'entire' method
    genUlamTypeAutoReadDefinitionForC(fp);

    //write 'entire' method
    genUlamTypeAutoWriteDefinitionForC(fp);

    //constructor for conditional-as (auto)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const UlamContext<EC> & uc) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    fp->write_decimal_unsigned(len); //atom-based size, includes: arraysize, and Type
    fp->write("u, targ, effself, ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::ELEMENTAL");
    fp->write(", uc) { }"); GCNL;

    //copy constructor here
    // t3670,
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r) : UlamRef<EC>(r, 0, r.GetLen(), r.GetEffectiveSelf(), ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::ELEMENTAL");
    fp->write(") { }"); GCNL;

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize (t3670)
    fp->write("u, effself, ");
    if(!isScalar())
      fp->write("UlamRef<EC>::ARRAY");
    else
      fp->write("UlamRef<EC>::ELEMENTAL");
    fp->write(") { }"); GCNL; //e.g. 3655, t3656, t3663, t3675, t3689, t3655

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
  } //genUlamTypeElementMangledAutoDefinitionForC

  void UlamTypeClassElement::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    //ref param to avoid excessive copying;
    m_state.indent(fp);
    fp->write(getTmpStorageTypeAsString().c_str()); //T or BV
    fp->write(" read() const { ");
    fp->write("return ");
    fp->write("UlamRef<EC>::");
    fp->write(readMethodForCodeGen().c_str());
    if(isScalar())
      {
	fp->write("(); /* read entire element ref */ }"); GCNL;
      }
    else
      {
	fp->write("(); /* read entire element array ref */ }"); GCNL;
      }

    //scalar and entire UNPACKED array handled by read method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	//reads an item of array
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32 or BV96
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return ");
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //const ref, rel offset
	fp->write("itemlen, &");  //itemlen,
	fp->write("Us::THE_INSTANCE, UlamRef<EC>::ELEMENTAL)."); //effself
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(); }"); GCNL;
      }
  } //genUlamTypeAutoReadDefinitionForC

  void UlamTypeClassElement::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    // write must be scalar; ref param to avoid excessive copying
    //not an array
    m_state.indent(fp);
    fp->write("void");
    fp->write(" write(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T or BV
    fp->write("& targ) { ");
    if(isScalar())
      fp->write("if(targ.GetType() != Us::THE_INSTANCE.GetType()) FAIL(ILLEGAL_ARGUMENT); ");
    fp->write("UlamRef<EC>::");
    fp->write(writeMethodForCodeGen().c_str());
    if(isScalar())
      {
	fp->write("(targ); /* write entire element */ }"); GCNL;
      }
    else
      {
	fp->write("(targ); /* write entire element array */ }"); GCNL;
      }

    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("void write(const ");
	fp->write("AtomRefBitStorage<EC>");
	fp->write("& v) { ");
	fp->write("if(v.GetType() != Us::THE_INSTANCE.GetType()) FAIL(ILLEGAL_ARGUMENT); ");
	fp->write("UlamRef<EC>::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(v.ReadAtom()); /* write entire atom */ }"); GCNL; //done
      }

    //scalar and entire PACKEDLOADABLE array handled by write method
    if(!isScalar())
      {
	// writes an item of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
	fp->write("& v, const u32 index, const u32 itemlen) { ");
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //rel offset
	fp->write("itemlen, &");  //itemlen,
	fp->write("Us::THE_INSTANCE, UlamRef<EC>::ELEMENTAL)."); //effself
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(v); }"); GCNL;
      }
  } //genUlamTypeAutoWriteDefinitionForC

  //builds the immediate definition
  void UlamTypeClassElement::genUlamTypeMangledDefinitionForC(File * fp)
  {
    if(!isScalar())
      return genUlamTypeMangledUnpackedArrayDefinitionForC(fp);

    //class instance idx is always the scalar uti
    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
    const std::string scalarmangledName = scalarut->getUlamTypeMangledName();
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

    //forward declaration of element (before struct!)
    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(scalarmangledName.c_str());
    fp->write(";  //forward"); GCNL;
    fp->write("\n");

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

    //element typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;"); GCNL;
    fp->write("\n");

    //read/write methods before constructors that may use them
    //read 'entire element' method
    genUlamTypeReadDefinitionForC(fp);

    //write 'entire element' method
    genUlamTypeWriteDefinitionForC(fp);

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : AtomBitStorage<EC>");
    fp->write("(Us::THE_INSTANCE");
    fp->write(".GetDefaultAtom()) { }"); GCNL; //returns object of type T

    //constructor here (used by const tmpVars) (e.g. t3706)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write("& d) : ");
    fp->write("AtomBitStorage<EC>");
    fp->write("(d) { ");
    fp->write("if(d.GetType() != Us::THE_INSTANCE.GetType()) FAIL(ILLEGAL_ARGUMENT);");
    fp->write(" }"); GCNL;

    // assignment copy constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str());
    fp->write("<EC> & arg) { ");
    fp->write("this->m_stg = arg.m_stg; }"); GCNL;

    //constructor for constants (t41230); MFM Element Type not in place yet
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const u32 * const ");
    fp->write("arg) : AtomBitStorage<EC>() { if(arg==NULL) FAIL(NULL_POINTER); ");
    fp->write("this->m_stg.GetBits().FromArray(arg); }"); GCNL;

    //assignment constructor, atom arg, for convenience
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const AtomRefBitStorage<EC> & arg) : ");
    fp->write("AtomBitStorage<EC>");
    fp->write("(arg.ReadAtom()) { ");
    fp->write("if(arg.GetType() != Us::THE_INSTANCE.GetType()) FAIL(ILLEGAL_ARGUMENT);");
    fp->write(" }"); GCNL;

    //constructor from ref of same type
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& d) : ");
    fp->write("AtomBitStorage<EC>");
    fp->write("(d.read()) { }"); GCNL;

    //default destructor (intentionally left out)

    //for var args native funcs, non-refs, required of a BitStorage
    UlamType::genGetUlamTypeMangledNameDefinitionForC(fp);

    //note: no operator= since base class has a T&

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

  void UlamTypeClassElement::genUlamTypeReadDefinitionForC(File * fp)
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
	//array (packed or UNPACKED)
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

    if(!isScalar())
      {
	//array item reads; entire PACKEDLOADABLE array read handled by Read()
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T, u64, or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return BVS::");
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen); }"); GCNL; //rel offset
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypeClassElement::genUlamTypeWriteDefinitionForC(File * fp)
  {
    //ref param to avoid excessive copying
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("void write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //T
	fp->write("& v) { ABS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(v); }"); GCNL; //done
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

    if(!isScalar())
      {
	// writes an item of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T, u64, u32
	fp->write("& v, const u32 index, const u32 itemlen) { BVS::");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, v); }"); GCNL;  //reloffset, val
      }
  } //genUlamTypeWriteDefinitionForC

  const std::string UlamTypeClassElement::readArrayItemMethodForCodeGen()
  {
    return "ReadAtom";
  }

  const std::string UlamTypeClassElement::writeArrayItemMethodForCodeGen()
  {
    return "WriteAtom";
  }

  void UlamTypeClassElement::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    s32 arraysize = getArraySize(); //needed for loops
    assert(arraysize >= 0); //zero-length array is legal to declare, but not access

    s32 len = getSizeofUlamType(); //could be 0, includes arrays

    m_state.m_currentIndentLevel = 0;

    //class instance idx is always the scalar uti of element
    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
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

    //element typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;"); GCNL;
    fp->write("\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
    fp->write(" tmpt = Us::THE_INSTANCE.GetDefaultAtom(); ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("writeArrayItem(tmpt, j, BPA); }"); GCNL;

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write("T& targ) { ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("writeArrayItem(targ, j, BPA); }"); GCNL;

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
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
    fp->write(" tmpt = arg.ReadAtom(); ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("writeArrayItem(tmpt, j, BPA); }"); GCNL;

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
