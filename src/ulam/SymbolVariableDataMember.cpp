#include "SymbolVariableDataMember.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableDataMember::SymbolVariableDataMember(const Token& id, UTI utype, u32 slot, CompilerState& state) : SymbolVariable(id, utype, state), m_posOffset(0), m_dataMemberUnpackedSlotIndex(slot)
  {
    setDataMemberClass(m_state.getCompileThisIdx());
  }

  SymbolVariableDataMember::SymbolVariableDataMember(const SymbolVariableDataMember& sref) : SymbolVariable(sref), m_posOffset(sref.m_posOffset), m_dataMemberUnpackedSlotIndex(sref.m_dataMemberUnpackedSlotIndex) { } //initval set by node vardecl c&l

  SymbolVariableDataMember::SymbolVariableDataMember(const SymbolVariableDataMember& sref, bool keepType) : SymbolVariable(sref, keepType), m_posOffset(sref.m_posOffset), m_dataMemberUnpackedSlotIndex(sref.m_dataMemberUnpackedSlotIndex) {} //initval set by node vardecl c&l

  SymbolVariableDataMember::~SymbolVariableDataMember()
  {
    //   m_static.clearAllocatedMemory(); //clean up arrays,etc.
  }

  Symbol * SymbolVariableDataMember::clone()
  {
    return new SymbolVariableDataMember(*this);
  }

  Symbol * SymbolVariableDataMember::cloneKeepsType()
  {
    return new SymbolVariableDataMember(*this, true);
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

  const std::string SymbolVariableDataMember::getMangledName()
  {
    // to distinguish btn an atomic parameter typedef and quark typedef;
    // use atomic parameter with array of classes
    UlamType * sut = m_state.getUlamTypeByIndex(getUlamTypeIdx());
    bool isaclass = ((sut->getUlamTypeEnum() == Class) && sut->isScalar());

    std::ostringstream mangled;
    std::string nstr = m_state.getDataAsStringMangled(getId());
    mangled << Symbol::getParameterTypePrefix(isaclass) << getMangledPrefix() << nstr.c_str();

    return mangled.str();
  } //getMangledName

  //packed bit position of data members; relative to ATOMFIRSTSTATEBITPOS (or 0u).
  u32 SymbolVariableDataMember::getPosOffset()
  {
    return m_posOffset;
  }

  void SymbolVariableDataMember::setPosOffset(u32 offsetIntoAtom)
  {
    m_posOffset = offsetIntoAtom; //relative to first state bit
  }

  // replaced by NodeVarDecl:genCode to leverage the declaration order preserved by the parse tree.
  void SymbolVariableDataMember::generateCodedVariableDeclarations(File * fp, ULAMCLASSTYPE classtype)
  {
    assert(classtype == UC_ELEMENT); //really?
    UTI vuti = getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();

    m_state.indentUlamCode(fp);
    fp->write(vut->getUlamTypeMangledName().c_str()); //for C++

    if(vclasstype == UC_QUARK) //called on classtype elements only
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
    fp->write(";"); GCNL;
  } //generateCodedVariableDeclarations

  // replaces NodeVarDecl:printPostfix to learn the values of Class' storage in center site
  void SymbolVariableDataMember::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    UTI vuti = getUlamTypeIdx();
    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(vuti);
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();
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
	//outputs the data members, not just the lump value (e.g. SWV::printPostfixValue())
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
	  classNode->printPostfixDataMembersSymbols(fp, slot, newstartpos + len * i, scalarquark);
      }
    else
      {
	PACKFIT packFit = m_state.determinePackable(vuti);
	assert(WritePacked(packFit)); //has to be to fit in an atom/site;

	char * valstr = new char[size * 8 + MAXBITSPERSTRING + 2 + 1]; //was 64

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
		if((vetyp == String) && (data == 0))
		  sprintf(valstr," ");
		else
		  vut->getDataAsString(data, valstr, 'z'); //'z' -> no preceeding ','
		if(vetyp == Unsigned || vetyp == Unary)
		  strcat(valstr, "u");

		for(s32 i = 1; i < size; i++)
		  {
		    char tmpstr[MAXBITSPERSTRING + 2 + 1]; //was 8
		    AssertBool isNext = nextPtr.incrementPtr(m_state);
		    assert(isNext);
		    atval = m_state.getPtrTarget(nextPtr);
		    data = atval.getDataFromAtom(nextPtr, m_state);
		    if((vetyp == String) && (data == 0))
		      strcat(valstr, ",");
		    else
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
		    char tmpstr[MAXBITSPERSTRING + 2 + 1]; //was 8
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
	      m_state.abortGreaterThanMaxBitsPerLong();

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
