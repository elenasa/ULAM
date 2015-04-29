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
    //    clearLeftoverUnknownBitsizeSubtrees();
    //clearLeftoverUnknownArraysizeSubtrees();
    //clearLeftoverNonreadyNamedConstantSubtrees();
    clearLeftoverNonreadyClassArgSubtrees();
    //clearLeftoverUnknownTypdedefsFromAnotherClass();
    m_mapUTItoUTI.clear();
    //m_incompleteArrayTypeToItsBaseScalarType.clear();
  } //clearLeftoverSubtrees()

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

#if 0
  NodeTypeBitsize * Resolver::findUnknownBitsizeUTI(UTI auti) const
  {
    if(m_unknownBitsizeSubtrees.empty()) return NULL;

    std::map<UTI, NodeTypeBitsize *>::const_iterator it = m_unknownBitsizeSubtrees.find(auti);
    if(it != m_unknownBitsizeSubtrees.end())
      return it->second;
    return NULL;
  }

  NodeSquareBracket * Resolver::findUnknownArraysizeUTI(UTI auti) const
  {
    if(m_unknownArraysizeSubtrees.empty()) return NULL;

    std::map<UTI, NodeSquareBracket *>::const_iterator it = m_unknownArraysizeSubtrees.find(auti);
    if(it != m_unknownArraysizeSubtrees.end())
      return it->second;
    return NULL;
  } //findUnknownArraysizeUTI

  UTI Resolver::findIncompleteArrayTypeBaseScalarType(UTI auti) const
  {
    if(m_incompleteArrayTypeToItsBaseScalarType.empty()) return Nav;

    std::map<UTI, UTI>::const_iterator it = m_incompleteArrayTypeToItsBaseScalarType.find(auti);
    if(it != m_incompleteArrayTypeToItsBaseScalarType.end())
      return it->second;
    return Nav;
  } //findincompleteArrayTypeToItsBaseScalarType
#endif

  bool Resolver::statusUnknownConstantExpressions()
  {
    bool sumbrtn = true;
    //    sumbrtn &= statusUnknownBitsizeUTI();
    //sumbrtn &= statusUnknownArraysizeUTI();
    //sumbrtn &= statusIncompleteArrayTypes();
    //sumbrtn &= statusNonreadyNamedConstants();
    //sumbrtn &= statusUnknownTypedefsFromAnotherClass();
    //sumbrtn &= statusMappedTypes(); //always true
    return sumbrtn;
  } //statusUnknownConstantExpressions

#if 0
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
#endif

#if 0
  void Resolver::linkConstantExpression(UTI uti, NodeTypeBitsize * ceNode)
  {
    if(ceNode)
      {
	assert(!isTypedefFromAnotherClass(uti));
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
#endif

#if 0
  void Resolver::cloneAndLinkConstantExpression(UTI fromtype, UTI totype)
  {
    assert(!m_unknownBitsizeSubtrees.empty());

    std::map<UTI, NodeTypeBitsize *>::iterator it = m_unknownBitsizeSubtrees.find(fromtype);
    assert(it != m_unknownBitsizeSubtrees.end());
    assert(it->first == fromtype);
    NodeTypeBitsize * ceNode = it->second;
    NodeTypeBitsize * cloneSubtree = new NodeTypeBitsize(*ceNode); //any symbols will be null until c&l
    assert(cloneSubtree);
    cloneSubtree->resetNodeNo(m_state.getNextNodeNo()); //mucks up finding nno when the same
    cloneSubtree->updateLineage(cloneSubtree->getNodeNo()); //for future constant folding; does it hurt to have clones' parent nno be itself?

    linkConstantExpression(totype, cloneSubtree);
  } //linkConstantExpression (bitsize, decllist)
#endif

#if 0
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
#endif

#if 0
  bool Resolver::constantFoldUnknownBitsize(UTI auti, s32& bitsize)
  {
    if(m_unknownBitsizeSubtrees.empty()) return true;

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
#endif

#if 0
  void Resolver::linkConstantExpression(UTI uti, NodeSquareBracket * ceNode)
  {
    if(ceNode)
      {
	bool notdupi = false;
	if(!isTypedefFromAnotherClass(uti))
	  {
	    std::pair<std::map<UTI, NodeSquareBracket * >::iterator, bool> ret;
	    ret = m_unknownArraysizeSubtrees.insert(std::pair<UTI, NodeSquareBracket *>(uti,ceNode));
	    notdupi = ret.second; //false if already existed, i.e. not added
	  }

	if(!notdupi)
	  {
	    delete ceNode; //prevent leaks
	    ceNode = NULL;
	  }
      }
  } //linkConstantExpression (arraysize)
#endif

#if 0
  bool Resolver::constantFoldUnknownArraysize(UTI auti, s32& arraysize)
  {
    if(m_unknownArraysizeSubtrees.empty()) return true;

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
#endif

#if 0
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
#endif

#if 0
  void Resolver::linkIncompleteArrayTypeToItsBaseScalarType(UTI arraytype, UTI scalartype)
  {
    std::pair<std::map<UTI, UTI>::iterator, bool> ret;
    ret = m_incompleteArrayTypeToItsBaseScalarType.insert(std::pair<UTI, UTI>(arraytype,scalartype));
    bool notdupi = ret.second; //false if already existed, i.e. not added
    if(!notdupi)
      {
	//not added
      }
  } //linkIncompleteArrayTypeToItsBaseScalarType
#endif

#if 0
  bool Resolver::statusIncompleteArrayTypes()
  {
    bool rtnstat = true; //ok, empty
    if(!m_incompleteArrayTypeToItsBaseScalarType.empty())
      {
	u32 uksize = m_incompleteArrayTypeToItsBaseScalarType.size();
	std::vector<UTI> foundAs;
	std::ostringstream msg;
	msg << "Found " << uksize << " incomplete array type";
	msg << (uksize > 1 ? "s " : " ");
	msg << "while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_classUTI).c_str();
	MSG("", msg.str().c_str(), DEBUG);

	std::map<UTI, UTI>::iterator it = m_incompleteArrayTypeToItsBaseScalarType.begin();
	while(it != m_incompleteArrayTypeToItsBaseScalarType.end())
	  {
	    UTI auti = it->first; //array
	    UTI buti = it->second; //base scalar

	    attemptToResolveHolderMappedType(auti);
	    //if auti is still incomplete with unknown bitsize:
	    //  1. if base is still incomplete, then still incomplete
	    //  2. if base bitsize is known, update auti, and remove from map
	    if(m_state.getBitSize(auti) > UNKNOWNSIZE)
	      {
		foundAs.push_back(auti);
	      }
	    else
	      {
		attemptToResolveHolderMappedType(buti);
		attemptToResolveHolderArrayType(auti, buti);

		if(m_state.getBitSize(auti) > UNKNOWNSIZE)
		  {
		    foundAs.push_back(auti);
		  }
	      }
	    it++;
	  } //while

	while(!foundAs.empty())
	  {
	    UTI futi = foundAs.back();
	    it = m_incompleteArrayTypeToItsBaseScalarType.find(futi);
	    m_incompleteArrayTypeToItsBaseScalarType.erase(it);
	    foundAs.pop_back();
	  }
	foundAs.clear();
	rtnstat = m_incompleteArrayTypeToItsBaseScalarType.empty();
      }
    return rtnstat;
  } //statusIncompleteArrayTypes
#endif

  bool Resolver::statusMappedTypes()
  {
    bool rtnstat = true; //ok, empty
    if(!m_mapUTItoUTI.empty())
      {
	u32 uksize = m_mapUTItoUTI.size();
	std::vector<UTI> foundAs;
	std::ostringstream msg;
	msg << "Found " << uksize << " mapped type";
	msg << (uksize > 1 ? "s " : " ");
	msg << "while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_classUTI).c_str();
	MSG("", msg.str().c_str(), DEBUG);

	std::map<UTI, UTI>::iterator it = m_mapUTItoUTI.begin();
	while(it != m_mapUTItoUTI.end())
	  {
	    UTI auti = it->first;
	    attemptToResolveHolderMappedType(auti);
	    it++;
	  } //while
      }
    return rtnstat;
  } //statusMappedTypes

  bool Resolver::attemptToResolveHolderMappedType(UTI uti)
  {
    bool rtnstat = true; //ok, empty
    if(!m_mapUTItoUTI.empty())
      {
	std::map<UTI, UTI>::iterator it = m_mapUTItoUTI.find(uti);
	if(it != m_mapUTItoUTI.end())
	  {
	    UTI auti = it->first;
	    UTI buti = it->second;

	    UlamType *aut = m_state.getUlamTypeByIndex(auti);
	    UlamType *but = m_state.getUlamTypeByIndex(buti);
	    //if auti or buti is a holder, but not both
	    // exchange keys
	    bool aholder = aut->isHolder();
	    bool bholder = but->isHolder();

	    if(aholder ^ bholder)
	      {
		UlamKeyTypeSignature akey = aut->getUlamKeyTypeSignature();
		UlamKeyTypeSignature bkey = but->getUlamKeyTypeSignature();
		if(aholder)
		  {
		    m_state.makeUlamTypeFromHolder(akey, bkey, but->getUlamTypeEnum(), auti);
		  }
		else
		  {
		    m_state.makeUlamTypeFromHolder(bkey, akey, aut->getUlamTypeEnum(), buti);
		  }
	      }
	    else
	      {
		// neither holders though there may be a bit size
		bool acomplete = aut->isComplete();
		bool bcomplete = but->isComplete();
		if(acomplete ^ bcomplete)
		  {
		    UlamKeyTypeSignature akey = aut->getUlamKeyTypeSignature();
		    UlamKeyTypeSignature bkey = but->getUlamKeyTypeSignature();
		    if(acomplete)
		      {
			UlamKeyTypeSignature newkey(bkey.getUlamKeyTypeSignatureNameId(), akey.getUlamKeyTypeSignatureBitSize(), bkey.getUlamKeyTypeSignatureArraySize(), bkey.getUlamKeyTypeSignatureClassInstanceIdx());
			m_state.makeUlamTypeFromHolder(bkey, newkey, aut->getUlamTypeEnum(), buti);
		      }
		    else
		      {
			UlamKeyTypeSignature newkey(akey.getUlamKeyTypeSignatureNameId(), bkey.getUlamKeyTypeSignatureBitSize(), akey.getUlamKeyTypeSignatureArraySize(), akey.getUlamKeyTypeSignatureClassInstanceIdx());
			m_state.makeUlamTypeFromHolder(akey, newkey, but->getUlamTypeEnum(), auti);
		      }
		  }
	      }

	  } //found
      }
    return rtnstat;
  } //attemptToResolveHolderMappedType

  bool Resolver::attemptToResolveHolderArrayType(UTI auti, UTI buti)
  {
    bool rtnstat = true; //ok, empty
    UlamType *aut = m_state.getUlamTypeByIndex(auti);
    UlamType *but = m_state.getUlamTypeByIndex(buti);
    //if auti or buti is a holder, but not both
    // update key
    bool aholder = aut->isHolder();
    bool bholder = but->isHolder();

    if(aholder ^ bholder)
      {
	UlamKeyTypeSignature akey = aut->getUlamKeyTypeSignature();
	UlamKeyTypeSignature bkey = but->getUlamKeyTypeSignature();
	if(aholder)
	  {
	    UlamKeyTypeSignature newkey(bkey.getUlamKeyTypeSignatureNameId(), bkey.getUlamKeyTypeSignatureBitSize(), akey.getUlamKeyTypeSignatureArraySize(), akey.getUlamKeyTypeSignatureClassInstanceIdx());
	    m_state.makeUlamTypeFromHolder(akey, newkey, but->getUlamTypeEnum(), auti);
	  }
	else
	  {
	    UlamKeyTypeSignature newkey(akey.getUlamKeyTypeSignatureNameId(), akey.getUlamKeyTypeSignatureBitSize(), bkey.getUlamKeyTypeSignatureArraySize(), bkey.getUlamKeyTypeSignatureClassInstanceIdx());
	    m_state.makeUlamTypeFromHolder(bkey, newkey, aut->getUlamTypeEnum(), buti);
	  }
      }
    else
      {
	// neither holders though there may be a bit size
	bool acomplete = aut->isComplete();
	bool bcomplete = but->isComplete();
	if(acomplete ^ bcomplete)
	  {
	    UlamKeyTypeSignature akey = aut->getUlamKeyTypeSignature();
	    UlamKeyTypeSignature bkey = but->getUlamKeyTypeSignature();
	    if(acomplete)
	      {
		UlamKeyTypeSignature newkey(akey.getUlamKeyTypeSignatureNameId(), akey.getUlamKeyTypeSignatureBitSize(), bkey.getUlamKeyTypeSignatureArraySize(), bkey.getUlamKeyTypeSignatureClassInstanceIdx());
		m_state.makeUlamTypeFromHolder(bkey, newkey, aut->getUlamTypeEnum(), buti);
	      }
	    else
	      {
		UlamKeyTypeSignature newkey(bkey.getUlamKeyTypeSignatureNameId(), bkey.getUlamKeyTypeSignatureBitSize(), akey.getUlamKeyTypeSignatureArraySize(), akey.getUlamKeyTypeSignatureClassInstanceIdx());
		m_state.makeUlamTypeFromHolder(akey, newkey, but->getUlamTypeEnum(), auti);
	      }
	  }
      }

    return rtnstat;
  } //attemptToResolveHolderArrayType

#if 0
  void Resolver::linkConstantExpression(NodeConstantDef * ceNode)
  {
    if(ceNode)
      m_nonreadyNamedConstantSubtrees.insert(ceNode);
  }
#endif

#if 0
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
		//ncd->foldConstantExpression();
		ncd->checkAndLabelType();
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
#endif

#if 0
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
#endif

#if 0
  bool Resolver::isTypedefFromAnotherClass(UTI uti)
  {
    if(m_unknownTypedefFromAnotherClass.empty()) return false;

    std::map<UTI, UTI>::iterator it = m_unknownTypedefFromAnotherClass.find(uti);
    if(it != m_unknownTypedefFromAnotherClass.end())
      {
	return true;
      }
    return false;
  } //isTypedefFromAnotherClass
#endif

#if 0
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

	    if(!m_state.isComplete(tduti))
	      {
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
			attemptToResolveHolderMappedType(tduti);
			// keep in case more is revealed about tduti next round; or test for completeness.
			//if(!m_state.getUlamTypeByIndex(tduti)->isHolder()) //keep if holder
			//  foundTs.push_back(tduti); //to be deleted
		      }
		  }

		if(m_state.getUlamTypeByIndex(aclassuti)->isHolder())
		  {
		    UTI classAlias;
		    if(m_state.findaUTIAlias(aclassuti, classAlias))
		      {
			//if classAlias is not a stub, look up tduti in its map of uti's
			SymbolClass * acsym = NULL;
			assert(m_state.alreadyDefinedSymbolClass(classAlias, acsym));
			if(!acsym->isStub())
			  {
			    UTI mappedUTI;
			    if(acsym->hasMappedUTI(tduti, mappedUTI))
			      {
				msg << tduti << "-maps-to-" << mappedUTI << " in alias class ";
				msg << m_state.getUlamTypeNameBriefByIndex(classAlias).c_str() << "; ";

				mapUTItoUTI(tduti, mappedUTI);
				attemptToResolveHolderMappedType(tduti);
			      }
			  }
		      }
		  }
		UTI aliastduti;
		if(m_state.findaUTIAlias(tduti, aliastduti))
		  {
		    msg << tduti << "-maps-to-" << aliastduti << " an alias ";
		    msg << m_state.getUlamTypeNameBriefByIndex(aliastduti).c_str() << "; ";

		    mapUTItoUTI(tduti, aliastduti);
		    attemptToResolveHolderMappedType(tduti);
		  }
	      }
	    else
	      foundTs.push_back(tduti); //to be deleted

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
#endif

  bool Resolver::assignClassArgValuesInStubCopy()
  {
    bool aok = true;
    // context is already set
    std::vector<NodeConstantDef *>::iterator vit = m_nonreadyClassArgSubtrees.begin();
    while(vit != m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;

	if(ceNode)
	  {
	    aok &= ceNode->assignClassArgValueInStubCopy();
	  }
	vit++;
      } //while thru vector of incomplete args only
    return aok;
  } //assignClassArgValuesInStubCopy

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
    std::vector<NodeConstantDef *> leftCArgs;
    std::vector<NodeConstantDef *>::iterator vit = m_nonreadyClassArgSubtrees.begin();
    while(vit != m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;
	// c&l calls foldConstantExpression instead of the reverse.
	//if(ceNode && ceNode->foldConstantExpression())
	if(ceNode)
	  {
	    UTI uti = ceNode->checkAndLabelType();
	    if(uti != Nav) //i.e. ready
	      {
		delete ceNode;
		*vit = NULL;
	      }
	    else
	      leftCArgs.push_back(ceNode);
	  }
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

	linkConstantExpressionForPendingArg(cloneNode); //resolve later
	vit++;
      }

    m_classContextUTIForPendingArgs = context; //update (might not be needed anymore?)

    //Cannot MIX the current block (context) to find symbols while
    //using this stub copy to find parent NNOs for constant folding;
    //therefore we separate them so that all we do now is update the
    //constant values in the stub copy's Resolver map.
    //Resolution of all context-dependent arg expressions will occur
    //during the resolving loop..
    m_state.pushClassContext(context, contextSym->getClassBlockNode(), contextSym->getClassBlockNode(), false, NULL);
    assignClassArgValuesInStubCopy();
    m_state.popClassContext(); //restore previous context
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

  bool Resolver::findNodeNo(NNO n, Node *& foundNode)
  {
    //    if(findNodeNoInUnknownBitsizes(n, foundNode))
    //  return true;

    //if(findNodeNoInUnknownArraysizes(n, foundNode))
    //  return true;

    //if(findNodeNoInNonreadyNamedConstants(n, foundNode))
    //  return true;

    if(findNodeNoInNonreadyClassArgs(n, foundNode))
      return true;

    return false;
  } //findNodeNo

#if 0
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
#endif

#if 0
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
#endif

  bool Resolver::findNodeNoInNonreadyClassArgs(NNO n, Node *& foundNode)
  {
    bool rtnB = false;

    std::vector<NodeConstantDef *>::const_iterator vit = m_nonreadyClassArgSubtrees.begin();
    while(vit != m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;
	assert(ceNode);
	if(ceNode->findNodeNo(n, foundNode))
	  {
	    rtnB = true;
	    break;
	  }
	vit++;
      }
    return rtnB;
  } //findNodeNoInNonreadyClassArgs

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
