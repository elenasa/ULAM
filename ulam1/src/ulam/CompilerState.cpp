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

namespace MFM {

  //#define _DEBUG_OUTPUT
  //#define _INFO_OUTPUT
  //#define _WARN_OUTPUT

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

  static const char * m_indentedSpaceLevel("  ");  //2 spaces per level

  static const char * HIDDEN_ARG_NAME = "Uv_4self";
  static const char * HIDDEN_CONTEXT_ARG_NAME = "uc"; //unmangled
  static const char * CUSTOMARRAY_GET_FUNC_NAME = "aref";  //unmangled
  static const char * CUSTOMARRAY_SET_FUNC_NAME = "aset";  //unmangled
  static const char * IS_MANGLED_FUNC_NAME = "internalCMethodImplementingIs";  //Uf_2is
  static const char * HAS_MANGLED_FUNC_NAME = "PositionOfDataMemberType"; //Uf_3has
  static const char * HAS_MANGLED_FUNC_NAME_FOR_ATOM = "UlamElement<EC>::PositionOfDataMember";

  //use of this in the initialization list seems to be okay;
  CompilerState::CompilerState(): m_programDefST(*this), m_currentFunctionBlockDeclSize(0), m_currentFunctionBlockMaxDepth(0), m_parsingControlLoop(0), m_parsingElementParameterVariable(false), m_parsingConditionalAs(false), m_genCodingConditionalAs(false), m_eventWindow(*this), m_currentSelfSymbolForCodeGen(NULL), m_nextTmpVarNumber(0), m_nextNodeNumber(0)
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
    m_keyToaUTI.clear();

    m_currentFunctionReturnNodes.clear();

    s32 unknownKeyC = m_unknownKeyUTICounter.size();
    if(unknownKeyC > 0)
      {
	std::ostringstream msg;
	msg << "Remaining Unknown Keys with UTI counts, cleared: " << unknownKeyC;
	MSG2("", msg.str().c_str(),DEBUG);

	std::map<UlamKeyTypeSignature, u32, less_than_key>::iterator it = m_unknownKeyUTICounter.begin();

	while(it != m_unknownKeyUTICounter.end())
	  {
	    UlamKeyTypeSignature key = it->first;
	    u32 count = it->second;
	    std::ostringstream msg;
	    msg << "Key: " << key.getUlamKeyTypeSignatureAsString(this).c_str();
	    msg << ", " << count << " counted";
	    MSG2("", msg.str().c_str(),DEBUG);
	    it++;
	  }
      }
    m_unknownKeyUTICounter.clear();
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

  //convenience method (refactors code originally from installSymbol)
  //if exists, just returns it, o.w. makes it; trick to know the base ULAMTYPE
  UTI CompilerState::makeUlamType(Token typeTok, s32 bitsize, s32 arraysize, UTI classinstanceidx)
  {
    //type names begin with capital letter..and the rest can be either
    u32 typeNameId = getTokenAsATypeNameId(typeTok); //Foo, Int, etc

    //can't be a typedef!! get's the wrong name for type key; use key as arg
    UTI tmputi;
    assert(!getUlamTypeByTypedefName(typeTok.m_dataindex, tmputi));

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
	assert(aDefinedUTI(key,uti));

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
		//save scalar in key
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

	incrementUnknownKeyUTICounter(key);

	std::pair<std::map<UlamKeyTypeSignature, UTI, less_than_key>::iterator, bool> ret;
	ret = m_keyToaUTI.insert(std::pair<UlamKeyTypeSignature,UTI>(key,uti)); //just one!
	assert(isDefined(key, ut));
      }
    else
      assert(aDefinedUTI(key,uti));

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

  bool CompilerState::aDefinedUTI(UlamKeyTypeSignature key, UTI& foundUTI)
  {
    bool rtnBool= false;

    std::map<UlamKeyTypeSignature, UTI, less_than_key>::iterator it = m_keyToaUTI.find(key);

    if(it != m_keyToaUTI.end())
      {
	assert(key == it->first);
	foundUTI = it->second;
	rtnBool = true;
      }
    return rtnBool;
  } //aDefinedUTI

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

  void CompilerState::incrementUnknownKeyUTICounter(UlamKeyTypeSignature key)
  {
    if(key.getUlamKeyTypeSignatureBitSize() == UNKNOWNSIZE || key.getUlamKeyTypeSignatureArraySize() == UNKNOWNSIZE)
      {
	std::map<UlamKeyTypeSignature, u32, less_than_key>::iterator it = m_unknownKeyUTICounter.find(key);
	if(it != m_unknownKeyUTICounter.end())
	  {
	    assert(key == it->first);
	    it->second++;
	  }
	else
	  {
	    m_unknownKeyUTICounter.insert(std::pair<UlamKeyTypeSignature,u32>(key,1));
	  }
      }
  } //incrementUnknownKeyUTICounter

  u32 CompilerState::decrementUnknownKeyUTICounter(UlamKeyTypeSignature key)
  {
    std::map<UlamKeyTypeSignature, u32, less_than_key>::iterator it = m_unknownKeyUTICounter.find(key);
    u32 count = 0;
    if(it != m_unknownKeyUTICounter.end())
      {
	assert(key == it->first);
	it->second--;
	count = it->second;
	if(count == 0)
	  m_unknownKeyUTICounter.erase(it);
      }
    return count;
  } //decrementUnknownKeyUTICounter

  u32 CompilerState::findUnknownKeyUTICounter(UlamKeyTypeSignature key)
  {
    std::map<UlamKeyTypeSignature, u32, less_than_key>::iterator it = m_unknownKeyUTICounter.find(key);
    u32 count = 0;
    if(it != m_unknownKeyUTICounter.end())
      {
	assert(key == it->first);
	count = it->second;
      }
    return count;
  } //findUnknownKeyUTICounter

  //used to update Class' calculated bit size (setBitSize)
  bool CompilerState::deleteUlamKeyTypeSignature(UlamKeyTypeSignature key)
  {
    bool rtnBool= false;
    if(decrementUnknownKeyUTICounter(key) == 0)
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

  //used to update Class' calculated bit size (setBitSize)
  bool CompilerState::updateUlamKeyTypeSignatureToaUTI(UlamKeyTypeSignature oldkey, UlamKeyTypeSignature newkey)
  {
    bool rtnBool= false;
    UTI uti;

    //skip happily if no others use the old key; new key was added by makeulamtype
    u32 counter = findUnknownKeyUTICounter(oldkey);
    std::map<UlamKeyTypeSignature, UTI, less_than_key>::iterator it = m_keyToaUTI.find(oldkey);
    if(it != m_keyToaUTI.end())
      {
	assert(oldkey == it->first);
	uti = it->second;
	if(counter == 0)
	  m_keyToaUTI.erase(it);
	rtnBool = true;
      }

    //insert new key to same UTI.
    if(rtnBool)
      {
	std::pair<std::map<UlamKeyTypeSignature, UTI, less_than_key>::iterator, bool> ret;
	ret = m_keyToaUTI.insert(std::pair<UlamKeyTypeSignature,UTI>(newkey,uti)); //just one!
	bool notdup = ret.second; //false if already existed, i.e. not added
	if(!notdup)
	  {
	    std::ostringstream msg;
	    msg << "Updated Key to A UTI: " << getUlamTypeNameByIndex(uti).c_str();
	    msg << " (UTI" << uti << ")";
	    MSG2("", msg.str().c_str(), DEBUG);
	  }
      }
    return rtnBool;
  } //updateUlamKeyTypeSignatureToaUTI

  bool CompilerState::mappedIncompleteUTI(UTI cuti, UTI auti, UTI& mappedUTI)
  {
    SymbolClass * csym = NULL;
    assert(alreadyDefinedSymbolClass(cuti, csym));
    return csym->hasMappedUTI(auti, mappedUTI);
  }

  //called by Symbol's copy constructor with ref's 'incomplete' uti
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

    //first time we've seen this 'incomplete' UTI for this class instance (being fully instantiated):
    //get a new UTI and add to cnsym's map for this instance in case we see it again;
    //Later, also update all its resolver's 'subtree' table references; For classes with
    //pending args, make a copy of the stub including its resolver with pending args, so
    //pending args can be resolved XXXX within the context of this class instance (e.g. dependent on
    //instances arg values which makes it different than others', like "self").
    //Context dependent pending args are resolved before they are added to the resolver's pending args.
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
      }
    return newuti;
  }//mapIncompleteUTIForCurrentClassInstance

  void CompilerState::linkConstantExpression(UTI uti, NodeTypeBitsize * ceNode)
  {
    if(ceNode)
      {
	SymbolClassName * cnsym = NULL;
	assert(alreadyDefinedSymbolClassName(getCompileThisId(), cnsym));
	cnsym->linkUnknownBitsizeConstantExpression(uti, ceNode);
      }
  } //linkConstantExpression (bitsize)

  void CompilerState::cloneAndLinkConstantExpression(UTI fromuti, UTI touti)
  {
    SymbolClassName * cnsym = NULL;
    assert(alreadyDefinedSymbolClassName(getCompileThisId(), cnsym));
    cnsym->linkUnknownBitsizeConstantExpression(fromuti, touti);
  } //linkConstantExpression (bitsize in decllist)

  void CompilerState::linkConstantExpression(UTI uti, NodeSquareBracket * ceNode)
  {
    if(ceNode)
      {
	SymbolClassName * cnsym = NULL;
	assert(alreadyDefinedSymbolClassName(getCompileThisId(), cnsym));
	cnsym->linkUnknownArraysizeConstantExpression(uti, ceNode);
      }
  } //linkConstantExpression (arraysize)

  void CompilerState::linkConstantExpression(NodeConstantDef * ceNode)
  {
    if(ceNode)
      {
	SymbolClassName * cnsym = NULL;
	assert(alreadyDefinedSymbolClassName(getCompileThisId(), cnsym));
	cnsym->linkUnknownNamedConstantExpression(ceNode);
      }
  } //linkConstantExpression (named constant)

  void CompilerState::linkUnknownTypedefFromAnotherClass(UTI tduti, UTI stubuti)
  {
    SymbolClass * csym = NULL;
    assert(alreadyDefinedSymbolClass(getCompileThisIdx(), csym));
    csym->linkTypedefFromAnotherClass(tduti, stubuti); //directly into the SC
  } //linkUnknownTypedefFromAnotherClass

  void CompilerState::constantFoldIncompleteUTI(UTI auti)
  {
    SymbolClassName * cnsym = NULL;
    if(!alreadyDefinedSymbolClassName(getCompileThisId(), cnsym))
      {
	std::string debugme = getClassContextAsStringForDebugging();
	assert(0); //forgot a pushClassContext somewhere!
      }
    cnsym->constantFoldIncompleteUTIOfClassInstance(getCompileThisIdx(), auti);
  } //constantFoldIncompleteUTI

  bool CompilerState::constantFoldPendingArgs(UTI cuti)
  {
    UlamType * cut = getUlamTypeByIndex(cuti);
    UlamKeyTypeSignature ckey = cut->getUlamKeyTypeSignature();

    SymbolClassName * cnsym = NULL; //could be a different class than being compiled
    assert(alreadyDefinedSymbolClassName(ckey.getUlamKeyTypeSignatureNameId(), cnsym));
    if(cnsym->isClassTemplate())
      return ((SymbolClassNameTemplate *) cnsym)->constantFoldClassArgumentsInAStubClassInstance(cuti);
    return true; //ok
  } //constantFoldPendingArgsInCurrentContext

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
    //is this name already a typedef for a complex type?
    if(getUlamTypeByTypedefName(tok.m_dataindex, ut))
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
	//SymbolClassName * cnsym = NULL;
	//if(alreadyDefinedSymbolClassName(tok.m_dataindex, cnsym))
	bUT = Class;
      }
    return bUT;
  } //getBaseTypeFromToken

  UTI CompilerState::getUlamTypeFromToken(Token tok, s32 typebitsize, s32 arraysize)
  {
    UTI uti = Nav;
    //is this name already a typedef for a complex type?
    if(!getUlamTypeByTypedefName(tok.m_dataindex, uti))
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
	      } //else  or make one if doesn't exist yet, while parsing --- do we do this anymore ???
	  }
      }
    return uti;
  } //getUlamTypeFromToken

  //new version! uses indexes
  bool CompilerState::getUlamTypeByTypedefName(u32 nameIdx, UTI & rtnType)
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
    ULAMTYPE bUT = ut->getUlamTypeEnum();

    UlamKeyTypeSignature keyOfArg = ut->getUlamKeyTypeSignature();

    if(bUT == Class)
      {
	return keyOfArg.getUlamKeyTypeSignatureClassInstanceIdx();
      }

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

  bool CompilerState::isComplete(UTI utArg)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    //for arrays, check if scalar is complete
    if(!ut->isComplete() && !ut->isScalar())
      {
	UTI scalarUTI = ut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureClassInstanceIdx();
	assert(isScalar(scalarUTI));
	return isComplete(scalarUTI);
      }
    return ut->isComplete();
  } //isComplete

  //this may go away..
  //updates key. we can do this now that UTI is used and the UlamType * isn't saved
  void CompilerState::setBitSize(UTI utArg, s32 bits)
  {
    return setUTISizes(utArg, bits, getArraySize(utArg));  //keep current arraysize
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
    deleteUlamKeyTypeSignature(key);

    UlamType * newut = NULL;
    //assert(!isDefined(key, newut) && !newut);

    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, Class);
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType *>(newkey,newut));
	((UlamTypeClass *) newut)->setUlamClass(classtype); //restore from original ut

	if(isCustomArray)
	  ((UlamTypeClass *) newut)->setCustomArrayType(caType);

	incrementUnknownKeyUTICounter(newkey);  //here???
      }
    m_indexToUlamKey[utArg] = newkey;

    {
      std::ostringstream msg;
      msg << "Sizes SET for Class: " << newut->getUlamTypeName().c_str() << " (UTI" << utArg << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }

    assert(updateUlamKeyTypeSignatureToaUTI(key,newkey));
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
    deleteUlamKeyTypeSignature(key1);
    m_indexToUlamKey[olduti] = key2;
    incrementUnknownKeyUTICounter(key2);
    {
      std::ostringstream msg;
      msg << "MERGED keys for duplicate Class (UTI" << olduti << ") WITH: " << ut2->getUlamTypeName().c_str() << " (UTI" << cuti << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
  } //mergeClassUTI

  void CompilerState::setSizesOfNonClass(UTI utArg, s32 bitsize, s32 arraysize)
  {
    UlamType * ut = getUlamTypeByIndex(utArg);
    ULAMTYPE bUT = ut->getUlamTypeEnum();
    ULAMCLASSTYPE classtype = ut->getUlamClass();

    assert(classtype == UC_NOTACLASS);

    if(ut->isComplete())
      return;  //nothing to do

    //disallow zero-sized primitives (no such thing as a BitVector<0u>)
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
    if(key.getUlamKeyTypeSignatureBitSize() == 0 || bitsize == 0)
      {
	std::ostringstream msg;
	msg << "Invalid zero sizes to set for nonClass: " << ut->getUlamTypeName().c_str();
	msg << "> (UTI" << utArg << ")";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), ERR);
	return;
      }

    //continue with valid number of bits
    UlamKeyTypeSignature newkey = UlamKeyTypeSignature(key.getUlamKeyTypeSignatureNameId(), bitsize, arraysize);

    if(key == newkey)
      return;

    //remove old key from map, if no longer pointed to by any UTIs
    deleteUlamKeyTypeSignature(key);

    UlamType * newut = NULL;
    //assert(!isDefined(key, newut) && !newut);

    if(!isDefined(newkey, newut))
      {
	newut = createUlamType(newkey, bUT);
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature, UlamType*>(newkey,newut));
	incrementUnknownKeyUTICounter(newkey);
      }

    m_indexToUlamKey[utArg] = newkey;

    {
      std::ostringstream msg;
      msg << "Sizes set for nonClass: " << newut->getUlamTypeName().c_str() << " (UTI" << utArg << ")";
      MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
    }
    assert(updateUlamKeyTypeSignatureToaUTI(key,newkey));
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
  void CompilerState::addIncompleteClassSymbolToProgramTable(u32 dataindex, SymbolClassName * & symptr)
  {
    assert(!alreadyDefinedSymbolClassName(dataindex,symptr));

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE);  //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class);  //**gets next unknown uti type

    //symbol ownership goes to the programDefST; distinguish between template and regular classes here:
    symptr = new SymbolClassName(dataindex, cuti, NULL, *this);  //NodeBlockClass is NULL for now
    m_programDefST.addToTable(dataindex, symptr);
  } //addIncompleteClassSymbolToProgramTable

  //temporary UlamType which will be updated during type labeling.
  void CompilerState::addIncompleteClassSymbolToProgramTable(u32 dataindex, SymbolClassNameTemplate * & symptr)
  {
    assert(!alreadyDefinedSymbolClassNameTemplate(dataindex,symptr));

    UlamKeyTypeSignature key(dataindex, UNKNOWNSIZE);  //"-2" and scalar default
    UTI cuti = makeUlamType(key, Class);  //**gets next unknown uti type

    //symbol ownership goes to the programDefST; distinguish between template and regular classes here:
    symptr = new SymbolClassNameTemplate(dataindex, cuti, NULL, *this);  //NodeBlockClass is NULL for now
    m_programDefST.addToTable(dataindex, symptr);
  } //addIncompleteClassSymbolToProgramTable

  bool CompilerState::completeIncompleteClassSymbol(UTI incomplete)
  {
    bool rtnB = false;
    SymbolClass * csym = NULL;
    UlamType * ict = getUlamTypeByIndex(incomplete);
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
		msg << "Sizes still unknown for Class Instance: " << ict->getUlamTypeName().c_str();
		msg << "(UTI" << but << ")";
		MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	      }
	    else
	      rtnB = true;
	  }
	else //else uc_incomplete
	  {
	    std::ostringstream msg;
	    msg << "Sizes still unknown for Class Instance: " << ict->getUlamTypeName().c_str();
	    msg << "(UTI" << incomplete << ") - Incomplete";
	    MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Sizes still unknown for Class Instance: " << ict->getUlamTypeName().c_str();
	msg << "(UTI" << incomplete << ") - NOT YET DEFINED";
	MSG2(getFullLocationAsString(m_locOfNextLineText).c_str(), msg.str().c_str(), DEBUG);
      }
    return rtnB;
  } //completeIncompleteClassSymbol

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
	blockNode = blockNode->getPreviousBlockPointer();  //traverse the chain
      }

    //data member variables in class block; function symbols are linked to their
    //function def block; check function data members separately.
    if(!brtn)
      {
	brtn = isFuncIdInClassScope(dataindex, symptr);
      }
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
    return m_pool.getDataAsString(loc.getFullPathIndex());
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
		msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str();
		msg << "''s Return type's: " << getUlamTypeNameByIndex(it).c_str();
		msg << " base type: <" << UlamType::getUlamTypeEnumAsString(itBUT);
		msg << ">, does not match resulting type's ";
		msg << getUlamTypeNameByIndex(rType).c_str() << " base type: <";
		msg << UlamType::getUlamTypeEnumAsString(rBUT) << ">";
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
      {
	fp->write(m_indentedSpaceLevel);
      }
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
	f << getUlamTypeByIndex(cuti)->getUlamTypeUPrefix().c_str() << getDataAsStringMangled(getCompileThisId()).c_str() << DigitCount(numParams, BASE10) << numParams << "_main.cpp";
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
	mangled << "<" << getTotalBitSize(uti) << ">";  //???
      }
    return mangled.str();
  } //getBitSizeTemplateString

  //unfortunately, the uti did not reveal a Class symbol; already down to primitive types
  //for casting.
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
	//error!
	assert(0);
      };
    return valAtIdx;  //return as-is
  } //getPtrTarget

  //general purpose store
  void CompilerState::assignValue(UlamValue lptr, UlamValue ruv)
  {
    assert(lptr.getUlamValueTypeIdx() == Ptr);

    //handle UAtom assignment as a singleton (not array values)
    if(ruv.getUlamValueTypeIdx() == Ptr && (ruv.getPtrTargetType() != UAtom || lptr.getPtrTargetType() != UAtom))
      {
	return assignArrayValues(lptr, ruv);
      }
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
    assert(lptr.getPtrTargetType() == rptr.getPtrTargetType());
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
	tuti = nextrptr.getPtrTargetType(); //update type

	for(u32 i = 0; i < arraysize; i++)
	  {
	    UlamValue atval = getPtrTarget(nextrptr);

	    //redo what getPtrTarget use to do, when types didn't match due to
	    //an element/quark or a requested scalar of an arraytype
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
  } //assignArrayValues

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

    m_locOfNextLineText = loc;  //during parsing here (see NodeStatements)
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

    std::ostringstream tmpVar;  //into
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
    std::ostringstream labelname;  //into
    labelname << "Ul_endcontrolloop_" << DigitCount(num, BASE10) << num;;
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

    //beware the classblock is the only block with different node no in SHALLOW instances
    if(getCurrentBlock()->getNodeNo() == n && getClassBlock()->getNodeType() == getCompileThisIdx())
      return getCurrentBlock(); //avoid chix-n-egg with functiondefs

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
  }

  NNO CompilerState::getCurrentBlockNo()
  {
    ClassContext cc;
    if(m_classContextStack.getCurrentClassContext(cc) && getCurrentBlock())
      return getCurrentBlock()->getNodeNo();
    return 0; //genesis of class symbol
  }

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
    assert(m_classContextStack.getCurrentClassContext(cc));
    return cc.useMemberBlock();
  }

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

} //end MFM
