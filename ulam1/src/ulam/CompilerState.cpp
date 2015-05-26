#include <stdio.h>
#include <iostream>
#include "ClassContext.h"
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
#include "UlamTypeHolder.h"

namespace MFM {

  //#define _DEBUG_OUTPUT
  //#define _INFO_OUTPUT
  #define _WARN_OUTPUT

#ifdef _DEBUG_OUTPUT
  static const bool debugOn = true;
#else
  static const bool debugOn = false;
#endif

#ifdef _INFO_OUTPUT
  static const bool infoOn = true;
#else
  static const bool infoOn = false;
#endif

#ifdef _WARN_OUTPUT
  static const bool warnOn = true;
#else
  static const bool warnOn = false;
#endif

  static const char * m_indentedSpaceLevel("  "); //2 spaces per level

  static const char * HIDDEN_ARG_NAME = "Uv_4self";
  static const char * HIDDEN_CONTEXT_ARG_NAME = "uc"; //unmangled
  static const char * CUSTOMARRAY_GET_FUNC_NAME = "aref"; //unmangled
  static const char * CUSTOMARRAY_SET_FUNC_NAME = "aset"; //unmangled
  static const char * IS_MANGLED_FUNC_NAME = "internalCMethodImplementingIs"; //Uf_2is
  static const char * HAS_MANGLED_FUNC_NAME = "PositionOfDataMemberType"; //Uf_3has
  static const char * HAS_MANGLED_FUNC_NAME_FOR_ATOM = "UlamElement<EC>::PositionOfDataMember";

  //use of this in the initialization list seems to be okay;
  CompilerState::CompilerState(): m_programDefST(*this), m_currentFunctionBlockDeclSize(0), m_currentFunctionBlockMaxDepth(0), m_parsingControlLoop(0), m_parsingElementParameterVariable(false), m_parsingConditionalAs(false), m_genCodingConditionalAs(false), m_eventWindow(*this), m_goAgainResolveLoop(false), m_currentSelfSymbolForCodeGen(NULL), m_nextTmpVarNumber(0), m_nextNodeNumber(0)
  {
    m_err.init(this, debugOn, infoOn, warnOn, NULL);
  }

  CompilerState::~CompilerState()
  {
    clearAllDefinedUlamTypes();
    clearAllLinesOfText();
    m_currentObjSymbolsForCodeGen.clear();
  }

  void CompilerState::clearAllDefinedUlamTypes()
  {
    std::map<UlamKeyTypeSignature, UlamType *, less_than_key>::iterator it = m_definedUlamTypes.begin();
    while(it != m_definedUlamTypes.end())
      {
	UlamType * ut = it->second;
	delete ut;
	it->second = NULL;
	it++;
      }

    m_definedUlamTypes.clear();
    m_indexToUlamKey.clear();
    m_keyToAnyUTI.clear();

    m_currentFunctionReturnNodes.clear();

    m_unionRootUTI.clear(); //aliasUTIs

    m_unseenClasses.clear();
  } //clearAllDefinedUlamTypes()

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
  } //clearAllLinesOfText

  UTI CompilerState::makeUlamTypeHolder()
  {
    UTI uti = m_indexToUlamKey.size();  //next index based on key
    UlamKeyTypeSignature hkey = getUlamTypeByIndex(Holder)->getUlamKeyTypeSignature();

    incrementKeyToAnyUTICounter(hkey, uti);
    m_indexToUlamKey.push_back(hkey);
    initUTIAlias(uti);
    return uti;
  } //makeUlamTypeHolder

  UTI CompilerState::makeUlamTypeFromHolder(UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti)
  {
    //we need to keep the uti, but change the key
    UlamKeyTypeSignature hkey = getUlamTypeByIndex(uti)->getUlamKeyTypeSignature();
    return makeUlamTypeFromHolder(hkey, newkey, utype, uti);
  } //makeUlamTypeFromHolder

  UTI CompilerState::makeUlamTypeFromHolder(UlamKeyTypeSignature oldkey, UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti)
  {
    //we need to keep the uti, but change the key
    //removes old key and its ulamtype from map, if no longer pointed to
    deleteUlamKeyTypeSignature(oldkey, uti);

    UlamType * newut = NULL;
    UTI auti = Nav;
    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, utype);

	std::pair<std::map<UlamKeyTypeSignature, UlamType *, less_than_key>::iterator, bool> reti;
	reti = m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType*>(newkey,newut)); //map owns ut
	bool notdupi = reti.second; //false if already existed, i.e. not added
	if(!notdupi)
	  {
	    delete newut;
	    newut = NULL;
	  }
	auti = uti;
      }
    else
      {
	assert(anyDefinedUTI(newkey,auti)); //don't wipe out uti
      }

    incrementKeyToAnyUTICounter(newkey, uti);

    m_indexToUlamKey[uti] = newkey;

    updateUTIAlias(uti, auti); //after index-to-key is set

    {
      std::ostringstream msg;
      msg << "Replace Holder key (UTI" << uti << ") WITH: ";
      msg << newut->getUlamTypeName().c_str() << " (UTI" << auti << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }

    UlamType * ut = NULL;
    assert(isDefined(newkey, ut));

    return  uti; //return same uti (third arg)
  } //makeUlamTypeFromHolder

  //similar to addIncompleteClassSymbolToProgramTable, yet different!
  SymbolClassName * CompilerState::makeAnonymousClassFromHolder(UTI cuti, Locator cloc)
  {
    SymbolClassName * cnsym = NULL;
    //make an 'anonymous class' key, if doesn't exist already
    u32 id = m_pool.getIndexForNumberAsString(cuti);

    UlamKeyTypeSignature ackey(id, UNKNOWNSIZE, NONARRAYSIZE, cuti);
    UTI cuti2 = makeUlamTypeFromHolder(ackey, Class, cuti);
    assert(cuti2 == cuti); //keeps same uti

    if(!m_programDefST.isInTable(id, (Symbol *&) cnsym))
      {
	NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
	assert(classblock);
	classblock->setNodeLocation(cloc);
	classblock->setNodeType(cuti);

	Token cTok(TOK_IDENTIFIER, cloc, id);
	//symbol ownership goes to the programDefST;
	//distinguish between template and regular classes, where?
	cnsym = new SymbolClassName(cTok, cuti, classblock, *this);
	assert(cnsym);
	m_programDefST.addToTable(id, cnsym); //ignored post-parse
	//m_unseenClasses.insert(symptr); don't include as unseen; needs a name.
      }
    return cnsym;
  } //makeAnonymousClassFromHolder

  //convenience method (refactors code originally from installSymbol)
  //if exists, just returns it, o.w. makes it; trick to know the base ULAMTYPE
  UTI CompilerState::makeUlamType(Token typeTok, s32 bitsize, s32 arraysize, UTI classinstanceidx)
  {
    //type names begin with capital letter..and the rest can be either
    u32 typeNameId = getTokenAsATypeNameId(typeTok); //Foo, Int, etc

    //can't be a typedef!! get's the wrong name for type key; use key as arg
    UTI tmputi;
    UTI tmpforscalaruti;
    assert(!getUlamTypeByTypedefName(typeTok.m_dataindex, tmputi, tmpforscalaruti));

    //is this name already a typedef for a complex type?
    ULAMTYPE bUT = getBaseTypeFromToken(typeTok);
    if(bitsize == 0)
      bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

    UlamKeyTypeSignature key(typeNameId,bitsize,arraysize,classinstanceidx);

    UTI uti = Nav;
    UlamType * ut = NULL; //for isDefined.

    if(!isDefined(key,ut) || bitsize == UNKNOWNSIZE || arraysize == UNKNOWNSIZE)
      {
	//no key, make new type, how to know baseUT? bitsize?
	uti = makeUlamType(key,bUT); //returns uti
      }
    else
      assert(anyDefinedUTI(key,uti));

    return uti;
  } //makeUlamType

  UTI CompilerState::makeUlamType(UlamKeyTypeSignature key, ULAMTYPE utype)
  {
    UTI uti;
    UlamType * ut = NULL;

    if(!isDefined(key,ut) || utype == Class || (utype != Class && key.getUlamKeyTypeSignatureBitSize() == UNKNOWNSIZE) || key.getUlamKeyTypeSignatureArraySize() == UNKNOWNSIZE)
      {
	uti = m_indexToUlamKey.size();  //next index based on key
	if(utype == Class)
	  {
	    UTI suti = key.getUlamKeyTypeSignatureClassInstanceIdx();
	    if(key.getUlamKeyTypeSignatureArraySize() != NONARRAYSIZE) //array type
	      {
		//keep classinstanceid of scalar in key
		assert(suti > 0 && !isComplete(suti));
	      }
	    else
	      {
		if(suti == Nav)
		  //this is a new class! add uti to key
		  key.append(uti);
		else
		  {
		    //if this classInstanceIdx (suti) is a template with parameters
		    //then make a new uti; o.w. no need for a new uti, it's defined.
		    SymbolClassName * cnsym = NULL;
		    UlamType * sut = getUlamTypeByIndex(suti);
		    assert(alreadyDefinedSymbolClassName(sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), cnsym));
		    if(cnsym->isClassTemplate())
		      key.append(uti);
		    else
		      return suti; //nothing to be done.
		  }
	      }
	  }
	else //not a class
	  {
	    if(key.getUlamKeyTypeSignatureArraySize() != NONARRAYSIZE) //array type
	      {
		//can't save scalar in key; unable to look up from token
		//saveNonClassScalarUTIForArrayUTI = suti;
	      }
	    key.append(Nav); //clear
	  }

	ut = createUlamType(key, utype);
	m_indexToUlamKey.push_back(key);

	std::pair<std::map<UlamKeyTypeSignature, UlamType *, less_than_key>::iterator, bool> reti;
	reti = m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType*>(key,ut)); //map owns ut
	bool notdupi = reti.second; //false if already existed, i.e. not added
	if(!notdupi)
	  {
	    delete ut;
	    ut = NULL;
	  }

	incrementKeyToAnyUTICounter(key, uti);

	assert(isDefined(key, ut));

	initUTIAlias(uti);
      }
    else
      assert(anyDefinedUTI(key,uti));

    return uti;
  } //makeUlamType

  bool CompilerState::isDefined(UlamKeyTypeSignature key, UlamType *& foundUT)
  {
    bool rtnBool= false;
    std::map<UlamKeyTypeSignature, UlamType *, less_than_key>::iterator it = m_definedUlamTypes.find(key);

    if(it != m_definedUlamTypes.end())
      {
	assert(key == it->first);
	foundUT = it->second;
	rtnBool = true;
      }
    return rtnBool;
  } //isDefined

  bool CompilerState::anyDefinedUTI(UlamKeyTypeSignature key, UTI& foundUTI)
  {
    bool rtnBool= false;
    std::map<UlamKeyTypeSignature, std::set<UTI>, less_than_key>::iterator it = m_keyToAnyUTI.find(key);

    if(it != m_keyToAnyUTI.end())
      {
	assert(key == it->first);
	foundUTI = *(it->second.lower_bound(Nav));
	rtnBool = true;
      }
    return rtnBool;
  } //anyDefinedUTI

  UlamType * CompilerState::createUlamType(UlamKeyTypeSignature key, ULAMTYPE utype)
  {
    UlamType * ut = NULL;
    switch(utype)
      {
      case Nav:
	ut = new UlamTypeNav(key, *this);
	break;
      case Void:
	ut = new UlamTypeVoid(key, *this);
	break;
      case Int:
	ut = new UlamTypeInt(key, *this);
	break;
      case Unsigned:
	ut = new UlamTypeUnsigned(key, *this);
	break;
      case Bool:
	ut = new UlamTypeBool(key, *this);
	break;
      case Unary:
	ut = new UlamTypeUnary(key, *this);
	break;
      case Bits:
	ut = new UlamTypeBits(key, *this);
	break;
      case Class:
	ut = new UlamTypeClass(key, *this);
	break;
      case UAtom:
	ut = new UlamTypeAtom(key, *this);
	break;
      case Ptr:
	ut = new UlamTypePtr(key, *this);
	break;
      case Holder:
	ut = new UlamTypeHolder(key, *this);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Undefined ULAMTYPE base type <" << utype << ">" ;
	  MSG2("",msg.str().c_str(),DEBUG);
	  assert(0);
	}
      };
    assert(ut);
    return ut;
  } //createUlamType

  void CompilerState::incrementKeyToAnyUTICounter(UlamKeyTypeSignature key, UTI utarg)
  {
    std::map<UlamKeyTypeSignature, std::set<UTI>, less_than_key>::iterator it = m_keyToAnyUTI.find(key);
    if(it != m_keyToAnyUTI.end())
      {
	assert(key == it->first);
	it->second.insert(utarg);
      }
    else
      {
	std::set<UTI> aset;
	aset.insert(utarg);
	m_keyToAnyUTI.insert(std::pair<UlamKeyTypeSignature,std::set<UTI> >(key, aset));
      }
  } //incrementKeyToAnyUTICounter

  u32 CompilerState::decrementKeyToAnyUTICounter(UlamKeyTypeSignature key, UTI utarg)
  {
    std::map<UlamKeyTypeSignature, std::set<UTI>, less_than_key>::iterator it = m_keyToAnyUTI.find(key);
    u32 count = 0;
    if(it != m_keyToAnyUTI.end())
      {
	assert(key == it->first);
	std::set<UTI>::iterator sit = it->second.find(utarg);
	if(sit != it->second.end())
	  {
	    assert(utarg == *sit);
	    it->second.erase(sit); //decrements count
	  }
	count = it->second.size();
	if(count == 0)
	  m_keyToAnyUTI.erase(it);
      }
    return count;
  } //decrementKeyToAnyUTICounter

  //used to update Class' calculated bit size (setBitSize)
  bool CompilerState::deleteUlamKeyTypeSignature(UlamKeyTypeSignature key, UTI utarg)
  {
    bool rtnBool= false;
    if(decrementKeyToAnyUTICounter(key, utarg) == 0)
      {
	std::map<UlamKeyTypeSignature, UlamType *, less_than_key>::iterator it = m_definedUlamTypes.find(key);
	if(it != m_definedUlamTypes.end())
	  {
	    assert(key == it->first);
	    UlamType * ut = it->second;
	    delete ut;
	    it->second = NULL;
	    m_definedUlamTypes.erase(it);
	    rtnBool = true;
	  }
      }
    return rtnBool;
  } //deleteUlamKeyTypeSignature

  bool CompilerState::mappedIncompleteUTI(UTI cuti, UTI auti, UTI& mappedUTI)
  {
    SymbolClass * csym = NULL;
    assert(alreadyDefinedSymbolClass(cuti, csym));
    if(csym->hasMappedUTI(auti, mappedUTI))
      return true;

    // does this hurt anything?
    if(findaUTIAlias(auti, mappedUTI))
       return mappedUTI; //anonymous UTI

    //move this test after looking for the mapped class symbol type in "cuti" (always compileThis?)
    UlamType * aut = getUlamTypeByIndex(auti);
    ULAMTYPE bUT = aut->getUlamTypeEnum();
    if(bUT == Class)
      {
	UlamKeyTypeSignature akey = aut->getUlamKeyTypeSignature();
	SymbolClassName * cnsymOfIncomplete = NULL; //could be a different class than being compiled
	assert(alreadyDefinedSymbolClassName(akey.getUlamKeyTypeSignatureNameId(), cnsymOfIncomplete));
	UTI utiofinc = cnsymOfIncomplete->getUlamTypeIdx();
	if(utiofinc != cuti)
	  {
	      if(!cnsymOfIncomplete->isClassTemplate())
		return false;
	    return (cnsymOfIncomplete->hasMappedUTI(auti, mappedUTI));
	  }
      }
    return false; //for compiler
  } //mappedIncompleteUTI

  //called by Symbol's copy constructor with ref's 'incomplete' uti;
  //also called by NodeTypeDescriptor's copy constructor since has no symbol;
  //please set getCompileThisIdx() to the instance's UTI.
  UTI CompilerState::mapIncompleteUTIForCurrentClassInstance(UTI suti)
  {
    UlamType * sut = getUlamTypeByIndex(suti);
    if(sut->isComplete())
      return suti;

    SymbolClassNameTemplate * cnsym = NULL;
    assert(alreadyDefinedSymbolClassNameTemplate(getCompileThisId(), cnsym));
    UTI mappedUTI;
    if(cnsym->hasInstanceMappedUTI(getCompileThisIdx(), suti, mappedUTI))
      return mappedUTI;  //e.g. decl list

    if(findaUTIAlias(suti, mappedUTI))
       return mappedUTI; //anonymous UTI

    //move this test after looking for the mapped class symbol type
    ULAMTYPE bUT = sut->getUlamTypeEnum();
    UlamKeyTypeSignature skey = sut->getUlamKeyTypeSignature();
    SymbolClassName * cnsymOfIncomplete = NULL; //could be a different class than being compiled
    if(bUT == Class)
      {
	assert(alreadyDefinedSymbolClassName(skey.getUlamKeyTypeSignatureNameId(), cnsymOfIncomplete));
	if(!cnsymOfIncomplete->isClassTemplate())
	  return suti;
	if(!((SymbolClassNameTemplate *) cnsymOfIncomplete)->pendingClassArgumentsForStubClassInstance(suti))
	  return suti;
      }

    //first time we've seen this 'incomplete' UTI for this class instance (as fully instantiated):
    //get a new UTI and add to cnsym's map for this instance in case we see it again;
    //For classes with pending args, make a copy of the stub including its resolver with pending
    //args, so pending args can be resolved XXXX within the context of this class instance
    //(e.g. dependent on instances arg values which makes it different than others', like "self").
    //Context dependent pending args are resolved before they are added to the resolver's
    //pending args.
    UlamKeyTypeSignature newkey(skey); //default constructor makes copy
    UTI newuti = makeUlamType(newkey,bUT);
    cnsym->mapInstanceUTI(getCompileThisIdx(), suti, newuti);

    if(bUT == Class)
      {
	UlamType * ut = getUlamTypeByIndex(suti);
	ULAMCLASSTYPE classtype = ut->getUlamClass();
	bool isCustomArray = ut->isCustomArray();
	UTI caType = (isCustomArray ? ((UlamTypeClass *) ut)->getCustomArrayType() : Nav);
	UlamType * newut = getUlamTypeByIndex(newuti);
	((UlamTypeClass *) newut)->setUlamClass(classtype); //restore from original ut
	if(isCustomArray)
	  ((UlamTypeClass *) newut)->setCustomArrayType(caType);

	//potential for unending process..
	((SymbolClassNameTemplate *)cnsymOfIncomplete)->copyAStubClassInstance(suti, newuti, getCompileThisIdx());

	std::ostringstream msg;
	msg << "MAPPED!! type: " << getUlamTypeNameBriefByIndex(suti).c_str();
	msg << "(UTI" << suti << ")";
	msg << " TO newtype: " << getUlamTypeNameBriefByIndex(newuti).c_str();
	msg << "(UTI" << newuti << ")";
	msg << " while compiling class " << getUlamTypeNameBriefByIndex(getCompileThisIdx()).c_str();
	msg << "(UTI" << getCompileThisIdx() << ")";
	msg << ", for imcomplete class " << getUlamTypeNameBriefByIndex(cnsymOfIncomplete->getUlamTypeIdx()).c_str();
	msg << "(UTI" << cnsymOfIncomplete->getUlamTypeIdx() << ")";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }
      //updateUTIAlias(suti, newuti);
    return newuti;
  } //mapIncompleteUTIForCurrentClassInstance

#if 0
  void CompilerState::mapHolderTypesInCurrentClass(UTI fm, UTI to, Locator loc)
  {
    updateUTIAlias(fm, to);

    updateClassSymbolsFromHolder(fm, to, loc);

    mapTypesInCurrentClass(fm, to);
  } //mapHolderTypesInCurrentClass
#endif

  void CompilerState::mapTypesInCurrentClass(UTI fm, UTI to)
  {
    SymbolClassName * cnsym = NULL;
    assert(alreadyDefinedSymbolClassName(getCompileThisId(), cnsym));
    if(cnsym->isClassTemplate())
      ((SymbolClassNameTemplate *) cnsym)->mapInstanceUTI(getCompileThisIdx(), fm, to);
    else
      cnsym->mapUTItoUTI(fm,to);
  } //mapTypesInCurrentClass

  UlamType * CompilerState::getUlamTypeByIndex(UTI typidx)
  {
    UlamType * rtnUT = NULL;
    if(typidx >= m_indexToUlamKey.size())
      {
	std::ostringstream msg;
	msg << "Undefined UTI <" << typidx << "> Max is: "
	    << m_indexToUlamKey.size() << ", returning Nav INSTEAD";
	MSG2("", msg.str().c_str(),DEBUG);
	typidx = 0;
      }
    assert(isDefined(m_indexToUlamKey[typidx], rtnUT));
    return rtnUT;
  } //getUlamTypeByIndex

  const std::string CompilerState::getUlamTypeNameBriefByIndex(UTI uti)
  {
    UlamType * ut = NULL;
    assert(isDefined(m_indexToUlamKey[uti], ut));
    return ut->getUlamTypeNameBrief();
  }

  const std::string CompilerState::getUlamTypeNameByIndex(UTI uti)
  {
    UlamType * ut = NULL;
    assert(isDefined(m_indexToUlamKey[uti], ut));
    return ut->getUlamTypeName();
  }

  ULAMTYPE CompilerState::getBaseTypeFromToken(Token tok)
  {
    ULAMTYPE bUT = Nav;
    UTI ut = Nav;
    UTI tmpforscalaruti = Nav;
    //is this name already a typedef for a complex type?
    if(getUlamTypeByTypedefName(tok.m_dataindex, ut, tmpforscalaruti))
      {
	bUT = getUlamTypeByIndex(ut)->getUlamTypeEnum();
      }
    else if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
      {
	std::string typeName = getTokenAsATypeName(tok); //Int, etc

	//no way to get the bUT, except to assume typeName is one of them?
	bUT = UlamType::getEnumFromUlamTypeString(typeName.c_str()); //could be Element, etc.;
      }
    else
      {
	//it's an element or quark! base type is Class.
	bUT = Class;
      }
    return bUT;
  } //getBaseTypeFromToken

  UTI CompilerState::getUlamTypeFromToken(Token tok, s32 typebitsize, s32 arraysize)
  {
    UTI uti = Nav;
    UTI tmpforscalaruti = Nav;
    //is this name already a typedef for a complex type?
    if(!getUlamTypeByTypedefName(tok.m_dataindex, uti, tmpforscalaruti))
      {
	if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    uti = makeUlamType(tok, typebitsize, arraysize, Nav);
	  }
	else
	  {
	    //check for existing Class type
	    SymbolClassName * cnsym = NULL;
	    if(alreadyDefinedSymbolClassName(tok.m_dataindex, cnsym))
	      {
		uti = cnsym->getUlamTypeIdx();  //beware: may not match class parameters!!!
	      } //else or make one if doesn't exist yet, while parsing---do we do this anymore?
	  }
      }
    return uti;
  } //getUlamTypeFromToken

  UTI CompilerState::getUlamTypeFromToken(TypeArgs & args)
  {
    UTI uti = Nav;
    UTI tmpforscalaruti = Nav;
    //is this name already a typedef for a complex type?
    if(!getUlamTypeByTypedefName(args.m_typeTok.m_dataindex, uti, tmpforscalaruti))
      {
	if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    uti = makeUlamType(args.m_typeTok, args.m_bitsize, args.m_arraysize, Nav);
	  }
	else
	  {
	    //check for existing Class type
	    SymbolClassName * cnsym = NULL;
	    if(alreadyDefinedSymbolClassName(args.m_typeTok.m_dataindex, cnsym))
	      {
		uti = cnsym->getUlamTypeIdx();  //beware: may not match class parameters!!!
	      } //else  or make one if doesn't exist yet, while parsing --- do we do this anymore ???
	  }
      }
    else
      args.m_declListOrTypedefScalarType = tmpforscalaruti; //also returns scalar uti
    return uti;
  } //getUlamTypeFromToken

  //new version! uses indexes
  bool CompilerState::getUlamTypeByTypedefName(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType)
  {
    bool rtnBool = false;
    Symbol * asymptr = NULL;

    //e.g. KEYWORDS have no m_dataindex (=0); short-circuit
    if(nameIdx == 0) return false;

    //searched back through all block's ST for idx
    if(alreadyDefinedSymbol(nameIdx, asymptr))
      {
	if(asymptr->isTypedef())
	  {
	    rtnType = asymptr->getUlamTypeIdx();
	    rtnScalarType = ((SymbolTypedef *) asymptr)->getScalarUTI();
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

    //for typedef array, the scalar is the primitive type
    // maintained in the symbol!! can't get to it from utarg. XXX
    ULAMTYPE bUT = ut->getUlamTypeEnum();

    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();

    if(bUT == Class)
      {
	return keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx();
      }

    u32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    UlamKeyTypeSignature baseKey(keyOfArg.m_typeNameId, bitsize);  //default array size is zero

    UTI buti = makeUlamType(baseKey, bUT); //could be a new one, oops.
    return buti;
  } //getUlamTypeAsScalar

  UTI CompilerState::getUlamTypeOfConstant(ULAMTYPE etype)
  {
    u32 enumStrIdx = m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(etype));
    UlamKeyTypeSignature ckey(enumStrIdx, getDefaultBitSize((UTI) etype), NONARRAYSIZE); //was ANYBITSIZECONSTANT
    return makeUlamType(ckey, etype); //may not exist yet, create
  } //getUlamTypeOfConstant

  UTI CompilerState::getDefaultUlamTypeOfConstant(UTI ctype)
  {
    return ctype; // use its own type
  } //getDefaultUlamTypeOfConstant

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

  bool CompilerState::isComplete(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return ut->isComplete();
  } //isComplete

  bool CompilerState::isHolder(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return (ut->isHolder());
  }

  //updates key. we can do this now that UTI is used and the UlamType * isn't saved
  void CompilerState::setBitSize(UTI utArg, s32 bits)
  {
    return setUTISizes(utArg, bits, getArraySize(utArg)); //keep current arraysize
  }

  void CompilerState::setUTISizes(UTI utArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);

    if(ut->isComplete())
      return;

    //redirect primitives;
    ULAMCLASSTYPE classtype = ut->getUlamClass();
    if(!(classtype == UC_ELEMENT || classtype == UC_QUARK || classtype == UC_UNSEEN))
      {
	return setSizesOfNonClass(utArg, bitsize, arraysize);
      }

    //bitsize could be UNKNOWN or CONSTANT (negative)
    s32 total = bitsize * (arraysize > 0 ? arraysize : 1); //?

    bool isCustomArray = ut->isCustomArray();
    UTI caType = (isCustomArray ? ((UlamTypeClass *) ut)->getCustomArrayType() : Nav);

    //verify total bits is within limits for elements and quarks
    if(classtype == UC_ELEMENT)
      {
	if(total > MAXSTATEBITS)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXSTATEBITS << ") for element ";
	    msg << ut->getUlamTypeName().c_str() << " with " << total << " bits";
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return;
	  }
      }

    if(classtype == UC_QUARK)
      {
	if(total > MAXBITSPERQUARK)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXBITSPERQUARK << ") for quark ";
	    msg << ut->getUlamTypeName().c_str() << " with " << total << " bits";
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return;
	  }
      }

    //old key
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();

    //continue with valid number of bits for Class UlamTypes only
    UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key.getUlamKeyTypeSignatureNameId(), bitsize, arraysize);
    newkey.append(key.getUlamKeyTypeSignatureClassInstanceIdx());

    if(key == newkey)
      return;

    //removes old key and its ulamtype from map, if no longer pointed to
    deleteUlamKeyTypeSignature(key, utArg);

    UlamType * newut = NULL;
    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, Class);
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType *>(newkey,newut));
	((UlamTypeClass *) newut)->setUlamClass(classtype); //restore from original ut

	if(isCustomArray)
	  ((UlamTypeClass *) newut)->setCustomArrayType(caType);
      }

    m_indexToUlamKey[utArg] = newkey;

    incrementKeyToAnyUTICounter(newkey, utArg);  //here

    {
      std::ostringstream msg;
      msg << "Sizes SET for Class: " << newut->getUlamTypeName().c_str() << " (UTI" << utArg << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
  } //setUTISizes

  void CompilerState::mergeClassUTI(UTI olduti, UTI cuti)
  {
    UlamType * ut1 = getUlamTypeByIndex(olduti);
    UlamType * ut2 = getUlamTypeByIndex(cuti);
    assert(ut1 && ut2);
    UlamKeyTypeSignature key1 = ut1->getUlamKeyTypeSignature();
    UlamKeyTypeSignature key2 = ut2->getUlamKeyTypeSignature();

    //bitsize of old could still be "unknown" (before size set, but args known and match 'cuti').
    assert(key1.getUlamKeyTypeSignatureNameId() == key2.getUlamKeyTypeSignatureNameId());

    //removes old key and its ulamtype from map, if no longer pointed to
    deleteUlamKeyTypeSignature(key1, olduti);
    m_indexToUlamKey[olduti] = key2;
    incrementKeyToAnyUTICounter(key2, olduti);
    {
      std::ostringstream msg;
      msg << "MERGED keys for duplicate Class (UTI" << olduti << ") WITH: ";
      msg << ut2->getUlamTypeName().c_str() << " (UTI" << cuti << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
  } //mergeClassUTI

  bool CompilerState::isARootUTI(UTI auti)
  {
    UTI tmpalias;
    return !findaUTIAlias(auti, tmpalias);
  }

  bool CompilerState::findaUTIAlias(UTI auti, UTI& aliasuti)
  {
    assert(auti < m_unionRootUTI.size());
    UTI finduti = m_unionRootUTI[auti];
    if(finduti != auti)
      {
	aliasuti = finduti;
	return true;
      }
    return false;
  } //findAClassUTIAlias

  void CompilerState::updateUTIAlias(UTI auti, UTI buti)
  {
    assert(auti < m_unionRootUTI.size());
    assert(buti < m_unionRootUTI.size());
    m_unionRootUTI[auti] = buti;
    {
      std::ostringstream msg;
      msg << "ALIASES for (UTI" << auti << ") ";
      msg << getUlamTypeNameBriefByIndex(auti).c_str();
      msg << " is update to (UTI" << buti << ") ";
      msg << getUlamTypeNameBriefByIndex(buti).c_str();
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
    return;
  } //updateClassUTIAlias

  void CompilerState::initUTIAlias(UTI auti)
  {
    assert(auti == m_unionRootUTI.size());
    m_unionRootUTI.push_back(auti);
    {
      std::ostringstream msg;
      msg << "ALIASES for (UTI" << auti << ") ";
      msg << getUlamTypeNameBriefByIndex(auti).c_str();
      msg << " is initialized to self";
      MSG2("", msg.str().c_str(), DEBUG);
    }
    return;
  } //initUTIAlias

#if 0
  bool CompilerState::updateClassSymbolsFromHolder(UTI fm, UTI to, Locator loc)
  {
    bool rtn = false; //not a class
    UlamType * tut = getUlamTypeByIndex(to);
    if(tut->getUlamClass() != UC_NOTACLASS)
      {
	SymbolClass * tcsym = NULL;
	assert(alreadyDefinedSymbolClass(to, tcsym));

	UlamType * fut = getUlamTypeByIndex(fm);
	if(fut->getUlamClass() != UC_NOTACLASS)
	  {
	    SymbolClass * fcsym = NULL;
	    assert(alreadyDefinedSymbolClass(fm, fcsym));

	    NodeBlockClass * fclassblock = fcsym->getClassBlockNode();
	    NodeBlockClass * tclassblock = tcsym->getClassBlockNode();

	    assert(fclassblock && tclassblock);

	    rtn = fclassblock->mergeAllSymbolsFromScopeIntoTable(tclassblock);
	  }
	else
	  {
	    updateClassName(fm, tcsym->getId());
	  }
      }
    else
      {
	UlamType * fut = getUlamTypeByIndex(fm);
	if(fut->getUlamClass() != UC_NOTACLASS)
	  {
	    //change 'to' holder to a class, keeping its uti
	    makeAnonymousClassFromHolder(to, loc);
	  }
      }
    return rtn;
  } //updateClassSymbolsFromHolder
#endif

#if 0
  bool CompilerState::updateClassName(UTI cuti, u32 cname)
  {
    bool rtnb = false; // not a class
    u32 id = m_pool.getIndexForNumberAsString(cuti);
    SymbolClassName * cnsymId = NULL;
    if(alreadyDefinedSymbolClassName(id, cnsymId))
      {
	if(cname)
	  {
	    SymbolClassName * cnsym = NULL;
	    // if real name exists, we can't do the update; instead..
	    if(alreadyDefinedSymbolClassName(cname, cnsym))
	      {
		UlamKeyTypeSignature idkey = getUlamTypeByIndex(cuti)->getUlamKeyTypeSignature();
		UlamKeyTypeSignature newkey(cname, getBitSize(cuti), getArraySize(cuti), cuti);
		//change the key id only
		makeUlamTypeFromHolder(idkey, newkey, Class, cuti);
	      }
	    else
	      m_programDefST.replaceInTable(id, cname, cnsym);
	  }
	rtnb = true;
      }
    else
      {
	// anonymous name doesn't exist
	if(cname)
	  {
	    SymbolClassName * cnsym = NULL;
	    // if real name exists, we can update the cuti (Holder) to it
	    if(alreadyDefinedSymbolClassName(cname, cnsym))
	      {
		UlamKeyTypeSignature cnkey = getUlamTypeByIndex(cnsym->getUlamTypeIdx())->getUlamKeyTypeSignature();
		//change the key only, including the class idx to
		// point to the "real" one!
		makeUlamTypeFromHolder(cnkey, Class, cuti);
		rtnb = true;
	      }
	  }
      }
    return rtnb;
  } //updateClassName
#endif

  void CompilerState::setSizesOfNonClass(UTI utArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    ULAMTYPE bUT = ut->getUlamTypeEnum();
    ULAMCLASSTYPE classtype = ut->getUlamClass();

    assert(classtype == UC_NOTACLASS);

    if(ut->isComplete())
      return;  //nothing to do

    //disallow zero-sized primitives (no such thing as a BitVector<0u>)
    //'Void' by default is zero and only zero bitsize (already complete)
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
    if(key.getUlamKeyTypeSignatureBitSize() == 0 || bitsize == 0)
      {
	if(bUT != Void)
	  {
	    std::ostringstream msg;
	    msg << "Invalid zero sizes to set for nonClass: " << ut->getUlamTypeName().c_str();
	    msg << "> (UTI" << utArg << ")";
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return;
	  }
	//Void with zero bitsize
	if(arraysize != NONARRAYSIZE)
	  {
	    // disallow an array of Void(0)'s
	    std::ostringstream msg;
	    msg << "Invalid Void type array: " << ut->getUlamTypeName().c_str();
	    msg << "> (UTI" << utArg << ")";
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return;
	  }
      }
    else if(bUT == Void)
      {
	std::ostringstream msg;
	msg << "Invalid nonzero size for Void type: " << ut->getUlamTypeName().c_str();
	msg << "> (UTI" << utArg << ")";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	return;
      }

    //continue with valid number of bits
    UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key.getUlamKeyTypeSignatureNameId(), bitsize, arraysize);

    if(key == newkey)
      return;

    //remove old key from map, if no longer pointed to by any UTIs
    deleteUlamKeyTypeSignature(key, utArg);

    UlamType * newut = NULL;
    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, bUT);
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType*>(newkey,newut));
      }

    m_indexToUlamKey[utArg] = newkey;

    incrementKeyToAnyUTICounter(newkey, utArg);

    {
      std::ostringstream msg;
      msg << "Sizes set for nonClass: " << newut->getUlamTypeName().c_str();
      msg << " (UTI" << utArg << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }

  } //setSizesOfNonClass

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

  bool CompilerState::alreadyDefinedSymbolClassName(u32 dataindex, SymbolClassName * & symptr)
  {
    return m_programDefST.isInTable(dataindex,(Symbol * &) symptr);
  }

  bool CompilerState::alreadyDefinedSymbolClassNameTemplate(u32 dataindex, SymbolClassNameTemplate * & symptr)
  {
    bool rtnb = m_programDefST.isInTable(dataindex,(Symbol * &) symptr);
    if(rtnb && !symptr->isClassTemplate())
      {
	std::ostringstream msg;
	msg << "Class without parameters already exists with the same name: ";
	msg << m_pool.getDataAsString(symptr->getId()).c_str() << " <UTI";
	msg << symptr->getUlamTypeIdx() << ">";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
      }
    return (rtnb && symptr->isClassTemplate());
  } //alreadyDefinedSymbolClassNameTemplate

  //if necessary, searches for instance of class "template" with matching SCALAR uti
  bool CompilerState::alreadyDefinedSymbolClass(UTI uti, SymbolClass * & symptr)
  {
    bool rtnb = false;
    UlamType * ut = getUlamTypeByIndex(uti);
    UTI scalarUTI = uti;
    if(!ut->isScalar())
      scalarUTI = getUlamTypeAsScalar(uti);

    SymbolClassName * cnsym = NULL;
    if(alreadyDefinedSymbolClassName(ut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), cnsym))
      {
	//not a regular class, and not the template, so dig deeper for the stub
	if(cnsym->getUlamTypeIdx() != scalarUTI && cnsym->isClassTemplate())
	  {
	    SymbolClass * csym = NULL;
	    if(((SymbolClassNameTemplate *) cnsym)->findClassInstanceByUTI(scalarUTI, csym))
	      {
		symptr = csym;
		rtnb = true;
	      }
	    else
	      {
		UTI mappedUTI;
		if(findaUTIAlias(scalarUTI, mappedUTI))
		  {
		    if(((SymbolClassNameTemplate *) cnsym)->findClassInstanceByUTI(mappedUTI, csym))
		      {
			symptr = csym;
			rtnb = true;
		      }
		  }
	      }
	  }
	else
	  {
	    symptr = cnsym;
	    rtnb = true;
	  }
      }
    return rtnb;
  } //alreadyDefinedSymbolClass

  //temporary UlamType which will be updated during type labeling.
  void CompilerState::addIncompleteClassSymbolToProgramTable(Token cTok, SymbolClassName * & symptr)
  {
    u32 dataindex = cTok.m_dataindex;
    assert(!alreadyDefinedSymbolClassName(dataindex,symptr));

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE);  //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class);  //**gets next unknown uti type

    NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
    assert(classblock);
    classblock->setNodeLocation(cTok.m_locator);
    classblock->setNodeType(cuti);

    //symbol ownership goes to the programDefST; distinguish btn template & regular classes here:
    symptr = new SymbolClassName(cTok, cuti, classblock, *this);
    m_programDefST.addToTable(dataindex, symptr);
    m_unseenClasses.insert(symptr);
  } //addIncompleteClassSymbolToProgramTable

  //temporary UlamType which will be updated during type labeling.
  void CompilerState::addIncompleteClassSymbolToProgramTable(Token cTok, SymbolClassNameTemplate * & symptr)
  {
    u32 dataindex = cTok.m_dataindex;
    assert(!alreadyDefinedSymbolClassNameTemplate(dataindex,symptr));

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE); //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class); //**gets next unknown uti type

    NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
    assert(classblock);
    classblock->setNodeLocation(cTok.m_locator);
    classblock->setNodeType(cuti);

    //symbol ownership goes to the programDefST; distinguish btn template & regular classes here:
    symptr = new SymbolClassNameTemplate(cTok, cuti, classblock, *this);
    m_programDefST.addToTable(dataindex, symptr);
    m_unseenClasses.insert(symptr);
  } //addIncompleteClassSymbolToProgramTable

  void CompilerState::resetUnseenClass(SymbolClassName * cnsym, Token identTok)
  {
    if(m_unseenClasses.empty())
      return;

    std::set<SymbolClassName *>::iterator it = m_unseenClasses.find(cnsym);
    if(it != m_unseenClasses.end())
      {
	m_unseenClasses.erase(it);
      }
    cnsym->resetUnseenClassLocation(identTok);
  } //resetUnseenClass

  bool CompilerState::getUnseenClassFilenames(std::vector<std::string>& unseenFiles)
  {
    std::set<SymbolClassName *>::iterator it = m_unseenClasses.begin();
    while(it != m_unseenClasses.end())
      {
	SymbolClassName * cnsym = *it;
	UTI cuti = cnsym->getUlamTypeIdx();
	UlamType * cut = getUlamTypeByIndex(cuti);
	//excludes anonymous classes, only unseen classes with known names
	ULAMCLASSTYPE classtype = cut->getUlamClass();
	assert(classtype == UC_UNSEEN);
	{
	  std::ostringstream fn;
	  fn << m_pool.getDataAsString(cnsym->getId()).c_str() << ".ulam";
	  unseenFiles.push_back(fn.str());
	}
	it++;
      } //while
    return !unseenFiles.empty();
  } //getUnseenClassFilenames

  bool CompilerState::completeIncompleteClassSymbolForTypedef(UTI incomplete)
  {
    bool rtnB = false;
    SymbolClass * csym = NULL;
    UlamType * ict = getUlamTypeByIndex(incomplete);
    assert(ict->getUlamClass() == UC_UNSEEN); //missing
    if(alreadyDefinedSymbolClass(incomplete, csym))
      {
	SymbolClassName * cnsym = NULL;
	assert(alreadyDefinedSymbolClassName(csym->getId(), cnsym));
	UTI but = cnsym->getUlamTypeIdx();

	ULAMCLASSTYPE bc = getUlamTypeByIndex(but)->getUlamClass();
	//e.g. out-of-scope typedef is not a class, return false
	if(bc == UC_ELEMENT || bc == UC_QUARK)
	  {
	    ((UlamTypeClass *) ict)->setUlamClass(bc);

	    if(getBitSize(but) == UNKNOWNSIZE || getArraySize(but) == UNKNOWNSIZE)
	      {
		std::ostringstream msg;
		msg << "Class Instance for typedef: " << ict->getUlamTypeName().c_str();
		msg << "(UTI" << but << ") - sizes still unknown";
		MSG2(cnsym->getTokPtr(), msg.str().c_str(), DEBUG);
	      }
	    else
	      rtnB = true;
	  }
	else //else uc_incomplete
	  {
	    std::ostringstream msg;
	    msg << "Class Instance for typedef: " << ict->getUlamTypeName().c_str();
	    msg << "(UTI" << incomplete << ") - still UNSEEN";
	    MSG2(cnsym->getTokPtr(), msg.str().c_str(), DEBUG);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Class Instance for typedef: " << ict->getUlamTypeName().c_str();
	msg << "(UTI" << incomplete << ") - NOT YET DEFINED";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }
    return rtnB;
  } //completeIncompleteClassSymbolForTypedef

  bool CompilerState::alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;

    //start with the current "top" block and look down the stack
    //until the 'variable id' is found.
    NodeBlock * blockNode = getCurrentBlock();

    //substitute another selected class block to search for data member
    if(useMemberBlock())
      blockNode = getCurrentMemberClassBlock();

    while(!brtn && blockNode)
      {
	brtn = blockNode->isIdInScope(dataindex,symptr);
	blockNode = blockNode->getPreviousBlockPointer(); //traverse the chain
      }

    //data member variables in class block; function symbols are linked to their
    //function def block; check function data members separately.
    if(!brtn)
      brtn = isFuncIdInClassScope(dataindex, symptr);

    return brtn;
  } //alreadyDefinedSymbol

  bool CompilerState::isFuncIdInClassScope(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;
    if(useMemberBlock())
      {
	NodeBlockClass * memberblock = getCurrentMemberClassBlock();
	if(memberblock)
	  brtn = memberblock->isFuncIdInScope(dataindex,symptr);
      }
    else
      {
	NodeBlockClass * classblock = getClassBlock();
	if(classblock)
	  brtn = classblock->isFuncIdInScope(dataindex,symptr);
      }
    return brtn;
  } //isFuncIdInClassScope

  bool CompilerState::isIdInClassScope(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;
    if(useMemberBlock())
      {
	NodeBlockClass * memberblock = getCurrentMemberClassBlock();
	if(memberblock)
	  brtn = memberblock->isIdInScope(dataindex,symptr);
      }
    else
      brtn = getClassBlock()->isIdInScope(dataindex,symptr);

    return brtn;
  } //isIdInClassScope

  //symbol ownership goes to the current block (end of vector)
  void CompilerState::addSymbolToCurrentScope(Symbol * symptr)
  {
    getCurrentBlock()->addIdToScope(symptr->getId(), symptr);
  }

  //symbol ownership goes to the member block (end of vector)
  // making stuff up!
  void CompilerState::addSymbolToCurrentMemberClassScope(Symbol * symptr)
  {
    getCurrentMemberClassBlock()->addIdToScope(symptr->getId(), symptr);
  }

  //symbol ownership goes to the current block (end of vector);
  //symbol is same, just id changed
  void CompilerState::replaceSymbolInCurrentScope(u32 oldid, Symbol * symptr)
  {
    getCurrentBlock()->replaceIdInScope(oldid, symptr->getId(), symptr);
  }

  //deletes the oldsym, id's must be identical
  void CompilerState::replaceSymbolInCurrentScope(Symbol * oldsym, Symbol * newsym)
  {
    getCurrentBlock()->replaceIdInScope(oldsym, newsym);
  }

  //symbol ownership goes to the caller;
  bool CompilerState::takeSymbolFromCurrentScope(u32 id, Symbol *& rtnsymptr)
  {
    return getCurrentBlock()->removeIdFromScope(id, rtnsymptr);
  }

  //Token to location as string:
  const std::string CompilerState::getTokenLocationAsString(Token * tok)
  {
    if(!tok)
      return std::string("");
    return getFullLocationAsString(tok->m_locator);
  }

  /** combines path, line, and byte number for error messages */
  const std::string CompilerState::getFullLocationAsString(const Locator& loc)
  {
    std::ostringstream ss;
    ss << getFullPathFromLocator(loc) << ":" << loc.getLineNo() << ":" << loc.getByteNo() << ":";
    return ss.str();
  }

  const std::string CompilerState::getPathFromLocator(const Locator& loc)
  {
    if(loc.getPathIndex() > 0)
      return m_pool.getDataAsString(loc.getPathIndex());
    return "";
  }

  const std::string CompilerState::getFullPathFromLocator(const Locator& loc)
  {
    if(loc.getFullPathIndex() > 0)
      return m_pool.getDataAsString(loc.getFullPathIndex());
    return "";
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
    mangled << ToLeximited(nstr);
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
	    UTI tmpforscalaruti = Nav;
	    if(getUlamTypeByTypedefName(tok.m_dataindex, tduti, tmpforscalaruti))
	      {
		UlamType * tdut = getUlamTypeByIndex(tduti);
		//for typedef quarks return quark name, o.w. base name
		return tdut->getUlamTypeNameOnly();
	      }
	    else
	      return getTokenDataAsString(&tok); //a class
	  }
      }
    return "Nav";
  } //getTokenAsATypeName

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
	    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str();
	    msg << "''s Return Statement is missing; Return type: ";
	    msg << getUlamTypeNameByIndex(it).c_str();
	    MSG2(fsym->getTokPtr(), msg.str().c_str(), ERR);
	    return false;
	  }
	return true; //okay to skip return statement for void function
      }

    for(u32 i = 0; i < m_currentFunctionReturnNodes.size(); i++)
      {
	NodeReturnStatement * rNode = m_currentFunctionReturnNodes.at(i);
	UTI rType = rNode->getNodeType();
	if(!isComplete(rType))
	  {
	    std::ostringstream msg;
	    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str();
	    msg << "''s Return type's: " << getUlamTypeNameByIndex(it).c_str();
	    msg << " does not match incomplete resulting type";
	    msg << " " << getUlamTypeNameBriefByIndex(rType).c_str();
	    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_DEBUG);
	    continue;
	  }

	if(UlamType::compare(rType, it, *this) != UTIC_SAME)
	  {
	    rtnBool = false;
	    ULAMTYPE rBUT = getUlamTypeByIndex(rType)->getUlamTypeEnum();
	    ULAMTYPE itBUT = getUlamTypeByIndex(it)->getUlamTypeEnum();
	    if(rBUT != itBUT)
	      {
		std::ostringstream msg;
		msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str();
		msg << "''s Return type's: " << getUlamTypeNameByIndex(it).c_str();
		msg << " base type: <" << UlamType::getUlamTypeEnumAsString(itBUT);
		msg << ">, does not match resulting type's ";
		msg << getUlamTypeNameByIndex(rType).c_str() << " base type: <";
		msg << UlamType::getUlamTypeEnumAsString(rBUT) << ">";

		if(fsym->getId() == m_pool.getIndexForDataString("toInt") && it == Int)
		  m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_DEBUG);
		else
		  m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_WARN); //ERR?
	      }
	    else
	      {
		if(getArraySize(rType) != getArraySize(it))
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str();
		    msg << "''s Return type's: " << getUlamTypeNameByIndex(it).c_str();
		    msg << " array size: <" << getArraySize(it);
		    msg << "> does not match resulting type's: ";
		    msg << getUlamTypeNameByIndex(rType).c_str() << " array size: <";
		    msg << getArraySize(rType) << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_ERR);
		  }

		if(getBitSize(rType) != getBitSize(it))
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str();
		    msg << "''s Return type's: " << getUlamTypeNameByIndex(it).c_str();
		    msg << " bit size: <" << getBitSize(it);
		    msg << "> does not match resulting type's: ";
		    msg << getUlamTypeNameByIndex(rType).c_str() << " bit size: <";
		    msg << getBitSize(rType) << ">";
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
      fp->write(m_indentedSpaceLevel);
  } //indent

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
  } //getAsMangledFunctionName

  std::string CompilerState::getFileNameForAClassHeader(UTI cuti, bool wSubDir)
  {
    std::ostringstream f;
    if(wSubDir)
      f << "include/";

    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str() << ".h";
    return f.str();
  } //getFileNameForAClassHeader

  std::string CompilerState::getFileNameForThisClassHeader(bool wSubDir)
  {
    return getFileNameForAClassHeader(getCompileThisIdx(), wSubDir);
  }

  std::string CompilerState::getFileNameForThisClassBody(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "include/";
    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str() << ".tcc";
    return f.str();
  } //getFileNameForThisClassBody

  std::string CompilerState::getFileNameForThisClassBodyNative(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "include/";

    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str() << "_native.tcc";
    return f.str();
  } //getFileNameForThisClassBodyNative

  std::string CompilerState::getFileNameForThisClassCPP(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "src/";
    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str() << ".cpp";
    return f.str();
  } //getFileNameForThisClassCPP

  std::string CompilerState::getFileNameForThisTypesHeader(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "include/";
    f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str() << "_Types.h";
    return f.str();
  } //getFileNameForThisTypesHeader

  //separate file for element compilations, avoid multiple mains, select the one to test during linking
  std::string CompilerState::getFileNameForThisClassMain(bool wSubDir)
  {
    std::ostringstream f;
    UTI cuti = getUlamTypeForThisClass();
    if(wSubDir)
      f << "src/";

    SymbolClassName * cnsym = NULL;
    assert(alreadyDefinedSymbolClassName(getCompileThisId(), cnsym));
    if(cnsym->isClassTemplate())
      {
	u32 numParams = ((SymbolClassNameTemplate *) cnsym)->getNumberOfParameters();
	f << getUlamTypeByIndex(cuti)->getUlamTypeUPrefix().c_str();
	f << getDataAsStringMangled(getCompileThisId()).c_str();
	f << ToLeximitedNumber(numParams) << "_main.cpp";
      }
    else
      f << getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str() << "_main.cpp";
    return f.str();
  } //getFileNameForThisClassMain

  ULAMCLASSTYPE CompilerState::getUlamClassForThisClass()
  {
    UTI cuti = getUlamTypeForThisClass();
    return getUlamTypeByIndex(cuti)->getUlamClass();
  } //getUlamClassForThisClass

  UTI CompilerState::getUlamTypeForThisClass()
  {
    return getCompileThisIdx();
  } //getUlamTypeForThisClass

  const std::string CompilerState::getBitSizeTemplateString(UTI uti)
  {
    ULAMCLASSTYPE classtype = getUlamTypeByIndex(uti)->getUlamClass();
    assert(classtype == UC_QUARK || classtype == UC_ELEMENT);

    std::ostringstream mangled;
    if(classtype == UC_QUARK)
      {
	mangled << "<" << getTotalBitSize(uti) << ">"; //?
      }
    return mangled.str();
  } //getBitSizeTemplateString

  //unfortunately, the uti did not reveal a Class symbol;
  //already down to primitive types for casting.
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
	if(alreadyDefinedSymbolClass(uti, csym))
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
	      assert(0); //error!! neither quark nor element
	  }
	else
	    assert(0); //error!! no class symbol for this type
      }
    return lenstr.str();
  } //getBitVectorLengthAsStringForCodeGen

  UlamValue CompilerState::getPtrTarget(UlamValue ptr)
  {
    assert(ptr.getUlamValueTypeIdx() == Ptr);
    //slot + storage
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
	assert(0); //error!
      };
    return valAtIdx; //return as-is
  } //getPtrTarget

  //general purpose store
  void CompilerState::assignValue(UlamValue lptr, UlamValue ruv)
  {
    assert(lptr.getUlamValueTypeIdx() == Ptr);

    //handle UAtom assignment as a singleton (not array values)
    if(ruv.getUlamValueTypeIdx() == Ptr && (ruv.getPtrTargetType() != UAtom || lptr.getPtrTargetType() != UAtom))
      return assignArrayValues(lptr, ruv);

#if 0
    if(ruv.getUlamValueTypeIdx() == Ptr)
      {
	if(getArraySize(ruv.getPtrTargetType()) != NONARRAYSIZE)
	  return assignArrayValues(lptr, ruv);
	else if(ruv.getPtrTargetType() != UAtom || lptr.getPtrTargetType() != UAtom)
	  return;// assignValuePtr(lptr, ruv);
      }
#endif

    //r is data (includes packed arrays), store it into where lptr is pointing
    assert(UlamType::compare(lptr.getPtrTargetType(), ruv.getUlamValueTypeIdx(), *this) == UTIC_SAME || lptr.getPtrTargetType() == UAtom || ruv.getUlamValueTypeIdx() == UAtom);

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
  } //assignValue

  void CompilerState::assignArrayValues(UlamValue lptr, UlamValue rptr)
  {
    assert(lptr.getUlamValueTypeIdx() == Ptr);
    assert(rptr.getUlamValueTypeIdx() == Ptr);

    //assert types..the same, and arrays
    assert(UlamType::compare(lptr.getPtrTargetType(), rptr.getPtrTargetType(), *this) == UTIC_SAME);
    UTI tuti = rptr.getPtrTargetType();

    //unless we're copying from different storage classes, or an element
    //assert(!isScalar(lptr.getPtrTargetType()));

    //assigned packed or unpacked
    PACKFIT packed = lptr.isTargetPacked();
    if(packed != rptr.isTargetPacked())
      {
	std::ostringstream msg;
	msg << "PACKFIT array differ! left packed is " << packed << ", right is ";
	msg << rptr.isTargetPacked() << " for target type: ";
	msg << getUlamTypeNameByIndex(rptr.getPtrTargetType()).c_str();
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }

    if(WritePacked(packed))
      {
	UlamValue atval = getPtrTarget(rptr); //entire array in one slot

	//redo what getPtrTarget use to do, when types didn't match due to
	//an element/quark or a requested scalar of an arraytype
	if(UlamType::compare(atval.getUlamValueTypeIdx(), tuti, *this) != UTIC_SAME)
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
	tuti = nextrptr.getPtrTargetType(); //update type

	for(u32 i = 0; i < arraysize; i++)
	  {
	    UlamValue atval = getPtrTarget(nextrptr);

	    //redo what getPtrTarget use to do, when types didn't match due to
	    //an element/quark or a requested scalar of an arraytype
	    if(UlamType::compare(atval.getUlamValueTypeIdx(), tuti, *this) != UTIC_SAME)
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
  } //assignArrayValues

  //store pointer (rptr) as value to where lptr is pointing
  void CompilerState::assignValuePtr(UlamValue lptr, UlamValue rptr)
  {
    assert(lptr.getUlamValueTypeIdx() == Ptr);
    assert(rptr.getUlamValueTypeIdx() == Ptr);

    assert(UlamType::compare(lptr.getPtrTargetType(), rptr.getPtrTargetType(), *this) == UTIC_SAME);

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
  } //assignValuePtr

  PACKFIT CompilerState::determinePackable(UTI aut)
  {
    return getUlamTypeByIndex(aut)->getPackable();
  }

  bool CompilerState::thisClassHasTheTestMethod()
  {
    Symbol * csym = m_programDefST.getSymbolPtr(getCompileThisId()); //safer approach
    NodeBlockClass * classNode = ((SymbolClass *) csym)->getClassBlockNode();
    assert(classNode);
    NodeBlockFunctionDefinition * func = classNode->findTestFunctionNode();
    return (func != NULL);
  } //thisClassHasTheTestMethod

  bool CompilerState::thisClassIsAQuark()
  {
    Symbol * csym = m_programDefST.getSymbolPtr(getCompileThisId());
    UTI cuti = csym->getUlamTypeIdx();
    return(getUlamTypeByIndex(cuti)->getUlamClass() == UC_QUARK);
  } //thisClassIsAQuark

  void CompilerState::setupCenterSiteForTesting()
  {
    //call again for code gen..
    //set up an atom in eventWindow; init m_currentObjPtr to point to it
    //set up stacks since func call not called
    Coord c0(0,0);

    //m_classBlock ok now, reset by NodeProgram after type label done
    Symbol * csym = m_programDefST.getSymbolPtr(getCompileThisId()); //safer approach
    UTI cuti = csym->getUlamTypeIdx();

    m_eventWindow.setSiteElementType(c0, cuti);
    m_currentSelfPtr = m_currentObjPtr = m_eventWindow.makePtrToCenter();

    //set up STACK since func call not called
    m_funcCallStack.pushArg(m_currentObjPtr); //hidden arg on STACK
    m_funcCallStack.pushArg(UlamValue::makeImmediate(Int, -1)); //return slot on STACK
  } //setupCenterSiteForTesting

  //used by SourceStream to build m_textByLinePerFilePath during parsing
  void CompilerState::appendNextLineOfText(Locator loc, std::string textstr)
  {
    std::vector<u32> * textOfLines = NULL;

    //get index for string of text in string pool; may exist, o.w. new
    u32 textid = m_pool.getIndexForDataString(textstr);

    u32 pathidx = loc.getPathIndex();
    u16 linenum = loc.getLineNo();

    //use path index in locator to access its vector of lines
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

    //may contain "empty" lines
    if(linenum > textOfLines->size())
      {
	//get index for string of text in string pool
	u32 blankid = m_pool.getIndexForDataString("\n");

	textOfLines->insert(textOfLines->end(), linenum - textOfLines->size(), blankid);
      }
    assert(linenum >= 0 && linenum <= textOfLines->size());
    textOfLines->push_back(textid);

    m_locOfNextLineText = loc; //during parsing here (see NodeStatements)
  } //appendNextLineOfText

  std::string CompilerState::getLineOfText(Locator loc)
  {
    std::vector<u32> * textOfLines = NULL;

    u32 pathidx = loc.getPathIndex();
    u16 linenum = loc.getLineNo();

    //use path index in locator to access its vector of lines
    std::map<u32, std::vector<u32>*>::iterator it = m_textByLinePerFilePath.find(pathidx);
    if(it != m_textByLinePerFilePath.end())
      {
	textOfLines = it->second;
      }
    else
      {
	std::ostringstream msg;
	msg << "Cannot find path index (" << pathidx << ") for line: " << linenum;
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

    std::ostringstream tmpVar; //into
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

    tmpVar << ToLeximitedNumber(num);

    return tmpVar.str();
  } //getTmpVarAsString

  const std::string CompilerState::getLabelNumAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Ul_endcontrolloop_" << ToLeximitedNumber(num);
    return labelname.str();
  } //getLabelNumAsString

  void CompilerState::saveIdentTokenForConditionalAs(Token iTok)
  {
    m_identTokenForConditionalAs = iTok;
    m_parsingConditionalAs = true; //cleared manually
  } //saveIdentTokenForConditionalAs

  NNO CompilerState::getNextNodeNo()
  {
    return ++m_nextNodeNumber; //first one is 1
  }

  Node * CompilerState::findNodeNoInThisClass(NNO n)
  {
    if(useMemberBlock())
      {
	UTI mbuti = getCurrentMemberClassBlock()->getNodeType();
	u32 mbid = getUlamTypeByIndex(mbuti)->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId();
	SymbolClassName * cnsym = NULL;
	assert(alreadyDefinedSymbolClassName(mbid, cnsym));
	return cnsym->findNodeNoInAClassInstance(mbuti, n);
      }

    NodeBlock * currBlock = getCurrentBlock();
    if(currBlock && currBlock->getNodeNo() == n && getClassBlock()->getNodeType() == getCompileThisIdx())
      return currBlock; //avoid chix-n-egg with functiondefs

    SymbolClassName * cnsym = NULL;
    assert(alreadyDefinedSymbolClassName(getCompileThisId(), cnsym));
    return cnsym->findNodeNoInAClassInstance(getCompileThisIdx(), n);
  } //findNodeNoInThisClass

  u32 CompilerState::getCompileThisId()
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    return cc.getCompileThisId();
  }

  UTI CompilerState::getCompileThisIdx()
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    return cc.getCompileThisIdx();
  }

  NodeBlock * CompilerState::getCurrentBlock()
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    return cc.getCurrentBlock();
  } //getCurrentBlock

  NNO CompilerState::getCurrentBlockNo()
  {
    ClassContext cc;
    if(m_classContextStack.getCurrentClassContext(cc) && getCurrentBlock())
      return getCurrentBlock()->getNodeNo();
    return 0; //genesis of class symbol
  } //getCurrentBlockNo

  NodeBlockClass * CompilerState::getClassBlock()
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    return cc.getClassBlock();
  }

  NNO CompilerState::getClassBlockNo()
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    if(cc.getClassBlock())
      return cc.getClassBlock()->getNodeNo();
    return 0;
  }

  bool CompilerState::useMemberBlock()
  {
    ClassContext cc;
    if(m_classContextStack.getCurrentClassContext(cc))
      return cc.useMemberBlock();
    return false; //genesis of a symbol getting current block no
  } //useMemberBlock

  NodeBlockClass * CompilerState::getCurrentMemberClassBlock()
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    return cc.getCurrentMemberClassBlock();
  }

  void CompilerState::pushClassContext(UTI idx, NodeBlock * currblock, NodeBlockClass * classblock, bool usemember, NodeBlockClass * memberblock)
  {
    u32 id = getUlamTypeByIndex(idx)->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId();
    ClassContext cc(id, idx, currblock, classblock, usemember, memberblock); //new
    m_classContextStack.pushClassContext(cc);
  }

  void CompilerState::popClassContext()
  {
    m_classContextStack.popClassContext();
  }

  void CompilerState::pushCurrentBlock(NodeBlock * currblock)
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    cc.setCurrentBlock(currblock); //could be NULL
    m_classContextStack.pushClassContext(cc);
  }

  void CompilerState::pushCurrentBlockAndDontUseMemberBlock(NodeBlock * currblock)
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    cc.setCurrentBlock(currblock);
    cc.useMemberBlock(false);
    m_classContextStack.pushClassContext(cc);
  }

  void CompilerState::pushClassContextUsingMemberClassBlock(NodeBlockClass * memberblock)
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    cc.setCurrentMemberClassBlock(memberblock);
    cc.useMemberBlock(true);
    m_classContextStack.pushClassContext(cc);
  }

  std::string CompilerState::getClassContextAsStringForDebugging()
  {
    ClassContext cc;
    assert(m_classContextStack.getCurrentClassContext(cc));
    return cc.getClassContextAsString();
  }

  void CompilerState::clearGoAgain()
  {
    m_goAgainResolveLoop = false;
  }

  void CompilerState::setGoAgain()
  {
    m_goAgainResolveLoop = true;
  }

  bool CompilerState::goAgain()
  {
    return m_goAgainResolveLoop;
  }

} //end MFM
