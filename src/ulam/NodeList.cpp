#include <stdio.h>
#include <assert.h>
#include "NodeList.h"
#include "CompilerState.h"

namespace MFM{

  NodeList::NodeList(CompilerState & state) : Node(state)
  {
    m_nodes.clear(); //init
  }

  NodeList::NodeList(const NodeList & ref) : Node(ref)
  {
    m_nodes.clear(); //init
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
    clearNodeList();
  }

  void NodeList::clearNodeList()
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	delete m_nodes[i];
	m_nodes[i] = NULL;
      }
    m_nodes.clear();
  } //clearNodeList

  Node * NodeList::instantiate()
  {
    return new NodeList(*this);
  }

  void NodeList::updateLineage(NNO pno)
  {
    NNO nno = getNodeNo();
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	m_nodes[i]->updateLineage(nno); //wrong! was pno
      }
    Node::setYourParentNo(pno); //missing
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
    if(Node::findNodeNo(n, foundNode))
      return true;

    bool rtnb = false;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	if(m_nodes[i]->findNodeNo(n, foundNode))
	  {
	    rtnb = true;
	    break;
	  }
      }
    return rtnb;
  } //findNodeNo

  void NodeList::checkAbstractInstanceErrors()
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	m_nodes[i]->checkAbstractInstanceErrors();
      }
  }

  void NodeList::resetNodeLocations(Locator loc)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	m_nodes[i]->resetNodeLocations(loc);
      }
  }

  void NodeList::printPostfix(File * fp)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	m_nodes[i]->printPostfix(fp);
      }
  } //printPostfix

  void NodeList::print(File * fp)
  {
    fp->write("(");
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	if(i > 0)
	  fp->write(", ");

	fp->write(m_state.getUlamTypeNameBriefByIndex(m_nodes[i]->getNodeType()).c_str());
	fp->write(" ");
	fp->write(m_nodes[i]->getName());
      }
    fp->write(")");
  } //print

  const char * NodeList::getName()
  {
    return "list";
  }

  const std::string NodeList::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeList::safeToCastTo(UTI newType)
  {
    return CAST_BAD;
  }

  UTI NodeList::checkAndLabelType()
  {
    UTI rtnuti = Void;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	UTI puti = m_nodes[i]->checkAndLabelType();
	if(puti == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Argument " << i + 1 << " has a problem";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnuti = Nav;
	  }
	else if((rtnuti != Nav) && !m_state.isComplete(puti))
	  rtnuti = Hzy; // all or none
      }
    setNodeType(rtnuti);
    if(rtnuti == Hzy)
      m_state.setGoAgain(); //since no error msg
    return rtnuti;
  } //checkAndLabelType

  bool NodeList::foldArrayInitExpression()
  {
    return true;
  }

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
	assert(m_nodes[i]);
	m_nodes[i]->calcMaxDepth(max1, nomaxdepth, negrun); //side effects func def param (NodeVarDecl)
	negrun += max1;
      }
  } //calcMaxDepth

  void NodeList::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	m_nodes[i]->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
      }
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //NodeList counts!
  } //countNavNodes

  void NodeList::printUnresolvedLocalVariables(u32 fid)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	m_nodes[i]->printUnresolvedLocalVariables(fid);
      }
  } //printUnresolvedLocalVariables

  EvalStatus NodeList::eval()
  {
    EvalStatus evs = NORMAL;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	evs = eval(i); //for side-effects
	if(evs != NORMAL)
	  break;
      }
    return evs;
  } //eval

  EvalStatus NodeList::eval(u32 n)
  {
    assert(n < m_nodes.size());
    assert(m_nodes[n]);
    return m_nodes[n]->eval();
  }

  EvalStatus NodeList::evalToStoreInto(u32 n)
  {
    assert(n < m_nodes.size());
    assert(m_nodes[n]);
    return m_nodes[n]->evalToStoreInto();
  }

  void NodeList::addNodeToList(Node * argNode)
  {
    m_nodes.push_back(argNode);
  }

  u32 NodeList::getNumberOfNodes() const
  {
    return m_nodes.size();
  }

  bool NodeList::isEmptyList() const
  {
    assert(m_nodes.size() > 0); //not NodeListEmpty, right?
    return m_nodes.empty(); //size==0
  }

  u32 NodeList::getTotalSlotsNeeded()
  {
    u32 nslots = 0;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	nslots += m_state.slotsNeeded(m_nodes[i]->getNodeType());
      }
    return nslots;
  }

  Node * NodeList::getNodePtr(u32 n) const
  {
    assert(n < m_nodes.size());
    return m_nodes[n];
  }

  UTI NodeList::getNodeType(u32 n)
  {
    assert(n < m_nodes.size());
    assert(m_nodes[n]);
    return m_nodes[n]->getNodeType();
  }

  bool NodeList::isAConstant()
  {
    bool rtnc = true;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	rtnc &= isAConstant(i); //t41202 (empty constant array init)
      }
    return rtnc;
  }

  bool NodeList::isAConstant(u32 n)
  {
    assert(n < m_nodes.size());
    assert(m_nodes[n]);
    return m_nodes[n]->isAConstant();
  }

  bool NodeList::isFunctionCall(u32 n)
  {
    assert(n < m_nodes.size());
    assert(m_nodes[n]);
    return m_nodes[n]->isFunctionCall();
  }

  bool NodeList::isAList()
  {
    return true;
  }

  void NodeList::genCode(File * fp, UVPass& uvpass)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	m_nodes[i]->genCode(fp, uvpass);
      }
  }

  void NodeList::genCode(File * fp, UVPass& uvpass, u32 n)
  {
    assert(n < m_nodes.size());
    assert(m_nodes[n]);
    m_nodes[n]->genCode(fp, uvpass);
  }

  void NodeList::genCodeToStoreInto(File * fp, UVPass& uvpass, u32 n)
  {
    assert(n < m_nodes.size());
    assert(m_nodes[n]);
    m_nodes[n]->genCodeToStoreInto(fp, uvpass);
  }

  void NodeList::genCodeConstantArrayInitialization(File * fp)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	m_nodes[i]->genCodeConstantArrayInitialization(fp);
      }
  }

  void NodeList::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	m_nodes[i]->generateBuiltinConstantClassOrArrayInitializationFunction(fp, declOnly);
      }
  }

  bool NodeList::initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask)
  {
    m_state.abortShouldntGetHere();
    return false;
  }


  bool NodeList::buildArrayValueInitialization(BV8K& bvtmp)
  {
    m_state.abortShouldntGetHere();
    return false;
  }

  bool NodeList::buildClassArrayValueInitialization(BV8K& bvtmp)
  {
    m_state.abortShouldntGetHere();
    return false;
  }

} //MFM
