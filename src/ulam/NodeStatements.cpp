#include <stdio.h>
#include "NodeStatements.h"
#include "CompilerState.h"

namespace MFM {

  NodeStatements::NodeStatements(Node * s, CompilerState & state) : Node(state), m_node(s), m_nodeNext(NULL)
  {
    if(s)
      setNodeLocation(s->getNodeLocation());
  }

  NodeStatements::NodeStatements(const NodeStatements& ref) : Node(ref), m_node(NULL), m_nodeNext(NULL)
  {
    if(ref.m_node)
      m_node = ref.m_node->instantiate();
    if(ref.m_nodeNext)
      m_nodeNext = (NodeStatements *) ref.m_nodeNext->instantiate();
  }

  NodeStatements::~NodeStatements()
  {
    delete m_nodeNext;
    m_nodeNext = NULL;
    delete m_node;
    m_node = NULL;
  }

  Node * NodeStatements::instantiate()
  {
    return new NodeStatements(*this);
  }

  void NodeStatements::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_node)
      m_node->updateLineage(getNodeNo());
    if(m_nodeNext)
      m_nodeNext->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeStatements::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_node && m_node == oldnptr)
      {
	m_node = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeStatements::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_node && m_node->findNodeNo(n, foundNode))
      return true;
    if(m_nodeNext && m_nodeNext->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeStatements::checkAbstractInstanceErrors()
  {
    if(m_node)
      m_node->checkAbstractInstanceErrors();
    if(m_nodeNext)
      m_nodeNext->checkAbstractInstanceErrors();
  }

  void NodeStatements::print(File * fp)
  {
    printNodeLocation(fp); //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    if(m_node)
      m_node->print(fp);
    else
      fp->write(" <EMPTYSTMT>\n");

    if(m_nodeNext)
      m_nodeNext->print(fp);
    else
      fp->write(" <NONEXTSTMT>\n");
    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  } //print

  void NodeStatements::printPostfix(File * fp)
  {
    assert(m_node);    //e.g. bad decl

    if(m_node)
      m_node->printPostfix(fp);
    else
      fp->write(" <EMPTYSTMT>");

    if(m_nodeNext)
      m_nodeNext->printPostfix(fp);
  } //printPostfix

  void NodeStatements::noteTypeAndName(s32 totalsize, u32& accumsize)
  {
    assert(m_node);    //e.g. bad decl
    m_node->noteTypeAndName(totalsize, accumsize);

    if(m_nodeNext)
      m_nodeNext->noteTypeAndName(totalsize, accumsize);
  }

  const char * NodeStatements::getName()
  {
    return "Stmts"; //?
  }

  const std::string NodeStatements::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeStatements::isAConstant()
  {
    bool rtn = m_node->isAConstant();
    if(rtn && m_nodeNext)
      rtn &= m_nodeNext->isAConstant(); //side-effect
    return rtn;
  }

  bool NodeStatements::isFunctionCall()
  {
    if(m_node->isFunctionCall())
      return true;
    if(m_nodeNext)
      return m_nodeNext->isFunctionCall(); //side-effect
    return false;
  }

  bool NodeStatements::isArrayItem()
  {
    if(m_node->isArrayItem())
      return true;
    if(m_nodeNext)
      return m_nodeNext->isArrayItem(); //side-effect
    return false;
  }

  bool NodeStatements::isExplicitReferenceCast()
  {
    if(m_node->isExplicitReferenceCast())
      return true;
    if(m_nodeNext)
      return m_nodeNext->isExplicitReferenceCast();
    return false;
  }

  UTI NodeStatements::checkAndLabelType()
  {
    assert(m_node);

    //unlike statements, blocks don't have an m_node
    m_node->checkAndLabelType(); //side-effect

    if(m_nodeNext)
      m_nodeNext->checkAndLabelType(); //side-effect

    //statements don't have types
    setNodeType(Void);
    return getNodeType();
  } //checkAndLabelType

  void NodeStatements::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    if(m_node)
      m_node->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if(m_nodeNext)
      m_nodeNext->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  bool NodeStatements::buildDefaultValue(u32 wlen, BV8K& dvref)
  {
    bool aok = true;
    if(m_node)
      aok &= m_node->buildDefaultValue(wlen, dvref); //yikes! (was |=) (t41185)
    if(aok && m_nodeNext) //why go on? (t41185)
      aok &= m_nodeNext->buildDefaultValue(wlen, dvref);
    return aok;
  }

  bool NodeStatements::buildDefaultValueForClassConstantDefs()
  {
    bool aok = true;
    if(m_node)
      aok &= m_node->buildDefaultValueForClassConstantDefs();
    if(aok && m_nodeNext) //why go on
      aok &= m_nodeNext->buildDefaultValueForClassConstantDefs();
    return aok;
  }

  void NodeStatements::genCodeDefaultValueStringRegistrationNumber(File * fp, u32 startpos)
  {
    if(m_node)
      m_node->genCodeDefaultValueStringRegistrationNumber(fp, startpos);
    if(m_nodeNext)
      m_nodeNext->genCodeDefaultValueStringRegistrationNumber(fp, startpos);
    return;
  }

  void NodeStatements::genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos)
  {
    if(m_node)
      m_node->genCodeElementTypeIntoDataMemberDefaultValue(fp, startpos);
    if(m_nodeNext)
      m_nodeNext->genCodeElementTypeIntoDataMemberDefaultValue(fp, startpos);
  }

  EvalStatus NodeStatements::eval()
  {
    assert(m_node);

    evalNodeProlog(0);
    makeRoomForNodeType(m_node->getNodeType());
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //not the last one, so thrown out results and continue
    if(m_nodeNext)
      {
	evalNodeEpilog();
	evs = m_nodeNext->eval();
	if(evs != NORMAL)
	  {
	    return evs;
	  }
      }
    else
      evalNodeEpilog();
    return NORMAL;
  } //eval

  void NodeStatements::setNextNode(NodeStatements * s)
  {
    m_nodeNext = s;
  }

  TBOOL NodeStatements::packBitsInOrderOfDeclaration(u32& offset)
  {
    TBOOL rtntb = m_node->packBitsInOrderOfDeclaration(offset); //updates offset
    if(m_nodeNext)
      {
	TBOOL nodetb = m_nodeNext->packBitsInOrderOfDeclaration(offset);
	rtntb = Node::minTBOOL(rtntb, nodetb);
      }
    return rtntb;
  }

  void NodeStatements::printUnresolvedVariableDataMembers()
  {
    m_node->printUnresolvedVariableDataMembers(); //updates offset
    if(m_nodeNext)
      m_nodeNext->printUnresolvedVariableDataMembers();
  }

  void NodeStatements::printUnresolvedLocalVariables(u32 fid)
  {
    if(m_node)
      m_node->printUnresolvedLocalVariables(fid); //updates offset
    if(m_nodeNext)
      m_nodeNext->printUnresolvedLocalVariables(fid);
  }

  void NodeStatements::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    u32 max2 = depth;
    u32 max1 = depth;
    m_node->calcMaxDepth(max1, maxdepth, base);

    if(m_nodeNext)
      m_nodeNext->calcMaxDepth(max1, maxdepth, base);

    depth = max1 > max2 ? max1 : max2; //no change to maxdepth here
  } //calcMaxDepth

  void NodeStatements::genCode(File * fp, UVPass& uvpass)
  {
    Locator nodeloc = getNodeLocation();
    m_state.outputTextAsCommentWithLocationUpdate(fp, nodeloc);

    m_node->genCode(fp, uvpass);

    if(m_nodeNext)
      m_nodeNext->genCode(fp, uvpass);
  } //genCode

  void NodeStatements::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    m_node->genCodeToStoreInto(fp, uvpass);
    if(m_nodeNext)
      m_nodeNext->genCodeToStoreInto(fp, uvpass);
  }

  void NodeStatements::genCodeExtern(File * fp, bool declOnly)
  {
    if(m_node)
      m_node->genCodeExtern(fp, declOnly);
    if(m_nodeNext)
      m_nodeNext->genCodeExtern(fp, declOnly);
  }

  void NodeStatements::genCodeConstantArrayInitialization(File * fp)
  {
    if(m_node)
      m_node->genCodeConstantArrayInitialization(fp);
    if(m_nodeNext)
      m_nodeNext->genCodeConstantArrayInitialization(fp);
  }

  void NodeStatements::generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly)
  {
    if(m_node)
      m_node->generateBuiltinConstantArrayInitializationFunction(fp, declOnly);
    if(m_nodeNext)
      m_nodeNext->generateBuiltinConstantArrayInitializationFunction(fp, declOnly);
  }

  void NodeStatements::cloneAndAppendNode(std::vector<Node *> & cloneVec)
  {
    if(m_node)
      m_node->cloneAndAppendNode(cloneVec);
    if(m_nodeNext)
      m_nodeNext->cloneAndAppendNode(cloneVec);
  }

  void NodeStatements::generateTestInstance(File * fp, bool runtest)
  {
    if(m_node)
      m_node->generateTestInstance(fp, runtest);
    if(m_nodeNext)
      m_nodeNext->generateTestInstance(fp, runtest);
    return;
  }

  void NodeStatements::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    m_node->generateUlamClassInfo(fp, declOnly, dmcount);
    if(m_nodeNext)
      m_nodeNext->generateUlamClassInfo(fp, declOnly, dmcount);
  }

  void NodeStatements::addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers)
  {
    m_node->addMemberDescriptionToInfoMap(classType, classmembers);
    if(m_nodeNext)
      m_nodeNext->addMemberDescriptionToInfoMap(classType, classmembers);
  }

} //end MFM
