#include "Resolver.h"
#include "CompilerState.h"
#include "SymbolClass.h"

namespace MFM {

  Resolver::Resolver(UTI instance, CompilerState& state) : m_state(state), m_classUTI(instance), m_classContextUTIForPendingArgValues(m_state.getCompileThisIdx()), m_classContextUTIForPendingArgTypes(instance) /*default*/ {}

  Resolver::~Resolver()
  {
    clearLeftoverSubtrees();
  }

  void Resolver::clearLeftoverSubtrees()
  {
    clearLeftoverNonreadyClassArgSubtrees();
    clearLeftoverUnknownTypeTokens();
    m_mapUTItoUTI.clear();
  } //clearLeftoverSubtrees

  void Resolver::clearLeftoverNonreadyClassArgSubtrees()
  {
    s32 nonreadyG = m_nonreadyClassArgSubtrees.size();
    if(nonreadyG > 0)
      {
	std::ostringstream msg;
	msg << "Class Instances with non-ready argument constant subtrees cleared: ";
	msg << nonreadyG;
	MSG("",msg.str().c_str(),DEBUG);

	std::vector<NodeConstantDef *>::iterator vit = m_nonreadyClassArgSubtrees.begin();
	while(vit != m_nonreadyClassArgSubtrees.end())
	  {
	    *vit = NULL; //NodeBlockClass is new owner!
	    vit++;
	  }
      }
    m_nonreadyClassArgSubtrees.clear();
  } //clearLeftoverNonreadyClassArgSubtrees

  void Resolver::clearLeftoverUnknownTypeTokens()
  {
    s32 unknowns = m_unknownTypeTokens.size();
    if(unknowns > 0)
      {
	std::ostringstream msg;
	msg << "Class Regular/Template with unknown Types cleared: ";
	msg << unknowns;
	MSG("", msg.str().c_str(),DEBUG);
      }
    m_unknownTypeTokens.clear();
  } //clearLeftoverUnknownTypeTokens

  void Resolver::addUnknownTypeToken(const Token& tok, UTI huti)
  {
    m_unknownTypeTokens.insert(std::pair<UTI, Token> (huti, tok));
  }

  Token Resolver::removeKnownTypeToken(UTI huti)
  {
    assert(!m_unknownTypeTokens.empty());

    Token rtnTok;
    std::map<UTI, Token>::iterator mit = m_unknownTypeTokens.begin();
    mit = m_unknownTypeTokens.find(huti);
    assert(mit != m_unknownTypeTokens.end());
      {
	rtnTok = mit->second;
	m_unknownTypeTokens.erase(mit);
      }
    return rtnTok; //in case of error? use hasUnknownTypeToken first
  } //removeUnknownTypeToken

  bool Resolver::hasUnknownTypeToken(UTI huti)
  {
    bool rtnb = false;
    if(m_unknownTypeTokens.empty())
      return false;

    std::map<UTI, Token>::iterator mit = m_unknownTypeTokens.find(huti);
    if(mit != m_unknownTypeTokens.end())
      {
	rtnb = true;
      }
    return rtnb;
  } //hasUnknownTypeToken

  bool Resolver::getUnknownTypeToken(UTI huti, Token & tok)
  {
    bool rtnb = false;
    if(m_unknownTypeTokens.empty())
      return false;

    std::map<UTI, Token>::iterator mit = m_unknownTypeTokens.find(huti);
    if(mit != m_unknownTypeTokens.end())
      {
	tok = mit->second;
	rtnb = true;
      }
    return rtnb;
  }

  bool Resolver::statusUnknownType(UTI huti, SymbolClass * csym)
  {
    bool aok = false; //not found
    // context already set by caller
    std::map<UTI, Token>::iterator mit = m_unknownTypeTokens.find(huti);
    if(mit != m_unknownTypeTokens.end())
      {
	Token tok = mit->second;
	UTI huti = mit->first;

	//check if still Hzy; true if resolved
	if((aok = checkUnknownTypeToResolve(huti, tok)))
	  removeKnownTypeToken(huti);
	else if(csym && (aok = checkUnknownTypeAsClassArgument(huti, tok, csym)))
	  removeKnownTypeToken(huti); //t41216
	//else
      }
    return aok;
  } //statusUnknownType

  bool Resolver::checkUnknownTypeToResolve(UTI huti, const Token& tok)
  {
    bool aok = false;
    ULAMTYPE etyp = m_state.getBaseTypeFromToken(tok);
    if((etyp == Hzy) || (etyp == Holder))
      return false;

    if(m_state.isComplete(huti))
      return true; //short-circuit, known (t41287,8)

    u32 tokid = m_state.getTokenDataAsStringId(tok);
    UTI kuti = Nav;
    if(etyp == Class)
      {
	SymbolClassName * cnsym = NULL; //no way a template or stub
	if(!m_state.alreadyDefinedSymbolClassName(tokid, cnsym))
	  {
	    SymbolClass * csym = NULL;
	    if(m_state.alreadyDefinedSymbolClassAsHolder(huti, csym))
	      {
		aok = false; //still a holder
		UTI mappedUTI;
		if(m_state.findaUTIAlias(huti, mappedUTI))
		  {
		    if(m_state.alreadyDefinedSymbolClass(mappedUTI, csym))
		      {
			u32 cid = csym->getId();
			AssertBool isDefined = m_state.alreadyDefinedSymbolClassName(cid, cnsym);
			assert(isDefined);
		      }

		    kuti = mappedUTI; //t3862
		  }
	      }
	    else if(m_state.alreadyDefinedSymbolClass(huti, csym))
	      {
		//e.g. t41287,8  array typedef finds its scalar class;
		// careful not to clobber the array UTI with cleanup.
		u32 cid = csym->getId();
		AssertBool isDefined = m_state.alreadyDefinedSymbolClassName(cid, cnsym);
		assert(isDefined);
		aok = m_state.isHolder(cnsym->getUlamTypeIdx()) ? false : true;
		aok &= m_state.isScalar(huti);
	      }
	    //else
	  }
	else
	  {
	    UTI cuti = cnsym->getUlamTypeIdx();
	    if(m_state.getUlamTypeByIndex(cuti)->getUlamClassType() == UC_UNSEEN)
	      aok = false; //still unseen
	    else
	      aok = true; //not missing, seen!
	  }

	if(aok)
	  {
	    assert(cnsym);
	    if(cnsym->isClassTemplate())
	      {
		SymbolClass * csym = NULL;
		if(m_state.alreadyDefinedSymbolClass(huti, csym))
		  kuti = csym->getUlamTypeIdx(); //perhaps an alias
		else
		  {
		    std::ostringstream msg;
		    msg << "Class with parameters seen with the same name: ";
		    msg << m_state.m_pool.getDataAsString(cnsym->getId()).c_str();
		    MSG(m_state.getFullLocationAsString(tok.m_locator).c_str(), msg.str().c_str(), ERR); //No corresponding Nav Node for this ERR (e.g. error/t3644)
		    //aok = false; continue so no more than one error for same problem
		    kuti = cnsym->getUlamTypeIdx();
		  }
	      }
	    else
	      kuti = cnsym->getUlamTypeIdx();
	  }
      } //end class type
	//else

    if(!aok)
      {
	//a typedef (e.g. t3379, 3381)
	UTI tmpscalar = Nouti;
	if(m_state.getUlamTypeByTypedefName(tokid, kuti, tmpscalar))
	  aok = !m_state.isHolder(kuti);
      }

    if(aok)
      {
	assert(!m_state.isHolder(kuti));
	m_state.cleanupExistingHolder(huti, kuti);
      }
    return aok;
  } //checkUnknownTypeToResolve

  bool Resolver::checkUnknownTypeAsClassArgument(UTI huti, const Token& tok, SymbolClass * csym)
  {
    assert(csym);
    if(!(csym->isStub() && (tok.m_type == TOK_IDENTIFIER)))
      return false;

    bool aok = false;
    //i.e. not a type tok, so perhaps a class argument (t41216)
    // let's see if the template has a clue regarding its type "family"
    SymbolClassNameTemplate * templateparent = csym->getParentClassTemplate();
    assert(templateparent);
    UTI tuti = templateparent->getUlamTypeIdx();
    if(!m_state.isASeenClass(tuti))
      return false;

    NodeBlockClass * templateclassblock = templateparent->getClassBlockNode();
    assert(templateclassblock);
    m_state.pushClassContext(tuti, templateclassblock, templateclassblock, false, NULL);

    u32 tokid = m_state.getTokenDataAsStringId(tok);
    Symbol * argSym = NULL;
    bool hazykin = false;
    if(m_state.alreadyDefinedSymbol(tokid, argSym, hazykin))
      {
	m_state.popClassContext(); //restore before another check call

	UTI auti = argSym->getUlamTypeIdx();
	if(!m_state.isHolder(auti))
	  {
	    Token argtok;
	    UlamType * aut = m_state.getUlamTypeByIndex(auti);
	    if(m_state.isAClass(auti))
	      {
		u32 aid = aut->getUlamTypeNameId();
		argtok.init(TOK_IDENTIFIER, tok.m_locator, aid);
	      }
	    else
	      {
		std::string aname = aut->getUlamTypeNameOnly();
		argtok.init(Token::getTokenTypeFromString(aname.c_str()), tok.m_locator, 0);
	      }
	    aok = checkUnknownTypeToResolve(huti, argtok);
	  }
      }
    else
      m_state.popClassContext(); //restore
    return aok;
  } //checkUnknownTypeAsClassArgument

  bool Resolver::statusAnyUnknownTypeNames()
  {
    bool aok = true;
    std::vector<UTI> knownList;
    // context already set by caller
    std::map<UTI, Token>::iterator mit = m_unknownTypeTokens.begin();
    while(mit != m_unknownTypeTokens.end())
      {
	Token tok = mit->second;
	UTI huti = mit->first;
	//check if still Hzy
	if(checkUnknownTypeToResolve(huti, tok))
	  knownList.push_back(huti); //resolved
	else
	  {
	    std::ostringstream msg;
	    msg << "Undetermined Type: <";
	    msg << m_state.getTokenDataAsString(tok) << ">; ";
	    msg << "Suggest 'use ";
	    msg << m_state.getTokenDataAsString(tok) << ";' if it's a class";
	    msg << ", otherwise a typedef is needed";
	    MSG(m_state.getTokenLocationAsString(&tok).c_str(), msg.str().c_str(), WAIT);
	    aok = false;
	  }
	mit++;
      }

    //clean up any known Types from unknownTypeTokens
    std::vector<UTI>::iterator vit = knownList.begin();
    while(vit != knownList.end())
      {
	removeKnownTypeToken(*vit);
	vit++;
      }
    assert(m_unknownTypeTokens.empty() == aok);
    return aok; //false if any remain; true if empty
  } //statusAnyUnknownTypeNames

  u32 Resolver::reportAnyUnknownTypeNames()
  {
    // context already set by caller
    std::map<UTI, Token>::iterator mit = m_unknownTypeTokens.begin();
    while(mit != m_unknownTypeTokens.end())
      {
	Token tok = mit->second;
	std::ostringstream msg;
	msg << "Undetermined Type: <";
	msg << m_state.getTokenDataAsString(tok) << ">; ";
	msg << "Suggest 'use ";
	msg << m_state.getTokenDataAsString(tok) << ";' if it's a class";
	msg << ", otherwise a typedef is needed in ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_classUTI).c_str();
	MSG(m_state.getTokenLocationAsString(&tok).c_str(), msg.str().c_str(), ERR);
	mit++;
      }
    return m_unknownTypeTokens.size();
  } //reportAnyUnknownTypeNames

  bool Resolver::assignClassArgValuesInStubCopy()
  {
    bool aok = true;
    // context already set by caller
    std::vector<NodeConstantDef *>::iterator vit = m_nonreadyClassArgSubtrees.begin();
    while(vit != m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;
	if(ceNode)
	  aok &= ceNode->assignClassArgValueInStubCopy();
	vit++;
      } //while thru vector of incomplete args only
    return aok;
  } //assignClassArgValuesInStubCopy

  u32 Resolver::countNonreadyClassArgs()
  {
    return m_nonreadyClassArgSubtrees.size();
  }

  bool Resolver::statusNonreadyClassArguments(SymbolClass * stubcsym)
  {
    bool rtnstat = true; //ok, empty
    if(!m_nonreadyClassArgSubtrees.empty())
      {
	rtnstat = false;

	u32 lostsize = m_nonreadyClassArgSubtrees.size();

	std::ostringstream msg;
	msg << "Found " << lostsize << " nonready arguments for class instance: ";
	msg << " (UTI" << m_classUTI << ") " << m_state.getUlamTypeNameByIndex(m_classUTI).c_str();

	msg << " trying to update now";
	MSG("", msg.str().c_str(), DEBUG);

	rtnstat = constantFoldNonreadyClassArgs(stubcsym); //forgot to update rtnstat?
      }
    return rtnstat;
  } //statusNonreadyClassArguments

  bool Resolver::constantFoldNonreadyClassArgs(SymbolClass * stubcsym)
  {
    bool rtnb = true;
    UTI context = getContextForPendingArgValues();
    m_state.pushClassOrLocalContextAndDontUseMemberBlock(context);

    m_state.m_pendingArgStubContext = m_classUTI; //set for folding surgery
    m_state.m_pendingArgTypeStubContext = getContextForPendingArgTypes(); //set for resolving types

    bool defaultval = false;
    bool pushedtemplate = false;
    assert(stubcsym);
    NodeBlockClass * stubclassblock = stubcsym->getClassBlockNode();
    assert(stubclassblock);
    Locator saveStubLoc = stubclassblock->getNodeLocation();

    std::vector<NodeConstantDef *> leftCArgs;
    std::vector<NodeConstantDef *>::iterator vit = m_nonreadyClassArgSubtrees.begin();
    while(vit != m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;
	if(ceNode)
	  {
	    //use default value if there is one AND there isn't a constant expression (t3893)
	    defaultval = ceNode->hasDefaultSymbolValue() && !ceNode->hasConstantExpr();

	    //OMG! if this was a default value for class arg, t3891,
	    // we want to use the class stub/template as the 'context' rather than where the
	    // stub was declared.
	    if(defaultval && !pushedtemplate)
	      {
		assert(stubcsym);
		SymbolClassNameTemplate * templateparent = stubcsym->getParentClassTemplate();
		assert(templateparent);
		NodeBlockClass * templateclassblock = templateparent->getClassBlockNode();
		//temporarily change stub loc, in case of local filescope, incl arg/params
		stubclassblock->resetNodeLocations(templateclassblock->getNodeLocation());

		m_state.pushClassContext(m_classUTI, stubclassblock, stubclassblock, false, NULL);
		pushedtemplate = true;
	      }
	    assert(defaultval == pushedtemplate); //once a default, always a default

	    UTI uti = ceNode->checkAndLabelType();
	    if(m_state.okUTItoContinue(uti)) //i.e. ready
	      {
		m_state.addCompleteUlamTypeToContextBlockSet(uti, stubclassblock); //t3895
		*vit = NULL; //NodeBlockClass is the owner now.
	      }
	    else
	      leftCArgs.push_back(ceNode);
	  }
	vit++;
      } //while thru vector of incomplete args only

    if(pushedtemplate)
      {
	m_state.popClassContext(); //no longer need stub context
	stubclassblock->setNodeLocation(saveStubLoc);
      }

    m_state.m_pendingArgStubContext = Nouti; //clear flag
    m_state.m_pendingArgTypeStubContext = Nouti; //clear flag
    m_state.popClassContext(); //restore previous context

    //clean up, replace vector with vector of those still unresolved
    m_nonreadyClassArgSubtrees.clear();
    if(!leftCArgs.empty())
      {
	m_nonreadyClassArgSubtrees = leftCArgs; //replace
	rtnb = false;
      }
    return rtnb;
  } //constantFoldNonreadyClassArgs

  //called while parsing this stub instance use;
  void Resolver::linkConstantExpressionForPendingArg(NodeConstantDef * ceNode)
  {
    if(ceNode)
      m_nonreadyClassArgSubtrees.push_back(ceNode);
  } //linkConstantExpressionForPendingArg

  bool Resolver::pendingClassArgumentsForClassInstance()
  {
    return !m_nonreadyClassArgSubtrees.empty();
  } //pendingClassArgumentsForClassInstance

  void Resolver::setContextForPendingArgValues(UTI context)
  {
    m_classContextUTIForPendingArgValues = context;
  }

  UTI Resolver::getContextForPendingArgValues()
  {
    return m_classContextUTIForPendingArgValues;
  }

  void Resolver::setContextForPendingArgTypes(UTI context)
  {
    m_classContextUTIForPendingArgTypes = context;
  }

  UTI Resolver::getContextForPendingArgTypes()
  {
    return m_classContextUTIForPendingArgTypes;
  }

  bool Resolver::mapUTItoUTI(UTI fmuti, UTI touti)
  {
    //if fm already mapped in full instance, (e.g. unknown typedeffromanotherclass)
    //use its mapped uti as the key to touti instead
    UTI mappedfmuti = fmuti;
    if(findMappedUTI(fmuti, mappedfmuti))
      {
	std::ostringstream msg;
	msg << "Substituting previously mapped UTI" << mappedfmuti;
	msg << " for the from UTI" << fmuti << ", while mapping to: " << touti;
	msg << " in class " << m_state.getUlamTypeNameBriefByIndex(m_classUTI).c_str();
	MSG("",msg.str().c_str(),DEBUG);
	fmuti = mappedfmuti;
      }

    std::pair<std::map<UTI, UTI>::iterator, bool> ret;
    ret = m_mapUTItoUTI.insert(std::pair<UTI, UTI>(fmuti,touti));
    bool notdup = ret.second; //false if already existed, i.e. not added
    if(notdup)
      {
	//sanity check please..
	UTI checkuti;
	AssertBool isMapped = findMappedUTI(fmuti,checkuti);
	assert(isMapped);
	assert(checkuti == touti);
      }
  return notdup;
  } //mapUTItoUTI

  bool Resolver::findMappedUTI(UTI auti, UTI& mappedUTI)
  {
    if(m_mapUTItoUTI.empty()) return false;

    bool brtn = false;
    std::map<UTI, UTI>::iterator mit = m_mapUTItoUTI.find(auti);
    if(mit != m_mapUTItoUTI.end())
      {
	brtn = true;
	assert(mit->first == auti);
	mappedUTI = mit->second;
      }
    return brtn;
  } //findMappedUTI

  void Resolver::cloneUTImap(SymbolClass * csym)
  {
    std::map<UTI, UTI>::iterator mit = m_mapUTItoUTI.begin();
    while(mit != m_mapUTItoUTI.end())
      {
	UTI a = mit->first;
	UTI b = mit->second;
	csym->mapUTItoUTI(a, b);
	mit++;
      }
  } //cloneUTImap

  void Resolver::cloneUnknownTypesTokenMap(SymbolClass * csym)
  {
    std::map<UTI, Token>::iterator mit = m_unknownTypeTokens.begin();
    while(mit != m_unknownTypeTokens.end())
      {
	UTI huti = mit->first;
	Token tok = mit->second;
	UTI mappedUTI = huti;
	csym->hasMappedUTI(huti, mappedUTI);
	csym->addUnknownTypeTokenToClass(tok, mappedUTI);
	mit++;
      }
  } //cloneUnknownTypesTokenMap

} //MFM
