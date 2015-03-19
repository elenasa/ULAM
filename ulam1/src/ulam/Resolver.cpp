#include "Resolver.h"
#include "SymbolClass.h"
#include "CompilerState.h"

namespace MFM {

  Resolver::Resolver(UTI instance, CompilerState& state) : m_state(state), m_classUTI(instance), m_classContextUTIForPendingArgs(m_state.getCompileThisIdx()) /*default*/ {}

  Resolver::~Resolver()
  {
    clearLeftoverSubtrees();
  }

  void Resolver::clearLeftoverSubtrees()
  {
    clearLeftoverUnknownBitsizeSubtrees();
    clearLeftoverUnknownArraysizeSubtrees();
    clearLeftoverNonreadyNamedConstantSubtrees();
    clearLeftoverNonreadyClassArgSubtrees();
    clearLeftoverUnknownTypdedefsFromAnotherClass();
    m_mapUTItoUTI.clear();
  } //clearLeftoverSubtrees()

  void Resolver::clearLeftoverUnknownBitsizeSubtrees()
  {
    s32 unknownB = m_unknownBitsizeSubtrees.size();
    if(unknownB > 0)
      {
	std::ostringstream msg;
	msg << "Unknown bitsize subtrees cleared: " << unknownB;
	MSG("",msg.str().c_str(),DEBUG);

	std::map<UTI, NodeTypeBitsize *>::iterator itb = m_unknownBitsizeSubtrees.begin();
	while(itb != m_unknownBitsizeSubtrees.end())
	  {
	    NodeTypeBitsize * bitsizeNode = itb->second;
	    delete bitsizeNode;
	    itb->second = NULL;
	    itb++;
	  }
      }
    m_unknownBitsizeSubtrees.clear();
  } //clearLeftoverUnknownBitsizeSubtrees

  void Resolver::clearLeftoverUnknownArraysizeSubtrees()
  {
    s32 unknownA = m_unknownArraysizeSubtrees.size();
    if(unknownA > 0)
      {
	std::ostringstream msg;
	msg << "Unknown arraysize subtrees cleared: " << unknownA;
	MSG("",msg.str().c_str(),DEBUG);

	std::map<UTI, NodeSquareBracket *>::iterator ita = m_unknownArraysizeSubtrees.begin();
	while(ita != m_unknownArraysizeSubtrees.end())
	  {
	    NodeSquareBracket * arraysizeNode = ita->second;
	    delete arraysizeNode;
	    ita->second = NULL;
	    ita++;
	  }
      }
    m_unknownArraysizeSubtrees.clear();
  } //clearLeftoverUnknownArraysizeSubtrees

  void Resolver::clearLeftoverNonreadyNamedConstantSubtrees()
  {
    s32 nonreadyC = m_nonreadyNamedConstantSubtrees.size();
    if(nonreadyC > 0)
      {
	std::ostringstream msg;
	msg << "Nonready named constant subtrees cleared: " << nonreadyC;
	MSG("",msg.str().c_str(),DEBUG);
      }
    m_nonreadyNamedConstantSubtrees.clear();
  } //clearLeftoverNonreadyNamedConstantSubtrees

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
	    NodeConstantDef * ceNode = *vit;
	    delete ceNode;
	    *vit = NULL;
	    vit++;
	  }
      }
    m_nonreadyClassArgSubtrees.clear();
  } //clearLeftoverNonreadyClassArgSubtrees

  void Resolver::clearLeftoverUnknownTypdedefsFromAnotherClass()
  {
    s32 tdfromanotherC = m_unknownTypedefFromAnotherClass.size();
    if(tdfromanotherC > 0)
      {
	std::ostringstream msg;
	msg << "Class Instances with unknown typedefs from another class cleared: ";
	msg << tdfromanotherC;
	MSG("",msg.str().c_str(),DEBUG);
      }
    m_unknownTypedefFromAnotherClass.clear();
  } //clearLeftoverUnknownTypdedefsFromAnotherClass

  void Resolver::cloneTemplateResolver(SymbolClass * to)
  {
    // Type bitsize UNKNOW:
    {
      std::map<UTI, NodeTypeBitsize *>::iterator it = m_unknownBitsizeSubtrees.begin();
      while(it != m_unknownBitsizeSubtrees.end())
	{
	  UTI uti = it->first;
	  UTI mappedUTI = uti;
	  to->hasMappedUTI(uti, mappedUTI);
	  NodeTypeBitsize * ceNode = it->second;
	  NodeTypeBitsize * cloneNode = new NodeTypeBitsize(*ceNode);
	  to->linkConstantExpression(mappedUTI, cloneNode);
	  it++;
	}
    }

    // Arraysize UNKNOWN:
    {
      std::map<UTI, NodeSquareBracket *>::iterator it = m_unknownArraysizeSubtrees.begin();
      while(it != m_unknownArraysizeSubtrees.end())
	{
	  UTI uti = it->first;
	  UTI mappedUTI = uti;
	  to->hasMappedUTI(uti, mappedUTI);
	  NodeSquareBracket * ceNode = it->second;
	  NodeSquareBracket * cloneNode = new NodeSquareBracket(*ceNode);
	  to->linkConstantExpression(mappedUTI, cloneNode);
	  it++;
      }
    }

    //Named Constants
    {
      std::set<NodeConstantDef *>::iterator it = m_nonreadyNamedConstantSubtrees.begin();
      while(it != m_nonreadyNamedConstantSubtrees.end())
	{
	  NodeConstantDef * constNode = *it;
	  Symbol * sym;
	  if(constNode->getSymbolPtr(sym) && !((SymbolConstantValue *) sym)->isReady())
	    {
	      NodeConstantDef * cloneNode = new NodeConstantDef(*constNode);
	      to->linkConstantExpression(cloneNode);
	    }
	  it++;
	}
    }

    //typedef from another class
    {
      std::map<UTI, UTI>::iterator it = m_unknownTypedefFromAnotherClass.begin();
      while(it != m_unknownTypedefFromAnotherClass.end())
	{
	  UTI tduti = it->first;
	  UTI aclassuti = it->second;
	  // SHOULDN'T BE mapped at this point; wait until resolution
	  to->linkTypedefFromAnotherClass(tduti, aclassuti);
	  it++;
	}
    }
  } //cloneTemplateResolver

  NodeTypeBitsize * Resolver::findUnknownBitsizeUTI(UTI auti) const
  {
    std::map<UTI, NodeTypeBitsize *>::const_iterator it = m_unknownBitsizeSubtrees.find(auti);
    if(it != m_unknownBitsizeSubtrees.end())
      return it->second;
    return NULL;
  }

  NodeSquareBracket * Resolver::findUnknownArraysizeUTI(UTI auti) const
  {
    std::map<UTI, NodeSquareBracket *>::const_iterator it = m_unknownArraysizeSubtrees.find(auti);
    if(it != m_unknownArraysizeSubtrees.end())
      return it->second;
    return NULL;
  } //findUnknownArraysizeUTI

  bool Resolver::statusUnknownConstantExpressions()
  {
    bool sumbrtn = true;
    sumbrtn &= statusUnknownBitsizeUTI();
    sumbrtn &= statusUnknownArraysizeUTI();
    sumbrtn &= statusNonreadyNamedConstants();
    sumbrtn &= statusUnknownTypedefsFromAnotherClass();
    return sumbrtn;
  } //statusUnknownConstantExpressions

  void Resolver::constantFoldIncompleteUTI(UTI uti)
  {
    //should we short-circuit if a template class?
    s32 bitsize = m_state.getBitSize(uti);
    assert(bitsize >= UNKNOWNSIZE);
    s32 arraysize = m_state.getArraySize(uti);
    assert(arraysize >= UNKNOWNSIZE);
    bool bs = (bitsize != UNKNOWNSIZE || constantFoldUnknownBitsize(uti,bitsize));
    bool as = (arraysize != UNKNOWNSIZE || constantFoldUnknownArraysize(uti, arraysize));
    if(bs || as)
      m_state.setUTISizes(uti, bitsize, arraysize); //update UlamType
  } //constantFoldIncompleteUTI

  void Resolver::linkConstantExpression(UTI uti, NodeTypeBitsize * ceNode)
  {
    if(ceNode)
      {
	std::pair<std::map<UTI, NodeTypeBitsize *>::iterator, bool> ret;
	ret = m_unknownBitsizeSubtrees.insert(std::pair<UTI, NodeTypeBitsize *>(uti,ceNode));
	bool notdupi = ret.second; //false if already existed, i.e. not added
	if(!notdupi)
	  {
	    delete ceNode; //prevent leaks
	    ceNode = NULL;
	  }
      }
  } //linkConstantExpression

  void Resolver::cloneAndLinkConstantExpression(UTI fromtype, UTI totype)
  {
    std::map<UTI, NodeTypeBitsize *>::iterator it = m_unknownBitsizeSubtrees.find(fromtype);
    assert(it != m_unknownBitsizeSubtrees.end());
    assert(it->first == fromtype);
    NodeTypeBitsize * ceNode = it->second;
    NodeTypeBitsize * cloneSubtree = new NodeTypeBitsize(*ceNode); //any symbols will be null until c&l
    assert(cloneSubtree);
    linkConstantExpression(totype, cloneSubtree);
  } //linkConstantExpression (bitsize, decllist)

  bool Resolver::statusUnknownBitsizeUTI()
  {
    bool rtnstat = true; //ok, empty
    if(!m_unknownBitsizeSubtrees.empty())
      {
	std::vector<UTI> lostUTIs;
	u32 lostsize = m_unknownBitsizeSubtrees.size();

	std::ostringstream msg;
	msg << "Found non-empty unknown bitsize subtrees, of class <";
	msg << m_state.getUlamTypeNameByIndex(m_classUTI).c_str();
	msg << ">, size " << lostsize << ":";

	std::map<UTI, NodeTypeBitsize *>::iterator it = m_unknownBitsizeSubtrees.begin();
	while(it != m_unknownBitsizeSubtrees.end())
	  {
	    UTI auti = it->first;
	    msg << " (UTI" << auti << ") ";
	    msg << m_state.getUlamTypeNameByIndex(auti).c_str() << ",";
	    lostUTIs.push_back(auti);
	    it++;
	  }

	msg << " trying to update now";
	MSG("", msg.str().c_str(), DEBUG);
	rtnstat = false;

	assert(lostUTIs.size() == lostsize);
	while(!lostUTIs.empty())
	  {
	    UTI auti = lostUTIs.back();
	    constantFoldIncompleteUTI(auti);
	    lostUTIs.pop_back();
	  }
	lostUTIs.clear();
      }
    return rtnstat;
  } //statusUnknownBitsizeUTI

  bool Resolver::constantFoldUnknownBitsize(UTI auti, s32& bitsize)
  {
    bool rtnBool = true; //unfound
    std::map<UTI, NodeTypeBitsize *>::iterator it = m_unknownBitsizeSubtrees.find(auti);

    if(it != m_unknownBitsizeSubtrees.end())
      {
	assert(auti == it->first);
	NodeTypeBitsize * ceNode = it->second;
	assert(ceNode);
	rtnBool = ceNode->getTypeBitSizeInParen(bitsize, m_state.getUlamTypeByIndex(auti)->getUlamTypeEnum()); //eval
	if(rtnBool)
	  {
	    delete ceNode;
	    it->second = NULL;
	    m_unknownBitsizeSubtrees.erase(it);
	  }
      }
    return rtnBool;
  } //constantFoldUnknownBitsize

  void Resolver::linkConstantExpression(UTI uti, NodeSquareBracket * ceNode)
  {
    if(ceNode)
      {
	std::pair<std::map<UTI, NodeSquareBracket * >::iterator, bool> ret;
	ret = m_unknownArraysizeSubtrees.insert(std::pair<UTI, NodeSquareBracket *>(uti,ceNode));
	bool notdupi = ret.second; //false if already existed, i.e. not added
	if(!notdupi)
	  {
	    delete ceNode; //prevent leaks
	    ceNode = NULL;
	  }
      }
  } //linkConstantExpression (arraysize)

  bool Resolver::constantFoldUnknownArraysize(UTI auti, s32& arraysize)
  {
    bool rtnBool = true;  //unfound
    std::map<UTI, NodeSquareBracket *>::iterator it = m_unknownArraysizeSubtrees.find(auti);

    if(it != m_unknownArraysizeSubtrees.end())
      {
	assert(auti == it->first);
	NodeSquareBracket * ceNode = it->second;
	assert(ceNode);
	rtnBool = ceNode->getArraysizeInBracket(arraysize); //eval
	if(rtnBool && arraysize != UNKNOWNSIZE)
	  {
	    delete ceNode;
	    it->second = NULL;
	    m_unknownArraysizeSubtrees.erase(it);
	  }
	else
	  rtnBool = false; //unknown returns false
      }
    return rtnBool;
  } //constantFoldUnknownArraysize

  bool Resolver::statusUnknownArraysizeUTI()
  {
    bool rtnstat = true; //ok, empty
    if(!m_unknownArraysizeSubtrees.empty())
      {
	std::vector<UTI> lostUTIs;
	u32 lostsize = m_unknownArraysizeSubtrees.size();

	std::ostringstream msg;
	msg << "Found non-empty unknown arraysize subtrees, of class instance<";
	msg << m_state.getUlamTypeNameByIndex(m_classUTI).c_str();
	msg << ">, size " << lostsize << ":";

	std::map<UTI, NodeSquareBracket *>::iterator it = m_unknownArraysizeSubtrees.begin();

	while(it != m_unknownArraysizeSubtrees.end())
	  {
	    UTI auti = it->first;
	    msg << " (UTI" << auti << ") ";
	    msg << m_state.getUlamTypeNameByIndex(auti).c_str() << ",";
	    lostUTIs.push_back(auti);
	    it++;
	  }

	msg << " trying to update now";
	MSG("", msg.str().c_str(), DEBUG);
	rtnstat = false;

	assert(lostUTIs.size() == lostsize);
	while(!lostUTIs.empty())
	  {
	    UTI auti = lostUTIs.back();
	    constantFoldIncompleteUTI(auti);
	    lostUTIs.pop_back();
	  }
	lostUTIs.clear();
      }
    return rtnstat;
  } //statusUnknownArraysizeUTI

  void Resolver::linkConstantExpression(NodeConstantDef * ceNode)
  {
    if(ceNode)
      m_nonreadyNamedConstantSubtrees.insert(ceNode);
  }

  bool Resolver::statusNonreadyNamedConstants()
  {
    bool rtnstat = true; //ok, empty
    if(!m_nonreadyNamedConstantSubtrees.empty())
      {
	std::vector<NodeConstantDef *> lostCs;
	u32 lostsize = m_nonreadyNamedConstantSubtrees.size();

	std::ostringstream msg;
	msg << "Found non-empty non-ready named constant subtrees, of class <";
	msg << m_state.m_pool.getDataAsString(m_classUTI).c_str();
	msg << ">, size " << lostsize << ":";

	std::set<NodeConstantDef *>::iterator it = m_nonreadyNamedConstantSubtrees.begin();
	while(it != m_nonreadyNamedConstantSubtrees.end())
	  {
	    NodeConstantDef * constNode = *it;
	    Symbol * csym = NULL;
	    // symbols can be delayed (i.e. NULL)
	    if(constNode->getSymbolPtr(csym) && (!csym || !((SymbolConstantValue *) csym)->isReady()))
	      {
		msg << constNode->getName() << ",";
		lostCs.push_back(constNode);
	      }
	    it++;
	  }

	if(!lostCs.empty())
	  {
	    msg << " trying to update now";
	    rtnstat = false;
	    assert(lostCs.size() == lostsize);
	    while(!lostCs.empty())
	      {
		NodeConstantDef * ncd = lostCs.back();
		ncd->foldConstantExpression();
		lostCs.pop_back();
	      }
	    lostCs.clear();
	  }
	else
	  {
	    msg << " all ready";
	    m_nonreadyNamedConstantSubtrees.clear();  //all ready
	  }
	MSG("", msg.str().c_str(), DEBUG);
      }
    return rtnstat;
  } //statusNonreadyNamedConstants

  void Resolver::linkUnknownTypedefFromAnotherClass(UTI tduti, UTI stubuti)
  {
    std::pair<std::map<UTI, UTI>::iterator, bool> ret;
    ret = m_unknownTypedefFromAnotherClass.insert(std::pair<UTI, UTI>(tduti,stubuti));
    bool notdupi = ret.second; //false if already existed, i.e. not added
    if(!notdupi)
      {
	//not added
      }
  } //linkUnknownTypedefFromAnotherClass

  bool Resolver::statusUnknownTypedefsFromAnotherClass()
  {
    bool rtnstat = true; //ok, empty
    if(!m_unknownTypedefFromAnotherClass.empty())
      {
	u32 uksize = m_unknownTypedefFromAnotherClass.size();
	std::vector<UTI> foundTs;
	std::ostringstream msg;
	msg << "Found " << uksize << " unknown typedef";
	msg << (uksize > 1 ? "s " : " ") << "from another class: ";

	std::map<UTI, UTI>::iterator it = m_unknownTypedefFromAnotherClass.begin();
	while(it != m_unknownTypedefFromAnotherClass.end())
	  {
	    UTI tduti = it->first;
	    UTI aclassuti = it->second;
	    //if aclassuti is not a stub, look up tduti in its map of uti's
	    SymbolClass * acsym = NULL;
	    assert(m_state.alreadyDefinedSymbolClass(aclassuti, acsym));
	    if(!acsym->isStub())
	      {
		UTI mappedUTI;
		if(acsym->hasMappedUTI(tduti, mappedUTI))
		  {
		    msg << tduti << "-maps-to-" << mappedUTI << " in class ";
		    msg << m_state.getUlamTypeNameBriefByIndex(aclassuti).c_str() << "; ";

		    mapUTItoUTI(tduti, mappedUTI);
		    foundTs.push_back(tduti); //to be deleted
		  }
	      }
	    it++;
	  }
	msg << "while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_classUTI).c_str();
	MSG("", msg.str().c_str(), DEBUG);

	while(!foundTs.empty())
	  {
	    UTI futi = foundTs.back();
	    it = m_unknownTypedefFromAnotherClass.find(futi);
	    m_unknownTypedefFromAnotherClass.erase(it);
	    foundTs.pop_back();
	  }
	foundTs.clear();
	rtnstat = m_unknownTypedefFromAnotherClass.empty();
      }
    return rtnstat;
  } //statusUnknownTypedefsFromAnotherClass

  bool Resolver::statusNonreadyClassArguments()
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

	constantFoldNonreadyClassArgs();
      }
    return rtnstat;
  } //statusNonreadyClassArguments

  bool Resolver::constantFoldNonreadyClassArgs()
  {
    bool rtnb = true;
    //HOPEFULLY, all context dependent expressions have been simplified so
    // this step is no longer required.
#if 0
    // before trying to resolve class args, reset the context responsible for its existence
    // during resolving loop the current context may be its shallow self rather than the deep
    // instantiation with the needed values for the constants used in these pending args.
    if(m_classContextUTIForPendingArgs != m_state.m_compileThisIdx)
      {

	SymbolClass * csymptr = NULL;
	assert(m_state.alreadyDefinedSymbolClass(m_classContextUTIForPendingArgs, csymptr));

	m_state.setCompileThisIdx(m_classContextUTIForPendingArgs);
	NodeBlockClass * classNode = csymptr->getClassBlockNode();
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;
      }
#endif

    std::vector<NodeConstantDef *> leftCArgs;
    std::vector<NodeConstantDef *>::iterator vit = m_nonreadyClassArgSubtrees.begin();
    while(vit != m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;

	if(ceNode && ceNode->foldConstantExpression())
	  {
	    delete ceNode;
	    *vit = NULL;
	  }
	else
	  leftCArgs.push_back(ceNode);
	vit++;
      } //while thru vector of incomplete args only

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
    if(!ceNode)
      return;
    m_nonreadyClassArgSubtrees.push_back(ceNode);
  } //linkConstantExpressionForPendingArg

  bool Resolver::pendingClassArgumentsForClassInstance()
  {
    return !m_nonreadyClassArgSubtrees.empty();
  } //pendingClassArgumentsForClassInstance

  void Resolver::clonePendingClassArgumentsForStubClassInstance(const Resolver& rslvr, UTI context, SymbolClass * mycsym)
  {
    NodeBlockClass * classblock = mycsym->getClassBlockNode();
    SymbolClass * contextSym = NULL;
    assert(m_state.alreadyDefinedSymbolClass(context,contextSym));

    std::vector<NodeConstantDef *>::const_iterator vit = rslvr.m_nonreadyClassArgSubtrees.begin();
    while(vit != rslvr.m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;
	NodeConstantDef * cloneNode = new NodeConstantDef(*ceNode);

	Symbol * cvsym = NULL;
	assert(classblock->isIdInScope(cloneNode->getSymbolId(), cvsym));
	cloneNode->setSymbolPtr((SymbolConstantValue *) cvsym);

	//set context and try to resolve all context-dependent arg expressions..
	m_state.pushClassContext(context, contextSym->getClassBlockNode(), contextSym->getClassBlockNode(), false, NULL);

	if(cloneNode->foldConstantExpression())
	  delete cloneNode;
	else
	  linkConstantExpressionForPendingArg(cloneNode);

	m_state.popClassContext(); //restore previous context
	vit++;
      }
    m_classContextUTIForPendingArgs = context; //update (might not be needed anymore?)
  } //clonePendingClassArgumentsForStubClassInstance

  UTI Resolver::getContextForPendingArgs()
  {
    return m_classContextUTIForPendingArgs;
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
	msg << " for the fm UTI" << fmuti << " while mapping to: " << touti;
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
	assert(findMappedUTI(fmuti,checkuti));
	assert(checkuti == touti);
      }
    return notdup;
  } //mapUTItoUTI

  bool Resolver::findMappedUTI(UTI auti, UTI& mappedUTI)
  {
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

  bool Resolver::findNodeNo(NNO n, Node *& foundNode)
  {
    if(findNodeNoInUnknownBitsizes(n, foundNode))
      return true;

    if(findNodeNoInUnknownArraysizes(n, foundNode))
      return true;

    if(findNodeNoInNonreadyNamedConstants(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  bool Resolver::findNodeNoInUnknownBitsizes(NNO n, Node *& foundNode)
  {
    bool rtnB = false;
    std::map<UTI, NodeTypeBitsize *>::iterator it = m_unknownBitsizeSubtrees.begin();

    while(it != m_unknownBitsizeSubtrees.end())
      {
	NodeTypeBitsize * ceNode = it->second;
	assert(ceNode);
	if(ceNode->findNodeNo(n, foundNode))
	  {
	    rtnB = true;
	    break;
	  }
	it++;
      }
    return rtnB;
  } //findNodeNoInUnknownBitsizes

  bool Resolver::findNodeNoInUnknownArraysizes(NNO n, Node *& foundNode)
  {
    bool rtnB = false;
    std::map<UTI, NodeSquareBracket *>::iterator it = m_unknownArraysizeSubtrees.begin();

    while(it != m_unknownArraysizeSubtrees.end())
      {
	NodeSquareBracket * ceNode = it->second;
	assert(ceNode);
	if(ceNode->findNodeNo(n, foundNode))
	  {
	    rtnB = true;
	    break;
	  }
	it++;
      }
    return rtnB;
  } //findNodeNoInUnknownArraysizes

  bool Resolver::findNodeNoInNonreadyNamedConstants(NNO n, Node *& foundNode)
  {
    bool rtnB = false;

    std::set<NodeConstantDef *>::iterator it = m_nonreadyNamedConstantSubtrees.begin();
    while(it != m_nonreadyNamedConstantSubtrees.end())
      {
	NodeConstantDef * constNode = *it;
	assert(constNode);
	if(constNode->findNodeNo(n, foundNode))
	  {
	    rtnB = true;
	    break;
	  }
	it++;
      }
    return rtnB;
  } //findNodeNoInNonreadyNamedConstants

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

} //MFM
