#include "SymbolVariableDataMember.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableDataMember::SymbolVariableDataMember(Token id, UTI utype, PACKFIT packed, u32 slot, CompilerState& state) : SymbolVariable(id, utype, packed, state), m_dataMemberUnpackedSlotIndex(slot), m_hasInitValue(false), m_initvalReady(false), m_initval(0)
  {
    setDataMemberClass(m_state.getCompileThisIdx());
  }

  SymbolVariableDataMember::SymbolVariableDataMember(const SymbolVariableDataMember& sref) : SymbolVariable(sref), m_dataMemberUnpackedSlotIndex(sref.m_dataMemberUnpackedSlotIndex), m_hasInitValue(sref.m_hasInitValue), m_initvalReady(false), m_initval(0) {} //initval set by node vardecl c&l

  SymbolVariableDataMember::~SymbolVariableDataMember()
  {
    //   m_static.clearAllocatedMemory(); //clean up arrays,etc.
  }

  Symbol * SymbolVariableDataMember::clone()
  {
    return new SymbolVariableDataMember(*this);
  }

  u32  SymbolVariableDataMember::getDataMemberUnpackedSlotIndex()
  {
    return m_dataMemberUnpackedSlotIndex;
  }

  s32 SymbolVariableDataMember::getBaseArrayIndex()
  {
    return (s32) getDataMemberUnpackedSlotIndex();
  }

  const std::string SymbolVariableDataMember::getMangledPrefix()
  {
    return "Um_";
  }

  bool SymbolVariableDataMember::hasInitValue()
  {
    return m_hasInitValue;
  }

  void SymbolVariableDataMember::setHasInitValue()
  {
     m_hasInitValue = true;
     m_initvalReady = false;
  }

  bool SymbolVariableDataMember::initValueReady()
  {
    return m_initvalReady;
  }

  bool SymbolVariableDataMember::getInitValue(u64& val)
  {
    assert(hasInitValue());

    if(initValueReady())
      {
	val = m_initval;
	return true;
      }
    return false;
  } //getInitValue

  void SymbolVariableDataMember::setInitValue(u64 val)
  {
    m_initvalReady = true;
    m_initval = val;
  }

  // replaced by NodeVarDecl:genCode to leverage the declaration order preserved by the parse tree.
  void SymbolVariableDataMember::generateCodedVariableDeclarations(File * fp, ULAMCLASSTYPE classtype)
  {
    UTI vuti = getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    m_state.indent(fp);
    fp->write(vut->getUlamTypeMangledName().c_str()); //for C++

    if(vclasstype == UC_QUARK)       // called on classtype elements only
      {
	fp->write("<");
	fp->write_decimal(getPosOffset());
	fp->write(">");
      }
    fp->write(" ");
    fp->write(getMangledName().c_str());

#if 0
    s32 arraysize = vut->getArraySize();
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }
#endif
    fp->write(";\n");
  } //generateCodedVariableDeclarations

  // replaces NodeVarDecl:printPostfix to learn the values of Class' storage in center site
  void SymbolVariableDataMember::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    UTI vuti = getUlamTypeIdx();
    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(vuti);
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClass();
    ULAMTYPE vetyp = vut->getUlamTypeEnum();

    fp->write(" ");
    if(vclasstype == UC_NOTACLASS)
      fp->write(vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
    else
      fp->write(vut->getUlamTypeNameBrief().c_str());

    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(getId()).c_str());

    s32 arraysize = m_state.getArraySize(vuti);
    //scalar has 'size=1'; empty array [0] is still '0'.
    s32 size = (arraysize > NONARRAYSIZE ? arraysize : 1);

    //output the arraysize (optional), and open paren
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }

    fp->write("(");

    if(vclasstype == UC_QUARK)
      {
	UTI scalarquark = m_state.getUlamTypeAsScalar(vuti);
	//printPostfixValuesForClass:
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(scalarquark, csym);
	assert(isDefined);

	NodeBlockClass * classNode = csym->getClassBlockNode();
	assert(classNode);
	u32 newstartpos = startpos + getPosOffset();
	s32 len = vut->getBitSize();
	for(s32 i = 0; i < size; i++)
	  classNode->printPostfixDataMembersSymbols(fp, slot, newstartpos + len * i, vclasstype);
      }
    else
      {
	PACKFIT packFit = m_state.determinePackable(vuti);
	assert(WritePacked(packFit)); //has to be to fit in an atom/site;

	char * valstr = new char[size * 8 + MAXBITSPERLONG]; //was 32

	if(size > 0)
	  {
	    //build the string of values (for both scalar and packed array)
	    UlamValue arrayPtr = UlamValue::makePtr(slot, EVENTWINDOW, vuti, packFit, m_state, startpos + getPosOffset(), getId());
	    UlamValue nextPtr = UlamValue::makeScalarPtr(arrayPtr, m_state);

	    UlamValue atval = m_state.getPtrTarget(nextPtr);
	    s32 len = m_state.getBitSize(vuti);
	    if(len == UNKNOWNSIZE)
	      {
		sprintf(valstr,"unknown");
		for(s32 i = 1; i < size; i++)
		  {
		    strcat(valstr,", unknown");
		  }
	      }
	    else if(len <= MAXBITSPERINT)
	      {
		u32 data = atval.getDataFromAtom(nextPtr, m_state);
		vut->getDataAsString(data, valstr, 'z'); //'z' -> no preceeding ','
		if(vetyp == Unsigned || vetyp == Unary)
		  strcat(valstr, "u");

		for(s32 i = 1; i < size; i++)
		  {
		    char tmpstr[8];
		    AssertBool isNext = nextPtr.incrementPtr(m_state);
		    assert(isNext);
		    atval = m_state.getPtrTarget(nextPtr);
		    data = atval.getDataFromAtom(nextPtr, m_state);
		    vut->getDataAsString(data, tmpstr, ',');
		    if(vetyp == Unsigned || vetyp == Unary)
		      strcat(tmpstr, "u");
		    strcat(valstr,tmpstr);
		  }
	      }
	    else if(len <= MAXBITSPERLONG)
	      {
		u64 data = atval.getDataLongFromAtom(nextPtr, m_state);
		vut->getDataLongAsString(data, valstr, 'z'); //'z' -> no preceeding ','
		if(vetyp == Unsigned || vetyp == Unary)
		  strcat(valstr, "u");

		for(s32 i = 1; i < size; i++)
		  {
		    char tmpstr[8];
		    AssertBool isNext = nextPtr.incrementPtr(m_state);
		    assert(isNext);
		    atval = m_state.getPtrTarget(nextPtr);
		    data = atval.getDataLongFromAtom(nextPtr, m_state);
		    vut->getDataLongAsString(data, tmpstr, ',');
		    if(vetyp == Unsigned || vetyp == Unary)
		      strcat(tmpstr, "u");
		    strcat(valstr,tmpstr);
		  }
	      }
	    else
	      assert(0);

	  } //end arrays > 0, and scalar
	else
	  {
	    sprintf(valstr," ");
	  }

	fp->write(valstr); //results out here!
	delete [] valstr;
      } //not a quark
    fp->write("); ");
  } //printPostfixValuesOfVariableDeclarations

  void SymbolVariableDataMember::setStructuredComment()
  {
    Token scTok;
    if(m_state.getStructuredCommentToken(scTok)) //and clears it
      {
	m_structuredCommentToken = scTok;
	m_gotStructuredCommentToken = true;
      }
  } //setStructuredComment

} //end MFM
