#include <stdio.h>
#include <iostream>
#include "ClassContext.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "SymbolFunctionName.h"
#include "SymbolTypedef.h"
#include "UlamTypeAtom.h"
#include "UlamTypeClass.h"
#include "UlamTypeClassElement.h"
#include "UlamTypeClassQuark.h"
#include "UlamTypeClassTransient.h"
#include "UlamTypeInternalHolder.h"
#include "UlamTypeInternalHzy.h"
#include "UlamTypeInternalLocalsFileScope.h"
#include "UlamTypeInternalNav.h"
#include "UlamTypeInternalNouti.h"
#include "UlamTypeInternalPtr.h"
#include "UlamTypePrimitiveBits.h"
#include "UlamTypePrimitiveBool.h"
#include "UlamTypePrimitiveInt.h"
#include "UlamTypePrimitiveUnary.h"
#include "UlamTypePrimitiveUnsigned.h"
#include "UlamTypePrimitiveVoid.h"

namespace MFM {

  //#define _DEBUG_OUTPUT
  //#define _INFO_OUTPUT
  #define _WARN_OUTPUT
  #define _WAIT_OUTPUT

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

#ifdef _WAIT_OUTPUT
  static const bool waitOn = true; //debug msg until last resolving loop iteration, then err
#else
  static const bool waitOn = false; //becomes err msg
#endif

  static const char * m_indentedSpaceLevel("  "); //2 spaces per level

  static const char * HIDDEN_ARG_NAME = "ur"; //was Uv_4self, then Uv_4atom
  static const char * HIDDEN_CONTEXT_ARG_NAME = "uc"; //unmangled
  static const char * CUSTOMARRAY_GET_FUNC_NAME = "aref"; //unmangled
  static const char * CUSTOMARRAY_SET_FUNC_NAME = "aset"; //unmangled
  static const char * CUSTOMARRAY_GET_MANGLEDNAME = "Uf_4aref";
  static const char * CUSTOMARRAY_SET_MANGLEDNAME = "Uf_4aset";

  static const char * IS_MANGLED_FUNC_NAME = "internalCMethodImplementingIs"; //Uf_2is
  static const char * IS_MANGLED_FUNC_NAME_FOR_ATOM = "UlamClass<EC>::IsMethod"; //Uf_2is

  static const char * HAS_MANGLED_FUNC_NAME = "PositionOfDataMemberType"; //Uf_3has
  static const char * HAS_MANGLED_FUNC_NAME_FOR_ATOM = "UlamClass<EC>::PositionOfDataMember";

  static const char * GETCLASSLENGTH_FUNCNAME = "GetClassLength";
  static const char * BUILD_DEFAULT_ATOM_FUNCNAME = "BuildDefaultAtom";
  static const char * BUILD_DEFAULT_QUARK_FUNCNAME = "getDefaultQuark";
  static const char * BUILD_DEFAULT_TRANSIENT_FUNCNAME = "getDefaultTransient";

  //use of this in the initialization list seems to be okay;
  CompilerState::CompilerState(): m_linesForDebug(false), m_programDefST(*this), m_parsingLocalDef(false), m_currentFunctionBlockDeclSize(0), m_currentFunctionBlockMaxDepth(0), m_parsingControlLoop(0), m_gotStructuredCommentToken(false), m_parsingConditionalAs(false), m_genCodingConditionalHas(false), m_eventWindow(*this), m_goAgainResolveLoop(false), m_pendingArgStubContext(0), m_currentSelfSymbolForCodeGen(NULL), m_nextTmpVarNumber(0), m_nextNodeNumber(0), m_urSelfUTI(Nouti), m_emptyUTI(Nouti)
  {
    m_err.init(this, debugOn, infoOn, warnOn, waitOn, NULL);
    Token::initTokenMap(*this);
  }

  CompilerState::~CompilerState()
  {
    clearAllDefinedUlamTypes();
    clearAllLinesOfText();
    clearAllLocalsPerFilePath();
    clearCurrentObjSymbolsForCodeGen();
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

  void CompilerState::clearAllLocalsPerFilePath()
  {
    std::map<u32, NodeBlockLocals *>::iterator it;

    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * locals = it->second;
	delete locals;
      }
    m_localsPerFilePath.clear();
  } //clearAllLocalsPerFilePath

  void CompilerState::clearCurrentObjSymbolsForCodeGen()
  {
    m_currentObjSymbolsForCodeGen.clear();
  } //clearCurrentObjSymbolsForCodeGen

  bool CompilerState::getClassNameFromFileName(std::string startstr, u32& compileThisId)
  {
    u32 foundSuffix = startstr.find(".ulam");
    if((foundSuffix == std::string::npos)        //.ulam not found
       || (foundSuffix != startstr.length()-5)   //ensure it's a suffix
       || (foundSuffix == 0))                    //and not also a prefix
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

  UTI CompilerState::makeUlamTypeFromHolder(UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti, ULAMCLASSTYPE classtype)
  {
    //we need to keep the uti, but change the key; utype to be.
    UlamKeyTypeSignature hkey = getUlamKeyTypeSignatureByIndex(uti);
    return makeUlamTypeFromHolder(hkey, newkey, utype, uti, classtype);
  } //makeUlamTypeFromHolder

  UTI CompilerState::makeUlamTypeFromHolder(UlamKeyTypeSignature oldkey, UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti, ULAMCLASSTYPE classtype)
  {
    if((getUlamTypeByIndex(uti)->getUlamTypeEnum() == Class) && isScalar(uti) && !isReference(uti))
      {
	if((utype != Class))
	  {
	    //transfer-basic type from "Class" to Non-class
	    //we need to also remove the name id from program ST
	    u32 oldnameid = oldkey.getUlamKeyTypeSignatureNameId();
	    AssertBool isGone = removeIncompleteClassSymbolFromProgramTable(oldnameid);
	    assert(isGone);
	  }
	else
	  {
	    u32 newnameid = newkey.getUlamKeyTypeSignatureNameId();

	    //let the new class name be represented instead of the old (e.g. typedef)
	    u32 oldnameid = oldkey.getUlamKeyTypeSignatureNameId();
	    SymbolClassName * cnsym = NULL;
	    if(alreadyDefinedSymbolClassName(oldnameid, cnsym))
	      {
		if(!cnsym->isClassTemplate())
		  {
		    SymbolClassName * newcnsym = NULL;
		    if(!alreadyDefinedSymbolClassName(newnameid, newcnsym))
		      {
			cnsym->setId(newnameid);
			m_programDefST.replaceInTable(oldnameid, newnameid, cnsym);
		      }
		    else //if(cnsym != NULL)
		      removeIncompleteClassSymbolFromProgramTable(oldnameid);
		  }
		else
		  {
		    //old is a template, names must be the same
		    assert(oldnameid == newnameid);
		    SymbolClass * csym = NULL;
		    if(((SymbolClassNameTemplate*) cnsym)->findClassInstanceByUTI(uti, csym))
		      {
			//already added into template; nothing to do
		      }
		    else if(alreadyDefinedSymbolClass(uti, csym))
		      {
			//must have found an alias!
			//for a class instance in template
		      }
		    else
		      assert(0); //t3379 wo NodeTypedef updating UTIAlias
		    return csym->getUlamTypeIdx();
		  }
	      }
	    else
	      {
		//oldname only a typedef
		AssertBool isDefined = alreadyDefinedSymbolClassName(newnameid, cnsym);
		assert(isDefined);
	      }
	  }

      }

    //we need to keep the uti, but change the key
    //removes old key and its ulamtype from map, if no longer pointed to
    deleteUlamKeyTypeSignature(oldkey, uti); //decrements counter

    UlamType * newut = NULL;
    UTI auti = Nav;
    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, utype, classtype);

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
  // never get resolved.
  SymbolClassName * CompilerState::makeClassFromHolder(UTI huti, const Token& tok)
  {
    SymbolClassName * cnsym = NULL;
    if(hasUnknownTypeInThisClassResolver(huti))
      {
	Token holdTok = removeKnownTypeTokenFromThisClassResolver(huti);
	if(!alreadyDefinedSymbolClassName(holdTok.m_dataindex, cnsym))
	  {
	    AssertBool isDefined = addIncompleteClassSymbolToProgramTable(holdTok, cnsym);
	    assert(isDefined);
	  }
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
    if(huti == newuti) return; //short-circuit (e.g. t3378) don't use compare; maybe Hzy etyp

    UlamType * newut = getUlamTypeByIndex(newuti);
    UlamKeyTypeSignature newkey = newut->getUlamKeyTypeSignature();
    ULAMTYPE newetyp = newut->getUlamTypeEnum();
    ULAMCLASSTYPE newctyp = newut->getUlamClassType();
    makeUlamTypeFromHolder(newkey, newetyp, huti, newctyp);
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
    AssertBool isCutie = (makeUlamTypeFromHolder(ackey, Class, cuti, UC_UNSEEN) == cuti);
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
  UTI CompilerState::makeUlamType(const Token& typeTok, s32 bitsize, s32 arraysize, UTI classinstanceidx, ALT reftype, ULAMCLASSTYPE classtype)
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

    if(!isDefined(key,ut) || (bitsize == UNKNOWNSIZE) || (arraysize == UNKNOWNSIZE))
      {
	//no key, make new type, how to know baseUT? bitsize?
	uti = makeUlamType(key, bUT, classtype); //returns uti
      }
    else
      {
	AssertBool isDefined = anyDefinedUTI(key,uti);
	assert(isDefined);
      }
    return uti;
  } //makeUlamType

  UTI CompilerState::makeUlamType(UlamKeyTypeSignature key, ULAMTYPE utype, ULAMCLASSTYPE classtype)
  {
    UTI uti;
    UlamType * ut = NULL;

    if(!isDefined(key,ut) || (utype == Class) || ((utype != Class) && (key.getUlamKeyTypeSignatureBitSize() == UNKNOWNSIZE)) || (key.getUlamKeyTypeSignatureArraySize() == UNKNOWNSIZE))
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
		      key.append(uti); //class instance idx
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

	ut = createUlamType(key, utype, classtype);
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

  UlamType * CompilerState::createUlamType(UlamKeyTypeSignature key, ULAMTYPE utype, ULAMCLASSTYPE classtype)
  {
    UlamType * ut = NULL;
    switch(utype)
      {
      case Nouti:
	ut = new UlamTypeInternalNouti(key, *this);
	break;
      case Nav:
	ut = new UlamTypeInternalNav(key, *this);
	break;
      case Hzy:
	ut = new UlamTypeInternalHzy(key, *this);
	break;
      case Void:
	ut = new UlamTypePrimitiveVoid(key, *this);
	break;
      case Int:
	ut = new UlamTypePrimitiveInt(key, *this);
	break;
      case Unsigned:
	ut = new UlamTypePrimitiveUnsigned(key, *this);
	break;
      case Bool:
	ut = new UlamTypePrimitiveBool(key, *this);
	break;
      case Unary:
	ut = new UlamTypePrimitiveUnary(key, *this);
	break;
      case Bits:
	ut = new UlamTypePrimitiveBits(key, *this);
	break;
      case Class:
	{
	  switch(classtype)
	    {
	    case UC_QUARK:
	      ut = new UlamTypeClassQuark(key, *this);
	      break;
	    case UC_ELEMENT:
	      ut = new UlamTypeClassElement(key, *this);
	      break;
	    case UC_TRANSIENT:
	      ut = new UlamTypeClassTransient(key, *this);
	      break;
	    case UC_UNSEEN:
	      ut = new UlamTypeClass(key, *this);
	      break;
	    default:
	      assert(0);
	    };
	}
	break;
      case UAtom:
	ut = new UlamTypeAtom(key, *this);
	break;
      case Ptr:
	ut = new UlamTypeInternalPtr(key, *this);
	break;
      case Holder:
	ut = new UlamTypeInternalHolder(key, *this);
	break;
      case LocalsFileScope:
	ut = new UlamTypeInternalLocalsFileScope(key, *this);
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

  bool CompilerState::replaceUlamTypeForUpdatedClassType(UlamKeyTypeSignature key, ULAMTYPE etype, ULAMCLASSTYPE classtype, bool isCArray)
  {
    bool rtnBool = false;
    std::map<UlamKeyTypeSignature, UlamType *, less_than_key>::iterator it = m_definedUlamTypes.find(key);
    if(it != m_definedUlamTypes.end())
      {
	assert(key == it->first);

	//no change to key or UTI; only ULAMCLASSTYPE
	UlamType * newut = createUlamType(key, etype, classtype);
	assert(newut);
	if(isCArray)
	  ((UlamTypeClass *) newut)->setCustomArray();

	UlamType * ut = it->second;
	delete ut;
	it->second = newut;
	rtnBool = true;
      }
    return rtnBool;
  } //replaceUlamTypeForUpdatedClassType

  // no side-effects, except to 3rd arg when return is true.
  bool CompilerState::mappedIncompleteUTI(UTI cuti, UTI auti, UTI& mappedUTI)
  {
    if(!isAClass(cuti))
      {
	//a local scope (t3862)
	return findaUTIAlias(auti, mappedUTI);
      }

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
      mappedUTI = auti; //auti no longer a holder
      return true;
    }

    if(!isHolder(auti))
      {
	UlamType * aut = getUlamTypeByIndex(auti);

	if(aut->isReference()) //not a special case
	  {
	    UTI refofuti = aut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureClassInstanceIdx();
	    if(!correctAReferenceTypeWith(auti, refofuti))
	      return false;
	    aut = getUlamTypeByIndex(auti);
	  }
	else if(!aut->isScalar())
	  {
	    UTI scalarofuti = aut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureClassInstanceIdx();
	    if(!correctAnArrayTypeWith(auti, scalarofuti))
	      return false;
	    aut = getUlamTypeByIndex(auti);
	  }

	UlamKeyTypeSignature akey = aut->getUlamKeyTypeSignature();
	u32 anameid = akey.getUlamKeyTypeSignatureNameId();

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
#if 0
	//Type is not a Class, e.g. Unsigned, no lookup!
	else if(aut->isScalar())
	  {
	    Symbol * symOfIncomplete = NULL;
	    bool tmphzykin = false;

	    if(alreadyDefinedSymbol(anameid, symOfIncomplete, tmphzykin))
	      {
		mappedUTI = symOfIncomplete->getUlamTypeIdx();
		return true;
	      }
	  }
#endif
	//else?
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

    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassName(getCompileThisId(), cnsym);
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
    ULAMCLASSTYPE sclasstype = sut->getUlamClassType(); //restore from original ut
    UTI newuti = makeUlamType(newkey, bUT, sclasstype);

    cnsym->mapInstanceUTI(getCompileThisIdx(), suti, newuti);

    if(bUT == Class)
      {
	UlamType * newut = getUlamTypeByIndex(newuti);
	if(sut->isCustomArray())
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
    //updateUTIAlias(suti, newuti); //what if..
    return newuti;
  } //mapIncompleteUTIForCurrentClassInstance

  void CompilerState::mapTypesInCurrentClass(UTI fm, UTI to)
  {
    UTI cuti = getCompileThisIdx();
    if(!isAClass(cuti))
      return; //a local def, skip

    if(getReferenceType(to) != ALT_NOT)
      return; //only a reference to a class type, skip

    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassName(getCompileThisId(), cnsym);
    assert(isDefined);
    cnsym->mapInstanceUTI(cuti, fm, to);
    //when not a template, the CompileThisId && Idx belong to the same SymbolClassName;
    // o.w. CompileThisIdx belongs to a class instance of the template (not an UTIAlias!).
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
	assert(0);
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

  // returns effective self name of the scalar dereferenced uti arg.
  const std::string CompilerState::getEffectiveSelfMangledNameByIndex(UTI uti)
  {
    UTI esuti = uti;
    if(!isScalar(uti))
      esuti = getUlamTypeAsScalar(uti);

    if(isReference(esuti))
      esuti = getUlamTypeAsDeref(esuti);

    UlamType * esut = NULL;
    AssertBool isDef = isDefined(m_indexToUlamKey[esuti], esut);
    assert(isDef);

    assert(esut->getUlamTypeEnum() == Class);
    std::ostringstream esmangled;
    esmangled << esut->getUlamTypeMangledName().c_str();
    esmangled << "<EC>::THE_INSTANCE";
    return esmangled.str();
  } //getEffectiveSelfMangledNameByIndex

  ULAMTYPE CompilerState::getBaseTypeFromToken(const Token& tok)
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

  UTI CompilerState::getUlamTypeFromToken(const Token& tok, s32 typebitsize, s32 arraysize)
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

  UTI CompilerState::getUlamTypeAsScalar(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    if(ut->isScalar())
      return utiArg;

    //for typedef array, the scalar is the primitive type
    // maintained in the symbol!! can't get to it from utarg.
    ULAMTYPE bUT = ut->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();
    UTI cuti = keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx(); // what-if a ref?

    if(bUT == Class)
      return cuti; //try this Mon May  2 10:36:56 2016

    s32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    u32 nameid = keyOfArg.getUlamKeyTypeSignatureNameId();
    UlamKeyTypeSignature baseKey(nameid, bitsize, NONARRAYSIZE, cuti, ALT_ARRAYITEM);  //default array size is NONARRAYSIZE, new reftype
    ULAMCLASSTYPE classtype = ut->getUlamClassType();
    UTI buti = makeUlamType(baseKey, bUT, classtype); //could be a new one, oops.

    //note: array of CA's is not a CA too
    return buti;
  } //getUlamTypeAsScalar

  UTI CompilerState::getUlamTypeAsDeref(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    if(!ut->isReference())
      return utiArg;

    ULAMTYPE bUT = ut->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();
    s32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    s32 arraysize = keyOfArg.getUlamKeyTypeSignatureArraySize();
    UTI classidx = keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx();

    if((bUT == Class) && ut->isScalar())
      return classidx; //scalar

    UlamKeyTypeSignature baseKey(keyOfArg.m_typeNameId, bitsize, arraysize, classidx, ALT_NOT);  //default array size is zero
    ULAMCLASSTYPE classtype = ut->getUlamClassType();
    UTI buti = makeUlamType(baseKey, bUT, classtype); //could be a new one, oops.
    if(ut->isCustomArray())
      {
	assert(bUT == Class);
	((UlamTypeClass *) getUlamTypeByIndex(buti))->setCustomArray();
      }
    return buti;
  } //getUlamTypeAsDeref

  UTI CompilerState::getUlamTypeAsRef(UTI utiArg)
  {
    return getUlamTypeAsRef(utiArg, ALT_REF);
  } //getUlamTypeAsRef

  UTI CompilerState::getUlamTypeAsRef(UTI utiArg, ALT altArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    ALT utalt = ut->getReferenceType();

    if(utalt == altArg)
      return utiArg; //same ref type

    //e.g. a ref typedef
    if((utalt != ALT_NOT) && (altArg == ALT_NOT))
      return utiArg; //deref used to remove alt type

    if((utalt != ALT_NOT) && (utalt != altArg))
      {
	std::ostringstream msg;
	msg << "Attempting to ref (" << altArg << ") a reference type <" ;
	msg <<  getUlamTypeNameByIndex(utiArg) << ">";
	MSG2("", msg.str().c_str(), DEBUG);
	//assert(0); //didn't hit during testing
	//continue..
      }

    ULAMTYPE bUT = ut->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();

    s32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    s32 arraysize = keyOfArg.getUlamKeyTypeSignatureArraySize();
    UTI classidx = keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx();
    u32 nameid = keyOfArg.m_typeNameId;
    UlamKeyTypeSignature baseKey(nameid, bitsize, arraysize, classidx, altArg);  //default array size is zero
    ULAMCLASSTYPE classtype = ut->getUlamClassType();
    //for now, a reference to a custom array is not itself a custom array.
    UTI buti = makeUlamType(baseKey, bUT, classtype); //could be a new one, oops.
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
    return makeUlamType(ckey, etype, UC_NOTACLASS); //may not exist yet, create
  } //getUlamTypeOfConstant

  UTI CompilerState::getDefaultUlamTypeOfConstant(UTI ctype)
  {
    return ctype; // use its own type
  } //getDefaultUlamTypeOfConstant

  bool CompilerState::getDefaultQuark(UTI cuti, u32& dqref)
  {
    if(!okUTItoContinue(cuti)) return false; //short-circuit

    bool rtnb = false;
    if(getUlamTypeByIndex(cuti)->getUlamClassType() == UC_QUARK)
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

  bool CompilerState::getPackedDefaultClass(UTI auti, u64& dpkref)
  {
    UlamType * aut = getUlamTypeByIndex(auti);
    assert(aut->getUlamTypeEnum() == Class);

    UTI scalaruti = getUlamTypeAsScalar(auti);

    PACKFIT packFit = determinePackable(scalaruti);
    if(packFit != PACKEDLOADABLE) return false; //scalar fits in a long

    if(!okUTItoContinue(auti)) return false; //short-circuit

    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(scalaruti, csym);
    assert(isDefined);
    return csym->getPackedDefaultValue(dpkref); //might be zero
  } //getPackedDefaultClass

  void CompilerState::getDefaultAsPackedArray(UTI auti, u64 dval, u64& darrval)
  {
    assert(okUTItoContinue(auti));
    darrval = 0; //return ref init
    if(dval == 0)
      return;

    UlamType * aut = getUlamTypeByIndex(auti);
    u32 len = aut->getTotalBitSize();
    u32 bitsize = aut->getBitSize();
    u32 arraysize = aut->getArraySize();
    arraysize = arraysize > 0 ? arraysize : 1;
    u32 pos = 0;
    getDefaultAsPackedArray(len, bitsize, arraysize, pos, dval, darrval);
  } //getDefaultAsPackedArray

  void CompilerState::getDefaultAsPackedArray(u32 len, u32 bitsize, u32 arraysize, u32 pos, u64 dval, u64& darrval)
  {
    darrval = dval; //return
    if(dval == 0)
      return;

    u64 mask = _GetNOnes64(bitsize);
    dval &= mask;

    for(u32 j = 1; j < arraysize; j++)
      darrval |= (dval << (len - (pos + (j * bitsize))));
  } //getDefaultAsPackedArray

  bool CompilerState::getDefaultClassValue(UTI cuti, BV8K& dvref)
  {
    if(!okUTItoContinue(cuti)) return false; //short-circuit

    UlamType * cut = getUlamTypeByIndex(cuti);
    assert(cut->getUlamTypeEnum() == Class);

    bool rtnb = true;
    if(cut->getBitSize() > 0)
      {
	UTI scalarcuti = getUlamTypeAsScalar(cuti);
	SymbolClass * csym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClass(scalarcuti, csym);
	assert(isDefined);
	rtnb = csym->getDefaultValue(dvref); //pass along ref
      }
    return rtnb;
  } //getDefaultClassValue

  void CompilerState::getDefaultAsArray(u32 bitsize, u32 arraysize, u32 tpos, const BV8K& dval, BV8K& darrval)
  {
    for(u32 j = 0; j < arraysize; j++)
      dval.CopyBV(0u, tpos + (j * bitsize), bitsize, darrval); //frompos, topos, len, destBV
  } //getDefaultAsArray

  bool CompilerState::genCodeClassDefaultConstantArray(File * fp, u32 len, BV8K& dval)
  {
    if(len == 0)
      return false;

    u32 uvals[ARRAY_LEN8K];
    dval.ToArray(uvals);

    u32 nwords = (len + 31)/MAXBITSPERINT;

    //short-circuit if all zeros
    bool isZero = true;
    for(u32 x = 0; x < nwords; x++)
      {
	if(uvals[x] != 0)
	  {
	    isZero = false;
	    break;
	  }
      }

    if(isZero && (getUlamTypeByIndex(getCompileThisIdx())->getUlamClassType() != UC_TRANSIENT))
      return false; //nothing to do


    //build static constant array of u32's for BV8K:
    indent(fp);
    fp->write("static const u32 vales[(");
    fp->write_decimal_unsigned(len); // == [nwords]
    fp->write(" + 31)/32] = { ");

    for(u32 w = 0; w < nwords; w++)
      {
	std::ostringstream dhex;
	dhex << "0x" << std::hex << uvals[w];

	if(w > 0)
	  fp->write(", ");

	fp->write(dhex.str().c_str());
      }
    fp->write(" };"); GCNL;

    // declare perfect size BV with constant array of defaults BV8K u32's
    indent(fp);
    fp->write("static BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("> initBV(vales);"); GCNL;
    return true;
  } //genCodeClassDefaultConstantArray

  bool CompilerState::isScalar(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return (ut->isScalar());
  }

  s32 CompilerState::getArraySize(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return (ut->getArraySize());
  }

  s32 CompilerState::getBitSize(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return (ut->getBitSize());
  }

  ALT CompilerState::getReferenceType(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return ut->getReferenceType();
  }

  bool CompilerState::isReference(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return ut->isReference();
  }

  bool CompilerState::correctAReferenceTypeWith(UTI utiArg, UTI derefuti)
  {
    //correcting a reference with a corrected deref'd type
    UlamType * derefut = getUlamTypeByIndex(derefuti);
    if(derefut->isComplete())
      {
	if(getUlamTypeByIndex(utiArg)->getUlamTypeEnum() != derefut->getUlamTypeEnum())
	  {
	    assert(0); //shouldn't happen now that we don't 'assumeAClassType' (t3668, t3651)

	    UlamKeyTypeSignature dekey = derefut->getUlamKeyTypeSignature();
	    UlamKeyTypeSignature newkey(dekey.getUlamKeyTypeSignatureNameId(), dekey.getUlamKeyTypeSignatureBitSize(), dekey.getUlamKeyTypeSignatureArraySize(), derefuti, ALT_REF);
	    makeUlamTypeFromHolder(newkey, derefut->getUlamTypeEnum(), utiArg, derefut->getUlamClassType());
	  }
	return true;
      }
    return false;
  } //correctAReferenceTypeWith

  bool CompilerState::correctAnArrayTypeWith(UTI utiArg, UTI scalaruti)
  {
    //correcting a reference with a corrected scalar type
    UlamType * scalarut = getUlamTypeByIndex(scalaruti);
    if(scalarut->isComplete())
      {
	if(getUlamTypeByIndex(utiArg)->getUlamTypeEnum() != scalarut->getUlamTypeEnum())
	  {
	    assert(0); //shouldn't happen now that we don't 'assumeAClassType' (t3668, t3651)

	    UlamKeyTypeSignature sckey = scalarut->getUlamKeyTypeSignature();
	    UlamType * arrut = getUlamTypeByIndex(utiArg);
	    UlamKeyTypeSignature newkey(sckey.getUlamKeyTypeSignatureNameId(), sckey.getUlamKeyTypeSignatureBitSize(), arrut->getArraySize(), scalaruti, ALT_NOT);
	    makeUlamTypeFromHolder(newkey, scalarut->getUlamTypeEnum(), utiArg, scalarut->getUlamClassType());
	  }
	return true;
      }
    return false;
  } //correctAnArrayTypeWith

  bool CompilerState::isComplete(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return ut->isComplete();
  } //isComplete

  bool CompilerState::completeAReferenceType(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    assert(ut->isReference());

    if(ut->isComplete())
      return true;

    UTI derefuti = getUlamTypeAsDeref(utiArg);
    UlamType * derefut = getUlamTypeByIndex(derefuti);

    if(!derefut->isComplete())
      return false;

    return completeAReferenceTypeWith(utiArg, derefuti);
  } //completeAReferenceType

  bool CompilerState::completeAReferenceTypeWith(UTI utiArg, UTI derefuti)
  {
    UlamType * derefut = getUlamTypeByIndex(derefuti);

    if(!derefut->isComplete())
      return false;

    if(isHolder(utiArg))
      {
	UlamKeyTypeSignature dekey = derefut->getUlamKeyTypeSignature();
	UlamKeyTypeSignature newkey(dekey.getUlamKeyTypeSignatureNameId(), dekey.getUlamKeyTypeSignatureBitSize(), dekey.getUlamKeyTypeSignatureArraySize(), dekey.getUlamKeyTypeSignatureClassInstanceIdx(), ALT_REF);
	makeUlamTypeFromHolder(newkey, derefut->getUlamTypeEnum(), utiArg, derefut->getUlamClassType());
	return true;
      }

    AssertBool sized = setUTISizes(utiArg, derefut->getBitSize(), derefut->getArraySize());
    assert(sized);

    UlamType * ut = getUlamTypeByIndex(utiArg);
    if(!ut->isComplete())
      {
	ULAMCLASSTYPE classtype = ut->getUlamClassType();
	if( classtype != UC_NOTACLASS)
	  {
	    assert(classtype == UC_UNSEEN);
	    replaceUlamTypeForUpdatedClassType(ut->getUlamKeyTypeSignature(), Class, derefut->getUlamClassType(), derefut->isCustomArray()); //e.g. error/t3763
	  }
	else
	  assert(0); //why not!!?
      }
    return getUlamTypeByIndex(utiArg)->isComplete();
  } //completeAReferenceTypeWith

  bool CompilerState::isHolder(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return (ut->isHolder());
  }

  //updates key. we can do this now that UTI is used and the UlamType * isn't saved;
  // return false if ERR
  bool CompilerState::setBitSize(UTI utiArg, s32 bits)
  {
    return setUTISizes(utiArg, bits, getArraySize(utiArg)); //keep current arraysize
  }

  // return false if ERR
  bool CompilerState::setUTISizes(UTI utiArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);

    if(ut->isComplete())
      return true;

    if(!okUTItoContinue(utiArg))
      return false;

    //redirect primitives and arrays (including class arrays)
    if(ut->isPrimitiveType() || !ut->isScalar())
      {
	return setSizesOfNonClassAndArrays(utiArg, bitsize, arraysize);
      }

    //bitsize could be UNKNOWN or CONSTANT (negative)
    s32 total = bitsize * (arraysize >= 0 ? arraysize : 1); //? allow zero arraysize..Mon Jul  4 13:56:27 2016
    bool isCustomArray = ut->isCustomArray();
    ULAMCLASSTYPE classtype = ut->getUlamClassType();

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

    if(classtype == UC_TRANSIENT)
      {
	if(total > MAXBITSPERTRANSIENT)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXBITSPERTRANSIENT << ") for transient ";
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
    deleteUlamKeyTypeSignature(key, utiArg);

    UlamType * newut = NULL;
    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, Class, classtype);
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType *>(newkey,newut));

	if(isCustomArray)
	  ((UlamTypeClass *) newut)->setCustomArray();
      }

    m_indexToUlamKey[utiArg] = newkey;

    incrementKeyToAnyUTICounter(newkey, utiArg); //here

    {
      std::ostringstream msg;
      msg << "Sizes SET for Class: " << newut->getUlamTypeName().c_str() << " (UTI" << utiArg << ")";
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
  } //updateUTIAliasForced

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
  // for primitives, and all arrays (class and nonclass)
  bool CompilerState::setSizesOfNonClassAndArrays(UTI utiArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    assert((ut->getUlamClassType() == UC_NOTACLASS) || !ut->isScalar());

    if(ut->isComplete())
      return true;  //nothing to do

    ULAMTYPE bUT = ut->getUlamTypeEnum();
    //disallow zero-sized primitives (no such thing as a BitVector<0u>)
    //'Void' by default is zero and only zero bitsize (already complete)
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
    ULAMCLASSTYPE classtype = ut->getUlamClassType();
    // checking for max size
    u32 total =  (arraysize < 0 ? 1 : arraysize);//ut->getTotalBitSize() not ready
    total *= (bitsize < 0 ? 0 : bitsize);

    if((bUT != Class) && ((key.getUlamKeyTypeSignatureBitSize() == 0) || (bitsize <= 0)))
      {
	if((bUT != Void) || (bitsize < 0))
	  {
	    std::ostringstream msg;
	    msg << "Invalid size (";
	    msg << bitsize;
	    msg << ") to set for primitive type: " << UlamType::getUlamTypeEnumAsString(bUT);
	    if(arraysize < 0)
	      {
		MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
		return false;
	      }
	    else
	      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	    //return false;
	  }
	//Void with zero bitsize
	if((bUT == Void) && (arraysize != NONARRAYSIZE))
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
    else if(total > MAXBITSPERTRANSIENT)
      {
	std::ostringstream msg;
	msg << "Trying to exceed allotted bit size (" << MAXBITSPERTRANSIENT << ") for array ";
	msg << ut->getUlamTypeNameBrief().c_str() << " with " << total << " bits";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	return false;
      }
    //else
    //continue with valid number of bits
    UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key.getUlamKeyTypeSignatureNameId(), bitsize, arraysize, key.getUlamKeyTypeSignatureClassInstanceIdx(), key.getUlamKeyTypeSignatureReferenceType());

    if(key == newkey)
      return true;

    //remove old key from map, if no longer pointed to by any UTIs
    deleteUlamKeyTypeSignature(key, utiArg);

    UlamType * newut = NULL;
    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, bUT, classtype); //possibly a class array
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType*>(newkey,newut));
      }

    m_indexToUlamKey[utiArg] = newkey;

    incrementKeyToAnyUTICounter(newkey, utiArg);

    {
      std::ostringstream msg;
      msg << "Sizes set for nonClass: " << newut->getUlamTypeName().c_str();
      msg << " (UTI" << utiArg << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
    return true;
  } //setSizesOfNonClassAndArrays

  s32 CompilerState::getDefaultBitSize(UTI uti)
  {
    ULAMTYPE et = getUlamTypeByIndex(uti)->getUlamTypeEnum();
    return ULAMTYPE_DEFAULTBITSIZE[et];
  }

  u32 CompilerState::getTotalBitSize(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return (ut->getTotalBitSize());
  }

  u32 CompilerState::getTotalWordSize(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
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
    if(!isAClass(cuti))
      return false;

    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->isClassTemplate(cuti);
  } //isClassATemplate

  UTI CompilerState::isClassASubclass(UTI cuti)
  {
    UTI subuti = getUlamTypeAsDeref(getUlamTypeAsScalar(cuti)); //in case of array

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
    UTI subuti = getUlamTypeAsDeref(getUlamTypeAsScalar(cuti)); //in case of array

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
    UTI derefsuperp = getUlamTypeAsDeref(superp);
    bool rtnb = false;
    UTI prevuti = getUlamTypeAsDeref(cuti); //init for the loop
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
	    rtnb = (prevuti != Nouti) && (UlamType::compare(derefsuperp, prevuti, *this) == UTIC_SAME); //compare (don't compare if Nouti)
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
    bool rtnb = alreadyDefinedSymbolClassName(dataindex, (SymbolClassName *&) symptr);
    if(rtnb && !((SymbolClassName *) symptr)->isClassTemplate())
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
		//template may not be aware of this mapping (e.g. t3379)
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
  void CompilerState::addUnknownTypeTokenToAClassResolver(UTI cuti, const Token& tok, UTI huti)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    csym->addUnknownTypeTokenToClass(tok, huti);
  } //addUnknownTypeTokenToThisClassResolver

  //temporary type name which will be updated during type labeling.
  void CompilerState::addUnknownTypeTokenToThisClassResolver(const Token& tok, UTI huti)
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
#if 1
    //just this class (original way)
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->statusUnknownTypeInClass(huti);
#else
    //all template class instances with same name
    UlamType * cut = getUlamTypeByIndex(cuti);
    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassName(cut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), cnsym);
    assert(isDefined);
    return cnsym->statusUnknownTypeInClassInstances(huti);
#endif
  } //statusUnknownTypeInThisClassResolver

  bool CompilerState::removeIncompleteClassSymbolFromProgramTable(u32 id)
  {
    Token ntok(TOK_IDENTIFIER, m_locOfNextLineText, id); //junk loc
    return removeIncompleteClassSymbolFromProgramTable(ntok);
  }

  bool CompilerState::removeIncompleteClassSymbolFromProgramTable(const Token& nTok)
  {
    bool rtnb = false;
    u32 id = nTok.m_dataindex;
    SymbolClassName * cnsym = NULL;
    if(alreadyDefinedSymbolClassName(id, cnsym))
      {
	UTI cuti = cnsym->getUlamTypeIdx();
	UlamType * cut = getUlamTypeByIndex(cuti);
	assert(cut->getUlamClassType() == UC_UNSEEN);

	UlamKeyTypeSignature ckey = cut->getUlamKeyTypeSignature();

	deleteUlamKeyTypeSignature(ckey, cuti); //decrements counter

	resetUnseenClass(cnsym, nTok); //before removing it from programDefST

	Symbol * rmcnsym  = NULL;
	AssertBool isGone = m_programDefST.removeFromTable(id, rmcnsym);
	assert(isGone);
	assert(rmcnsym == cnsym);
	delete rmcnsym;
	rmcnsym = cnsym = NULL;
	rtnb = true;
      }
    return rtnb;
  } //removeIncompleteClassSymbolFromProgramTable

  //temporary UlamType which will be updated during type labeling.
  bool CompilerState::addIncompleteClassSymbolToProgramTable(const Token& cTok, SymbolClassName * & symptr)
  {
    u32 dataindex = cTok.m_dataindex;
    bool isNotDefined = (symptr == NULL) && !alreadyDefinedSymbolClassName(dataindex, symptr);
    if(!isNotDefined)
      {
	std::ostringstream msg;
	msg << "Unseen Class '" << m_pool.getDataAsString(dataindex).c_str();
	msg << "' was already seen";
	MSG2(&cTok, msg.str().c_str(), ERR);
	assert(0);
	return false;
      }

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE); //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class, UC_UNSEEN); //**gets next unknown uti type

    NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
    assert(classblock);
    //classblock->setNodeLocation(cTok.m_locator); only where first used, not defined!
    classblock->setNodeType(cuti);

    //avoid Invalid Read whenconstructing class' Symbol
    pushClassContext(cuti, classblock, classblock, false, NULL); //null blocks likely

    //symbol ownership goes to the programDefST; distinguish btn template & regular classes here:
    symptr = new SymbolClassName(cTok, cuti, classblock, *this);
    m_programDefST.addToTable(dataindex, symptr);
    m_unseenClasses.insert(dataindex);

    popClassContext();
    return true; //compatible with alreadyDefinedSymbolClassName return
  } //addIncompleteClassSymbolToProgramTable

  //temporary UlamType which will be updated during type labeling.
  bool CompilerState::addIncompleteTemplateClassSymbolToProgramTable(const Token& cTok, SymbolClassNameTemplate * & symptr)
  {
    u32 dataindex = cTok.m_dataindex;
    AssertBool isNotDefined = ((symptr == NULL) && !alreadyDefinedSymbolClassNameTemplate(dataindex,symptr));
    assert(isNotDefined);

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE); //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class, UC_UNSEEN); //**gets next unknown uti type

    NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
    assert(classblock);
    classblock->setNodeLocation(cTok.m_locator);
    classblock->setNodeType(cuti);

    //avoid Invalid Read whenconstructing class' Symbol
    pushClassContext(cuti, classblock, classblock, false, NULL); //null blocks likely

    //symbol ownership goes to the programDefST; distinguish btn template & regular classes here:
    symptr = new SymbolClassNameTemplate(cTok, cuti, classblock, *this);
    m_programDefST.addToTable(dataindex, symptr);
    m_unseenClasses.insert(dataindex);
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

    ULAMCLASSTYPE superclasstype = superut->getUlamClassType();
    assert((superclasstype != UC_UNSEEN) && (superclasstype != UC_ELEMENT)); //quark or transient
    UlamKeyTypeSignature newstubkey(superid, UNKNOWNSIZE); //"-2" and scalar default
    UTI newstubcopyuti = makeUlamType(newstubkey, Class, superclasstype); //**gets next unknown uti type
    superctsym->copyAStubClassInstance(superuti, newstubcopyuti, context);
    superctsym->mergeClassInstancesFromTEMP(); //not mid-iteration!!
    return newstubcopyuti;
  } //addStubCopyToAncestorClassTemplate

  void CompilerState::resetUnseenClass(SymbolClassName * cnsym, const Token& identTok)
  {
    if(m_unseenClasses.empty())
      return;

    std::set<u32>::iterator it = m_unseenClasses.find(identTok.m_dataindex);
    if(it != m_unseenClasses.end())
      {
	m_unseenClasses.erase(it);
      }
    cnsym->resetUnseenClassLocation(identTok);
  } //resetUnseenClass

  bool CompilerState::getUnseenClassFilenames(std::vector<std::string>& unseenFiles)
  {
    std::set<u32>::iterator it = m_unseenClasses.begin();
    while(it != m_unseenClasses.end())
      {
	u32 id = *it;
	//excludes anonymous classes, only unseen classes with known names
	std::ostringstream fn;
	fn << m_pool.getDataAsString(id).c_str() << ".ulam";
	unseenFiles.push_back(fn.str());
	it++;
      } //while
    m_unseenClasses.clear(); //avoid looping over those that don't exist.
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
    assert(ict->getUlamClassType() == UC_UNSEEN); //missing, may be array
    if(alreadyDefinedSymbolClass(incomplete, csym))
      {
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClassName(csym->getId(), cnsym);
	assert(isDefined);
	UTI cuti = cnsym->getUlamTypeIdx();
	assert(okUTItoContinue(cuti));
	UlamType * cut = getUlamTypeByIndex(cuti);
	//e.g. out-of-scope typedef is not a class, return false
	if(isASeenClass(cuti))
	  {
	    AssertBool isReplaced = replaceUlamTypeForUpdatedClassType(cut->getUlamKeyTypeSignature(), Class, cut->getUlamClassType(), cut->isCustomArray());
	    assert(isReplaced);
	    cut = getUlamTypeByIndex(cuti); //update (e.g. t3832)

	    if((cut->getBitSize() == UNKNOWNSIZE) || (cut->getArraySize() == UNKNOWNSIZE))
	      {
		std::ostringstream msg;
		msg << "Class Instance for typedef: " << ict->getUlamTypeName().c_str();
		msg << "(UTI" << cuti << ") - sizes still unknown";
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

  void CompilerState::updateLineageAndFirstCheckAndLabelPass()
  {
    m_programDefST.updateLineageForTableOfClasses(); //+ first c&l
    updateLineageAndFirstCheckAndLabelPassForLocals();
  }

  void CompilerState::updateLineageAndFirstCheckAndLabelPassForLocals()
  {
    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * locals = it->second;
	assert(locals);

	Locator nloc = locals->getNodeLocation();
	u32 pathidx = nloc.getPathIndex();
	u32 nidFromLoc;

	AssertBool isFileName = getClassNameFromFileName(m_pool.getDataAsString(pathidx), nidFromLoc);
	assert(isFileName);

	//get name-sake class UTI from node loc's path id
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClassName(nidFromLoc, cnsym);
	assert(isDefined);
	NodeBlockClass * cblock = cnsym->getClassBlockNode();
	assert(cblock);

	UTI cuti = cnsym->getUlamTypeIdx();

	pushClassContext(cuti, locals, cblock, false, NULL);

	locals->updateLineage(0);
	locals->checkAndLabelType();

	popClassContext(); //restore
      }
  } //updateLineageAndFirstCheckAndLabelPassForLocals

  bool CompilerState::checkAndLabelPassForLocals()
  {
    clearGoAgain();

    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * locals = it->second;

	Locator nloc = locals->getNodeLocation();
	u32 pathidx = nloc.getPathIndex();
	u32 nidFromLoc;

	AssertBool isFileName = getClassNameFromFileName(m_pool.getDataAsString(pathidx), nidFromLoc);
	assert(isFileName);

	//get name-sake class UTI from node loc's path id
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClassName(nidFromLoc, cnsym);
	assert(isDefined);
	NodeBlockClass * cblock = cnsym->getClassBlockNode();
	assert(cblock);

	UTI cuti = cnsym->getUlamTypeIdx();

	pushClassContext(cuti, locals, cblock, false, NULL);

	locals->checkAndLabelType();

	popClassContext(); //restore
      }
    return (!goAgain() && (m_err.getErrorCount() + m_err.getWarningCount() == 0));
  } //checkAndLabelPassForLocals

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
	  {
	    if(!alreadyDefinedSymbolByAncestor(dataindex, symptr, hasHazyKin))
	      {
		popClassContext(); //restore, if false on recursion
		return false;
	      }
	  }
	popClassContext(); //restore, after
	hasHazyKin = tmphzykin;
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
	hasHazyKin |= checkHasHazyKin(blockNode);
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
    NodeBlockContext * cblock = getContextBlock();

    //substitute another selected class block to search for data member
    if(useMemberBlock())
      cblock = getCurrentMemberClassBlock();

    Locator cloc;
    if(cblock)
      cloc = cblock->getNodeLocation(); //to check local scope

    while(!brtn && cblock)
      {
	brtn = cblock->isIdInScope(dataindex,symptr); //returns symbol
	hasHazyKin |= checkHasHazyKin(cblock); //self is stub
	cblock = cblock->isAClassBlock() ? ((NodeBlockClass *) cblock)->getSuperBlockPointer() : NULL; //inheritance chain
      }

    //search current class file scope only (not ancestors')
    if(!brtn)
      brtn = isIdInLocalFileScope(cloc, dataindex, symptr); //local constant or typedef

    return brtn;
  } //isDataMemberIdInClassScope

  bool CompilerState::isIdInLocalFileScope(Locator loc, u32 id, Symbol *& symptr)
  {
    bool brtn = false;
    NodeBlockLocals * locals = getLocalScopeBlock(loc);
    if(locals)
      brtn = locals->isIdInScope(id, symptr);
    return brtn;
  } //isIdInLocalFileScope

  bool CompilerState::isFuncIdInClassScope(u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    bool brtn = false;
    //assert(!hasHazyKin); might come from alreadyDefinedSymbol now, and have a hazy chain.

    //start with the current class block and look up family tree
    //until the 'variable id' is found.
    NodeBlockContext * cblock = getContextBlock();

    //substitute another selected class block to search for data member
    if(useMemberBlock())
      cblock = getCurrentMemberClassBlock();

    if(cblock && !cblock->isAClassBlock())
      return false;

    while(!brtn && cblock)
      {
	brtn = ((NodeBlockClass *) cblock)->isFuncIdInScope(dataindex,symptr); //returns symbol
	hasHazyKin |= checkHasHazyKin(cblock); //self is stub
	cblock = ((NodeBlockClass *) cblock)->getSuperBlockPointer(); //inheritance chain
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

  bool CompilerState::isIdInCurrentScope(u32 id, Symbol *& asymptr)
  {
    bool brtn = false;
    //also applies when isParsingLocalDef()
    brtn = getCurrentBlock()->isIdInScope(id, asymptr);
    return brtn;
  } //isIdInCurrentScope

  //symbol ownership goes to the current block (end of vector)
  void CompilerState::addSymbolToCurrentScope(Symbol * symptr)
  {
    //also applies when isParsingLocalDef()
    getCurrentBlock()->addIdToScope(symptr->getId(), symptr);
  }

  void CompilerState::addSymbolToLocalScope(Symbol * symptr, Locator loc)
  {
    assert(symptr);
    NodeBlockLocals * locals = makeLocalScopeBlock(loc); //getLocalScopeLocator
    assert(locals);

    locals->addIdToScope(symptr->getId(), symptr);
  } //addSymbolToLocalScope

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
  const std::string CompilerState::getTokenLocationAsString(const Token * tok)
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

  const std::string CompilerState::getTokenDataAsString(const Token & tok)
  {
    //assert(tok);
    if(tok.m_dataindex > 0)
      {
	return m_pool.getDataAsString(tok.m_dataindex);
      }
    //return std::string(tok->getTokenString()); //VG: Invalid Read
    return tok.getTokenStringFromPool(this);
  }

  std::string CompilerState::getDataAsStringMangled(u32 dataindex)
  {
    std::ostringstream mangled;
    std::string nstr = m_pool.getDataAsString(dataindex);
    mangled << ToLeximited(nstr);
    return mangled.str();
  } //getDataAsStringMangled

  //does it check for existence?
  const std::string CompilerState::getTokenAsATypeName(const Token& tok)
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
	      return getTokenDataAsString(tok); //a class
	  }
      }
    return "Nav";
  } //getTokenAsATypeName

  u32 CompilerState::getTokenAsATypeNameId(const Token& tok)
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
	if((it != Void) && !fsym->isNativeFunctionDeclaration() && (!fsym->isVirtualFunction() || !fsym->isPureVirtualFunction()))
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

	if(UlamType::compareForArgumentMatching(rType, it, *this) != UTIC_SAME)
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

		if((fsym->getId() == m_pool.getIndexForDataString("toInt")) && (it == Int))
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

  void CompilerState::indentUlamCode(File * fp)
  {
    outputLineNumberForDebugging(fp, m_locOfNextLineText);
    indent(fp);
  } //indentUlamCode

  void CompilerState::indent(File * fp)
  {
    // NO outputLineNumberForDebugging
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
    ULAMCLASSTYPE rclasstype = getUlamTypeByIndex(rtype)->getUlamClassType();
    if(rclasstype == UC_QUARK)
      return getIsMangledFunctionName(ltype);
    else if (rclasstype == UC_ELEMENT)
      return IS_MANGLED_FUNC_NAME;
    else if (rclasstype == UC_TRANSIENT)
      return IS_MANGLED_FUNC_NAME;
    else
      assert(0);

    return "AS_ERROR";
  } //getAsMangledFunctionName

  const char * CompilerState::getClassLengthFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETCLASSLENGTH_FUNCNAME;
  } //getClassLengthFunctionName

  const char * CompilerState::getBuildDefaultAtomFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    ULAMCLASSTYPE lclasstype = getUlamTypeByIndex(ltype)->getUlamClassType();
    if(lclasstype == UC_ELEMENT)
      return BUILD_DEFAULT_ATOM_FUNCNAME; //for elements
    else if(lclasstype == UC_QUARK)
      return getDefaultQuarkFunctionName();
    else if(lclasstype == UC_TRANSIENT)
      return BUILD_DEFAULT_TRANSIENT_FUNCNAME;
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
    return getUlamTypeByIndex(cuti)->getUlamClassType();
  } //getUlamClassForThisClass

  UTI CompilerState::getUlamTypeForThisClass()
  {
    return getCompileThisIdx();
  } //getUlamTypeForThisClass

  //unfortunately, the uti did not reveal a Class symbol;
  //already down to primitive types for casting.
  const std::string CompilerState::getBitVectorLengthAsStringForCodeGen(UTI uti)
  {
    assert(okUTItoContinue(uti));
    ULAMCLASSTYPE classtype = getUlamTypeByIndex(uti)->getUlamClassType();

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
	    else if(classtype == UC_TRANSIENT)
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

    aptr.checkForAbsolutePtr(m_currentSelfPtr);
    return aptr;
  } //getAtomPtrFromSelfPtr (for eval)

  UlamValue CompilerState::getPtrTarget(UlamValue ptr)
  {
    assert(ptr.isPtr());
    if(ptr.isPtrAbs())
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
    assert(ptr.isPtrAbs());

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
    if(ruv.isPtr() && ((UlamType::compareForUlamValueAssignment(ruv.getPtrTargetType(), UAtom, *this) == UTIC_NOTSAME) || (UlamType::compareForUlamValueAssignment(lptr.getPtrTargetType(), UAtom, *this) == UTIC_NOTSAME)))
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
	    if(UlamType::compareForUlamValueAssignment(atval.getUlamValueTypeIdx(), tuti, *this) != UTIC_SAME)
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
    NodeBlockFunctionDefinition * func = NULL;
    NodeBlockContext * cblock = getContextBlock();
    if(cblock->isAClassBlock())
      func = ((NodeBlockClass *) cblock)->findTestFunctionNode();
    return (func != NULL);
  } //thisClassHasTheTestMethod

  bool CompilerState::thisClassIsAQuark()
  {
    UTI cuti = getCompileThisIdx();
    assert(okUTItoContinue(cuti));
    return(getUlamTypeByIndex(cuti)->getUlamClassType() == UC_QUARK);
  } //thisClassIsAQuark

  bool CompilerState::quarkHasAToIntMethod(UTI quti)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(quti, csym);
    assert(isDefined);

    UTI cuti = csym->getUlamTypeIdx();
    assert(okUTItoContinue(cuti));
    AssertBool isQuark = (getUlamTypeByIndex(cuti)->getUlamClassType() == UC_QUARK);
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

  void CompilerState::setupCenterSiteForGenCode()
  {
    Coord c0(0,0);

    //m_classBlock ok now, reset by NodeProgram after type label done
    UTI cuti = getCompileThisIdx();
    m_eventWindow.setSiteElementType(c0, cuti); //includes default values
  } //setupCenterSiteForGenCode

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
    assert((linenum >= 0) && (linenum <= textOfLines->size()));
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
    fp->write("//! ");
    fp->write(getLocationTextAsString(nodeloc).c_str());
  } //outputTextAsComment

  void CompilerState::outputTextAsCommentWithLocationUpdate(File * fp, Locator nodeloc)
  {
    outputTextAsComment(fp, nodeloc);
    m_locOfNextLineText = nodeloc;
  }

  void CompilerState::outputLineNumberForDebugging(File * fp, Locator nodeloc)
  {
    if(m_linesForDebug)
      {
	fp->write("\n");
	fp->write("#line ");
	fp->write_decimal_unsigned(nodeloc.getLineNo());
	fp->write(" \"");
	fp->write(getPathFromLocator(nodeloc).c_str());
	fp->write("\"");
	fp->write("\n");
      }
  }

  s32 CompilerState::getNextTmpVarNumber()
  {
    return ++m_nextTmpVarNumber;
  }

  const std::string CompilerState::getTmpVarAsString(UTI uti, s32 num, TMPSTORAGE stg)
  {
    assert(uti != Void);
    assert(okUTItoContinue(uti));

    if(isAtom(uti)) //elements are packed!
      {
	//stg = TMPBITVAL or TMPTATOM; avoid loading a T into a tmpregister!
	assert(stg != TMPREGISTER);
      }

    std::ostringstream tmpVar; //into
    PACKFIT packed = determinePackable(uti);
    bool isLoadableRegister = (packed == PACKEDLOADABLE);

    if(stg == TMPREGISTER)
      {
	if(isLoadableRegister)
	  tmpVar << "Uh_5tlreg" ; //tmp loadable register
	else
	  tmpVar << "Uh_5tureg" ; //tmp unpacked register
      }
    else if(stg == TMPBITVAL)
      {
	if(isLoadableRegister)
	  tmpVar << "Uh_5tlval" ; //tmp loadable value
	else
	  tmpVar << "Uh_5tuval" ; //tmp unpacked value
      }
    else if(stg == TMPAUTOREF)
      {
	if(isLoadableRegister)
	  tmpVar << "Uh_6tlref" ; //tmp loadable autoref
	else
	  tmpVar << "Uh_6turef" ; //tmp unpacked autoref
      }
    else if(stg == TMPTATOM)
      {
	tmpVar << "Uh_3tut" ; //tmp unpacked atom T
      }
    else if(stg == TMPATOMBS)
      {
	tmpVar << "Uh_4tabs" ; //tmp atombitstorage
      }
    else
      assert(0); //removes assumptions about tmpbitval.

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
    labelname << "Uh_7tuclass" << ToLeximitedNumber(num);
    return labelname.str();
  } //getUlamClassTmpVarAsString

  const std::string CompilerState::getAtomBitStorageTmpVarAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_4tabs" << ToLeximitedNumber(num);
    return labelname.str();
  } //getAtomBitStorageTmpVarAsString

  const std::string CompilerState::getLabelNumAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Ul_214endcontrolloop" << ToLeximitedNumber(num);
    return labelname.str();
  } //getLabelNumAsString

  const std::string CompilerState::getVFuncPtrTmpNumAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uf_tvfp" << ToLeximitedNumber(num);
    return labelname.str();
  } //getVFuncPtrTmpNumAsString

  void CompilerState::saveIdentTokenForConditionalAs(const Token& iTok, const Token& cTok)
  {
    m_identTokenForConditionalAs = iTok;
    m_parsingConditionalToken = cTok;
    m_parsingConditionalAs = true; //cleared manually
  } //saveIdentTokenForConditionalAs

  void CompilerState::saveStructuredCommentToken(const Token& scTok)
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

  void CompilerState::setLocalScopeForParsing(const Token& localTok)
  {
    m_parsingLocalDef = true;
    m_currentLocalDefToken = localTok;
    NodeBlockLocals * locals = makeLocalScopeBlock(localTok.m_locator);
    assert(locals);
    pushClassContext(locals->getNodeType(), locals, locals, false, NULL);
  }

  void CompilerState::clearLocalScopeForParsing()
  {
    m_parsingLocalDef = false;
    popClassContext(); //restore
  }

  bool CompilerState::isParsingLocalDef()
  {
    return m_parsingLocalDef;
  }

  Locator CompilerState::getLocalScopeLocator()
  {
    assert(isParsingLocalDef());
    return m_currentLocalDefToken.m_locator;
  }

  NodeBlockLocals * CompilerState::getLocalScopeBlock(Locator loc)
  {
    u32 pathidx = loc.getPathIndex();
    return getLocalScopeBlockByPathId(pathidx);
  } //getLocalScopeBlock

  NodeBlockLocals * CompilerState::getLocalScopeBlockByIndex(UTI luti)
  {
    UlamType * lut = getUlamTypeByIndex(luti);
    u32 pathid = lut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId();
    return getLocalScopeBlockByPathId(pathid);
  } //getLocalScopeBlockByIndex

  NodeBlockLocals * CompilerState::getLocalScopeBlockByPathId(u32 pathid)
  {
    NodeBlockLocals * rtnLocals = NULL;

    std::map<u32, NodeBlockLocals*>::iterator it = m_localsPerFilePath.find(pathid);

    if(it != m_localsPerFilePath.end())
      {
	rtnLocals = it->second;
	assert(rtnLocals);
	assert(getUlamTypeByIndex(rtnLocals->getNodeType())->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId() == pathid); //sanity check
      }
    return rtnLocals;
  } //getLocalScopeBlockByPathId

  NodeBlockLocals * CompilerState::makeLocalScopeBlock(Locator loc)
  {
    NodeBlockLocals * rtnLocals = getLocalScopeBlock(loc);
    if(!rtnLocals)
      {
      	//add new entry for this loc
	NodeBlockLocals * newblocklocals = new NodeBlockLocals(NULL, *this); //no previous block
	assert(newblocklocals);
	newblocklocals->setNodeLocation(loc);

	u32 pathidx = loc.getPathIndex();

	std::pair<std::map<u32, NodeBlockLocals*>::iterator, bool> reti;
	reti = m_localsPerFilePath.insert(std::pair<u32, NodeBlockLocals*>(pathidx,newblocklocals)); //map owns block
	bool notdupi = reti.second; //false if already existed, i.e. not added
	if(!notdupi)
	  {
	    delete newblocklocals;
	    newblocklocals = NULL;
	    assert(0);
	  }

	//set node type based on ulam file name id
	UlamKeyTypeSignature lkey(pathidx, 0, NONARRAYSIZE, 0, ALT_NOT);
	UTI luti = makeUlamType(lkey, LocalsFileScope, UC_NOTACLASS);
	newblocklocals->setNodeType(luti);
	rtnLocals = newblocklocals;
      }
    return rtnLocals;
  } //makeLocalScopeBlock

  NNO CompilerState::getNextNodeNo()
  {
    return ++m_nextNodeNumber; //first one is 1
  }

  Node * CompilerState::findNodeNoInThisClass(NNO n)
  {
    if(useMemberBlock())
      {
	UTI mbuti = getCurrentMemberClassBlock()->getNodeType();
	return findNodeNoInAClass(n, mbuti);
      }

    NodeBlock * currBlock = getCurrentBlock();
    if(currBlock && (currBlock->getNodeNo() == n) && (getContextBlock()->getNodeType() == getCompileThisIdx()))
      return currBlock; //avoid chix-n-egg with functiondefs

    UTI cuti = getCompileThisIdx();
    Node * rtnNode = findNodeNoInAClass(n, cuti);

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
	    rtnNode = cntsym->findNodeNoInAClassInstance(stubuti, n);
	    //local def?
	    if(!rtnNode)
	      rtnNode = findNodeNoInALocalScope(cntsym->getLoc(), n);
	  }
	else
	  {
	    //what if in ancestor? (not its local scope)
	    UTI superuti = isClassASubclass(getCompileThisIdx());
	    if((superuti != Nouti) && (superuti != Hzy))
	      rtnNode = findNodeNoInAClass(n, superuti);
	  }
      }

    if(!rtnNode)
      {
	//check in local scope
	u32 cid = getUlamKeyTypeSignatureByIndex(cuti).getUlamKeyTypeSignatureNameId();
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClassName(cid, cnsym);
	assert(isDefined);
	rtnNode = findNodeNoInALocalScope(cnsym->getLoc(), n);
      }
    return rtnNode;
  } //findNodeNoInThisClass

  Node * CompilerState::findNodeNoInAClass(NNO n, UTI cuti)
  {
    Node * rtnNode = NULL;
    u32 cid = getUlamKeyTypeSignatureByIndex(cuti).getUlamKeyTypeSignatureNameId();
    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassName(cid, cnsym);
    assert(isDefined);
    rtnNode = cnsym->findNodeNoInAClassInstance(cuti, n);
    //don't check local scope automatically, e.g. in case of superclass
    return rtnNode;
  } //findNodeNoInAClass

  UTI CompilerState::findAClassByNodeNo(NNO n)
  {
    return m_programDefST.findClassNodeNoForTableOfClasses(n); //Nav not found
  } //findAClassByNodeNo

  NodeBlockLocals * CompilerState::findALocalScopeByNodeNo(NNO n)
  {
    NodeBlockLocals * rtnlocals = NULL;
    std::map<u32, NodeBlockLocals *>::iterator it;

    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * locals = it->second;
	assert(locals);
	if(locals->getNodeNo() == n)
	  {
	    rtnlocals = locals;
	    break;
	  }
      }
    return rtnlocals;
  } //findALocalScopeByNodeNo

  Node * CompilerState::findNodeNoInALocalScope(Locator loc, NNO n)
  {
    Node * rtnNode = NULL;
    NodeBlockLocals * locals = getLocalScopeBlock(loc);
    if(locals)
      locals->findNodeNo(n, rtnNode);
    return rtnNode;
  }

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

  NodeBlockContext * CompilerState::getContextBlock()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    return cc.getContextBlock();
  }

  NNO CompilerState::getContextBlockNo()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    if(cc.getContextBlock())
      return cc.getContextBlock()->getNodeNo();
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

  void CompilerState::pushClassContext(UTI idx, NodeBlock * currblock, NodeBlockContext * contextblock, bool usemember, NodeBlockClass * memberblock)
  {
    u32 id = getUlamKeyTypeSignatureByIndex(idx).getUlamKeyTypeSignatureNameId();
    ClassContext cc(id, idx, currblock, contextblock, usemember, memberblock); //new
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
    return makeUlamType(ilkey, Int, UC_NOTACLASS);
  }

  UTI CompilerState::getUnsignedLongUTI()
  {
    UlamKeyTypeSignature ulikey(m_pool.getIndexForDataString("Unsigned"), MAXBITSPERLONG);
    return makeUlamType(ulikey, Unsigned, UC_NOTACLASS);
  }

  UTI CompilerState::getBigBitsUTI()
  {
    UlamKeyTypeSignature ubitskey(m_pool.getIndexForDataString("Bits"), MAXBITSPERLONG);
    return makeUlamType(ubitskey, Bits, UC_NOTACLASS);
  }

  bool CompilerState::isPtr(UTI puti)
  {
    return ((puti == Ptr) || (puti == PtrAbs));
  }

  bool CompilerState::isAtom(UTI auti)
  {
    //includes refs, and arrays!!!
    return (getUlamTypeByIndex(auti)->getUlamTypeEnum() == UAtom);
  }

  bool CompilerState::isAtomRef(UTI auti)
  {
    //do not include ALT_AS, ALT_ARRAYITEM, etc as Ref here. Specifically a ref (&).
    UlamType * aut = getUlamTypeByIndex(auti);
    return ((aut->getUlamTypeEnum() == UAtom) && (aut->getReferenceType() == ALT_REF));
  }

  bool CompilerState::isAClass(UTI uti)
  {
    return (getUlamTypeByIndex(uti)->getUlamTypeEnum() == Class);
  }

  bool CompilerState::isASeenClass(UTI cuti)
  {
    //includes refs, and arrays!!! elements, quarks, transients, NOT UNSEEN!
    ULAMCLASSTYPE classtype = getUlamTypeByIndex(cuti)->getUlamClassType();
    return ((classtype == UC_ELEMENT) || (classtype == UC_QUARK) || (classtype == UC_TRANSIENT));
  } //isASeenClass

  bool CompilerState::isAnonymousClass(UTI cuti)
  {
    assert(okUTItoContinue(cuti));
#if 0
    // anonymous classes have their UTI number as their nameid. t3808
    u32 nameid = getUlamTypeByIndex(cuti)->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId();
    u32 cutiAsStringId = m_pool.getIndexForNumberAsString(cuti);
    return ((nameid == cutiAsStringId) || isHolder(cuti));
#else
    return(!isARootUTI(cuti) || isHolder(cuti)); //t3808
#endif
  } //isAnonymousClass

  void CompilerState::saveUrSelf(UTI uti)
  {
    m_urSelfUTI = uti;
  }

  bool CompilerState::isUrSelf(UTI cuti)
  {
    if(m_urSelfUTI == Nouti)
      {
	UlamKeyTypeSignature ckey = getUlamTypeByIndex(cuti)->getUlamKeyTypeSignature();
	if(ckey.getUlamKeyTypeSignatureNameId() == m_pool.getIndexForDataString("UrSelf"))
	  saveUrSelf(cuti); //error/t3318
	else
	  return false; //t3336
      }
    return (cuti == m_urSelfUTI); //no compare
  } //isUrSelf

  void CompilerState::saveEmptyUTI(UTI uti)
  {
    m_emptyUTI = uti;
  }

  bool CompilerState::isEmpty(UTI cuti)
  {
    if(m_emptyUTI == Nouti)
      {
	UlamKeyTypeSignature ckey = getUlamTypeByIndex(cuti)->getUlamKeyTypeSignature();
	if(ckey.getUlamKeyTypeSignatureNameId() == m_pool.getIndexForDataString("Empty"))
	  saveEmptyUTI(cuti);
	else
	  return false;
      }
    return (cuti == m_emptyUTI); //no compare
  } //isEmpty

  bool CompilerState::okUTItoContinue(UTI uti)
  {
    //if((uti != Hzy) && (getUlamTypeByIndex(uti)->getUlamTypeEnum() == Hzy))
    //  assert(0); //t3378?
    return ((uti != Nav) && (uti != Hzy) && (uti != Nouti));
  }

  bool CompilerState::okUTItoContinue(UTI uti1, UTI uti2)
  {
    return ((uti1 != Nav) && (uti2 != Nav));
  }

  bool CompilerState::checkHasHazyKin(NodeBlock * block)
  {
    assert(block);
    UTI buti = block->getNodeType();
    return (block->isAClassBlock() && (isClassAStub(buti) || ((isClassASubclass(buti) != Nouti) && !((NodeBlockClass *) block)->isSuperClassLinkReady())));
  }

} //end MFM
