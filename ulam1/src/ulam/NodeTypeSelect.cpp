#include <stdlib.h>
#include "NodeTypeSelect.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeSelect::NodeTypeSelect(NodeTypeSelect * node, TypeArgs args, CompilerState & state) : Node(state), m_nodeSelect(node), m_typeargs(args), m_ready(false)
  {
    m_nodeSelect->updateLineage(getNodeNo()); //for unknown subtrees
  }

  NodeTypeSelect::NodeTypeSelect(const NodeTypeSelect& ref) : Node(ref), m_typeargs(ref.m_typeargs), m_ready(false)
  {
    m_nodeSelect = (NodeTypeSelect *) ref.m_nodeSelect->instantiate();
  }

  NodeTypeSelect::~NodeTypeSelect()
  {
    delete m_nodeSelect;
    m_nodeSelect = NULL;
  }

  Node * NodeTypeSelect::instantiate()
  {
    return new NodeTypeSelect(*this);
  }

  void NodeTypeSelect::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_nodeSelect->updateLineage(getNodeNo());
  }//updateLineage

  bool NodeTypeSelect::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeSelect == (NodeTypeSelect *) oldnptr)
      {
	m_nodeSelect = (NodeTypeSelect *) newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeTypeSelect::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeSelect && m_nodeSelect->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypeSelect::printPostfix(File * fp)
  {
    m_nodeSelect->printPostfix(fp);
  }

  const char * NodeTypeSelect::getName()
  {
    return ".";
  }

  const std::string NodeTypeSelect::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeTypeSelect::isReadyType()
  {
    return m_ready;
  }

  UTI NodeTypeSelect::checkAndLabelType()
  {
    if(isReadyType())
      return getNodeType();


    UTI it = m_nodeSelect->checkAndLabelType();

    m_ready = it != Nav; // set

    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  void NodeTypeSelect::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    m_nodeSelect->countNavNodes(cnt);
  }

  bool NodeTypeSelect::assignClassArgValueInStubCopy()
  {
    //return m_nodeSelect->assignClassArgValueInStubCopy();
    return true;
  }

  EvalStatus NodeTypeSelect::eval()
  {
    assert(0);  //not in parse tree; part of symbol's type
    return NORMAL;
  }

} //end MFM
