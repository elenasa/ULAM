#include <stdio.h>
#include <iostream>
#include <errno.h>
#include "ClassContext.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "SymbolFunctionName.h"
#include "SymbolTypedef.h"
#include "SymbolWithValue.h"
#include "UlamTypeAtom.h"
#include "UlamTypeClass.h"
#include "UlamTypeClassElement.h"
#include "UlamTypeClassQuark.h"
#include "UlamTypeClassTransient.h"
#include "UlamTypeClassLocalsFilescope.h"
#include "UlamTypeInternalHolder.h"
#include "UlamTypeInternalHzy.h"
#include "UlamTypeInternalLocalsFileScope.h"
#include "UlamTypeInternalNav.h"
#include "UlamTypeInternalNouti.h"
#include "UlamTypeInternalPtr.h"
#include "UlamTypePrimitiveBits.h"
#include "UlamTypePrimitiveBool.h"
#include "UlamTypePrimitiveInt.h"
#include "UlamTypePrimitiveString.h"
#include "UlamTypePrimitiveUnary.h"
#include "UlamTypePrimitiveUnsigned.h"
#include "UlamTypePrimitiveVoid.h"

namespace MFM {

  //#define _DEBUG_OUTPUT
  //#define _INFO_OUTPUT
#define _NOTE_OUTPUT
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

#ifdef _NOTE_OUTPUT
  static const bool noteOn = true;
#else
  static const bool noteOn = false;
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
  static const char * CUSTOMARRAY_GET_FUNCNAME = "aref"; //unmangled
  static const char * CUSTOMARRAY_SET_FUNCNAME = "aset"; //unmangled (deprecated)
  static const char * CUSTOMARRAY_GET_MANGLEDNAME = "Uf_4aref";
  static const char * CUSTOMARRAY_LENGTHOF_FUNCNAME = "alengthof"; //unmangled
  static const char * CUSTOMARRAY_LENGTHOF_MANGLEDNAME = "Uf_919alengthof";

  static const char * IS_MANGLED_FUNCNAME = "internalCMethodImplementingIs"; //Uf_2is
  static const char * IS_MANGLED_FUNCNAME_FOR_ATOM = "UlamClass<EC>::IsMethod"; //Uf_2is

  static const char * GETRELPOS_MANGLED_FUNCNAME = "internalCMethodImplementingGetRelativePositionOfBaseClass"; //Uf_2is
  static const char * GETRELPOS_MANGLED_FUNCNAME_WITHCHECK = "internalCMethodImplementingGetRelativePositionOfBaseClassWithCheck"; //Uf_2is
  static const char * GETRELPOS_MANGLED_FUNCNAME_FOR_ATOM = "UlamClass<EC>::GetRelativePositionOfBaseClass"; //Uf_2is

  static const char * GETDATAMEMBERINFO_FUNCNAME = "GetDataMemberInfo";
  static const char * GETDATAMEMBERCOUNT_FUNCNAME = "GetDataMemberCount";

  static const char * GETNUMBEROFBASES_FUNCNAME = "GetBaseClassCount";
  static const char * GETNUMBEROFDIRECTBASES_FUNCNAME = "GetDirectBaseClassCount";
  static const char * GETORDEREDBASE_FUNCNAME = "GetOrderedBaseClassAsUlamClass";
  static const char * GETISDIRECTBASECLASS_FUNCNAME = "IsDirectBaseClass";

  static const char * GETCLASSMANGLEDNAME_FUNCNAME = "GetMangledClassName";
  static const char * GETCLASSMANGLEDNAMESTRINGINDEX_FUNCNAME = "GetMangledClassNameAsStringIndex";
  static const char * GETCLASSNAMESTRINGINDEX_FUNCNAME = "GetUlamClassNameAsStringIndex";

  static const char * GETCLASSLENGTH_FUNCNAME = "GetClassLength";
  static const char * GETBASECLASSLENGTH_FUNCNAME = "GetClassDataMembersSize";
  static const char * GETCLASSREGISTRATIONNUMBER_FUNCNAME = "GetRegistrationNumber";
  static const char * GETELEMENTTYPE_FUNCNAME = "GetTypeFromThisElement";
  static const char * READTYPEFIELD_FUNCNAME = "ReadTypeField";
  static const char * WRITETYPEFIELD_FUNCNAME = "WriteTypeField";

  static const char * BUILD_DEFAULT_ATOM_FUNCNAME = "BuildDefaultAtom";
  static const char * BUILD_DEFAULT_QUARK_FUNCNAME = "getDefaultQuark";
  static const char * BUILD_DEFAULT_TRANSIENT_FUNCNAME = "getDefaultTransient";

  static const char * GETVTABLEENTRY_FUNCNAME = "getVTableEntry";
  static const char * GETVTABLEENTRYCLASSPTR_FUNCNAME = "getVTableEntryUlamClassPtr";
  static const char * GETVTABLEENTRYSTARTOFFSETFORCLASS_FUNCNAME = "GetVTStartOffsetForClassByRegNum";

  static const char * GETSTRING_FUNCNAME = "GetStringPointerFromGlobalStringPool";
  static const char * GETSTRINGLENGTH_FUNCNAME = "GetStringLengthFromGlobalStringPool";
  static const char * USERSTRINGPOOL_MANGLEDNAME = "Ug_globalStringPoolData";
  static const char * USERSTRINGPOOLINDEXES_MANGLEDNAME = "Ug_globalStringIndexes";
  static const char * USERSTRINGPOOL_SIZEDEFINENAME = "Ug_globalStringPoolSize";
  static const char * USERSTRINGPOOL_NUMBERSTRINGSDEFINENAME = "Ug_globalNumberofStrings";
  static const char * USERSTRINGPOOL_FILENAME = "GlobalStringPool"; //also used by ulam.tmpl


  static const char * CModeForHeaderFiles = "/**                                      -*- mode:C++ -*- */\n\n";

  // First line something like: "/* NodeProgram.h - Root Node of Programs for ULAM\n"
  static const char * CopyrightAndLicenseForUlamHeader =   "*\n"
    "**********************************************************************************\n"
    "* This code is generated by the ULAM programming language compilation system.\n"
    "*\n"
    "* The ULAM programming language compilation system is free software:\n"
    "* you can redistribute it and/or modify it under the terms of the GNU\n"
    "* General Public License as published by the Free Software\n"
    "* Foundation, either version 3 of the License, or (at your option)\n"
    "* any later version.\n"
    "*\n"
    "* The ULAM programming language compilation system is distributed in\n"
    "* the hope that it will be useful, but WITHOUT ANY WARRANTY; without\n"
    "* even the implied warranty of MERCHANTABILITY or FITNESS FOR A\n"
    "* PARTICULAR PURPOSE.  See the GNU General Public License for more\n"
    "* details.\n"
    "*\n"
    "* You should have received a copy of the GNU General Public License\n"
    "* along with the ULAM programming language compilation system\n"
    "* software.  If not, see <http://www.gnu.org/licenses/>.\n"
    "*\n"
    "* @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>\n"
    "*/\n\n";

  //use of this in the initialization list seems to be okay;
  CompilerState::CompilerState(): m_linesForDebug(false), m_programDefST(*this), m_parsingLocalDef(false), m_parsingFUNCid(0), m_nextFunctionOrderNumber(1), m_parsingThisClass(Nouti), m_parsingConcreteClassFlag(false), m_parsingVariableSymbolTypeFlag(STF_NEEDSATYPE), m_initSubtreeSymbolsWithConstantsOnly(false), m_gotStructuredCommentToken(false), m_parsingConditionalAs(false), m_eventWindow(*this), m_goAgainResolveLoop(false), m_pendingArgStubContext(0), m_pendingArgTypeStubContext(0), m_currentSelfSymbolForCodeGen(NULL), m_nextTmpVarNumber(0), m_nextNodeNumber(0), m_urSelfUTI(Nouti), m_emptyElementUTI(Nouti), m_classIdBits(CLASSIDBITS)
  {
    m_classIdRegistryUTI.push_back(0); //initialize 0 for UrSelf
    m_err.init(this, debugOn, infoOn, noteOn, warnOn, waitOn, NULL);
    Token::initTokenMap(*this);
    //m_constantStack.addFrameSlots(1); //initialize for incremental update; init instead.

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

    m_classIdRegistryUTI.clear();

    m_unseenClasses.clear();
  } //clearAllDefinedUlamTypes

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
	NodeBlockLocals * localsblock = it->second;
	delete localsblock;
      }
    m_localsPerFilePath.clear();
  } //clearAllLocalsPerFilePath

  void CompilerState::clearCurrentObjSymbolsForCodeGen()
  {
    m_currentObjSymbolsForCodeGen.clear();
  } //clearCurrentObjSymbolsForCodeGen

  bool CompilerState::getClassNameFromFileName(std::string startstr, u32& compileThisId)
  {
    u32 foundSuffix = FindUlamFilenameSuffix(startstr);
    if(foundSuffix == 0)
      {
	std::ostringstream msg;
        msg << "File name <" << startstr << "> doesn't end with '.ulam'";
	MSG2("", msg.str().c_str() , ERR);
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

  void CompilerState::getTargetDescriptorsForLocalsFilescopes(TargetMap & localstargets)
  {
    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);
	localsblock->addTargetDescriptionToInfoMap(localstargets, 0);
      }
  }

  void CompilerState::getMembersDescriptionsForLocalsFilescopes(ClassMemberMap & localsmembers)
  {
    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);
	localsblock->addMemberDescriptionsToInfoMap(localsmembers);
      }
  }

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
    if(hkey == newkey)
      {
	UlamType * ut = NULL;
	AssertBool isDef = isDefined(newkey, ut);
	assert(isDef);
	assert(ut->getUlamTypeEnum() == utype);
	assert(ut->getUlamClassType() == classtype);
	return uti; //t3373,5,6,7, t3374, t3379, 41009, 41010 short-circuit..
      }
    return makeUlamTypeFromHolder(hkey, newkey, utype, uti, classtype);
  } //makeUlamTypeFromHolder

  UTI CompilerState::makeUlamTypeFromHolder(UlamKeyTypeSignature oldkey, UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti, ULAMCLASSTYPE classtype)
  {
    assert(!(oldkey == newkey));

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
		      abortShouldntGetHere(); //t3379 wo NodeTypedef updating UTIAlias
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

    updateUTIAlias(uti, auti); //after index-to-key is set, forced ok? NOPE (t41489)

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

  void CompilerState::cleanupExistingHolder(UTI huti, UTI newuti)
  {
    if(huti == newuti) return; //short-circuit (e.g. t3378) don't use compare; maybe Hzy etyp

    assert(isHolder(huti) || !isComplete(huti)); //t41288, t41287; incomplete: t3378, t3380
    assert(okUTItoContinue(newuti)); //t3868

    UlamType * newut = getUlamTypeByIndex(newuti);
    UlamKeyTypeSignature newkey = newut->getUlamKeyTypeSignature();
    ULAMTYPE newetyp = newut->getUlamTypeEnum();
    ULAMCLASSTYPE newctyp = newut->getUlamClassType();
    makeUlamTypeFromHolder(newkey, newetyp, huti, newctyp);
    mapTypesInCurrentClass(huti, newuti);
    updateUTIAliasForced(huti, newuti);
  }

  void CompilerState::mergeAnyDataMembersBeforeAnonymousClassReplaced(UTI olduti, UTI newuti)
  {
    assert(isAClass(olduti));
    assert(isAnonymousClass(olduti));
    SymbolClass * oldcsym = NULL;
    if(alreadyDefinedSymbolClass(olduti, oldcsym))
      {
	NodeBlockClass * fmblock = oldcsym->getClassBlockNode();
	assert(fmblock);
	//from anonymous class, with possible tmp typedefs expected (t3347)
	SymbolClass * newcsym = NULL;
	AssertBool gotnew = alreadyDefinedSymbolClass(newuti, newcsym);
	assert(gotnew);
	NodeBlockClass * cblock = newcsym->getClassBlockNode();
	if(cblock)
	  {
	    UTI mappednew = newuti;
	    if(newcsym->getUlamTypeIdx() != newuti)
	      findRootUTIAlias(newuti, mappednew); //t41312,3
	    cblock->setDataMembersSymbolTable(mappednew, *fmblock);
	  }
	//else error(t3186,t41159)
      }
  }

  u32 CompilerState::replaceUTIKeyAndAlias(UTI olduti, UTI newuti)
  {
    assert(okUTItoContinue(olduti));
    assert(okUTItoContinue(newuti));
    UlamType * newut = getUlamTypeByIndex(newuti);
    UlamKeyTypeSignature newkey = newut->getUlamKeyTypeSignature();
    UlamKeyTypeSignature oldkey = getUlamKeyTypeSignatureByIndex(olduti);

    // apparently, wrong about maybeIsAClass assumption during parseClassArguments (t41437)
    if(isHolder(olduti) && isAClass(olduti)) //dont remove and alias (t41513)
      {
	if(isAClass(newuti))
	  mergeAnyDataMembersBeforeAnonymousClassReplaced(olduti, newuti); //t3384
	u32 oldnameid = oldkey.getUlamKeyTypeSignatureNameId();
	AssertBool isGone = removeIncompleteClassSymbolFromProgramTable(oldnameid);
	assert(isGone);
      }

    //we need to keep the huti, but change the key
    //removes old key and its ulamtype from map, if no longer pointed to
    deleteUlamKeyTypeSignature(oldkey, olduti); //decrements counter

    u32 newkeycount = incrementKeyToAnyUTICounter(newkey, olduti);

    m_indexToUlamKey[olduti] = newkey;

    updateUTIAliasForced(olduti, newuti); //after index-to-key is set
    return newkeycount;
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
	classblock->setNodeType(cuti); //incomplete

	Token cTok(TOK_TYPE_IDENTIFIER, cloc, id);
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

  UTI CompilerState::makeCulamGeneratedTypedefSymbolInCurrentContext(Token tok, bool isaclass)
  {
    u32 tokid = getTokenDataAsStringId(tok);
    Symbol * tdsymptr = NULL;
    bool hashazykin = false;
    if(isDataMemberIdInClassScope(tokid, tdsymptr, hashazykin))
      return tdsymptr->getUlamTypeIdx(); //avoid making another holder

    UTI parsingthisclass = getThisClassForParsing();
    if(getUlamTypeNameIdByIndex(parsingthisclass) == tokid)
      {
	if(isClassATemplate(parsingthisclass))
	  {
	    std::ostringstream msg;
	    msg << "Class name used as a type within Template Definition of same name: ";
	    msg << m_pool.getDataAsString(tokid).c_str();
	    MSG2(&tok, msg.str().c_str(), ERR);
	    return Nav; //t41155
	  }
	return parsingthisclass; //t3565, no typedef needed
      }

    UTI huti = makeUlamTypeHolder();
    SymbolTypedef * symtypedef = new SymbolTypedef(tok, huti, Nav, *this);
    assert(symtypedef);
    symtypedef->setCulamGeneratedTypedef();

    if(useMemberBlock())
      addSymbolToCurrentMemberClassScope(symtypedef); //not locals scope
    else if(isThisLocalsFileScope())
      addSymbolToLocalsScope(symtypedef, tok.m_locator);
    else
      addSymbolToCurrentContextScope(symtypedef);

    if(isaclass)
      {
	makeAnonymousClassFromHolder(huti, tok.m_locator); //default is not a class
	if(!isUnseenClassId(tokid)) //for ulamexports, ish 20210825
	  addUnseenClassId(tokid);
      }
    return huti;
  } //makeCulamGeneratedTypedefSymbolInCurrentContext

  //convenience method (refactors code originally from installSymbol)
  //if exists, just returns it, o.w. makes it; trick to know the base ULAMTYPE
  UTI CompilerState::makeUlamType(const Token& typeTok, s32 bitsize, s32 arraysize, UTI classinstanceidx, ALT reftype, ULAMCLASSTYPE classtype)
  {
    //type names begin with capital letter..and the rest can be either
    u32 typeNameId = getTokenAsATypeNameId(typeTok); //Foo, Int, etc

    //can't be a typedef!! get's the wrong name for type key; use key as arg
    UTI tmputi = Nav;
    UTI tmpforscalaruti = Nouti;
    u32 tokid = getTokenDataAsStringId(typeTok);
    if(getUlamTypeByTypedefName(tokid, tmputi, tmpforscalaruti) == TBOOL_TRUE)
      {
	//no assert, return uti instead
	return tmpforscalaruti; //t41398-t41401
      }
    //else continue..

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
	findRootUTIAlias(uti,uti); //?
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

    s32 keybs = key.getUlamKeyTypeSignatureBitSize();
    s32 keyarr = key.getUlamKeyTypeSignatureArraySize();

    //if(!isDefined(key,ut) || (utype == Class) || ((utype != Class) && (key.getUlamKeyTypeSignatureBitSize() == UNKNOWNSIZE)) || (key.getUlamKeyTypeSignatureArraySize() == UNKNOWNSIZE))
    if((keybs != UNKNOWNSIZE) && (keyarr != UNKNOWNSIZE) && isDefined(key, ut))
      {
	AssertBool isDefined = anyDefinedUTI(key,uti);
	assert(isDefined);
      }
    else
      {
	//if((utype == Class) || (keybs == UNKNOWNSIZE) || (keyarr == UNKNOWNSIZE) )
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
		//need a new UTI for reference, or const ref
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
		    AssertBool isDefined = alreadyDefinedSymbolClassNameByUTI(suti, cnsym);
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
		//can't save scalar in key; unable to look up from token (t41055)
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
    return uti;
  } //makeUlamType

  void CompilerState::addCompleteUlamTypeToThisContextSet(UTI uti)
  {
    NodeBlockContext * contextblock = getContextBlock();
    addCompleteUlamTypeToContextBlockSet(uti, contextblock);
  } //addCompleteUlamTypeToThisContextSet

  void CompilerState::addCompleteUlamTypeToContextBlockSet(UTI uti, NodeBlockContext * contextblock)
  {
    assert(contextblock);

    //for all Nodes' setNodeType(); used to genCode include files.
    assert(okUTItoContinue(uti));
    UTI deref = getUlamTypeAsDeref(uti);
    if(isComplete(deref))
      {
	//always add the deref (e.g. t3224)
	UlamKeyTypeSignature key = getUlamKeyTypeSignatureByIndex(deref);
	contextblock->addUlamTypeKeyToSet(key);
      }
  } //addCompleteUlamTypeToContextBlockSet

  bool CompilerState::isOtherClassInThisContext(UTI suti)
  {
    //true only if used by this class, or is its base class
    bool rtnb = false;
    UTI cuti = getCompileThisIdx();

    if((suti != cuti) && isComplete(suti))
      {
	NodeBlockContext * contextblock = getContextBlock();
	if(contextblock->hasUlamType(suti))
	  rtnb = true;
	else if(isEmptyElement(suti))
	  rtnb = true;
	else if(isClassASubclass(cuti))
	  {
	    rtnb = isClassASubclassOf(cuti, suti); //t3640, t3605
	  }
	else //special case, since here, superclass is Nouti
	  {
	    if(isUrSelf(suti))
	      {
		rtnb = true;
	      }
	  }
      }
    return rtnb;
  } //isOtherClassInThisContext

  bool CompilerState::isAStringDataMemberInClass(UTI cuti)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    NodeBlockClass * classblock = csym->getClassBlockNode();
    assert(classblock);
    return classblock->hasStringDataMembers();
  } //isAStringDataMemberInClass

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
	    case UC_LOCALSFILESCOPE:
	      ut = new UlamTypeClassLocalsFilescope(key, *this);
	      break;
	    default:
	      abortUndefinedUlamClassType();
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
      case String:
	ut = new UlamTypePrimitiveString(key, *this);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Undefined ULAMTYPE base type <" << utype << ">" ;
	  MSG2("",msg.str().c_str(), DEBUG);
	  abortUndefinedUlamType();
	}
      };
    assert(ut);
    return ut;
  } //createUlamType

  u32 CompilerState::incrementKeyToAnyUTICounter(UlamKeyTypeSignature key, UTI utiarg)
  {
    u32 count = 0;
    std::map<UlamKeyTypeSignature, std::set<UTI>, less_than_key>::iterator it = m_keyToAnyUTI.find(key);
    if(it != m_keyToAnyUTI.end())
      {
	assert(key == it->first);
	it->second.insert(utiarg);
	count = it->second.size();
      }
    else
      {
	std::set<UTI> aset;
	aset.insert(utiarg);
	m_keyToAnyUTI.insert(std::pair<UlamKeyTypeSignature,std::set<UTI> >(key, aset));
	count = 1;
      }
    return count;
  } //incrementKeyToAnyUTICounter

  u32 CompilerState::decrementKeyToAnyUTICounter(UlamKeyTypeSignature key, UTI utiarg)
  {
    std::map<UlamKeyTypeSignature, std::set<UTI>, less_than_key>::iterator it = m_keyToAnyUTI.find(key);
    u32 count = 0;
    if(it != m_keyToAnyUTI.end())
      {
	assert(key == it->first);
	std::set<UTI>::iterator sit = it->second.find(utiarg);
	if(sit != it->second.end())
	  {
	    assert(utiarg == *sit);
	    it->second.erase(sit); //decrements count
	  }
	count = it->second.size();
	if(count == 0)
	  m_keyToAnyUTI.erase(it);
      }
    return count;
  } //decrementKeyToAnyUTICounter


  u32 CompilerState::getKeyToAnyUTICounter(UlamKeyTypeSignature key)
  {
    std::map<UlamKeyTypeSignature, std::set<UTI>, less_than_key>::iterator it = m_keyToAnyUTI.find(key);
    u32 count = 0;
    if(it != m_keyToAnyUTI.end())
      {
	assert(key == it->first);
	count = it->second.size();
      }
    return count;
  }


  //used to update Class' calculated bit size (setBitSize)
  bool CompilerState::deleteUlamKeyTypeSignature(UlamKeyTypeSignature key, UTI utiarg)
  {
    bool rtnBool= false;
    if(decrementKeyToAnyUTICounter(key, utiarg) == 0)
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
    UlamKeyTypeSignature nokey = getUlamTypeByIndex(Nouti)->getUlamKeyTypeSignature();
    m_indexToUlamKey[utiarg] = nokey; //clear to Nouti key
    incrementKeyToAnyUTICounter(nokey, utiarg); //here
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
  // doesn't pass along holders as mapped type (t3375)
  bool CompilerState::mappedIncompleteUTI(UTI cuti, UTI auti, UTI& mappedUTI)
  {
    if(!okUTItoContinue(auti))
      return false; //t41448

    UTI tmpmappedUTI = auti;

    if(isALocalsFileScope(cuti))
      {
	//if(findaUTIAlias(auti, tmpmappedUTI))
	if(findRootUTIAlias(auti, tmpmappedUTI)) //not a class (t3862, t3326)
	{
	  if(!isHolder(tmpmappedUTI))
	    {
	      mappedUTI = tmpmappedUTI;
	      return true;
	    }
	}
	return false;
      }

    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    //recursively checks ancestors, for mapped uti (ish 20210315)
    if(csym->hasMappedUTI(auti, tmpmappedUTI))
      {
	UTI muti = tmpmappedUTI;
	findRootUTIAlias(muti,tmpmappedUTI);
	if(!isHolder(tmpmappedUTI))
	  {
	    mappedUTI = tmpmappedUTI;
	    return true; //may not be root (t41431)
	  }
	else
	  return false;
      }

    // does this hurt anything? t3326?
    if(findRootUTIAlias(auti, tmpmappedUTI))
      {
	if(!isHolder(tmpmappedUTI))
	  {
	    mappedUTI = tmpmappedUTI;
	    return true; //anonymous UTI
	  }
	else
	  return false;
      }

    //or this? (works for template instances too)
    if((csym->getClassBlockNode() != NULL) && statusUnknownTypeInAClassResolver(cuti, auti))
    {
      tmpmappedUTI = auti;
      findRootUTIAlias(auti, tmpmappedUTI); //auti no longer a holder (ish 20210315-22xx)
      if(!isHolder(tmpmappedUTI))
	{
	  assert(getReferenceType(auti) == getReferenceType(tmpmappedUTI)); //t41455
	  mappedUTI = tmpmappedUTI;
	  return true;
	}
      else
	return false;
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

	//move this test after looking for the mapped class symbol type in "cuti" (always compileThis?)
	ULAMTYPE bUT = aut->getUlamTypeEnum();
	if(bUT == Class)
	  {
	    SymbolClassName * cnsymOfIncomplete = NULL; //could be a different class than being compiled
	    AssertBool isDefined = alreadyDefinedSymbolClassNameByUTI(auti, cnsymOfIncomplete);
	    assert(isDefined);
	    UTI utiofinc = cnsymOfIncomplete->getUlamTypeIdx();
	    if(utiofinc != cuti)
	      {
		if(!cnsymOfIncomplete->isClassTemplate())
		  return false;

		if(cnsymOfIncomplete->hasMappedUTI(auti, tmpmappedUTI))
		  {
		    if(!isHolder(tmpmappedUTI))
		      {
			mappedUTI = tmpmappedUTI;
			return true;
		      }
		    else
		      return false;
		  }
	      }
	  }
	//else?
      }
    //else is a holder

    return false;
  } //mappedIncompleteUTI

  //called by Symbol's copy constructor with ref's 'incomplete' uti;
  //also called by NodeTypeDescriptor's copy constructor since has no symbol;
  //please set getCompileThisIdx() to the instance's UTI.
  UTI CompilerState::mapIncompleteUTIForCurrentClassInstance(UTI suti, Locator loc)
  {
    return mapIncompleteUTIForAClassInstance(getCompileThisIdx(), suti, loc);
  } //helper

  UTI CompilerState::mapIncompleteUTIForAClassInstance(UTI cuti, UTI suti, Locator loc)
  {
    UlamType * sut = getUlamTypeByIndex(suti);
    if(sut->isComplete())
      return suti;

    if(!okUTItoContinue(suti))
      return suti; //forgiving; e.g. may be used for unset referencedUTI

    if(isALocalsFileScope(cuti))
      {
	UTI mappedluti = suti;
	findRootUTIAlias(suti, mappedluti); //?
	return mappedluti;
	//return suti; //t3526, t3892
      }

    if(isUrSelf(suti))
      return suti; //special case

    if(isEmptyElement(suti))
      return suti; //special case

    if(suti == cuti)
      return suti;

    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassNameByUTI(cuti, cnsym);
    assert(isDefined);

    UTI mappedUTI;
    if(cnsym->hasInstanceMappedUTI(cuti, suti, mappedUTI))
      {
	UTI muti = mappedUTI;
	findRootUTIAlias(muti, mappedUTI); //?
	return mappedUTI;  //e.g. decl list //t41225,t41228,t3327..etc
      }

    if(findRootUTIAlias(suti, mappedUTI)) //t3326
       return mappedUTI; //anonymous UTI

    //move this test after looking for the mapped class symbol type
    ULAMTYPE bUT = sut->getUlamTypeEnum();
    SymbolClassName * cnsymOfIncomplete = NULL; //could be a different class than being compiled
    if(bUT == Class)
      {
	AssertBool isDefined = alreadyDefinedSymbolClassNameByUTI(suti, cnsymOfIncomplete);
	assert(isDefined);

	if(!cnsymOfIncomplete->isClassTemplate())
	  return suti;

	//suti may be a template...a stub or stubcopy...or a fully instantiated instance

	ALT salt = getReferenceType(suti);
	if(salt != ALT_NOT)
	  {
	    UTI asref = mapIncompleteUTIForCurrentClassInstance(getUlamTypeAsDeref(suti), loc);
	    if(okUTItoContinue(asref) && !isHolder(asref))
	      return getUlamTypeAsRef(asref, salt);
	    return asref; //hzy
	  }

	bool isClassArrayType = !sut->isScalar(); //t41555
	if(isClassArrayType)
	  {
	    UTI ascalar = mapIncompleteUTIForCurrentClassInstance(getUlamTypeAsScalar(suti), loc);
	    if(okUTItoContinue(ascalar) && !isHolder(ascalar))
	      return getUlamTypeAsArrayOfScalar(ascalar);
	    return ascalar; //hzy?
	  }

	//t3859, t3328, t3873
	bool classnamesame = (cnsymOfIncomplete->getId() == cnsym->getId());
	if(classnamesame)
	  {
	    if(isClassATemplate(suti))
	      return cuti;
	  }

	if(!isClassAStub(suti))
	  return suti;

	UTI stubcopyof = getStubCopyOf(suti);
	bool isastubcopy = (stubcopyof != Nouti); //or was a stubcopy
	//stubcopies have no args yet to be pending (t41223,..)
	if(!isClassAStubCopy(suti) && !((SymbolClassNameTemplate *) cnsymOfIncomplete)->pendingClassArgumentsForStubClassInstance(suti))
	  return suti; //as good as an instance!

	if(getContextForPendingArgValuesForStub(cuti) == suti)
	  return suti; //t41436?
	if(getContextForPendingArgValuesForStub(cuti) == stubcopyof)
	  return suti; //t41436?

	if(isastubcopy)
	  {
	    suti = stubcopyof; //continue as if the original..
	    if(cnsym->hasInstanceMappedUTI(cuti, suti, mappedUTI))
	      {
		UTI muti = mappedUTI; //stubcopyof already used in this class, ok to reuse
		findRootUTIAlias(muti, mappedUTI);
		return mappedUTI;  //20210726-2 ish; err/t41448
		//suti = mappedUTI; SCNT:450 assert failed '!twasastubcopy'
	      }
	    sut = getUlamTypeByIndex(suti);
	  }

	//o.w. make a stub copy...
	//return Hzy;
      }

    //first time we've seen this 'incomplete' UTI for this class instance (as fully instantiated):
    //get a new UTI and add to cnsym's map for this instance in case we see it again;
    //For classes with pending args, make a copy of the stub including its resolver with pending
    //args, so pending args can be resolved XXXX within the context of this class instance
    //(e.g. dependent on instances arg values which makes it different than others', like "self").
    //Context dependent pending args are resolved before they are added to the resolver's
    //pending args.

    UlamKeyTypeSignature skey = sut->getUlamKeyTypeSignature();
    UlamKeyTypeSignature newkey(skey); //default constructor makes copy
    //restore from original ut; t41436: suti stub type unseen, while template was a transient
    ULAMCLASSTYPE sclasstype = bUT==Class ? cnsymOfIncomplete->getUlamClass() : sut->getUlamClassType();

    UTI newuti = makeUlamType(newkey, bUT, sclasstype); //could be another holder copy (t3765)
    AssertBool notdup = cnsym->mapInstanceUTI(cuti, suti, newuti);
    assert(notdup);

    if(bUT == Class)
      {
	UlamType * newut = getUlamTypeByIndex(newuti);
	if(sut->isCustomArray())
	  ((UlamTypeClass *) newut)->setCustomArray();

	//potential for unending process..(t41436)
	//notes: sclasstype may be UNSEEN. 'cuti' may not be getCompileThis class (t41209,t41217,8)
	UTI cnuti = cnsym->getUlamTypeIdx();
	((SymbolClassNameTemplate *)cnsymOfIncomplete)->copyAStubClassInstance(suti, newuti, cuti, newuti, cnuti, loc);
	std::ostringstream msg;
	msg << "MAPPED!! type: " << getUlamTypeNameByIndex(suti).c_str();
	msg << "(UTI" << suti << ")";
	msg << " TO newtype: " << getUlamTypeNameByIndex(newuti).c_str();
	msg << "(UTI" << newuti << ")";
	msg << " while compiling class " << getUlamTypeNameByIndex(cuti).c_str();
	msg << "(UTI" << cuti << ")";
	msg << ", for incomplete class " << getUlamTypeNameByIndex(cnsymOfIncomplete->getUlamTypeIdx()).c_str();
	msg << "(UTI" << cnsymOfIncomplete->getUlamTypeIdx() << ")";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }
    return newuti;
  } //mapIncompleteUTIForAClassInstance

  void CompilerState::mapTypesInCurrentClass(UTI fm, UTI to)
  {
    UTI cuti = getCompileThisIdx();
    if(isALocalsFileScope(cuti))
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

  u32 CompilerState::getUlamTypeNameIdByIndex(UTI typidx)
  {
    assert(typidx < m_indexToUlamKey.size());
    return m_indexToUlamKey[typidx].getUlamKeyTypeSignatureNameId();
  }

  UlamType * CompilerState::getUlamTypeByIndex(UTI typidx)
  {
    UlamType * rtnUT = NULL;
    if(typidx >= m_indexToUlamKey.size())
      {
	std::ostringstream msg;
	msg << "Undefined UTI <" << typidx << "> Max is: "
	    << m_indexToUlamKey.size() << ", returning Nav INSTEAD";
	MSG2("", msg.str().c_str(), DEBUG);
	typidx = 0;
	abortShouldntGetHere();
      }
    AssertBool isDef = isDefined(m_indexToUlamKey[typidx], rtnUT);
    if(!isDef)
      return getUlamTypeByIndex(Nav); //e.g. decrementKey fell to none and was erased (t41452,3)

    return rtnUT;
  } //getUlamTypeByIndex

  const std::string CompilerState::getUlamTypeNameBriefByIndex(UTI uti)
  {
    UlamType * ut = getUlamTypeByIndex(uti);
    if(ut->getUlamTypeEnum() == Class)
      {
	SymbolClass * csym = NULL;
	if(alreadyDefinedSymbolClass(uti, csym) && !csym->isStubCopy())
	  return ut->getUlamTypeClassNameBrief(uti);
	else
	  {
	    u32 nameid = ut->getUlamTypeNameId();
	    return m_pool.getDataAsString(nameid);
	  }
      }
    return ut->getUlamTypeNameBrief();
  }

  const std::string CompilerState::getUlamTypeNameByIndex(UTI uti)
  {
    return getUlamTypeByIndex(uti)->getUlamTypeName(); //prettier names for internal types(error/t3113); String arrays(t3976)
  }

  // returns effective self name of the scalar dereferenced uti arg.
  const std::string CompilerState::getTheInstanceMangledNameByIndex(UTI uti)
  {
    UTI esuti = uti;
    if(!isScalar(uti))
      esuti = getUlamTypeAsScalar(uti);

    if(isReference(esuti))
      esuti = getUlamTypeAsDeref(esuti);

    UlamType * esut = NULL;
    AssertBool isDef = isDefined(m_indexToUlamKey[esuti], esut);
    assert(isDef);

    if(isALocalsFileScope(esuti) && !isAClass(esuti))
      return getLocalsFilescopeTheInstanceMangledNameByIndex(esuti);

    assert(esut->getUlamTypeEnum() == Class);
    assert(!isClassAStub(esuti)); //t41223

    std::ostringstream esmangled;
    esmangled << esut->getUlamTypeMangledName().c_str();
    esmangled << "<EC>::THE_INSTANCE";
    return esmangled.str();
  } //getTheInstanceMangledNameByIndex

  const std::string CompilerState::getLocalsFilescopeTheInstanceMangledNameByIndex(UTI uti)
  {
    //e.g. t3952, t3954
    assert(isALocalsFileScope(uti));
    u32 mangledclassid = getMangledClassNameIdForUlamLocalsFilescope(uti);
    std::ostringstream esmangled;
    esmangled << m_pool.getDataAsString(mangledclassid).c_str();
    esmangled << "<EC>::THE_INSTANCE";
    return esmangled.str();
  }

  ULAMTYPE CompilerState::getBaseTypeFromToken(const Token& tok)
  {
    ULAMTYPE bUT = Nav;
    UTI uti = Nav;
    UTI tmpforscalaruti = Nouti;
    u32 tokid = getTokenDataAsStringId(tok);
    //is this name already a typedef for a complex type?
    if(getUlamTypeByTypedefName(tokid, uti, tmpforscalaruti) == TBOOL_TRUE)
      bUT = getUlamTypeByIndex(uti)->getUlamTypeEnum();
    else if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
      {
	std::string typeName = getTokenAsATypeName(tok); //Int, etc
	//no way to get the bUT, except to assume typeName is one of them?
	bUT = UlamType::getEnumFromUlamTypeString(typeName.c_str()); //could be Element, etc.;
      }
    else if(tok.m_type == TOK_KW_LOCALDEF)
      {
	bUT = LocalsFileScope;
      }
    else
      {
	//check for existing Class type
	SymbolClassName * cnsym = NULL;
	if(alreadyDefinedSymbolClassName(tokid, cnsym))
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
    u32 tokid = getTokenDataAsStringId(tok);
    //is this name already a typedef for a complex type?
    if(getUlamTypeByTypedefName(tokid, uti, tmpforscalaruti) != TBOOL_TRUE)
      {
	if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    uti = makeUlamType(tok, typebitsize, arraysize, Nouti);
	  }
	else if(tok.m_type == TOK_KW_LOCALDEF)
	  {
	    UlamKeyTypeSignature lkey(tok.m_locator.getPathIndex(), 0); //bits 0
	    uti = makeUlamType(lkey, LocalsFileScope, UC_NOTACLASS);
	  }
	else
	  {
	    //check for existing Class type
	    SymbolClassName * cnsym = NULL;
	    if(alreadyDefinedSymbolClassName(tokid, cnsym))
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
    u32 tokid = getTokenDataAsStringId(args.m_typeTok);
    //is this name already a typedef for a complex type?
    if(getUlamTypeByTypedefName(tokid, uti, tmpforscalaruti) != TBOOL_TRUE)
      {
	if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    uti = makeUlamType(args.m_typeTok, args.m_bitsize, args.m_arraysize, Nouti);
	  }
	else if(args.m_typeTok.m_type == TOK_KW_LOCALDEF)
	  {
	    UlamKeyTypeSignature lkey(args.m_typeTok.m_locator.getPathIndex(), 0); //bits 0
	    uti = makeUlamType(lkey, LocalsFileScope, UC_NOTACLASS);
	  }
	else
	  {
	    //check for existing Class type
	    SymbolClassName * cnsym = NULL;
	    if(alreadyDefinedSymbolClassName(tokid, cnsym))
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

  TBOOL CompilerState::getUlamTypeByTypedefNameInClassHierarchyThenLocalsScope(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType)
  {
    //first look in the class hierarchy, bases depth-first, then this locals scope if not found.
    if(getUlamTypeByTypedefName(nameIdx, rtnType, rtnScalarType) == TBOOL_TRUE)
      return TBOOL_TRUE;
    return getUlamTypeByTypedefNameInLocalsScope(nameIdx, rtnType, rtnScalarType);
  }

  TBOOL CompilerState::getUlamTypeByTypedefNameInClassHierarchyThenLocalsScope(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType, Symbol *& asymptr)
  {
    //first look in the class hierarchy, bases depth-first, then this locals scope if not found.
    if(getUlamTypeByTypedefName(nameIdx, rtnType, rtnScalarType, asymptr) == TBOOL_TRUE)
      return TBOOL_TRUE;
    return getUlamTypeByTypedefNameInLocalsScope(nameIdx, rtnType, rtnScalarType, asymptr);
  }

  TBOOL CompilerState::getUlamTypeByTypedefName(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType)
  {
    Symbol * dropsymptr = NULL;
    return getUlamTypeByTypedefName(nameIdx, rtnType, rtnScalarType, dropsymptr);
  } //getUlamTypeByTypedefName

  TBOOL CompilerState::getUlamTypeByTypedefName(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType, Symbol* & tdsymptr)
  {
    TBOOL rtnBool = TBOOL_FALSE;
    Symbol * asymptr = NULL;
    bool hazyKin = false; //useful?

    //e.g. KEYWORDS have no m_dataindex (=0); short-circuit
    if(nameIdx == 0) return rtnBool;

    //searched back through all block's ST for idx (t3555 and ancestors; t3126 within funcdef)
    // no longer in locals filescope (hence Here).
    if(alreadyDefinedSymbolHere(nameIdx, asymptr, hazyKin))
      {
	if(asymptr->isTypedef())
	  {
	    rtnType = asymptr->getUlamTypeIdx();
	    rtnScalarType = ((SymbolTypedef *) asymptr)->getScalarUTI();
	    tdsymptr = asymptr;
	    rtnBool = TBOOL_TRUE;
	  }
	//else not found
      }
    else
      {
	//don't forget the ancestors (t41527)
	UTI cuti = getCompileThisIdx();

	if(useMemberBlock()) //(t3555 and ancestors; t3126 within funcdef)
	  {
	    NodeBlockClass* memberblock = getCurrentMemberClassBlock();
	    if(memberblock) //could be null during parsing e.g. parseMemberSelectExpr->parseIdentExpr
	      cuti = memberblock->getNodeType();
	    else
	      {
		hazyKin = true;
		return TBOOL_HAZY; // was false; //wait
	      }
	  }
	assert(okUTItoContinue(cuti));

	if(alreadyDefinedSymbolByAncestorOf(cuti, nameIdx, asymptr, hazyKin))
	  {
	    if(asymptr->isTypedef())
	      {
		rtnType = asymptr->getUlamTypeIdx();
		rtnScalarType = ((SymbolTypedef *) asymptr)->getScalarUTI();
		tdsymptr = asymptr;
		rtnBool = TBOOL_TRUE;
	      }
	  }
      }

    if((rtnBool != TBOOL_TRUE) && hazyKin)
      rtnBool = TBOOL_HAZY;

    return rtnBool;
  } //getUlamTypeByTypedefName

  TBOOL CompilerState::getUlamTypeByTypedefNameInLocalsScope(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType)
  {
    Symbol * dropsymptr = NULL;
    return getUlamTypeByTypedefNameInLocalsScope(nameIdx, rtnType, rtnScalarType, dropsymptr);
  }

  TBOOL CompilerState::getUlamTypeByTypedefNameInLocalsScope(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType, Symbol *& asymptr)
  {
    TBOOL rtnBool = TBOOL_FALSE;
    assert(asymptr == NULL);

    //e.g. KEYWORDS have no m_dataindex (=0); short-circuit
    if(nameIdx == 0) return rtnBool;

    if(isIdInLocalFileScope(nameIdx, asymptr))
      {
	if(asymptr->isTypedef())
	  {
	    rtnType = asymptr->getUlamTypeIdx();
	    rtnScalarType = ((SymbolTypedef *) asymptr)->getScalarUTI();
	    rtnBool = TBOOL_TRUE;
	  }
      }
    return rtnBool;
  } //getUlamTypeByTypedefNameInLocalsScope

  TBOOL CompilerState::getUlamTypeByTypedefNameInAnyLocalsScope(u32 nameIdx, std::map<UTI, k_localstypedefpair> & mapref)
  {
    TBOOL rtnb = TBOOL_FALSE;
    std::map<u32, NodeBlockLocals *>::iterator it;

    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	UTI tduti = Nouti;
	UTI tdscalaruti = Nouti;
	Symbol * asymptr = NULL;
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);

	UTI luti = localsblock->getNodeType();
	pushClassContext(luti, localsblock, localsblock, false, NULL);

	if(getUlamTypeByTypedefNameInLocalsScope(nameIdx, tduti, tdscalaruti, asymptr) == TBOOL_TRUE)
	  {
	    assert(asymptr->isTypedef());
	    UTI auti = asymptr->getUlamTypeIdx();
	    std::pair<std::map<UTI,k_localstypedefpair>::iterator, bool> reti;
	    k_localstypedefpair kpair(localsblock,asymptr);
	    reti = mapref.insert(std::pair<UTI,k_localstypedefpair>(auti, kpair));
	    assert(reti.second); //false if already existed, i.e. not added.
	    rtnb = TBOOL_TRUE;
	  }
	popClassContext(); //restore (t3860)
      }
    return rtnb;
  } //getUlamTypeByTypedefNameInAnyLocalsScope

  UTI CompilerState::getUlamTypeAsScalar(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    if(ut->isScalar())
      return utiArg;

    //for typedef array, the scalar is the primitive type
    // maintained in the symbol!! can't get to it from utiarg.
    ULAMTYPE bUT = ut->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();
    UTI cuti = keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx(); // what-if a ref?

    if(bUT == Class)
      {
	assert(okUTItoContinue(cuti)); //at least not Nouti (t41509)
	return cuti; //try this Mon May  2 10:36:56 2016
      }

    s32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    u32 nameid = keyOfArg.getUlamKeyTypeSignatureNameId();
    UlamKeyTypeSignature baseKey(nameid, bitsize, NONARRAYSIZE, cuti, ALT_NOT);  //default array size, was NONARRAYSIZE, not new reftype //t41660,1
    ULAMCLASSTYPE classtype = ut->getUlamClassType();
    UTI buti = makeUlamType(baseKey, bUT, classtype); //could be a new one, oops.

    //note: array of CA's is not a CA too
    return buti;
  } //getUlamTypeAsScalar

  UTI CompilerState::getUlamTypeAsArrayOfScalar(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    if(!ut->isScalar())
      return utiArg;

    //for typedef array, the scalar is the primitive type
    // maintained in the symbol!! can't get to it from utiarg.
    ULAMTYPE bUT = ut->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();
    UTI cuti = keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx(); // what-if a ref?

    s32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    u32 nameid = keyOfArg.getUlamKeyTypeSignatureNameId();
    UlamKeyTypeSignature baseKey(nameid, bitsize, UNKNOWNSIZE, cuti, ALT_NOT);
    ULAMCLASSTYPE classtype = ut->getUlamClassType();
    UTI buti = makeUlamType(baseKey, bUT, classtype); //could be a new one, oops.

    //note: array of CA's is not a CA too
    return buti;
  } //getUlamTypeAsArrayOfScalar

  UTI CompilerState::getUlamTypeAsDeref(UTI utiArg)
  {
    assert(okUTItoContinue(utiArg)); //t3172,t41449,t41450,1,2,3,5

    UlamType * ut = getUlamTypeByIndex(utiArg);

    //if(!ut->isReference()) return utiArg; //hid Nouti,Hzy,Nav; set effSelf in uv for eval tests

    ULAMTYPE bUT = ut->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();
    s32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    s32 arraysize = keyOfArg.getUlamKeyTypeSignatureArraySize();
    UTI classidx = keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx();

    if((bUT == Class) && ut->isScalar())
      return classidx; //scalar error/t3845

    UlamKeyTypeSignature baseKey(keyOfArg.m_typeNameId, bitsize, arraysize, classidx, ALT_NOT);    //default array size is zero
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
    return getUlamTypeAsRef(utiArg, ALT_REF); //default
  }

  UTI CompilerState::getUlamTypeAsRef(UTI utiArg, ALT altArg, bool isConstRef)
  {
    //constant-type modifier for non-ref uti is dropped here
    if(isConstRef && (altArg == ALT_REF))
      return getUlamTypeAsRef(utiArg, ALT_CONSTREF); //t3136
    return getUlamTypeAsRef(utiArg, altArg);
  }

  UTI CompilerState::getUlamTypeAsRef(UTI utiArg, ALT altArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    ALT utalt = ut->getReferenceType();

    if(utalt == altArg)
      return utiArg; //same alt ref type

    //e.g. an alt ref typedef
    if((utalt != ALT_NOT) && (altArg == ALT_NOT))
      return utiArg; //deref used to remove alt type

    if((utalt != ALT_NOT) && (utalt != altArg))
      {
	std::ostringstream msg;
	msg << "Attempting to ref (" << altArg << ") a reference type <" ;
	msg <<  getUlamTypeNameByIndex(utiArg) << ">";
	MSG2("", msg.str().c_str(), DEBUG);
	// error/t3729 trying to change ALT_REF to ALT_AS. continue..
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

  ULAMTYPECOMPARERESULTS CompilerState::isARefTypeOfUlamType(UTI uti1, UTI uti2)
  {
    //either may be a ref of the other; because uses checked both directions.
    // may both be references, for example, arrayitem and its reference; or,
    // may be arrayitem and its base type (i.e. no ALT_REF). (t3651, t3653)
    if(isReference(uti1) || isReference(uti2))
      return UlamType::compareForMakingCastingNode(getUlamTypeAsDeref(uti1), getUlamTypeAsDeref(uti2), *this);
    return UTIC_NOTSAME; //neither a ref
  }

  UTI CompilerState::getUlamTypeOfConstant(ULAMTYPE etype)
  {
    u32 enumStrIdx = m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(etype));
    UlamKeyTypeSignature ckey(enumStrIdx, getDefaultBitSize((UTI) etype), NONARRAYSIZE); //was ANYBITSIZECONSTANT
    return makeUlamType(ckey, etype, UC_NOTACLASS); //may not exist yet, create
  }

  UTI CompilerState::getDefaultUlamTypeOfConstant(UTI ctype)
  {
    return ctype; // use its own type
  }

  bool CompilerState::getDefaultQuark(UTI cuti, u64& dqref)
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

  TBOOL CompilerState::tryToPackAClass(UTI cuti)
  {
    //on the fly during c&l to hopefully make some progress with constant class values
    UlamType * cut = getUlamTypeByIndex(cuti);
    assert(cut->getUlamTypeEnum() == Class);

    assert(okUTItoContinue(cuti));

    if(!isComplete(cuti)) return TBOOL_HAZY;

    TBOOL rtntb = TBOOL_TRUE;
    if(cut->getSizeofUlamType() > 0)
      {
	UTI scalarcuti = getUlamTypeAsScalar(cuti);
	SymbolClass * csym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClass(scalarcuti, csym);
	assert(isDefined);
	rtntb = csym->packBitsForClassVariableDataMembers();
      }
    return rtntb;
  } //trytoPackAClass

  TBOOL CompilerState::tryToPackAClassHierarchy(UTI cuti)
  {
    //on the fly during c&l to hopefully make some progress with constant class values
    UlamType * cut = getUlamTypeByIndex(cuti);
    assert(cut->getUlamTypeEnum() == Class);

    assert(okUTItoContinue(cuti));

    if(!isComplete(cuti)) return TBOOL_HAZY;

    TBOOL rtntb = TBOOL_TRUE;

    BaseclassWalker walker;
    walker.init(cuti);

    //ulam-5 supports multiple base classes; superclass optional; all bases are shared;
    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    TBOOL trybase = tryToPackAClass(baseuti);
	    if(trybase != TBOOL_TRUE)
	      {
		rtntb = trybase;
		break;
	      }
	    walker.addAncestorsOf(basecsym); //search all
	  }
	else
	  rtntb = TBOOL_FALSE;
      } //end while

    return rtntb;
  } //trytoPackAClassHierarchy

  bool CompilerState::getDefaultClassValue(UTI cuti, BV8K& dvref)
  {
    if(!okUTItoContinue(cuti)) return false; //short-circuit

    UlamType * cut = getUlamTypeByIndex(cuti);
    assert(cut->getUlamTypeEnum() == Class);

    bool rtnb = true;
    //zero-size elements have their Element Types (ulam-4) e.g. Empty (t3802)
    if(cut->getSizeofUlamType() > 0)
      {
	UTI scalarcuti = getUlamTypeAsScalar(cuti);
	SymbolClass * csym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClass(scalarcuti, csym);
	assert(isDefined);
	if(csym->packBitsForClassVariableDataMembers() == TBOOL_TRUE) //might as well see
	  rtnb = csym->getDefaultValue(dvref); //pass along ref
	else
	  rtnb = false; //really not ready
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

    std::string dhex;
    //UTI cuti = getCompileThisIdx();
    //bool notZero =
    SymbolWithValue::getHexValueAsString(len, dval, dhex);

    //build static constant array of u32's for BV8K:
    indent(fp);
    fp->write("static const u32 vales");
    if(len == 0)
      fp->write("[1] = { ");
    else
      {
	fp->write("[(");
	fp->write_decimal_unsigned(len); // == [nwords]
	fp->write(" + 31)/32] = { ");
      }
    fp->write(dhex.c_str());
    fp->write(" };"); GCNL;

    // declare perfect size BV with constant array of defaults BV8K u32's
    indent(fp);
    fp->write("static BitVector<");
    fp->write_decimal_unsigned(len);
    fp->write("> initBV;"); GCNL;

    indent(fp);
    fp->write("static bool initdone;\n");

    indent(fp);
    fp->write("if(!initdone)\n");
    indent(fp);
    fp->write("{\n");

    m_currentIndentLevel++;

    indent(fp);
    fp->write("initdone = true;\n");

    indent(fp);
    fp->write("initBV.FromArray(vales);"); GCNL; //t3776

    m_currentIndentLevel--;

    indent(fp);
    fp->write("}"); GCNL;

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

  s32 CompilerState::getBaseClassBitSize(UTI utiArg)
  {
    UTI deref = getUlamTypeAsDeref(utiArg);
    UlamType * ut = getUlamTypeByIndex(deref);
    ULAMCLASSTYPE classtype = ut->getUlamClassType();
    if((classtype == UC_ELEMENT) || (classtype == UC_NOTACLASS))
      return 0; //not a base class
    return (ut->getBitsizeAsBaseClass()); //quarks, transients wo shared bases
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

  bool CompilerState::isAltRefType(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return ut->isAltRefType();
  }

  bool CompilerState::isConstantRefType(UTI utiArg)
  {
    return (getReferenceType(utiArg) == ALT_CONSTREF);
  }

  bool CompilerState::correctAReferenceTypeWith(UTI utiArg, UTI derefuti)
  {
    //correcting a reference with a corrected deref'd type
    UlamType * derefut = getUlamTypeByIndex(derefuti);
    if(derefut->isComplete())
      {
	//shouldn't happen now that we don't 'assumeAClassType' (t3668, t3651)
	assert(getUlamTypeByIndex(utiArg)->getUlamTypeEnum() == derefut->getUlamTypeEnum());
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
	//shouldn't happen now that we don't 'assumeAClassType' (t3668, t3651)
	assert(getUlamTypeByIndex(utiArg)->getUlamTypeEnum() == scalarut->getUlamTypeEnum());
	return true;
      }
    return false;
  } //correctAnArrayTypeWith

  bool CompilerState::isComplete(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return ut->isComplete();
  }

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
	UlamKeyTypeSignature newkey(dekey.getUlamKeyTypeSignatureNameId(), dekey.getUlamKeyTypeSignatureBitSize(), dekey.getUlamKeyTypeSignatureArraySize(), dekey.getUlamKeyTypeSignatureClassInstanceIdx(), getReferenceType(utiArg)); //use holder's reference type, maybe CONSTREF (t41195)
	ULAMTYPE detyp = derefut->getUlamTypeEnum();
	makeUlamTypeFromHolder(newkey, detyp, utiArg, derefut->getUlamClassType());

	//sanity check, informed by pesky ish 03/26/2017
	if(detyp == Class)
	  {
	    SymbolClassName * cnsym = NULL;
	    AssertBool isDefined = alreadyDefinedSymbolClassNameByUTI(derefuti, cnsym);
	    assert(isDefined);
	  }
	return true;
      } //else
    AssertBool sized = setUTISizes(utiArg, derefut->getBitSize(), derefut->getArraySize());
    assert(sized);

    UlamType * ut = getUlamTypeByIndex(utiArg);
    if(!ut->isComplete())
      {
	if(ut->getUlamTypeEnum() == Class)
	  {
	    assert(ut->getUlamClassType() == UC_UNSEEN);
	    replaceUlamTypeForUpdatedClassType(ut->getUlamKeyTypeSignature(), Class, derefut->getUlamClassType(), derefut->isCustomArray()); //e.g. error/t3763
	  }
	else
	  abortShouldntGetHere(); //why not!!?
      }
    return getUlamTypeByIndex(utiArg)->isComplete();
  } //completeAReferenceTypeWith

  bool CompilerState::isHolder(UTI utiArg)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    return (ut->isHolder());
  }

  //updates key. we can do this now that UTI is used and the UlamType
  // * isn't saved; return false if ERR.  Special Case: used by
  // NodeTypeDescriptorArray, sometimes a holder completed by a scalar
  // typedef type needs to be corrected at be an array type (t41301)
  bool CompilerState::setUTIArraySize(UTI utiArg, s32 arraysize)
  {
    return setUTISizes(utiArg, getBitSize(utiArg), arraysize, true); //keep current bitsize
  }

  //updates key. we can do this now that UTI is used and the UlamType * isn't saved;
  // return false if ERR
  bool CompilerState::setUTIBitSize(UTI utiArg, s32 bits)
  {
    return setUTISizes(utiArg, bits, getArraySize(utiArg), false); //keep current arraysize
  }

  // return false if ERR
  bool CompilerState::setUTISizes(UTI utiArg, s32 bitsize, s32 arraysize, bool makescalarintoarray)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);

    if(!okUTItoContinue(utiArg))
      return false;

    if(isHolder(utiArg))
      return setSizeAsHolderArray(utiArg, bitsize, arraysize); //t3765,t41265

    //redirect primitives and arrays (including class arrays)
    if(ut->isPrimitiveType() || !ut->isScalar())
      {
	return setSizesOfNonClassAndArrays(utiArg, bitsize, arraysize, makescalarintoarray);
      }

    if(ut->isComplete() && !makescalarintoarray)
      return true;  //nothing to do

    //but sometimes we need to reset the array size of a completed scalar type (t41301)
    assert(!ut->isComplete() || (arraysize != ut->getArraySize()));

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
	    msg << ut->getUlamTypeClassNameBrief(utiArg).c_str() << " with " << total << " bits";
	    MSG2(getFullLocationAsString(getCompileThisLoc()).c_str(), msg.str().c_str(), ERR);

	    noteClassDataMembersTypeAndName(utiArg, total); //t3155
	    return false;
	  }
      }

    if(classtype == UC_QUARK)
      {
	if(total > MAXBITSPERQUARK)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXBITSPERQUARK << ") for quark ";
	    msg << ut->getUlamTypeClassNameBrief(utiArg).c_str() << " with " << total << " bits";
	    MSG2(getFullLocationAsString(getCompileThisLoc()).c_str(), msg.str().c_str(), ERR);

	    noteClassDataMembersTypeAndName(utiArg, total); //t41013
	    return false;
	  }
      }

    if(classtype == UC_TRANSIENT)
      {
	if(total > MAXBITSPERTRANSIENT)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXBITSPERTRANSIENT << ") for transient ";
	    msg << ut->getUlamTypeClassNameBrief(utiArg).c_str() << " with " << total << " bits";
	    MSG2(getFullLocationAsString(getCompileThisLoc()).c_str(), msg.str().c_str(), ERR);

	    noteClassDataMembersTypeAndName(utiArg, total);
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

    updateUTIAliasForced(utiArg, utiArg); //reset any alias back to root

    {
      std::ostringstream msg;
      msg << "Sizes SET for Class: " << newut->getUlamTypeName().c_str() << " (UTI" << utiArg << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }


    return true;
  } //setUTISizes

  // return false if ERR; skips elements
  bool CompilerState::setBaseClassBitSize(UTI utiArg, s32 basebitsize)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);

    if(!okUTItoContinue(utiArg))
      return false;

    if(ut->isPrimitiveType() || !ut->isScalar())
      return false;

    ULAMCLASSTYPE classtype = ut->getUlamClassType();

    if(classtype == UC_ELEMENT)
      {
	//shared bases reflected in element's total bitsize
	return true; //done
      }

    //verify total bits is within limits for quarks and transients
    if(classtype == UC_QUARK)
      {
	if(basebitsize > MAXBITSPERQUARK)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXBITSPERQUARK << ") for a base quark ";
	    msg << ut->getUlamTypeClassNameBrief(utiArg).c_str() << " with " << basebitsize << " bits";
	    MSG2(getFullLocationAsString(getCompileThisLoc()).c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
      }

    if(classtype == UC_TRANSIENT)
      {
	if(basebitsize > MAXBITSPERTRANSIENT)
	  {
	    std::ostringstream msg;
	    msg << "Trying to exceed allotted bit size (" << MAXBITSPERTRANSIENT << ") for a base transient ";
	    msg << ut->getUlamTypeClassNameBrief(utiArg).c_str() << " with " << basebitsize << " bits";
	    MSG2(getFullLocationAsString(getCompileThisLoc()).c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
      }
    ut->setBitsizeAsBaseClass(basebitsize);
    return true;
  } //setBaseClassBitSize

  void CompilerState::noteClassDataMembersTypeAndName(UTI cuti, s32 totalsize)
  {
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    NodeBlockClass * cblock = csym->getClassBlockNode();
    assert(cblock);
    cblock->noteDataMembersParseTree(cuti, totalsize);
  }

  void CompilerState::verifyZeroSizeUrSelf()
  {
    assert(okUTItoContinue(m_urSelfUTI));
    UlamType * urselfut = getUlamTypeByIndex(m_urSelfUTI);
    if(urselfut->getTotalBitSize() != 0)
      {
	std::ostringstream msg;
        msg << getUlamTypeNameBriefByIndex(m_urSelfUTI).c_str();
	msg << " has a NON-ZERO size";
	MSG2("", msg.str().c_str() , ERR);
      }
  }

  void CompilerState::verifyZeroRegistryIdForUrSelf()
  {
    u32 urselfrn = getAClassRegistrationNumber(m_urSelfUTI);
    if(urselfrn != 0)
      {
	std::ostringstream msg;
        msg << getUlamTypeNameBriefByIndex(m_urSelfUTI).c_str();
	msg << " has a NON-ZERO Registry Number (";
	msg << urselfrn << ")";
	MSG2("", msg.str().c_str() , ERR);
      }
  }

  u32 CompilerState::mergeClassUTI(UTI olduti, UTI cuti, Locator loc)
  {
    UlamKeyTypeSignature key1 = getUlamKeyTypeSignatureByIndex(olduti);
    UlamKeyTypeSignature key2 = getUlamKeyTypeSignatureByIndex(cuti);

    //bitsize of old could still be "unknown" (before size set, but args known and match 'cuti').
    assert(key1.getUlamKeyTypeSignatureNameId() == key2.getUlamKeyTypeSignatureNameId());

    if(key1.getUlamKeyTypeSignatureReferenceType() != key2.getUlamKeyTypeSignatureReferenceType())
      return 0; //don't destory olduti if either is a reference of the other

    u32 count = replaceUTIKeyAndAlias(olduti, cuti); //t3327
    {
      std::ostringstream msg;
      msg << "MERGED keys of duplicate Class (UTI " << olduti << ") WITH: ";
      msg << getUlamTypeNameByIndex(cuti).c_str() << " (UTI " << cuti << ")";
      msg << " count is now at " << count;

#define PERCENTAGE_THRESOLD_DUPLICATE_DIVISOR 10

      if((olduti > 100) && (count >= ((u32)(olduti/PERCENTAGE_THRESOLD_DUPLICATE_DIVISOR))))
	{
	  msg << "; TOO COMMON, SUSPECT LOOPING"; // (t41455)
	}
      else
	count = 0; //clear ok

      MSG2(getFullLocationAsString(loc).c_str(), msg.str().c_str(), DEBUG);
    }
    return count; // >0 too many
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
  } //findaUTIAlias

  bool CompilerState::findRootUTIAlias(UTI auti, UTI& aliasuti)
  {
    assert(auti < m_unionRootUTI.size());
    bool foundroot = false;
    u32 cntr = 0;
    UTI finduti = auti;
    const u32 MAXCHAIN = 7;
    while(!foundroot && (cntr++ < MAXCHAIN))
      {
	UTI xuti = finduti;
	finduti = m_unionRootUTI[xuti];
	if(finduti == xuti)
	  foundroot = true;
      }
    if(isAClass(auti) && !isReference(auti) && isScalar(auti) && isAClass(finduti))
      {
	//cannot assume an anonymous class has a class alias - 20210724-162342 ish)
	UlamKeyTypeSignature key = getUlamTypeByIndex(finduti)->getUlamKeyTypeSignature();
	assert(finduti == key.getUlamKeyTypeSignatureClassInstanceIdx()); //invarient
      }
    assert(cntr <= MAXCHAIN); //big enough?
    if(auti != finduti)
      {
	aliasuti = finduti; //no change o.w.
	return true;
      }
    return false;
  } //findRootUTIAlias

  UTI CompilerState::lookupUTIAlias(UTI auti)
  {
    assert(auti < m_unionRootUTI.size());
    return m_unionRootUTI[auti];
  } //lookupClassUTIAlias

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
    //assert uses findRootUTIAlias to answer Stub question; expensive (20210722 ish)
    //assert(!isAClass(buti) || !isClassAStub(auti) || !isClassATemplate(buti)); //don't alias a stub to a template (t3451)
    m_unionRootUTI[auti] = buti;
    {
      std::ostringstream msg;
      msg << "ALIASES for (UTI" << auti << ") ";
      msg << getUlamTypeNameByIndex(auti).c_str();
      msg << " is update to (UTI" << buti << ") ";
      msg << getUlamTypeNameByIndex(buti).c_str();
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
      msg << getUlamTypeNameByIndex(auti).c_str();
      msg << " is initialized to self";
      MSG2("", msg.str().c_str(), DEBUG);
    }
    return;
  } //initUTIAlias

  // return false if ERR
  // for primitives, and all arrays (class and nonclass)
  // makescalarintoarray is by default false (used by NodeTypeDescriptorArray t41301)
  bool CompilerState::setSizesOfNonClassAndArrays(UTI utiArg, s32 bitsize, s32 arraysize, bool makescalarintoarray)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    assert((ut->getUlamTypeEnum() != Class) || !ut->isScalar());

    if(ut->isComplete() && !makescalarintoarray)
      return true;  //nothing to do

    //but sometimes we need to reset the array size of a completed scalar type (t41301)
    assert(!ut->isComplete() || (arraysize != ut->getArraySize()));

    assert(!ut->isHolder()); //t41265, sent to setSizeAsHolderArray

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
	    // disallow an array of Void(0)'s (t3389)
	    std::ostringstream msg;
	    msg << "Invalid Void type array: " << ut->getUlamTypeName().c_str();
	    MSG2(getFullLocationAsString(getCompileThisLoc()).c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
      }
    else if(bUT == Void)
      {
	std::ostringstream msg;
	msg << "Invalid nonzero bitsize (" << bitsize << ") for Void type";
	MSG2(getFullLocationAsString(getCompileThisLoc()).c_str(), msg.str().c_str(), ERR);
	return false;
      }
    else if(total > MAXBITSPERTRANSIENT)
      {
	std::ostringstream msg;
	msg << "Trying to exceed allotted bit size (" << MAXBITSPERTRANSIENT << ") for array ";
	msg << ut->getUlamTypeNameBrief().c_str() << " with " << total << " bits";
	MSG2(getFullLocationAsString(getCompileThisLoc()).c_str(), msg.str().c_str(), ERR);
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

    updateUTIAliasForced(utiArg, utiArg); //reset any alias back to root

    {
      std::ostringstream msg;
      msg << "Sizes set for nonClass: " << newut->getUlamTypeName().c_str();
      msg << " (UTI" << utiArg << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
    return true;
  } //setSizesOfNonClassAndArrays

    // return false if ERR
  // for primitives, and all arrays (class and nonclass)
  // makescalarintoarray is by default false (used by NodeTypeDescriptorArray t41301)
  bool CompilerState::setSizeAsHolderArray(UTI utiArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utiArg);
    assert(ut->isHolder());

    ULAMTYPE bUT = ut->getUlamTypeEnum();
    //disallow zero-sized primitives (no such thing as a BitVector<0u>)
    //'Void' by default is zero and only zero bitsize (already complete)
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
    ULAMCLASSTYPE classtype = ut->getUlamClassType();

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

    updateUTIAliasForced(utiArg, utiArg); //reset any alias back to root

    {
      std::ostringstream msg;
      msg << "Sizes set for Holder Array: " << newut->getUlamTypeName().c_str();
      msg << " (UTI" << utiArg << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
    return true;
  } //setSizeAsHolderArray

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

  bool CompilerState::isClassATemplate(NodeBlockClass * cblock)
  {
    UTI cuti = cblock->getNodeType();
    assert(okUTItoContinue(cuti));

    if(!isAClass(cuti))
      return false;

    //this version doesn't look up the 'cuti' class symbol, which may not exist yet (t41361)
    SymbolClassNameTemplate * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassNameTemplateByUTI(cuti, cnsym);
    assert(isDefined);

    assert(cnsym->isClassTemplate());

    return (cuti == cnsym->getUlamTypeIdx());
  } //isClassATemplate

  bool CompilerState::isClassATemplateInstance(UTI cuti)
  {
    //can also check isClassAStub before fullyInstantiated.
    assert(okUTItoContinue(cuti));

    if(!isAClass(cuti))
      return false;

    NodeBlockClass * cblock = getAClassBlock(cuti);
    assert(cblock);

    return (cblock->getNumberOfArgumentNodes() > 0);
  } //isClassATemplateInstance

  UTI CompilerState::getTemplateTypeOfStub(UTI stubuti)
  {
    UTI tuti = Nouti;
    SymbolClassName * cnsym = NULL;
    if(alreadyDefinedSymbolClassNameByUTI(stubuti, cnsym))
      {
	tuti = cnsym->getUlamTypeIdx();
	assert(cnsym->isClassTemplate());
      }
    return tuti;
  }

  //FORMERLY RETURNED SUPERCLASS uti, Nouti or Hzy.
  bool CompilerState::isClassASubclass(UTI cuti)
  {
    u32 count = 0;
    UTI subuti = getUlamTypeAsDeref(getUlamTypeAsScalar(cuti)); //in case of array

    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(subuti, csym))
      {
	count = csym->getBaseClassCount(); //added bases
	//super optional, UrSelf is default superclass
	UTI superuti = csym->getBaseClass(0);
	if((superuti != Nouti) && !isUrSelf(superuti))
	  count++; //Hzy if UNSEEN counts here
      }
    return (count > 0);
  } //isClassASubclass

  //return true if the second arg is a base class of the first arg.
  // i.e. cuti is a subclass of base. recurses the family tree.
  bool CompilerState::isClassASubclassOf(UTI cuti, UTI basep)
  {
    bool hasbase = false;
    UTI derefbasep = getUlamTypeAsDeref(basep);
    UTI subcuti = getUlamTypeAsDeref(cuti); //init for the loop

    if(subcuti==derefbasep)
      return false; //t41312, all gen code

    BaseclassWalker walker; //use default, doesn't really matter..all shared.
    walker.init(subcuti);

    //ulam-5 supports multiple base classes; superclass optional;
    UTI baseuti = Nouti;
    while(!hasbase && walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    hasbase = (basecsym->isABaseClassItem(derefbasep) >= 0); //t3281

	    if(!hasbase)
	      walker.addAncestorsOf(basecsym);
	  }
      } //end while
    return hasbase; //even for non-classes
  } //isClassASubclassOf

  UTI CompilerState::getSuperBaseclassType(UTI cuti)
  {
    UTI subuti = getUlamTypeAsDeref(getUlamTypeAsScalar(cuti)); //in case of array
    UTI superuti = 0;

    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(subuti, csym))
      {
	superuti = csym->getBaseClass(0);
      }
    return superuti;
  }

  //return true if the second arg is a base class of the first arg.
  // i.e. cuti is a subclass of base. recurses the family tree.
  bool CompilerState::isBaseClassADirectAncestorOf(UTI cuti, UTI basep)
  {
    bool hasbase = false;

    UTI defcuti = getUlamTypeAsDeref(cuti);
    UTI defbasep = getUlamTypeAsDeref(basep);

    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(defcuti, csym))
      {
	hasbase = (csym->isABaseClassItem(defbasep) >= 0); //t41308
      }
    return hasbase; //even for non-classes
  } //isBaseClassADirectAncestorOf

  //return true if a baseclass of the first arg starts with id.
  // i.e. cuti is a subclass of basep. recurses the family tree.
  u32 CompilerState::findClassAncestorWithMatchingNameid(UTI cuti, u32 nameid, UTI& basep)
  {
    u32 countids = 0;
    UTI subcuti = getUlamTypeAsDeref(cuti); //init for the loop

    BaseclassWalker walker; //use default; doesn't really matter
    walker.init(subcuti);

    //ulam-5 supports multiple base classes; superclass optional;
    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    if(getUlamTypeByIndex(baseuti)->getUlamTypeNameId() == nameid)
	      {
		basep = baseuti;
		countids++;
	      }
	    walker.addAncestorsOf(basecsym); //search all
	  }
      } //end while

    return countids;
  } //findClassAncestorWithMatchingNameid

  //return total count of shared base classes in the hierarchy of the first arg,
  // and updated mapref of each with number of sharers in 2nd arg; recurses the family tree.
  u32 CompilerState::findTheSharedVirtualBasesInAClassHierarchy(UTI cuti, std::map<UTI, u32>& svbmapref)
  {
    u32 count = 0;
    UTI subcuti = getUlamTypeAsDeref(cuti); //init for the loop

    BaseclassWalker walker;
    walker.init(subcuti);

    //ulam-5 supports multiple base classes; superclass optional;
    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this) )
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    u32 bcnt = basecsym->findDirectSharedBases(svbmapref);
	    count += bcnt;

	    walker.addAncestorsOf(basecsym); //search all
	  }
      } //end while

    return count;
  } //findTheSharedVirtualBasesInAClassHierarchy

  void CompilerState::resetABaseClassType(UTI cuti, UTI olduti, UTI newuti)
  {
    if( (olduti == newuti) || UlamType::compare(olduti, newuti, *this) == UTIC_SAME)
      return;

    UTI subuti = getUlamTypeAsDeref(getUlamTypeAsScalar(cuti)); //in case of array
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(subuti, csym))
      {
	s32 item = csym->isABaseClassItem(olduti);
	if(item >= 0)
	  csym->updateBaseClass(olduti, (u32) item, newuti);
      }
  } //resetABaseClassType

  void CompilerState::resetABaseClassItem(UTI cuti, UTI olduti, UTI newuti, u32 item)
  {
    if( (olduti == newuti) || UlamType::compare(olduti, newuti, *this) == UTIC_SAME)
      return;

    UTI subuti = getUlamTypeAsDeref(getUlamTypeAsScalar(cuti)); //in case of array
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(subuti, csym))
      {
	csym->updateBaseClass(olduti, (u32) item, newuti);
      }
  } //resetABaseClassItem

  bool CompilerState::getABaseClassRelativePositionInAClass(UTI cuti, UTI basep, u32& relposref)
  {
    UTI defcuti = getUlamTypeAsDeref(cuti); //t3754
    UTI defbasep = getUlamTypeAsDeref(basep); //t3824
    if(UlamType::compare(defcuti, defbasep, *this) == UTIC_SAME)
      {
	relposref = 0; //special case, base class is self
	return true;
      }
    s32 accumpos = 0;
    bool hasbase = false;

    BaseclassWalker walker;
    walker.init(defcuti); //t3831

    //ulam-5 supports multiple base classes; superclass optional; all bases are shared;
    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this) && !hasbase)
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    UTI foundinbase;
	    hasbase = findNearestBaseClassToAnAncestor(baseuti, defbasep, foundinbase);
	    if(hasbase)
	      {
		SymbolClass * foundbasecsym = NULL;
		AssertBool isDefined = alreadyDefinedSymbolClass(foundinbase, foundbasecsym);
		assert(isDefined);

		s32 foundbaseitem = foundbasecsym->isABaseClassItem(defbasep); //direct
		if(foundbaseitem >= 0)
		  {
		    s32 pos = foundbasecsym->getBaseClassRelativePosition(foundbaseitem);
		    if(pos == UNKNOWNSIZE)
		      {
			accumpos = UNRELIABLEPOS; //t41619
			hasbase = false;
			break;
		      }
		    assert(pos >= 0);
		    accumpos += pos; //more specific position within nextbase
		  }
		else
		  {
		    //all bases are "shared"; like getASharedBaseClassRelativePositionInAClass
		    s32 foundsharedbaseitem = foundbasecsym->isASharedBaseClassItem(defbasep); //direct
		    if(foundsharedbaseitem >= 0)
		      {
			s32 pos = foundbasecsym->getSharedBaseClassRelativePosition(foundsharedbaseitem);
			if(pos == UNKNOWNSIZE)
			  {
			    accumpos = UNRELIABLEPOS;
			    hasbase = false;
			    break;
			  }
			assert(pos >= 0);
			accumpos += pos; //more specific position within nextbase
		      }
		    else
		      walker.addAncestorsOf(foundbasecsym); //t3565, t3568
		  }
	      }
	    else
	      walker.addAncestorsOf(basecsym); //search all (error:t3842,41094,t41158)
	  }
      } //end while

    relposref = accumpos;
    return hasbase; //false for non-classes
  } //getABaseClassRelativePositionInAClass

  bool CompilerState::getASharedBaseClassRelativePositionInAClass(UTI cuti, UTI basep, u32& relposref)
  {
    UTI defcuti = getUlamTypeAsDeref(cuti);
    UTI defbasep = getUlamTypeAsDeref(basep);
    if(UlamType::compare(defcuti, defbasep, *this) == UTIC_SAME)
      return false; //not shared (t3102)

    //ulam-5 supports shared base classes;
    bool hasbase = false;
    SymbolClass * csym = NULL;
    s32 pos = UNKNOWNSIZE;
    if(alreadyDefinedSymbolClass(defcuti, csym))
      {
	s32 sharedbaseitem = csym->isASharedBaseClassItem(defbasep);
	if(sharedbaseitem >= 0)
	  {
	    pos = csym->getSharedBaseClassRelativePosition(sharedbaseitem);
	    assert(pos >= 0);
	    hasbase = true;
	  }
      }
    relposref = pos;
    return hasbase; //even for non-classes (t3300)
  } //getASharedBaseClassRelativePositionInAClass

  bool CompilerState::isClassAStub(UTI cuti)
  {
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      return csym->isStub();

    return false; //even for non-classes
  } //isClassAStub

  bool CompilerState::isClassAStubCopy(UTI cuti)
  {
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      return csym->isStubCopy();

    return false; //even for non-classes
  } //isClassAStubCopy

  UTI CompilerState::getStubCopyOf(UTI baseuti)
  {
    SymbolClass * basecsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(baseuti, basecsym);
    assert(isDefined); //will be in TEMP map if a stubcopy
    //may no longer be a stub copy, upgraded from its stub "of"
    return basecsym->getStubCopyOf();
  }

  bool CompilerState::hasClassAStubInHierarchy(UTI cuti)
  {
    bool hasstub = false;

    BaseclassWalker walker; //searches entire tree until a stub is found
    walker.init(cuti);

    UTI baseuti = Nouti;
    //ulam-5 supports multiple base classes; superclass optional;
    while(!hasstub && walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    hasstub = basecsym->isStub();

	    if(!hasstub)
	      walker.addAncestorsOf(basecsym);
	    else
	      {
		//sanity check (t41440)
		assert(!isClassATemplate(cuti) || basecsym->isStubForTemplate());
	      }
	  }
      } //end while
    return hasstub; //even for non-classes
  } //hasClassAStubInHierarchy

  bool CompilerState::isClassAMemberStubInATemplate(UTI baseuti)
  {
    //not localscope (t3852,..)
    return (isAClass(baseuti) && (getMemberStubForTemplateType(baseuti) != Nouti));
  }

  UTI CompilerState::getMemberStubForTemplateType(UTI baseuti)
  {
    SymbolClass * basecsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(baseuti, basecsym);
    assert(isDefined);
    return basecsym->getStubForTemplateType();
  }

  void CompilerState::setStubFlagForThisClassTemplate(UTI baseuti)
  {
    UTI cuti = getCompileThisIdx();
    if((isClassATemplate(cuti) || isClassAMemberStubInATemplate(cuti)) && isClassAStub(baseuti))
      {
	SymbolClass * basecsym = NULL;
	AssertBool isDefined = alreadyDefinedSymbolClass(baseuti, basecsym);
	assert(isDefined);
	//assert(basecsym->getStubForTemplateType() == Nouti); t41223 Parser may reset contexts..
	basecsym->setStubForTemplateType(cuti); //t41440
      }
  }

  UTI CompilerState::getContextForPendingArgValuesForStub(UTI stubuti)
  {
    UTI context = Nouti;
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(stubuti, csym);
    assert(isDefined);
    //    if(csym->isStub())
      {
	context = csym->getContextForPendingArgValues();
	context = lookupUTIAlias(context);
      }
    return context;
  }

  UTI CompilerState::getContextForPendingArgTypesForStub(UTI stubuti)
  {
    UTI typecontext = Nouti;
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(stubuti, csym);
    assert(isDefined);
    //if(csym->isStub())
      {
	typecontext = csym->getContextForPendingArgTypes();
	typecontext = lookupUTIAlias(typecontext);
      }
    return typecontext;
  }

  bool CompilerState::turnWaitMessageIntoErrorMessage()
  {
    return turnWaitMessageIntoErrorMessage(getCompileThisIdx());
  }

  bool CompilerState::turnWaitMessageIntoErrorMessage(UTI cuti)
  {
    if(!isAClass(cuti) || isClassATemplate(cuti) || isClassAMemberStubInATemplate(cuti))
      return false; //t41440 special case become debug messages instead;t41204

    UTI context = getContextForPendingArgValuesForStub(cuti);
    if((context != Nouti) && (context != cuti))
      return turnWaitMessageIntoErrorMessage(context); //t3328, a stub

    return true;
  } //recursive helper

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

    //deref cuti (t3653, t3942, t3947, t3998, t41000, t41001, t41071)
    if(isAClass(cuti))
      return hasCustomArrayInAClassOrAncestor(getUlamTypeAsDeref(cuti));

    return false; //even for non-classes, t3543
  } //isClassACustomArray

  UTI CompilerState::getAClassCustomArrayType(UTI cuti)
  {
    assert(isScalar(cuti));
    NodeBlockClass * cblock = getAClassBlock(cuti); //checks via classblock in case of inheritance
    assert(cblock);
    return cblock->getCustomArrayTypeFromGetFunction(cuti); //returns canonical type
  } //getAClassCustomArrayType

  u32 CompilerState::getAClassCustomArrayIndexType(UTI cuti, Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    assert(isScalar(cuti));
    NodeBlockClass * cblock = getAClassBlock(cuti); //checks via classblock in case of inheritance
    assert(cblock);
    //returns number of matching types; updates last two args.
    return cblock->getCustomArrayIndexTypeFromGetFunction(cuti, rnode, idxuti, hasHazyArgs);
  } //getAClassCustomArrayIndexType

  bool CompilerState::hasAClassCustomArrayLengthof(UTI cuti)
  {
    assert(isScalar(cuti));
    NodeBlockClass * cblock = getAClassBlock(cuti); //checks via classblock in case of inheritance
    assert(cblock);
    return cblock->hasCustomArrayLengthofFunction(cuti);
  } //hasAClassCustomArrayLengthof

  bool CompilerState::hasCustomArrayInAClassOrAncestor(UTI cuti)
  {
    bool hasCA = false;
    // custom array flag set at parse time

    BaseclassWalker walker; //search for existence
    walker.init(cuti);

    UTI baseuti = Nouti;
    //ulam-5 supports multiple base classes; superclass optional;
    while(!hasCA && walker.getNextBase(baseuti, *this))
      {
	if(okUTItoContinue(baseuti))
	  {
	    UlamType * baseut = getUlamTypeByIndex(baseuti);
	    hasCA = ((UlamTypeClass *) baseut)->isCustomArray();

	    if(!hasCA)
	      {
		SymbolClass * basecsym = NULL;
		if(alreadyDefinedSymbolClass(baseuti, basecsym))
		    walker.addAncestorsOf(basecsym);
	      } //else hasCA
	  } //not ok
      } //end while
    return hasCA;
  } //hasCustomArrayInAClassOrAncestor

  bool CompilerState::alreadyDefinedSymbolClassName(u32 dataindex, SymbolClassName * & symptr)
  {
    return m_programDefST.isInTable(dataindex,(Symbol * &) symptr);
  }

  bool CompilerState::alreadyDefinedSymbolClassNameByUTI(UTI suti, SymbolClassName * & symptr)
  {
    if(!okUTItoContinue(suti))
      return false;

    //gets name id from suti key
    return alreadyDefinedSymbolClassName(getUlamTypeNameIdByIndex(suti), symptr);
  } //helper

  bool CompilerState::alreadyDefinedSymbolClassNameTemplate(u32 dataindex, SymbolClassNameTemplate * & symptr)
  {
    bool rtnb = alreadyDefinedSymbolClassName(dataindex, (SymbolClassName *&) symptr);
    if(rtnb && !((SymbolClassName *) symptr)->isClassTemplate())
      {
	std::ostringstream msg;
	msg << "Class without parameters already exists with the same name: ";
	msg << m_pool.getDataAsString(symptr->getId()).c_str();
	msg << ", and was first noticed at: ."; //..
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR); //parsing

	std::ostringstream imsg;
	imsg << ".. this location"; //(t3556, t3340)
	MSG2(getFullLocationAsString(symptr->getLoc()).c_str(), imsg.str().c_str(), ERR);
      }
    return (rtnb && symptr->isClassTemplate());
  } //alreadyDefinedSymbolClassNameTemplate

  bool CompilerState::alreadyDefinedSymbolClassNameTemplateByUTI(UTI suti, SymbolClassNameTemplate * & symptr)
  {
    if(!okUTItoContinue(suti))
      return false;

    return alreadyDefinedSymbolClassNameTemplate(getUlamTypeNameIdByIndex(suti), symptr);
  } //helper

  //if necessary, searches for instance of class "template" with matching SCALAR uti;
  // automatically reduces arrays and references to their base class UTI.
  bool CompilerState::alreadyDefinedSymbolClass(UTI uti, SymbolClass * & symptr)
  {
    bool rtnb = false;
    UlamType * ut = getUlamTypeByIndex(uti);

    SymbolClassName * cnsym = NULL;
    if(alreadyDefinedSymbolClassNameByUTI(uti, cnsym))
      {
	UTI scalarUTI = uti;
	if(!ut->isScalar())
	  scalarUTI = getUlamTypeAsScalar(uti); //ALT_ARRAYITEM no longer
	scalarUTI = getUlamTypeAsDeref(scalarUTI); //and deref

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
		if(findRootUTIAlias(scalarUTI, mappedUTI)) //t3326
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
    assert(!isALocalsFileScope(cuti));
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    csym->addUnknownTypeTokenToClass(tok, huti);
  } //addUnknownTypeTokenToThisClassResolver

  //temporary type name which will be updated during type labeling.
  void CompilerState::addUnknownTypeTokenToThisClassResolver(const Token& tok, UTI huti)
  {
    assert(!isThisLocalsFileScope());
    addUnknownTypeTokenToAClassResolver(getCompileThisIdx(), tok, huti);
  } //addUnknownTypeTokenToThisClassResolver

  //temporary type name which will be updated during type labeling.
  Token CompilerState::removeKnownTypeTokenFromThisClassResolver(UTI huti)
  {
    assert(!isThisLocalsFileScope());
    UTI cuti = getCompileThisIdx(); //not memberclass
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    return csym->removeKnownTypeTokenFromClass(huti);
  } //removeKnownTypeTokenFromThisClassResolver

  //before removing, check existence
  bool CompilerState::hasUnknownTypeInThisClassResolver(UTI huti)
  {
    if(isThisLocalsFileScope())
      return false;

    UTI cuti = getCompileThisIdx(); //not memberclass
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    return csym->hasUnknownTypeInClass(huti);
  } //hasUnknownTypeInThisClassResolver

  //false if not there, or still hazy; true if resolved; called during c&l
  bool CompilerState::statusUnknownTypeInAClassResolver(UTI acuti, UTI huti)
  {
    pushClassOrLocalContextAndDontUseMemberBlock(acuti);

    bool rtnb = statusUnknownTypeInThisClassResolver(huti);

    popClassContext(); //restore
    return rtnb;
  } //helper

  //false if not there, or still hazy; true if resolved; called during c&l
  bool CompilerState::statusUnknownTypeInThisClassResolver(UTI huti)
  {
    assert(!isThisLocalsFileScope());
    UTI cuti = getCompileThisIdx();
#if 1
    //just this class (original way)
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->statusUnknownTypeInClass(huti);
#else
    //all template class instances with same name
    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassNameByUTI(cuti, cnsym);
    assert(isDefined);
    return cnsym->statusUnknownTypeInClassInstances(huti);
#endif
  } //statusUnknownTypeInThisClassResolver

  bool CompilerState::statusUnknownClassTypeInThisLocalsScope(const Token& tok, UTI huti, UTI& rtnuti)
  {
    //because localsscope has no resolver, and therefore, no unknown type resolution for class types
    assert(isHolder(huti));
    assert(isThisLocalsFileScope());
    if((tok.m_dataindex == 0) || (tok.m_type != TOK_TYPE_IDENTIFIER))
      return false;

    bool rtnb = false;
    u32 tokid = getTokenDataAsStringId(tok);
    UTI kuti = huti; //init

    UTI tmpscalar = Nouti;
    Symbol * ltdsymptr = NULL;
    if(getUlamTypeByTypedefNameInLocalsScope(tokid, kuti, tmpscalar, ltdsymptr) == TBOOL_TRUE)
      {
	assert(ltdsymptr && ltdsymptr->isTypedef());
	if(ltdsymptr->isCulamGeneratedTypedef())
	  {
	    if(ltdsymptr->isCulamGeneratedTypedefAliased())
	      return true;
	    else //handled by CS::checkforAnyRemainingCulamGeneratedTypedefsInThisContext
	      return false;
	  }
	rtnb = !isHolder(kuti);
      }

    if(!rtnb)
      {
	SymbolClassName * cnsym = NULL;
	if((rtnb = alreadyDefinedSymbolClassName(tok.m_dataindex, cnsym)))
	  {
	    if(cnsym->isClassTemplate())
	      {
		SymbolClass * csym = NULL;
		if(alreadyDefinedSymbolClass(huti, csym))
		  {
		    kuti = csym->getUlamTypeIdx(); //perhaps an alias
		  }
		else if(((SymbolClassNameTemplate *)cnsym)->findClassInstanceByUTI(huti, csym))
		  {
		    kuti = csym->getUlamTypeIdx(); //also checks for mapped uti
		  }
		else
		  {
		    rtnb = false; //t3868
		  }
	      }
	    else
	      kuti = cnsym->getUlamTypeIdx();
	  } //no class name
      }

    if(rtnb)
      {
	assert(!isHolder(kuti));
	cleanupExistingHolder(huti, kuti); //t3868 kuti is Nav;
	rtnuti = kuti; //? only if rtnb is true?
      }
    return rtnb;
  } //statusUnknownClassTypeInThisLocalsScope

  bool CompilerState::removeIncompleteClassSymbolFromProgramTable(u32 id)
  {
    Token ntok(TOK_TYPE_IDENTIFIER, m_locOfNextLineText, id); //junk loc
    return removeIncompleteClassSymbolFromProgramTable(ntok);
  }

  bool CompilerState::removeIncompleteClassSymbolFromProgramTable(const Token& nTok)
  {
    bool rtnb = false;
    assert(nTok.m_type == TOK_TYPE_IDENTIFIER);
    u32 id = nTok.m_dataindex;
    SymbolClassName * cnsym = NULL;
    if(alreadyDefinedSymbolClassName(id, cnsym))
      {
	UTI cuti = cnsym->getUlamTypeIdx();
	UlamType * cut = getUlamTypeByIndex(cuti);

	if((cut->getUlamClassType() == UC_UNSEEN) || (cut->getUlamClassType() == UC_LOCALSFILESCOPE))
	  {
	    UlamKeyTypeSignature ckey = cut->getUlamKeyTypeSignature();

	    deleteUlamKeyTypeSignature(ckey, cuti); //decrements counter

	    resetUnseenClassId(cnsym, nTok); //before removing it from programDefST

	    Symbol * rmcnsym  = NULL;
	    AssertBool isGone = m_programDefST.removeFromTable(id, rmcnsym);
	    assert(isGone);
	    assert(rmcnsym == cnsym);
	    delete rmcnsym;
	    rmcnsym = cnsym = NULL;
	    rtnb = true;
	  }
	else
	  {
	    assert(cut->isComplete()); //t3381, t3385
	  }
      }
    //else don't call remove!
    return rtnb;
  } //removeIncompleteClassSymbolFromProgramTable

  s32 CompilerState::cleanupAllGeneratedLocalsTypedefsOfThisClassHolder(u32 dataindex, UTI& cuti, bool& isaclassholder)
  {
    std::map<UTI,k_localstypedefpair> tdmap;

    s32 localscountaliased = 0;
    if(getUlamTypeByTypedefNameInAnyLocalsScope(dataindex, tdmap) == TBOOL_TRUE)
      {
	std::map<UTI, k_localstypedefpair>::iterator it;

	for(it = tdmap.begin(); it != tdmap.end(); it++)
	  {
	    UTI ltd = it->first;
	    k_localstypedefpair ltdpair = it->second;
	    NodeBlockLocals * localsblock = ltdpair.first;
	    UTI luti = localsblock->getNodeType();
	    Symbol * tdsymptr = ltdpair.second;
	    assert(ltd == tdsymptr->getUlamTypeIdx());
	    bool isUlamGenerated = tdsymptr->isCulamGeneratedTypedef();
	    if(isHolder(ltd) && !isUlamGenerated)
	      {
		std::ostringstream msg;
		msg << "Typedef '";
		msg << m_pool.getDataAsString(dataindex).c_str();
		msg << "' in locals scope ";
		msg << getUlamTypeNameBriefByIndex(luti).c_str();
		msg << " has a class name";
		MSG2(getFullLocationAsString(tdsymptr->getLoc()).c_str(), msg.str().c_str(), DEBUG);
	      }
	    else if(isHolder(ltd) && isUlamGenerated)
	      {
		assert(isScalar(ltd));
		//yes, we found a guess in a locals filescope before class was seen. (t41509)
		// without a user-defined typedef. so was a substitute!
		//reuse its uti, fix holder key, and remove the typdedef symbol from localsfilescope.
		//assert(isAClass(ltd)); t41232,3,4, t41262,5,6,t41483,4,5
		if(cuti == Nouti)
		  {
		    isaclassholder = isAClass(ltd); //set once
		    if(!isARootUTI(ltd))
		      ltd = lookupUTIAlias(ltd); //t41516, Gah case
		    assert(isARootUTI(ltd)); //sanity, or call findRootUTIAlias instead?

		    //first one found becomes the incomplete class uti, as we prepare to parse it.
		    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE, NONARRAYSIZE, ltd); //ltd for classinstanceidx (t41509)
		    cuti = makeUlamTypeFromHolder(key, Class, ltd, UC_UNSEEN);  //setup cuti here!!
		    assert(cuti==ltd);
		    assert(okUTItoContinue(luti));
		  }
		else
		  {
		    replaceUTIKeyAndAlias(ltd, cuti);
		  }

		//remove the holder typdedef symbol from its localsfilescope.
		((SymbolTypedef *)tdsymptr)->setCulamGeneratedTypedefAliased();
		localscountaliased++;
	      }
	    else if(isClassAStub(ltd))
	      {
		if(cuti == Nouti)
		  {
		    //making a template
		    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE); //"-2" and scalar default
		    cuti = makeUlamType(key, Class, UC_UNSEEN); //**gets next unknown uti type
		  }
		//continue.. (t3860,t3872,t41130)
	      }
	    else if(isAClass(ltd) && isScalar(ltd))
	      {
		if(cuti == Nouti)
		  {
		    //a true typedef (e.g. another class, not a holder)
		    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE); //"-2" and scalar default
		    cuti = makeUlamType(key, Class, UC_UNSEEN); //**gets next unknown uti type
		  }
		//continue..(t41513)
	      }
	    else
	      {
		//error, e.g. non-class locals typedef mimics class name (t41519)
		//abortNotImplementedYet();
		//return false;
		//skip
	      }
	  }

	std::ostringstream msg;
	msg << "Aliased " << localscountaliased << " holders typedefs in locals scopes for class ";
	msg << getUlamTypeNameBriefByIndex(cuti).c_str();
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }
    return localscountaliased;
  } //cleanupAllGeneratedLocalsTypedefsOfThisClassHolder

  //ready to parse this class, check for temporary (locals typedef) UlamTypes holders until seen
  bool CompilerState::addIncompleteParseThisClassSymbolToProgramTable(const Token& cTok, SymbolClassName * & symptr)
  {
    assert(cTok.m_type == TOK_TYPE_IDENTIFIER); //3676
    u32 dataindex = cTok.m_dataindex;
    AssertBool isNotDefined = (symptr == NULL) && !alreadyDefinedSymbolClassName(dataindex, symptr);
    assert(isNotDefined);

    UTI cuti = Nouti;
    bool ltdisatypedefclassholder = false; //too late to check after, t41232
    cleanupAllGeneratedLocalsTypedefsOfThisClassHolder(dataindex, cuti, ltdisatypedefclassholder);

    if(cuti==Nouti)
      {
	UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE); //"-2" and scalar default
	cuti = makeUlamType(key, Class, UC_UNSEEN); //**gets next unknown uti type
      }

    if(!ltdisatypedefclassholder)
      {
	NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
	assert(classblock);
	classblock->setNodeLocation(cTok.m_locator); //only where first used, not defined!

	//avoid Invalid Read whenconstructing class' Symbol
	pushClassContext(cuti, classblock, classblock, false, NULL); //null blocks likely

	classblock->setNodeType(cuti); //t3895, after classblock pushed

	//symbol ownership goes to the programDefST; distinguish btn template & regular classes here:
	symptr = new SymbolClassName(cTok, cuti, classblock, *this);
	m_programDefST.addToTable(dataindex, symptr);

	popClassContext(); //restore
      }
    else //makeUlamTypeFromHolder already replaced the holder class for us.
      {
	AssertBool isDefined = alreadyDefinedSymbolClassName(dataindex, symptr);
	assert(isDefined);
      }
    return true; //compatible with alreadyDefinedSymbolClassName return
  } //addIncompleteParseThisClassSymbolToProgramTable

    //temporary UlamType which will be updated during type labeling.
  bool CompilerState::addIncompleteClassSymbolToProgramTable(const Token& cTok, SymbolClassName * & symptr)
  {
    assert(cTok.m_type == TOK_TYPE_IDENTIFIER); //3676
    u32 dataindex = cTok.m_dataindex;
    AssertBool isNotDefined = (symptr == NULL) && !alreadyDefinedSymbolClassName(dataindex, symptr);
    assert(isNotDefined);

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE); //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class, UC_UNSEEN); //**gets next unknown uti type

    NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
    assert(classblock);
    classblock->setNodeLocation(cTok.m_locator); //only where first used, not defined!

    //avoid Invalid Read whenconstructing class' Symbol
    pushClassContext(cuti, classblock, classblock, false, NULL); //null blocks likely

    classblock->setNodeType(cuti); //t3895, after classblock pushed

    //symbol ownership goes to the programDefST; distinguish btn template & regular classes here:
    symptr = new SymbolClassName(cTok, cuti, classblock, *this);
    m_programDefST.addToTable(dataindex, symptr);
    addUnseenClassId(dataindex);

    popClassContext(); //restore
    return true; //compatible with alreadyDefinedSymbolClassName return
  } //addIncompleteClassSymbolToProgramTable

  //temporary UlamType which will be updated during type labeling.
  bool CompilerState::addIncompleteTemplateClassSymbolToProgramTable(const Token& cTok, SymbolClassNameTemplate * & symptr)
  {
    assert(cTok.m_type == TOK_TYPE_IDENTIFIER);
    u32 dataindex = cTok.m_dataindex;
    AssertBool isNotDefined = ((symptr == NULL) && !alreadyDefinedSymbolClassNameTemplate(dataindex,symptr));
    assert(isNotDefined);

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE); //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class, UC_UNSEEN); //**gets next unknown uti type

    NodeBlockClass * classblock = new NodeBlockClass(NULL, *this);
    assert(classblock);
    classblock->setNodeLocation(cTok.m_locator); //only where first used, not defined!
    classblock->setNodeType(cuti); //incomplete

    //avoid Invalid Read whenconstructing class' Symbol
    pushClassContext(cuti, classblock, classblock, false, NULL); //null blocks likely

    //symbol ownership goes to the programDefST; distinguish btn template & regular classes here:
    symptr = new SymbolClassNameTemplate(cTok, cuti, classblock, *this);
    m_programDefST.addToTable(dataindex, symptr);
    addUnseenClassId(dataindex);
    symptr->setBaseClass(Hzy, 0);

    popClassContext();
    return true; //compatible with alreadyDefinedSymbolClassNameTemplate return
  } //addIncompleteTemplateClassSymbolToProgramTable

  UTI CompilerState::addStubCopyToAncestorClassTemplate(UTI stubTypeToCopy, UTI argvaluecontext, UTI argtypecontext, Locator stubloc)
  {
    //handle inheritance of stub super class, based on its template
    UTI superuti = stubTypeToCopy;
    assert((superuti != Nouti) && (superuti != Hzy));
    assert(isClassAStub(superuti));
    UlamType * superut = getUlamTypeByIndex(superuti);

    SymbolClassNameTemplate * superctsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassNameTemplateByUTI(superuti, superctsym);
    assert(isDefined);

    SymbolClass * supercsym = NULL;
    AssertBool gotSuper = alreadyDefinedSymbolClass(superuti, supercsym);
    assert(gotSuper);

    ULAMCLASSTYPE superclasstype = superut->getUlamClassType();
    assert((superclasstype != UC_UNSEEN) && (superclasstype != UC_ELEMENT)); //quark or transient

    if(!supercsym->isStub() || !supercsym->pendingClassArgumentsForClassInstance())
      return superuti; //t41222

    UlamKeyTypeSignature newstubkey(superut->getUlamTypeNameId(), UNKNOWNSIZE); //"-2" and scalar default
    UTI newstubcopyuti = makeUlamType(newstubkey, Class, superclasstype); //**gets next unknown uti type

    //arg added to avoid recursive mapping in new stub copy (t41645)
    SymbolClassName * valuecontextcnsym = NULL;
    AssertBool valcontextisDefined = alreadyDefinedSymbolClassNameByUTI(argvaluecontext, valuecontextcnsym);
    assert(valcontextisDefined);

    SymbolClass * superstubcopy = superctsym->copyAStubClassInstance(superuti, newstubcopyuti, argvaluecontext, newstubcopyuti, valuecontextcnsym->getUlamTypeIdx(), stubloc); //t3365. t41221
    assert(superstubcopy);

    return newstubcopyuti;
  } //addStubCopyToAncestorClassTemplate

  bool CompilerState::isUnseenClassId(u32 id)
  {
    if(m_unseenClasses.empty())
      return false;

    bool foundit = false;
    std::set<u32>::iterator it = m_unseenClasses.find(id);
    if(it != m_unseenClasses.end())
      {
	foundit = true;
      }
    return foundit;
  }

  bool CompilerState::addUnseenClassId(u32 id)
  {
    std::pair<std::set<u32>::iterator, bool> reti;
    reti = m_unseenClasses.insert(id);
    return reti.second; //false if already existed, i.e. not added;
  }

  void CompilerState::resetUnseenClassId(u32 id)
  {
    if(m_unseenClasses.empty())
      return;

    std::set<u32>::iterator it = m_unseenClasses.find(id);
    if(it != m_unseenClasses.end())
      {
	m_unseenClasses.erase(it);
      }
  }

  void CompilerState::resetUnseenClassId(SymbolClassName * cnsym, const Token& identTok)
  {
    assert(identTok.m_type == TOK_TYPE_IDENTIFIER); //3380
    resetUnseenClassId(identTok.m_dataindex);
    cnsym->resetUnseenClassLocation(identTok);
  } //resetUnseenClassId

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
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);

	UTI luti = localsblock->getNodeType();
	pushClassContext(luti, localsblock, localsblock, false, NULL);

	localsblock->updateLineage(0);
	localsblock->checkAndLabelType(NULL);

	popClassContext(); //restore
      }
  } //updateLineageAndFirstCheckAndLabelPassForLocals

  bool CompilerState::checkAndLabelPassForLocals()
  {
    clearGoAgain();

    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);

	UTI luti = localsblock->getNodeType();
	pushClassContext(luti, localsblock, localsblock, false, NULL);

	localsblock->checkAndLabelType(NULL);

	checkforAnyRemainingCulamGeneratedTypedefsInThisContext(luti); //returns bool, errs

	popClassContext(); //restore
      }
    //return (!goAgain() && (m_err.getErrorCount() + m_err.getWarningCount() == 0));
    return !goAgain();
  } //checkAndLabelPassForLocals

  bool CompilerState::checkforAnyRemainingCulamGeneratedTypedefsInThisContext(UTI thisarg)
  {
    bool rtnb = true;
    NodeBlockContext * cblock = getContextBlock();
    assert(cblock);
    UTI nuti = cblock->getNodeType();
    if(nuti == Nav)
      return false; //t3155,t3390

    assert((cblock->getNodeType() == thisarg) || ((cblock->getNodeType() == Hzy) && (getCompileThisIdx() == thisarg))); //t41511,13,14
    bool islocalscope = isThisLocalsFileScope();

    std::map<u32, Symbol *> tdmap;
    u32 remaining = cblock->getAllRemainingCulamGeneratedTypedefSymbolsInContext(tdmap);
    if(remaining > 0)
      {
	std::map<u32, Symbol *>::iterator it = tdmap.begin();
	while(it != tdmap.end())
	  {
	    bool aok = true;
	    bool aokwithhazykin = false;

	    Symbol * sym = it->second;
	    assert(sym);
	    assert(sym->isTypedef());
	    UTI tduti = sym->getUlamTypeIdx();
	    u32 tdid = sym->getId();
	    assert(tdid == it->first);
	    assert(sym->isCulamGeneratedTypedef());

	    if(!sym->isCulamGeneratedTypedefAliased())
	      {
		bool isaliasednow = false;
		if(!okUTItoContinue(tduti))
		  aok = false; //t41437

		if(!islocalscope && aok)
		  {
		    //not locals filescope, a class; looks through class hierarchy before locals
		    //if too many choices, provide an error;
		    std::vector<k_typedefpair> kinvec;
		    bool hazyKin = false;
		    if(alreadyDefinedSymbolByAncestorsOf(thisarg, tdid, kinvec, hazyKin) && !hazyKin)
		      {
			u32 kinvecsize = kinvec.size();
			if(kinvecsize == 1)
			  {
			    //UTI baseuti = kinvec[0].first;
			    Symbol * ksym = kinvec[0].second;
			    UTI ktd = ksym->getUlamTypeIdx();
			    if(okUTItoContinue(ktd))
			      {
				UTI ktdmapped = ktd;
				findRootUTIAlias(ktd, ktdmapped); //t3555
				assert(okUTItoContinue(ktdmapped)); //ish 20230116, not ok if Hzy
				replaceUTIKeyAndAlias(tduti, ktdmapped);
				((SymbolTypedef *)sym)->setCulamGeneratedTypedefAliased();
				isaliasednow = true;
			      }
			    else
			      aok = false; //t3742
			  }
			else
			  {
			    //check if kin typedefs compare same before error
			    std::set<UTI> kinset;
			    for(u32 i = 0; i < kinvecsize; i++)
			      {
				Symbol * ksym = kinvec[0].second;
				UTI ktd = ksym->getUlamTypeIdx();
				//assert(okUTItoContinue(ktd)); //good question, ulamexports:WallPort
				if(okUTItoContinue(ktd))
				  {
				    UTI ktdmapped = ktd;
				    findRootUTIAlias(ktd,ktdmapped);
				    kinset.insert(ktdmapped);
				  }
			      }
			    UTI kuti = Nouti;
			    std::set<UTI>::iterator it;
			    for(it = kinset.begin(); it != kinset.end(); it++)
			      {
				UTI ktd = *it;
				if(okUTItoContinue(ktd))
				  {
				    UTI ktdmapped = ktd;
				    findRootUTIAlias(ktd, ktdmapped);
				    if(kuti == Nouti)
				      {
					kuti = ktdmapped; //for comparisons
				      }
				    else if(UlamType::compare(ktdmapped,kuti,*this) == UTIC_NOTSAME) //neither hazy nor same
				      {
					aok = false;
					break;
				      }
				  }
			      }

			    if(aok)
			      {
				if((kinvecsize != 0) && (kuti == Nouti))
				  aok = false; //ulamexports:Wallport
				else
				  {
				    assert(okUTItoContinue(kuti));
				    replaceUTIKeyAndAlias(tduti, kuti);
				    ((SymbolTypedef *)sym)->setCulamGeneratedTypedefAliased();
				    isaliasednow = true;
				  }
			      }
			    else
			      {
				std::ostringstream msg;
				msg << "Ambiguous typedefs '";
				msg << m_pool.getDataAsString(tdid).c_str();
				msg << "' found in ";
				msg << kinvecsize << " ancestors; ";
				msg << "Specify from which baseclass";
				MSG2(getFullLocationAsString(sym->getLoc()).c_str(), msg.str().c_str(), ERR);
			      }
			  }
		      }
		    else if(hazyKin && !kinvec.empty())
		      {
			aok = false; //wait..only if worth waiting for (t41444,t41624)
			aokwithhazykin = true; //t41626
		      }
		  }

		if(!isaliasednow && aok)
		  {
		    //look for this locals typedef, before global class
		    Symbol * ltdsymptr = NULL;
		    Locator cloc = cblock->getNodeLocation();
		    NodeBlockLocals * localsblock = getLocalsScopeBlock(cloc);
		    if(localsblock && localsblock->isIdInScope(tdid, ltdsymptr)) //checks one scope
		      {
			assert(ltdsymptr->isTypedef());
			if(!ltdsymptr->isCulamGeneratedTypedef() || ltdsymptr->isCulamGeneratedTypedefAliased())
			  {
			    if(isHolder(tduti) && isAClass(tduti))
			      {
				removeIncompleteClassSymbolFromProgramTable(getUlamTypeNameIdByIndex(tduti)); //t41513?
			      }

			    UTI ltd = ltdsymptr->getUlamTypeIdx();
			    UTI ltdmapped = ltd;
			    findRootUTIAlias(ltd, ltdmapped);
			    replaceUTIKeyAndAlias(tduti, ltdmapped);
			    ((SymbolTypedef *)sym)->setCulamGeneratedTypedefAliased();
			    isaliasednow = true;
			  }
		      }
		  }

		// finds either Specified or unspecified base's baseclass constants,typedefs
		// (e.g. as class params/args) (t41624).
		if(!isaliasednow && (aok || aokwithhazykin))
		  {
		    SymbolClassName * cnsym = NULL;
		    if(isScalar(tduti) && alreadyDefinedSymbolClassName(tdid, cnsym))
		      {
			UTI cuti = Nouti;
			if(cnsym->isClassTemplate())
			  {
			    SymbolClass * csym = NULL;
			    if(alreadyDefinedSymbolClass(tduti, csym))
			      {
				cuti = csym->getUlamTypeIdx(); //perhaps an alias
			      }
			    else if(((SymbolClassNameTemplate *)cnsym)->findClassInstanceByUTI(tduti, csym))
			      {
				cuti = csym->getUlamTypeIdx(); //also checks for mapped uti.
			      }
			    //else
			    if((cuti == Nouti) || (cuti == tduti))
			      {
				// t3644, t3554
				std::ostringstream msg;
				msg << "Class with parameters seen with the same name: ";
				msg << m_pool.getDataAsString(tdid).c_str();
				msg << "; No shadow typedef in ";
				if(isClassATemplate(thisarg))
				  msg << "template scope, ";
				else if(isALocalsFileScope(thisarg))
				  msg << "locals scope, ";
				else
				  msg << "class scope, ";
				msg << getUlamTypeNameBriefByIndex(thisarg).c_str();
				MSG2(getFullLocationAsString(sym->getLoc()).c_str(), msg.str().c_str(), ERR);
			      }
			  }
			else
			  {
			    cuti = cnsym->getUlamTypeIdx();
			  }

			if((cuti != Nouti) && !isHolder(cuti)) //t3644 (or cuti != tduti)
			  {
			    assert(!isClassATemplate(cuti));
			    UTI calias = cuti;
			    findRootUTIAlias(cuti,calias); //was:cuti = lookupUTIAlias(cuti); //t41266
			    replaceUTIKeyAndAlias(tduti, calias); //t41230
			    ((SymbolTypedef *)sym)->setCulamGeneratedTypedefAliased();
			    isaliasednow = true;
			  }
		      }
		    else
		      {
			//(template instances need a c&l round to clear possible ug-typedef flags)
			//t41013, t3384
			std::ostringstream msg;
			msg << "Typedef '";
			msg << m_pool.getDataAsString(tdid).c_str();
			if(islocalscope)
			  msg << "' in locals scope "; //t41511,14
			else
			  msg << "' in class ";
			msg << getUlamTypeNameBriefByIndex(thisarg).c_str();
			msg << " remains undefined";
			MSG2(getFullLocationAsString(sym->getLoc()).c_str(), msg.str().c_str(), WAIT);
			if(!isUnseenClassId(tdid)) //for ulamexports..
			  addUnseenClassId(tdid);
			aok &= false;
		      }
		  }
		aok &= isaliasednow;
	      }
	    rtnb &= aok;
	    it++;
	  }
      }
    return rtnb;
  } //checkforAnyRemainingCulamGeneratedTypedefsInThisContext

  u32 CompilerState::findNameIdOfCulamGeneratedTypedefTypeInThisContext(UTI typearg)
  {
    u32 rtnid = 0; //not found == 0
    NodeBlockContext * cblock = getContextBlock();
    assert(cblock);
    UTI nuti = cblock->getNodeType();
    if(nuti == Nav)
      return 0;

    std::map<u32, Symbol *> tdmap;
    u32 remaining = cblock->getAllRemainingCulamGeneratedTypedefSymbolsInContext(tdmap);
    if(remaining > 0)
      {
	std::map<u32, Symbol *>::iterator it = tdmap.begin();
	while(it != tdmap.end())
	  {
	    Symbol * sym = it->second;
	    assert(sym);
	    assert(sym->isTypedef());
	    UTI tduti = sym->getUlamTypeIdx();
	    u32 tdid = sym->getId();
	    assert(tdid == it->first);
	    assert(sym->isCulamGeneratedTypedef());

	    if(tduti == typearg)
	      {
	        rtnid = tdid;
		break;
	      }
	    it++;
	  }
      }
    return rtnid;
  } //findNameIdOfCulamGeneratedTypedefTypeInThisContext (unused)

  bool CompilerState::checkForAnyIncompleteTemplateClassInstances(UTI thisarg) {
    assert(isClassATemplate(thisarg));

    SymbolClassNameTemplate * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassNameTemplateByUTI(thisarg, cnsym);
    assert(isDefined);

    cnsym->fixAnyIncompleteClassInstances();

    return false;
  } // checkForAnyIncompleteTemplateClassInstances

  u32 CompilerState::getClassIdBits()
  {
    //initialized to constant CLASSIDBITS
    return m_classIdBits;  //== _getLogBase2(m_state.getMaxNumberOfRegisteredUlamClasses()) + 1;
  }

  u32 CompilerState::getMaxNumberOfRegisteredUlamClasses()
  {
    u32 maxregistry = m_classIdRegistryUTI.size();
    assert(maxregistry < MAX_REGISTRY_NUMBER); //UlamClassRegistry<EC>::TABLE_SIZE
    return maxregistry;
  }

  void CompilerState::defineRegistrationNumberForUlamClasses()
  {
    //post c&l, fill in those that are missing..
    m_programDefST.defineRegistrationNumberForTableOfClasses();
    defineRegistrationNumberForLocals();
  }

  void CompilerState::defineRegistrationNumberForLocals()
  {
    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);
	localsblock->getRegistrationNumberForLocalsBlock();
      }
  }

  void CompilerState::defineClassNamesAsUserStrings()
  {
    //post c&l, 3 names per class accessible to ulam programmer (locals?)
    m_programDefST.defineClassNamesAsUserStringsForTableOfClasses();
  }

  void CompilerState::generateCodeForUlamClasses(FileManager * fm)
  {
    m_programDefST.genCodeForTableOfClasses(fm);
    generateUlamClassForLocals(fm);
    generateGlobalUserStringPool(fm); //ulam-4
  }

  void CompilerState::generateUlamClassForLocals(FileManager * fm)
  {
    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);
	UTI locuti = localsblock->getNodeType();

	//create a temporary "class" !!!
	u32 cid = getClassNameIdForUlamLocalsFilescope(locuti);
	Token cTok(TOK_TYPE_IDENTIFIER, localsblock->getNodeLocation(), cid); //t3852
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = addIncompleteClassSymbolToProgramTable(cTok, cnsym);
	assert(isDefined);

	UTI cuti = cnsym->getUlamTypeIdx();
	UlamType * cut = getUlamTypeByIndex(cuti);
	AssertBool isReplaced = replaceUlamTypeForUpdatedClassType(cut->getUlamKeyTypeSignature(), Class, UC_LOCALSFILESCOPE, false);
	assert(isReplaced);

	{
	  //assert(cuti == locuti); //not same number
	  //use pre-assigned registration number in tmp class (ulam-4)
	  u32 localrn = localsblock->getRegistrationNumberForLocalsBlock(); //modified, so..
	  cnsym->assignRegistryNumber(localrn);
	  u32 tmpclassid = cnsym->getRegistryNumber(); //..minor scope, t3852
	  assert((tmpclassid != UNINITTED_REGISTRY_NUMBER) && (tmpclassid==localrn)); //t41167
	}

	//populate with NodeConstantDefs clones (ptr to same symbol),
	// and NodeTypedef clones for gencode purposes;
	std::vector<Node*> fmLocals;
	localsblock->cloneAndAppendNode(fmLocals);


	NodeBlockClass * classblock = cnsym->getClassBlockNode();
	assert(classblock);
	assert(classblock->getLastStatementPtr() == classblock); //check this.

	std::vector<Node *>::iterator vit;
	for(vit = fmLocals.begin(); vit != fmLocals.end(); vit++)
	  {
	    Node * cenode = *vit;
	    assert(cenode);
	    classblock->appendNextNode(cenode); //tfr node ownership
	  }
	fmLocals.clear(); //done with vector of clones

	//classblock->setUserStringPoolRef(localsblock->getUserStringPoolRef()); //?

	localsblock->copyUlamTypeKeys(classblock);

	//generate a single class, .h, _Types.h, .tcc and .cpp
	cnsym->generateCodeForClassInstances(fm);

	AssertBool isGone = removeIncompleteClassSymbolFromProgramTable(cTok);
	assert(isGone);
      } //next filescope locals
  } //generateUlamClassForLocals

  void CompilerState::generateGlobalUserStringPool(FileManager * fm)
  {
    // this header
    {
      File * fp = fm->open(getFileNameForUserStringPoolHeader(WSUBDIR).c_str(), WRITE);
      if(!fp)
	{
	  std::ostringstream msg;
	  msg << "System failure: " << strerror(errno) << " to write <";
	  msg << getFileNameForUserStringPoolHeader(WSUBDIR).c_str() << ">";
	  MSG2(" ", msg.str().c_str(), ERR);
	  return;
	}

      //preamble
      m_currentIndentLevel = 0;
      genCModeForHeaderFile(fp);
      fp->write("/***********************         DO NOT EDIT        ******************************\n");
      fp->write("*\n");
      fp->write("* ");
      fp->write(getFileNameForUserStringPoolHeader().c_str());
      fp->write(" - ");
      fp->write(" header for ULAM"); GCNL;

      genCopyrightAndLicenseForUlamHeader(fp);

      //ifndef for header
      fp->write("#ifndef ");
      fp->write(USERSTRINGPOOL_FILENAME);
      fp->write("_H\n");

      fp->write("#define ");
      fp->write(USERSTRINGPOOL_FILENAME);
      fp->write("_H\n\n");

      //header includes
      fp->write("#include \"Fail.h\"");
      fp->write("\n");

      //code
      fp->write("\nnamespace MFM{\n\n");

      genCodeBuiltInFunctionGetString(fp, true);
      genCodeBuiltInFunctionGetStringLength(fp, true);

      fp->write("} //MFM\n\n");

      //endif for header
      fp->write("#endif //");
      fp->write(USERSTRINGPOOL_FILENAME);
      fp->write("_H\n");

      delete fp; //close
    }

    // "table" .cpp includes .h
    {
      File * fp = fm->open(getFileNameForUserStringPoolCPP(WSUBDIR).c_str(), WRITE);
      if(!fp)
	{
	  std::ostringstream msg;
	  msg << "System failure: " << strerror(errno) << " to write <";
	  msg << getFileNameForUserStringPoolCPP(WSUBDIR).c_str() << ">";
	  MSG2(" ", msg.str().c_str(), ERR);
	  return;
	}

      m_currentIndentLevel = 0;

      // include .h in the .cpp
      indent(fp);
      fp->write("#include \"");
      fp->write(getFileNameForUserStringPoolHeader().c_str());
      fp->write("\"");
      fp->write("\n");

      fp->write("\nnamespace MFM{\n\n");

      genCodeBuiltInFunctionGetString(fp, false);

      fp->write("} //MFM\n\n");

      delete fp; //close
    }
  } //generateGlobalUserStringPool

  void CompilerState::genCodeBuiltInFunctionGetString(File * fp, bool declOnly)
  {
    //GLOBAL (ulam-4: no longer class specific) UserStringPool
    if(declOnly)
      {
	m_currentIndentLevel++;

	indent(fp);
	fp->write("const unsigned int ");
	fp->write(getDefineNameForNumberOfUserStrings());
	fp->write(" = ");
	fp->write_decimal_unsigned(m_upool.getUserStringCount());
	fp->write(";"); GCNL;

	indent(fp);
	fp->write("const unsigned int ");
	fp->write(getDefineNameForUserStringPoolSize());
	fp->write(" = ");
	fp->write_decimal_unsigned(m_upool.getUserStringPoolSize());
	fp->write(" + 1;"); GCNL;

	indent(fp);
	fp->write("extern const unsigned char ");
	fp->write(getMangledNameForUserStringPool());
	fp->write("[");
	fp->write(getDefineNameForUserStringPoolSize());
	fp->write("];"); GCNL;
	fp->write("\n");

	indent(fp);
	fp->write("extern const unsigned int ");
	fp->write(getMangledNameForUserStringPoolIndexes());
	fp->write("[");
	fp->write(getDefineNameForNumberOfUserStrings());
	fp->write("];"); GCNL;
	fp->write("\n");

	indent(fp);
	fp->write("inline const "); //inline important so function body can be in the .h
	fp->write("unsigned char * ");
	fp->write(getGetStringFunctionName());
	fp->write("(unsigned int sidx)\n");
	indent(fp);
	fp->write("{\n");

	m_currentIndentLevel++;

	indent(fp);
	fp->write("if(sidx == 0) ");
	fp->write("FAIL(UNINITIALIZED_VALUE);"); GCNL;

	indent(fp);
	fp->write("if(sidx >= ");
	fp->write(getDefineNameForNumberOfUserStrings());
	fp->write(") ");
	fp->write("FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);"); GCNL;

	indent(fp);
	fp->write("return &");
	fp->write(getMangledNameForUserStringPool());
	fp->write("[");
	fp->write(getMangledNameForUserStringPoolIndexes());
	fp->write("[sidx] + 1];"); GCNL;

	m_currentIndentLevel--;

	indent(fp);
	fp->write("} //"); //comment..
	fp->write(getGetStringFunctionName());
	fp->write("\n\n");

	m_currentIndentLevel--;
	return;
      }

    //inside the .cpp
    //the user string pool table definition
    m_upool.generateUserStringPoolEntries(fp, this);

    //inside the .cpp
    //the user string pool valid string indexes
    m_upool.generateUserStringPoolIndexes(fp, this);

  } //genCodeBuiltInFunctionGetString

  void CompilerState::genCodeBuiltInFunctionGetStringLength(File * fp, bool declOnly)
  {
    //GLOBAL (ulam-4: no longer class specific) UserStringPool
    if(declOnly)
      {
	m_currentIndentLevel++;

	indent(fp);
	fp->write("inline unsigned int ");
	fp->write(getGetStringLengthFunctionName());
	fp->write("(unsigned int sidx)"); GCNL;
	indent(fp);
	fp->write("{\n");

	m_currentIndentLevel++;

	indent(fp);
	fp->write("if(sidx == 0) ");
	fp->write("FAIL(UNINITIALIZED_VALUE);"); GCNL; // fail/t3934

	indent(fp);
	fp->write("if(sidx >= ");
	fp->write(getDefineNameForNumberOfUserStrings());
	fp->write(") ");
	fp->write("FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);"); GCNL;

	indent(fp);
	fp->write("return ");
	fp->write(getMangledNameForUserStringPool());
	fp->write("[");
	fp->write(getMangledNameForUserStringPoolIndexes());
	fp->write("[sidx]];"); GCNL;

	m_currentIndentLevel--;
	indent(fp);
	fp->write("} //"); //commment..
	fp->write(getGetStringLengthFunctionName());
	fp->write("\n\n");
	m_currentIndentLevel--;
	return;
      }
  } //genCodeBuiltInFunctionGetStringLength

  bool CompilerState::countNavHzyNoutiNodesPass()
  {
    bool rtnb = true;
    u32 navcount = 0;
    u32 hzycount = 0;
    u32 unsetcount = 0;
    clearGoAgain(); //missing

    m_programDefST.countNavNodesAcrossTableOfClasses(navcount, hzycount, unsetcount);
    countNavNodesForLocals(navcount, hzycount, unsetcount);

    u32 errCnt = m_err.getErrorCount(); //latest count
    if(navcount > 0)
      {
	// not necessarily goAgain, e.g. atom is Empty, where Empty is a quark instead of an element
	// the NodeTypeDescriptor is perfectly fine with a complete quark type, so no need to go again;
	// however, in the context of "is", this is an error and t.f. a Nav node.

	assert(errCnt > 0); //sanity check; ran out of iterations

	std::ostringstream msg;
	msg << navcount << " Nodes with erroneous types detected after type labeling";
	MSG2("", msg.str().c_str(), INFO);
	rtnb = false;
      }
    //else
    //assert(errCnt == 0); //e.g. error/t3644 (not sure what to do about it, error discovery too deep)

    if(hzycount > 0)
      {
	//doesn't include incomplete stubs: if(a is S(x,y))
	//assert(goAgain()); //sanity check; ran out of iterations

	std::ostringstream msg;
	msg << hzycount << " Nodes with unresolved types detected after type labeling";
	// if we had such a thing:
	//msg << ". Supplying --info on command line will provide additional internal details";
	MSG2("", msg.str().c_str(), INFO);
	rtnb = false;
      }
    else
      assert(!goAgain()); //t3740 (resets Hzy to Nav);

    if(unsetcount > 0)
      {
	std::ostringstream msg;
	msg << unsetcount << " Nodes with unset types detected after type labeling class: ";
	MSG2("", msg.str().c_str(), INFO);
      }
    return rtnb;
  } //countNavHzyNoutiNodesPass

  void CompilerState::countNavNodesForLocals(u32& navcount, u32& hzycount, u32& unsetcount)
  {
    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);

	UTI luti = localsblock->getNodeType();
	pushClassContext(luti, localsblock, localsblock, false, NULL);

	localsblock->countNavHzyNoutiNodes(navcount, hzycount, unsetcount);

	popClassContext(); //restore
      }
    return;
  } //countNavNodesForLocals

  bool CompilerState::alreadyDefinedSymbol(u32 dataindex, Symbol *& symptr, bool& hasHazyKin)
  {
    // also searches ancestors, if necessary; like callers expected prior to ulam-5.
    bool tmphazykin = false;

    bool found = alreadyDefinedSymbolHere(dataindex, symptr, tmphazykin);
    if(found)
      hasHazyKin = tmphazykin;
    else
      {
	bool tmphazys = false;
	UTI cuti = getCompileThisIdx();

	if(useMemberBlock()) //(t3555 and ancestors; t3126 within funcdef)
	  {
	    NodeBlockClass* memberblock = getCurrentMemberClassBlock();
	    if(memberblock) //could be null during parsing e.g. parseMemberSelectExpr->parseIdentExpr
	      cuti = memberblock->getNodeType();
	    else
	      cuti = Hzy;
	  }
	if(!okUTItoContinue(cuti))
	  {
	    hasHazyKin = true;
	    return false; //wait
	  }
	//assert(okUTItoContinue(cuti));

	found = alreadyDefinedSymbolByAncestorOf(cuti, dataindex, symptr, tmphazys);
	hasHazyKin = tmphazykin || tmphazys;
      }

    //search current class's local file scope only (not ancestors' unless in same file)
    //if(!found) t41700, t41266, t41245,6,7
    if(!found && (!useMemberBlock() || (getContextBlockNo() == getCurrentMemberClassBlock()->getNodeNo())))
      found = isIdInLocalFileScope(dataindex, symptr); //local constant or typedef

    return found;
  } //alreadyDefinedSymbol

  bool CompilerState::alreadyDefinedSymbolByAncestorOf(UTI cuti, u32 dataindex, Symbol *& symptr, bool& hasHazyKin)
  {
    BaseclassWalker walker;

    //recursively check ancestors (default depth/breadth-first) for first defined name
    //(and not a Holder? t41298,9); see next method below for complete set;
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      walker.addAncestorsOf(csym);

    bool kinhadit = false;
    UTI baseuti = Nouti;
    while(!kinhadit && walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    pushClassContext(baseuti, basecblock, basecblock, false, NULL);
	    bool tmphzykin = false;
	    kinhadit = alreadyDefinedSymbolHere(dataindex, symptr, tmphzykin);

	    popClassContext(); //restore

	    if(!kinhadit)
	      walker.addAncestorsOf(basecsym);
	    else
	      hasHazyKin = tmphzykin;
	  }
	else if(baseuti == Hzy)
	  {
	    hasHazyKin = true; //like t3641
	  }
	//else
      } //end while
    return kinhadit;
  } //alreadyDefinedSymbolByAncestorOf

  bool CompilerState::alreadyDefinedSymbolByAncestorsOf(UTI cuti, u32 dataindex, std::set<UTI>& kinsetref, bool& hasHazyKin)
  {
    BaseclassWalker walker;

    // return complete set of base class UTIs that share the symbol name id (error/t41331).
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      walker.addAncestorsOf(csym);

    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    pushClassContext(baseuti, basecblock, basecblock, false, NULL);
	    Symbol * symptr = NULL;
	    bool tmphzykin = false;
	    bool kinhadit = alreadyDefinedSymbolHere(dataindex, symptr, tmphzykin);

	    popClassContext(); //restore

	    if(kinhadit)
	      {
		kinsetref.insert(baseuti);
		hasHazyKin |= tmphzykin;
	      }
	    //all
	    walker.addAncestorsOf(basecsym);
	  }
	else if(baseuti == Hzy)
	  {
	    hasHazyKin = true; //like t3641
	  }
	//else
      } //end while
    return !kinsetref.empty();
  } //alreadyDefinedSymbolByAncestorOf

  bool CompilerState::alreadyDefinedSymbolByAncestorsOf(UTI cuti, u32 dataindex, std::vector<k_typedefpair>& kinvecref, bool& hasHazyKin)
  {
    BaseclassWalker walker;

    // return complete set of base class UTIs that share the symbol name id (error/t41331).
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      walker.addAncestorsOf(csym);

    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    pushClassContext(baseuti, basecblock, basecblock, false, NULL);
	    Symbol * symptr = NULL;
	    bool tmphzykin = false;
	    bool kinhadit = alreadyDefinedSymbolHere(dataindex, symptr, tmphzykin);

	    popClassContext(); //restore

	    if(kinhadit)
	      {
		k_typedefpair kpair(baseuti,symptr);
		kinvecref.push_back(kpair);
		hasHazyKin |= tmphzykin;
	      }
	    //all
	    walker.addAncestorsOf(basecsym);
	  }
	else if((baseuti == Hzy))
	  {
	    hasHazyKin = true; //like t3641
	  }
	//else
      } //end while
    return !kinvecref.empty();
  } //alreadyDefinedSymbolByAncestorOf

  bool CompilerState::alreadyDefinedSymbolByAClassOrAncestor(UTI cuti, u32 dataindex, Symbol *& symptr, bool& hasHazyKin)
  {
    BaseclassWalker walker;
    walker.init(cuti);

    //recursively check class and ancestors (default depth/breadth-first),
    //for defined name (and not a Holder?)
    bool kinhadit = false;
    UTI baseuti = Nouti;
    while(!kinhadit && walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    pushClassContext(baseuti, basecblock, basecblock, false, NULL);
	    bool tmphzykin = false;
	    kinhadit = alreadyDefinedSymbolHere(dataindex, symptr, tmphzykin);

	    popClassContext(); //restore

	    if(!kinhadit)
	      walker.addAncestorsOf(basecsym);
	    else
	      hasHazyKin = tmphzykin;
	  }
	else if(baseuti == Hzy)
	  {
	    hasHazyKin = true; //like t3641
	  }
	//else
      } //end while
    return kinhadit;
  } //alreadyDefinedSymbolByAClassOrAncestor

  bool CompilerState::alreadyDefinedSymbolHere(u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    bool brtn = false;
    bool isType = isPossibleTypeName(dataindex); //t41488

    assert(!hasHazyKin);

    //start with the current "top" block and look down the stack
    //until the 'variable id' is found. may be member block.
    NodeBlock * blockNode = getCurrentBlockForSearching();

    while(!brtn && blockNode)
      {
	brtn = blockNode->isIdInScope(dataindex,symptr); //check ST
	if(!isType)
	  brtn = brtn && (!m_initSubtreeSymbolsWithConstantsOnly || symptr->isConstant());

	//hazy check..
	hasHazyKin |= checkHasHazyKin(blockNode);
	//traverse the chain, including templates (not ancestors)
	blockNode = blockNode->getPreviousBlockPointer();
      }

    //data member variables in class block; function symbols are linked to their
    //function def block; check function data members separately.
    //no longer checks local filescope
    if(!brtn)
      brtn = isDataMemberIdInClassScope(dataindex, symptr, hasHazyKin);

    if(!brtn)
      brtn = isFuncIdInClassScope(dataindex, symptr, hasHazyKin);
    return brtn;
  } //alreadyDefinedSymbolHere

  bool CompilerState::isDataMemberIdInClassScope(u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    bool brtn = false;
    bool isType = isPossibleTypeName(dataindex);
    //might come from alreadyDefinedSymbol now, and have a hazy chain.

    //start with the current class block, until the 'variable id' is found.
    //substitutes another selected class block to search for data member
    NodeBlockContext * cblock = getContextBlockForSearching();

    while(!brtn && cblock)
      {
	brtn = cblock->isIdInScope(dataindex,symptr); //returns symbol
	if(!isType)
	  brtn = brtn && (!m_initSubtreeSymbolsWithConstantsOnly || symptr->isConstant());

	hasHazyKin |= checkHasHazyKin(cblock); //self is stub
	//traverse the chain, including templates
	//(not ancestors; see alreadyDefinedSymbolByAncestorOf)
	cblock = (NodeBlockContext *) (cblock->getPreviousBlockPointer());
      }

    return brtn;
  } //isDataMemberIdInClassScope

  bool CompilerState::isIdInLocalFileScope(u32 id, Symbol *& symptr)
  {
    bool brtn = false;

    //start with the current class block; doesn't look up family tree
    //until the 'variable id' is found. returns false if not found.
    //substitute another selected class block to search for data member
    NodeBlockContext * cblock = getContextBlockForSearching();

    Locator cloc;
    if(cblock)
      cloc = cblock->getNodeLocation(); //to check local scope

    NodeBlockLocals * localsblock = getLocalsScopeBlock(cloc);
    if(localsblock)
      brtn = localsblock->isIdInScope(id, symptr); //checks one scope only!

    return brtn;
  } //isIdInLocalFileScope

  bool CompilerState::isFuncIdInClassScope(u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    bool brtn = false;
    //might come from alreadyDefinedSymbol now, and have a hazy chain.

    //start with current class block, not family tree (see isFuncIdInAClassScopeOrAncestor)
    //until the 'variable id' is found.
    //substitute another selected class block to search for data member
    NodeBlockContext * cblock = getContextBlockForSearching();

    if(cblock && !cblock->isAClassBlock())
      return false;

    while(!brtn && cblock)
      {
	brtn = ((NodeBlockClass *) cblock)->isFuncIdInScope(dataindex,symptr); //returns symbol
	hasHazyKin |= checkHasHazyKin(cblock); //self is stub
	cblock = (NodeBlockContext *) (cblock->getPreviousBlockPointer()); //traverse any chain
      }
    return brtn;
  } //isFuncIdInClassScope

  bool CompilerState::isFuncIdInClassScopeNNO(NNO cnno, u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    UTI cuti = findAClassByNodeNo(cnno); //class of cnno
    assert(okUTItoContinue(cuti));
    assert(!hasHazyKin);
    return isFuncIdInAClassScopeOrAncestor(cuti, dataindex, symptr, hasHazyKin);
  } //isFuncIdInClassScopeNNO

  bool CompilerState::isFuncIdInAClassScopeOrAncestor(UTI cuti, u32 dataindex, Symbol * & symptr, bool& hasHazyKin)
  {
    bool rtnb = false;

    BaseclassWalker walker; //ambiguous non-virtual funcs must be called specifically
    walker.init(cuti);

    UTI baseuti = Nouti;
    while(!rtnb && walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    assert(basecblock);
	    pushClassContextUsingMemberClassBlock(basecblock);

	    bool tmphazykin = false;
	    Symbol * fnSym = NULL;
	    if((rtnb = isFuncIdInClassScope(dataindex, fnSym, tmphazykin))) //not ancestor
	      {
		hasHazyKin = tmphazykin;
		symptr = fnSym;
	      }
	    else
	      {
		walker.addAncestorsOf(basecsym);
		hasHazyKin |= tmphazykin; //t3641
	      }
	    popClassContext(); //didn't forget!!
	  }
	else if(baseuti == Hzy)
	  {
	    hasHazyKin = true; //t3641
	  }
	//else
      } //end while
    return rtnb;
  } //isFuncIdInAClassScopeOrAncestor

  u32 CompilerState::getNextFunctionOrderNumber()
  {
    assert(m_nextFunctionOrderNumber > 0); //no more than U32_MAX please.
    return m_nextFunctionOrderNumber++;
  }

  bool CompilerState::findMatchingFunctionStrictlyByTypesInClassScope(u32 fid, std::vector<UTI> typeVec, SymbolFunction*& fsymref)
  {
    bool rtnb = false;

    //start with the current class block, not hierarchy
    //substitute another selected class block to search for function
    NodeBlockContext * cblock = getContextBlockForSearching();
    assert(cblock);

    Symbol * fnSym = NULL;
    if(((NodeBlockClass *) cblock)->isFuncIdInScope(fid, fnSym)) //dont check ancestor
      {
	bool tmphazyargs = false;
	rtnb = (((SymbolFunctionName *) fnSym)->findMatchingFunctionStrictlyByTypes(typeVec, fsymref, tmphazyargs) == 1); //exact
	assert(!tmphazyargs);
      }

    return rtnb;
  } //findMatchingFunctionStrictlyByTypesInClassScope

  bool CompilerState::findOverrideMatchingVirtualFunctionStrictlyByTypesInAncestorOf(UTI cuti, u32 fid, std::vector<UTI> typeVec, bool virtualInSub, SymbolFunction*& fsymref, UTI& foundInAncestor)
  {
    bool rtnb = false;
    UTI foundinbase = Nouti;

    BaseclassWalker walker(true); //breadth-first please (t41325), ow must search all

    //called again while initializing vtable, looking for overrides in subclasses;
    //don't look in cuti (yet), just base classes (uses breadth-first)
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    walker.addAncestorsOf(csym);

    UTI baseuti = Nouti;
    while(!rtnb && walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    assert(basecblock);
	    pushClassContextUsingMemberClassBlock(basecblock);

	    SymbolFunction * tmpfsym = NULL; //repeated use
	    bool gotmatch = findMatchingFunctionStrictlyByTypesInClassScope(fid, typeVec, tmpfsym);
	    popClassContext(); //didn't forget!!

	    if(gotmatch)
	      {
		if(!tmpfsym->isVirtualFunction())
		  {
		    if(virtualInSub)
		      {
			//c++, quietly supports it (t3746, t41160).
			std::ostringstream msg;
			msg << "Virtual overloaded function '";
			msg << m_pool.getDataAsString(fid).c_str();
			msg << "' has a NON-VIRTUAL ancestor in class: ";
			msg << getUlamTypeNameBriefByIndex(baseuti).c_str();
			msg << " while compiling ";
			msg << getUlamTypeNameBriefByIndex(cuti).c_str();
			MSG2(tmpfsym->getTokPtr(), msg.str().c_str(), WARN);
		      }
		    //else
		  }
		else
		  {
		    if(foundinbase == Nouti)
		      {
			foundinbase = baseuti;
			fsymref = tmpfsym;
			//rtnb = true; //(t41325,19) stop? since breadth-first search;
			//can't stop (t41394)
		      }
		    else if(isClassASubclassOf(baseuti, foundinbase))
		      {
			//baseuti is a subclass of foundinbase,
			// hence baseuti is "more specific" (t41394)
			foundinbase = baseuti;
			fsymref = tmpfsym;
		      }
		    else if(!isClassASubclassOf(foundinbase, baseuti))
		      {
			//siblings, keep "most specific", i.e. "first-seen" (breadth) t41391
		      }
		  }
	      } //gotmatch
	    walker.addAncestorsOf(basecsym); //chk all bases until found (t3602))
	  }
      } //end while

    foundInAncestor = foundinbase;
    rtnb = fsymref && okUTItoContinue(foundInAncestor); //neither Nav, nor Nouti
    return rtnb;
  } //findOverrideMatchingVirtualFunctionStrictlyByTypesInAncestorOf

  bool CompilerState::findOriginatingMatchingVirtualFunctionStrictlyByTypesInAncestorOf(UTI cuti, u32 fid, std::vector<UTI> typeVec, SymbolFunction*& origfsymref, UTI& firstInAncestor)
  {
    bool rtnb = false;
    UTI foundOriginator = Nouti;
    bool warns = false;

    BaseclassWalker walker(false); //depth-first, please (t41312)

    //don't look in cuti, just base classes (depth-first, all for error check)
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    walker.addAncestorsOf(csym);

    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this) && (foundOriginator != Nav))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    assert(basecblock);
	    pushClassContextUsingMemberClassBlock(basecblock);

	    SymbolFunction * tmpfsym = NULL; //repeated use (t3562, t3608, t3748)
	    bool gotmatch = findMatchingFunctionStrictlyByTypesInClassScope(fid, typeVec, tmpfsym);
	    popClassContext(); //didn't forget!!

	    if(gotmatch)
	      {
		if(!tmpfsym->isVirtualFunction())
		  {
		    //c++, quietly supports it (t3746).
		    std::ostringstream msg;
		    msg << "Virtual overloaded function '";
		    msg << m_pool.getDataAsString(fid).c_str();
		    msg << "' has a NON-VIRTUAL ancestor in class: ";
		    msg << getUlamTypeNameBriefByIndex(baseuti).c_str();
		    msg << " while compiling ";
		    msg << getUlamTypeNameBriefByIndex(cuti).c_str();
		    MSG2(tmpfsym->getTokPtr(), msg.str().c_str(), WARN);
		    warns = true; //t3746
		  }
		else
		  {
		    //check for earlier first definition (vown)
		    // for vowned classes, func must be virtual (t3746)
		    if(foundOriginator == Nouti)
		      {
			foundOriginator = baseuti;
			origfsymref = tmpfsym;
		      }
		    else if(UlamType::compare(foundOriginator, baseuti, *this) != UTIC_NOTSAME)
		      {
			//noop (t41385)
		      }
		    else if(isClassASubclassOf(foundOriginator, baseuti))
		      {
			//foundOriginator is a subclass of baseuti (e.g. UrSelf),
			//hence baseuti is more Original
			foundOriginator = baseuti;
			origfsymref = tmpfsym; //switch ok (e.g. 3600)
		      }
		    else if(!isClassASubclassOf(baseuti, foundOriginator))
		      {
			//unrelated if neither is a subclass of the other..
			std::ostringstream msg;
			if(tmpfsym->isVirtualFunction())
			  msg << "Virtual ";
			msg << "Function: "  << m_pool.getDataAsString(tmpfsym->getFunctionNameId()).c_str();
			msg << "(";
			for (u32 i = 0; i < typeVec.size(); i++)
			  {
			    if(i > 0)
			      msg << ", ";
			    msg << getUlamTypeNameBriefByIndex(typeVec[i]).c_str();
			  }
			msg << ") has conflicting Originating declarations in multiple base classes: ";
			msg << getUlamTypeNameBriefByIndex(baseuti).c_str();
			msg << " and ";
			msg << getUlamTypeNameBriefByIndex(foundOriginator).c_str();
			msg << ", while compiling ";
			msg << getUlamTypeNameBriefByIndex(cuti).c_str();
			MSG2(tmpfsym->getTokPtr(), msg.str().c_str(), ERR);
			origfsymref = NULL; //t41312
		      }
		  }
	      } //gotmatch
	    walker.addAncestorsOf(basecsym); // check all bases for originator, and errors
	  }
      } //end while

    rtnb = origfsymref && okUTItoContinue(foundOriginator); //neither Nav, nor Nouti
    firstInAncestor = foundOriginator;

    if(warns)
      {
	std::ostringstream msg;
	msg << "Virtual overloaded function '";
	msg << m_pool.getDataAsString(fid).c_str();
	msg << "' Originating class is: ";
	msg << getUlamTypeNameBriefByIndex(firstInAncestor).c_str();
	msg << " while compiling ";
	msg << getUlamTypeNameBriefByIndex(cuti).c_str();
	MSG2(csym->getTokPtr(), msg.str().c_str(), WARN);
      }
    return rtnb;
  } //findOriginatingMatchingVirtualFunctionStrictlyByTypesInAncestorOf

  u32 CompilerState::findMatchingFunctionInClassScope(u32 fid, std::vector<Node *> nodeArgs, SymbolFunction*& fsymref, bool& hasHazyArgs)
  {
    u32 matches = 0;

    //start with the current class block, not hierarchy
    //substitute another selected class block to search for function
    NodeBlockContext * cblock = getContextBlockForSearching();
    assert(cblock);

    Symbol * fnSym = NULL;
    if(((NodeBlockClass *) cblock)->isFuncIdInScope(fid, fnSym)) //dont check ancestor
      {
	matches = ((SymbolFunctionName *) fnSym)->findMatchingFunction(nodeArgs, fsymref, hasHazyArgs); //exact
	if(hasHazyArgs)
	  matches = 0;
      }

    return matches;
  } //findMatchingFunctionInClassScope

  bool CompilerState::findMatchingFunctionInAClassScopeOrAncestor(UTI cuti, u32 fid, std::vector<Node *> nodeArgs, SymbolFunction*& fsymref, bool& hasHazyArgs,  UTI& foundInAncestor)
  {
    //uses argument nodes, not just vector of arg types
    bool exactlyone = false; //true if exact match found
    u32 matchingFuncCount = 0; //U32_MAX;

    BaseclassWalker walker; //default ok for functions
    walker.init(cuti);

    //can't assume class context already pushed

    //Like in C++, exact matches in a subclass overrides any possible exact matches
    //in base classes; ow, use baseclass w exact match, assuming no ambiguity among others.
    UTI baseuti = Nouti;
    while(!exactlyone && walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    assert(basecblock);
	    pushClassContextUsingMemberClassBlock(basecblock);

	    bool tmphazyargs = false;
	    SymbolFunction * tmpfsym = NULL; //e.g. t3357
	    u32 matches = findMatchingFunctionInClassScope(fid, nodeArgs, tmpfsym, tmphazyargs); //exact
	    matchingFuncCount += matches;
	    hasHazyArgs |= tmphazyargs; //t3484

	    if(matches == 1)
	      {
		foundInAncestor = baseuti;
		fsymref = tmpfsym;
		//could be ambiguous amongst the baseclasses (t3600)
		exactlyone = walker.isDone();
		//assert(!hasHazyArgs); t3395
	      }

	    popClassContext(); //didn't forget!!

	    if(!exactlyone) //not found
	      walker.addAncestorsOf(basecsym);
	  }
	else if(baseuti == Hzy)
	  {
	    hasHazyArgs = true; //like t3641
	  }
      } //end while

    if(matchingFuncCount == 0)
      foundInAncestor = Nouti; //none found
    else if(matchingFuncCount > 1)
      {
	fsymref = NULL;
	foundInAncestor = Nav; //more than one, ambiguous
	exactlyone = false;
      }
    else //exactly one exact match found in a base class
      {
	assert(matchingFuncCount == 1);
	exactlyone = true;
      }

    return exactlyone;
  } //findMatchingFunctionInAClassScopeOrAncestor

  u32 CompilerState::findMatchingFunctionWithSafeCastsInClassScope(u32 fid, std::vector<Node *> nodeArgs, SymbolFunction*& fsymref, bool& hasHazyArgs, FSTable & fstable)
  {
    u32 safematches = 0;

    //start with the current class block, not hierarchy
    //substitute another selected class block to search for function
    NodeBlockContext * cblock = getContextBlockForSearching();
    assert(cblock);

    Symbol * fnSym = NULL;
    if(((NodeBlockClass *) cblock)->isFuncIdInScope(fid, fnSym)) //dont check ancestor
      {
	safematches = ((SymbolFunctionName *) fnSym)->findMatchingFunctionWithSafeCasts(nodeArgs, fsymref, hasHazyArgs, fstable);
      }

    return safematches;
  } //findMatchingFunctionWithSafeCastsInClassScope

  u32 CompilerState::findMatchingFunctionWithSafeCastsInAClassScopeOrAncestor(UTI cuti, u32 fid, std::vector<Node *> argNodes, SymbolFunction*& fsymref, bool& hasHazyArgs, UTI& foundInAncestor)
  {
    //traverse hierarchy for exact match
    if(findMatchingFunctionInAClassScopeOrAncestor(cuti, fid, argNodes, fsymref, hasHazyArgs, foundInAncestor))
      return 1;

    bool rtnb = false;
    UTI foundinbase = Nouti;
    u32 matchingFuncCount = 0;

    FSTable FST; //starts here!!

    BaseclassWalker walker; //default okay for functions
    walker.init(cuti);

    UTI baseuti = Nouti;
    while(!rtnb && walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    assert(basecblock);
	    pushClassContextUsingMemberClassBlock(basecblock);

	    bool tmphazyargs = false;
	    SymbolFunction * tmpfsym = NULL; //e.g. t3357
	    u32 safematches = findMatchingFunctionWithSafeCastsInClassScope(fid, argNodes, tmpfsym, tmphazyargs, FST);

	    hasHazyArgs |= tmphazyargs; //(like t3483)

	    if(safematches == 1) //found one, and only one, here first!!
	      {
		if(foundinbase == Nouti)
		  {
		    foundinbase = baseuti;
		    fsymref = tmpfsym;
		    rtnb = walker.isDone(); //only one level inheritance, let's go with that..
		    //assert(!hasHazyArgs); //t3484
		    matchingFuncCount = safematches;
		  }
		else if(isClassASubclassOf(baseuti, foundinbase))
		  {
		    foundinbase = baseuti; //baseuti is subclass of foundinbase (e.g. UrSelf), switch
		    fsymref = tmpfsym;
		    rtnb = walker.isDone(); //only one level inheritance, let's go with that..
		    matchingFuncCount = safematches;
		  }
		else if(!isClassASubclassOf(foundinbase, baseuti))
		      {
			std::ostringstream msg;
			if(tmpfsym->isVirtualFunction()) //t41329 non-virtual func;
			  msg << "Virtual "; //t41305, t41437
			msg << "Function: "  << m_pool.getDataAsString(tmpfsym->getFunctionNameId()).c_str();
			msg << "(";
			for (u32 i = 0; i < argNodes.size(); i++)
			  {
			    if(i > 0)
			      msg << ", ";
			    msg << getUlamTypeNameBriefByIndex(argNodes[i]->getNodeType()).c_str();
			  }
			msg << ") has conflicting declarations in multiple base classes: ";
			msg << getUlamTypeNameBriefByIndex(foundinbase).c_str();
			msg << " and ";
			msg << getUlamTypeNameBriefByIndex(baseuti).c_str();
			msg << ", while compiling ";
			msg << getUlamTypeNameBriefByIndex(cuti).c_str();
			MSG2(tmpfsym->getTokPtr(), msg.str().c_str(), WARN); //was WARN
			foundinbase = Nav; //WARNING
			fsymref = NULL;
			matchingFuncCount += safematches;
		      }
		//else skip this one, related and less specific.
	      }
	    else
	      {
		matchingFuncCount += safematches; //err tests: t3479, t41132, t41305
		walker.addAncestorsOf(basecsym); //keep looking deeper
	      }

	    popClassContext(); //didn't forget!!
	  }
	else if(baseuti == Hzy)
	  {
	    hasHazyArgs = true; //like t3641
	  }
	//else
      } //end while

    if(matchingFuncCount == 0) //U32_MAX)
      foundInAncestor = Nouti; //none found
    else if(matchingFuncCount > 1)
      {
	rtnb = false;
	foundInAncestor = Nav; //more than one, ambiguous
      }
    else
      {
	assert(matchingFuncCount == 1);
	rtnb = true;
	foundInAncestor = foundinbase;
      }

      FST.clear();
      return matchingFuncCount; //rtnb;
  } //findMatchingFunctionWithSafeCastsInAClassScopeOrAncestor

  void CompilerState::noteAmbiguousFunctionSignaturesInAClassHierarchy(UTI cuti, u32 fid, std::vector<Node *> argNodes, u32 matchingFuncCount)
  {
    std::map<UTI,u32> classsafematchmap; //uti->safematches of most-specific baseclasses

    BaseclassWalker walker;
    walker.init(cuti);

    //first while, build map..
    u32 count = 0;
    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    NodeBlockClass * basecblock = basecsym->getClassBlockNode();
	    assert(basecblock);
	    pushClassContextUsingMemberClassBlock(basecblock);

	    u32 safematches = 0;
	    Symbol * fnSym = NULL;
	    if(basecblock->isFuncIdInScope(fid, fnSym))
	      {
		bool skipthisbase = false;
		std::map<UTI,u32>::iterator it;

		for(it = classsafematchmap.begin(); it != classsafematchmap.end(); it++)
		  {
		    UTI cit = it->first;
		    if(isClassASubclassOf(cit, baseuti))
		      {
			skipthisbase = true; //already have subclass of this baseuti, t41587
			break;
		      }
		    else if(isClassASubclassOf(baseuti, cit))
		      {
			count -= it->second;
			classsafematchmap.erase(it); //switch to more specific baseuti, t41588
			break;
		      }
		    //else
		  } //end checking classes in set

		if(!skipthisbase)
		  {
		    safematches = ((SymbolFunctionName *) fnSym)->noteAmbiguousFunctionSignatures(argNodes, count, matchingFuncCount, false); //no note this time
		    count += safematches;
		    classsafematchmap.insert(std::pair<UTI,u32>(baseuti, safematches)); //update map
		  }
		//else skip this one, related and less specific.
	      }

	    popClassContext(); //didn't forget!!

	    if(safematches != 1) //comparing with the "safe" matches found (t41437)
	      walker.addAncestorsOf(basecsym); // check them all..
	  }
      } //end while
    assert(count == matchingFuncCount); //sanity (t41587 and ish 20211115 insane!)

    //second time, output notes in map:
    u32 count2 = 0;
    std::map<UTI,u32>::iterator it;
    for(it = classsafematchmap.begin(); it != classsafematchmap.end(); it++)
      {
	UTI cit = it->first;
	SymbolClass * csym = NULL;
	AssertBool gotclass = alreadyDefinedSymbolClass(cit, csym);
	assert(gotclass);

	NodeBlockClass * cblock = csym->getClassBlockNode();
	assert(cblock);
	pushClassContextUsingMemberClassBlock(cblock);

	u32 safematches = 0;
	Symbol * fnSym = NULL;
	cblock->isFuncIdInScope(fid, fnSym);
	assert(fnSym);

	safematches = ((SymbolFunctionName *) fnSym)->noteAmbiguousFunctionSignatures(argNodes, count2, matchingFuncCount, true); //note this time
	assert(safematches == it->second); //sanity t3479,t41132
	count2 += safematches;

	popClassContext(); //didn't forget!!
      }
    assert(count2 == matchingFuncCount); //sanity (t41587,8)
    return;
  } //noteAmbiguousFunctionSignaturesInClassHierarchy

  bool CompilerState::isIdInCurrentScope(u32 id, Symbol *& asymptr)
  {
    //substitute member selected class block to search for data member
    NodeBlock * blockNode = getCurrentBlockForSearching();

    //also applies when isParsingLocalDef()
    return blockNode && blockNode->isIdInScope(id, asymptr);
  } //isIdInCurrentScope

  //symbol ownership goes to the current block (end of vector)
  void CompilerState::addSymbolToCurrentScope(Symbol * symptr)
  {
    assert(!useMemberBlock());
    //also applies when isParsingLocalDef()
    NodeBlock * currBlock = getCurrentBlock();
    assert(currBlock);
    symptr->setBlockNoOfST(currBlock->getNodeNo()); //invariant
    currBlock->addIdToScope(symptr->getId(), symptr);
  }

  //symbol ownership goes to current context block (end of vector)
  void CompilerState::addSymbolToCurrentContextScope(Symbol * symptr)
  {
    assert(!useMemberBlock());
    NodeBlockContext * cBlock = getContextBlock(); //class or locals
    assert(cBlock);
    symptr->setBlockNoOfST(cBlock->getNodeNo()); //invariant
    cBlock->addIdToScope(symptr->getId(), symptr);
  }

  void CompilerState::addSymbolToLocalsScope(Symbol * symptr, Locator loc)
  {
    assert(symptr);
    Token fTok;
    getFirstTokenForParsing(fTok); //supercedes loc arg
    NodeBlockLocals * localsblock = makeLocalsScopeBlock(fTok.m_locator); //getLocalsScopeLocator
    assert(localsblock);

    symptr->setBlockNoOfST(localsblock->getNodeNo()); //invariant
    localsblock->addIdToScope(symptr->getId(), symptr);
  } //addSymbolToLocalsScope

  //symbol ownership goes to the member block (end of vector)
  // making stuff up!
  void CompilerState::addSymbolToCurrentMemberClassScope(Symbol * symptr)
  {
    NodeBlockClass * memberblock = getCurrentMemberClassBlock();
    assert(memberblock);
    symptr->setBlockNoOfST(memberblock->getNodeNo()); //invariant
    memberblock->addIdToScope(symptr->getId(), symptr);
  }

  //symbol ownership goes to the current block (end of vector);
  //symbol is same, just id changed
  void CompilerState::replaceSymbolInCurrentScope(u32 oldid, Symbol * symptr)
  {
    assert(!useMemberBlock());
    getCurrentBlock()->replaceIdInScope(oldid, symptr->getId(), symptr);
  }

  //deletes the oldsym, id's must be identical
  void CompilerState::replaceSymbolInCurrentScope(Symbol * oldsym, Symbol * newsym)
  {
    assert(!useMemberBlock());
    getCurrentBlock()->replaceIdInScope(oldsym, newsym);
  }

  //symbol ownership goes to the caller;
  bool CompilerState::takeSymbolFromCurrentScope(u32 id, Symbol *& rtnsymptr)
  {
    assert(!useMemberBlock());
    return getCurrentBlock()->removeIdFromScope(id, rtnsymptr);
  }

  bool CompilerState::findNearestBaseClassToAnAncestor(UTI cuti, UTI auti, UTI& foundInBase)
  {
    bool rtnb = false;

    BaseclassWalker walkerpair; //"nearest" irrelevent, all bases are shared
    walkerpair.init(cuti); //Nouti pair

    UTI baseuti = Nouti;
    UTI headuti = Nouti;
    while(!rtnb && walkerpair.getNextBasePair(baseuti, headuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    s32 baseitem = basecsym->isABaseClassItem(auti);
	    rtnb = (baseitem >= 0);

	    if(rtnb)
	      {
		if(headuti == Nouti) headuti = baseuti;
		foundInBase = headuti;
	      }
	    else
	      {
		UTI basehead = (headuti == Nouti) ? baseuti : headuti; //t3600
		walkerpair.addAncestorPairsOf(basecsym, basehead);
	      }
	  }
      } //end while

    if(!rtnb)
      foundInBase = Nouti;

    return rtnb;
  } //findNearestBaseClassToAnAncestor

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

  u32 CompilerState::getTokenDataAsStringId(const Token & tok)
  {
    if(tok.m_dataindex > 0)
      {
	if(tok.m_type == TOK_DQUOTED_STRING)
	  return tok.m_dataindex; //different pool
	else if(tok.isOperatorOverloadIdentToken(this))
	  return tok.m_dataindex; //t41238
	return tok.m_dataindex; //(else)
      }
    return tok.getTokenStringId();
  }

  const std::string & CompilerState::getTokenDataAsString(const Token & tok)
  {
    if(tok.m_dataindex > 0)
      {
	if(tok.m_type == TOK_DQUOTED_STRING)
	  return m_tokenupool.getDataAsFormattedString(tok.m_dataindex, this);
	else if(tok.isOperatorOverloadIdentToken(this))
	  return m_pool.getDataAsString(tok.getUlamNameIdForOperatorOverloadToken(this));

	return m_pool.getDataAsString(tok.m_dataindex);
      }
    //return std::string(tok->getTokenString()); //VG: Invalid Read
    return tok.getTokenStringFromPool(this);
  }

  const std::string & CompilerState::getDataAsFormattedUserString(u32 combinedidx)
  {
    //UTI cuti = (combinedidx >> STRINGIDXBITS);
    u32 sidx = (combinedidx & STRINGIDXMASK);
    //assert(cuti > 0 && sidx > 0); // error/t3987
    assert(sidx > 0); // error/t3987
    //StringPoolUser& classupool = getUPoolRefForClass(cuti);
    //return classupool.getDataAsFormattedString(sidx, this);
    return m_upool.getDataAsFormattedString(sidx, this);
  }

  const std::string & CompilerState::getDataAsUnFormattedUserString(u32 combinedidx)
  {
    //UTI cuti = (combinedidx >> STRINGIDXBITS);
    u32 sidx = (combinedidx & STRINGIDXMASK);
    //assert(cuti > 0 && sidx > 0); // t3959
    assert(sidx > 0); // t3959
    //StringPoolUser& classupool = getUPoolRefForClass(cuti);
    //return classupool.getDataAsString(sidx);
    return m_upool.getDataAsString(sidx);
  }

  bool CompilerState::isValidUserStringIndex(u32 combinedidx)
  {
    //UTI cuti = (combinedidx >> STRINGIDXBITS);
    u32 sidx = (combinedidx & STRINGIDXMASK);
    //if(cuti == 0 || sidx == 0)
    if(sidx == 0)
      return false;
    //StringPoolUser& classupool = getUPoolRefForClass(cuti);
    //return (sidx < classupool.getUserStringPoolSize());
    return (sidx < m_upool.getUserStringPoolSize());
  }

  u32 CompilerState::getUserStringLength(u32 combinedidx)
  {
    //UTI cuti = (combinedidx >> STRINGIDXBITS);
    u32 sidx = (combinedidx & STRINGIDXMASK);
    //assert(cuti > 0 && sidx > 0);
    assert(sidx > 0);
    //StringPoolUser& classupool = getUPoolRefForClass(cuti);
    //return classupool.getStringLength(sidx);
    return m_upool.getStringLength(sidx);
  }

  u32 CompilerState::formatAndGetIndexForDataUserString(std::string& astring)
  {
    // e.g. used to get function name as a string for ulam programmer (t41368);
    // format user string; length must be less than 256. similar to Lexer.
    u32 slen = astring.length();
    if(slen > MAX_USERSTRING_LENGTH)
      {
	std::ostringstream errmsg;
	errmsg << "Could not complete user string <" << astring;
	errmsg << ">; Must be less than " << MAX_USERSTRING_LENGTH + 1;
	errmsg << " length, not " << slen;
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), errmsg.str().c_str(), ERR);
	return 0; //t41370
      }

    std::ostringstream newstr;
    if(slen == 0)
      newstr << (u8) 0;
    else
      newstr << (u8) slen << astring << (u8) 0; //slen doesn't include itself or terminating byte; see StringPoolUser.
    return m_upool.getIndexForDataString(newstr.str());
  } //formatAndGetIndexForDataUserString

  std::string CompilerState::getDataAsStringMangled(u32 dataindex)
  {
    std::ostringstream mangled;
    std::string nstr = m_pool.getDataAsString(dataindex);
    mangled << ToLeximited(nstr);
    return mangled.str();
  }

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
	    if(getUlamTypeByTypedefName(tok.m_dataindex, tduti, tmpforscalaruti) == TBOOL_TRUE)
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

  bool CompilerState::isPossibleTypeName(u32 id)
  {
    std::string str = m_pool.getDataAsString(id);
    char c = str.at(0);
    return Token::isUpper(c);
  }

  // note: we may miss a missing return value for non-void function; non-trivial to
  // do logic-flow analysis; gcc catches them until ulam template absorbs gcc errors.
  bool CompilerState::checkFunctionReturnNodeTypes(SymbolFunction * fsym)
  {
    bool rtnBool = true;
    UTI it = fsym->getUlamTypeIdx();

    if(m_currentFunctionReturnNodes.empty())
      {
	if((it != Void) && !fsym->isNativeFunctionDeclaration() && (!fsym->isVirtualFunction() || !fsym->isPureVirtualFunction()) && !fsym->isConstructorFunction())
	  {
	    std::ostringstream msg;
	    msg << "Function '" << m_pool.getDataAsString(fsym->getFunctionNameId()).c_str();
	    msg << "''s Return Statement is missing; Return type: ";
	    msg << getUlamTypeNameByIndex(it).c_str();
	    MSG2(fsym->getTokPtr(), msg.str().c_str(), ERR);
	    return false; //t41564 overload op[];
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
	    msg << "Function '" << m_pool.getDataAsString(fsym->getFunctionNameId()).c_str();
	    msg << "''s Return type's: " << getUlamTypeNameByIndex(it).c_str();
	    msg << " does not match incomplete resulting type";
	    msg << " " << getUlamTypeNameByIndex(rType).c_str();
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
		msg << "Function '" << m_pool.getDataAsString(fsym->getFunctionNameId()).c_str();
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
		    msg << "Function '" << m_pool.getDataAsString(fsym->getFunctionNameId()).c_str();
		    msg << "''s Return type's " << getUlamTypeNameByIndex(it).c_str();
		    msg << " array size [" << getArraySize(it);
		    msg << "] does not match resulting type's ";
		    msg << getUlamTypeNameByIndex(rType).c_str() << " array size [";
		    msg << getArraySize(rType) << "]";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_ERR);
		  }

		if(getBitSize(rType) != getBitSize(it))
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getFunctionNameId()).c_str();
		    msg << "''s Return type's " << getUlamTypeNameByIndex(it).c_str();
		    msg << " bit size (" << getBitSize(it);
		    msg << ") does not match resulting type's ";
		    msg << getUlamTypeNameByIndex(rType).c_str() << " bit size (";
		    msg << getBitSize(rType) << ")";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", rNode->getNodeLocation().getLineNo(), MSG_ERR);
		  }

		if(getReferenceType(rType) != getReferenceType(it))
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getFunctionNameId()).c_str();
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
  }

  void CompilerState::indent(File * fp)
  {
    // NO outputLineNumberForDebugging
    for(u32 i = 0; i < m_currentIndentLevel; i++)
      fp->write(m_indentedSpaceLevel);
  }

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
    std::string str(CUSTOMARRAY_GET_FUNCNAME);
    return m_pool.getIndexForDataString(str);
  }

  u32 CompilerState::getCustomArraySetFunctionNameId()
  {
    //kept for backward-compatible error msg
    std::string str(CUSTOMARRAY_SET_FUNCNAME);
    return m_pool.getIndexForDataString(str);
  }

  const char * CompilerState::getCustomArrayGetMangledFunctionName()
  {
    return CUSTOMARRAY_GET_MANGLEDNAME;
  }

  u32 CompilerState::getCustomArrayLengthofFunctionNameId()
  {
    std::string str(CUSTOMARRAY_LENGTHOF_FUNCNAME);
    return m_pool.getIndexForDataString(str);
  }

  const char * CompilerState::getCustomArrayLengthofMangledFunctionName()
  {
    return CUSTOMARRAY_LENGTHOF_MANGLEDNAME;
  }

  const char * CompilerState::getIsMangledFunctionName(UTI ltype)
  {
    if(isAtom(ltype))
      return IS_MANGLED_FUNCNAME_FOR_ATOM;

    return IS_MANGLED_FUNCNAME;
  }

  const char * CompilerState::getAsMangledFunctionName(UTI ltype, UTI rtype)
  {
    assert(okUTItoContinue(rtype));
    ULAMCLASSTYPE rclasstype = getUlamTypeByIndex(rtype)->getUlamClassType();
    if(rclasstype == UC_QUARK)
      return getIsMangledFunctionName(ltype);
    else if (rclasstype == UC_ELEMENT)
      return IS_MANGLED_FUNCNAME;
    else if (rclasstype == UC_TRANSIENT)
      return IS_MANGLED_FUNCNAME;
    else
      abortUndefinedUlamClassType();

    return "AS_ERROR";
  } //getAsMangledFunctionName

  const char * CompilerState::getGetRelPosMangledFunctionName(UTI ltype, bool doChk)
  {
    if(isAtom(ltype))
      return GETRELPOS_MANGLED_FUNCNAME_FOR_ATOM; //wo check, uses UlamClass method

    if(doChk)
      return GETRELPOS_MANGLED_FUNCNAME_WITHCHECK;
    return GETRELPOS_MANGLED_FUNCNAME;
  }

  const char * CompilerState::getDataMemberInfoFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype)); //if atom?
    return GETDATAMEMBERINFO_FUNCNAME;
  }

  const char * CompilerState::getDataMemberCountFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype)); //if atom?
    return GETDATAMEMBERCOUNT_FUNCNAME;
  }

  const char * CompilerState::getNumberOfBasesFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype)); //if atom?
    return GETNUMBEROFBASES_FUNCNAME;
  }

  const char * CompilerState::getNumberOfDirectBasesFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype)); //if atom?
    return GETNUMBEROFDIRECTBASES_FUNCNAME;
  }

  const char * CompilerState::getOrderedBaseClassFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype)); //if atom?
    return GETORDEREDBASE_FUNCNAME;
  }

  const char * CompilerState::getIsDirectBaseClassFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype)); //if atom?
    return GETISDIRECTBASECLASS_FUNCNAME;
  }

  const char * CompilerState::getClassMangledNameFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETCLASSMANGLEDNAME_FUNCNAME;
  }

  const char * CompilerState::getClassMangledNameAsStringIndexFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETCLASSMANGLEDNAMESTRINGINDEX_FUNCNAME;
  }

  const char * CompilerState::getClassNameAsStringIndexFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETCLASSNAMESTRINGINDEX_FUNCNAME;
  }

  const char * CompilerState::getClassLengthFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETCLASSLENGTH_FUNCNAME;
  }

  const char * CompilerState::getBaseClassLengthFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETBASECLASSLENGTH_FUNCNAME;
  }

  const char * CompilerState::getClassRegistrationNumberFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETCLASSREGISTRATIONNUMBER_FUNCNAME;
  }

  const char * CompilerState::getElementTypeFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETELEMENTTYPE_FUNCNAME;
  }

  const char * CompilerState::getReadTypeFieldFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return READTYPEFIELD_FUNCNAME;
  }

  const char * CompilerState::getWriteTypeFieldFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return WRITETYPEFIELD_FUNCNAME;
  }

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
      abortUndefinedUlamClassType();
    return "BUILDEFAULTATOM_ERROR";
  } //getBuildDefaultAtomFunctionName

  const char * CompilerState::getDefaultQuarkFunctionName()
  {
    return BUILD_DEFAULT_QUARK_FUNCNAME;
  }

  const char * CompilerState::getVTableEntryFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETVTABLEENTRY_FUNCNAME;
  }

  const char * CompilerState::getVTableEntryClassPtrFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETVTABLEENTRYCLASSPTR_FUNCNAME;
  }

  const char * CompilerState::getVTableEntryStartOffsetForClassFunctionName(UTI ltype)
  {
    assert(okUTItoContinue(ltype));
    return GETVTABLEENTRYSTARTOFFSETFORCLASS_FUNCNAME;
  }

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

  void CompilerState::genCModeForHeaderFile(File * fp)
  {
    assert(fp);
    fp->write(CModeForHeaderFiles);
  }

  void CompilerState::genCopyrightAndLicenseForUlamHeader(File * fp)
  {
    assert(fp);
    fp->write(CopyrightAndLicenseForUlamHeader);
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

  u32 CompilerState::getMangledClassNameIdForUlamLocalsFilescope(UTI locuti)
  {
    assert(isALocalsFileScope(locuti));
    UlamType * locut = getUlamTypeByIndex(locuti);
    u32 classid = 0;
    AssertBool foundClassName = getClassNameFromFileName(locut->getUlamTypeNameOnly(), classid); //without trailing .ulam (no dots allowed)
    assert(foundClassName);

    std::ostringstream localclassname;
    localclassname << "_l" << m_pool.getDataAsString(classid);// << "ulam";

    std::ostringstream mangled;
    mangled << "Ul_1010" << ToLeximited(localclassname.str()).c_str() << "10"; //leximited
    return m_pool.getIndexForDataString(mangled.str());
  }

  u32 CompilerState::getClassNameIdForUlamLocalsFilescope(UTI locuti)
  {
    assert(isALocalsFileScope(locuti));
    UlamType * locut = getUlamTypeByIndex(locuti);
    u32 classid = 0;
    AssertBool foundClassName = getClassNameFromFileName(locut->getUlamTypeNameOnly(), classid); //without trailing .ulam (no dots allowed)
    assert(foundClassName);

    std::ostringstream f;
    f << "_l" << m_pool.getDataAsString(classid).c_str();// "_l" prepended to be distinct "temporary" class name.
    return m_pool.getIndexForDataString(f.str());
  }

  const char * CompilerState::getGetStringFunctionName()
  {
    return GETSTRING_FUNCNAME;
  }

  const char * CompilerState::getGetStringLengthFunctionName()
  {
    return GETSTRINGLENGTH_FUNCNAME;
  }

  const std::string CompilerState::getStringMangledName()
  {
    return getUlamTypeByIndex(String)->getLocalStorageTypeAsString();
  }

  const char * CompilerState::getMangledNameForUserStringPool()
  {
    return USERSTRINGPOOL_MANGLEDNAME;
  }

  const char * CompilerState::getMangledNameForUserStringPoolIndexes()
  {
    return USERSTRINGPOOLINDEXES_MANGLEDNAME;
  }

  const char * CompilerState::getDefineNameForUserStringPoolSize()
  {
    return USERSTRINGPOOL_SIZEDEFINENAME;
  }

  const char * CompilerState::getDefineNameForNumberOfUserStrings()
  {
    return USERSTRINGPOOL_NUMBERSTRINGSDEFINENAME;
  }

  std::string CompilerState::getFileNameForUserStringPoolHeader(bool wSubDir)
  {
    std::ostringstream f;
    if(wSubDir)
      f << "include/";

    f << USERSTRINGPOOL_FILENAME << ".h";
    return f.str();
  } //getFileNameForUserStringPoolHeader

  std::string CompilerState::getFileNameForUserStringPoolCPP(bool wSubDir)
  {
    std::ostringstream f;
    if(wSubDir)
      f << "src/";

    f << USERSTRINGPOOL_FILENAME << ".cpp";
    return f.str();
  } //getFileNameForUserStringPoolCPP

  ULAMCLASSTYPE CompilerState::getUlamClassForThisClass()
  {
    UTI cuti = getUlamTypeForThisClass();
    return getUlamTypeByIndex(cuti)->getUlamClassType();
  }

  UTI CompilerState::getUlamTypeForThisClass()
  {
    return getCompileThisIdx();
  }

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
	    lenstr << csym->getMangledName(); //t41141

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
	      abortUndefinedUlamClassType(); //error!! neither quark nor element
	  }
	else
	    abortUndefinedUlamClassType(); //error!! no class symbol for this type
      }
    return lenstr.str();
  } //getBitVectorLengthAsStringForCodeGen

  UlamValue CompilerState::getPtrTarget(UlamValue ptr)
  {
    s32 cnt = 0; //no more than MAX times around
    if(ptr.getPtrStorage() == EVALRETURN)
      return getPtrTargetOnce(ptr);
    return getPtrTarget(ptr, ptr.isPtrAbs(), false, cnt); //to value
  }

  UlamValue CompilerState::getPtrTargetOnce(UlamValue ptr)
  {
    s32 cnt = PTR_LOOP_MAX; //no more than MAX times around
    return getPtrTarget(ptr, ptr.isPtrAbs(), false, cnt); //to value
  }

  UlamValue CompilerState::getPtrTargetLastPtr(UlamValue ptr)
  {
    s32 cnt = 0; //no more than MAX times around
    return getPtrTarget(ptr, ptr.isPtrAbs(), true, cnt); //last ptr in chain
  }

  UlamValue CompilerState::getPtrTarget(UlamValue ptr, bool isAbsolute, bool ptrOnly, s32& cntref)
  {
    if(cntref++ > PTR_LOOP_MAX)
      return ptr; //abortNotSupported();

    if(!ptrOnly && !ptr.isPtr())
      return ptr;

    //slot + storage
    UlamValue valAtIdx;
    switch(ptr.getPtrStorage())
      {
      case STACK:
	{
	  if(isAbsolute)
	    valAtIdx = m_funcCallStack.loadUlamValueFromStackIndex(ptr.getPtrSlotIndex());
	  else
	    valAtIdx = m_funcCallStack.loadUlamValueFromSlot(ptr.getPtrSlotIndex());
	}
	break;
      case EVALRETURN:
	{
	  if(isAbsolute)
	    valAtIdx = m_nodeEvalStack.loadUlamValueFromStackIndex(ptr.getPtrSlotIndex());
	  else
	    valAtIdx = m_nodeEvalStack.loadUlamValueFromSlot(ptr.getPtrSlotIndex());
	}
	break;
      case EVENTWINDOW:
	{
	  if(isAbsolute)
	    {
	      abortShouldntGetHere();
	      valAtIdx = m_eventWindow.loadAtomFromSite(ptr.getPtrSlotIndex()); //?
	    }
	  else
	    valAtIdx = m_eventWindow.loadAtomFromSite(ptr.getPtrSlotIndex());
	}
	break;
      case CNSTSTACK:
	valAtIdx = m_constantStack.loadUlamValueFromStackIndex(ptr.getPtrSlotIndex()); //same
	break;
      default:
	abortUndefinedCallStack(); //error!
	break;
      };

    if(ptrOnly && !valAtIdx.isPtr())
      return ptr;

    return getPtrTarget(valAtIdx, valAtIdx.isPtrAbs(), ptrOnly, cntref); //return as-is
  } //getPtrTarget (recursive helper)

  bool CompilerState::isLocalUnreturnableReferenceForEval(UlamValue ptr)
  {
    assert(ptr.isPtr());

    if(ptr == m_currentSelfPtr)
      return false;

    if(ptr.isPtrAbs())
      return isLocalUnreturnableAbsoluteReferenceForEval(ptr); //short-circuit

    bool brtn = true;
    switch(ptr.getPtrStorage())
      {
      case STACK:
	brtn = m_funcCallStack.isLocalSlot(ptr.getPtrSlotIndex(), *this);
	break;
      case EVALRETURN:
	brtn = m_nodeEvalStack.isLocalSlot(ptr.getPtrSlotIndex(), *this);
	break;
      case CNSTSTACK:
	brtn = m_constantStack.isLocalStackIndex(ptr.getPtrSlotIndex(), *this);
	break;
      case EVENTWINDOW:
	brtn = false; //data member t3630, t3942, t3946, t41006, t41007
	break;
      default:
	abortUndefinedCallStack(); //error!
	break;
      };
    return brtn;
  } //isLocalUnreturnableReferenceForEval

  bool CompilerState::isLocalUnreturnableAbsoluteReferenceForEval(UlamValue ptr)
  {
    assert(ptr.isPtrAbs());

    bool brtn = true;
    switch(ptr.getPtrStorage())
      {
      case STACK:
	brtn = m_funcCallStack.isLocalStackIndex(ptr.getPtrSlotIndex(), *this);
	break;
      case EVALRETURN:
	brtn = m_nodeEvalStack.isLocalStackIndex(ptr.getPtrSlotIndex(), *this);
	break;
      case CNSTSTACK:
	brtn = m_constantStack.isLocalStackIndex(ptr.getPtrSlotIndex(), *this);
	break;
      case EVENTWINDOW:
	brtn = false; //data member
	break;
      default:
	abortUndefinedCallStack(); //error!
	break;
      };
    return brtn;
  } //isLocalUnreturnableAbsoluteReferenceForEval

  //general purpose store
  void CompilerState::assignValue(UlamValue lptr, UlamValue ruv)
  {
    assert(lptr.isPtr());
    UTI lttype = lptr.getPtrTargetType();
    UTI rttype = ruv.isPtr() ? ruv.getPtrTargetType() : ruv.getUlamValueTypeIdx(); //t3250

    //handle UAtom assignment as a singleton (not array values)
    if(ruv.isPtr() || (!isScalar(lttype) && isScalar(rttype)))
      {
	PACKFIT rpacked = ruv.isPtr() ? ruv.isTargetPacked() : determinePackable(rttype); //t3250
	if((rttype != UAtom) && (rpacked != UNPACKED) && (lttype == rttype))
	  return assignArrayValues(lptr, ruv);
	else if((rttype != UAtom) && (UlamType::compare(getUlamTypeAsScalar(lttype), rttype, *this) == UTIC_SAME))
	  return assignArrayValues(lptr, ruv); //t3707, t3250
	else if((rttype != UAtom) && (UlamType::compare(lttype, rttype, *this) == UTIC_SAME))
	  return assignArrayValues(lptr, ruv); //t3707
	else
	  return assignValuePtr(lptr, ruv); //t41483, t3707 (UNPACKED)
      }
    //else //t3250 after ruv is target of ptr;

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
      case CNSTSTACK:
      default:
	abortUndefinedCallStack();
	break;
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
	//assert(!atval.isPtr());
	if(atval.isPtr())
	  return assignValuePtr(lptr, rptr); //t41548

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
	//assign each array item, packed or unpacked
	u32 size = slotsNeeded(rptr.getPtrTargetType());

	UlamValue nextlptr = UlamValue::makeScalarPtr(lptr,*this);
	UlamValue nextrptr = UlamValue::makeScalarPtr(rptr,*this);
	tuti = nextrptr.getPtrTargetType(); //update type

	for(u32 i = 0; i < size; i++)
	  {
	    UlamValue atval = getPtrTarget(nextrptr);
	    assert(!atval.isPtr());

	    //redo what getPtrTarget use to do, when types didn't match due to
	    //an element/quark or a requested scalar of an arraytype
	    if(UlamType::compareForUlamValueAssignment(atval.getUlamValueTypeIdx(), tuti, *this) != UTIC_SAME)
	      {
		if(rptr.isTargetPacked() == PACKED)
		  {
		    UlamValue atvalUV = UlamValue::getPackedArrayDataFromAtom(rptr, atval, *this);
		    assignValue(nextlptr, atvalUV);
		  }
		else //UNPACKED, but <= long (t3707)
		  {
		    UlamValue atvalUV = UlamValue::getPackedArrayDataFromAtom(rptr, atval, *this);
		    assignValue(nextlptr, atvalUV);
		  }
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
    assert(lptr.isPtr());
    assert(rptr.isPtr());

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
      case CNSTSTACK:
      default:
       abortUndefinedCallStack();
       break;
      };
  } //assignValuePtr

  UlamValue CompilerState::getByteOfUserStringForEval(u32 usrStr, u32 offsetInt)
  {
    //UTI cuti = (usrStr >> STRINGIDXBITS);
    u32 sidx = (usrStr & STRINGIDXMASK);
    //assert((cuti > 0) && (sidx > 0));
    assert((sidx > 0));
    //StringPoolUser& classupool = getUPoolRefForClass(cuti);
    //u8 c = classupool.getByteOf(sidx, offsetInt);
    u8 c = m_upool.getByteOf(sidx, offsetInt);
    return UlamValue::makeImmediate(ASCII, c, *this);
  }

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

  bool CompilerState::classCustomArraySetable(UTI cuti)
  {
    assert(isScalar(cuti));
    bool rtnb = false;
    bool hazykin = false;
    Symbol * fsym = NULL;
    if(isFuncIdInAClassScopeOrAncestor(cuti, getCustomArrayGetFunctionNameId(), fsym, hazykin))
      rtnb = isReference(((SymbolFunctionName *) fsym)->getCustomArrayReturnType());
    return rtnb;
  } //classCustomArraySetable

  bool CompilerState::hasThisClassStringDataMembers()
  {
    NodeBlockContext * cxblock = getContextBlock();
    assert(cxblock);
    assert(cxblock->getNodeType() == getCompileThisIdx());
    return cxblock->hasStringDataMembers();
  }

  void CompilerState::extractQuarkBaseFromSubclassForEval(UlamValue fmuvarg, UTI buti, UlamValue& touvref)
  {
    UTI debuti = getUlamTypeAsDeref(buti); //t41319 attempts to cast ele to base quarkref
    UTI cuti = fmuvarg.getUlamValueTypeIdx();
    if(isAtom(cuti))
      cuti = fmuvarg.getUlamValueEffSelfTypeIdx(); //t41315
    assert(isClassASubclassOf(cuti, debuti) || isClassASubclassOf(debuti, cuti) || (UlamType::compare(cuti, debuti, *this) == UTIC_SAME) );

    UTI tuti = touvref.getUlamValueTypeIdx(); //t41640
    assert(!touvref.isPtr());
    if(isAtom(tuti))
      tuti = touvref.getUlamValueEffSelfTypeIdx();
    assert(isClassASubclassOf(tuti, debuti) || isClassASubclassOf(debuti, tuti) || (UlamType::compare(tuti, debuti, *this) == UTIC_SAME) );

    s32 pos = ATOMFIRSTSTATEBITPOS; //all classes start after type in ulamvalue

    BaseclassWalker walker;
    walker.init(debuti);

    //ulam-5 supports multiple base classes; superclass optional;
    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    s32 blen = getBaseClassBitSize(baseuti);
	    assert(blen <= MAXBITSPERLONG);

	    if(blen > 0)
	      {
		u32 fmrelpos = 0;
		bool gotfm = getABaseClassRelativePositionInAClass(cuti, baseuti, fmrelpos);

		u32 torelpos = 0;
		bool gotto = getABaseClassRelativePositionInAClass(tuti, baseuti, torelpos);

		if(gotfm && gotto)
		  {
		    u64 qdata = fmuvarg.getDataLong(pos + fmrelpos, blen);
		    touvref.putDataLong(pos + torelpos, blen, qdata);
		  }
	      }

	    //include all ancestors of arg buti
	    walker.addAncestorsOf(basecsym);
	  }
      } //end while
  } //extractQuarkBaseFromSubclassForEval

  void CompilerState::extractTransientBaseFromSubclassForEval(UlamValue fmuvarg, UTI buti, UlamValue& touvref)
  {
    UTI debuti = getUlamTypeAsDeref(buti);
    UTI cuti = fmuvarg.getUlamValueTypeIdx();
    assert(isClassASubclassOf(cuti, debuti) || isClassASubclassOf(debuti, cuti) || (UlamType::compare(cuti, debuti, *this) == UTIC_SAME) );

    UTI tuti = touvref.getUlamValueTypeIdx();
    assert(!touvref.isPtr());
    if(isAtom(tuti))
      tuti = touvref.getUlamValueEffSelfTypeIdx();
    assert(isClassASubclassOf(tuti, debuti) || isClassASubclassOf(debuti, tuti) || (UlamType::compare(tuti, debuti, *this) == UTIC_SAME) );

    s32 pos = ATOMFIRSTSTATEBITPOS; //all classes start after type in ulamvalue

    BaseclassWalker walker;
    walker.init(debuti);

    //ulam-5 supports multiple base classes; superclass optional;
    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    s32 blen = getBaseClassBitSize(baseuti);
	    assert(blen <= MAXSTATEBITS);

	    if(blen > 0)
	      {
		u32 fmrelpos = 0;
		bool gotfm = getABaseClassRelativePositionInAClass(cuti, baseuti, fmrelpos);

		u32 torelpos = 0;
		bool gotto = getABaseClassRelativePositionInAClass(tuti, baseuti, torelpos);

		if(gotfm && gotto)
		  {
		    BV8K bvdata;
		    fmuvarg.getDataBig(pos + fmrelpos, blen, bvdata);
		    touvref.putDataBig(pos + torelpos, blen, bvdata);
		  }
	      }

	    //include all ancestors of arg buti
	    walker.addAncestorsOf(basecsym);
	  }
      } //end while
  } //extractTransientBaseFromSubclassForEval

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
    m_funcCallStack.pushArg(m_currentObjPtr); //hidden self arg on STACK
    m_funcCallStack.pushArg(UlamValue::makeImmediate(Int, -1)); //uc,t3189
    m_funcCallStack.pushArg(UlamValue::makeImmediate(Int, -1)); //return slot on STACK
  } //setupCenterSiteForTesting

  void CompilerState::setupCenterSiteForGenCode()
  {
    Coord c0(0,0);

    //m_classBlock ok now, reset by NodeProgram after type label done
    UTI cuti = getCompileThisIdx();
    m_eventWindow.setSiteElementType(c0, cuti); //includes default values
  } //setupCenterSiteForGenCode

  void CompilerState::generateTestInstancesForLocalsFilescopes(File * fp)
  {
    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);
	localsblock->generateTestInstance(fp, false);
      }
  }

  void CompilerState::generateAllIncludesTestMainForLocalsFilescopes(File * fp)
  {
    std::map<u32, NodeBlockLocals *>::iterator it;
    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);
	localsblock->generateIncludeTestMain(fp);
      }
  }

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
    assert((linenum <= textOfLines->size()));
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
  }

  void CompilerState::outputTextAsCommentWithLocationUpdate(File * fp, Locator nodeloc)
  {
    outputTextAsComment(fp, nodeloc);
    m_locOfNextLineText = nodeloc; //during codeGen: NodeStatements, NodeBlockFunctionDef
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
    assert(m_nextTmpVarNumber < S32_MAX);
    return ++m_nextTmpVarNumber;
  }

  bool CompilerState::isStringATmpVar(u32 id)
  {
    std::ostringstream str;
    str << m_pool.getDataAsString(id);
    std::string name = str.str();
    if(name.length() > 3)
      return((name[0] == 'U') && (name[1] == 'h') && (name[2] == '_'));
    return false;
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

    switch(stg)
      {
      case TMPREGISTER:
      case TMPARRAYIDX:
	{
	  if(isLoadableRegister)
	    tmpVar << "Uh_5tlreg" ; //tmp loadable register
	  else
	    tmpVar << "Uh_5tureg" ; //tmp unpacked register
	}
	break;
      case TMPBITVAL:
	{
	  if(isLoadableRegister)
	    tmpVar << "Uh_5tlval" ; //tmp loadable value
	  else
	    tmpVar << "Uh_5tuval" ; //tmp unpacked value
	}
	break;
      case TMPAUTOREF:
	{
	  if(isLoadableRegister)
	    tmpVar << "Uh_6tlref" ; //tmp loadable autoref
	  else
	    tmpVar << "Uh_6turef" ; //tmp unpacked autoref
	}
	break;
      case TMPTATOM:
	{
	  tmpVar << "Uh_3tut" ; //tmp unpacked atom T
	}
	break;
      case TMPATOMBS:
	{
	  tmpVar << "Uh_4tabs" ; //tmp atombitstorage
	}
	break;
      case TMPTBV:
	{
	  if(isLoadableRegister)
	    tmpVar << "Uh_3tlbv" ; //tmp loadable bitvector
	  else
	    tmpVar << "Uh_3tubv" ; //tmp unpacked bitvector
	}
	break;
      default:
	abortShouldntGetHere(); //removes assumptions about tmpbitval.
      };
    tmpVar << ToLeximitedNumber(num);
    return tmpVar.str();
  } //getTmpVarAsString

  const std::string CompilerState::getUlamRefTmpVarAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_3tur" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getUlamRefMutTmpVarAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_4turm" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getUlamClassTmpVarAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_7tuclass" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getUlamClassRegistryTmpVarAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_4tucr" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getAtomBitStorageTmpVarAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_4tabs" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getLabelNumAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Ul_214endcontrolloop" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getSwitchConditionNumAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_switchcond" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getSwitchTypedefNameAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "_SWITCHTYPEDEF" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getInitDoneVarAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uh_8initdone" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getVFuncPtrTmpNumAsString(s32 num)
  {
    std::ostringstream labelname; //into
    labelname << "Uf_tvfp" << ToLeximitedNumber(num);
    return labelname.str();
  }

  const std::string CompilerState::getParserSymbolTypeFlagAsString()
  {
    return getParserSymbolTypeFlagAsString(m_parsingVariableSymbolTypeFlag);
  }

  const std::string CompilerState::getParserSymbolTypeFlagAsString(SYMBOLTYPEFLAG stf)
  {
    std::ostringstream labelname; //into

    switch(stf)
      {
      case STF_CLASSINHERITANCE:
	labelname << "class ancestor";
	break;
      case STF_DATAMEMBER:
	labelname << "data member";
	break;
      case STF_FUNCPARAMETER:
	labelname << "function parameter";
	break;
      case STF_FUNCLOCALVAR:
	labelname << "function variable";
	break;
      case STF_FUNCARGUMENT:
	labelname << "function argument";
	break;
      case STF_FUNCLOCALREF:
	labelname << "function reference variable";
	break;
      case STF_FUNCLOCALCONSTREF:
	labelname << "function constant reference variable";
	break;
      case STF_MEMBERCONSTANT:
	labelname << "member constant";
	break;
      case STF_MEMBERTYPEDEF:
	labelname << "member typedef";
	break;

      case STF_NEEDSATYPE:
      default:
	labelname << "symbol of unflagged useage";
	break;
      }
    return labelname.str();
  } //getParserSymbolTypeFlagAsString

  void CompilerState::saveIdentTokenForPendingConditionalAs(const Token& iTok)
  {
    m_identTokenForConditionalAs = iTok;
  }

  void CompilerState::confirmParsingConditionalAs(const Token& cTok)
  {
    m_parsingConditionalToken = cTok;
    m_parsingConditionalAs = true; //cleared manually
  }

  void CompilerState::saveIdentTokenForConditionalAs(const Token& iTok, const Token& cTok)
  {
    m_identTokenForConditionalAs = iTok;
    m_parsingConditionalToken = cTok;
    m_parsingConditionalAs = true; //cleared manually
  }

  void CompilerState::saveStructuredCommentToken(const Token& scTok)
  {
    m_precedingStructuredCommentToken = scTok;
    m_gotStructuredCommentToken = true;
  }

  void CompilerState::clearStructuredCommentToken()
  {
    Token blankTok; //unitialized
    m_precedingStructuredCommentToken = blankTok;
    m_gotStructuredCommentToken = false;
  }

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

  void CompilerState::setThisClassForParsing(UTI cuti)
  {
    assert(m_parsingThisClass == Nouti);
    m_parsingThisClass = cuti;
  }

  void CompilerState::clearThisClassForParsing()
  {
    m_parsingThisClass = Nouti;
  }

  UTI CompilerState::getThisClassForParsing()
  {
    return m_parsingThisClass;
  }

  void CompilerState::saveFirstTokenForParsing(Token fTok)
  {
    m_firstTokenForParsing = fTok;
  }

  void CompilerState::getFirstTokenForParsing(Token& fTok)
  {
    fTok = m_firstTokenForParsing;
  }

  void CompilerState::clearFirstTokenForParsing()
  {
    Token clearTok;
    m_firstTokenForParsing = clearTok;
 }

  //set temporary flag for concrete class; used to set flag in SymbolClass
  void CompilerState::setConcreteClassFlagForParsing()
  {
    assert(m_parsingConcreteClassFlag == false);
    m_parsingConcreteClassFlag = true;
  }

  void CompilerState::clearConcreteClassFlagForParsing()
  {
    m_parsingConcreteClassFlag = false;
  }

  bool CompilerState::getConcreteClassFlagForParsing()
  {
    return m_parsingConcreteClassFlag;
  }

  void CompilerState::setLocalsScopeForParsing()
  {
    Token fTok;
    getFirstTokenForParsing(fTok);
    setLocalsScopeForParsing(fTok);
  }

  void CompilerState::setLocalsScopeForParsing(const Token& localTok)
  {
    m_parsingLocalDef = true;
    m_currentLocalDefToken = localTok;
    NodeBlockLocals * localsblock = makeLocalsScopeBlock(localTok.m_locator);
    assert(localsblock);
    pushClassContext(localsblock->getNodeType(), localsblock, localsblock, false, NULL);
  }

  void CompilerState::clearLocalsScopeForParsing()
  {
    m_parsingLocalDef = false;
    popClassContext(); //restore
  }

  bool CompilerState::isParsingLocalDef()
  {
    return m_parsingLocalDef;
  }

  Locator CompilerState::getLocalsScopeLocator()
  {
    assert(isParsingLocalDef());
    return m_currentLocalDefToken.m_locator;
  }

  NodeBlockLocals * CompilerState::getLocalsScopeBlock(Locator loc)
  {
    u32 pathidx = loc.getPathIndex();
    return getLocalsScopeBlockByPathId(pathidx);
  }

  NodeBlockLocals * CompilerState::getLocalsScopeBlockByIndex(UTI luti)
  {
    UlamType * lut = getUlamTypeByIndex(luti);
    u32 pathid = lut->getUlamTypeNameId();
    return getLocalsScopeBlockByPathId(pathid);
  } //getLocalsScopeBlockByIndex

  NodeBlockLocals * CompilerState::getLocalsScopeBlockByPathId(u32 pathid)
  {
    NodeBlockLocals * rtnLocals = NULL;

    std::map<u32, NodeBlockLocals*>::iterator it = m_localsPerFilePath.find(pathid);

    if(it != m_localsPerFilePath.end())
      {
	rtnLocals = it->second;
	assert(rtnLocals);
	assert(getUlamTypeByIndex(rtnLocals->getNodeType())->getUlamTypeNameId() == pathid); //sanity check
      }
    return rtnLocals;
  } //getLocalsScopeBlockByPathId

  NodeBlockLocals * CompilerState::makeLocalsScopeBlock(Locator loc)
  {
    NodeBlockLocals * rtnLocals = getLocalsScopeBlock(loc);
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
	assert(notdupi);

	//set node type based on ulam file name id
	UlamKeyTypeSignature lkey(pathidx, 0, NONARRAYSIZE, 0, ALT_NOT);
	UTI luti = makeUlamType(lkey, LocalsFileScope, UC_NOTACLASS);
	newblocklocals->setNodeType(luti);
	rtnLocals = newblocklocals;
      }
    return rtnLocals;
  } //makeLocalsScopeBlock

  u32 CompilerState::findTypedefNameIdInLocalsScopeByIndex(UTI uti)
  {
    NodeBlockLocals * rtnLocals = getLocalsScopeBlock(getContextBlockLoc());
    if(rtnLocals)
      return rtnLocals->findTypedefNameIdByType(uti);
    return 0; //not found
  } //findTypedefNameIdInLocalsScopeByIndex

  NNO CompilerState::getNextNodeNo()
  {
    return ++m_nextNodeNumber; //first one is 1
  }

  Node * CompilerState::findNodeNoInThisClassOrLocalsScope(NNO n)
  {
    if(useMemberBlock())
      {
	NodeBlockClass * memberblock = getCurrentMemberClassBlock();
	assert(memberblock);
	UTI mbuti = memberblock->getNodeType();
	return findNodeNoInAClassOrLocalsScope(n, mbuti);
      }

    NodeBlock * currBlock = getCurrentBlock();
    if(currBlock && (currBlock->getNodeNo() == n) && (getContextBlock()->getNodeType() == getCompileThisIdx()))
      return currBlock; //avoid chix-n-egg with functiondefs

    Node * rtnNode = NULL;

    //the the short way first..(safe tests timing no faster!)
    if(currBlock->findNodeNo(n, rtnNode))
      return rtnNode;

    //go the long way around..
    UTI cuti = getCompileThisIdx();
    rtnNode = findNodeNoInAClassOrLocalsScope(n, cuti);
    return rtnNode;
  } //findNodeNoInThisClass

  Node * CompilerState::findNodeNoInAncestorsClassOrLocalsScope(NNO n, UTI cuti)
  {
    Node * rtnNode = NULL;
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(cuti, csym))
      {
	u32 basecount = csym->getBaseClassCount() + 1; //include super

	u32 i = 0;
	while((rtnNode == NULL) && (i < basecount))
	  {
	    UTI baseuti = csym->getBaseClass(i);
	    SymbolClassName * basecnsym = NULL;
	    if(alreadyDefinedSymbolClassNameByUTI(baseuti, basecnsym))
	      {
		rtnNode = basecnsym->findNodeNoInAClassInstance(baseuti, n); //includes complete ancestors
		//local def, using (possible) template's local scope
		if(rtnNode == NULL)
		  rtnNode = findNodeNoInALocalsScope(basecnsym->getLoc(), n);

		if(rtnNode == NULL)
		  rtnNode = findNodeNoInAncestorsClassOrLocalsScope(n, baseuti); //recurse
	      }
	    i++;
	  } //end while
      }
    return rtnNode;
  } //findNodeNoInAncestorsClassOrLocalsScope

  Node * CompilerState::findNodeNoInThisClassForParent(NNO n)
  {
    abortShouldntGetHere();
    return findNodeNoInThisClassStubFirst(n);
  }

  Node * CompilerState::findNodeNoInThisClassStubFirst(NNO n)
  {
    Node * rtnNode = NULL;
    //if we are in the middle of resolving pending args for a stub
    // and to do constant folding, we need to find the parent node that's in the
    // stub's argument list (was resolver), NOT the context where the stub appears. t3362,3,4,6,7,8..
    if(!rtnNode)
      rtnNode = findNodeNoInThisClassOrLocalsScope(n);

    if(!rtnNode)
      {
	//exhaustive search as last resort
	UTI acuti = findAClassByNodeNo(n);
	if(acuti != Nouti)
	  rtnNode = findNodeNoInAClass(n, acuti);
      }

    return rtnNode;
  } //findNodeNoInThisClassStubFirst

  Node * CompilerState::findNodeNoInAClass(NNO n, UTI cuti)
  {
    Node * rtnNode = NULL;

    if(isALocalsFileScope(cuti))
      return NULL; //not a class (t3873)

    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassNameByUTI(cuti, cnsym);
    assert(isDefined);
    rtnNode = cnsym->findNodeNoInAClassInstance(cuti, n);

    //don't check local scope automatically, e.g. in case of superclass
    return rtnNode;
  } //findNodeNoInAClass

  Node * CompilerState::findNodeNoInAClassOrLocalsScope(NNO n, UTI cuti)
  {
    Node * rtnNode = NULL;

    if(isALocalsFileScope(cuti))
      {
	rtnNode = findNodeNoInALocalsScope(cuti, n);
	return rtnNode;
      }

    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClassNameByUTI(cuti, cnsym);
    assert(isDefined);
    rtnNode = cnsym->findNodeNoInAClassInstance(cuti, n); //includes ancestor when complete

    //check local scope automatically, e.g. in case of superclass;
    //using possible template's local scope, not use-scope
    if(!rtnNode)
      rtnNode = findNodeNoInALocalsScope(cnsym->getLoc(), n); //not ancestor's locals scopes

    return rtnNode;
  } //findNodeNoInAClassOrLocalFilescope

  UTI CompilerState::findAClassByNodeNo(NNO n)
  {
    return m_programDefST.findClassNodeNoForTableOfClasses(n); //Nouti not found
  }

  NodeBlockLocals * CompilerState::findALocalsScopeByNodeNo(NNO n)
  {
    NodeBlockLocals * rtnlocals = NULL;
    std::map<u32, NodeBlockLocals *>::iterator it;

    for(it = m_localsPerFilePath.begin(); it != m_localsPerFilePath.end(); it++)
      {
	NodeBlockLocals * localsblock = it->second;
	assert(localsblock);
	if(localsblock->getNodeNo() == n)
	  {
	    rtnlocals = localsblock;
	    break;
	  }
      }
    return rtnlocals;
  } //findALocalsScopeByNodeNo

  Node * CompilerState::findNodeNoInALocalsScope(Locator loc, NNO n)
  {
    Node * rtnNode = NULL;
    NodeBlockLocals * localsblock = getLocalsScopeBlock(loc);
    if(localsblock)
      localsblock->findNodeNo(n, rtnNode);
    return rtnNode;
  }

  Node * CompilerState::findNodeNoInALocalsScope(UTI luti, NNO n)
  {
    Node * rtnNode = NULL;
    NodeBlockLocals * localsblock = getLocalsScopeBlockByIndex(luti);
    if(localsblock)
      localsblock->findNodeNo(n, rtnNode);
    return rtnNode;
  }

  u32 CompilerState::getRegistrationNumberForClassOrLocalsScope(UTI cuti)
  {
    if(isAClass(cuti))
      return getAClassRegistrationNumber(cuti);
    else if(isALocalsFileScope(cuti))
      return getALocalsScopeRegistrationNumber(cuti);
    else
      abortShouldntGetHere();
    return 0;
  }

  u32 CompilerState::getAClassRegistrationNumber(UTI cuti)
  {
    assert(isAClass(cuti));
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->getRegistryNumber();
  }

  u32 CompilerState::getALocalsScopeRegistrationNumber(UTI luti)
  {
    assert(isALocalsFileScope(luti));
    NodeBlockLocals * rtnLocals = getLocalsScopeBlockByIndex(luti);
    if(rtnLocals)
      return rtnLocals->getRegistrationNumberForLocalsBlock();
    return 0; //not found
  }

  ELE_TYPE CompilerState::getAClassElementType(UTI cuti)
  {
    assert(isASeenElement(cuti));
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->getElementType();
  }

  ELE_TYPE CompilerState::getNextElementType(UTI cuti)
  {
    return m_elementTypeGenerator.makeNextType(cuti);
  }

  UTI CompilerState::lookupClassByElementType(ELE_TYPE ele)
  {
    if(ele == EMPTY_ELEMENT_TYPE)
      return getEmptyElementUTI();
    assert(ele != UNDEFINED_ELEMENT_TYPE);
    return m_elementTypeGenerator.findElementTypeClass(ele);
  }

  u32 CompilerState::assignClassId(UTI cuti)
  {
    if(isUrSelf(cuti))
      {
	m_classIdRegistryUTI[0] = cuti;  //assigned at last
	return 0;
      }
    assert(getMaxNumberOfRegisteredUlamClasses() < MAX_REGISTRY_NUMBER);
    m_classIdRegistryUTI.push_back(cuti);
    return m_classIdRegistryUTI.size() - 1;
  } //assignClassId

  NodeBlockClass * CompilerState::getAClassBlock(UTI cuti)
  {
    assert(!isALocalsFileScope(cuti));
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->getClassBlockNode();
  }

  NNO CompilerState::getAClassBlockNo(UTI cuti)
  {
    assert(!isALocalsFileScope(cuti));
    SymbolClass * csym = NULL;
    AssertBool isDefined = alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    return csym->getClassBlockNode()->getNodeNo();
  }

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

  Locator CompilerState::getCompileThisLoc()
  {
    NodeBlockContext * cblock = getContextBlock();
    assert(cblock);
    return cblock->getNodeLocation();
  }

  SymbolClass * CompilerState::getCurrentSelfSymbolForCodeGen()
  {
    return (SymbolClass *) m_currentSelfSymbolForCodeGen;
  }

  void CompilerState::appendNodeToCurrentBlock(Node * node)
  {
    NodeBlock * cblock = getCurrentBlockForSearching();
    assert(cblock);
    cblock->appendNextNode(node); //adds a NodeStatements, becomes the new end
  }

  NodeBlock * CompilerState::getCurrentBlock()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    return cc.getCurrentBlock();
  }

  NNO CompilerState::getCurrentBlockNo()
  {
    ClassContext cc;
    if(m_classContextStack.getCurrentClassContext(cc) && getCurrentBlock())
      return getCurrentBlock()->getNodeNo();
    return 0; //genesis of class symbol
  }

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

  Locator CompilerState::getContextBlockLoc()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    assert(cc.getContextBlock());
    return cc.getContextBlock()->getNodeLocation();
  }

  bool CompilerState::useMemberBlock()
  {
    ClassContext cc;
    if(m_classContextStack.getCurrentClassContext(cc))
      return cc.useMemberBlock();
    return false; //genesis of a symbol getting current block no
  }

  NodeBlockClass * CompilerState::getCurrentMemberClassBlock()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    return cc.getCurrentMemberClassBlock();
  }

  NodeBlock * CompilerState::getCurrentBlockForSearching()
  {
    if(useMemberBlock())
      return getCurrentMemberClassBlock();
    return getCurrentBlock();
  }

  NodeBlockContext * CompilerState::getContextBlockForSearching()
  {
    if(useMemberBlock())
      return getCurrentMemberClassBlock();
    return getContextBlock();
  }

  void CompilerState::pushClassContext(UTI idx, NodeBlock * currblock, NodeBlockContext * contextblock, bool usemember, NodeBlockClass * memberblock)
  {
    u32 id = getUlamTypeNameIdByIndex(idx);
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

  void CompilerState::pushClassOrLocalContextAndDontUseMemberBlock(UTI context)
  {
    if(isAClass(context))
      {
	NodeBlockClass * contextclassblock = getAClassBlock(context);
	assert(contextclassblock);
	pushClassContext(context, contextclassblock, contextclassblock, false, NULL);
      }
    else
      {
	NodeBlockLocals * localsblock = getLocalsScopeBlockByIndex(context);
	assert(localsblock);
	pushClassContext(context, localsblock, localsblock, false, NULL);
      }
  } //helper

  void CompilerState::pushClassOrLocalCurrentBlock(UTI context)
  {
    if(isAClass(context))
      {
	NodeBlockClass * contextclassblock = getAClassBlock(context);
	assert(contextclassblock);
	pushCurrentBlock(contextclassblock);
      }
    else
      {
	NodeBlockLocals * localsblock = getLocalsScopeBlockByIndex(context);
	assert(localsblock);
	pushCurrentBlock(localsblock);
      }
  } //helper

  std::string CompilerState::getClassContextAsStringForDebugging()
  {
    ClassContext cc;
    AssertBool isDefined = m_classContextStack.getCurrentClassContext(cc);
    assert(isDefined);
    return cc.getClassContextAsString();
  }

  u32 CompilerState::countClassBlockOnContextStack(const NodeBlockClass * cblock)
  {
    //trying to detect cycles..stop the maddness!! how many is too many?
    return m_classContextStack.countClassContextOnStack(cblock);
  }

  void CompilerState::clearGoAgain()
  {
    m_goAgainResolveLoop = false;
  }

  void CompilerState::setGoAgain()
  {
    //convention: called only after a node type is actually set to Hzy
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
    //do not include ALT_AS, etc as Ref here. Specifically a ref (&).
    UlamType * aut = getUlamTypeByIndex(auti);
    return ((aut->getUlamTypeEnum() == UAtom) && isAltRefType(auti));
  }

  bool CompilerState::isThisLocalsFileScope()
  {
    return isALocalsFileScope(getCompileThisIdx());
  }

  bool CompilerState::isALocalsFileScope(UTI uti)
  {
    UlamType * ut = getUlamTypeByIndex(uti);
    return ((ut->getUlamTypeEnum() == LocalsFileScope) || (ut->getUlamClassType() == UC_LOCALSFILESCOPE));
  }

  bool CompilerState::isAPrimitiveType(UTI uti)
  {
    return (getUlamTypeByIndex(uti)->isPrimitiveType());
  }

  bool CompilerState::isAStringType(UTI uti)
  {
    //    return UlamType::compareForString(uti, *this) == UTIC_SAME; //arrays not detected
    return (getUlamTypeByIndex(uti)->getUlamTypeEnum() == String); //also includes arrays
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
  }

  bool CompilerState::isASeenElement(UTI cuti)
  {
    //includes refs, and arrays!!! NOT UNSEEN!
    ULAMCLASSTYPE classtype = getUlamTypeByIndex(cuti)->getUlamClassType();
    return (classtype == UC_ELEMENT);
  }

  bool CompilerState::isAnonymousClass(UTI cuti)
  {
    assert(okUTItoContinue(cuti));
    assert(isAClass(cuti));
    // anonymous classes have their UTI number as their nameid. (t3808)
    return(!isARootUTI(cuti) || isHolder(cuti));
  }

  void CompilerState::saveUrSelf(UTI uti)
  {
    m_urSelfUTI = uti;
  }

  bool CompilerState::isUrSelf(UTI cuti)
  {
    if(m_urSelfUTI == Nouti)
      {
	u32 urid = m_pool.getIndexForDataString("UrSelf");
	if(getUlamTypeNameIdByIndex(cuti) == urid)
	  saveUrSelf(cuti); //error/t3318
	else
	  {
	    SymbolClassName * ursym = NULL;
	    if(!alreadyDefinedSymbolClassName(urid, ursym))
	      {
		//required only once!
		Token urTok(TOK_TYPE_IDENTIFIER, m_locOfNextLineText, urid);
		addIncompleteClassSymbolToProgramTable(urTok, ursym);
		saveUrSelf(ursym->getUlamTypeIdx());
		//return false; //t3336
	      }
	  }
      }
    return (cuti == m_urSelfUTI); //no compare
  } //isUrSelf

  void CompilerState::saveEmptyElementUTI(UTI uti)
  {
    m_emptyElementUTI = uti;
  }

  bool CompilerState::isEmptyElement(UTI cuti)
  {
    if(m_emptyElementUTI == Nouti)
      {
	if(getUlamTypeNameIdByIndex(cuti) == m_pool.getIndexForDataString("Empty"))
	  saveEmptyElementUTI(cuti);
	else
	  return false;
      }
    return (cuti == m_emptyElementUTI); //no compare
  } //isEmptyElement

  UTI CompilerState::getEmptyElementUTI()
  {
    assert(m_emptyElementUTI != Nouti);
    return m_emptyElementUTI;
  }

  bool CompilerState::okUTItoContinue(UTI uti)
  {
    return ((uti != Nav) && (uti != Hzy) && (uti != Nouti) && !isStillHazy(uti));
  }

  bool CompilerState::neitherNAVokUTItoContinue(UTI uti1, UTI uti2)
  {
    return ((uti1 != Nav) && (uti2 != Nav));
  }

  bool CompilerState::checkHasHazyKin(NodeBlock * block)
  {
    assert(block);
    bool rtnb = block->isAClassBlock(); //t3172
    UTI buti = block->getNodeType(); //Int when eval test function (t41315)

    if(rtnb && !isClassAStub(buti) && !isHolder(buti))
      {
	if(!okUTItoContinue(buti))
	  rtnb = true; //HZY classes are not stubs (t41434)
	else if(buti == Int)
	  rtnb = false; //middle of test (t41315)
	else
	  {
	    assert(isAClass(buti)); //sanity
	    bool hasHazyKin = false;
	    SymbolClass * csym = NULL;
	    if(alreadyDefinedSymbolClass(buti, csym))
	      {
		u32 basecount = csym->getBaseClassCount() + 1; //include super
		u32 i = 0;
		while(!hasHazyKin && (i < basecount))
		  {
		    UTI baseuti = csym->getBaseClass(i);
		    if((baseuti != Nouti) && !isUrSelf(baseuti)) //super is optional, urSelf ok
		      hasHazyKin = !((NodeBlockClass *) block)->isBaseClassBlockReady(buti,baseuti);
		    i++;
		  } //end while
		rtnb = hasHazyKin;
	      }
	  }
      }
    //true if isaclass, AND either isastub, a holder, OR has hazykin (ie a baseclasslink notready)
    return rtnb;
  }

  bool CompilerState::hasHazyClassInHierarchy(UTI cuti)
  {
    bool hashazy = false;

    BaseclassWalker walker; //searches entire tree until a stub is found
    walker.init(cuti);

    UTI baseuti = Nouti;
    //ulam-5 supports multiple base classes; superclass optional;
    while(!hashazy && walker.getNextBase(baseuti, *this))
      {
	SymbolClass * basecsym = NULL;
	if(alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    hashazy = basecsym->isStub();

	    if(!hashazy)
	      walker.addAncestorsOf(basecsym);
	  }
      } //end while
    return hashazy;
  } //hasHazyClassInHierarchy

  bool CompilerState::isStillHazy(UTI uti)
  {
    return (getUlamTypeByIndex(uti)->getUlamTypeEnum() == Hzy);
  }

} //end MFM
