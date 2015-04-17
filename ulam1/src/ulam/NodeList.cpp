#include <stdio.h>
#include <assert.h>
#include "NodeList.h"
#include "CompilerState.h"

namespace MFM{

  NodeList::NodeList(CompilerState & state) : Node(state) { }

  NodeList::NodeList(const NodeList & ref) : Node(ref)
  {
    u32 numparams = ref.getNumberOfNodes();
    for(u32 i = 0; i < numparams; i++)
      {
	Node * anode = ref.getNodePtr(i);
	Node * clone = anode->instantiate();
	addNodeToList(clone);
      }
  }

  NodeList::~NodeList()
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	delete m_nodes[i];
	m_nodes[i] = NULL;
      }
    m_nodes.clear();
  }

  Node * NodeList::instantiate()
  {
    return new NodeList(*this);
  }

  void NodeList::updateLineage(NNO pno)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->updateLineage(pno);
      }
  } //updateLineage

  bool NodeList::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    bool rtnb = false;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	if(m_nodes[i] == oldnptr)
	  {
	    m_nodes[i] = newnptr;
	    rtnb = true;
	    break;
	  }
      }
    return rtnb;
  } //exchangeKids

  bool NodeList::findNodeNo(NNO n, Node *& foundNode)
  {
    bool rtnb = false;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	if(m_nodes[i]->findNodeNo(n, foundNode))
	  {
	    rtnb = true;
	    break;
	  }
      }
    return rtnb;
  } //findNodeNo

  void NodeList::printPostfix(File * fp)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->printPostfix(fp);
      }
  } //printPostfix

  void NodeList::print(File * fp)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->print(fp);
      }
  } //print

  const char * NodeList::getName()
  {
    return "list";
  }

  const std::string NodeList::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeList::checkAndLabelType()
  {
    UTI rtnuti = Void; //ok
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	UTI puti = m_nodes[i]->checkAndLabelType();
	if(!m_state.isComplete(puti))
	  rtnuti = Nav; //all or none
      }
    setNodeType(rtnuti);
    return rtnuti;
  } //checkAndLabelType

  void NodeList::countNavNodes(u32& cnt)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->countNavNodes(cnt);
      }
  } //countNavNodes

  EvalStatus NodeList::eval()
  {
    assert(0);
    return NORMAL;
  }

  void NodeList::addNodeToList(Node * argNode)
  {
    m_nodes.push_back(argNode);
  }

  u32 NodeList::getNumberOfNodes() const
  {
    return m_nodes.size();
  }

  Node * NodeList::getNodePtr(u32 n) const
  {
    assert(n < m_nodes.size());
    return m_nodes[n];
  }

  UTI NodeList::getNodeType(u32 n)
  {
    assert(n < m_nodes.size());
    return m_nodes[n]->getNodeType();
  } //getNodeType

} //MFM
