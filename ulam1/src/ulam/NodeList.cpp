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

  bool NodeList::exchangeKids(Node * oldnptr, Node * newnptr, u32 n)
  {
    bool rtnb = false;
    assert(n < m_nodes.size());
    if(m_nodes[n] == oldnptr)
      {
         m_nodes[n] = newnptr;
	 rtnb = true;
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
	else if(!WritePacked(m_state.determinePackable(puti)) && !m_state.isScalar(puti))
	  {
	    std::ostringstream msg;
	    msg << "Function Definition parameter ";
	    msg << i+1 << ", type: ";
	    msg << m_state.getUlamTypeNameByIndex(puti).c_str();
	    msg << " (UTI" << puti << "), requires UNPACKED array support";
	    MSG(m_nodes[i]->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnuti = Nav;
	  }
      }
    setNodeType(rtnuti);
    return rtnuti;
  } //checkAndLabelType

  void NodeList::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    s32 negrun = -base;
    u32 nomaxdepth = 0;
    if(m_nodes.empty())
      return;
    //reverse order
    for(s32 i = m_nodes.size() - 1; i >= 0; i--)
      {
	u32 max1 = 0;
	m_nodes[i]->calcMaxDepth(max1, nomaxdepth, negrun);
	negrun += max1;
      }
  } //calcMaxDepth

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
  } //eval

  EvalStatus NodeList::eval(u32 n)
  {
    assert(n < m_nodes.size());
    return m_nodes[n]->eval();
  } //eval (overloaded)

  void NodeList::addNodeToList(Node * argNode)
  {
    m_nodes.push_back(argNode);
  } //addNodeToList

  u32 NodeList::getNumberOfNodes() const
  {
    return m_nodes.size();
  } //getNumberOfNodes

  u32 NodeList::getTotalSlotsNeeded()
  {
    u32 nslots = 0;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	nslots += m_state.slotsNeeded(m_nodes[i]->getNodeType());
      }
    return nslots;
  } //getTotalSlotsNeeded

  Node * NodeList::getNodePtr(u32 n) const
  {
    assert(n < m_nodes.size());
    return m_nodes[n];
  } //getNodePtr

  UTI NodeList::getNodeType(u32 n)
  {
    assert(n < m_nodes.size());
    return m_nodes[n]->getNodeType();
  } //getNodeType

  bool NodeList::isAConstant(u32 n)
  {
    assert(n < m_nodes.size());
    return m_nodes[n]->isAConstant();
  } //isAConstant

  void NodeList::genCode(File * fp, UlamValue& uvpass, u32 n)
  {
    assert(n < m_nodes.size());
    m_nodes[n]->genCode(fp, uvpass);
  } //genCode

} //MFM
