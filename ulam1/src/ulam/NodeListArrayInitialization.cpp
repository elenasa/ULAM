#include <stdio.h>
#include <assert.h>
#include "NodeListArrayInitialization.h"
#include "CompilerState.h"

namespace MFM{

  NodeListArrayInitialization::NodeListArrayInitialization(CompilerState & state) : NodeList(state)
  {
    setNodeType(Void); //initialized to Void
  }

  NodeListArrayInitialization::NodeListArrayInitialization(const NodeListArrayInitialization & ref) : NodeList(ref) { }

  NodeListArrayInitialization::~NodeListArrayInitialization() { }

  Node * NodeListArrayInitialization::instantiate()
  {
    return new NodeListArrayInitialization(*this);
  }

  const char * NodeListArrayInitialization::getName()
  {
    return "arrayinitlist";
  }

  const std::string NodeListArrayInitialization::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeListArrayInitialization::checkAndLabelType()
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
	else if(m_state.okUTItoContinue(rtnuti) && (rtnuti != Void))
	  {
	    // this clause added for array initializations
	    UTI scalaruti = m_state.getUlamTypeAsScalar(rtnuti);
	    if(UlamType::compareForArgumentMatching(puti, scalaruti, m_state) != UTIC_SAME)
	      {
		FORECAST scr = m_nodes[i]->safeToCastTo(scalaruti);
		if(scr == CAST_CLEAR)
		  {
#if 0
		    //don't complicate the parse tree
		    if(!Node::makeCastingNode(m_nodes[i], scalaruti, m_nodes[i]))
		      {
			std::ostringstream msg;
			msg << "Argument " << i + 1 << " has a casting problem";
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			rtnuti = Nav; //no casting node
		      }
		    else
		      puti = m_nodes[i]->getNodeType(); //casted item
#endif
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

  EvalStatus NodeListArrayInitialization::eval()
  {
    EvalStatus evs = NORMAL;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	evs = NodeList::eval(i);
	if(evs != NORMAL)
	  break;
      }
    return evs;
  } //eval

  void NodeListArrayInitialization::genCode(File * fp, UVPass& uvpass)
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

} //MFM
