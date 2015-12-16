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

  const std::string UlamTypeAtom::getImmediateStorageTypeAsString()
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
    genUlamTypeReadDefinitionForC(fp);

    //write BV method
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
  } //genUlamTypeMangledAutoDefinitionForC

  void UlamTypeAtom::genUlamTypeReadDefinitionForC(File * fp)
  {
    assert(isScalar()); //req custom array

    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write(" read() const { return AutoRefBase<EC>::");
    fp->write(readMethodForCodeGen().c_str());
    fp->write("(); /* entire atom */ ");
    fp->write("}\n"); //done
  } //genUlamTypeReadDefinitionForC

  void UlamTypeAtom::genUlamTypeWriteDefinitionForC(File * fp)
  {
    assert(isScalar()); //req custom array
    m_state.indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T&
    fp->write("& v) { AutoRefBase<EC>::");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(v); /* entire atom */ ");
    fp->write("}\n");
  } //genUlamTypeWriteDefinitionForC

  //inside out like ulamtypeclass: generates immediate for whole atom
  void UlamTypeAtom::genUlamTypeMangledDefinitionForC(File * fp)
  {
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
