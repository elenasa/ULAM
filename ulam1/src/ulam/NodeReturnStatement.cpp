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
    //assert(m_node); a return without an expression (i.e. Void)
    UTI nodeType = Void;
    if(m_node)
      nodeType = m_node->checkAndLabelType();

    if(m_state.isComplete(nodeType) && m_state.isComplete(m_state.m_currentFunctionReturnType))
      {
	if(UlamType::compare(nodeType, m_state.m_currentFunctionReturnType, m_state) != UTIC_SAME)
	  {
	    if(UlamType::compare(m_state.m_currentFunctionReturnType, Void, m_state) == UTIC_NOTSAME)
	      {
		UTI rtnType = m_state.m_currentFunctionReturnType;
		//if(m_node && checkForSafeImplicitCasting(m_state.m_currentFunctionReturnType, nodeType, rtnType)) //ref)
		if(m_node)
		  {
		    SAFECAST scr = m_node->safeToCastTo(nodeType);
		    if( scr == SAFE)
		      {
			assert(rtnType == m_state.m_currentFunctionReturnType); //are we ignoring cast change
			if(!makeCastingNode(m_node, m_state.m_currentFunctionReturnType, m_node))
			  nodeType = Nav;
			else
			  nodeType = m_node->getNodeType();
		      }
		    else
		      {
			std::ostringstream msg;
			msg << "Returning an mixed sign type: "; // mixed signs
			msg << m_state.getUlamTypeNameByIndex(nodeType).c_str();
			msg << " as ";
			msg << m_state.getUlamTypeNameByIndex(m_state.m_currentFunctionReturnType).c_str();
			msg << " requires explicit casting";
			if(scr == UNSAFE)
			  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			else
			  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			nodeType = Nav; //missing?
		      }
		  }
		else
		  nodeType = Nav;  //no casting node
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "ISO C forbids ‘return’ with expression, in function returning void";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		nodeType = Nav; //missing?
	      }
	  }
      } // not nav
    else if(!m_state.isComplete(nodeType))
      {
	std::ostringstream msg;
	msg << "Function return type is still incomplete: ";
	msg << m_state.getUlamTypeNameByIndex(nodeType).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	//	nodeType = Nav; needed???
      }

    //check later against defined function return type
    m_state.m_currentFunctionReturnNodes.push_back(this);

    setNodeType(nodeType); //return take type of their node
    return nodeType;
  } //checkAndLabelType

  bool NodeReturnStatement::checkAnyConstantsFit(ULAMTYPE ltypEnum, ULAMTYPE rtypEnum, UTI& newType)
  {
    bool rtnOK = true;
    bool rconst = m_node->isAConstant();

    if(rconst)
      {
	bool rready = rconst && m_node->isReadyConstant();

	// if one is a constant, check for value to fit in new type bits.
	bool doErrMsg = rready;

	if(rready && m_node->fitsInBits(newType))
	  doErrMsg = false;

	if(doErrMsg)
	  {
	    if(rready)
	      {
		std::ostringstream msg;
		msg << "Constant <";
		if(rready)
		  {
		    msg << m_node->getName();
		    msg <<  "> is not representable as return type: ";
		    msg << m_state.getUlamTypeNameByIndex(newType).c_str(); //was lt
		  }
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		newType = Nav; //for error
	      }
	  } //err
	rtnOK = !doErrMsg;
      } //a constant
    return rtnOK;
  } //checkAnyConstantsFit

  bool NodeReturnStatement::checkForMixedSignsOfVariables(ULAMTYPE ltypEnum, ULAMTYPE rtypEnum, UTI lt, UTI rt, UTI& newType)
  {
    bool rtnOK = true;
    ULAMTYPE ntypEnum = m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum();
    s32 nbs = m_state.getBitSize(newType);

    //Int to Unsigned of any size is unsafe!
    if(ntypEnum == Unsigned)
      {
	if(rtypEnum == Int && !m_node->isAConstant())
	  rtnOK = false;
      }
    else if(ntypEnum == Int)
      {
	// Unsigned to Int gets an error if the bitsizes are "unsafe"
	// (including the SAME size);
	if(rtypEnum == Unsigned && !m_node->isAConstant() && m_state.getBitSize(rt) >= nbs)
	  rtnOK = false;
      }

    if(!rtnOK)
      {
	std::ostringstream msg;
	msg << "Returning mixed signs type "; // mixed signs
	msg << m_state.getUlamTypeNameByIndex(rt).c_str();
	msg << " as ";
	msg << m_state.getUlamTypeNameByIndex(newType).c_str();
	msg << " requires explicit casting";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      } //mixing unsigned and signed
    return rtnOK;
  } //checkforMixedSignsOfVariables

  bool NodeReturnStatement::checkIntToNonBitsNonIntCast(ULAMTYPE rtypEnum, UTI rt, UTI& newType)
  {
    bool rtnOK = true;
    // Int to anything, except Bits or Int same or larger bitsize
    if(rtypEnum == Int && !m_node->isAConstant())
      {
	s32 rbs = m_state.getBitSize(rt);
	s32 nbs = m_state.getBitSize(newType);
	ULAMTYPE ntypEnum = m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum();
	if(!((ntypEnum == Bits || ntypEnum == Int) && nbs >= rbs))
	  rtnOK = false;
      }

    if(!rtnOK)
      {
	std::ostringstream msg;
	msg << "Returning Int "; //Int
	msg << m_state.getUlamTypeNameByIndex(rt).c_str();
	msg << " as ";
	msg << m_state.getUlamTypeNameByIndex(newType).c_str();
	msg << " requires explicit casting";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }
    return rtnOK;
  } //checkIntToNonBitsNonIntCast

  bool NodeReturnStatement::checkNonBoolToBoolCast(ULAMTYPE rtypEnum, UTI rt, UTI& newType)
  {
    ULAMTYPE ntypEnum = m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum();
    if(ntypEnum != Bool)
      return true;

    bool rtnOK = false;
    if(!m_node->isAConstant() && rtypEnum != ntypEnum)
      {
	if(m_state.getBitSize(rt) == 1 && (rtypEnum == Unsigned || rtypEnum == Unary))
	  rtnOK = true;
      }
    else
      rtnOK = true; //bools of any size are safe to cast

    if(!rtnOK)
      {
	std::ostringstream msg;
	msg << "Returning non-Bool "; //non-Bool
	msg << m_state.getUlamTypeNameByIndex(rt).c_str();
	msg << " as ";
	msg << m_state.getUlamTypeNameByIndex(newType).c_str();
	msg << " requires explicit casting";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }
    return rtnOK;
  } //checkNonBoolToBoolCast

  bool NodeReturnStatement::checkToUnaryCast(ULAMTYPE rtypEnum, UTI rt, UTI& newType)
  {
    UlamType * nit = m_state.getUlamTypeByIndex(newType);
    ULAMTYPE ntypEnum = nit->getUlamTypeEnum();
    if(ntypEnum != Unary)
      return true; //not to unary

    bool rtnOK = false;
    if(!m_node->isAConstant())
      {
	UlamType * rit = m_state.getUlamTypeByIndex(rt);
	if((rit->getMax() <= nit->getMax()) && (rit->getMin() == 0))
	  rtnOK = true;
      }
    else
      rtnOK = true;

    if(!rtnOK)
      {
	std::ostringstream msg;
	msg << "Returning ";
	msg << m_state.getUlamTypeNameByIndex(rt).c_str();
	msg << " as Unary "; //Unary
	msg << m_state.getUlamTypeNameByIndex(newType).c_str();
	msg << " requires explicit casting";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }
    return rtnOK;
  } //checkToUnaryCast

  bool NodeReturnStatement::checkBitsizeOfCastLast(ULAMTYPE rtypEnum, UTI rt, UTI& newType)
  {
    bool rtnOK = true;
    ULAMTYPE ntypEnum = m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum();
    // constants already checked; Any size Bool to Bool safe.
    // Atom may be larger than an element.
    if(!m_node->isAConstant() && (ntypEnum != Bool && rtypEnum != Bool) && (rtypEnum != UAtom))
      {
	if(m_state.getBitSize(rt) > m_state.getBitSize(newType))
	  {
	    std::ostringstream msg;
	    msg << "Returning ";
	    msg << m_state.getUlamTypeNameByIndex(rt).c_str();
	    msg << " as a smaller type ";
	    msg << m_state.getUlamTypeNameByIndex(newType).c_str();
	    msg << " requires explicit casting";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	    rtnOK = false;
	  }
      }
    return rtnOK;
  } //checkBitsizeOfCastLast

  void NodeReturnStatement::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt); //missing
    if(m_node)
      m_node->countNavNodes(cnt);
  }

  EvalStatus NodeReturnStatement::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

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

    if(nuti == UAtom)
      {
	//avoid pointer to atom situation
	UlamValue rtnUV = m_state.m_nodeEvalStack.popArg();
	assignReturnValueToStack(rtnUV, STACK);
      }
    else
      {
	//end, so copy to -1
	UlamValue rtnPtr = UlamValue::makePtr(1, EVALRETURN, nuti, m_state.determinePackable(nuti), m_state);  //positive to current frame pointer

	assignReturnValueToStack(rtnPtr, STACK); //uses STACK, unlike all the other nodes
      }

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
	m_node->genCode(fp, uvpass);
	UTI vuti = uvpass.getUlamValueTypeIdx();

	Node::genCodeConvertATmpVarIntoBitVector(fp, uvpass);

	m_state.indent(fp);
	fp->write("return ");
	fp->write("(");
	vuti = uvpass.getPtrTargetType();
	fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex(), uvpass.getPtrStorage()).c_str());

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
	fp->write("return;\n"); //void
      }
  } //genCode

} //end MFM
