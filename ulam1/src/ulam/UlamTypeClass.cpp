#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "UlamTypeClass.h"
#include "UlamValue.h"
#include "CompilerState.h"
#include "Util.h"

namespace MFM {

  UlamTypeClass::UlamTypeClass(const UlamKeyTypeSignature key, CompilerState & state, ULAMCLASSTYPE type) : UlamType(key, state), m_class(type), m_customArray(false) /* , m_customArrayType(Nav)*/
  {
    m_wordLengthTotal = calcWordSize(getTotalBitSize());
    m_wordLengthItem = calcWordSize(getBitSize());
  }

   ULAMTYPE UlamTypeClass::getUlamTypeEnum()
   {
     return Class;
   }

  bool UlamTypeClass::isNumericType()
  {
    if(m_class == UC_QUARK)
      {
	u32 quti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	return (m_state.quarkHasAToIntMethod(quti));
      }
    return false;
  } //isNumericType

  bool UlamTypeClass::isPrimitiveType()
  {
    return isNumericType();
  }

  bool UlamTypeClass::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this);
    UTI valtypidx = val.getUlamValueTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(valtypidx);
    assert(vut->isScalar() && isScalar());

    //now allowing atoms to be cast as quarks, as well as elements;
    if(m_class == UC_ELEMENT)
      assert(valtypidx == UAtom || UlamType::compare(valtypidx, typidx, m_state) == UTIC_SAME);
    else if(m_class == UC_QUARK)
      {
	ULAMCLASSTYPE vclasstype = vut->getUlamClass();
	assert(valtypidx == UAtom || vclasstype == UC_ELEMENT || UlamType::compare(valtypidx, typidx, m_state) == UTIC_SAME);
	if(vclasstype == UC_ELEMENT)
	  {
	    SymbolClass * csym = NULL;
	    assert(m_state.alreadyDefinedSymbolClass(valtypidx, csym));
	    NodeBlockClass * cblock = csym->getClassBlockNode();
	    assert(cblock);
	    s32 pos = cblock->findUlamTypeInTable(typidx);
	    if(pos >= 0)
	      {
		s32 len = getTotalBitSize();
		assert(len != UNKNOWNSIZE);
		if(len <= MAXBITSPERINT)
		  {
		    u32 qdata = val.getDataFromAtom(pos, len);
		    val = UlamValue::makeImmediate(typidx, qdata, len);
		  }
		else if(len <= MAXBITSPERLONG)
		  {
		    u64 qdata = val.getDataLongFromAtom(pos, len);
		    val = UlamValue::makeImmediateLong(typidx, qdata, len);
		  }
		else
		  assert(0);
	      }
	    else
	      assert(0);
	  }
	//if same type nothing to do; if atom, shows as element in eval-land.
      }
    else
      assert(0);
    // what change is to be made ????
    // atom type vs. class type
    // how can it be both in an UlamValue?
    // what of its contents?
    // val = UlamValue::makeAtom(valtypidx);

    return brtn;
  } //end cast

  FORECAST UlamTypeClass::safeCast(UTI typidx)
  {
    FORECAST scr = UlamType::safeCast(typidx);
    if(scr != CAST_CLEAR)
      return scr;

    if(m_state.getUlamTypeByIndex(typidx) == this)
      return CAST_CLEAR; //same class, quark or element

    return CAST_BAD; //e.g. (typidx == UAtom)
  } //safeCast

  const char * UlamTypeClass::getUlamTypeAsSingleLowercaseLetter()
  {
    switch(m_class)
      {
      case UC_ELEMENT:
	return "e";
      case UC_QUARK:
	return "q";
      case UC_UNSEEN:
      default:
	assert(0);
      };
    return UlamType::getUlamTypeEnumCodeChar(getUlamTypeEnum());
  } //getUlamTypeAsSingleLowercaseLetter()

  const std::string UlamTypeClass::getUlamTypeVDAsStringForC()
  {
    return "VD::BITS"; //for quark use bits
  }

  const std::string UlamTypeClass::getUlamTypeMangledType()
  {
    // e.g. parsing overloaded functions, may not be complete.
    std::ostringstream mangled;
    s32 bitsize = getBitSize();
    s32 arraysize = getArraySize();

    if(arraysize > 0)
      mangled << ToLeximitedNumber(arraysize);
    else
      mangled << 10;

    if(bitsize > 0)
      mangled << ToLeximitedNumber(bitsize);
    else
      mangled << 10;

    mangled << m_state.getDataAsStringMangled(m_key.getUlamKeyTypeSignatureNameId()).c_str();
    //without types and values of args!!
    return mangled.str();
  } //getUlamTypeMangledType

  const std::string UlamTypeClass::getUlamTypeMangledName()
  {
    std::ostringstream mangledclassname;
    mangledclassname << UlamType::getUlamTypeMangledName(); //includes Uprefix

    //appends '10', or numberOfParameters followed by each digi-encoded: mangled type and value
    u32 id = m_key.getUlamKeyTypeSignatureNameId();
    UTI cuti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(id);
    mangledclassname << cnsym->formatAnInstancesArgValuesAsAString(cuti);
    return mangledclassname.str();
  } //getUlamTypeMangledName

  //quarks are right-justified in an atom space
  const std::string UlamTypeClass::getUlamTypeAsStringForC()
  {
    assert(m_class != UC_UNSEEN);
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
      case UC_UNSEEN:
	return "U?_";
     default:
	assert(0);
      };
    return "xx";
  } //getUlamTypeUPrefix

  const std::string UlamTypeClass::getUlamTypeNameBrief()
  {
    std::ostringstream namestr;
    namestr << m_key.getUlamKeyTypeSignatureName(&m_state).c_str();

    u32 id = m_key.getUlamKeyTypeSignatureNameId();
    u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(id);
    if(cnsym && cnsym->isClassTemplate())
      namestr << ((SymbolClassNameTemplate *) cnsym)->formatAnInstancesArgValuesAsCommaDelimitedString(cuti).c_str();
    return namestr.str();
  } //getUlamTypeNameBrief

  //see SymbolVariableDataMember printPostfix for recursive output
  void UlamTypeClass::getDataAsString(const u32 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%d", data);
    else
      sprintf(valstr,"%c%d", prefix, data);
  }

  void UlamTypeClass::getDataLongAsString(const u64 data, char * valstr, char prefix)
  {
    if(prefix == 'z')
      sprintf(valstr,"%s", ToSignedDecimal(data).c_str());
    else
      sprintf(valstr,"%c%s", prefix, ToSignedDecimal(data).c_str());
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

  bool UlamTypeClass::isScalar()
  {
    return (m_key.getUlamKeyTypeSignatureArraySize() == NONARRAYSIZE);
  }

  bool UlamTypeClass::isCustomArray()
  {
    return m_customArray; //canonical
  }

  void UlamTypeClass::setCustomArray()
  {
    m_customArray = true;
  }

  UTI UlamTypeClass::getCustomArrayType()
  {
    UTI caType = Nav;
    assert(m_customArray);
    u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    SymbolClass * csym = NULL;

    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      {
	caType = csym->getCustomArrayType();
      }
    return caType;
  } //getCustomArrayType

  u32 UlamTypeClass::getCustomArrayIndexTypeFor(Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    u32 camatches = 0;
    assert(m_customArray);
    u32 cuti = m_key.getUlamKeyTypeSignatureClassInstanceIdx();
    SymbolClass * csym = NULL;

    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      {
	camatches = csym->getCustomArrayIndexTypeFor(rnode, idxuti, hasHazyArgs);
      }
    return camatches;
  } //getCustomArrayIndexTypeFor

  s32 UlamTypeClass::getBitSize()
  {
    s32 bitsize = m_key.getUlamKeyTypeSignatureBitSize();
    return bitsize; //could be negative "unknown"; allow for empty quarks
  }

  bool UlamTypeClass::isHolder()
  {
    std::string name = m_key.getUlamKeyTypeSignatureName(&m_state);
    s32 c = name.at(0);
    return (!isalpha(c)); //id same as UTI number, out-of-band
  }

  bool UlamTypeClass::isComplete()
  {
    if(isHolder())
      return false;

    return UlamType::isComplete();
  }

  bool UlamTypeClass::isMinMaxAllowed()
  {
    return false;
  }

  PACKFIT UlamTypeClass::getPackable()
  {
    if(m_class == UC_ELEMENT)
      return UNPACKED; //was PACKED, now matches ATOM regardless of its bit size.
    return UlamType::getPackable(); //quarks depend their size
  }

  bool UlamTypeClass::needsImmediateType()
  {
    if(!isComplete())
      return false;

    bool rtnb = false;
    if(m_class == UC_QUARK || m_class == UC_ELEMENT)
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

  const std::string UlamTypeClass::getUlamTypeImmediateMangledName()
  {
    if(needsImmediateType())
      {
	return UlamType::getUlamTypeImmediateMangledName();
      }
    return "T";
  } //getUlamTypeImmediateMangledName

  const std::string UlamTypeClass::getUlamTypeImmediateAutoMangledName()
  {
    assert(needsImmediateType());
    std::ostringstream  automn;
    automn << getUlamTypeImmediateMangledName().c_str();

    if(m_class == UC_QUARK)
      automn << "4auto" ;

    return automn.str();
  } //getUlamTypeImmediateAutoMangledName

  const std::string UlamTypeClass::getTmpStorageTypeAsString()
  {
    if(m_class == UC_QUARK)
      {
	return UlamType::getTmpStorageTypeAsString(); // entire, u32 or u64
      }
    else if(m_class == UC_ELEMENT)
      {
 	return "T";
      }
    else
      assert(0);
    return ""; //error!
  } //getTmpStorageTypeAsString

  const std::string UlamTypeClass::getArrayItemTmpStorageTypeAsString()
  {
    if(!isScalar())
      return UlamType::getArrayItemTmpStorageTypeAsString();

    assert(isCustomArray());
    return m_state.getUlamTypeByIndex(getCustomArrayType())->getTmpStorageTypeAsString();
  } //getArrayItemTmpStorageTypeAsString

  const std::string UlamTypeClass::getImmediateStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName();

    if(m_class == UC_QUARK && isScalar())
      ctype << "<EC>"; // not ,POS> because immediates know their position

    if(m_class == UC_ELEMENT)
      ctype << "<EC>";

    return ctype.str();
  } //getImmediateStorageTypeAsString

  void UlamTypeClass::genUlamTypeReadDefinitionForC(File * fp)
  {
    if(m_class == UC_QUARK)
      return genUlamTypeQuarkReadDefinitionForC(fp);
    else if(m_class == UC_ELEMENT)
      return genUlamTypeElementReadDefinitionForC(fp);
    else
      assert(0);
  } //genUlamTypeReadDefinitionForC

  void UlamTypeClass::genUlamTypeWriteDefinitionForC(File * fp)
  {
    if(m_class == UC_QUARK)
      return genUlamTypeQuarkWriteDefinitionForC(fp);
    else if(m_class == UC_ELEMENT)
      return genUlamTypeElementWriteDefinitionForC(fp);
    else
      assert(0);
  } //genUlamTypeWriteDefinitionForC

  const std::string UlamTypeClass::castMethodForCodeGen(UTI nodetype)
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

    if(m_class != UC_ELEMENT)
      {
	std::ostringstream msg;
	msg << "Quarks only cast 'toInt': value type and size was: ";
	msg << nut->getUlamTypeName().c_str();
	msg << ", to be: " << getUlamTypeName().c_str();
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(),msg.str().c_str(), ERR);
      }

    //e.g. casting an element to an element, redundant and not supported: Element96ToElement96?
    if(nodetype != UAtom)
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

  void UlamTypeClass::genUlamTypeMangledDefinitionForC(File * fp)
  {
    if(m_class == UC_QUARK)
      return genUlamTypeQuarkMangledDefinitionForC(fp);
    else if(m_class == UC_ELEMENT)
      return genUlamTypeElementMangledDefinitionForC(fp);
    else
      assert(0);
  } //genUlamTypeMangledDefinitionForC

  void UlamTypeClass::genUlamTypeQuarkMangledDefinitionForC(File * fp)
  {
    assert(m_class == UC_QUARK);

    if(!isScalar())
      {
	// immediate quark arrays are treated like immediate primitives, not quarks
	  return UlamType::genUlamTypeMangledDefinitionForC(fp);
      }

    //is scalar here..
    m_state.m_currentIndentLevel = 0;
    const std::string mangledName = getUlamTypeImmediateMangledName();
    std::ostringstream  ud;
    ud << "Ud_" << mangledName; //d for define (p used for atomicparametrictype)
    std::string udstr = ud.str();

    s32 len = getTotalBitSize();

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
    fp->write("template<class EC, u32 POS> class ");
    fp->write(getUlamTypeMangledName().c_str());
    fp->write(";  //forward\n\n");

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
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

    //quark typedef (right-justified)
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeMangledName().c_str());
    fp->write("<EC, ");
    fp->write_decimal(BITSPERATOM - len);
    fp->write("> Us;\n");

    m_state.indent(fp);
    fp->write("typedef AtomicParameterType");
    fp->write("<EC"); //BITSPERATOM
    fp->write(", ");
    fp->write(getUlamTypeVDAsStringForC().c_str());
    fp->write(", ");
    fp->write_decimal(len);   //include arraysize
    fp->write(", ");
    fp->write_decimal(BITSPERATOM - len); //right-justified
    fp->write("> ");
    fp->write(" Up_Us;\n");

    u32 dqval = 0;
    bool hasDQ = genUlamTypeDefaultQuarkConstant(fp, dqval);

    //storage here in atom
    m_state.indent(fp);
    fp->write("T m_stg;  //storage here!\n");

    //default constructor (used by local vars)
    //(unlike element) call build default in case of initialized data members
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stg( ) { ");

    if(hasDQ)
      fp->write("write(DEFAULT_QUARK); }\n");
    else
      fp->write(" }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" d) { write(d); }\n");

    //read 'entire quark' method
    genUlamTypeQuarkReadDefinitionForC(fp);

    //write 'entire quark' method
    genUlamTypeQuarkWriteDefinitionForC(fp);

    // getBits method for scalar
    m_state.indent(fp);
    fp->write("BitVector<BPA>& getBits() { return m_stg.GetBits(); }\n");

    // non-const T ref method for scalar
    m_state.indent(fp);
    fp->write("T& getRef() { return m_stg; }\n");

    // aref/aset calls generated inline for immediates.
    if(isCustomArray())
      {
	m_state.indent(fp);
	fp->write("/* a custom array, btw ('Us' has aref, aset methods) */\n");
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
  } //genUlamTypeQuarkMangledDefinitionForC

  void UlamTypeClass::genUlamTypeQuarkReadDefinitionForC(File * fp)
  {
    // arrays are handled separately, not like a quark, more like a primitive
    if(!isScalar())
      return UlamType::genUlamTypeReadDefinitionForC(fp);

    //not an array
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" read() const { return Up_Us::");
    fp->write(readMethodForCodeGen().c_str());
    fp->write("(m_stg.GetBits() ); }\n");
  } //genUlamTypeQuarkReadDefinitionForC

  void UlamTypeClass::genUlamTypeQuarkWriteDefinitionForC(File * fp)
  {
    // arrays are handled separately, not like a quark, more like a primitive
    if(!isScalar())
      return UlamType::genUlamTypeWriteDefinitionForC(fp);

    // here, must be scalar
    m_state.indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //s32 or u32
    fp->write(" v) { Up_Us::");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(m_stg.GetBits(), v); }\n");
  } //genUlamTypeQuarkWriteDefinitionForC

  //whole element (based on atom)
  void UlamTypeClass::genUlamTypeElementMangledDefinitionForC(File * fp)
  {
    assert(isScalar());

    m_state.m_currentIndentLevel = 0;
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

    //forward declaration of element (before struct!)
    m_state.indent(fp);
    fp->write("template<class EC> class ");
    fp->write(getUlamTypeMangledName().c_str());
    fp->write(";  //forward\n\n");

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(mangledName.c_str());
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

    //element typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(getUlamTypeMangledName().c_str());
    fp->write("<EC> Us;\n");

    //storage here in atom
    m_state.indent(fp);
    fp->write("T m_stg;  //possible storage here\n");

    // ref to storage in atom
    m_state.indent(fp);
    fp->write("T& m_stgRef;  //storage ref\n");

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() : m_stg(), m_stgRef(m_stg) { }\n");

    //constructor here (used by const tmpVars); ref param to avoid excessive copying
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write("& d) : m_stg(d), m_stgRef(m_stg) {}\n");

    if(m_class == UC_ELEMENT)
      {
	//constructor here (used by As for elements);
	// ref points directly into an atom; not so for quark
	// since pos is required for template (uses auto).
	m_state.indent(fp);
	fp->write(mangledName.c_str());
	fp->write("(");   //not const
	fp->write(getTmpStorageTypeAsString().c_str()); //T&
	fp->write("& d, bool badass) : m_stg(), m_stgRef(d) {}\n");
      }

    //copy constructor here (used by func call return values)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getImmediateStorageTypeAsString().c_str());
    fp->write("& d) : m_stg(d.m_stg), m_stgRef(m_stg) {}\n");

    //default destructor (for completeness)
    m_state.indent(fp);
    fp->write("~");
    fp->write(mangledName.c_str());
    fp->write("() {}\n");

    //read 'entire atom' method
    genUlamTypeElementReadDefinitionForC(fp);

    //write 'entire atom' method
    genUlamTypeElementWriteDefinitionForC(fp);

    // getBits method for scalar
    m_state.indent(fp);
    fp->write("BitVector<BPA>& getBits() { return m_stgRef.GetBits(); }\n");

    // non-const T ref method for scalar
    m_state.indent(fp);
    fp->write("T& getRef() { return m_stgRef; }\n");

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
  } //genUlamTypeElementMangledDefinitionForC

  void UlamTypeClass::genUlamTypeElementReadDefinitionForC(File * fp)
  {
    // arrays are handled separately
    assert(isScalar());

    //not an array
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write(" read() const { return m_stgRef; }\n");
  } //genUlamTypeElementReadDefinitionForC

  void UlamTypeClass::genUlamTypeElementWriteDefinitionForC(File * fp)
  {
    // arrays are handled separately
    assert(isScalar());

    // here, must be scalar; ref param to avoid excessive copying
    m_state.indent(fp);
    fp->write("void write(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //T
    fp->write("& v) { m_stgRef = v; }\n");
  } //genUlamTypeElementWriteDefinitionForC

  const std::string UlamTypeClass::readArrayItemMethodForCodeGen()
  {
    if(isCustomArray())
      return m_state.getCustomArrayGetMangledFunctionName();
    return UlamType::readArrayItemMethodForCodeGen();
  }

  const std::string UlamTypeClass::writeArrayItemMethodForCodeGen()
  {
    if(isCustomArray())
      return m_state.getCustomArraySetMangledFunctionName();
    return UlamType::writeArrayItemMethodForCodeGen();
  }

  // a special struct for quarks (including zero-size quarks access
  // their methods) and elements that inherits from their immediates
  // and have a T ref into a "live" atom/element, and a magical
  // destructor that updates it; used with Conditional-As.
  void UlamTypeClass::genUlamTypeMangledAutoDefinitionForC(File * fp)
  {
    if(!isScalar())
      return;

    s32 len = getTotalBitSize();

    const std::string mangledName = getUlamTypeImmediateMangledName();
    const std::string automangledName = getUlamTypeImmediateAutoMangledName();

    std::ostringstream  ud;
    ud << "Ud_" << automangledName; //d for define (p used for atomicparametrictype)
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
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("struct ");
    fp->write(automangledName.c_str());
    fp->write(" : public ");
    fp->write(mangledName.c_str());
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

    //reference to storage in atom
    m_state.indent(fp);
    fp->write("T& m_stgToChange;  //ref to storage here!\n");

    if(m_class == UC_QUARK)
      {
	m_state.indent(fp);
	fp->write("const u32 m_pos;   //pos in atom\n");

	// constructor with args
	m_state.indent(fp);
	fp->write(automangledName.c_str());
	fp->write("(T & realStg, u32 idx) : m_stgToChange(realStg), m_pos(idx)\n");
	m_state.indent(fp);
	fp->write("{\n");

	//initialize immediate T in constuctor
	m_state.m_currentIndentLevel++;
	m_state.indent(fp);
	fp->write("const u32 val = realStg.GetBits().Read(m_pos + T::ATOM_FIRST_STATE_BIT, ");
	fp->write_decimal(len);
	fp->write(");\n");
	m_state.indent(fp);
	fp->write(mangledName.c_str());
	fp->write("<EC>::write(val);\n");
	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("}\n");

	// magical destructor
	m_state.indent(fp);
	fp->write("~");
	fp->write(automangledName.c_str());
	fp->write("()\n");
	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;
  	m_state.indent(fp);
	fp->write("m_stgToChange.GetBits().Write(m_pos + T::ATOM_FIRST_STATE_BIT, ");
	fp->write_decimal(len);
	fp->write(", ");
	fp->write(mangledName.c_str());
	fp->write("<EC>::read());\n");
	m_state.m_currentIndentLevel--;
  	m_state.indent(fp);
	fp->write("}\n");
      }
    else if(m_class == UC_ELEMENT)
      {
	// constructor with args
	m_state.indent(fp);
	fp->write(automangledName.c_str());
	fp->write("(T & realStg) : ");
	fp->write(mangledName.c_str());
	fp->write("<EC>(realStg), m_stgToChange(realStg) {}\n");

	// magical destructor
	m_state.indent(fp);
	fp->write("~");
	fp->write(automangledName.c_str());
	fp->write("() { m_stgToChange = ");
	fp->write(mangledName.c_str());
	fp->write("<EC>::m_stg; }\n");
      }
    else
      assert(0);

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

  void UlamTypeClass::genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp)
  {
    assert(0); //only primitive types
  }

  bool UlamTypeClass::genUlamTypeDefaultQuarkConstant(File * fp, u32& dqref)
  {
    bool rtnb = false;
    //if(m_class == UC_QUARK && getBitSize() > 0)
    if(m_class == UC_QUARK)
      {
	//if(m_state.getDefaultQuark(m_key.getUlamKeyTypeSignatureClassInstanceIdx(), dqref) && dqref > 0)
	if(m_state.getDefaultQuark(m_key.getUlamKeyTypeSignatureClassInstanceIdx(), dqref))
	  {
	    m_state.indent(fp);
	    fp->write("static const u32 DEFAULT_QUARK = ");
	    fp->write_decimal_unsigned(dqref);
	    fp->write(";\n\n");
	    rtnb = true;
	  }
      }
    return rtnb;
  } //genUlamTypeDefaultQuarkConstant

} //end MFM
