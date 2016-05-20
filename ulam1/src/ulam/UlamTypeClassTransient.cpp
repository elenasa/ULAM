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
  } //isNumericType

  ULAMCLASSTYPE UlamTypeClassTransient::getUlamClassType()
  {
    return UC_TRANSIENT;
  }

  bool UlamTypeClassTransient::cast(UlamValue & val, UTI typidx)
  {
    bool brtn = true;
    assert(m_state.getUlamTypeByIndex(typidx) == this); //tobe
    UTI valtypidx = val.getUlamValueTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(valtypidx);
    assert(vut->isScalar() && isScalar());
    //now allowing atoms to be cast as transients, as well as elements;
    // also allowing subclasses to be cast as their superclass (u1.2.2)
    if(!(UlamType::compare(valtypidx, typidx, m_state) == UTIC_SAME))
      {
	brtn = false;
      }
    return brtn;
  } //end cast

  const char * UlamTypeClassTransient::getUlamTypeAsSingleLowercaseLetter()
  {
    return "n";
  }

  const std::string UlamTypeClassTransient::getUlamTypeUPrefix()
  {
    if(getArraySize() > 0)
      return "Ut_";

    return "Un_";
  }

  const std::string UlamTypeClassTransient::readMethodForCodeGen()
  {
    return "ReadBV"; //UlamType::readMethodForCodeGen();
  }

  const std::string UlamTypeClassTransient::writeMethodForCodeGen()
  {
    return "WriteBV"; //return UlamType::writeMethodForCodeGen();
  }

  bool UlamTypeClassTransient::needsImmediateType()
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

  const std::string UlamTypeClassTransient::getTmpStorageTypeAsString()
  {
    std::ostringstream stype;
    stype << "BitVector<" << getTotalBitSize() << ">";

    return stype.str();
  } //getTmpStorageTypeAsString

  const std::string UlamTypeClassTransient::getLocalStorageTypeAsString()
  {
    std::ostringstream ctype;
    ctype << getUlamTypeImmediateMangledName();
    ctype << "<EC>";
    return ctype.str();
  } //getLocalStorageTypeAsString

  TMPSTORAGE UlamTypeClassTransient::getTmpStorageTypeForTmpVar()
  {
    TMPSTORAGE rtnStgType = TMPTBV;
    u32 sizebyints = getTotalWordSize();
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
	rtnStgType = TMPBITVAL;
      };
    return rtnStgType;
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

    //forward declaration of class (before struct!)
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

    //class typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;\n");

    m_state.indent(fp);
    fp->write("typedef UlamRef"); //was atomicparametertype
    fp->write("<EC> Up_Us;\n");

    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself) : UlamRef<EC>");
    fp->write("(idx, "); //the real pos!!!
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, targ, effself) { }\n");

    //constructor for chain of autorefs (e.g. memberselect with array item)
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, ");
    fp->write_decimal_unsigned(len); //includes arraysize
    fp->write("u, effself) { }\n");

    //copy constructor here; pos relative to exisiting (i.e. same).
    m_state.indent(fp);
    fp->write(automangledName.c_str());
    fp->write("(const ");
    fp->write(automangledName.c_str());
    fp->write("<EC>& r) : UlamRef<EC>(r, 0, r.GetLen(), r.GetEffectiveSelf()) { }\n");

    // aref/aset calls generated inline for immediates.
    if(isCustomArray())
      {
	m_state.indent(fp);
	fp->write("/* a custom array, btw ('Us' has aref, aset methods) */\n");
      }

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
  } //genUlamTypeMangledAutoDefinitionForC

  void UlamTypeClassTransient::genUlamTypeAutoReadDefinitionForC(File * fp)
  {
    m_state.indent(fp);
    fp->write("const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
    fp->write(" read() const { ");
    fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
    fp->write(" tmpbv; this->GetStorage().");
    fp->write(readMethodForCodeGen().c_str()); //just the guts
    fp->write("(this->GetPos(), tmpbv); ");
    fp->write("return tmpbv; /* entire transient */ }\n");

    //scalar and entire PACKEDLOADABLE array handled by read method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	//reads an item of array
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return "); //was const after )
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //const ref, rel offset
	fp->write("itemlen, &");  //itemlen,
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(scalaruti).c_str());
	fp->write(").");
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(); }\n");
      }
  } //genUlamTypeAutoReadDefinitionForC

  void UlamTypeClassTransient::genUlamTypeAutoWriteDefinitionForC(File * fp)
  {
    // write must be scalar; ref param to avoid excessive copying
    //not an array
    m_state.indent(fp);
    fp->write("void");
    fp->write(" write(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //u32, u64, or BV96
    fp->write("& targ) { this->GetStorage().");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(this->GetPos(), targ); /* entire transient */ }\n");

    //scalar and entire PACKEDLOADABLE array handled by write method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	// writes an item of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" v, const u32 index, const u32 itemlen) { ");
	fp->write("UlamRef<EC>(");
	fp->write("*this, index * itemlen, "); //rel offset
	fp->write("itemlen, &");  //itemlen,
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(scalaruti).c_str());
	fp->write(").");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(v); }\n");
      }
  } //genUlamTypeAutoWriteDefinitionForC

  void UlamTypeClassTransient::genUlamTypeMangledDefinitionForC(File * fp)
  {
    s32 len = getTotalBitSize(); //could be 0, includes arrays
    s32 bitsize = getBitSize();

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

    //class typedef
    m_state.indent(fp);
    fp->write("typedef ");
    fp->write(scalarmangledName.c_str());
    fp->write("<EC> Us;\n\n");

    //read/write methods before constructors in case used.
    genUlamTypeReadDefinitionForC(fp);

    genUlamTypeWriteDefinitionForC(fp);

    //default constructor uses the default value
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("() { ");
    if(isScalar())
      fp->write("Us::THE_INSTANCE.getDefaultTransient(0u, *this); }\n");
    else
      {
	BV8K dval, darrval;
	AssertBool isDefault = m_state.getDefaultClassValue(scalaruti, dval);
	m_state.getDefaultAsArray(bitsize, getArraySize(), 0u, dval, darrval);
	fp->write("\n");
	m_state.m_currentIndentLevel++;
	if(m_state.genCodeClassDefaultConstantArray(fp, len, darrval))
	  {
	    m_state.indent(fp);
	    fp->write("BVS::WriteBV(0u, "); //first arg
	    fp->write("initBV);\n");
	  }
	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write(" }\n");
      }

    //constructor here (used by const tmpVars)
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //BVn
    fp->write("& d) { ");
    fp->write("write(d); }\n");

    // assignment constructor
    m_state.indent(fp);
    fp->write(mangledName.c_str());
    fp->write("(const ");
    fp->write(mangledName.c_str());
    fp->write("<EC> & arg) { ");
    fp->write("write(arg.read()); }\n");

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

    //scalar and entire PACKEDLOADABLE array handled by read method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	//reads an item of array
	//2nd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write(" readArrayItem(");
	fp->write("const u32 index, const u32 itemlen) const { return BVS::"); //was const after )
	fp->write(readArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, "); //const ref, rel offset
	fp->write("itemlen); }\n");  //itemlen,
      }
  } //genUlamTypeReadDefinitionForC

  void UlamTypeClassTransient::genUlamTypeWriteDefinitionForC(File * fp)
  {
    m_state.indent(fp);
    fp->write("void ");
    fp->write(" write(const ");
    fp->write(getTmpStorageTypeAsString().c_str()); //BV
    fp->write("& bv) { BVS::");
    fp->write(writeMethodForCodeGen().c_str());
    fp->write("(0u, bv); ");
    fp->write("} //writes entire BV\n");

    //scalar and entire PACKEDLOADABLE array handled by write method
    if(!isScalar())
      {
	//class instance idx is always the scalar uti
	UTI scalaruti =  m_key.getUlamKeyTypeSignatureClassInstanceIdx();
	const std::string scalarmangledName = m_state.getUlamTypeByIndex(scalaruti)->getUlamTypeMangledName();
	// writes an item of array
	//3rd argument generated for compatibility with underlying method
	m_state.indent(fp);
	fp->write("void writeArrayItem(const ");
	fp->write(getArrayItemTmpStorageTypeAsString().c_str()); //s32 or u32
	fp->write("& v, const u32 index, const u32 itemlen) { BVS::");
	fp->write(writeArrayItemMethodForCodeGen().c_str());
	fp->write("(index * itemlen, "); //rel offset
	fp->write("itemlen, v); }\n");  //itemlen, val
      }
  } //genUlamTypeWriteDefinitionForC

} //end MFM
