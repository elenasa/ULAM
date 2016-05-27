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
    UlamType * vut = m_state.getUlamTypeByIndex(valtypidx);
    assert(vut->isScalar() && isScalar());
    ULAMTYPE vetype = vut->getUlamTypeEnum();
    //now allowing atoms to be cast as quarks, as well as elements;
    // also allowing subclasses to be cast as their superclass (u1.2.2)
    if(!(vetype == UAtom || UlamType::compare(valtypidx, typidx, m_state) == UTIC_SAME))
      {
	//elements inherit from only quarks.
	brtn = false;
      }
    else if(vetype == UAtom)
      val.setAtomElementTypeIdx(typidx); //for testing purposes, assume ok
    //else true
    return brtn;
  } //end cast

  FORECAST UlamTypeClassElement::safeCast(UTI typidx)
  {
    FORECAST scr = UlamTypeClass::safeCast(typidx);
    if(scr == CAST_BAD)
      {
	//check from atom or atomref, possible ok for (non-packed) elements (runtime)
	if(m_state.isAtom(typidx))
	  scr = CAST_CLEAR;
      }
    return scr;
  } //safeCast

  const char * UlamTypeClassElement::getUlamTypeAsSingleLowercaseLetter()
  {
    return "e";
  }

  const std::string UlamTypeClassElement::getUlamTypeUPrefix()
  {
    if(getArraySize() > 0)
      return "Ut_";

    return "Ue_";
  } //getUlamTypeUPrefix

  PACKFIT UlamTypeClassElement::getPackable()
  {
    return UNPACKED; //UlamType::getPackable(); //depends on PACKED size
  }

  const std::string UlamTypeClassElement::readMethodForCodeGen()
  {
    if(!isScalar())
      return "ReadBV";
    return "ReadAtom";
  } //readMethodForCodeGen

  const std::string UlamTypeClassElement::writeMethodForCodeGen()
  {
    if(!isScalar())
      return "WriteBV";
    return "WriteAtom";
  } //writeMethodForCodeGen

  bool UlamTypeClassElement::needsImmediateType()
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

  const std::string UlamTypeClassElement::getArrayItemTmpStorageTypeAsString()
  {
    return "T";
  }

  const std::string UlamTypeClassElement::getTmpStorageTypeAsString()
  {
    if(!isScalar())
      {
	s32 arraysize = getArraySize();
	arraysize = (arraysize <= 0 ? 1 : arraysize);
	s32 len = BITSPERATOM * arraysize;//getTotalBitSize(); //could be 0, includes arrays

	std::ostringstream cstr;
	cstr << "BitVector<" << len << ">";
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
  } //getTmpStorageTypeForTmpVar

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

    //e.g. casting an element to an element, redundant and not supported: Element96ToElement96?
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
    //if(!isScalar())
    //  return genUlamTypeMangledUnpackedArrayAutoDefinitionForC(fp);

    s32 len = getTotalBitSize(); //BITSPERATOM - ATOMFIRSTSTATEBITPOS;
    if(!isScalar())
      {
	s32 arraysize = getArraySize();
	arraysize = (arraysize <= 0 ? 1 : arraysize);
	len = BITSPERATOM * arraysize;//getTotalBitSize(); //could be 0, includes arrays
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
    fp->write(";  //forward\n\n");

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
    m_state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");
    fp->write("\n");

    //element typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;\n");
    fp->write("\n");

    //constructor for conditional-as (auto)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    fp->write_decimal_unsigned(len); //atom-based size, includes: arraysize, and Type
    fp->write("u, targ, effself) { }\n");

    //copy constructor here
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r) : UlamRef<EC>(r, 0, r.GetLen(), r.GetEffectiveSelf()) { }\n");

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize (t3670)
    fp->write("u, effself) {}\n"); //e.g. 3655, t3656, t3663, t3675, t3689, t3655

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(automangledName.c_str());
    fp->write("() {}\n");

    //read 'entire' method
    genUlamTypeAutoReadDefinitionForC(fp);

    //write 'entire' method
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
  } //genUlamTypeElementMangledAutoDefinitionForC

  void UlamTypeClassElement::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    //ref param to avoid excessive copying
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T or BV
    fp->write(" read() { ");
    fp->write("return ");
    fp->write("UlamRef<EC>::");
    fp->write(readMethodForCodeGen().c_str());
    if(isScalar())
      fp->write("(); /* read entire element ref */ }\n");
    else
      fp->write("(); /* read entire element array ref */ }\n");

    //scalar and entire UNPACKED array handled by read method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	//reads an item of array
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32 or BV96
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return "); //was const after )
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //const ref, rel offset
	fp->write("itemlen, &");  //itemlen,
	fp->write("Us::THE_INSTANCE)."); //effself
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(); }\n");
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
      fp->write("(targ); /* write entire element */ }\n");
    else
      fp->write("(targ); /* write entire element array */ }\n");

    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("void write(const ");
	fp->write("AtomRefBitStorage<EC>");
	fp->write("& v) { ");
	fp->write("if(v.GetType() != Us::THE_INSTANCE.GetType()) FAIL(ILLEGAL_ARGUMENT); ");
	fp->write("UlamRef<EC>::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(v.ReadAtom()); /* write entire atom */ }\n"); //done
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
	fp->write("Us::THE_INSTANCE)."); //effself
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(v); }\n");
      }
  } //genUlamTypeAutoWriteDefinitionForC

  //builds the immediate definition, inheriting from auto
  void UlamTypeClassElement::genUlamTypeMangledDefinitionForC(File * fp)
  {
    if(!isScalar())
      return genUlamTypeMangledUnpackedArrayDefinitionForC(fp);

    //class instance idx is always the scalar uti
    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
    const std::string scalarmangledName = scalarut->getUlamTypeMangledName();
    const std::string mangledName = getUlamTypeImmediateMangledName();

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
    fp->write(";  //forward\n\n");

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
    m_state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");

    m_state.indent(fp);
    fp->write("typedef AtomRefBitStorage<EC> ABS;\n");
    fp->write("\n");

    //element typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;\n");
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
    fp->write(".GetDefaultAtom()) { }\n"); //returns object of type T

    //constructor here (used by const tmpVars) (e.g. t3706)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write("& d) : ");
    fp->write("AtomBitStorage<EC>");
    fp->write("(d) { ");
    fp->write("if(d.GetType() != Us::THE_INSTANCE.GetType()) FAIL(ILLEGAL_ARGUMENT);");
    fp->write(" }\n");

    // assignment constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str());
    fp->write("<EC> & arg) { ");
    fp->write("this->m_stg = arg.m_stg; }\n");

    //assignment constructor, atom arg, for convenience
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const AtomRefBitStorage<EC> & arg) : ");
    fp->write("AtomBitStorage<EC>");
    fp->write("(arg.ReadAtom()) { ");
    fp->write("if(arg.GetType() != Us::THE_INSTANCE.GetType()) FAIL(ILLEGAL_ARGUMENT);");
    fp->write(" }\n");

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

  void UlamTypeClassElement::genUlamTypeReadDefinitionForC(File * fp)
  {
    //if(WritePacked(getPackable()))
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //T
	fp->write(" read() const { ");
	fp->write("return ABS::");
	fp->write(readMethodForCodeGen().c_str());
	fp->write("(); }\n"); //done
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
	fp->write("} //reads entire BV\n");
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
	fp->write("(index * itemlen); }\n");  //rel offset
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypeClassElement::genUlamTypeWriteDefinitionForC(File * fp)
  {
    //ref param to avoid excessive copying
    //if(WritePacked(getPackable()))
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("void write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //T
	fp->write("& v) { ABS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(v); }\n"); //done
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
	fp->write("} //writes entire BV\n");
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
	fp->write("(index * itemlen, v); }\n");  //reloffset, val
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

  void UlamTypeClassElement::genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp)
  {
    s32 arraysize = getArraySize();
    s32 len = BITSPERATOM * arraysize;//getTotalBitSize(); //could be 0, includes arrays

    m_state.m_currentIndentLevel = 0;
    UTI scalarUTI =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalarUTI)->getUlamTypeMangledName();
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
    fp->write(";  //forward\n\n");

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
    m_state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");
    fp->write("\n");

    //constructor for conditional-as (auto)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, targ, NULL) { }\n"); //no effective self

    //constructor with exact same immediate storage (uc arg unused) e.g. t3671
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("("); //non-const for ref
    fp->write(mangledName.c_str());
    fp->write("<EC>& s, const UlamContext<EC>& uc) : UlamRef<EC>(0u, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, s, NULL) { }\n"); //no effective self

    //copy constructor here (uc arg unused)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r, const UlamContext<EC>& uc) : UlamRef<EC>(r, 0, r.GetLen(), r.GetEffectiveSelf()) { }\n");

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, effself) {}\n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(automangledName.c_str());
    fp->write("() {}\n");

#if 0
    //DO WE NEED THESE???
    //Unpacked Read Array Item
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
    fp->write(" readArrayItem(");
    fp->write("const u32 index, const u32 itemlen, const UlamContext<EC>& uc) const { return ");
    fp->write("UlamRef<EC>(");
    fp->write("*this, index * itemlen, "); //const ref, rel offset
    fp->write("itemlen, ");  //itemlen,
    fp->write("uc.LookupElementTypeFromContext(this->GetType()))."); //effself
    fp->write(readArrayItemMethodForCodeGen().c_str());
    fp->write("(); /* entire atom item */ }\n");

    //Unpacked Write Array Item
    m_state.indent(fp);
    fp->write("void writeArrayItem(const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
    fp->write("& v, const u32 index, const u32 itemlen, const UlamContext<EC>& uc) { ");
    fp->write("UlamRef<EC>(");
    fp->write("*this, index * itemlen, "); //rel offset
    fp->write("itemlen, ");  //itemlen,
    fp->write("uc.LookupElementTypeFromContext(v.GetType()))."); //effself
    fp->write(writeArrayItemMethodForCodeGen().c_str());
    fp->write("(v); /* entire atom item */ }\n");
#endif

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

  void UlamTypeClassElement::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    s32 arraysize = getArraySize();
    s32 len = BITSPERATOM * arraysize;//getTotalBitSize(); //could be 0, includes arrays

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
    m_state.indent(fp);
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");
    fp->write("\n");

    m_state.indent(fp);
    fp->write("typedef BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("> BV;\n");

    m_state.indent(fp);
    fp->write("typedef BitVectorBitStorage<EC, BV> BVS;\n");
    fp->write("\n");

    //element typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;\n");
    fp->write("\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
    fp->write(" tmpt = Us::THE_INSTANCE.GetDefaultAtom(); ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("writeArrayItem(tmpt, j, BPA); }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write("T& targ) { ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("writeArrayItem(targ, j, BPA); }\n");

    //the whole thing (e.g. t3709)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //BV
    fp->write("& d) { ");
    fp->write("write(d);");
    fp->write(" }\n");

    // assignment constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str());
    fp->write("<EC> & arg) { ");
    fp->write("this->m_stg = arg.m_stg; }\n");

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
    fp->write("writeArrayItem(tmpt, j, BPA); }\n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

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
