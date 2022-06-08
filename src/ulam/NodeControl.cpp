#include <stdio.h>
#include "NodeControl.h"
#include "NodeBlockSwitch.h"
#include "CompilerState.h"
#include "UlamTypePrimitiveBool.h"

namespace MFM {

  NodeControl::NodeControl(Node * condNode, Node * trueNode, CompilerState & state): Node(state), m_nodeCondition(condNode), m_nodeBody(trueNode) {}

  NodeControl::NodeControl(const NodeControl& ref) : Node(ref)
  {
    m_nodeCondition = ref.m_nodeCondition->instantiate();
    m_nodeBody = ref.m_nodeBody->instantiate();
  }

  NodeControl::~NodeControl()
  {
    delete m_nodeCondition;
    m_nodeCondition = NULL;
    delete m_nodeBody;
    m_nodeBody = NULL;
  }

  void NodeControl::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_nodeCondition->updateLineage(getNodeNo());
    m_nodeBody->updateLineage(getNodeNo());
  }

  bool NodeControl::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeCondition == oldnptr)
      {
	m_nodeCondition = newnptr;
	return true;
      }
    if(m_nodeBody == oldnptr)
      {
	m_nodeBody = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeControl::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeCondition->findNodeNo(n, foundNode))
      return true;
    return m_nodeBody->findNodeNo(n, foundNode);
  } //findNodeNo

  void NodeControl::checkAbstractInstanceErrors()
  {
    m_nodeCondition->checkAbstractInstanceErrors();
    m_nodeBody->checkAbstractInstanceErrors();
  }

  void NodeControl::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n",prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    fp->write("condition:\n");
    if(m_nodeCondition)
      m_nodeCondition->print(fp);
    else
      fp->write("<NULLCOND>\n");

    sprintf(id,"%s:\n",getName());
    fp->write(id);

    if(m_nodeBody)
      m_nodeBody->print(fp);
    else
      fp->write("<NULLTRUE>\n");
  } //print

  void NodeControl::printPostfix(File * fp)
  {
    if(m_nodeCondition)
      m_nodeCondition->printPostfix(fp);
    else
      fp->write("<NULLCONDITION>");

    fp->write(" cond");

    if(m_nodeBody)
      m_nodeBody->printPostfix(fp);
    else
      fp->write("<NULLTRUE>");

    printOp(fp);  //operators last
  } //printPostfix

  void NodeControl::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  } //printOp

  UTI NodeControl::checkAndLabelType(Node * thisparentnode)
  {
    assert(m_nodeCondition && m_nodeBody);
    UTI newType = Bool;
    ULAMTYPE newEnumTyp = Bool;

    // condition should be a bool, safely cast
    UTI cuti = m_nodeCondition->checkAndLabelType(this);
    if(m_state.okUTItoContinue(cuti) && m_state.isComplete(cuti))
      {
	assert(m_state.isScalar(cuti));
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	ULAMTYPE ctypEnum = cut->getUlamTypeEnum();
	if(ctypEnum != newEnumTyp)
	  {
	    if(checkSafeToCastTo(cuti, newType))
	      {
		if(!Node::makeCastingNode(m_nodeCondition, newType, m_nodeCondition))
		  newType = Nav;
	      }
	    //else not safe, newType changed
	  }
	else
	  {
	    //caution: c&l called multiple times
	    //always cast: Bools are maintained as unsigned in gen code,
	    //until c-bool is needed
	    if(cuti != newType)
	      {
		if(checkSafeToCastTo(cuti, newType))
		  {
		    if(!Node::makeCastingNode(m_nodeCondition, cuti, m_nodeCondition))
		      newType = Nav;
		    else
		      newType = cuti;
		  }
	      }
	  }
      }
    else
      newType = Hzy; //was = cuti;

    //don't do body when condition is error, or isn't ready and as-cond (t3924)
    if((newType != Nav) && ((newType != Hzy) || !m_nodeCondition->asConditionalNode()))
      m_nodeBody->checkAndLabelType(this); //side-effect

    setNodeType(newType); //stays the same
    if(newType == Hzy) m_state.setGoAgain();
    Node::setStoreIntoAble(TBOOL_FALSE);
    return getNodeType();
  } //checkAndLabelType

  void NodeControl::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    if(m_nodeCondition)
      m_nodeCondition->calcMaxDepth(depth, maxdepth, base);

    m_nodeBody->calcMaxDepth(depth, maxdepth, base);
  }

  void NodeControl::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //missing
    m_nodeCondition->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    m_nodeBody->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  void NodeControl::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeCondition && m_nodeBody);

    NodeBlock * currblock = m_state.getCurrentBlock();
    assert(currblock);
    bool isdefaultswcase = currblock->isASwitchBlock() && (((NodeBlockSwitch*) currblock)->getDefaultCaseNodeNo() == m_nodeCondition->getNodeNo());

    //bracket for overall tmpvars (e.g. condition and body) is
    // generated by the specific control; e.g. final bracket depends
    // on presence of 'else'.
    Symbol * saveyourself = m_state.getCurrentSelfSymbolForCodeGen();

    if(!isdefaultswcase)
      {
	m_nodeCondition->genCode(fp, uvpass);

	//saveyourself = m_state.getCurrentSelfSymbolForCodeGen();
	bool isTerminal = (uvpass.getPassStorage() == TERMINAL);
	UTI cuti = uvpass.getPassTargetType();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);

	m_state.indentUlamCode(fp);
	fp->write(getName());  //if, while
	fp->write("(");

	if(isTerminal)
	  {
	    fp->write(m_state.m_pool.getDataAsString(uvpass.getPassNameId()).c_str());
	  }
	else
	  {
	    //regular condition
	    assert(cut->getUlamTypeEnum() == Bool);
	    fp->write(((UlamTypePrimitiveBool *) cut)->getConvertToCboolMethod().c_str());
	    fp->write("(");
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(", ");
	    fp->write_decimal(cut->getBitSize());
	    fp->write(")");
	  }
	fp->write(")"); GCNL;

	m_state.indentUlamCode(fp);
	fp->write("{\n");
	m_state.m_currentIndentLevel++;
      }
    //else default is always true, do body

    //note: in case of has-conditional, uvpass still has the tmpvariable containing the pos!
    m_nodeBody->genCode(fp, uvpass);

    if(!isdefaultswcase)
      {
	m_state.m_currentIndentLevel--;
	m_state.indentUlamCode(fp);
	fp->write("} // end ");
	fp->write(getName()); //end if,while
	fp->write("\n");
      }
    else
      {
	m_state.indentUlamCode(fp);
	fp->write("// end default case"); GCNL; //t41038
      }

    m_state.m_currentSelfSymbolForCodeGen = saveyourself; //restore self (e.g. t3831)
  } //genCode

} //end MFM
