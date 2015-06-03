#include <stdio.h>
#include "NodeControlIf.h"
#include "CompilerState.h"

namespace MFM {

  NodeControlIf::NodeControlIf(Node * condNode, Node * trueNode, Node * falseNode, CompilerState & state): NodeControl(condNode, trueNode, state), m_nodeElse(falseNode) {}

  NodeControlIf::NodeControlIf(const NodeControlIf& ref) : NodeControl(ref)
  {
    if(ref.m_nodeElse)
      m_nodeElse = ref.m_nodeElse->instantiate();
    else
      m_nodeElse = NULL;
  }

  NodeControlIf::~NodeControlIf()
  {
    delete m_nodeElse;
    m_nodeElse = NULL;
  }

  Node * NodeControlIf::instantiate()
  {
    return new NodeControlIf(*this);
  }

  void NodeControlIf::updateLineage(NNO pno)
  {
    NodeControl::updateLineage(pno);
    if(m_nodeElse)
      m_nodeElse->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeControlIf::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeElse == oldnptr)
      {
	m_nodeElse = newnptr;
	return true;
      }
    return NodeControl::exchangeKids(oldnptr, newnptr);
  } //exhangeKids

  bool NodeControlIf::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeControl::findNodeNo(n, foundNode))
      return true;
    if(m_nodeElse && m_nodeElse->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeControlIf::print(File * fp)
  {
    NodeControl::print(fp);

    fp->write("else:\n");
    if(m_nodeElse)
      m_nodeElse->print(fp);
    else
      fp->write("<NULLFALSE>\n");

    fp->write("-----------------NodeControlif\n");
  } //print

  void NodeControlIf::printPostfix(File * fp)
  {
    NodeControl::printPostfix(fp);

    if(m_nodeElse)
      {
	m_nodeElse->printPostfix(fp);
	fp->write(" else"); //operators last
      }
    //else
    //  fp->write("<NULLFALSE>");
  } //printPostfix

  const char * NodeControlIf::getName()
  {
    return "if";
  }

  const std::string NodeControlIf::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeControlIf::checkAndLabelType()
  {
    NodeControl::checkAndLabelType(); //does condition and true

    if(m_nodeElse)
      {
	m_nodeElse->checkAndLabelType();
      }

    return getNodeType(); //Bool
  } //checkAndLabelType

  void NodeControlIf::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    if(m_nodeCondition)
      m_nodeCondition->calcMaxDepth(depth, maxdepth, base);

    u32 maxbody = depth;
    m_nodeBody->calcMaxDepth(maxbody, maxdepth, base);

    u32 maxelse = depth;
    if(m_nodeElse)
      m_nodeElse->calcMaxDepth(maxelse, maxdepth, base);

    depth += maxbody > maxelse ? maxbody : maxelse;
  } //calcMaxDepth

  void NodeControlIf::countNavNodes(u32& cnt)
  {
    NodeControl::countNavNodes(cnt);
    if(m_nodeElse)
      m_nodeElse->countNavNodes(cnt);
  } //countNavNodes

  EvalStatus  NodeControlIf::eval()
  {
    assert(m_nodeCondition && m_nodeBody);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(nuti);
    EvalStatus evs = m_nodeCondition->eval();
    if(evs != NORMAL) //what if RETURN
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue cuv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
    if((bool) cuv.getImmediateData(m_state) == false)
      {
	if(m_nodeElse)  //not necessarily
	  {
	    makeRoomForNodeType(m_nodeElse->getNodeType());
	    evs = m_nodeElse->eval();
	  }
      }
    else
      {
	makeRoomForNodeType(m_nodeBody->getNodeType());
	evs = m_nodeBody->eval();
      }

    // the type of this node is Bool for the condition since the
    // results may be of different types/sizes. Results need to be
    // stored in a variable somewhere.

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(cuv); //skip this for a break statement ???

    evalNodeEpilog();
    return evs;
  } //eval

  void NodeControlIf::genCode(File * fp, UlamValue& uvpass)
  {
#ifdef TMPVARBRACES
    m_state.indent(fp);
    fp->write("{\n");  //for overall tmpvars
    m_state.m_currentIndentLevel++;
#endif

    NodeControl::genCode(fp, uvpass);  //condition and body

    if(m_nodeElse)
      {
	m_state.indent(fp);
	fp->write("else\n");
	m_state.indent(fp);
	fp->write("{\n");
	m_state.m_currentIndentLevel++;

	m_nodeElse->genCode(fp, uvpass);

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //end else\n");
      }

#ifdef TMPVARBRACES
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");  //overall tmpvar
#endif
  } //genCode

} //end MFM
