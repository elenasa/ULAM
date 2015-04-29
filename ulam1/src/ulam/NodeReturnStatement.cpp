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
  }//updateLineage

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

  void NodeReturnStatement::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
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
    assert(m_node);    //e.g. bad decl

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
    assert(m_node);
    UTI nodeType = m_node->checkAndLabelType();
    if(nodeType != Nav && m_state.isComplete(nodeType) && m_state.isComplete(m_state.m_currentFunctionReturnType))
      {
	if(UlamType::compare(nodeType, m_state.m_currentFunctionReturnType, m_state) != UTIC_SAME)
	  {
	    if(m_state.m_currentFunctionReturnType != Void)
	      {
		m_node = makeCastingNode(m_node, m_state.m_currentFunctionReturnType);
		if(m_node)
		  nodeType = m_node->getNodeType();
		else
		  nodeType = Nav;  //no casting node
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "ISO C forbids ‘return’ with expression, in function returning void";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
      } // not nav
    else if(!m_state.isComplete(nodeType))
      {
	std::ostringstream msg;
	msg << "Function return type is still incomplete: ";
	msg << m_state.getUlamTypeNameByIndex(nodeType).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	nodeType = Nav;
      }

    //check later against defined function return type
    m_state.m_currentFunctionReturnNodes.push_back(this);

    setNodeType(nodeType); //return take type of their node
    return nodeType;
  } //checkAndLabelType

  void NodeReturnStatement::countNavNodes(u32& cnt)
  {
    if(m_node)
      m_node->countNavNodes(cnt);
  }

  EvalStatus NodeReturnStatement::eval()
  {
    assert(m_node);

    evalNodeProlog(0);
    makeRoomForNodeType(getNodeType());
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	assert(evs != CONTINUE && evs != BREAK);
	evalNodeEpilog();
	return evs;
      }

    //end, so copy to -1
    //UlamValue rtnPtr(getNodeType(), 1, true, EVALRETURN);  //positive to current frame pointer
    UlamValue rtnPtr = UlamValue::makePtr(1, EVALRETURN, getNodeType(), m_state.determinePackable(getNodeType()), m_state);  //positive to current frame pointer

    assignReturnValueToStack(rtnPtr, STACK); //uses STACK, unlike all the other nodes
    evalNodeEpilog();
    return RETURN;
  } //eval

  void NodeReturnStatement::genCode(File * fp, UlamValue& uvpass)
  {
    // return for void type has a NodeStatementEmpty m_node
    if(m_node && getNodeType() != Void)
      {
#ifdef TMPVARBRACES
	m_state.indent(fp);
	fp->write("{\n");
	m_state.m_currentIndentLevel++;
#endif
	//m_node->genCodeToStoreInto(fp, uvpass);
	m_node->genCode(fp, uvpass);
	UTI vuti = uvpass.getUlamValueTypeIdx();
	//bool isTerminal = (vuti != Ptr);

	Node::genCodeConvertATmpVarIntoBitVector(fp, uvpass);

	//if(isTerminal)
	//  {
	//    // write out terminal explicitly
	//    m_state.indent(fp);
	//    fp->write("return ");
	//    fp->write("(");
	//    u32 data = uvpass.getImmediateData(m_state);
	//    char dstr[40];
	//    m_state.getUlamTypeByIndex(vuti)->getDataAsString(data, dstr, 'z', m_state);
	//    fp->write(dstr);
	//  }
	//else
	  {
	    m_state.indent(fp);
	    fp->write("return ");
	    fp->write("(");
	    vuti = uvpass.getPtrTargetType();
	    fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex(), uvpass.getPtrStorage()).c_str());
	  }

	fp->write(")");
	fp->write(";\n");

#ifdef TMPVARBRACES
	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("}\n");
#endif
      }
    else
      {
	m_state.indent(fp);
	fp->write("return;\n");   //void
      }
  } //genCode

} //end MFM
