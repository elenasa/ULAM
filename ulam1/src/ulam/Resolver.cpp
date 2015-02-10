#include "Resolver.h"
#include "CompilerState.h"

namespace MFM {

  Resolver::Resolver(UTI instance, CompilerState& state) : m_state(state), m_classUTI(instance) {}
  Resolver::~Resolver()
  {
    clearLeftoverSubtrees();
  }

  void Resolver::clearLeftoverSubtrees()
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

    s32 nonreadyC = m_nonreadyNamedConstantSubtrees.size();
    if(nonreadyC > 0)
      {
	std::ostringstream msg;
	msg << "Nonready named constant subtrees cleared: " << nonreadyC;
	MSG("",msg.str().c_str(),DEBUG);
      }
    m_nonreadyNamedConstantSubtrees.clear();


    s32 nonreadyG = m_nonreadyClassArgSubtrees.size();
    if(nonreadyG > 0)
      {
	std::ostringstream msg;
	msg << "Class Instances with non-ready argument constant subtrees cleared: " << nonreadyG;
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
  } //clearLeftoverSubtrees()

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
  }

  void Resolver::cloneConstantExpressionSubtreesByUTI(UTI olduti, UTI newuti, const Resolver& templateRslvr)
  {
    //Type bitsize UNKNOWN:
    NodeTypeBitsize * tbceNode = templateRslvr.findUnknownBitsizeUTI(olduti);
    if(tbceNode)
      {
	NodeTypeBitsize * cloneNode = new NodeTypeBitsize(*tbceNode);
	linkConstantExpression(newuti, cloneNode);
      }

    //Arraysize UNKNOWN:
    NodeSquareBracket * asceNode = templateRslvr.findUnknownArraysizeUTI(olduti);
    if(asceNode)
      {
	NodeSquareBracket * cloneNode = new NodeSquareBracket(*asceNode);
	linkConstantExpression(newuti, cloneNode);
      }
  }//cloneConstantExpressionSubtreesByUTI

  void Resolver::cloneNamedConstantExpressionSubtrees(const Resolver& templateRslvr)
  {
    if(!templateRslvr.m_nonreadyNamedConstantSubtrees.empty())
      {
	u32 lostsize = m_nonreadyNamedConstantSubtrees.size();
	std::ostringstream msg;
	msg << "Found non-empty non-ready named constant subtrees, while cloning class <";
	msg << m_state.getUlamTypeNameByIndex(m_classUTI).c_str();
	msg << ">, size " << lostsize << ":";

	std::set<NodeConstantDef *>::iterator it = templateRslvr.m_nonreadyNamedConstantSubtrees.begin();

	while(it != templateRslvr.m_nonreadyNamedConstantSubtrees.end())
	  {
	    NodeConstantDef * constNode = *it;
	    Symbol * sym;
	    if(constNode->getSymbolPtr(sym) && !((SymbolConstantValue *) sym)->isReady())
	      {
		msg << constNode->getName() << ",";
		NodeConstantDef * cloneNode = new NodeConstantDef(*constNode);
		linkConstantExpression(cloneNode);
	      }
	    it++;
	  }
	MSG("", msg.str().c_str(), DEBUG);
      }
  } //cloneNamedConstantExpressionSubtrees

  bool Resolver::statusUnknownConstantExpressions()
  {
    bool sumbrtn = true;
    sumbrtn &= statusUnknownBitsizeUTI();
    sumbrtn &= statusUnknownArraysizeUTI();
    sumbrtn &= statusNonreadyNamedConstants();
    return sumbrtn;
  }//statusUnknownConstantExpressions

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
	    msg << " (UTI" << auti << ") " << m_state.getUlamTypeNameByIndex(auti).c_str() << ",";
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
	if(rtnBool)
	  {
	    delete ceNode;
	    it->second = NULL;
	    m_unknownArraysizeSubtrees.erase(it);
	  }
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
	    msg << " (UTI" << auti << ") " << m_state.getUlamTypeNameByIndex(auti).c_str() << ",";
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
	msg << m_state.m_pool.getDataAsString(m_classUTI);
	msg << ">, size " << lostsize << ":";

	std::set<NodeConstantDef *>::iterator it = m_nonreadyNamedConstantSubtrees.begin();

	while(it != m_nonreadyNamedConstantSubtrees.end())
	  {
	    NodeConstantDef * constNode = *it;
	    Symbol * csym;
	    if(constNode->getSymbolPtr(csym) && !((SymbolConstantValue *) csym)->isReady())
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

  //called while parsing this shallow class instance use;
  void Resolver::linkConstantExpressionForPendingArg(NodeConstantDef * ceNode)
  {
    if(!ceNode)
      return;
    m_nonreadyClassArgSubtrees.push_back(ceNode);
  } //linkConstantExpressionForPendingArg

  bool Resolver::constantFoldNonreadyClassArgs()
  {
    bool rtnb = true;
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
      } //while thru vector of arg's

    //clean up, replace vector with vector of those still unresolved
    m_nonreadyClassArgSubtrees.clear();
    if(!leftCArgs.empty())
      {
	m_nonreadyClassArgSubtrees = leftCArgs; //replace
	rtnb = false;
      }
    return rtnb;
  } //constantFoldNonreadyClassArgs

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

  bool Resolver::pendingClassArgumentsForClassInstance()
  {
    return !m_nonreadyClassArgSubtrees.empty();
  } //pendingClassArgumentsForClassInstance

} //MFM
