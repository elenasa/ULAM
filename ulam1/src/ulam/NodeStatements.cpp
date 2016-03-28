#include <stdio.h>
#include "NodeStatements.h"
#include "CompilerState.h"

namespace MFM {

  NodeStatements::NodeStatements(Node * s, CompilerState & state) : Node(state), m_node(s), m_nodeNext(NULL) {}

  NodeStatements::NodeStatements(const NodeStatements& ref) : Node(ref)
  {
    if(ref.m_node)
      m_node = ref.m_node->instantiate();
    else
      m_node = NULL;

    if(ref.m_nodeNext)
      m_nodeNext = (NodeStatements *) ref.m_nodeNext->instantiate();
    else
      m_nodeNext = NULL;
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
  } //checkAbstractInstanceErrors

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
  } //isAConstant

  bool NodeStatements::isFunctionCall()
  {

    if(m_node->isFunctionCall())
      return true;

    if(m_nodeNext)
      return m_nodeNext->isFunctionCall(); //side-effect

    return false;
  } //isFunctionCall

  bool NodeStatements::isExplicitReferenceCast()
  {
    if(m_node->isExplicitReferenceCast())
      return true;

    if(m_nodeNext)
      return m_nodeNext->isExplicitReferenceCast();
    return false;
  } //isExplicitReferenceCast

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
  } //countNavNodes

  bool NodeStatements::buildDefaultQuarkValue(u32& dqref)
  {
    bool aok = true;
    if(m_node)
      aok |= m_node->buildDefaultQuarkValue(dqref);
    if(m_nodeNext)
      aok |= m_nodeNext->buildDefaultQuarkValue(dqref);
    return aok;
  } //obuildDefaultQuarkValue

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

  void NodeStatements::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_node->packBitsInOrderOfDeclaration(offset); //updates offset

    if(m_nodeNext)
      m_nodeNext->packBitsInOrderOfDeclaration(offset);
  } //packBitsInOrderOfDeclaration

  void NodeStatements::printUnresolvedVariableDataMembers()
  {
    m_node->printUnresolvedVariableDataMembers(); //updates offset

    if(m_nodeNext)
      m_nodeNext->printUnresolvedVariableDataMembers();
  } //printUnresolvedVariableDataMembers

  void NodeStatements::printUnresolvedLocalVariables(u32 fid)
  {
    if(m_node)
      m_node->printUnresolvedLocalVariables(fid); //updates offset

    if(m_nodeNext)
      m_nodeNext->printUnresolvedLocalVariables(fid);
  } //printUnresolvedLocalVariables

  void NodeStatements::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    u32 max2 = depth;
    u32 max1 = depth;
    m_node->calcMaxDepth(max1, maxdepth, base);

    if(m_nodeNext)
      m_nodeNext->calcMaxDepth(max1, maxdepth, base);

    depth = max1 > max2 ? max1 : max2; //no change to maxdepth here
  } //calcMaxDepth

  void NodeStatements::genCode(File * fp, UlamValue& uvpass)
  {
    Locator nodeloc = getNodeLocation();
    m_state.outputTextAsComment(fp, nodeloc);
    m_state.m_locOfNextLineText = nodeloc; //during gen code here

    m_node->genCode(fp, uvpass);

    if(m_nodeNext)
      m_nodeNext->genCode(fp, uvpass);
  } //genCode

  void NodeStatements::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    m_node->genCodeToStoreInto(fp, uvpass);

    if(m_nodeNext)
      m_nodeNext->genCodeToStoreInto(fp, uvpass);
  } //genCodeToStoreInto

  void NodeStatements::genCodeExtern(File * fp, bool declOnly)
  {
    if(m_node)
      m_node->genCodeExtern(fp, declOnly);

    if(m_nodeNext)
      m_nodeNext->genCodeExtern(fp, declOnly);
  } //genCodeExtern

  void NodeStatements::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    m_node->generateUlamClassInfo(fp, declOnly, dmcount);

    if(m_nodeNext)
      m_nodeNext->generateUlamClassInfo(fp, declOnly, dmcount);
  } //generateUlamClassInfo

} //end MFM
