#include <stdio.h>
#include "NodeReturnStatement.h"
#include "CompilerState.h"

namespace MFM {

  NodeReturnStatement::NodeReturnStatement(Node * s, CompilerState & state) : Node(state), m_node(s) {}

  NodeReturnStatement::NodeReturnStatement(const NodeReturnStatement& ref) : Node(ref)
  {
    if(ref.m_node)
      m_node = ref.m_node->instantiate();
    else
      m_node = NULL;
  }

  NodeReturnStatement::~NodeReturnStatement()
  {
    delete m_node;
    m_node = NULL;
  }

  Node * NodeReturnStatement::instantiate()
  {
    return new NodeReturnStatement(*this);
  }

  void NodeReturnStatement::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_node)
      m_node->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeReturnStatement::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_node == oldnptr)
      {
	m_node = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeReturnStatement::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_node && m_node->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeReturnStatement::checkAbstractInstanceErrors()
  {
    if(m_node)
      m_node->checkAbstractInstanceErrors();
  } //checkAbstractInstanceErrors

  void NodeReturnStatement::print(File * fp)
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

  void NodeReturnStatement::printPostfix(File * fp)
  {
    if(m_node)
      m_node->printPostfix(fp);
    else
      fp->write(" <EMPTYSTMT>");

    fp->write(" ");
    fp->write(getName());
  } //printPostfix

  const char * NodeReturnStatement::getName()
  {
    return "return";
  }

  const std::string NodeReturnStatement::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeReturnStatement::checkAndLabelType()
  {
    UTI rtnType = m_state.m_currentFunctionReturnType;
    //assert(m_node); a return without an expression (i.e. Void)
    UTI nodeType = Void;
    if(m_node)
      nodeType = m_node->checkAndLabelType();

    if(m_state.isComplete(nodeType) && m_state.isComplete(rtnType))
      {
	if(m_state.isScalar(nodeType) ^ m_state.isScalar(rtnType))
	  {
	    std::ostringstream msg;
	    msg << "Returning incompatible (nonscalar) types: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
	    msg << " as ";
	    msg << m_state.getUlamTypeNameBriefByIndex(rtnType).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    nodeType = Nav;
	  }
	else if(UlamType::compare(nodeType, rtnType, m_state) != UTIC_SAME)
	  {
	    if(UlamType::compare(rtnType, Void, m_state) == UTIC_NOTSAME)
	      {
		if(m_node)
		  {
		    if(m_node->isAConstant() && !m_node->isReadyConstant())
		      {
			m_node->constantFold();
		      }

		    FORECAST scr = m_node->safeToCastTo(rtnType);
		    if(scr == CAST_CLEAR)
		      {
			if(!Node::makeCastingNode(m_node, rtnType, m_node))
			  nodeType = Nav; //no casting node
			else
			  nodeType = m_node->getNodeType(); //casted
		      }
		    else
		      {
			std::ostringstream msg;
			if(m_state.getUlamTypeByIndex(rtnType)->getUlamTypeEnum() == Bool)
			  msg << "Use a comparison operator";
			else
			  msg << "Use explicit cast";
			msg << " to return ";
			msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
			msg << " as ";
			msg << m_state.getUlamTypeNameBriefByIndex(rtnType).c_str();
			if(scr == CAST_BAD)
			  {
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			    nodeType = Nav;
			  }
			else
			  {
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			    nodeType = Hzy;
			  }
		      }
		  }
	      } //no node
	    else
	      {
		std::ostringstream msg;
		msg << "ISO C forbids return with expression in a function returning void";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		nodeType = Nav; //missing?
	      }
	  } // not the same
      } // not nav
    else if((nodeType != Nav) && !m_state.isComplete(nodeType))
      {
	std::ostringstream msg;
	msg << "Function return type is still unresolved: ";
	msg << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	nodeType = Hzy; //needed?
      }

    //check later against defined function return type
    m_state.m_currentFunctionReturnNodes.push_back(this);

    if(nodeType == Hzy)
      m_state.setGoAgain();

    setNodeType(nodeType); //return take type of their node
    return nodeType;
  } //checkAndLabelType

  void NodeReturnStatement::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //missing
    if(m_node)
      m_node->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  EvalStatus NodeReturnStatement::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    if(!m_node)
      return RETURN;

    evalNodeProlog(0);
    makeRoomForNodeType(nuti);
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	assert(evs != CONTINUE && evs != BREAK);
	evalNodeEpilog();
	return evs;
      }

    if(m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum() == UAtom)
      {
	//avoid pointer to atom situation
	UlamValue rtnUV = m_state.m_nodeEvalStack.popArg();
	Node::assignReturnValueToStack(rtnUV, STACK);
      }
    else
      {
	//end, so copy to -1
	UlamValue rtnPtr = UlamValue::makePtr(1, EVALRETURN, nuti, m_state.determinePackable(nuti), m_state);  //positive to current frame pointer

	Node::assignReturnValueToStack(rtnPtr, STACK); //uses STACK, unlike all the other nodes
      }

    evalNodeEpilog();
    return RETURN;
  } //eval

  void NodeReturnStatement::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    if(m_node)
      m_node->calcMaxDepth(depth, maxdepth, base); //funccall?
    return; //work done by NodeStatements and NodeBlock
  }

  void NodeReturnStatement::genCode(File * fp, UlamValue& uvpass)
  {
    // return for void type has a NodeStatementEmpty m_node
    if(m_node && getNodeType() != Void)
      {
	m_node->genCode(fp, uvpass);
	UTI vuti = uvpass.getUlamValueTypeIdx();

	Node::genCodeConvertATmpVarIntoBitVector(fp, uvpass);

	m_state.indent(fp);
	fp->write("return ");
	fp->write("(");
	vuti = uvpass.getPtrTargetType();
	fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex(), uvpass.getPtrStorage()).c_str());

	fp->write(");\n");
      }
    else
      {
	m_state.indent(fp);
	fp->write("return;\n"); //void
      }
  } //genCode

} //end MFM
