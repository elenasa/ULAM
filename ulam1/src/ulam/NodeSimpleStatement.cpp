#include <stdio.h>
#include "NodeSimpleStatement.h"
#include "CompilerState.h"

namespace MFM {

  NodeSimpleStatement::NodeSimpleStatement(Node * s, CompilerState & state) : Node(state), m_node(s) {}

  NodeSimpleStatement::NodeSimpleStatement(const NodeSimpleStatement& ref) : Node(ref)
  {
    m_node = ref.m_node->instantiate();
  }

  NodeSimpleStatement::~NodeSimpleStatement()
  {
    delete m_node;
    m_node = NULL;
  }

  Node * NodeSimpleStatement::instantiate()
  {
    return new NodeSimpleStatement(*this);
  }

  void NodeSimpleStatement::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_node->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeSimpleStatement::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_node == oldnptr)
      {
	m_node = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeSimpleStatement::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_node && m_node->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeSimpleStatement::checkAbstractInstanceErrors()
  {
    if(m_node)
      m_node->checkAbstractInstanceErrors();
  } //checkAbstractInstanceErrors

  void NodeSimpleStatement::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    if(m_node)
      m_node->print(fp);
    else
      fp->write(" <EMPTYSTMT>\n");

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  } //print

  void NodeSimpleStatement::printPostfix(File * fp)
  {
    assert(m_node); //e.g. bad decl

    if(m_node)
      m_node->printPostfix(fp);
    else
      fp->write(" <EMPTYSTMT>");
  } //printPostfix

  const char * NodeSimpleStatement::getName()
  {
    return "simple"; //?
  }

  const std::string NodeSimpleStatement::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeSimpleStatement::checkAndLabelType()
  {
    assert(m_node);

    m_node->checkAndLabelType(); //side-effect

    //statements don't have types
    setNodeType(Void);
    return getNodeType();
  } //checkAndLabelType

  void NodeSimpleStatement::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    m_node->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  EvalStatus NodeSimpleStatement::eval()
  {
    assert(m_node);
    evalNodeProlog(0);
    makeRoomForNodeType(m_node->getNodeType());
    EvalStatus evs = m_node->eval();

    evalNodeEpilog();
    return evs;
  } //eval

  void NodeSimpleStatement::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_node);
    m_node->genCode(fp, uvpass);
  }

} //end MFM
