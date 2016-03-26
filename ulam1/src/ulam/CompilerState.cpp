#include <stdio.h>
#include <iostream>
#include "ClassContext.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "SymbolFunctionName.h"
#include "SymbolTable.h"
#include "SymbolTypedef.h"
#include "SymbolVariable.h"
#include "UlamTypeAtom.h"
#include "UlamTypeBits.h"
#include "UlamTypeBool.h"
#include "UlamTypeHolder.h"
#include "UlamTypeHzy.h"
#include "UlamTypeInt.h"
#include "UlamTypeNav.h"
#include "UlamTypeNouti.h"
#include "UlamTypePtr.h"
#include "UlamTypeUnary.h"
#include "UlamTypeUnsigned.h"
#include "UlamTypeVoid.h"

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

  static const char * HIDDEN_ARG_NAME = "ur"; //was Uv_4self, then Uv_4atom
  static const char * HIDDEN_CONTEXT_ARG_NAME = "uc"; //unmangled
  //  static const char * AUTO_HIDDEN_CONTEXT_ARG_NAME = "uc_"; //unmangled, plus its mangled var
  //static const char * TMP_FOR_AUTO_HIDDEN_CONTEXT_ARG_NAME = "Uh_4tluc";
  static const char * CUSTOMARRAY_GET_FUNC_NAME = "aref"; //unmangled
  static const char * CUSTOMARRAY_SET_FUNC_NAME = "aset"; //unmangled
  static const char * CUSTOMARRAY_GET_MANGLEDNAME = "Uf_4aref";
  static const char * CUSTOMARRAY_SET_MANGLEDNAME = "Uf_4aset";

  static const char * IS_MANGLED_FUNC_NAME = "internalCMethodImplementingIs"; //Uf_2is
  static const char * IS_MANGLED_FUNC_NAME_FOR_ATOM = "UlamClass<EC>::IsMethod"; //Uf_2is

  static const char * HAS_MANGLED_FUNC_NAME = "PositionOfDataMemberType"; //Uf_3has
  static const char * HAS_MANGLED_FUNC_NAME_FOR_ATOM = "UlamClass<EC>::PositionOfDataMember";

  static const char * BUILD_DEFAULT_ATOM_FUNCNAME = "BuildDefaultAtom";
  static const char * BUILD_DEFAULT_QUARK_FUNCNAME = "getDefaultQuark";

  //use of this in the initialization list seems to be okay;
  CompilerState::CompilerState(): m_programDefST(*this), m_currentFunctionBlockDeclSize(0), m_currentFunctionBlockMaxDepth(0), m_parsingControlLoop(0), m_gotStructuredCommentToken(false), m_parsingConditionalAs(false), m_genCodingConditionalHas(false), m_eventWindow(*this), m_goAgainResolveLoop(false), m_pendingArgStubContext(0), m_currentSelfSymbolForCodeGen(NULL), m_nextTmpVarNumber(0), m_nextNodeNumber(0)
  {
    m_err.init(this, debugOn, infoOn, warnOn, NULL);
    Token::initTokenMap(*this);
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

  bool CompilerState::getClassNameFromFileName(std::string startstr, u32& compileThisId)
  {
    u32 foundSuffix = startstr.find(".ulam");
    if(foundSuffix == std::string::npos        //.ulam not found
       || foundSuffix != startstr.length()-5   //ensure it's a suffix
       || foundSuffix == 0)                    //and not also a prefix
      {
	std::ostringstream msg;
        msg << "File name <" << startstr << "> doesn't end with '.ulam'";
	MSG2("",msg.str().c_str() , ERR);
	return false;
      }

    std::string compileThis = startstr.substr(0,foundSuffix);

    char c = compileThis.at(0);
    if(!Token::isUpper(c))
      {
	std::ostringstream msg;
	msg << "File name <" << startstr;
	msg << "> must match a valid class name (uppercase) to compile";
	MSG2("", msg.str().c_str() , ERR);
	return  false;
      }

    compileThisId = m_pool.getIndexForDataString(compileThis);
    return true;
  } //getClassNameFromFileName

  UTI CompilerState::makeUlamTypeHolder()
  {
    //note: only anonymous class holders use uti as nameid
    UTI uti = m_indexToUlamKey.size();  //next index based on key
    UlamKeyTypeSignature hkey = getUlamTypeByIndex(Holder)->getUlamKeyTypeSignature();

    incrementKeyToAnyUTICounter(hkey, uti);
    m_indexToUlamKey.push_back(hkey);
    initUTIAlias(uti); //remove if using makeulamtype
    assert(isARootUTI(uti));
    return uti;
  } //makeUlamTypeHolder

  UTI CompilerState::makeUlamTypeFromHolder(UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti)
  {
    //we need to keep the uti, but change the key; utype to be.
    UlamKeyTypeSignature hkey = getUlamKeyTypeSignatureByIndex(uti);
    return makeUlamTypeFromHolder(hkey, newkey, utype, uti);
  } //makeUlamTypeFromHolder

  UTI CompilerState::makeUlamTypeFromHolder(UlamKeyTypeSignature oldkey, UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti)
  {
    if((getUlamTypeByIndex(uti)->getUlamTypeEnum() == Class) && isScalar(uti))
      {
	if((utype != Class))
	  {
	    //trans-basic type from "Class" to Non-class
	    //we need to also remove the name id from program ST
	    u32 oldnameid = oldkey.getUlamKeyTypeSignatureNameId();
	    Symbol * tmpsym = NULL;
	    AssertBool isGone = m_programDefST.removeFromTable(oldnameid, tmpsym);
	    assert(isGone);
	    delete tmpsym;
	  }
	else
	  {
	    //let the new class name be represented instead of the old (e.g. typedef)
	    u32 oldnameid = oldkey.getUlamKeyTypeSignatureNameId();
	    SymbolClassName * cnsym = NULL;
	    AssertBool isDefined = alreadyDefinedSymbolClassName(oldnameid, cnsym);
	    assert(isDefined);
	    u32 newnameid = newkey.getUlamKeyTypeSignatureNameId();

	    SymbolClassName * newcnsym = NULL;
	    if(!alreadyDefinedSymbolClassName(newnameid, newcnsym))
	      {
		cnsym->setId(newnameid);
		m_programDefST.replaceInTable(oldnameid, newnameid, cnsym);
	      }
	    else
	      {
		Symbol * rmcnsym  = NULL;
		AssertBool isGone = m_programDefST.removeFromTable(oldnameid, rmcnsym);
		assert(isGone);
		assert(rmcnsym == cnsym);
		delete rmcnsym;
		rmcnsym = cnsym = NULL;
	      }
	  }
      }

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
	AssertBool isDefined = anyDefinedUTI(newkey,auti); //don't wipe out uti
	assert(isDefined);
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
    AssertBool isDef = isDefined(newkey, ut);
    assert(isDef);

    return  uti; //return same uti (third arg)
  } //makeUlamTypeFromHolder

  //extends use of addIncompleteClassSymbolToProgramTable, if seen before, but
  // didn't know it was a class; before resorting to AnonymousClass that may
  // neer get resolved.
  SymbolClassName * CompilerState::makeClassFromHolder(UTI huti, Token tok)
  {
    SymbolClassName * cnsym = NULL;
    if(hasUnknownTypeInThisClassResolver(huti))
      {
	Token holdTok = removeKnownTypeTokenFromThisClassResolver(huti);
	AssertBool isDefined = addIncompleteClassSymbolToProgramTable(holdTok, cnsym);
	assert(isDefined);
	//clean up existing holder
	UTI cuti = cnsym->getUlamTypeIdx();
	cleanupExistingHolder(huti, cuti);
      }
    else
      cnsym = makeAnonymousClassFromHolder(huti, tok.m_locator);
    return cnsym;
  } //makeClassFromHolder

  void CompilerState::cleanupExistingHolder(UTI huti, UTI newuti)
  {
    UlamType * newut = getUlamTypeByIndex(newuti);
    UlamKeyTypeSignature newkey = newut->getUlamKeyTypeSignature();
    ULAMTYPE newetyp = newut->getUlamTypeEnum();
    makeUlamTypeFromHolder(newkey, newetyp, huti);
    mapTypesInCurrentClass(huti, newuti);
    updateUTIAliasForced(huti, newuti);
  }

  //similar to addIncompleteClassSymbolToProgramTable, yet different!
  SymbolClassName * CompilerState::makeAnonymousClassFromHolder(UTI cuti, Locator cloc)
  {
    SymbolClassName * cnsym = NULL;
    //make an 'anonymous class' key, if doesn't exist already
    u32 id = m_pool.getIndexForNumberAsString(cuti);

    UlamKeyTypeSignature ackey(id, UNKNOWNSIZE, NONARRAYSIZE, cuti);
    AssertBool isCutie = (makeUlamTypeFromHolder(ackey, Class, cuti) == cuti);
    assert(isCutie); //keeps same uti

    if(!m_programDefST.isInTable(id, (Symbol *&) cnsym))
      {
	NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
	assert(classblock);
	classblock->setNodeLocation(cloc);
	classblock->setNodeType(cuti);

	Token cTok(TOK_IDENTIFIER, cloc, id);
	//symbol ownership goes to the programDefST;
	//distinguish between template and regular classes, where?
	//assert(cnsym == NULL); //leak?
	cnsym = new SymbolClassName(cTok, cuti, classblock, *this);
	assert(cnsym);
	m_programDefST.addToTable(id, cnsym); //ignored post-parse
	//m_unseenClasses.insert(symptr); don't include as unseen; needs a name.
      }
    return cnsym;
  } //makeAnonymousClassFromHolder

  //convenience method (refactors code originally from installSymbol)
  //if exists, just returns it, o.w. makes it; trick to know the base ULAMTYPE
  UTI CompilerState::makeUlamType(Token typeTok, s32 bitsize, s32 arraysize, UTI classinstanceidx, ALT reftype)
  {
    //type names begin with capital letter..and the rest can be either
    u32 typeNameId = getTokenAsATypeNameId(typeTok); //Foo, Int, etc

    //can't be a typedef!! get's the wrong name for type key; use key as arg
    UTI tmputi = Nav;
    UTI tmpforscalaruti = Nouti;
    AssertBool isDef = getUlamTypeByTypedefName(typeTok.m_dataindex, tmputi, tmpforscalaruti);
    assert(!isDef);

    //is this name already a typedef for a complex type?
    ULAMTYPE bUT = getBaseTypeFromToken(typeTok);
    if(bitsize == 0)
      bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

    UlamKeyTypeSignature key(typeNameId, bitsize, arraysize, classinstanceidx, reftype);

    UTI uti = Nav;
    UlamType * ut = NULL; //for isDefined.

    if(!isDefined(key,ut) || bitsize == UNKNOWNSIZE || arraysize == UNKNOWNSIZE)
      {
	//no key, make new type, how to know baseUT? bitsize?
	uti = makeUlamType(key, bUT); //returns uti
      }
    else
      {
	AssertBool isDefined = anyDefinedUTI(key,uti);
	assert(isDefined);
      }
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
		//assert(suti > 0 && !isComplete(suti)); e.g.ALT_REF for array of 2 quarks for _Types.h
	      }
	    else if(key.getUlamKeyTypeSignatureReferenceType() != ALT_NOT) //array type
	      {
		//need a new UTI for reference
	      }
	    else
	      {
		if(suti == Nouti)
		  //this is a new class! add uti to key
		  key.append(uti);
		else
		  {
		    //if this classInstanceIdx (suti) is a template with parameters
		    //then make a new uti; o.w. no need for a new uti, it's defined.
		    SymbolClassName * cnsym = NULL;
		    AssertBool isDefined = alreadyDefinedSymbolClassName(getUlamKeyTypeSignatureByIndex(suti).getUlamKeyTypeSignatureNameId(), cnsym);
		    assert(isDefined);
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
	    key.append(Nouti); //clear
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

	AssertBool isDef = isDefined(key, ut);
	assert(isDef);

	initUTIAlias(uti);
      }
    else
      {
	AssertBool isDefined = anyDefinedUTI(key,uti);
	assert(isDefined);
      }
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
	foundUTI = *(it->second.lower_bound(Nouti));
	rtnBool = true;
      }
    return rtnBool;
  } //anyDefinedUTI

  UlamType * CompilerState::createUlamType(UlamKeyTypeSignature key, ULAMTYPE utype)
  {
    UlamType * ut = NULL;
    switch(utype)
      {
      case Nouti:
	ut = new UlamTypeNouti(key, *this);
	break;
      case Nav:
	ut = new UlamTypeNav(key, *this);
	break;
      case Hzy:
	ut = new UlamTypeHzy(key, *this);
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

  // no side-effects, except to 3rd arg when return is true.
  bool CompilerState::mappedIncompleteUTI(UTI cuti, UTI auti, UTI& mappedUTI)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    if(csym->hasMappedUTI(auti, mappedUTI))
      return true;

    // does this hurt anything?
    if(findaUTIAlias(auti, mappedUTI))
       return true; //anonymous UTI

    //or this? (works for template instances too)
    if(statusUnknownTypeInThisClassResolver(auti))
    {
      mappedUTI = auti;
      return true;
    }

    if(!isHolder(auti))
      {
	UlamType * aut = getUlamTypeByIndex(auti);
	UlamKeyTypeSignature akey = getUlamKeyTypeSignatureByIndex(auti);
	u32 anameid = akey.getUlamKeyTypeSignatureNameId();

	//if(aut->isReference()) not a special case

	//move this test after looking for the mapped class symbol type in "cuti" (always compileThis?)
	ULAMTYPE bUT = aut->getUlamTypeEnum();
	if(bUT == Class)
	  {
	    SymbolClassName * cnsymOfIncomplete = NULL; //could be a different class than being compiled
	    AssertBool isDefined = alreadyDefinedSymbolClassName(anameid, cnsymOfIncomplete);
	    assert(isDefined);
	    UTI utiofinc = cnsymOfIncomplete->getUlamTypeIdx();
	    if(utiofinc != cuti)
	      {
		if(!cnsymOfIncomplete->isClassTemplate())
		  return false;
		return (cnsymOfIncomplete->hasMappedUTI(auti, mappedUTI));
	      }
	  }
	else
	  {
	    Symbol * symOfIncomplete = NULL;
	    bool tmphzykin = false;

	    if(alreadyDefinedSymbol(anameid, symOfIncomplete, tmphzykin))
	      {
		mappedUTI = symOfIncomplete->getUlamTypeIdx();
		return true;
	      }
	  }
      } //not a holder

    //recursively check ancestors, for mapped uti's
    UTI superuti = isClassASubclass(cuti);
    if(okUTItoContinue(superuti))
      {
	UlamKeyTypeSignature supkey = getUlamKeyTypeSignatureByIndex(superuti);

	SymbolClassName * supcsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClassName(supkey.getUlamKeyTypeSignatureNameId(), supcsym);
	assert(isDefined);

	pushClassContext(superuti, supcsym->getClassBlockNode(), supcsym->getClassBlockNode(), false, NULL);
	bool kinhadit = mappedIncompleteUTI(superuti, auti, mappedUTI);

	popClassContext(); //restore

	if(kinhadit)
	  return true;
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

    if(!okUTItoContinue(suti))
      return suti; //forgiving; e.g. may be used for unset referencedUTI

    SymbolClassNameTemplate * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassNameTemplate(getCompileThisId(), cnsym);
    assert(isDefined);

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
	AssertBool isDefined = alreadyDefinedSymbolClassName(skey.getUlamKeyTypeSignatureNameId(), cnsymOfIncomplete);
	assert(isDefined);

	if(!cnsymOfIncomplete->isClassTemplate())
	  return suti;

	ALT salt = getReferenceType(suti);
	if(salt != ALT_NOT)
	  {
	    UTI asref = mapIncompleteUTIForCurrentClassInstance(getUlamTypeAsDeref(suti));
	    return getUlamTypeAsRef(asref, salt);
	  }

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
    UTI newuti = makeUlamType(newkey, bUT);
    cnsym->mapInstanceUTI(getCompileThisIdx(), suti, newuti);

    if(bUT == Class)
      {
	UlamType * ut = getUlamTypeByIndex(suti);
	ULAMCLASSTYPE classtype = ut->getUlamClass();
	UlamType * newut = getUlamTypeByIndex(newuti);
	((UlamTypeClass *) newut)->setUlamClass(classtype); //restore from original ut
	if(ut->isCustomArray())
	  ((UlamTypeClass *) newut)->setCustomArray();

	//potential for unending process..
	((SymbolClassNameTemplate *)cnsymOfIncomplete)->copyAStubClassInstance(suti, newuti, getCompileThisIdx());

	std::ostringstream msg;
	msg << "MAPPED!! type: " << getUlamTypeNameBriefByIndex(suti).c_str();
	msg << "(UTI" << suti << ")";
	msg << " TO newtype: " << getUlamTypeNameBriefByIndex(newuti).c_str();
	msg << "(UTI" << newuti << ")";
	msg << " while compiling class " << getUlamTypeNameBriefByIndex(getCompileThisIdx()).c_str();
	msg << "(UTI" << getCompileThisIdx() << ")";
	msg << ", for incomplete class " << getUlamTypeNameBriefByIndex(cnsymOfIncomplete->getUlamTypeIdx()).c_str();
	msg << "(UTI" << cnsymOfIncomplete->getUlamTypeIdx() << ")";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);

	if(getCompileThisId() != skey.getUlamKeyTypeSignatureNameId())
	  {
	    //e.g. inheritance
	    ((SymbolClassNameTemplate *)cnsymOfIncomplete)->mergeClassInstancesFromTEMP(); //not mid-iteration!! makes alreadydefined.
	  }
      }
      //updateUTIAlias(suti, newuti);
    return newuti;
  } //mapIncompleteUTIForCurrentClassInstance

  void CompilerState::mapTypesInCurrentClass(UTI fm, UTI to)
  {
    if(getReferenceType(to) != ALT_NOT)
      return; //only a reference to a class type, skip

    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassName(getCompileThisId(), cnsym);
    assert(isDefined);
    if(cnsym->isClassTemplate())
      ((SymbolClassNameTemplate *) cnsym)->mapInstanceUTI(getCompileThisIdx(), fm, to);
    else
      cnsym->mapUTItoUTI(fm,to);
  } //mapTypesInCurrentClass

  UlamKeyTypeSignature CompilerState::getUlamKeyTypeSignatureByIndex(UTI typidx)
  {
    assert(typidx < m_indexToUlamKey.size());
    return m_indexToUlamKey[typidx];
  }

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
    AssertBool isDef = isDefined(m_indexToUlamKey[typidx], rtnUT);
    assert(isDef);
    return rtnUT;
  } //getUlamTypeByIndex

  const std::string CompilerState::getUlamTypeNameBriefByIndex(UTI uti)
  {
    UlamType * ut = NULL;
    AssertBool isDef = isDefined(m_indexToUlamKey[uti], ut);
    assert(isDef);
    return ut->getUlamTypeNameBrief();
  } //getUlamTypeNameBriefByIndex

  const std::string CompilerState::getUlamTypeNameByIndex(UTI uti)
  {
    UlamType * ut = NULL;
    AssertBool isDef = isDefined(m_indexToUlamKey[uti], ut);
    assert(isDef);
    return ut->getUlamTypeName();
  }

  const std::string CompilerState::getEffectiveSelfMangledNameByIndex(UTI uti)
  {
    UTI esuti = uti;
    if(isReference(uti))
      esuti = getUlamTypeAsDeref(uti);

    UlamType * esut = NULL;
    AssertBool isDef = isDefined(m_indexToUlamKey[esuti], esut);
    assert(isDef);

    assert(esut->getUlamTypeEnum() == Class);
    std::ostringstream esmangled;
    esmangled << esut->getUlamTypeMangledName().c_str();
    esmangled << "<EC>::THE_INSTANCE";
    return esmangled.str();
  } //getEffectiveSelfMangledNameByIndex

  ULAMTYPE CompilerState::getBaseTypeFromToken(Token tok)
  {
    ULAMTYPE bUT = Nav;
    UTI uti = Nav;
    UTI tmpforscalaruti = Nouti;
    //is this name already a typedef for a complex type?
    if(getUlamTypeByTypedefName(tok.m_dataindex, uti, tmpforscalaruti))
      bUT = getUlamTypeByIndex(uti)->getUlamTypeEnum();
    else if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
      {
	std::string typeName = getTokenAsATypeName(tok); //Int, etc
	//no way to get the bUT, except to assume typeName is one of them?
	bUT = UlamType::getEnumFromUlamTypeString(typeName.c_str()); //could be Element, etc.;
      }
    else
      {
	//check for existing Class type
	SymbolClassName * cnsym = NULL;
	if(alreadyDefinedSymbolClassName(tok.m_dataindex, cnsym))
	  {
	    bUT = Class;
	  } //else or make one if doesn't exist yet, while parsing---do we do this anymore?
	else
	  bUT = Hzy; //it's an element or quark or yet unseen typedef! base type is Hzy (was assumed to be Class).
      }
    return bUT;
  } //getBaseTypeFromToken

  UTI CompilerState::getUlamTypeFromToken(Token tok, s32 typebitsize, s32 arraysize)
  {
    UTI uti = Nav;
    UTI tmpforscalaruti = Nouti;
    //is this name already a typedef for a complex type?
    if(!getUlamTypeByTypedefName(tok.m_dataindex, uti, tmpforscalaruti))
      {
	if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    uti = makeUlamType(tok, typebitsize, arraysize, Nouti);
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
    UTI tmpforscalaruti = Nouti;
    //is this name already a typedef for a complex type?
    if(!getUlamTypeByTypedefName(args.m_typeTok.m_dataindex, uti, tmpforscalaruti))
      {
	if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    uti = makeUlamType(args.m_typeTok, args.m_bitsize, args.m_arraysize, Nouti);
	  }
	else
	  {
	    //check for existing Class type
	    SymbolClassName * cnsym = NULL;
	    if(alreadyDefinedSymbolClassName(args.m_typeTok.m_dataindex, cnsym))
	      {
		uti = cnsym->getUlamTypeIdx();  //beware: may not match class parameters!!!
	      }
	    //else make one if doesn't exist yet, while parsing --- do we do this anymore ???
	  }
      }
    else
      args.m_declListOrTypedefScalarType = tmpforscalaruti; //also returns scalar uti

    args.m_bitsize = getBitSize(uti);
    return uti;
  } //getUlamTypeFromToken

  bool CompilerState::getUlamTypeByTypedefName(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType)
  {
    bool rtnBool = false;
    Symbol * asymptr = NULL;
    bool hazyKin = false; //useful?

    //e.g. KEYWORDS have no m_dataindex (=0); short-circuit
    if(nameIdx == 0) return false;

    //searched back through all block's ST for idx
    if(alreadyDefinedSymbol(nameIdx, asymptr, hazyKin))
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
    // maintained in the symbol!! can't get to it from utarg.
    ULAMTYPE bUT = ut->getUlamTypeEnum();

    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();
    UTI cuti = keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx(); // what-if a ref?
    u32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    u32 nameid = keyOfArg.getUlamKeyTypeSignatureNameId();
    UlamKeyTypeSignature baseKey(nameid, bitsize, NONARRAYSIZE, cuti, ALT_ARRAYITEM);  //default array size is NONARRAYSIZE, new reftype

    UTI buti = makeUlamType(baseKey, bUT); //could be a new one, oops.

    //note: array of CA's is not a CA too
    return buti;
  } //getUlamTypeAsScalar

  UTI CompilerState::getUlamTypeAsDeref(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    if(!ut->isReference())
      return utArg;

    ULAMTYPE bUT = ut->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();
    u32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    u32 arraysize = keyOfArg.getUlamKeyTypeSignatureArraySize();
    UTI classidx = keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx();

    if((bUT == Class) && ut->isScalar())
      return classidx; //scalar

    UlamKeyTypeSignature baseKey(keyOfArg.m_typeNameId, bitsize, arraysize, classidx, ALT_NOT);  //default array size is zero

    UTI buti = makeUlamType(baseKey, bUT); //could be a new one, oops.
    if(ut->isCustomArray())
      {
	assert(bUT == Class);
	((UlamTypeClass *) getUlamTypeByIndex(buti))->setCustomArray();
      }

    return buti;
  } //getUlamTypeAsDeref

  UTI CompilerState::getUlamTypeAsRef(UTI utArg)
  {
    return getUlamTypeAsRef(utArg, ALT_REF);
  } //getUlamTypeAsRef

  UTI CompilerState::getUlamTypeAsRef(UTI utArg, ALT altArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    ALT utalt = ut->getReferenceType();

#if 0
    if(utalt != ALT_NOT)
      return utArg //fails tests!!!
#else
    if(utalt == altArg)
      return utArg; //same ref type

    //e.g. a ref typedef
    if((utalt != ALT_NOT) && (altArg == ALT_NOT))
      return utArg; //deref used to remove alt type

    if((utalt != ALT_NOT) && (utalt != altArg))
      {
	std::ostringstream msg;
	msg << "Attempting to ref (" << altArg << ") a reference type <" ;
	msg <<  getUlamTypeNameByIndex(utArg) << ">";
	MSG2("", msg.str().c_str(), DEBUG);
	//assert(0); //didn't hit during testing
	//continue..
      }
#endif

    ULAMTYPE bUT = ut->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();

    u32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    u32 arraysize = keyOfArg.getUlamKeyTypeSignatureArraySize();
    UTI classidx = keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx();
    u32 nameid = keyOfArg.m_typeNameId;
    UlamKeyTypeSignature baseKey(nameid, bitsize, arraysize, classidx, altArg);  //default array size is zero

    //for now, a reference to a custom array is not itself a custom array.
    UTI buti = makeUlamType(baseKey, bUT); //could be a new one, oops.
    return buti;
  } //getUlamTypeAsRef

  ULAMTYPECOMPARERESULTS CompilerState::isARefTypeOfUlamType(UTI refuti, UTI ofuti)
  {
    //either may be a ref of the other; uses checked both directions.
    return UlamType::compare(getUlamTypeAsDeref(refuti), getUlamTypeAsDeref(ofuti), *this);
  } //isARefTypeOfUlamType

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

  bool CompilerState::getDefaultQuark(UTI cuti, u32& dqref)
  {
    if(!okUTItoContinue(cuti)) return false; //short-circuit

    bool rtnb = false;
    if(getUlamTypeByIndex(cuti)->getUlamClass() == UC_QUARK)
      {
	rtnb = true;
	if(getBitSize(cuti) > 0)
	  {
	    UTI scalarquark = getUlamTypeAsScalar(cuti);
	    SymbolClass * csym = NULL;
	    AssertBool isDefined = alreadyDefinedSymbolClass(scalarquark, csym);
	    assert(isDefined);
	    rtnb = csym->getDefaultQuark(dqref);
	  }
      }
    return rtnb;
  } //getDefaultQuark

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

  ALT CompilerState::getReferenceType(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return ut->getReferenceType();
  }

  bool CompilerState::isReference(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return ut->isReference();
  }

  bool CompilerState::isComplete(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return ut->isComplete();
  } //isComplete

  bool CompilerState::completeAReferenceType(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    assert(ut->isReference());

    if(ut->isComplete())
      return true;

    UTI derefuti = getUlamTypeAsDeref(utArg);
    UlamType * derefut = getUlamTypeByIndex(derefuti);

    if(!derefut->isComplete())
      return false;

    return completeAReferenceTypeWith(utArg, derefuti);
  } //completeAReferenceType

  bool CompilerState::completeAReferenceTypeWith(UTI utArg, UTI derefuti)
  {
    UlamType * derefut = getUlamTypeByIndex(derefuti);

    if(!derefut->isComplete())
      return false;

    if(isHolder(utArg))
      {
	UlamKeyTypeSignature dekey = derefut->getUlamKeyTypeSignature();
	UlamKeyTypeSignature newkey(dekey.getUlamKeyTypeSignatureNameId(), dekey.getUlamKeyTypeSignatureBitSize(), dekey.getUlamKeyTypeSignatureArraySize(), dekey.getUlamKeyTypeSignatureClassInstanceIdx(), ALT_REF);
	makeUlamTypeFromHolder(newkey, derefut->getUlamTypeEnum(), utArg);
	return true;
      }

    return setUTISizes(utArg, derefut->getBitSize(), derefut->getArraySize());
  } //completeAReferenceTypeWith

  bool CompilerState::isHolder(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return (ut->isHolder());
  }

  //updates key. we can do this now that UTI is used and the UlamType * isn't saved;
  // return false if ERR
  bool CompilerState::setBitSize(UTI utArg, s32 bits)
  {
    return setUTISizes(utArg, bits, getArraySize(utArg)); //keep current arraysize
  }

  // return false if ERR
  bool CompilerState::setUTISizes(UTI utArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);

    if(ut->isComplete())
      return true;

    if(!okUTItoContinue(utArg))
      return false;

    //redirect primitives;
    ULAMCLASSTYPE classtype = ut->getUlamClass();
    if(!(classtype == UC_ELEMENT || classtype == UC_QUARK || classtype == UC_UNSEEN))
      {
	return setSizesOfNonClass(utArg, bitsize, arraysize);
      }

    if(!ut->isScalar())
      return setSizesOfNonClass(utArg, bitsize, arraysize); //arrays of classes like non-class

    //bitsize could be UNKNOWN or CONSTANT (negative)
    s32 total = bitsize * (arraysize > 0 ? arraysize : 1); //?
    bool isCustomArray = ut->isCustomArray();

    //verify total bits is within limits for elements and quarks
    if(classtype == UC_ELEMENT)
      {
	if(total > MAXSTATEBITS)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXSTATEBITS << ") for element ";
	    msg << ut->getUlamTypeNameBrief().c_str() << " with " << total << " bits";
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
      }

    if(classtype == UC_QUARK)
      {
	if(total > MAXBITSPERQUARK)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXBITSPERQUARK << ") for quark ";
	    msg << ut->getUlamTypeNameBrief().c_str() << " with " << total << " bits";
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
      }

    //old key
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();

    //continue with valid number of bits for Class UlamTypes only
    UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key.getUlamKeyTypeSignatureNameId(), bitsize, arraysize, key.getUlamKeyTypeSignatureClassInstanceIdx(), key.getUlamKeyTypeSignatureReferenceType());

    if(key == newkey)
      return true;

    //removes old key and its ulamtype from map, if no longer pointed to
    deleteUlamKeyTypeSignature(key, utArg);

    UlamType * newut = NULL;
    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, Class);
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType *>(newkey,newut));
	((UlamTypeClass *) newut)->setUlamClass(classtype); //restore from original ut

	if(isCustomArray)
	  ((UlamTypeClass *) newut)->setCustomArray();
      }

    m_indexToUlamKey[utArg] = newkey;

    incrementKeyToAnyUTICounter(newkey, utArg); //here

    {
      std::ostringstream msg;
      msg << "Sizes SET for Class: " << newut->getUlamTypeName().c_str() << " (UTI" << utArg << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
    return true;
  } //setUTISizes

  void CompilerState::mergeClassUTI(UTI olduti, UTI cuti)
  {
    UlamKeyTypeSignature key1 = getUlamKeyTypeSignatureByIndex(olduti);
    UlamKeyTypeSignature key2 = getUlamKeyTypeSignatureByIndex(cuti);

    //bitsize of old could still be "unknown" (before size set, but args known and match 'cuti').
    assert(key1.getUlamKeyTypeSignatureNameId() == key2.getUlamKeyTypeSignatureNameId());

    if(key1.getUlamKeyTypeSignatureReferenceType() != key2.getUlamKeyTypeSignatureReferenceType())
      return; //don't destory olduti if either is a reference of the other

    //removes old key and its ulamtype from map, if no longer pointed to
    deleteUlamKeyTypeSignature(key1, olduti);
    m_indexToUlamKey[olduti] = key2;
    incrementKeyToAnyUTICounter(key2, olduti);
    {
      std::ostringstream msg;
      msg << "MERGED keys for duplicate Class (UTI" << olduti << ") WITH: ";
      msg << getUlamTypeNameBriefByIndex(cuti).c_str() << " (UTI" << cuti << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
  } //mergeClassUTI

  //(unused)
  void CompilerState::rekeyToReferenceUTI(ALT autoreftype, UTI auti)
  {
    UlamType * aut = getUlamTypeByIndex(auti);

    UlamKeyTypeSignature key1 = aut->getUlamKeyTypeSignature();
    UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key1.getUlamKeyTypeSignatureNameId(), key1.getUlamKeyTypeSignatureBitSize(), key1.getUlamKeyTypeSignatureArraySize(), key1.getUlamKeyTypeSignatureClassInstanceIdx(), autoreftype);

    if(key1 == newkey)
      return;

    ULAMTYPE etype = aut->getUlamTypeEnum();
    bool isAClass = (etype == Class);
    bool isCustomArray = (isAClass ? aut->isCustomArray() : false);
    ULAMCLASSTYPE classtype = aut->getUlamClass(); // or notaclass

    //removes old key and its ulamtype from map, if no longer pointed to
    deleteUlamKeyTypeSignature(key1, auti);

    UlamType * newut = NULL;
    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, etype);
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType *>(newkey,newut));
	if(isAClass)
	  {
	    ((UlamTypeClass *) newut)->setUlamClass(classtype); //restore from original ut
	    if(isCustomArray)
	      ((UlamTypeClass *) newut)->setCustomArray();
	  }
      }

    m_indexToUlamKey[auti] = newkey;
    incrementKeyToAnyUTICounter(newkey, auti);
    {
      std::ostringstream msg;
      msg << "Reference key for (UTI" << auti << ") ";
      msg << getUlamTypeNameBriefByIndex(auti).c_str();
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
  } //rekeyToReferenceUTI

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
    if(!isComplete(auti))
      return; //without knowing the bitsize, don't alias it

    updateUTIAliasForced(auti, buti);
  } //updateUTIAlias

  void CompilerState::updateUTIAliasForced(UTI auti, UTI buti)
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
  } //updateUTIAlias

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

  // return false if ERR
  bool CompilerState::setSizesOfNonClass(UTI utArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    assert((ut->getUlamClass() == UC_NOTACLASS) || !ut->isScalar());

    if(ut->isComplete())
      return true;  //nothing to do

    ULAMTYPE bUT = ut->getUlamTypeEnum();
    //disallow zero-sized primitives (no such thing as a BitVector<0u>)
    //'Void' by default is zero and only zero bitsize (already complete)
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
    if((bUT != Class) && ((key.getUlamKeyTypeSignatureBitSize() == 0) || (bitsize <= 0)))
      {
	if((bUT != Void) || (bitsize < 0))
	  {
	    std::ostringstream msg;
	    msg << "Invalid size (";
	    msg << bitsize;
	    msg << ") to set for primitive type: " << UlamType::getUlamTypeEnumAsString(bUT);
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
	//Void with zero bitsize
	if(arraysize != NONARRAYSIZE)
	  {
	    // disallow an array of Void(0)'s
	    std::ostringstream msg;
	    msg << "Invalid Void type array: " << ut->getUlamTypeNameBrief().c_str();
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
      }
    else if(bUT == Void)
      {
	std::ostringstream msg;
	msg << "Invalid nonzero bitsize (" << bitsize << ") for Void type";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	return false;
      }

    //continue with valid number of bits
    UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key.getUlamKeyTypeSignatureNameId(), bitsize, arraysize, key.getUlamKeyTypeSignatureClassInstanceIdx(), key.getUlamKeyTypeSignatureReferenceType());

    if(key == newkey)
      return true;

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
    return true;
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

  u32 CompilerState::getTotalWordSize(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    return (ut->getTotalWordSize());
  }

  s32 CompilerState::slotsNeeded(UTI uti)
  {
    if(uti == Void)
      return 0;

    UlamType * ut = getUlamTypeByIndex(uti);
    if(ut->isReference())
      return 1;

    s32 arraysize = ut->getArraySize();
    PACKFIT packed = ut->getPackable();

    if(WritePacked(packed))
      arraysize = 1;
    else
      arraysize = (arraysize > NONARRAYSIZE ? arraysize : 1);
    return arraysize;
  } //slotsNeeded

  bool CompilerState::isClassATemplate(UTI cuti)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->isClassTemplate(cuti);
  } //isClassATemplate

  UTI CompilerState::isClassASubclass(UTI cuti)
  {
    UTI subuti = getUlamTypeAsScalar(cuti); //in case of array

    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(subuti, csym))
      {
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClassName(csym->getId(), cnsym);
	assert(isDefined);
	return cnsym->getSuperClassForClassInstance(subuti); //returns super UTI, or Nouti if no inheritance, Hzy if UNSEEN
      }
    return Nouti; //even for non-classes
  } //isClassASubclass

  void CompilerState::resetClassSuperclass(UTI cuti, UTI superuti)
  {
    UTI subuti = getUlamTypeAsScalar(cuti); //in case of array

    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(subuti, csym))
      {
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClassName(csym->getId(), cnsym);
	assert(isDefined);
	cnsym->setSuperClassForClassInstance(superuti, cuti);
      }
  } //resetClassSuperclass

  //return true if the second arg is a superclass of the first arg.
  // i.e. cuti is a subclass of superp. recurses the family tree.
  bool CompilerState::isClassASubclassOf(UTI cuti, UTI superp)
  {
    bool rtnb = false;
    UTI prevuti = cuti; //init for the loop
    while(!rtnb && (prevuti != Nouti))
      {
	cuti = prevuti;
	SymbolClass * csym = NULL;
	if(alreadyDefinedSymbolClass(cuti, csym))
	  {
	    SymbolClassName * cnsym = NULL;
	    AssertBool isDefined = alreadyDefinedSymbolClassName(csym->getId(), cnsym);
	    assert(isDefined);
	    prevuti = cnsym->getSuperClassForClassInstance(cuti); //returns super UTI, or Nouti if no inheritance
	    rtnb = (superp == prevuti); //compare
	  }
	else
	  prevuti = Nouti; //avoid inf loop
      } //end while
    return rtnb; //even for non-classes
  } //isClassASubclassOf

  bool CompilerState::isClassAStub(UTI cuti)
  {
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      return csym->isStub();

    return false; //even for non-classes
  } //isClassAStub

  bool CompilerState::hasClassAStub(UTI cuti)
  {
    bool rtnb = false;
    UTI prevuti = cuti; //init for the loop

    while(!rtnb && (prevuti != Nouti))
      {
	rtnb = isClassAStub(prevuti);
	prevuti = isClassASubclass(prevuti);
      }
    return rtnb; //even for non-classes
  } //hasClassAStub

  bool CompilerState::isClassAQuarkUnion(UTI cuti)
  {
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      {
	return csym->isQuarkUnion();
      }
    return false; //even for non-classes
  } //isClassAQuarkUnion

  bool CompilerState::isClassACustomArray(UTI cuti)
  {
    if(!isScalar(cuti)) return false;
    //assert(isScalar(cuti));

    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      {
	return csym->isCustomArray(); //checks via classblock in case of inheritance
      }
    return false; //even for non-classes
  } //isClassACustomArray

  UTI CompilerState::getAClassCustomArrayType(UTI cuti)
  {
    assert(isScalar(cuti));
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->getCustomArrayType(); //checks via classblock in case of inheritance
  } //getAClassCustomArrayType

  UTI CompilerState::getAClassCustomArrayIndexType(UTI cuti, Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    assert(isScalar(cuti));
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->getCustomArrayIndexTypeFor(rnode, idxuti, hasHazyArgs); //checks via classblock in case of inheritance
  } //getAClassCustomArrayIndexType

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
	msg << m_pool.getDataAsString(symptr->getId()).c_str();
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR); //parsing
      }
    return (rtnb && symptr->isClassTemplate());
  } //alreadyDefinedSymbolClassNameTemplate

  //if necessary, searches for instance of class "template" with matching SCALAR uti;
  // automatically reduces arrays and references to their base class UTI.
  bool CompilerState::alreadyDefinedSymbolClass(UTI uti, SymbolClass * & symptr)
  {
    bool rtnb = false;
    UlamType * ut = getUlamTypeByIndex(uti);

    UTI scalarUTI = uti;
    if(!ut->isScalar())
      scalarUTI = getUlamTypeAsScalar(uti); //ALT_ARRAYITEM ?

    scalarUTI = getUlamTypeAsDeref(scalarUTI); //and deref

    SymbolClassName * cnsym = NULL;
    if(alreadyDefinedSymbolClassName(ut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), cnsym))
      {
	//not a regular class, and not the template, so dig deeper for the stub
	if((cnsym->getUlamTypeIdx() != scalarUTI) && cnsym->isClassTemplate())
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

  bool CompilerState::alreadyDefinedSymbolClassAsHolder(UTI uti, SymbolClass * & symptr)
  {
    bool rtnb = false;
    u32 id = m_pool.getIndexForNumberAsString(uti);

    SymbolClassName * cnsym = NULL;
    if(alreadyDefinedSymbolClassName(id, cnsym))
      {
	symptr = cnsym;
	rtnb = true;
      }
    return rtnb;
  } //alreadyDefinedSymbolClassAsHolder

  //temporary type name which will be updated during type labeling.
  void CompilerState::addUnknownTypeTokenToAClassResolver(UTI cuti, Token tok, UTI huti)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    csym->addUnknownTypeTokenToClass(tok, huti);
  } //addUnknownTypeTokenToThisClassResolver

  //temporary type name which will be updated during type labeling.
  void CompilerState::addUnknownTypeTokenToThisClassResolver(Token tok, UTI huti)
  {
    addUnknownTypeTokenToAClassResolver(getCompileThisIdx(), tok, huti);
  } //addUnknownTypeTokenToThisClassResolver

  //temporary type name which will be updated during type labeling.
  Token CompilerState::removeKnownTypeTokenFromThisClassResolver(UTI huti)
  {
    UTI cuti = getCompileThisIdx();
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    return csym->removeKnownTypeTokenFromClass(huti);
  } //removeKnownTypeTokenFromThisClassResolver

  //before removing, check existence
  bool CompilerState::hasUnknownTypeInThisClassResolver(UTI huti)
  {
    UTI cuti = getCompileThisIdx();
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    return csym->hasUnknownTypeInClass(huti);
  } //hasUnknownTypeInThisClassResolver

  //false if not there, or still hazy; true if resolved; called during c&l
  bool CompilerState::statusUnknownTypeInThisClassResolver(UTI huti)
  {
    UTI cuti = getCompileThisIdx();
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    return csym->statusUnknownTypeInClass(huti);
  } //statusUnknownTypeInThisClassResolver

  //temporary UlamType which will be updated during type labeling.
  bool CompilerState::addIncompleteClassSymbolToProgramTable(Token cTok, SymbolClassName * & symptr)
  {
    u32 dataindex = cTok.m_dataindex;
    AssertBool isNotDefined = ((symptr == NULL) && !alreadyDefinedSymbolClassName(dataindex, symptr));
    assert(isNotDefined);

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE);  //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class);  //**gets next unknown uti type

    NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
    assert(classblock);
    classblock->setNodeLocation(cTok.m_locator);
    classblock->setNodeType(cuti);

    //avoid Invalid Read whenconstructing class' Symbol
    pushClassContext(cuti, classblock, classblock, false, NULL); //null blocks likely

    //symbol ownership goes to the programDefST; distinguish btn template & regular classes here:
    symptr = new SymbolClassName(cTok, cuti, classblock, *this);
    m_programDefST.addToTable(dataindex, symptr);
    m_unseenClasses.insert(symptr);

    popClassContext();
    return true; //compatible with alreadyDefinedSymbolClassName return
  } //addIncompleteClassSymbolToProgramTable

  //temporary UlamType which will be updated during type labeling.
  bool CompilerState::addIncompleteTemplateClassSymbolToProgramTable(Token cTok, SymbolClassNameTemplate * & symptr)
  {
    u32 dataindex = cTok.m_dataindex;
    AssertBool isNotDefined = ((symptr == NULL) && !alreadyDefinedSymbolClassNameTemplate(dataindex,symptr));
    assert(isNotDefined);

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE); //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class); //**gets next unknown uti type

    NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
    assert(classblock);
    classblock->setNodeLocation(cTok.m_locator);
    classblock->setNodeType(cuti);

    //avoid Invalid Read whenconstructing class' Symbol
    pushClassContext(cuti, classblock, classblock, false, NULL); //null blocks likely

    //symbol ownership goes to the programDefST; distinguish btn template & regular classes here:
    symptr = new SymbolClassNameTemplate(cTok, cuti, classblock, *this);
    m_programDefST.addToTable(dataindex, symptr);
    m_unseenClasses.insert(symptr);
    symptr->setSuperClass(Hzy);

    popClassContext();
    return true; //compatible with alreadyDefinedSymbolClassNameTemplate return
  } //addIncompleteTemplateClassSymbolToProgramTable

  UTI CompilerState::addStubCopyToAncestorClassTemplate(UTI stubTypeToCopy,  UTI context)
  {
    //handle inheritance of stub super class, based on its template
    UTI superuti = stubTypeToCopy;
    assert((superuti != Nouti) && (superuti != Hzy));
    assert(isClassAStub(superuti));
    UlamType * superut = getUlamTypeByIndex(superuti);
    UlamKeyTypeSignature superkey = superut->getUlamKeyTypeSignature();
    u32 superid = superkey.getUlamKeyTypeSignatureNameId();
    SymbolClassNameTemplate * superctsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassNameTemplate(superid, superctsym);
    assert(isDefined);

    UlamKeyTypeSignature newstubkey(superid, UNKNOWNSIZE);  //"-2" and scalar default
    UTI newstubcopyuti = makeUlamType(newstubkey, Class);  //**gets next unknown uti type
    superctsym->copyAStubClassInstance(superuti, newstubcopyuti, context);
    superctsym->mergeClassInstancesFromTEMP(); //not mid-iteration!!
    return newstubcopyuti;
  } //addStubCopyToAncestorClassTemplate

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
	//excludes anonymous classes, only unseen classes with known names
	assert(getUlamTypeByIndex(cnsym->getUlamTypeIdx())->getUlamClass() == UC_UNSEEN);
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

    assert(okUTItoContinue(incomplete));

    if(isHolder(incomplete))
      return false; //short-circuit

    UlamType * ict = getUlamTypeByIndex(incomplete);
    assert(ict->getUlamClass() == UC_UNSEEN); //missing
    if(alreadyDefinedSymbolClass(incomplete, csym))
      {
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClassName(csym->getId(), cnsym);
	assert(isDefined);
	UTI but = cnsym->getUlamTypeIdx();
	assert(okUTItoContinue(but));
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

  bool CompilerState::alreadyDefinedSymbolByAncestor(u32 dataindex, Symbol *& symptr, bool& hasHazyKin)
  {
    //maybe in current class as a holder, which doesn't progress the search (just the opposite, stops it!).
    //recursively check ancestors, for defined name (and not a Holder)
    UTI cuti = getCompileThisIdx();
    UTI superuti = isClassASubclass(cuti);
    if(okUTItoContinue(superuti))
      {
	SymbolClass * supcsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClass(superuti, supcsym);
	assert(isDefined);

	pushClassContext(superuti, supcsym->getClassBlockNode(), supcsym->getClassBlockNode(), false, NULL);

	bool tmphzykin = false;
	bool kinhadit = alreadyDefinedSymbol(dataindex, symptr, tmphzykin);

	if(!kinhadit || (isHolder(symptr->getUlamTypeIdx())))
	  return alreadyDefinedSymbolByAncestor(dataindex, symptr, hasHazyKin);

	hasHazyKin = tmphzykin;
	popClassContext(); //restore
	return kinhadit;
      }
    return false;
  } //alreadyDefinedSymbolByAncestor

  bool CompilerState::alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    bool brtn = false;
    assert(!hasHazyKin);

    //start with the current "top" block and look down the stack
    //until the 'variable id' is found.
    NodeBlock * blockNode = getCurrentBlock();

    //substitute another selected class block to search for data member
    if(useMemberBlock())
      blockNode = getCurrentMemberClassBlock();

    while(!brtn && blockNode)
      {
	brtn = blockNode->isIdInScope(dataindex,symptr); //check ST

	//hazy check..
	UTI buti = blockNode->getNodeType();
	if(blockNode->isAClassBlock() && (isClassAStub(buti) || ((isClassASubclass(buti) != Nouti) && !((NodeBlockClass *) blockNode)->isSuperClassLinkReady())))
	  hasHazyKin = true;

	blockNode = blockNode->getPreviousBlockPointer(); //traverse the chain, including templates (not ancestors)
      }

    //data member variables in class block; function symbols are linked to their
    //function def block; check function data members separately.
    if(!brtn)
      brtn = isDataMemberIdInClassScope(dataindex, symptr, hasHazyKin);

    if(!brtn)
      brtn = isFuncIdInClassScope(dataindex, symptr, hasHazyKin);
    return brtn;
  } //alreadyDefinedSymbol

  bool CompilerState::isDataMemberIdInClassScope(u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    bool brtn = false;
    //assert(!hasHazyKin); might come from alreadyDefinedSymbol now, and have a hazy chain.

    //start with the current class block and look up family tree
    //until the 'variable id' is found.
    NodeBlockClass * classblock = getClassBlock();

    //substitute another selected class block to search for data member
    if(useMemberBlock())
      classblock = getCurrentMemberClassBlock();

    while(!brtn && classblock)
      {
	brtn = classblock->isIdInScope(dataindex,symptr); //returns symbol

	UTI cuti = classblock->getNodeType();
	if(isClassAStub(cuti) || ((isClassASubclass(cuti) != Nouti) && !classblock->isSuperClassLinkReady()))
	  hasHazyKin = true; //self is stub

	classblock = classblock->getSuperBlockPointer(); //inheritance chain
      }
    return brtn;
  } //isDataMemberIdInClassScope

  bool CompilerState::isFuncIdInClassScope(u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    bool brtn = false;
    //assert(!hasHazyKin); might come from alreadyDefinedSymbol now, and have a hazy chain.

    //start with the current class block and look up family tree
    //until the 'variable id' is found.
    NodeBlockClass * classblock = getClassBlock();

    //substitute another selected class block to search for data member
    if(useMemberBlock())
      classblock = getCurrentMemberClassBlock();

    while(!brtn && classblock)
      {
	brtn = classblock->isFuncIdInScope(dataindex,symptr); //returns symbol

	UTI cuti = classblock->getNodeType();
	if(isClassAStub(cuti) || ((isClassASubclass(cuti) != Nouti) && !classblock->isSuperClassLinkReady()))
	  hasHazyKin = true; //self is stub

	classblock = classblock->getSuperBlockPointer(); //inheritance chain
      }
    return brtn;
  } //isFuncIdInClassScope

  bool CompilerState::isFuncIdInClassScopeNNO(NNO cnno, u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    UTI cuti = findAClassByNodeNo(cnno); //class of cnno
    assert(okUTItoContinue(cuti));
    assert(!hasHazyKin);
    return isFuncIdInAClassScope(cuti, dataindex, symptr, hasHazyKin);
  } //isFuncIdInClassScopeNNO

bool CompilerState::isFuncIdInAClassScope(UTI cuti, u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    NodeBlockClass * cblock = csym->getClassBlockNode();
    assert(cblock);
    pushClassContext(cuti, cblock, cblock, false, NULL);

    bool rtnb = isFuncIdInClassScope(dataindex, symptr, hasHazyKin);
    popClassContext(); //don't forget!!
    return rtnb;
  } //isFuncIdInAClassScope

  bool CompilerState::findMatchingFunctionInAncestor(UTI cuti, u32 fid, std::vector<UTI> typeVec, SymbolFunction*& fsymref, UTI& foundInAncestor)
  {
    bool rtnb = false;
    UTI superuti = isClassASubclass(cuti);
    while(!rtnb)
      {
	if((superuti != Nouti) && (superuti != Hzy))
	  {
	    SymbolClass * supercsym = NULL;
	    AssertBool isDefined = alreadyDefinedSymbolClass(superuti, supercsym);
	    assert(isDefined);
	    NodeBlockClass * cblock = supercsym->getClassBlockNode();
	    assert(cblock);
	    pushClassContextUsingMemberClassBlock(cblock);

	    Symbol * fnSym = NULL;
	    if(cblock->isFuncIdInScope(fid, fnSym)) //dont check ancestor
	      rtnb = (((SymbolFunctionName *) fnSym)->findMatchingFunctionStrictlyByTypes(typeVec, fsymref) == 1); //exact
	    if(rtnb)
	      foundInAncestor = superuti;
	    else
	      superuti = isClassASubclass(superuti);

	    popClassContext(); //don't forget!!
	  }
	else
	  {
	    foundInAncestor = Nav;
	    break;
	  }
      } //while
    return rtnb;
  } //findMatchingFunctionInAncestor

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
    //return std::string(tok->getTokenString()); //VG: Invalid Read
    return tok->getTokenStringFromPool(this);
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
	    return tok.getTokenStringFromPool(this);
	  }
	else
	  {
	    UTI tduti = Nav;
	    UTI tmpforscalaruti = Nouti;
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

  // note: we may miss a missing return value for non-void function; non-trivial to
  // do logic-flow analysis; gcc catches them until ulam template absorbs gcc errors.
  bool CompilerState::checkFunctionReturnNodeTypes(SymbolFunction * fsym)
  {
    bool rtnBool = true;
    UTI it = fsym->getUlamTypeIdx();

    if(m_currentFunctionReturnNodes.empty())
      {
	if(it != Void && !fsym->isNativeFunctionDeclaration() && (!fsym->isVirtualFunction() || !fsym->isPureVirtualFunction()))
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
		msg << "''s Return type's " << getUlamTypeNameByIndex(it).c_str();
		msg << " base type <" << UlamType::getUlamTypeEnumAsString(itBUT);
		msg << "> does not match resulting type's ";
		msg << getUlamTypeNameByIndex(rType).c_str() << " base type <";
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
		    msg << "''s Return type's " << getUlamTypeNameByIndex(it).c_str();
		    msg << " array size <" << getArraySize(it);
		    msg << "> does not match resulting type's ";
		    msg << getUlamTypeNameByIndex(rType).c_str() << " array size <";
		    msg << getArraySize(rType) << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_ERR);
		  }

		if(getBitSize(rType) != getBitSize(it))
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str();
		    msg << "''s Return type's " << getUlamTypeNameByIndex(it).c_str();
		    msg << " bit size <" << getBitSize(it);
		    msg << "> does not match resulting type's ";
		    msg << getUlamTypeNameByIndex(rType).c_str() << " bit size <";
		    msg << getBitSize(rType) << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_ERR);
		  }

		if(getReferenceType(rType) != getReferenceType(it))
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str();
		    msg << "''s Return type's " << getUlamTypeNameByIndex(it).c_str();
		    msg << " reference type <" << getReferenceType(it);
		    msg << "> does not match resulting type's ";
		    msg << getUlamTypeNameByIndex(rType).c_str() << " reference type <";
		    msg << getReferenceType(rType) << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_ERR);
		  }
	      } //base types are the same..array and bit size might vary, or ref type
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
    return HIDDEN_ARG_NAME;
  }

  const char * CompilerState::getHiddenContextArgName()
  {
    return HIDDEN_CONTEXT_ARG_NAME;
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

  const char * CompilerState::getCustomArrayGetMangledFunctionName()
  {
    return CUSTOMARRAY_GET_MANGLEDNAME;
  }

  const char * CompilerState::getCustomArraySetMangledFunctionName()
  {
    return CUSTOMARRAY_SET_MANGLEDNAME;
  }

  const char * CompilerState::getIsMangledFunctionName(UTI ltype)
  {
    if(isAtom(ltype))
      return IS_MANGLED_FUNC_NAME_FOR_ATOM;

    return IS_MANGLED_FUNC_NAME;
  }

  const char * CompilerState::getHasMangledFunctionName(UTI ltype)
  {
    if(isAtom(ltype))
      return HAS_MANGLED_FUNC_NAME_FOR_ATOM;
    return HAS_MANGLED_FUNC_NAME;
  }

  const char * CompilerState::getAsMangledFunctionName(UTI ltype, UTI rtype)
  {
    assert(okUTItoContinue(rtype));
    ULAMCLASSTYPE rclasstype = getUlamTypeByIndex(rtype)->getUlamClass();
    if(rclasstype == UC_QUARK)
      return getIsMangledFunctionName(ltype);
    else if (rclasstype == UC_ELEMENT)
      return IS_MANGLED_FUNC_NAME;
    else
      assert(0);

    return "AS_ERROR";
  } //getAsMangledFunctionName

  const char * CompilerState::getBuildDefaultAtomFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    ULAMCLASSTYPE lclasstype = getUlamTypeByIndex(ltype)->getUlamClass();
    if(lclasstype == UC_ELEMENT)
      return BUILD_DEFAULT_ATOM_FUNCNAME; //for elements
    else if(lclasstype == UC_QUARK)
      return getDefaultQuarkFunctionName();
    else
      assert(0);
    return "BUILDEFAULTATOM_ERROR";
  } //getBuildDefaultAtomFunctionName

  const char * CompilerState::getDefaultQuarkFunctionName()
  {
      return BUILD_DEFAULT_QUARK_FUNCNAME;
  } //getDefaultQuarkFunctionName

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
    AssertBool isDefined = alreadyDefinedSymbolClassName(getCompileThisId(), cnsym);
    assert(isDefined);
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
    assert(okUTItoContinue(uti));
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
    assert(okUTItoContinue(uti));
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

  UlamValue CompilerState::getAtomPtrFromSelfPtr()
  {
    UlamValue tuv = getPtrTarget(m_currentSelfPtr);
    UTI tuti = tuv.getUlamValueTypeIdx();

    UlamValue aptr = UlamValue::makePtr(m_currentSelfPtr.getPtrSlotIndex(), m_currentSelfPtr.getPtrStorage(), tuti, determinePackable(tuti), *this, 0, m_currentSelfPtr.getPtrNameId());
    return aptr;
  } //getAtomPtrFromSelfPtr (for eval)

  UlamValue CompilerState::getPtrTarget(UlamValue ptr)
  {
    assert(ptr.isPtr());
    if(ptr.getUlamValueTypeIdx() == PtrAbs)
      return getPtrTargetFromAbsoluteIndex(ptr); //short-circuit

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

  UlamValue CompilerState::getPtrTargetFromAbsoluteIndex(UlamValue ptr)
  {
    assert(ptr.getUlamValueTypeIdx() == PtrAbs);

    //slot + storage
    UlamValue valAtIdx;
    switch(ptr.getPtrStorage())
      {
      case STACK:
	valAtIdx = m_funcCallStack.loadUlamValueFromStackIndex(ptr.getPtrSlotIndex());
	break;
      case EVALRETURN:
	valAtIdx = m_nodeEvalStack.loadUlamValueFromStackIndex(ptr.getPtrSlotIndex());
	break;
      case EVENTWINDOW:
	assert(0);
	valAtIdx = m_eventWindow.loadAtomFromSite(ptr.getPtrSlotIndex()); //?
	break;
      default:
	assert(0); //error!
      };
    return valAtIdx; //return as-is
  } //getPtrTargetFromAbsoluteIndex

  //general purpose store
  void CompilerState::assignValue(UlamValue lptr, UlamValue ruv)
  {
    assert(lptr.isPtr());

    //handle UAtom assignment as a singleton (not array values)
    if(ruv.isPtr() && (UlamType::compareForUlamValueAssignment(ruv.getPtrTargetType(), UAtom, *this) == UTIC_NOTSAME || UlamType::compareForUlamValueAssignment(lptr.getPtrTargetType(), UAtom, *this) == UTIC_NOTSAME))
      return assignArrayValues(lptr, ruv);

    //r is data (includes packed arrays), store it into where lptr is pointing
    assert((UlamType::compareForUlamValueAssignment(lptr.getPtrTargetType(), ruv.getUlamValueTypeIdx(), *this) == UTIC_SAME) || (UlamType::compareForUlamValueAssignment(lptr.getPtrTargetType(), UAtom, *this) == UTIC_SAME) || (UlamType::compareForUlamValueAssignment(ruv.getUlamValueTypeIdx(), UAtom, *this) == UTIC_SAME));

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
    assert(lptr.isPtr());
    assert(rptr.isPtr());

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
	u32 size = slotsNeeded(rptr.getPtrTargetType());

	UlamValue nextlptr = UlamValue::makeScalarPtr(lptr,*this);
	UlamValue nextrptr = UlamValue::makeScalarPtr(rptr,*this);
	tuti = nextrptr.getPtrTargetType(); //update type

	for(u32 i = 0; i < size; i++)
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

	    AssertBool isNextLeft = nextlptr.incrementPtr(*this);
	    assert(isNextLeft || ((i+1) == size));
	    AssertBool isNextRight = nextrptr.incrementPtr(*this);
	    assert(isNextRight || ((i+1) == size));
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
    NodeBlockFunctionDefinition * func = getClassBlock()->findTestFunctionNode();
    return (func != NULL);
  } //thisClassHasTheTestMethod

  bool CompilerState::thisClassIsAQuark()
  {
    UTI cuti = getCompileThisIdx();
    assert(okUTItoContinue(cuti));
    return(getUlamTypeByIndex(cuti)->getUlamClass() == UC_QUARK);
  } //thisClassIsAQuark

  bool CompilerState::quarkHasAToIntMethod(UTI quti)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(quti, csym);
    assert(isDefined);

    UTI cuti = csym->getUlamTypeIdx();
    assert(okUTItoContinue(cuti));
    AssertBool isQuark = (getUlamTypeByIndex(cuti)->getUlamClass() == UC_QUARK);
    assert(isQuark);
    NodeBlockClass * classNode = csym->getClassBlockNode();
    assert(classNode);

    NodeBlockFunctionDefinition * func = classNode->findToIntFunctionNode();
    return (func != NULL);
  } //quarkHasAToIntMethod

  bool CompilerState::classHasACustomArraySetMethod(UTI cuti)
  {
    assert(isScalar(cuti));
    bool rtnb = false;
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      {
	NodeBlockClass * cblock = csym->getClassBlockNode();
	pushClassContextUsingMemberClassBlock(cblock);

	//hasACustomArraySetFunction();
	bool hazykin = false;
	Symbol * fsym = NULL;
	rtnb = isFuncIdInClassScope(getCustomArraySetFunctionNameId(), fsym, hazykin);

	popClassContext();
      }
    return rtnb;
  } //thisClassHasACustomArraySetMethod

  void CompilerState::setupCenterSiteForTesting()
  {
    //call again for code gen..
    //set up an atom in eventWindow; init m_currentObjPtr to point to it
    //set up stacks since func call not called
    Coord c0(0,0);

    //m_classBlock ok now, reset by NodeProgram after type label done
    UTI cuti = getCompileThisIdx();
    m_eventWindow.setSiteElementType(c0, cuti); //includes default values
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
	return "<empty path>\n"; //used in gen code
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
    assert(okUTItoContinue(uti));

    std::ostringstream tmpVar; //into
    PACKFIT packed = determinePackable(uti);

    UlamType * ut = getUlamTypeByIndex(uti);
    if(isAtom(uti) || (ut->getUlamClass() == UC_ELEMENT))
      {
	//stg = TMPBITVAL or TMPTATOM; avoid loading a T into a tmpregister!
	assert(stg != TMPREGISTER);
      }

    if(stg == TMPREGISTER)
      {
	if(WritePacked(packed))
	  tmpVar << "Uh_5tlreg" ; //tmp loadable register
	else
	  tmpVar << "Uh_5tureg" ; //tmp unpacked register
      }
    else if(stg == TMPBITVAL)
      {
	if(WritePacked(packed))
	  tmpVar << "Uh_5tlval" ; //tmp loadable value
	else
	  tmpVar << "Uh_5tuval" ; //tmp unpacked value
      }
    else if(stg == TMPAUTOREF)
      {
	if(WritePacked(packed))
	  tmpVar << "Uh_6tlref" ; //tmp loadable autoref
	else
	  tmpVar << "Uh_6turef" ; //tmp unpacked autoref
      }
    else if(stg == TMPTATOM)
      {
	tmpVar << "Uh_3tut" ; //tmp unpacked atom T
      }
    else
      assert(0); //remove assumptions about tmpbitval.

    tmpVar << ToLeximitedNumber(num);

    return tmpVar.str();
  } //getTmpVarAsString

  const std::string CompilerState::getUlamRefTmpVarAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_3tur" << ToLeximitedNumber(num);
    return labelname.str();
  } //getUlamRefTmpVarAsString

  const std::string CompilerState::getUlamClassTmpVarAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_3tuclass" << ToLeximitedNumber(num);
    return labelname.str();
  } //getUlamClassTmpVarAsString

  const std::string CompilerState::getLabelNumAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Ul_endcontrolloop_" << ToLeximitedNumber(num);
    return labelname.str();
  } //getLabelNumAsString

  void CompilerState::saveIdentTokenForConditionalAs(Token iTok, Token cTok)
  {
    m_identTokenForConditionalAs = iTok;
    m_parsingConditionalToken = cTok;
    m_parsingConditionalAs = true; //cleared manually
  } //saveIdentTokenForConditionalAs

  void CompilerState::saveStructuredCommentToken(Token scTok)
  {
    m_precedingStructuredCommentToken = scTok;
    m_gotStructuredCommentToken = true;
  } //saveStructuredCommentToken

  void CompilerState::clearStructuredCommentToken()
  {
    Token blankTok; //unitialized
    m_precedingStructuredCommentToken = blankTok;
    m_gotStructuredCommentToken = false;
  } //clearStructuredCommentToken

  bool CompilerState::getStructuredCommentToken(Token& scTok)
  {
    if(m_gotStructuredCommentToken)
      {
	assert(m_precedingStructuredCommentToken.m_type == TOK_STRUCTURED_COMMENT);
	scTok = m_precedingStructuredCommentToken;
	clearStructuredCommentToken(); //auto clear; 1-get per scTOK
	return true;
      }
    return false;
  } //getStructuredCommentToken

  NNO CompilerState::getNextNodeNo()
  {
    return ++m_nextNodeNumber; //first one is 1
  }

  Node * CompilerState::findNodeNoInThisClass(NNO n)
  {
    if(useMemberBlock())
      {
	UTI mbuti = getCurrentMemberClassBlock()->getNodeType();
	u32 mbid = getUlamKeyTypeSignatureByIndex(mbuti).getUlamKeyTypeSignatureNameId();
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClassName(mbid, cnsym);
	assert(isDefined);
	return cnsym->findNodeNoInAClassInstance(mbuti, n);
      }

    NodeBlock * currBlock = getCurrentBlock();
    if(currBlock && currBlock->getNodeNo() == n && getClassBlock()->getNodeType() == getCompileThisIdx())
      return currBlock; //avoid chix-n-egg with functiondefs

    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassName(getCompileThisId(), cnsym);
    assert(isDefined);
    Node * rtnNode = cnsym->findNodeNoInAClassInstance(getCompileThisIdx(), n);

    //last resort, if we are in the middle of resolving pending args for a stub
    // and to do constant folding, we need to find the parent node that's in the
    // stub's resolver, NOT the context where the stub appears.
    if(!rtnNode)
      {
	UTI stubuti = m_pendingArgStubContext;
	if(stubuti != Nouti)
	  {
	    u32 stubid = getUlamKeyTypeSignatureByIndex(stubuti).getUlamKeyTypeSignatureNameId();
	    SymbolClassNameTemplate * cntsym = NULL;
	    AssertBool isDefined = alreadyDefinedSymbolClassNameTemplate(stubid, cntsym);
	    assert(isDefined);
	    rtnNode =  cntsym->findNodeNoInAClassInstance(stubuti, n);
	  }
	else
	  {
	    //what if in ancestor?
	    UTI superuti = isClassASubclass(getCompileThisIdx());
	    if((superuti != Nouti) && (superuti != Hzy))
	      rtnNode = findNodeNoInAClass(n, superuti);
	  }
      }
    return rtnNode;
  } //findNodeNoInThisClass

  Node * CompilerState::findNodeNoInAClass(NNO n, UTI cuti)
  {
    u32 cid = getUlamKeyTypeSignatureByIndex(cuti).getUlamKeyTypeSignatureNameId();
    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassName(cid, cnsym);
    assert(isDefined);
    return cnsym->findNodeNoInAClassInstance(cuti, n);
  } //findNodeNoInAClass

  UTI CompilerState::findAClassByNodeNo(NNO n)
  {
    return m_programDefST.findClassNodeNoForTableOfClasses(n); //Nav not found
  } //findAClassByNodeNo

  NodeBlockClass * CompilerState::getAClassBlock(UTI cuti)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->getClassBlockNode();
  } //getAClassBlock

  NNO CompilerState::getAClassBlockNo(UTI cuti)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->getClassBlockNode()->getNodeNo();
  } //getAClassBlockNo

  u32 CompilerState::getCompileThisId()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    return cc.getCompileThisId();
  }

  //also getUlamTypeForThisClass()
  UTI CompilerState::getCompileThisIdx()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    return cc.getCompileThisIdx();
  }

  SymbolClass * CompilerState::getCurrentSelfSymbolForCodeGen()
  {
    return (SymbolClass *) m_currentSelfSymbolForCodeGen;
  } //getCurrentSelfSymbolForCodeGen

  NodeBlock * CompilerState::getCurrentBlock()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
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
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    return cc.getClassBlock();
  }

  NNO CompilerState::getClassBlockNo()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
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
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    return cc.getCurrentMemberClassBlock();
  }

  void CompilerState::pushClassContext(UTI idx, NodeBlock * currblock, NodeBlockClass * classblock, bool usemember, NodeBlockClass * memberblock)
  {
    u32 id = getUlamKeyTypeSignatureByIndex(idx).getUlamKeyTypeSignatureNameId();
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
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    cc.setCurrentBlock(currblock); //could be NULL
    m_classContextStack.pushClassContext(cc);
  }

  void CompilerState::pushCurrentBlockAndDontUseMemberBlock(NodeBlock * currblock)
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    cc.setCurrentBlock(currblock);
    cc.useMemberBlock(false);
    m_classContextStack.pushClassContext(cc);
  }

  void CompilerState::pushClassContextUsingMemberClassBlock(NodeBlockClass * memberblock)
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    cc.setCurrentMemberClassBlock(memberblock);
    cc.useMemberBlock(true);
    m_classContextStack.pushClassContext(cc);
  }

  std::string CompilerState::getClassContextAsStringForDebugging()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
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

  UTI CompilerState::getLongUTI()
  {
    UlamKeyTypeSignature ilkey(m_pool.getIndexForDataString("Int"), MAXBITSPERLONG);
    return makeUlamType(ilkey, Int);
  }

  UTI CompilerState::getUnsignedLongUTI()
  {
    UlamKeyTypeSignature ulikey(m_pool.getIndexForDataString("Unsigned"), MAXBITSPERLONG);
    return makeUlamType(ulikey, Unsigned);
  }

  UTI CompilerState::getBigBitsUTI()
  {
    UlamKeyTypeSignature ubitskey(m_pool.getIndexForDataString("Bits"), MAXBITSPERLONG);
    return makeUlamType(ubitskey, Bits);
  }

  bool CompilerState::isPtr(UTI puti)
  {
    return ((puti == Ptr) || (puti == PtrAbs));
  }

  bool CompilerState::isAtom(UTI auti)
  {
    return (getUlamTypeByIndex(auti)->getUlamTypeEnum() == UAtom);
  }

  bool CompilerState::isAtomRef(UTI auti)
  {
    UlamType * aut = getUlamTypeByIndex(auti);
    return ((aut->getUlamTypeEnum() == UAtom) && aut->isReference());
  }

  bool CompilerState::okUTItoContinue(UTI uti)
  {
    return ((uti != Nav) && (uti != Hzy) && (uti != Nouti));
  }

} //end MFM
