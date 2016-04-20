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
    //setTotalWordSize(BITSPERATOM);
    //setItemWordSize(BITSPERATOM);
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
	//elements inherit from only quarks.
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
    //return UNPACKED; //was PACKED, now matches ATOM regardless of its bit size.
    return UlamType::getPackable(); //depends on PACKED size
  }

  const std::string UlamTypeClassElement::readMethodForCodeGen()
  {
    //return "ReadAtom";
    std::string method;
    u32 sizeByIntBits = getTotalWordSize();
    switch(sizeByIntBits)
      {
      case 0: //e.g. empty elements
      case 32:
	method = "Read";
	break;
      case 64:
	method = "ReadLong";
	break;
      default:
	{
	  if(isScalar())
	    method = "ReadBig";
	  else
	    {
	      std::ostringstream mstr;
	      mstr << "ReadBV<" << getTotalBitSize() << ">";
	      method = mstr.str();
	    }
	}
      };
    return method;
  } //readMethodForCodeGen

  const std::string UlamTypeClassElement::writeMethodForCodeGen()
  {
    //return "WriteAtom";
    std::string method;
    u32 sizeByIntBits = getTotalWordSize();
    switch(sizeByIntBits)
      {
      case 0: //e.g. empty elements
      case 32:
	method = "Write";
	break;
      case 64:
	method = "WriteLong";
	break;
      default:
	{
	  if(isScalar())
	    method = "WriteBig";
	  else
	    {
	      std::ostringstream mstr;
	      mstr << "WriteBV<" << getTotalBitSize() << ">";
	      method = mstr.str();
	    }
	}
      };
    return method;
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

  const std::string UlamTypeClassElement::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName();
    ctype << "<EC>";
    return ctype.str();
  } //getLocalStorageTypeAsString

  STORAGE UlamTypeClassElement::getTmpStorageTypeForTmpVar()
  {
    //return TMPTATOM;
    u32 sizebyints = getTotalWordSize();
    STORAGE rtnStgType = TMPTBV; //?
    std::string ctype;
    switch(sizebyints)
      {
      case 0: //e.g. empty elements
      case 32:
	rtnStgType = TMPREGISTER;
	break;
      case 64:
	rtnStgType = TMPREGISTER;
	break;
      case 96:
      default:
	{
	  rtnStgType = TMPBITVAL;
	}
      };
    return rtnStgType;
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
    // element size is PACKED now! unless atom is asked for.
    s32 len = getTotalBitSize(); //could be 0, includes arrays
    //s32 bitsize = getBitSize();

    //regardless of actual element size, it takes up the full atom space
    //if(!isScalar())
    //  return genUlamTypeMangledUnpackedArrayAutoDefinitionForC(fp);

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
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, targ, effself) { }\n");

    //constructor for conditional-as (auto)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(AtomBitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself) : UlamRef<EC>");
    fp->write("(idx + T::ATOM_FIRST_STATE_BIT, "); //the real pos!!!
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, targ, effself) { }\n");

    //copy constructor here
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r) : UlamRef<EC>(r, 0u, r.GetLen(), r.GetEffectiveSelf()) { }\n");

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, u32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, effself) {}\n");

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
    // arrays are handled separately
    //assert(isScalar());
    //scalar and entire PACKEDLOADABLE or UNPACKED array handled by base class read method
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

    if(isScalar() || WritePacked(getPackable()))
      {
	// write must be scalar; ref param to avoid excessive copying
	//not an array
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write(" read() { ");
	fp->write("return ");
	fp->write("UlamRef<EC>::");
	fp->write(readMethodForCodeGen().c_str()); //just the guts
	fp->write("(); /* entire element */ }\n");
      }
  } //genUlamTypeAutoReadDefinitionForC

  void UlamTypeClassElement::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    // arrays are handled separately
    //assert(isScalar());
    //scalar and entire PACKEDLOADABLE array handled by base class write method
    if(!isScalar())
      {
	// writes an item of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32 or BV96
	fp->write("& v, const u32 index, const u32 itemlen) { ");
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //rel offset
	fp->write("itemlen, &");  //itemlen,
	fp->write("Us::THE_INSTANCE)."); //effself
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(v); }\n");
      }

    if(isScalar() || WritePacked(getPackable()))
      {
	// write must be scalar; ref param to avoid excessive copying
	//not an array
	m_state.indent(fp);
	fp->write("void");
	fp->write(" write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
	fp->write("& targ) { UlamRef<EC>::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(targ); /* entire element */ }\n");

	//m_state.indent(fp);
	//fp->write("void writeTypeField(const u32 v)");
	//fp->write("{ UlamRefAtom<EC>::SetType(v); }\n"); //GONE???
      }
  } //genUlamTypeAutoWriteDefinitionForC

  //builds the immediate definition, inheriting from auto
  void UlamTypeClassElement::genUlamTypeMangledDefinitionForC(File * fp)
  {
    //packed elements!
    s32 len = getTotalBitSize(); //could be 0, includes arrays
    u32 bitsize = getBitSize();

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
    //fp->write("AtomBitStorage<EC>\n"); //storage here!
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

    //read/write methods before constructors that may use them
    //read 'entire element' method
    genUlamTypeReadDefinitionForC(fp);

    //write 'entire element' method
    genUlamTypeWriteDefinitionForC(fp);

    //default constructor (used by local vars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { ");

    fp->write("AtomBitStorage<EC> tmp(");
    fp->write("Us::THE_INSTANCE");
    fp->write(".GetDefaultAtom()); "); //returns object of type T

    if(isScalar())
      {
	fp->write("BVS::WriteBig");
	fp->write("(0u, ");
	fp->write_decimal_unsigned(len);
	fp->write("u, tmp.ReadBig(0u + T::ATOM_FIRST_STATE_BIT, ");
	fp->write_decimal_unsigned(len);
	fp->write("u));");
      }
    else
      {
	fp->write("BV96 tmpval = tmp.");
	fp->write("ReadBig");
	fp->write("(0u + T::ATOM_FIRST_STATE_BIT, ");
	fp->write_decimal_unsigned(bitsize);
	fp->write("); ");
	fp->write("u32 n = ");
	fp->write_decimal_unsigned(getArraySize());
	fp->write("u; while(n--) { ");
	fp->write("BVS::WriteBig(n * ");
	fp->write_decimal_unsigned(bitsize);
	fp->write(", ");
	fp->write_decimal_unsigned(bitsize);
	fp->write(", tmpval); }");
      }
    fp->write(" }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write("T& d) { ");
    fp->write("AtomBitStorage<EC> tmp(d); ");
    if(isScalar())
      {
	fp->write("BVS::WriteBig");
	fp->write("(0u, ");
	fp->write_decimal_unsigned(len);
	fp->write("u, tmp.ReadBig(0u + T::ATOM_FIRST_STATE_BIT, ");
	fp->write_decimal_unsigned(len);
	fp->write("u));");
      }
    else
      {
	//(t3670)
	fp->write("BV96 tmpval = tmp.ReadBig(0u + T::ATOM_FIRST_STATE_BIT, ");
	fp->write_decimal_unsigned(bitsize);
	fp->write("u); ");
	fp->write("u32 n = ");
	fp->write_decimal(getArraySize());
	fp->write("u; while(n--) { ");
	fp->write("BVS::WriteBig(n * ");
	fp->write_decimal_unsigned(bitsize);
	fp->write(", ");
	fp->write_decimal_unsigned(bitsize);
	fp->write(", tmpval); }");
      }
    fp->write(" }\n");

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(scalarut->getTmpStorageTypeAsString().c_str()); //u64, u32, BV96
    fp->write("& d) { ");
    if(isScalar())
      {
	fp->write("write(d);");
      }
    else
      {
	fp->write("u32 n = ");
	fp->write_decimal(getArraySize());
	fp->write("u; while(n--) { ");
	fp->write("writeArrayItem(d, n, ");
	fp->write_decimal_unsigned(bitsize);
	fp->write("); }");
      }
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
    fp->write("(const AtomBitStorage<EC> & arg) { ");
    fp->write("if(arg.GetType() != Us::THE_INSTANCE.GetType()) FAIL(ILLEGAL_ARGUMENT); BVS::WriteBig(0u, ");
    fp->write_decimal_unsigned(bitsize);
    fp->write("u, arg.ReadBig(0u + T::ATOM_FIRST_STATE_BIT, ");
    fp->write_decimal_unsigned(bitsize);
    fp->write("u)); }\n");

    //create a T, from immediate element
    if(isScalar())
      {
	m_state.indent(fp);
	fp->write("T CreateAtom() const {");
	fp->write("AtomBitStorage<EC> tmp(");
	fp->write("Us::THE_INSTANCE");
	fp->write(".GetDefaultAtom()); "); //returns object of type T

	fp->write("tmp.WriteBig(0u + T::ATOM_FIRST_STATE_BIT, ");
	fp->write_decimal_unsigned(len);
	fp->write(", ");
	fp->write("BVS::ReadBig(0u, ");
	fp->write_decimal_unsigned(len);
	fp->write("u));");
	fp->write(" return tmp.ReadAtom(); }\n");
      }

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

    if(isScalar() || WritePacked(getPackable()))
      {
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32 or u64 or BV96
	fp->write(" read() const { ");
	fp->write("return BVS::");
	fp->write(readMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal_unsigned(getTotalBitSize());
	if(isScalar())
	  fp->write("u); }\n"); //done
	else
	  fp->write("u); } //reads entire array\n");
      }

    if(!isScalar())
      {
	//array item reads; entire PACKEDLOADABLE array read handled by Read()
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //T, u64, or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return BVS::"); //was const after )
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, "); //const ref, rel offset
	fp->write("itemlen); }\n");  //itemlen,
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypeClassElement::genUlamTypeWriteDefinitionForC(File * fp)
  {
    //ref param to avoid excessive copying
    if(isScalar() || (getPackable() == PACKEDLOADABLE))
      {
	m_state.indent(fp);
	fp->write("void write(const ");
	fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, BV96
	fp->write("& v) { ");
	fp->write("BVS::");
	fp->write(writeMethodForCodeGen().c_str());
	fp->write("(0u, ");
	fp->write_decimal_unsigned(getTotalBitSize());
	if(isScalar())
	  fp->write("u, v); }\n"); //done
	else
	  fp->write("u, v); } //writes entire array\n");
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
	fp->write("(index * itemlen, "); //rel offset
	fp->write("itemlen, v); }\n");  //itemlen, val
      }
  } //genUlamTypeWriteDefinitionForC

  //deprecated with packed elements
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

#if 0
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
#endif

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

  //deprecated with packed elements
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
    fp->write("T foo = ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC>::THE_INSTANCE.GetDefaultAtom(); ");
    fp->write("for(u32 j = 0; j < ");
    fp->write_decimal_unsigned(arraysize);
    fp->write("u; j++) ");
    fp->write("writeArrayItem(");
    fp->write("foo, j, BPA); }\n");

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

#if 0
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
#endif

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
