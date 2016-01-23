#include "Resolver.h"
#include "CompilerState.h"
#include "SymbolClass.h"

namespace MFM {

  Resolver::Resolver(UTI instance, CompilerState& state) : m_state(state), m_classUTI(instance), m_classContextUTIForPendingArgs(m_state.getCompileThisIdx()) /*default*/ {}

  Resolver::~Resolver()
  {
    clearLeftoverSubtrees();
  }

  void Resolver::clearLeftoverSubtrees()
  {
    clearLeftoverNonreadyClassArgSubtrees();
    m_mapUTItoUTI.clear();
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

	rtnstat = constantFoldNonreadyClassArgs(); //forgot to update rtnstat?
      }
    return rtnstat;
  } //statusNonreadyClassArguments

  bool Resolver::constantFoldNonreadyClassArgs()
  {
    bool rtnb = true;
    UTI context = getContextForPendingArgs();
    SymbolClass * contextSym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(context, contextSym);
    assert(isDefined);
    m_state.pushClassContext(context, contextSym->getClassBlockNode(), contextSym->getClassBlockNode(), false, NULL);

    m_state.m_pendingArgStubContext = m_classUTI; //set for folding surgery

    std::vector<NodeConstantDef *> leftCArgs;
    std::vector<NodeConstantDef *>::iterator vit = m_nonreadyClassArgSubtrees.begin();
    while(vit != m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;
	if(ceNode)
	  {
	    UTI uti = ceNode->checkAndLabelType();
	    if(m_state.okUTItoContinue(uti)) //i.e. ready
	      {
		delete ceNode;
		*vit = NULL;
	      }
	    else
	      leftCArgs.push_back(ceNode);
	  }
	vit++;
      } //while thru vector of incomplete args only

    m_state.m_pendingArgStubContext = Nouti; //clear flag
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

  void Resolver::clonePendingClassArgumentsForStubClassInstance(const Resolver& rslvr, UTI context, SymbolClass * mycsym)
  {
    NodeBlockClass * classblock = mycsym->getClassBlockNode();
    SymbolClass * contextSym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(context, contextSym);
    assert(isDefined);

    std::vector<NodeConstantDef *>::const_iterator vit = rslvr.m_nonreadyClassArgSubtrees.begin();
    while(vit != rslvr.m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;
	ceNode->fixPendingArgumentNode();
	NodeConstantDef * cloneNode = new NodeConstantDef(*ceNode);

	Symbol * cvsym = NULL;
	AssertBool isDefined = classblock->isIdInScope(cloneNode->getSymbolId(), cvsym);
	assert(isDefined);
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

  bool Resolver::findNodeNo(NNO n, Node *& foundNode)
  {
    if(findNodeNoInNonreadyClassArgs(n, foundNode))
      return true;

    return false;
  } //findNodeNo

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

  void Resolver::countNavNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    countNavNodesInPendingArgs(ncnt, hcnt, nocnt);
  }

  void Resolver::countNavNodesInPendingArgs(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    std::vector<NodeConstantDef *>::const_iterator vit = m_nonreadyClassArgSubtrees.begin();
    while(vit != m_nonreadyClassArgSubtrees.end())
      {
	NodeConstantDef * ceNode = *vit;
	assert(ceNode);
	ceNode->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
	vit++;
      }
  } //countNavNodesInPendingArgs

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
