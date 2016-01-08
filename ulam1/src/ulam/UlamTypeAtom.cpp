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

  ULAMCLASSTYPE UlamTypeAtom::getUlamClass()
  {
    //return UC_ATOM; //llok into this!!
    return UC_NOTACLASS;
  }

  const std::string UlamTypeAtom::getUlamTypeVDAsStringForC()
  {
    //return "VD::ATOM";
    return "VD::BITS";
  }

  bool UlamTypeAtom::needsImmediateType()
  {
    return true;
  }

  const std::string UlamTypeAtom::getTmpStorageTypeAsString()
  {
    return "T";
  }

  const std::string UlamTypeAtom::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName() << "<EC>";
    return ctype.str();
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
    UlamType * vut = m_state.getUlamTypeByIndex(valtypidx);
    AssertBool isScalars = (vut->isScalar() && isScalar());
    assert(isScalars);

    assert(vut->getUlamClass() == UC_ELEMENT);
    // what change is to be made ????
    // atom type vs. class type
    // how can it be both in an UlamValue?
    // what of its contents?
    // val = UlamValue::makeAtom(valtypidx);
    val.setUlamValueTypeIdx(typidx); //try this
    return brtn;
  } //end cast

  FORECAST UlamTypeAtom::safeCast(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    UlamType * vut = m_state.getUlamTypeByIndex(typidx);
    if(vut->getUlamTypeEnum() == UAtom)
      return CAST_CLEAR; //atom to atom

    return (vut->getUlamClass() == UC_ELEMENT) ? CAST_CLEAR : CAST_BAD;
   } //safeCast

  const std::string UlamTypeAtom::castMethodForCodeGen(UTI nodetype)
  {
    std::ostringstream rtnMethod;
    UlamType * nut = m_state.getUlamTypeByIndex(nodetype);

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    u32 sizeByIntBitsToBe = getTotalWordSize();
    u32 sizeByIntBits = nut->getTotalWordSize();

    assert(sizeByIntBitsToBe == sizeByIntBits);
    assert(nut->getUlamClass() == UC_ELEMENT);  //quarks only cast toInt

    rtnMethod << "_" << "Element"  << sizeByIntBits << "To";
    rtnMethod << getUlamTypeNameOnly().c_str() << sizeByIntBitsToBe;
    return rtnMethod.str();
  } //castMethodForCodeGen

  void UlamTypeAtom::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    if(!isScalar())
      return; //no auto for unpacked array, just immediate

    m_state.m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName();
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
    fp->write(" : public AutoRefBase<EC>");
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

    // see UlamClass.h for AutoRefBase
    //constructor for ref (auto)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(T& targ) : AutoRefBase<EC>(targ, 0u) { }\n");

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(AutoRefBase<EC>& arg) : AutoRefBase<EC>(arg, 0u) { }\n");

    //read BV method
    genUlamTypeAutoReadDefinitionForC(fp);

    //write BV method
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

  void UlamTypeAtom::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    assert(isScalar()); //req custom array

    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write(" read() const { return AutoRefBase<EC>::");
    fp->write(readMethodForCodeGen().c_str());
    fp->write("(); /* entire atom */ ");
    fp->write("}\n"); //done
  } //genUlamTypeAutoReadDefinitionForC

  void UlamTypeAtom::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    assert(isScalar()); //req custom array
    m_state.indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T&
    fp->write("& v) { AutoRefBase<EC>::");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(v); /* entire atom */ ");
    fp->write("}\n");
  } //genUlamTypeAutoWriteDefinitionForC

  //inside out like ulamtypeclass: generates immediate for whole atom
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
    fp->write(automangledName.c_str());
    fp->write("<EC>\n");

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

    //storage here (as an atom)
    m_state.indent(fp);
    fp->write("T m_stg;  //storage here!\n\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : ");
    fp->write(automangledName.c_str());
    fp->write("<EC>");
    fp->write("(m_stg), ");
    fp->write("m_stg() { }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //u32
    fp->write("& d) : ");
    fp->write(automangledName.c_str());
    fp->write("<EC>");
    fp->write("(m_stg), ");
    fp->write("m_stg(d) { }\n");

    //copy constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str());
    fp->write("& d) : ");
    fp->write(automangledName.c_str());
    fp->write("<EC>");
    fp->write("(m_stg), ");
    fp->write("m_stg(d.m_stg) { }\n");

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
  } //genUlamTypeMangledDefinitionForC

  void UlamTypeAtom::genUlamTypeReadDefinitionForC(File * fp)
  {
    return; //uses base class read
  } //genUlamTypeReadDefinitionForC

  void UlamTypeAtom::genUlamTypeWriteDefinitionForC(File * fp)
  {
    return; //uses base class write
  } //genUlamTypeWriteDefinitionForC

  void UlamTypeAtom::genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp)
  {
    m_state.m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName();
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();
    std::ostringstream  ud;
    ud << "Ud_" << mangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    //u32 itemlen = getBitSize();
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

    //storage here (an array of T's)
    m_state.indent(fp);
    fp->write("T m_stgarr[");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u];  //big storage here!\n\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("m_stgarr[j].SetEmptyImpl();"); //T::ATOM_EMPTY_TYPE
    fp->write(" }\n");

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
    fp->write("m_stgarr[index]; /* entire atom item */ }\n");

    //Unpacked Write entire Array Item
    m_state.indent(fp);
    fp->write("void writeArrayItem(const ");
    fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T&o
    fp->write("& v, const u32 index, const u32 itemlen) { ");
    fp->write("m_stgarr[index] = v; /* entire atom item */ }\n");

    //Unpacked, an item T
    m_state.indent(fp);
    fp->write("BitVector<BPA>& ");
    fp->write(" getBits(");
    fp->write("const u32 index) { return ");
    fp->write("m_stgarr[index].GetBits(); }\n");

    //Unpacked, an item T const
    m_state.indent(fp);
    fp->write("const BitVector<BPA>& ");
    fp->write(" getBits(");
    fp->write("const u32 index) const { return ");
    fp->write("m_stgarr[index].GetBits(); }\n");

    //Unpacked, an item T&
    m_state.indent(fp);
    fp->write("T& ");
    fp->write(" getRef(");
    fp->write("const u32 index) { return ");
    fp->write("m_stgarr[index]; }\n");

    //Unpacked, position within whole
    m_state.indent(fp);
    fp->write("const u32 ");
    fp->write(" getPosOffset(");
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
  } //genUlamTypeMangledUnpackedArrayDefinitionForC

  const std::string UlamTypeAtom::readMethodForCodeGen()
  {
    return "read";
  }

  const std::string UlamTypeAtom::writeMethodForCodeGen()
  {
    return "write";
  }

  void UlamTypeAtom::genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp)
  {
    assert(0); //only primitive types
  }

} //end MFM
