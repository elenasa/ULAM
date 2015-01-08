#include <stdio.h>
#include <iostream>
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "SymbolTable.h"
#include "SymbolTypedef.h"
#include "SymbolVariable.h"
#include "UlamTypeAtom.h"
#include "UlamTypeBool.h"
#include "UlamTypeUnary.h"
#include "UlamTypeBits.h"
#include "UlamTypeInt.h"
#include "UlamTypeUnsigned.h"
#include "UlamTypeNav.h"
#include "UlamTypeVoid.h"
#include "UlamTypePtr.h"


namespace MFM {

  //#define _DEBUG_OUTPUT
#ifdef _DEBUG_OUTPUT
  static const bool debugOn = true;
#else
  static const bool debugOn = false;
#endif

  static const char * m_indentedSpaceLevel("  ");

  static const char * HIDDEN_ARG_NAME = "Uv_4self";
  static const char * HIDDEN_CONTEXT_ARG_NAME = "uc";
  static const char * CUSTOMARRAY_GET_FUNC_NAME = "aref";
  static const char * CUSTOMARRAY_SET_FUNC_NAME = "aset";
  static const char * IS_MANGLED_FUNC_NAME = "internalCMethodImplementingIs";   //Uf_2is;
  static const char * HAS_MANGLED_FUNC_NAME = "PositionOfDataMemberType";       //"Uf_3has";
  static const char * HAS_MANGLED_FUNC_NAME_FOR_ATOM = "UlamElement<CC>::PositionOfDataMember";

  //use of this in the initialization list seems to be okay;
  CompilerState::CompilerState(): m_programDefST(*this), m_currentBlock(NULL), m_classBlock(NULL), m_useMemberBlock(false), m_currentMemberClassBlock(NULL), m_currentFunctionBlockDeclSize(0), m_currentFunctionBlockMaxDepth(0), m_parsingControlLoop(0), m_parsingElementParameterVariable(false), m_parsingConditionalAs(false), m_genCodingConditionalAs(false), m_eventWindow(*this), m_currentSelfSymbolForCodeGen(NULL), m_nextTmpVarNumber(0)
  {
    m_err.init(this, debugOn, NULL);
  }


  CompilerState::~CompilerState()
  {
    clearAllDefinedUlamTypes();
    clearAllLinesOfText();
    m_currentObjSymbolsForCodeGen.clear();
  }


  void CompilerState::clearAllDefinedUlamTypes()
  {
    for(u32 i = 0; i < m_indexToUlamType.size(); i++)
      {
	UlamType * ut = m_indexToUlamType.at(i);
	delete ut;
      }
    m_indexToUlamType.clear();
    m_definedUlamTypes.clear();
    m_currentFunctionReturnNodes.clear();
  }


  void CompilerState::clearAllLinesOfText()
  {
    std::map<u32, std::vector<u32>*>::iterator it;

    for(it = m_textByLinePerFilePath.begin(); it != m_textByLinePerFilePath.end(); it++)
      {
	std::vector<u32> * v = it->second;
	v->clear();
	delete v;
      }

    m_textByLinePerFilePath.clear();
  }


  //convenience method (refactors code originally from installSymbol)
  //if exists, just returns it, o.w. makes it;
  // trick to know the base ULAMTYPE
  UTI CompilerState::makeUlamType(Token typeTok, s32 bitsize, s32 arraysize)
  {
    //type names begin with capital letter..and the rest can be either
    u32 typeNameId = getTokenAsATypeNameId(typeTok); //Foo, Int, etc

    //can't be a typedef!! get's the wrong name for type key; use key as arg
    UTI tmputi;
    assert(!getUlamTypeByTypedefName(typeTok.m_dataindex, tmputi));

    // is this name already a typedef for a complex type?
    ULAMTYPE bUT = getBaseTypeFromToken(typeTok);
    if(bitsize == 0)
      bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

    UlamKeyTypeSignature key(typeNameId,bitsize,arraysize);
    UTI uti = Nav;
    //    if(!isDefined(key,uti))
    if(!isDefined(key,uti) || bitsize == UNKNOWNSIZE || arraysize == UNKNOWNSIZE)
      {
	//no key, make new type, how to know baseUT? bitsize?
	uti = makeUlamType(key,bUT);
      }
    return uti;
  } //makeUlamType


  UTI CompilerState::makeUlamType(UlamKeyTypeSignature key, ULAMTYPE utype)
  {
    UTI uti;
    //if(!isDefined(key, uti))
    if(!isDefined(key,uti) || key.getUlamKeyTypeSignatureBitSize() == UNKNOWNSIZE || key.getUlamKeyTypeSignatureArraySize() == UNKNOWNSIZE)
      {
	uti = m_indexToUlamType.size();  //custom index based on key
	UlamType * ut = createUlamType(key, uti, utype);
	m_indexToUlamType.push_back(ut);  //vector owns ut
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature,UTI>(key,uti));
	assert(isDefined(key, uti));
      }
    return uti;
  } //makeUlamType


  bool CompilerState::isDefined(UlamKeyTypeSignature key, UTI& foundUTI)
  {
    bool rtnBool= false;

    std::map<UlamKeyTypeSignature, UTI, less_than_key>::iterator it = m_definedUlamTypes.begin();

    while(it != m_definedUlamTypes.end())
      {
	if(key == it->first)
	  {
	    foundUTI = it->second;
	    rtnBool = true;
	    break;
	  }
	it++;
      }
    return rtnBool;
  } //isDefined


  UlamType * CompilerState::createUlamType(UlamKeyTypeSignature key, UTI uti, ULAMTYPE utype)
  {
    UlamType * ut = NULL;

    switch(utype)
      {
      case Nav:
	ut = new UlamTypeNav(key, uti);
	break;
      case Void:
	ut = new UlamTypeVoid(key, uti);
	break;
      case Int:
	ut = new UlamTypeInt(key, uti);
	break;
      case Unsigned:
	ut = new UlamTypeUnsigned(key, uti);
	break;
      case Bool:
	ut = new UlamTypeBool(key, uti);
	break;
      case Unary:
	ut = new UlamTypeUnary(key, uti);
	break;
      case Bits:
	ut = new UlamTypeBits(key, uti);
	break;
      case Class:
	ut = new UlamTypeClass(key, uti);
	break;
      case UAtom:
	ut = new UlamTypeAtom(key, uti);
	break;
      case Ptr:
	ut = new UlamTypePtr(key, uti);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Undefined ULAMTYPE base type <" << utype << ">" ;
	  MSG2("",msg.str().c_str(),DEBUG);
	  assert(0);
	}
      };

    return ut;
  } //createUlamType


  //used to update Class' calculated bit size (setBitSize)
  bool CompilerState::deleteUlamKeyTypeSignature(UlamKeyTypeSignature key, UTI uti)
  {
    bool rtnBool= false;

    std::map<UlamKeyTypeSignature, UTI, less_than_key>::iterator it = m_definedUlamTypes.begin();

    while(it != m_definedUlamTypes.end())
      {
	if(key == it->first && it->second == uti)
	  {
	    m_definedUlamTypes.erase(it);
	    rtnBool = true;
	    break;
	  }
	it++;
      }
    return rtnBool;
  } //deleteUlamKeyTypeSignature


  UlamType * CompilerState::getUlamTypeByIndex(UTI typidx)
  {
    if(typidx >= m_indexToUlamType.size())
      {
	std::ostringstream msg;
	msg << "Undefined UTI <" << typidx << "> Max is: "
	    << m_indexToUlamType.size() << ", returning Nav INSTEAD";
	MSG2("", msg.str().c_str(),DEBUG);
	typidx = 0;
      }
    return m_indexToUlamType[typidx];
  } //getUlamTypeByIndex


  const std::string CompilerState::getUlamTypeNameBriefByIndex(UTI uti)
  {
    return m_indexToUlamType[uti]->getUlamTypeNameBrief(this);
  }


  const std::string CompilerState::getUlamTypeNameByIndex(UTI uti)
  {
    return m_indexToUlamType[uti]->getUlamTypeName(this);
  }


  UTI CompilerState::getUlamTypeIndex(UlamType * ut)
  {
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
    UTI rtnUTI = 0;         //Nav
    isDefined(key,rtnUTI);  //updates rtnUTI if found
    return rtnUTI;
  } //getUlamTypeIndex


  ULAMTYPE CompilerState::getBaseTypeFromToken(Token tok)
  {
    // is this name already a typedef for a complex type?
    ULAMTYPE bUT = Nav;
    UTI ut = Nav;
    if(getUlamTypeByTypedefName(tok.m_dataindex, ut))
      {
	bUT = getUlamTypeByIndex(ut)->getUlamTypeEnum();
      }
    else
      {
	if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    std::string typeName = getTokenAsATypeName(tok); //Int, etc

	    //no way to get the bUT, except to assume typeName is one of them?
	    bUT = UlamType::getEnumFromUlamTypeString(typeName.c_str()); //could be Element, etc.;
	  }
	else
	  {
	    // it's an element or quark! base type is Ut_Class?
	    SymbolClass * csym = NULL;
	    if(alreadyDefinedSymbolClass(tok.m_dataindex, csym))
	      {
		UTI ut = csym->getUlamTypeIdx();
		bUT = getUlamTypeByIndex(ut)->getUlamTypeEnum();
	      }
	  }
      }
    return bUT;
  } //getBaseTypeFromToken


  UTI CompilerState::getUlamTypeFromToken(Token tok, s32 typebitsize, s32 arraysize)
  {
    UTI uti = Nav;

    // is this name already a typedef for a complex type?
    if(!getUlamTypeByTypedefName(tok.m_dataindex, uti))
      {
	if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    uti = makeUlamType(tok, typebitsize, arraysize);
	  }
	else
	  {
	    //check for a Class type, or make one if doesn't exist yet, while parsing.
	    SymbolClass * csym = NULL;
	    if(alreadyDefinedSymbolClass(tok.m_dataindex, csym))
	      {
		uti = csym->getUlamTypeIdx();
	      }
	  }
      }
    return uti;
  } //getUlamTypeFromToken


  // new version! uses indexes
  bool CompilerState::getUlamTypeByTypedefName(u32 nameIdx, UTI & rtnType)
  {
    bool rtnBool = false;
    Symbol * asymptr = NULL;

    // e.g. KEYWORDS have no m_dataindex (=0); short-circuit
    if(nameIdx == 0) return false;

    //searched back through all block's ST for idx
    if(alreadyDefinedSymbol(nameIdx, asymptr))
      {
	if(asymptr->isTypedef())
	  {
	    rtnType = asymptr->getUlamTypeIdx();
	    rtnBool = true;
	  }
      }
    return rtnBool;
  } //getUlamTypeByTypedefName


  UTI CompilerState::getUlamTypeAsScalar(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    if(ut->isScalar())
      return utArg;

    //    if(ut->getUlamClass() != UC_NOTACLASS)
    //  return Atom;  //e.g. a Window quark ???

    // for typedef array, the scalar is the primitive type
    ULAMTYPE bUT = ut->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();

    u32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    UlamKeyTypeSignature baseKey(keyOfArg.m_typeNameId, bitsize);  //default array size is zero

    UTI buti = makeUlamType(baseKey, bUT);
    return buti;
  } //getUlamTypeAsScalar


  UTI CompilerState::getUlamTypeOfConstant(ULAMTYPE etype)
  {
    u32 enumStrIdx = m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(etype));
    UlamKeyTypeSignature ckey(enumStrIdx, ANYBITSIZECONSTANT, NONARRAYSIZE);
    return makeUlamType(ckey, etype); //may not exist yet, create
  }


  UTI CompilerState::getDefaultUlamTypeOfConstant(UTI ctype)
  {
    ULAMTYPE etype = getUlamTypeByIndex(ctype)->getUlamTypeEnum();
    u32 enumStrIdx = m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(etype));
    UlamKeyTypeSignature ckey(enumStrIdx, getDefaultBitSize(ctype), NONARRAYSIZE);
    return makeUlamType(ckey, etype); //may not exist yet, create
  }


  bool CompilerState::isConstant(UTI uti)
  {
    UlamType * ut = getUlamTypeByIndex(uti);
    return (ut->isConstant());
  }


  bool CompilerState::isScalar(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return (ut->isScalar());
  }


  s32 CompilerState::getArraySize(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return (ut->getArraySize());
  }


  s32 CompilerState::getBitSize(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return (ut->getBitSize());
  }


  // this may go away..
  // updates key. we can do this now that UTI is used and the UlamType * isn't saved
  void CompilerState::setBitSize(UTI utArg, s32 total)
  {
    return setSizesOfClass(utArg, total, NONARRAYSIZE);  //formerly
  }


  void CompilerState::setSizesOfClass(UTI utArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    ULAMCLASSTYPE classtype = ut->getUlamClass();
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();

    //redirect primitives;
    if(!(classtype == UC_ELEMENT || classtype == UC_QUARK))
      {
	return setSizesOfNonClass(utArg, bitsize, arraysize);
      }

    //allow for zero-sized classes; insure same bit size
    if(key.getUlamKeyTypeSignatureBitSize() == 0)
      {
	assert((s32) key.getUlamKeyTypeSignatureBitSize() == bitsize);
	return;
      }

    s32 total = bitsize + arraysize; //???

    bool isCustomArray = ut->isCustomArray();
    UTI caType = (isCustomArray ? ((UlamTypeClass *) ut)->getCustomArrayType() : Nav);

    //verify total bits is within limits for elements and quarks
    if(classtype == UC_ELEMENT)
      {
	if(total > MAXSTATEBITS)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXSTATEBITS << ") for element " << ut->getUlamTypeName(this).c_str() << " with " << total << " bits";
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return;
	  }
      }

    if(classtype == UC_QUARK)
      {
	if(total > MAXBITSPERQUARK)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXBITSPERQUARK << ") for quark " << ut->getUlamTypeName(this).c_str() << " with " << total << " bits";
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return;
	  }
      }

    //continue with valid number of bits for Class UlamTypes only
    //UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key.getUlamKeyTypeSignatureNameId(), total, key.getUlamKeyTypeSignatureArraySize());
    UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key.getUlamKeyTypeSignatureNameId(), bitsize, arraysize);

    //remove old key from map
    deleteUlamKeyTypeSignature(key, utArg);
    delete ut;  // clear vector
    m_indexToUlamType[utArg] = NULL;
    assert(!isDefined(key, utArg));

    UlamType * newut = createUlamType(newkey, utArg, Class);
    m_indexToUlamType[utArg] = newut;
    m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature,UTI>(newkey,utArg));
    ((UlamTypeClass *) newut)->setUlamClass(classtype); //restore from original ut

    if(isCustomArray)
      ((UlamTypeClass *) newut)->setCustomArrayType(caType);

    assert(isDefined(newkey, utArg));
#if 1
    {
      std::ostringstream msg;
      msg << "Sizes set for Class: " << newut->getUlamTypeName(this).c_str();
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), INFO);
    }
#endif
  } //setSizesOfClass


  void CompilerState::setSizesOfNonClass(UTI utArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    ULAMTYPE bUT = ut->getUlamTypeEnum();
    ULAMCLASSTYPE classtype = ut->getUlamClass();
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();

    assert(classtype == UC_NOTACLASS && (key.getUlamKeyTypeSignatureBitSize() == UNKNOWNSIZE || key.getUlamKeyTypeSignatureArraySize() == UNKNOWNSIZE));

    //disallow zero-sized primitives (no such thing as a BitVector<0u>)
    if(key.getUlamKeyTypeSignatureBitSize() == 0 || bitsize == 0)
      {
	assert(0);
	return;
      }

    //continue with valid number of bits
    UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key.getUlamKeyTypeSignatureNameId(), bitsize, arraysize);

    //remove old key from map
    deleteUlamKeyTypeSignature(key, utArg);
    delete ut;  // clear vector
    m_indexToUlamType[utArg] = NULL;
    assert(!isDefined(key, utArg));

    UlamType * newut = createUlamType(newkey, utArg, bUT);
    m_indexToUlamType[utArg] = newut;
    m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature,UTI>(newkey,utArg));
    ((UlamTypeClass *) newut)->setUlamClass(classtype); //restore from original ut

    assert(isDefined(newkey, utArg));
#if 1
    {
      std::ostringstream msg;
      msg << "Sizes set for nonClass: " << newut->getUlamTypeName(this).c_str();
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), INFO);
    }
#endif
  } // setSizesOfNonClass


  s32 CompilerState::getDefaultBitSize(UTI uti)
  {
    ULAMTYPE et = getUlamTypeByIndex(uti)->getUlamTypeEnum();
    return ULAMTYPE_DEFAULTBITSIZE[et];
  }


  u32 CompilerState::getTotalBitSize(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return (ut->getTotalBitSize());
  }


  s32 CompilerState::slotsNeeded(UTI uti)
  {
    if(uti == Void)
      return 0;

    s32 arraysize = getArraySize(uti);
    PACKFIT packed = determinePackable(uti);

    if(WritePacked(packed))
      arraysize = 1;
    else
      arraysize = (arraysize > NONARRAYSIZE ? arraysize : 1);
    return arraysize;
  } //slotsNeeded


  bool CompilerState::getUlamTypeByClassToken(Token ctok, UTI & rtnType)
  {
    u32 cidx = getTokenAsATypeNameId(ctok);
    return getUlamTypeByClassNameId(cidx, rtnType);
  }


  bool CompilerState::getUlamTypeByClassNameId(u32 idx, UTI & rtnType)
  {
    bool rtnBool = false;
    SymbolClass * csymptr = NULL;

    if(alreadyDefinedSymbolClass(idx, csymptr) || (addIncompleteClassSymbolToProgramTable(idx, csymptr), true) )
      {
	rtnType = csymptr->getUlamTypeIdx();
	rtnBool = true;
      }

    assert(rtnBool);  //no way it's false!
    return rtnBool;
  }


  bool CompilerState::alreadyDefinedSymbolClass(u32 dataindex, SymbolClass * & symptr)
  {
    return m_programDefST.isInTable(dataindex,(Symbol * &) symptr);
  }


  //temporary UlamType which will be updated during type labeling.
  void CompilerState::addIncompleteClassSymbolToProgramTable(u32 dataindex, SymbolClass * & symptr)
  {
    assert(!alreadyDefinedSymbolClass(dataindex,symptr));

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE);  //0 and scalar default
    UTI cuti = makeUlamType(key, Class);  //**gets next unknown uti type

    // symbol ownership goes to the programDefST;
    symptr = new SymbolClass(dataindex, cuti, NULL, *this);  //NodeBlockClass is NULL for now
    m_programDefST.addToTable(dataindex, symptr);
  } //addIncompleteClassSymbolToProgramTable


  bool CompilerState::completeIncompleteClassSymbol(UTI incomplete)
  {
    bool rtnB = false;
    SymbolClass * csym = NULL;
    UlamType * ict = getUlamTypeByIndex(incomplete);
    if(alreadyDefinedSymbolClass(ict->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
      {
	UTI but = csym->getUlamTypeIdx();
	ULAMCLASSTYPE bc = getUlamTypeByIndex(but)->getUlamClass();

	//e.g. out-of-scope typedef is not a class, return false
	if(bc == UC_ELEMENT || bc == UC_QUARK)
	  {
	    ((UlamTypeClass *) ict)->setUlamClass(bc);
#if 1
	    if(getBitSize(but) == UNKNOWNSIZE || getArraySize(but) == UNKNOWNSIZE)
	      {
		std::ostringstream msg;
		msg << "Sizes still unknown for Class: " << ict->getUlamTypeName(this).c_str();
		MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), INFO);
	      }
	    else
#endif
	    rtnB = true;
	  }
      }
    else
      {
	assert(0);
      }

    return rtnB;
  } //completeIncompleteClassSymbol


  bool CompilerState::alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;

    // start with the current "top" block and look down the stack
    // until the 'variable id' is found.
    NodeBlock * blockNode = m_currentBlock;

    // substitute another selected class block to search for data member
    if(m_useMemberBlock)
      blockNode = m_currentMemberClassBlock;

    while(!brtn && blockNode)
      {
	brtn = blockNode->isIdInScope(dataindex,symptr);
	blockNode = blockNode->getPreviousBlockPointer();  //traverse the chain
      }

    // data member variables in class block; function symbols are linked to their
    // function def block; check function data members separately.
    if(!brtn)
      {
	brtn = isFuncIdInClassScope(dataindex, symptr);
      }

    return brtn;
  }


  bool CompilerState::isFuncIdInClassScope(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;
    if(m_useMemberBlock)
      {
	if(m_currentMemberClassBlock)
	  brtn = m_currentMemberClassBlock->isFuncIdInScope(dataindex,symptr);
      }
    else
      brtn = m_classBlock->isFuncIdInScope(dataindex,symptr);

    return brtn;
  }


  bool CompilerState::isIdInClassScope(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;
    if(m_useMemberBlock)
      {
	if(m_currentMemberClassBlock)
	  brtn = m_currentMemberClassBlock->isIdInScope(dataindex,symptr);
      }
    else
      brtn = m_classBlock->isIdInScope(dataindex,symptr);

    return brtn;
  }


  //symbol ownership goes to the current block (end of vector)
  void CompilerState::addSymbolToCurrentScope(Symbol * symptr)
  {
    m_currentBlock->addIdToScope(symptr->getId(), symptr);
  }


  //Token to location as string:
  const std::string CompilerState::getTokenLocationAsString(Token * tok)
  {
    if(!tok)
      return std::string("");

    return getFullLocationAsString(tok->m_locator);
  }


  /** combines path, line, and byte number for error messages */
  const std::string CompilerState::getFullLocationAsString(Locator& loc)
  {
    std::ostringstream ss;
    ss << getPathFromLocator(loc) << ":" << loc.getLineNo() << ":" << loc.getByteNo() << ":";
    return ss.str();
  }


  const std::string CompilerState::getPathFromLocator(Locator& loc)
  {
    return m_pool.getDataAsString(loc.getPathIndex());
  }


  const std::string CompilerState::getTokenDataAsString(Token * tok)
  {
    assert(tok);
    if(tok->m_dataindex > 0)
      {
	return m_pool.getDataAsString(tok->m_dataindex);
      }
    return std::string(tok->getTokenString());
  }


  std::string CompilerState::getDataAsStringMangled(u32 dataindex)
  {
    std::ostringstream mangled;
    std::string nstr = m_pool.getDataAsString(dataindex);
    u32 nstrlen = nstr.length();

    if(nstrlen < BASE10 - 1)
      {
	mangled << nstrlen << nstr.c_str();
      }
    else
      {
	mangled << 9 << DigitCount(nstrlen, BASE10) << nstrlen << nstr.c_str();
      }
    return mangled.str();
  } //getDataAsStringMangled


  //does it check for existence?
  const std::string CompilerState::getTokenAsATypeName(Token tok)
  {
    if(Token::isTokenAType(tok))
      {
	if((Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD))
	  {
	    return std::string(Token::getTokenAsString(tok.m_type));
	  }
	else
	  {
	    UTI tduti = Nav;
	    if(getUlamTypeByTypedefName(tok.m_dataindex, tduti))
	      {
		UlamType * tdut = getUlamTypeByIndex(tduti);
		//for typedef quarks return quark name, o.w. base name
		return tdut->getUlamTypeNameOnly(this);
	      }
	    else
	      return getTokenDataAsString(&tok); //a class
	  }
      }
    return "Nav";
  }


  u32 CompilerState::getTokenAsATypeNameId(Token tok)
  {
    std::string nstr = getTokenAsATypeName(tok);
    return m_pool.getIndexForDataString(nstr);
  }


  bool CompilerState::checkFunctionReturnNodeTypes(SymbolFunction * fsym)
  {
    bool rtnBool = true;
    UTI it = fsym->getUlamTypeIdx();

    if(m_currentFunctionReturnNodes.empty())
      {
	if(it != Void && !fsym->isNativeFunctionDeclaration())
	  {
	    std::ostringstream msg;
	    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return Statement is missing; Return type: " << getUlamTypeNameByIndex(it).c_str();
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
	return true;  //okay to skip return statement for void function
      }


    for(u32 i = 0; i < m_currentFunctionReturnNodes.size(); i++)
      {
	NodeReturnStatement * rNode = m_currentFunctionReturnNodes.at(i);
	UTI rType = rNode->getNodeType();

	if(rType != it)
	  {
	    rtnBool = false;

	    ULAMTYPE rBUT = getUlamTypeByIndex(rType)->getUlamTypeEnum();
	    ULAMTYPE itBUT = getUlamTypeByIndex(it)->getUlamTypeEnum();
	    if(rBUT != itBUT)
	      {
		std::ostringstream msg;
		msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return type's: " << getUlamTypeNameByIndex(it).c_str() << " base type: <" << UlamType::getUlamTypeEnumAsString(itBUT) << ">, does not match resulting type's " << getUlamTypeNameByIndex(rType).c_str() << " base type: <" << UlamType::getUlamTypeEnumAsString(rBUT) << ">";
		m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_ERR);
	      }
	    else
	      {
		if(getArraySize(rType) != getArraySize(it))
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return type's: " << getUlamTypeNameByIndex(it).c_str() << " array size: <" << getArraySize(it) << "> does not match resulting type's: " << getUlamTypeNameByIndex(rType).c_str() << " array size: <" << getArraySize(rType) << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_ERR);
		  }

		if(getBitSize(rType) != getBitSize(it))
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return type's: " << getUlamTypeNameByIndex(it).c_str() << " bit size: <" << getBitSize(it) << "> does not match resulting type's: " << getUlamTypeNameByIndex(rType).c_str() << " bit size: <" << getBitSize(rType) << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_ERR);
		  }
	      } //base types are the same..array and bit size might vary
	  } //different ulamtype
      } //next return node

    return rtnBool;
  } //checkFunctionReturnNodeTypes


  void CompilerState::indent(File * fp)
  {
    for(u32 i = 0; i < m_currentIndentLevel; i++)
      {
	fp->write(m_indentedSpaceLevel);
      }
  }


  const char * CompilerState::getHiddenArgName()
  {
    return  HIDDEN_ARG_NAME;
  }


  const char * CompilerState::getHiddenContextArgName()
  {
    return  HIDDEN_CONTEXT_ARG_NAME;
  }


  u32 CompilerState::getCustomArrayGetFunctionNameId()
  {
    std::string str(CUSTOMARRAY_GET_FUNC_NAME);
    return m_pool.getIndexForDataString(str);
  }


  u32 CompilerState::getCustomArraySetFunctionNameId()
  {
    std::string str(CUSTOMARRAY_SET_FUNC_NAME);
    return  m_pool.getIndexForDataString(str);
  }

  const char * CompilerState::getIsMangledFunctionName()
  {
    return IS_MANGLED_FUNC_NAME;
  }


  const char * CompilerState::getHasMangledFunctionName(UTI ltype)
  {
    if(ltype == UAtom)
      return HAS_MANGLED_FUNC_NAME_FOR_ATOM;
    return HAS_MANGLED_FUNC_NAME;
  }


  const char * CompilerState::getAsMangledFunctionName(UTI ltype, UTI rtype)
  {
    ULAMCLASSTYPE rclasstype = getUlamTypeByIndex(rtype)->getUlamClass();
    if(rclasstype == UC_QUARK)
      return getHasMangledFunctionName(ltype);
    else if (rclasstype == UC_ELEMENT)
      return IS_MANGLED_FUNC_NAME;
    else
      assert(0);
    return "AS_ERROR";
  }


  std::string CompilerState::getFileNameForAClassHeader(u32 id, bool wSubDir)
  {
    std::ostringstream f;
    Symbol * csym = m_programDefST.getSymbolPtr(id);
    UTI cuti = csym->getUlamTypeIdx();
    if(wSubDir)
      f << "include/";

    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName(this).c_str() << ".h";
    return f.str();
  }


  std::string CompilerState::getFileNameForThisClassHeader(bool wSubDir)
  {
    return getFileNameForAClassHeader(m_compileThisId, wSubDir);
  }


  std::string CompilerState::getFileNameForThisClassBody(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "include/";

    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName(this).c_str() << ".tcc";
    return f.str();
  }


  std::string CompilerState::getFileNameForThisClassBodyNative(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "include/";

    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName(this).c_str() << "_native.tcc";
    return f.str();
  }


  std::string CompilerState::getFileNameForThisClassCPP(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "src/";

    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName(this).c_str() << ".cpp";
    return f.str();
  }


  std::string CompilerState::getFileNameForThisTypesHeader(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "include/";

    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName(this).c_str() << "_Types.h";
    return f.str();
  }


  //separate file for element compilations, avoid multiple mains, select the one to test during linking
  std::string CompilerState::getFileNameForThisClassMain(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "src/";

    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName(this).c_str() << "_main.cpp";
    return f.str();
  }


  ULAMCLASSTYPE CompilerState::getUlamClassForThisClass()
  {
    UTI cuti = getUlamTypeForThisClass();
    return getUlamTypeByIndex(cuti)->getUlamClass();
  }


  UTI CompilerState::getUlamTypeForThisClass()
  {
    Symbol * csym = m_programDefST.getSymbolPtr(m_compileThisId);
    assert(csym);
    return csym->getUlamTypeIdx();
  }


  const std::string CompilerState::getBitSizeTemplateString(UTI uti)
  {
    ULAMCLASSTYPE classtype = getUlamTypeByIndex(uti)->getUlamClass();
    assert(classtype == UC_QUARK || classtype == UC_ELEMENT);

    std::ostringstream mangled;
    if(classtype == UC_QUARK)
      {
	mangled << "<" << getTotalBitSize(uti) << ">";  //???
      }
    return mangled.str();
  }

  //unfortunately, the uti did not reveal a Class symbol; already down to primitive types
  // for casting.
  const std::string CompilerState::getBitVectorLengthAsStringForCodeGen(UTI uti)
  {
    ULAMCLASSTYPE classtype = getUlamTypeByIndex(uti)->getUlamClass();

    std::ostringstream lenstr;
    if(classtype == UC_NOTACLASS)
      {
	lenstr << getTotalBitSize(uti);
      }
    else
      {
	SymbolClass * csym = NULL;
	UlamType * ict = getUlamTypeByIndex(uti);
	if(alreadyDefinedSymbolClass(ict->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
	  {
	    lenstr << csym->getMangledNameForParameterType();

	    if(classtype == UC_QUARK)
	      {
		lenstr << "::QUARK_SIZE";
	      }
	    else if(classtype == UC_ELEMENT)
	      {
		lenstr << "::LENGTH";
	      }
	    else
	      assert(0);  	    //error!! neither quark nor element
	  }
	else
	  {
	    //error!! no class symbol for this type
	    assert(0);
	  }
      }

    return lenstr.str();
  } //getBitVectorLengthAsStringForCodeGen


  UlamValue CompilerState::getPtrTarget(UlamValue ptr)
  {
    assert(ptr.getUlamValueTypeIdx() == Ptr);

    // slot + storage
    UlamValue valAtIdx;

    switch(ptr.getPtrStorage())
      {
      case STACK:
	valAtIdx = m_funcCallStack.loadUlamValueFromSlot(ptr.getPtrSlotIndex());
	break;
      case EVALRETURN:
	valAtIdx = m_nodeEvalStack.loadUlamValueFromSlot(ptr.getPtrSlotIndex());
	break;
      case EVENTWINDOW:
	valAtIdx = m_eventWindow.loadAtomFromSite(ptr.getPtrSlotIndex());
	break;
      default:
	//error!
	assert(0);
      };

    return valAtIdx;  //return as-is
  }


  //general purpose store
  void CompilerState::assignValue(UlamValue lptr, UlamValue ruv)
  {
    assert(lptr.getUlamValueTypeIdx() == Ptr);

    //if(ruv.getUlamValueTypeIdx() == Ptr)
    // handle UAtom assignment as a singleton (not array values)
    if(ruv.getUlamValueTypeIdx() == Ptr && (ruv.getPtrTargetType() != UAtom || lptr.getPtrTargetType() != UAtom))
      return assignArrayValues(lptr, ruv);

    // r is data (includes packed arrays), store it into where lptr is pointing
    //assert(lptr.getPtrTargetType() == ruv.getUlamValueTypeIdx());
    assert(lptr.getPtrTargetType() == ruv.getUlamValueTypeIdx() || lptr.getPtrTargetType() == UAtom || ruv.getUlamValueTypeIdx() == UAtom);

    STORAGE place = lptr.getPtrStorage();
    switch(place)
      {
      case STACK:
	m_funcCallStack.assignUlamValue(lptr, ruv, *this);
	break;
      case EVALRETURN:
	m_nodeEvalStack.assignUlamValue(lptr, ruv, *this);
	break;
      case EVENTWINDOW:
	m_eventWindow.assignUlamValue(lptr, ruv);
	break;
      default:
	assert(0);
      };
  }


  void CompilerState::assignArrayValues(UlamValue lptr, UlamValue rptr)
  {
    assert(lptr.getUlamValueTypeIdx() == Ptr);
    assert(rptr.getUlamValueTypeIdx() == Ptr);

    //assert types..the same, and arrays
    assert(lptr.getPtrTargetType() == rptr.getPtrTargetType());
    UTI tuti = rptr.getPtrTargetType();

    // unless we're copying from different storage classes, or an element
    //assert(!isScalar(lptr.getPtrTargetType()));

    //assigned packed or unpacked
    PACKFIT packed = lptr.isTargetPacked();
    if(packed != rptr.isTargetPacked())
      {
	std::ostringstream msg;
	msg << "PACKFIT array differ! left packed is " << packed << ", right is " << rptr.isTargetPacked() << " for target type: " << getUlamTypeNameByIndex(rptr.getPtrTargetType()).c_str();
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }

    if(WritePacked(packed))
      {
	UlamValue atval = getPtrTarget(rptr); // entire array in one slot

	// redo what getPtrTarget use to do, when types didn't match due to
	// an element/quark or a requested scalar of an arraytype
	if(atval.getUlamValueTypeIdx() != tuti)
	  {
	    UlamValue atvalUV = UlamValue::getPackedArrayDataFromAtom(rptr, atval, *this);
	    assignValue(lptr, atvalUV);
	  }
	else
	  assignValue(lptr, atval);
      }
    else
      {
	//assign each array element, packed or unpacked
	u32 arraysize = slotsNeeded(rptr.getPtrTargetType());

	UlamValue nextlptr = UlamValue::makeScalarPtr(lptr,*this);
	UlamValue nextrptr = UlamValue::makeScalarPtr(rptr,*this);
	tuti = nextrptr.getPtrTargetType(); // update type

	for(u32 i = 0; i < arraysize; i++)
	  {
	    UlamValue atval = getPtrTarget(nextrptr);

	    // redo what getPtrTarget use to do, when types didn't match due to
	    // an element/quark or a requested scalar of an arraytype
	    if(atval.getUlamValueTypeIdx() != tuti)
	      {
		UlamValue atvalUV = UlamValue::getPackedArrayDataFromAtom(rptr, atval, *this);
		assignValue(nextlptr, atvalUV);
	      }
	    else
	      assignValue(nextlptr, atval);

	    nextlptr.incrementPtr(*this);
	    nextrptr.incrementPtr(*this);
	  }
      }
  }


  //store pointer (rptr) as value to where lptr is pointing
  void CompilerState::assignValuePtr(UlamValue lptr, UlamValue rptr)
  {
    assert(lptr.getUlamValueTypeIdx() == Ptr);
    assert(rptr.getUlamValueTypeIdx() == Ptr);

    assert(lptr.getPtrTargetType() == rptr.getPtrTargetType());

    STORAGE place = lptr.getPtrStorage();
    switch(place)
      {
      case STACK:
	m_funcCallStack.assignUlamValuePtr(lptr, rptr);
	break;
      case EVALRETURN:
	m_nodeEvalStack.assignUlamValuePtr(lptr, rptr);
	break;
      case EVENTWINDOW:
	m_eventWindow.assignUlamValuePtr(lptr, rptr);
	break;
      default:
	assert(0);
      };
  }


  PACKFIT CompilerState::determinePackable(UTI aut)
  {
    return getUlamTypeByIndex(aut)->getPackable();
  }


  bool CompilerState::thisClassHasTheTestMethod()
  {
    Symbol * csym = m_programDefST.getSymbolPtr(m_compileThisId); //safer approach
    NodeBlockClass * classNode = ((SymbolClass *) csym)->getClassBlockNode();
    assert(classNode);
    NodeBlockFunctionDefinition * func = classNode->findTestFunctionNode();
    return (func != NULL);
  }


  bool CompilerState::thisClassIsAQuark()
  {
    Symbol * csym = m_programDefST.getSymbolPtr(m_compileThisId);
    UTI cuti = csym->getUlamTypeIdx();
    return(getUlamTypeByIndex(cuti)->getUlamClass() == UC_QUARK);
  }


  void CompilerState::setupCenterSiteForTesting()
  {
    // call again for code gen..
    // assert(m_currentObjPtr.getUlamValueTypeIdx() == Nav);
    // set up an atom in eventWindow; init m_currentObjPtr to point to it
    // set up stacks since func call not called
    Coord c0(0,0);

    //m_classBlock ok now, reset by NodeProgram after type label done
    //UTI cuti = m_state.m_classBlock->getNodeType();
    Symbol * csym = m_programDefST.getSymbolPtr(m_compileThisId); //safer approach
    UTI cuti = csym->getUlamTypeIdx();

    m_eventWindow.setSiteElementType(c0, cuti);
    m_currentSelfPtr = m_currentObjPtr = m_eventWindow.makePtrToCenter();

    // set up STACK since func call not called
    m_funcCallStack.pushArg(m_currentObjPtr);                        //hidden arg on STACK
    m_funcCallStack.pushArg(UlamValue::makeImmediate(Int, -1));      //return slot on STACK
  } //setupCenterSiteForTesting


  // used by SourceStream to build m_textByLinePerFilePath during parsing
  void CompilerState::appendNextLineOfText(Locator loc, std::string textstr)
  {
    std::vector<u32> * textOfLines = NULL;

    // get index for string of text in string pool; may exist, o.w. new
    u32 textid = m_pool.getIndexForDataString(textstr);

    u32 pathidx = loc.getPathIndex();
    u16 linenum = loc.getLineNo();

    // use path index in locator to access its vector of lines
    std::map<u32, std::vector<u32>*>::iterator it = m_textByLinePerFilePath.find(pathidx);
    if(it != m_textByLinePerFilePath.end())
      {
	textOfLines = it->second;
      }
    else
      {
	textOfLines = new std::vector<u32>();
	assert(textOfLines);
	m_textByLinePerFilePath.insert(std::pair<u32, std::vector<u32>*> (pathidx,textOfLines));
      }

    // may contain "empty" lines
    if(linenum > textOfLines->size())
      {
	// get index for string of text in string pool
	u32 blankid = m_pool.getIndexForDataString("\n");

	textOfLines->insert(textOfLines->end(), linenum - textOfLines->size(), blankid);
      }
    assert(linenum >= 0 && linenum <= textOfLines->size());
    textOfLines->push_back(textid);

    m_locOfNextLineText = loc;  //during parsing here (see NodeStatements)
  } //appendNextLineOfText


  std::string CompilerState::getLineOfText(Locator loc)
  {
    std::vector<u32> * textOfLines = NULL;

    u32 pathidx = loc.getPathIndex();
    u16 linenum = loc.getLineNo();

    // use path index in locator to access its vector of lines
    std::map<u32, std::vector<u32>*>::iterator it = m_textByLinePerFilePath.find(pathidx);
    if(it != m_textByLinePerFilePath.end())
      {
	textOfLines = it->second;
      }
    else
      {
	std::ostringstream msg;
	msg << "Cannot find path index (" << pathidx << ") for line " << linenum << ": " << m_pool.getDataAsString(pathidx).c_str();
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	return "<empty path>\n";
      }

    if(linenum >= textOfLines->size())
      {
	std::ostringstream txt;
	txt << "<empty line " << linenum << ">\n";
	return txt.str();
      }

    u32 textid = (*textOfLines)[linenum];
    if(textid == 0)
      {
	return "<no line>\n>";
      }

    return m_pool.getDataAsString(textid);
  } //getLineOfText


  std::string CompilerState::getLocationTextAsString(Locator nodeloc)
  {
    std::ostringstream txt;
    txt << getPathFromLocator(nodeloc).c_str();
    txt << ":";
    txt << nodeloc.getLineNo();
    txt << ": ";
    txt << getLineOfText(nodeloc).c_str();
    return txt.str();
  } //getTextAsString


  void CompilerState::outputTextAsComment(File * fp, Locator nodeloc)
  {
    fp->write("\n");
    indent(fp);
    fp->write("//! ");
    fp->write(getLocationTextAsString(nodeloc).c_str());
  } //outputTextAsComment


  s32 CompilerState::getNextTmpVarNumber()
  {
    return ++m_nextTmpVarNumber;
  }

  const std::string CompilerState::getTmpVarAsString(UTI uti, s32 num, STORAGE stg)
  {
    assert(uti != Void);

    std::ostringstream tmpVar;  // into
    PACKFIT packed = determinePackable(uti);

    if(uti == UAtom || getUlamTypeByIndex(uti)->getUlamClass() == UC_ELEMENT)
      {
	stg = TMPBITVAL; //avoid loading a T into a tmpregister!
      }

    if(stg == TMPREGISTER)
      {
	if(WritePacked(packed))
	  tmpVar << "Uh_tmpreg_loadable_" ;
	else
	  tmpVar << "Uh_tmpreg_unpacked_" ;
      }
    else if(stg == TMPBITVAL)
      {
	if(WritePacked(packed))
	  tmpVar << "Uh_tmpval_loadable_" ;
	else
	  tmpVar << "Uh_tmpval_unpacked_" ;
      }
    else
      assert(0); //remove assumptions about tmpbitval.

    tmpVar << DigitCount(num, BASE10) << num;

    return tmpVar.str();
  } //getTmpVarAsString


  const std::string CompilerState::getLabelNumAsString(s32 num)
  {
    std::ostringstream labelname;  // into
    labelname << "Ul_endcontrolloop_" << DigitCount(num, BASE10) << num;;
    return labelname.str();
  }


  void CompilerState::saveIdentTokenForConditionalAs(Token iTok)
    {
      m_identTokenForConditionalAs = iTok;
      m_parsingConditionalAs = true;    //cleared manually
    }


} //end MFM
