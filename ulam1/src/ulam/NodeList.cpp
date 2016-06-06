#include <stdio.h>
#include <assert.h>
#include "NodeList.h"
#include "CompilerState.h"

namespace MFM{

  NodeList::NodeList(CompilerState & state) : Node(state)
  {
    setNodeType(Void); //initialized to Void
  }

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

  void NodeList::checkAbstractInstanceErrors()
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      m_nodes[i]->checkAbstractInstanceErrors();
  } //checkAbstractInstanceErrors

  void NodeList::printPostfix(File * fp)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->printPostfix(fp);
      }
  } //printPostfix

  void NodeList::print(File * fp)
  {
    fp->write("(");
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
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

  UTI NodeList::checkAndLabelType()
  {
    UTI rtnuti = Node::getNodeType(); //init to Void; //ok
    if(rtnuti == Hzy)
      rtnuti = Void; //resets
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	UTI puti = m_nodes[i]->checkAndLabelType();
	if(puti == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Argument " << i + 1 << " has a problem";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnuti = Nav;
	  }
	else if((rtnuti != Nav) && !m_state.isComplete(puti))
	  {
	    rtnuti = Hzy; // all or none
	    m_state.setGoAgain(); //since no error msg
	  }
	else if(rtnuti != Void)
	  {
	    UTI scalaruti = m_state.getUlamTypeAsScalar(rtnuti);
	    if(UlamType::compareForArgumentMatching(puti, scalaruti, m_state) != UTIC_SAME)
	      {
		FORECAST scr = m_nodes[i]->safeToCastTo(scalaruti);
		if(scr == CAST_CLEAR)
		  {
		    if(!Node::makeCastingNode(m_nodes[i], scalaruti, m_nodes[i]))
		      {
			std::ostringstream msg;
			msg << "Argument " << i + 1 << " has a casting problem";
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			rtnuti = Nav; //no casting node
		      }
		    else
		      puti = m_nodes[i]->getNodeType(); //casted item
		  }
		else
		  {
		    std::ostringstream msg;
		    if(m_state.getUlamTypeByIndex(rtnuti)->getUlamTypeEnum() == Bool)
		      msg << "Use a comparison operator";
		    else
		      msg << "Use explicit cast";
		    msg << " to return ";
		    msg << m_state.getUlamTypeNameBriefByIndex(puti).c_str();
		    msg << " as ";
		    msg << m_state.getUlamTypeNameBriefByIndex(scalaruti).c_str();
		    if(scr == CAST_BAD)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			rtnuti = Nav;
		      }
		    else
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			rtnuti = Hzy; //Void?
		      }
		  }
	      }
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
	m_nodes[i]->calcMaxDepth(max1, nomaxdepth, negrun); //side effects func def param (NodeVarDecl)
	negrun += max1;
      }
  } //calcMaxDepth

  void NodeList::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
      }
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //NodeList counts!
  } //countNavNodes

  void NodeList::printUnresolvedLocalVariables(u32 fid)
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->printUnresolvedLocalVariables(fid);
      }
  } //printUnresolvedLocalVariables

  EvalStatus NodeList::eval()
  {
    EvalStatus evs = NORMAL;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	evs = eval(i);
	if(evs != NORMAL)
	  break;
      }
    return evs;
  } //eval

  EvalStatus NodeList::eval(u32 n)
  {
    assert(n < m_nodes.size());
    return m_nodes[n]->eval();
  } //eval (overloaded)

  EvalStatus NodeList::evalToStoreInto(u32 n)
  {
    assert(n < m_nodes.size());
    return m_nodes[n]->evalToStoreInto();
  } //evalToStoreInto (overloaded)

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

  bool NodeList::isFunctionCall(u32 n)
  {
    assert(n < m_nodes.size());
    return m_nodes[n]->isFunctionCall();
  } //isAFunctionCall

  void NodeList::genCode(File * fp, UVPass& uvpass)
  {
    UTI nuti = Node::getNodeType();
    assert(!m_state.isScalar(nuti));
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    PACKFIT packFit = nut->getPackable();
    if(packFit == PACKEDLOADABLE)
      {
	//u32 len = nut->getTotalBitSize();
	u32 wdsize = nut->getTotalWordSize();
	//u32 bitsize = nut->getBitSize();
	u32 n = m_nodes.size();

	std::vector<UVPass> uvpassList;
	for(u32 i = 0; i < n; i++)
	  {
	    UVPass uvp;
	    genCodeToStoreInto(fp, uvp, i);
	    uvpassList.push_back(uvp);
	  }

	u32 tmpvarnum = m_state.getNextTmpVarNumber();
	TMPSTORAGE nstor = nut->getTmpStorageTypeForTmpVar();
	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(nut->getTmpStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpvarnum, nstor).c_str());
	fp->write(" = ");

	for(u32 i = 0; i < n; i++)
	  {
	    UVPass uvp = uvpassList[i];
	    if(i > 0)
	      fp->write("| ");

	    fp->write("(");
	    fp->write(uvp.getTmpVarAsString(m_state).c_str());
	    fp->write(" << ((");
	    fp->write_decimal_unsigned(n); //(n - 1 - i) *
	    fp->write(" - 1 - ");
	    fp->write_decimal_unsigned(i);
	    fp->write(") * (");
	    fp->write_decimal_unsigned(wdsize); // 32/n
	    fp->write(" / ");
	    fp->write_decimal_unsigned(n);
	    fp->write("))) ");
	  }
	fp->write(";\n");
	uvpassList.clear();
	uvpass = UVPass::makePass(tmpvarnum, nstor, nuti, packFit, m_state, 0, 0); //POS 0 justified (atom-based).
      }
    else
      {
	//unpacked
	assert(0); //TBD
      }
  } //genCode

  void NodeList::genCode(File * fp, UVPass& uvpass, u32 n)
  {
    assert(n < m_nodes.size());
    m_nodes[n]->genCode(fp, uvpass);
  } //genCode

  void NodeList::genCodeToStoreInto(File * fp, UVPass& uvpass, u32 n)
  {
    assert(n < m_nodes.size());
    m_nodes[n]->genCodeToStoreInto(fp, uvpass);
  } //genCode

} //MFM
