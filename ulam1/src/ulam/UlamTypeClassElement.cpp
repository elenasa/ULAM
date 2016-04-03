#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClassElement.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClassElement::UlamTypeClassElement(const UlamKeyTypeSignature key, CompilerState & state) : UlamTypeClass(key, state)
  {
    setTotalWordSize(BITSPERATOM);
    setItemWordSize(BITSPERATOM);
  }

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
	//no longer elements inherit from elements, only quarks.
	brtn = false;
      }
    else if(vetype == UAtom)
      val.setAtomElementTypeIdx(typidx); //for testing purposes, assume ok
    //else true
    return brtn;
  } //end cast

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
    return UNPACKED; //was PACKED, now matches ATOM regardless of its bit size.
  }

  const std::string UlamTypeClassElement::readMethodForCodeGen()
  {
    return "ReadAtom";
  }

  const std::string UlamTypeClassElement::writeMethodForCodeGen()
  {
    return "WriteAtom";
  }

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

  const std::string UlamTypeClassElement::getTmpStorageTypeAsString()
  {
    return "T";
  } //getTmpStorageTypeAsString

  const std::string UlamTypeClassElement::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName();
    ctype << "<EC>";
    return ctype.str();
  } //getLocalStorageTypeAsString

  STORAGE UlamTypeClassElement::getTmpStorageTypeForTmpVar()
  {
    return TMPTATOM;
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
    //regardless of actual element size, it takes up the full atom space
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
    fp->write("\n");

    m_state.m_currentIndentLevel++;

    //forward declaration of element (before struct!)
    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(getUlamTypeMangledName().c_str());
    fp->write(";  //forward\n\n");

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(automangledName.c_str());
    fp->write(" : public UlamRefAtom<EC>");
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
    //m_state.indent(fp);
    //fp->write("typedef BitField<BitVector<BPA>, VD::BITS, T::ATOM_FIRST_STATE_BIT, 0> BFTYP;\n");
    fp->write("\n");

    //element typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeMangledName().c_str());
    fp->write("<EC> Us;\n");
    fp->write("\n");

    //constructor for conditional-as (auto)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(AtomBitStorage<EC>& targ, const UlamClass<EC> * effself) : UlamRefAtom<EC>(targ, effself) { }\n");

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRefAtom<EC>& arg, const UlamClass<EC> * effself) : UlamRefAtom<EC>(arg, effself) { }\n");

    //copy constructor here
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r) : UlamRefAtom<EC>(r, r.GetEffectiveSelf()) { }\n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(automangledName.c_str());
    fp->write("() {}\n");

    //read 'entire atom' method
    genUlamTypeAutoReadDefinitionForC(fp);

    //write 'entire atom' method
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
    // arrays are handled separately
    assert(isScalar());
  } //genUlamTypeAutoReadDefinitionForC

  void UlamTypeClassElement::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    // arrays are handled separately
    assert(isScalar());

    // write must be scalar; ref param to avoid excessive copying
    //not an array
    m_state.indent(fp);
    fp->write("void");
    fp->write(" write(");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write("& targ) { return UlamRefAtom<EC>::WriteAtom(targ); /* entire element */ }\n");

    //m_state.indent(fp);
    //fp->write("void writeTypeField(const u32 v)");
    //fp->write("{ UlamRefAtom<EC>::SetType(v); }\n"); //GONE???
  } //genUlamTypeAutoWriteDefinitionForC

  //builds the immediate definition, inheriting from auto
  void UlamTypeClassElement::genUlamTypeMangledDefinitionForC(File * fp)
  {
    //regardless of actual element size, it takes up the full atom space
    if(!isScalar())
      return genUlamTypeMangledUnpackedArrayDefinitionForC(fp);

    //class instance idx is always the scalar uti
    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
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

    //forward declaration of quark (before struct!)
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
    fp->write("AtomBitStorage<EC>\n"); //storage here!

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

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : ");
    fp->write("AtomBitStorage<EC>(");
    fp->write("Us::THE_INSTANCE");
    fp->write(".GetDefaultAtom()"); //returns object of type T
    fp->write(") { }\n");

    //constructor here (used by const tmpVars); ref param to avoid excessive copying
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write("& d) : ");
    fp->write("AtomBitStorage<EC>(d) { }\n");

    //assignment constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    //fp->write(mangledName.c_str());
    fp->write("AtomBitStorage");
    fp->write("<EC> & arg) : ");
    fp->write("AtomBitStorage<EC>(arg) {");
    fp->write("if(arg.ReadAtom().GetType() != this->ReadAtom().GetType()) FAIL(ILLEGAL_ARGUMENT); } \n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

    //read 'entire atom' method
    genUlamTypeReadDefinitionForC(fp);

    //write 'entire atom' method
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

  void UlamTypeClassElement::genUlamTypeReadDefinitionForC(File * fp)
  {
    // arrays are handled separately
    assert(isScalar());
  } //genUlamTypeReadDefinitionForC

  void UlamTypeClassElement::genUlamTypeWriteDefinitionForC(File * fp)
  {
    // arrays are handled separately
    assert(isScalar());

    // write must be scalar; ref param to avoid excessive copying
    //not an array
    m_state.indent(fp);
    fp->write("void");
    fp->write(" write(");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write("& targ) { WriteAtom(targ); /* entire element */ }\n");

    //m_state.indent(fp);
    //fp->write("void writeTypeField(const u32 v)");
    //fp->write("{ GetStorage().SetType(v); }\n"); //GONE???
  } //genUlamTypeWriteDefinitionForC

  //similar to ulamtypeatom array of unpacked atoms
  void UlamTypeClassElement::genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp)
  {
    m_state.m_currentIndentLevel = 0;

    //class instance idx is always the scalar uti
    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();
    const std::string mangledName = getUlamTypeImmediateMangledName();

    std::ostringstream  ud;
    ud << "Ud_" << automangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    u32 arraysize = getArraySize();

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

    //storage here (an array of T's)
    m_state.indent(fp);
    fp->write("AtomBitStorage<EC> (& m_stgarrayref)[");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u];  //ref to BIG storage!\n\n");

    //copy constructor here
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r) : m_stgarrayref(r.m_stgarrayref) { }\n");

    //constructor here (used by tmpVars)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(");
    fp->write(mangledName.c_str());
    fp->write("<EC>& d) : m_stgarrayref(d.getStorageRef()) { }\n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(automangledName.c_str());
    fp->write("() {}\n");

    //Unpacked Read entire Array Item
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
    fp->write(" readArrayItem(");
    fp->write("const u32 index, const u32 itemlen) const { return ");
    fp->write("m_stgarrayref[index].ReadAtom(); /* entire element item */ }\n");

    //Unpacked Write entire Array Item
    m_state.indent(fp);
    fp->write("void writeArrayItem(const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T&
    fp->write("& v, const u32 index, const u32 itemlen) { ");
    fp->write("m_stgarrayref[index].WriteAtom(v); /* entire element item */ }\n");

    //Unpacked, an item T
    m_state.indent(fp);
    fp->write("BitStorage<EC>& ");
    fp->write("getBits(");
    fp->write("const u32 index) { return ");
    fp->write("m_stgarrayref[index]; }\n");

    //Unpacked, an item T const
    m_state.indent(fp);
    fp->write("const BitStorage<EC>& ");
    fp->write("getBits(");
    fp->write("const u32 index) const { return ");
    fp->write("m_stgarrayref[index]; }\n");

    //Unpacked, an item T&
    m_state.indent(fp);
    fp->write("T& ");
    fp->write("getRef(");
    fp->write("const u32 index) { return ");
    fp->write("m_stgarrayref[index].GetStorage(); }\n");

    //Unpacked, position within whole
    m_state.indent(fp);
    fp->write("const u32 ");
    fp->write("getPosOffset(");
    fp->write("const u32 index) const { return ");
    fp->write("(BPA * index); }\n");

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
  } //genUlamTypeMangledUnpackedElementArrayAutoDefinitionForC

  //similar to ulamtypeatom array of unpacked atoms
  void UlamTypeClassElement::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    m_state.m_currentIndentLevel = 0;

    //class instance idx is always the scalar uti
    UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
    const std::string mangledName = getUlamTypeImmediateMangledName();

    std::ostringstream  ud;
    ud << "Ud_" << mangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    u32 arraysize = getArraySize();

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

    //Unpacked, storage reference T (&) [N]
    m_state.indent(fp);
    fp->write("typedef AtomBitStorage<EC> TARR[");
    fp->write_decimal_unsigned(arraysize);
    fp->write("];\n");

    //storage here (an array of T's)
    m_state.indent(fp);
    fp->write("TARR m_stgarr;  //BIG storage here!\n\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("writeArrayItem(");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC>::THE_INSTANCE");
    fp->write(".GetDefaultAtom()"); //returns object of type T
    fp->write(", j, BPA); }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write("& d) { ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) {");
    fp->write("writeArrayItem(d, j, BPA);");
    fp->write(" } }\n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

    //Unpacked Read entire Array Item
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T
    fp->write(" readArrayItem(");
    fp->write("const u32 index, const u32 itemlen) const { return ");
    fp->write("m_stgarr[index].ReadAtom(); /* entire element item */ }\n");

    //Unpacked Write entire Array Item
    m_state.indent(fp);
    fp->write("void writeArrayItem(const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T&
    fp->write("& v, const u32 index, const u32 itemlen) { ");
    fp->write("m_stgarr[index].WriteAtom(v); /* entire element item */ }\n");

    //Unpacked, an item T
    m_state.indent(fp);
    fp->write("BitStorage<EC>& ");
    fp->write("getBits(");
    fp->write("const u32 index) { return ");
    fp->write("m_stgarr[index]; }\n");

    //Unpacked, an item T const
    m_state.indent(fp);
    fp->write("const BitStorage<EC>& ");
    fp->write("getBits(");
    fp->write("const u32 index) const { return ");
    fp->write("m_stgarr[index]; }\n");

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
    fp->write("(BPA * index); }\n");

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

} //end MFM
