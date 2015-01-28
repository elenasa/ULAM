#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClass.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  static const char * CUSTOMARRAY_GET_MANGLEDNAME = "Uf_4aref";

  static const char * CUSTOMARRAY_SET_MANGLEDNAME = "Uf_4aset";


  UlamTypeClass::UlamTypeClass(const UlamKeyTypeSignature key, ULAMCLASSTYPE type) : UlamType(key), m_class(type), m_customArray(false), m_customArrayType(Nav)
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
  }

   ULAMTYPE UlamTypeClass::getUlamTypeEnum()
   {
     return Class;
   }

  bool UlamTypeClass::cast(UlamValue & val, UTI typidx, CompilerState& state)
  {
    bool brtn = true;
    //    UTI typidx = getUlamTypeIndex();
    assert(state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();
    UlamType * vut = state.getUlamTypeByIndex(valtypidx);
    assert(vut->isScalar() && isScalar());

    assert(m_class == UC_ELEMENT);
    assert(valtypidx == UAtom || valtypidx == typidx);
    // what change is to be made ????
    // atom type vs. class type
    // how can it be both in an UlamValue?
    // what of its contents?
    // val = UlamValue::makeAtom(valtypidx);

    return brtn;
  } //end cast

  const char * UlamTypeClass::getUlamTypeAsSingleLowercaseLetter()
  {
    switch(m_class)
      {
      case UC_ELEMENT:
	return "e";
      case UC_QUARK:
	return "q";
      case UC_INCOMPLETE:
      default:
	assert(0);
      };
    return "?";
  } //getUlamTypeAsSingleLowercaseLetter()

  const std::string UlamTypeClass::getUlamTypeVDAsStringForC()
  {
    return "VD::BITS";  //for quark use bits
  }

  const std::string UlamTypeClass::getUlamTypeMangledName(CompilerState * state)
  {
    std::ostringstream mangledclassname;
    mangledclassname << UlamType::getUlamTypeMangledName(state);

    //append 0, or each digi-encoded: numberOfParameters, (enum) types and values
    u32 id = m_key.getUlamKeyTypeSignatureNameId();
    UTI cuti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    SymbolClassName * cnsym = (SymbolClassName *) state->m_programDefST.getSymbolPtr(id);
    mangledclassname << cnsym->formatAnInstancesArgValuesAsAString(cuti);
    return mangledclassname.str();
  } //getUlamTypeMangledName

  //quarks are right-justified in an atom space
  const std::string UlamTypeClass::getUlamTypeAsStringForC()
  {
    assert(m_class != UC_INCOMPLETE);
    if(m_class == UC_QUARK)
      {
	return UlamType::getUlamTypeAsStringForC();
      }
    return "T"; //for elements
  } //getUlamTypeAsStringForC()

  const std::string UlamTypeClass::getUlamTypeUPrefix()
  {
    if(getArraySize() > 0)
      return "Ut_";

    switch(m_class)
      {
      case UC_ELEMENT:
	return "Ue_";
      case UC_QUARK:
	return "Uq_";
      case UC_INCOMPLETE:
	return "U?_";
     default:
	assert(0);
      };
    return "xx";
  } //getUlamTypeUPrefix

  const std::string UlamTypeClass::getUlamTypeNameBrief(CompilerState * state)
  {
    return m_key.getUlamKeyTypeSignatureName(state);
  }

  //see SymbolVariableDataMember printPostfix for recursive output
  void UlamTypeClass::getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state)
  {
    if(prefix == 'z')
      sprintf(valstr,"%d", data);
    else
      sprintf(valstr,"%c%d", prefix, data);
  }

  ULAMCLASSTYPE UlamTypeClass::getUlamClass()
  {
    return m_class;
  }

  void UlamTypeClass::setUlamClass(ULAMCLASSTYPE type)
  {
    m_class = type;
    if(m_class == UC_ELEMENT)
      {
	setTotalWordSize(BITSPERATOM);
	setItemWordSize(BITSPERATOM);
      }
  } //setUlamClass

  bool UlamTypeClass::isConstant()
  {
    return false;   //e.g. zero-size quark is not a constant
  }

  bool UlamTypeClass::isScalar()
  {
    return (m_key.getUlamKeyTypeSignatureArraySize() == NONARRAYSIZE);
  }

  bool UlamTypeClass::isCustomArray()
  {
    return m_customArray;
  }

  void UlamTypeClass::setCustomArrayType(UTI type)
  {
    assert(isScalar());
    m_customArray = true;
    m_customArrayType = type;
  }

  UTI UlamTypeClass::getCustomArrayType()
  {
    assert(m_customArray);
    return m_customArrayType;
  }

  s32 UlamTypeClass::getBitSize()
  {
    s32 bitsize = m_key.getUlamKeyTypeSignatureBitSize();
    //return bitsize < 0 ? 0 : bitsize; //allow for empty quarks
    return bitsize;  //could be negative "unknown"
  }

  bool UlamTypeClass::isMinMaxAllowed()
  {
    return false;
  }

  PACKFIT UlamTypeClass::getPackable()
  {
    if(m_class == UC_ELEMENT)
      return UNPACKED; //was PACKED, now matches ATOM regardless of its bit size.

    return UlamType::getPackable();  //quarks depend their size
  }

  bool UlamTypeClass::needsImmediateType()
  {
    // gencode is too late for UC_INCOMPLETE
    // NOW allowing complete immediate elements (like atoms)
    // simply test for quarks..since..
    //   also needed for 'empty' quarks without data
    // NOW allowing right-justified naked quarks
    //return (m_class == UC_ELEMENT || m_class == UC_INCOMPLETE || getBitSize() == 0 ? false : true);
    //return (m_class == UC_QUARK);
    return (m_class == UC_QUARK || m_class == UC_ELEMENT);
  } //needsImmediateType

  const std::string UlamTypeClass::getUlamTypeImmediateMangledName(CompilerState * state)
  {
    if(needsImmediateType())
      {
	return UlamType::getUlamTypeImmediateMangledName(state);
      }
    return "T";
  } //getUlamTypeImmediateMangledName

  const std::string UlamTypeClass::getUlamTypeImmediateAutoMangledName(CompilerState * state)
  {
    assert(needsImmediateType());
    std::ostringstream  automn;
    automn << getUlamTypeImmediateMangledName(state).c_str();

    if(m_class == UC_QUARK)
      automn << "4auto" ;

    return automn.str();
  } //getUlamTypeImmediateAutoMangledName

  const std::string UlamTypeClass::getTmpStorageTypeAsString(CompilerState * state)
  {
    if(m_class == UC_QUARK)
      {
	return UlamType::getTmpStorageTypeAsString(state); // entire, u32 or u64
      }
    else if(m_class == UC_ELEMENT)
      {
 	return "T";
      }
    else
      assert(0);
    return ""; //error!
  } //getTmpStorageTypeAsString

  const std::string UlamTypeClass::getArrayItemTmpStorageTypeAsString(CompilerState * state)
  {
    if(!isScalar())
      return UlamType::getArrayItemTmpStorageTypeAsString(state);

    assert(isCustomArray());
    return state->getUlamTypeByIndex(getCustomArrayType())->getTmpStorageTypeAsString(state);
  } //getArrayItemTmpStorageTypeAsString

  const std::string UlamTypeClass::getImmediateStorageTypeAsString(CompilerState * state)
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName(state);

    if(m_class == UC_QUARK && isScalar())
      ctype << "<CC>";  // not ,POS> because immediates know their position

    if(m_class == UC_ELEMENT)
      ctype << "<CC>";

    return ctype.str();
  } //getImmediateStorageTypeAsString

  void UlamTypeClass::genUlamTypeReadDefinitionForC(File * fp, CompilerState * state)
  {
    if(m_class == UC_QUARK)
      return genUlamTypeQuarkReadDefinitionForC(fp,state);
    else if(m_class == UC_ELEMENT)
      return genUlamTypeElementReadDefinitionForC(fp,state);
    else
      assert(0);
  } //genUlamTypeReadDefinitionForC

  void UlamTypeClass::genUlamTypeWriteDefinitionForC(File * fp, CompilerState * state)
  {
    if(m_class == UC_QUARK)
      return genUlamTypeQuarkWriteDefinitionForC(fp,state);
    else if(m_class == UC_ELEMENT)
      return genUlamTypeElementWriteDefinitionForC(fp,state);
    else
      assert(0);
  } //genUlamTypeWriteDefinitionForC

  const std::string UlamTypeClass::castMethodForCodeGen(UTI nodetype, CompilerState& state)
  {
    std::ostringstream rtnMethod;
    UlamType * nut = state.getUlamTypeByIndex(nodetype);
    //base types e.g. Int, Bool, Unary, Foo, Bar..
    s32 sizeByIntBitsToBe = getTotalWordSize();
    s32 sizeByIntBits = nut->getTotalWordSize();

    //assert(sizeByIntBitsToBe == sizeByIntBits);
    if(sizeByIntBitsToBe != sizeByIntBits)
      {
	std::ostringstream msg;
	msg << "Casting different word sizes; " << sizeByIntBits << ", Value Type and size was: " << nut->getUlamTypeName(&state).c_str() << ", to be: " << sizeByIntBitsToBe << " for type: " << getUlamTypeName(&state).c_str();// << " -- [" << state.getLocationTextAsString(state.m_locOfNextLineText).c_str() << "]";
	MSG3(state.getFullLocationAsString(state.m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
      }

    //assert(m_class == UC_ELEMENT);  //quarks only cast toInt
    if(m_class != UC_ELEMENT)
      {
	std::ostringstream msg;
	msg << "Quarks only cast 'toInt': value type and size was: " << nut->getUlamTypeName(&state).c_str() << ", to be: " << getUlamTypeName(&state).c_str(); // << " -- [" << state.getLocationTextAsString(state.m_locOfNextLineText).c_str() << "]";
	MSG3(state.getFullLocationAsString(state.m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
      }

    //e.g. casting an element to an element, redundant and not supported: Element96ToElement96?
    if(nodetype != UAtom)
      {
	std::ostringstream msg;
	msg << "Attempting to illegally cast a non-atom type to an element: value type and size was: " << nut->getUlamTypeName(&state).c_str() << ", to be: " << getUlamTypeName(&state).c_str(); // << " -- [" << state.getLocationTextAsString(state.m_locOfNextLineText).c_str() << "]";
	MSG3(state.getFullLocationAsString(state.m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
      }

    rtnMethod << "_" << nut->getUlamTypeNameOnly(&state).c_str()  << sizeByIntBits << "ToElement" << sizeByIntBitsToBe;
    return rtnMethod.str();
  } //castMethodForCodeGen

  void UlamTypeClass::genUlamTypeMangledDefinitionForC(File * fp, CompilerState * state)
  {
    if(m_class == UC_QUARK)
      return genUlamTypeQuarkMangledDefinitionForC(fp,state);
    else if(m_class == UC_ELEMENT)
      return genUlamTypeElementMangledDefinitionForC(fp,state);
    else
      assert(0);
  } //genUlamTypeMangledDefinitionForC


  void UlamTypeClass::genUlamTypeQuarkMangledDefinitionForC(File * fp, CompilerState * state)
  {
    assert(m_class == UC_QUARK);

    if(!isScalar())
      {
	// immediate quark arrays are treated like immediate primitives, not quarks
	  return UlamType::genUlamTypeMangledDefinitionForC(fp, state);
      }

    //is scalar here..
    state->m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName(state);
    std::ostringstream  ud;
    ud << "Ud_" << mangledName;  //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    //s32 sizeByIntBits = getTotalWordSize();
    s32 len = getTotalBitSize();

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

    //forward declaration of quark (before struct!)
    state->indent(fp);
    fp->write("template<class CC, u32 POS> class ");
    fp->write(getUlamTypeMangledName(state).c_str());
    fp->write(";  //forward\n\n");

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
    fp->write("\n");

    //quark typedef (right-justified)
    state->indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeMangledName(state).c_str());
    fp->write("<CC, ");
    fp->write_decimal(BITSPERATOM - len);
    fp->write("> Us;\n");

    state->indent(fp);
    fp->write("typedef AtomicParameterType<");
    fp->write("CC");  //BITSPERATOM
    fp->write(", ");
    fp->write(getUlamTypeVDAsStringForC().c_str());
    fp->write(", ");
    fp->write_decimal(len);   //include arraysize
    fp->write(", ");
    fp->write_decimal(BITSPERATOM - len); //right-justified
    fp->write("> ");
    fp->write(" Up_Us;\n");

    //storage here in atom
    state->indent(fp);
    fp->write("T m_stg;  //storage here!\n");

    //default constructor (used by local vars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stg() { }\n");

    //constructor here (used by const tmpVars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
    //fp->write(" d) : m_t(d) {}\n");
    fp->write(" d) { write(d); }\n");

    //copy constructor here (used by func call return values)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getImmediateStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write("& d) : m_stg(d.m_stg) {}\n");

    //default destructor (for completeness)
    state->indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

    //read 'entire quark' method
    genUlamTypeQuarkReadDefinitionForC(fp, state);

    //write 'entire quark' method
    genUlamTypeQuarkWriteDefinitionForC(fp, state);

    // getBits method for scalar
    state->indent(fp);
    fp->write("BitVector<BPA>& getBits() { return m_stg.GetBits(); }\n");

    // non-const T ref method for scalar
    state->indent(fp);
    fp->write("T& getRef() { return m_stg; }\n");

    if(isCustomArray())
      genCustomArrayMangledDefinitionForC(fp, state);

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
  } //genUlamTypeQuarkMangledDefinitionForC

  void UlamTypeClass::genUlamTypeQuarkReadDefinitionForC(File * fp, CompilerState * state)
  {
    // arrays are handled separately, not like a quark, more like a primitive
    if(!isScalar())
      return UlamType::genUlamTypeReadDefinitionForC(fp,state);

    //not an array
    state->indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write(" read() const { return Up_Us::");
    fp->write(readMethodForCodeGen().c_str());
    fp->write("(m_stg.GetBits() ); }\n");
  } //genUlamTypeQuarkReadDefinitionForC

  void UlamTypeClass::genUlamTypeQuarkWriteDefinitionForC(File * fp, CompilerState * state)
  {
    // arrays are handled separately, not like a quark, more like a primitive
    if(!isScalar())
      return UlamType::genUlamTypeWriteDefinitionForC(fp,state);

    // here, must be scalar
    state->indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write(" v) { Up_Us::");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(m_stg.GetBits(), v); }\n");
  } //genUlamTypeQuarkWriteDefinitionForC


  void UlamTypeClass::genCustomArrayMangledDefinitionForC(File * fp, CompilerState * state)
  {
    assert(m_class == UC_QUARK);

    assert(isScalar() && isCustomArray());

    state->indent(fp);
    fp->write("//CustomArray accessor methods:\n");
    // reads an element of array
    state->indent(fp);
    fp->write("const ");
    fp->write(getArrayItemTmpStorageTypeAsString(state).c_str()); //T
    fp->write(" readArrayItem(");
    fp->write("UlamContext<CC>& uc, ");
    fp->write("const u32 index, const u32 unitsize) { return Us::"); //const unhappy w first arg
    fp->write(readArrayItemMethodForCodeGen().c_str());  //aref
    fp->write("(uc, m_stg, ");
    fp->write(state->getUlamTypeByIndex(Int)->getImmediateStorageTypeAsString(state).c_str());
    fp->write("((const s32)index)).read(); }\n");

    // reads an element of array
    state->indent(fp);
    fp->write("void writeArrayItem(");
    fp->write("UlamContext<CC>& uc, const ");
    fp->write(getArrayItemTmpStorageTypeAsString(state).c_str()); //s32 or u32
    fp->write(" v, const u32 index, const u32 unitsize) { Us::");
    fp->write(writeArrayItemMethodForCodeGen().c_str());  //aset
    fp->write("(uc, m_stg, ");
    fp->write(state->getUlamTypeByIndex(Int)->getImmediateStorageTypeAsString(state).c_str());
    fp->write("((const s32) index), ");
    fp->write(state->getUlamTypeByIndex(UAtom)->getImmediateStorageTypeAsString(state).c_str());
    fp->write("(v)); }\n");
  } //genCustomArrayMangledDefinitionForC

  //whole element (based on atom)
  void UlamTypeClass::genUlamTypeElementMangledDefinitionForC(File * fp, CompilerState * state)
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

    //forward declaration of element (before struct!)
    state->indent(fp);
    fp->write("template<class CC> class ");
    fp->write(getUlamTypeMangledName(state).c_str());
    fp->write(";  //forward\n\n");

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

    //element typedef
    state->indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeMangledName(state).c_str());
    fp->write("<CC> Us;\n");

    //storage here in atom
    state->indent(fp);
    fp->write("T m_stg;  //possible storage here\n");

    // ref to storage in atom
    state->indent(fp);
    fp->write("T& m_stgRef;  //storage ref\n");

    //default constructor (used by local vars)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stg(), m_stgRef(m_stg) { }\n");

    //constructor here (used by const tmpVars); ref param to avoid excessive copying
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //T
    fp->write("& d) : m_stg(d), m_stgRef(m_stg) {}\n");

    if(m_class == UC_ELEMENT)
      {
	//constructor here (used by As for elements);
	// ref points directly into an atom; not so for quark
	// since pos is required for template (uses auto).
	state->indent(fp);
	fp->write(mangledName.c_str());
	fp->write("(");   //not const
	fp->write(getTmpStorageTypeAsString(state).c_str()); //T&
	fp->write("& d, bool badass) : m_stg(), m_stgRef(d) {}\n");
      }

    //copy constructor here (used by func call return values)
    state->indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getImmediateStorageTypeAsString(state).c_str());
    fp->write("& d) : m_stg(d.m_stg), m_stgRef(m_stg) {}\n");

    //default destructor (for completeness)
    state->indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

    //read 'entire atom' method
    genUlamTypeElementReadDefinitionForC(fp, state);

    //write 'entire atom' method
    genUlamTypeElementWriteDefinitionForC(fp, state);

    // getBits method for scalar
    state->indent(fp);
    fp->write("BitVector<BPA>& getBits() { return m_stgRef.GetBits(); }\n");

    // non-const T ref method for scalar
    state->indent(fp);
    fp->write("T& getRef() { return m_stgRef; }\n");

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
  } //genUlamTypeElementMangledDefinitionForC


  void UlamTypeClass::genUlamTypeElementReadDefinitionForC(File * fp, CompilerState * state)
  {
    // arrays are handled separately
    assert(isScalar());

    //not an array
    state->indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //T
    fp->write(" read() const { return m_stgRef; }\n");
  } //genUlamTypeElementReadDefinitionForC


  void UlamTypeClass::genUlamTypeElementWriteDefinitionForC(File * fp, CompilerState * state)
  {
    // arrays are handled separately
    assert(isScalar());

    // here, must be scalar; ref param to avoid excessive copying
    state->indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString(state).c_str()); //T
    fp->write("& v) { m_stgRef = v; }\n");
  } //genUlamTypeElementWriteDefinitionForC


  const std::string UlamTypeClass::readArrayItemMethodForCodeGen()
  {
    if(isCustomArray())
      return CUSTOMARRAY_GET_MANGLEDNAME;
    return UlamType::readArrayItemMethodForCodeGen();
  }


  const std::string UlamTypeClass::writeArrayItemMethodForCodeGen()
  {
    if(isCustomArray())
      return CUSTOMARRAY_SET_MANGLEDNAME;
    return UlamType::writeArrayItemMethodForCodeGen();
  }

  // a special struct for quarks (including zero-size quarks access
  // their methods) and elements that inherits from their immediates
  // and have a T ref into a "live" atom/element, and a magical
  // destructor that updates it; used with Conditional-As.
  void UlamTypeClass::genUlamTypeMangledAutoDefinitionForC(File * fp, CompilerState * state)
  {
    if(!isScalar())
      return;

    s32 len = getTotalBitSize();

    const std::string mangledName = getUlamTypeImmediateMangledName(state);
    const std::string automangledName = getUlamTypeImmediateAutoMangledName(state);

    std::ostringstream  ud;
    ud << "Ud_" << automangledName;  //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    state->m_currentIndentLevel = 0;

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
    fp->write(automangledName.c_str());
    fp->write(" : public ");
    fp->write(mangledName.c_str());
    fp->write("<CC>\n");
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
    fp->write("\n");

    //reference to storage in atom
    state->indent(fp);
    fp->write("T& m_stgToChange;  //ref to storage here!\n");

    if(m_class == UC_QUARK)
      {
	state->indent(fp);
	fp->write("const u32 m_pos;   //pos in atom\n");

	// constructor with args
	state->indent(fp);
	fp->write(automangledName.c_str());
	fp->write("(T & realStg, u32 idx) : m_stgToChange(realStg), m_pos(idx)\n");
	state->indent(fp);
	fp->write("{\n");

	//initialize immediate T in constuctor
	state->m_currentIndentLevel++;
	state->indent(fp);
	fp->write("const u32 val = realStg.GetBits().Read(m_pos + T::ATOM_FIRST_STATE_BIT, ");
	fp->write_decimal(len);
	fp->write(");\n");
	state->indent(fp);
	fp->write(mangledName.c_str());
	fp->write("<CC>::write(val);\n");
	state->m_currentIndentLevel--;
	state->indent(fp);
	fp->write("}\n");

	// magical destructor
	state->indent(fp);
	fp->write("~");
	fp->write(automangledName.c_str());
	fp->write("()\n");
	state->indent(fp);
	fp->write("{\n");

	state->m_currentIndentLevel++;
  	state->indent(fp);
	fp->write("m_stgToChange.GetBits().Write(m_pos + T::ATOM_FIRST_STATE_BIT, ");
	fp->write_decimal(len);
	fp->write(", ");
	fp->write(mangledName.c_str());
	fp->write("<CC>::read());\n");
	state->m_currentIndentLevel--;
  	state->indent(fp);
	fp->write("}\n");
      }
    else if(m_class == UC_ELEMENT)
      {
	// constructor with args
	state->indent(fp);
	fp->write(automangledName.c_str());
	fp->write("(T & realStg) : ");
	fp->write(mangledName.c_str());
	fp->write("<CC>(realStg), m_stgToChange(realStg) {}\n");

	// magical destructor
	state->indent(fp);
	fp->write("~");
	fp->write(automangledName.c_str());
	fp->write("() { m_stgToChange = ");
	fp->write(mangledName.c_str());
	fp->write("<CC>::m_stg; }\n");
      }
    else
      assert(0);

#if 0
    // non-const T ref method for scalar
    state->indent(fp);
    fp->write("T& getRef() { return ");
    fp->write(mangledName.c_str());
    fp->write("<CC>::getRef(); }\n");
#endif

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
  } //genUlamTypeMangledAutoDefinitionForC


} //end MFM
