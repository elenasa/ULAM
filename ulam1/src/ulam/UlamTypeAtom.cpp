#include <sstream>
#include <stdio.h>
#include "UlamTypeAtom.h"
#include "UlamValue.h"
#include "CompilerState.h"

namespace MFM {

  UlamTypeAtom::UlamTypeAtom(const UlamKeyTypeSignature key) : UlamType(key)
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
  }


   ULAMTYPE UlamTypeAtom::getUlamTypeEnum()
   {
     return UAtom;
   }

#if 0
  ULAMCLASSTYPE UlamTypeAtom::getUlamClass()
  {
    return UC_ATOM;  //???
  }
#endif

  const std::string UlamTypeAtom::getUlamTypeVDAsStringForC()
  {
    return "VD::ATOM";
  }


  bool UlamTypeAtom::needsImmediateType()
  {
    //return false;
    return true;
  }


  const std::string UlamTypeAtom::getTmpStorageTypeAsString(CompilerState * state)
  {
    return "T";
  }

  const std::string UlamTypeAtom::getImmediateStorageTypeAsString(CompilerState * state)
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName(state) << "<CC>";
    return ctype.str();
  } //getImmediateStorageTypeAsString


  const char * UlamTypeAtom::getUlamTypeAsSingleLowercaseLetter()
  {
    return "a";  //self ???
  }

  bool UlamTypeAtom::isMinMaxAllowed()
  {
    return false;
  }

  PACKFIT UlamTypeAtom::getPackable()
  {
    return UNPACKED;
  }


  bool UlamTypeAtom::cast(UlamValue & val, UTI typidx, CompilerState& state)
  {
    bool brtn = true;
    //UTI typidx = getUlamTypeIndex();
    assert(state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();
    UlamType * vut = state.getUlamTypeByIndex(valtypidx);
    assert(vut->isScalar() && isScalar());

    assert(vut->getUlamClass() == UC_ELEMENT);
    // what change is to be made ????
    // atom type vs. class type
    // how can it be both in an UlamValue?
    // what of its contents?
    // val = UlamValue::makeAtom(valtypidx);

    return brtn;
  } //end cast


  const std::string UlamTypeAtom::castMethodForCodeGen(UTI nodetype, CompilerState& state)
  {
    std::ostringstream rtnMethod;
    UlamType * nut = state.getUlamTypeByIndex(nodetype);

    //base types e.g. Int, Bool, Unary, Foo, Bar..
    s32 sizeByIntBitsToBe = getTotalWordSize();
    s32 sizeByIntBits = nut->getTotalWordSize();

    assert(sizeByIntBitsToBe == sizeByIntBits);
    assert(nut->getUlamClass() == UC_ELEMENT);  //quarks only cast toInt

    rtnMethod << "_" << "Element"  << sizeByIntBits << "To" << getUlamTypeNameOnly(&state).c_str() << sizeByIntBitsToBe;
    return rtnMethod.str();
  } //castMethodForCodeGen


  //whole atom
  void UlamTypeAtom::genUlamTypeMangledDefinitionForC(File * fp, CompilerState * state)
  {
    assert(isScalar());

    state->m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName(state);
    std::ostringstream  ud;
    ud << "Ud_" << mangledName;  //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    //s32 len = getTotalBitSize();  //BITSPERATOM

    state->indent(fp);
    fp->write("#ifndef ");
    fp->write(udstr.c_str());
    fp->write("\n");

    state->indent(fp);
    fp->write("#define ");
    fp->write(udstr.c_str());
    fp->write("\n");

    state->indent(fp);
    fp->write("namespace MFM{\n");
    fp->write("\n");

    state->m_currentIndentLevel++;

    state->indent(fp);
    fp->write("template<class CC>\n");

    state->indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
    fp->write("\n");
    state->indent(fp);
    fp->write("{\n");

    state->m_currentIndentLevel++;

    //typedef atomic parameter type inside struct
    state->indent(fp);
    fp->write("typedef typename CC::ATOM_TYPE T;\n");
    state->indent(fp);
    fp->write("typedef typename CC::PARAM_CONFIG P;\n");
    state->indent(fp);
    fp->write("enum { BPA = P::BITS_PER_ATOM };\n");

    //storage here in atom
    state->indent(fp);
    fp->write("T m_stg;  //storage here!\n");

    //default constructor (used by local vars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stg() { }\n");

    //constructor here (used by const tmpVars); ref param to avoid excessive copying
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //T
    fp->write("& d) : m_stg(d) {}\n");

    //copy constructor here (used by func call return values)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getImmediateStorageTypeAsString(state).c_str());
    fp->write("& d) : m_stg(d.m_stg) {}\n");

    //default destructor (for completeness)
    state->indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

    //read 'entire atom' method
    genUlamTypeReadDefinitionForC(fp, state);

    //write 'entire atom' method
    genUlamTypeWriteDefinitionForC(fp, state);

    // non-const T ref method for scalar
    state->indent(fp);
    fp->write("T& getRef() { return m_stg; }\n");

    state->m_currentIndentLevel--;
    state->indent(fp);
    fp->write("};\n");

    state->m_currentIndentLevel--;
    state->indent(fp);
    fp->write("} //MFM\n");

    state->indent(fp);
    fp->write("#endif /*");
    fp->write(udstr.c_str());
    fp->write(" */\n\n");
  } //genUlamTypeMangledDefinitionForC


  void UlamTypeAtom::genUlamTypeReadDefinitionForC(File * fp, CompilerState * state)
  {
    // arrays are handled separately
    assert(isScalar());

    //not an array
    state->indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //T
    fp->write(" read() const { return m_stg; }\n");
  } //genUlamTypeReadDefinitionForC


  void UlamTypeAtom::genUlamTypeWriteDefinitionForC(File * fp, CompilerState * state)
  {
    // arrays are handled separately
    assert(isScalar());

    // here, must be scalar; ref param to avoid excessive copying
    state->indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //T
    fp->write("& v) { m_stg = v; }\n");
  } //genUlamTypeWriteDefinitionForC


} //end MFM
