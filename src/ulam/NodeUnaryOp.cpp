#include <stdio.h>
#include "NodeUnaryOp.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "NodeTerminal.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOp::NodeUnaryOp(Node * n, CompilerState & state): Node(state), m_node(n) {}

  NodeUnaryOp::NodeUnaryOp(const NodeUnaryOp& ref) : Node(ref)
  {
    m_node = ref.m_node->instantiate();
  }

  NodeUnaryOp::~NodeUnaryOp()
  {
    delete m_node;
    m_node = NULL;
  }

  void NodeUnaryOp::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_node->updateLineage(getNodeNo());
  }

  bool NodeUnaryOp::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_node == oldnptr)
      {
	m_node = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeUnaryOp::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_node && m_node->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeUnaryOp::checkAbstractInstanceErrors()
  {
    if(m_node)
      m_node->checkAbstractInstanceErrors();
  }

  void NodeUnaryOp::resetNodeLocations(Locator loc)
  {
    Node::setNodeLocation(loc);
    if(m_node) m_node->resetNodeLocations(loc);
  }

  void NodeUnaryOp::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    if(m_node)
      m_node->print(fp);
    else
      fp->write("<NULL>\n");
    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  } //print

  void NodeUnaryOp::printPostfix(File * fp)
  {
    if(m_node)
      m_node->printPostfix(fp);
    else
      fp->write("<NULL>");

    printOp(fp); //operators last
  } //printPostfix

  void NodeUnaryOp::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }

  const std::string NodeUnaryOp::methodNameForCodeGen()
  {
    m_state.abortShouldntGetHere();
    return "_UNARY_NOOP";
  }

  bool NodeUnaryOp::isAConstant()
  {
    assert(m_node);
    return m_node->isAConstant();
  }

  bool NodeUnaryOp::isReadyConstant()
  {
    return false; //needs constant folding
  }

  FORECAST NodeUnaryOp::safeToCastTo(UTI newType)
  {
    //ulamtype checks for complete, non array, and type specific rules
    return m_state.getUlamTypeByIndex(newType)->safeCast(getNodeType());
  }

  UTI NodeUnaryOp::checkAndLabelType()
  {
    assert(m_node);
    UTI uti = m_node->checkAndLabelType();

    if(m_state.isComplete(uti) && !m_state.isScalar(uti)) //array unsupported at this time
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	msg << ", for unary " << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    //replace node with func call to matching function overload operator for class
    // of left, with no arguments for unary (t41???);
    // quark toInt must be used on rhs of operators (t3191, t3200, t3513, t3648,9)
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    if((ut->getUlamTypeEnum() == Class))
      {
	Node * newnode = buildOperatorOverloadFuncCallNode();
	if(newnode)
	  {
	    AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
	    assert(swapOk);

	    m_node = NULL; //recycle as memberselect

	    delete this; //suicide is painless..

	    return newnode->checkAndLabelType();
	  }
	//else should fail again as non-primitive;
      } //done

    UTI newType = Nav;
    if(uti != Nav)
      newType = calcNodeType(uti); //does safety check

    if(m_state.isComplete(newType))
      {
	if(UlamType::compareForMakingCastingNode(newType, uti, m_state) != UTIC_SAME) //not same|dontknow
	  {
	    if(!Node::makeCastingNode(m_node, newType, m_node))
	      newType = Nav;
	  }
      }
    else
      newType = Hzy;

    setNodeType(newType);
    if(newType == Hzy)
      m_state.setGoAgain(); //since not error
    Node::setStoreIntoAble(TBOOL_FALSE);

    if((newType != Nav) && isAConstant() && m_node->isReadyConstant())
      return constantFold();

    return newType;
  } //checkAndLabelType

  Node * NodeUnaryOp::buildOperatorOverloadFuncCallNode()
  {
    Token identTok;
    TokenType opTokType = Token::getTokenTypeFromString(getName());
    assert(opTokType != TOK_LAST_ONE);
    Token opTok(opTokType, getNodeLocation(), 0);
    u32 opolId = Token::getOperatorOverloadFullNameId(opTok, &m_state);
    if(opolId == 0)
      {
	std::ostringstream msg;
	msg << "Overload for operator <" << getName();
	msg << "> is not supported as operand for class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_node->getNodeType()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return NULL;
      }

    identTok.init(TOK_IDENTIFIER, getNodeLocation(), opolId);

    //fill in func symbol during type labeling;
    NodeFunctionCall * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
    assert(fcallNode);
    fcallNode->setNodeLocation(identTok.m_locator);

    //similar to Binary Op's except no argument

    NodeMemberSelect * mselectNode = new NodeMemberSelect(m_node, fcallNode, m_state);
    assert(mselectNode);
    mselectNode->setNodeLocation(identTok.m_locator);

    //redo check and type labeling done by caller!!
    return mselectNode; //replace right node with new branch
  } //buildOperatorOverloadFuncCallNode

  bool NodeUnaryOp::checkSafeToCastTo(UTI unused, UTI& newType)
  {
    bool rtnOK = true;
    FORECAST scr = m_node->safeToCastTo(newType);
    if(scr != CAST_CLEAR)
      {
	ULAMTYPE etyp = m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum();
	std::ostringstream msg;
	if(etyp == Bool)
	  msg << "Use a comparison operation";
	else if(etyp == String)
	  msg << "Invalid";
	else
	  msg << "Use explicit cast";
	msg << " to convert "; // the real converting-message
	msg << m_state.getUlamTypeNameBriefByIndex(m_node->getNodeType()).c_str();
	msg << " to ";
	msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
	msg << " for unary " << getName();
	if(scr == CAST_HAZY)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    newType = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
	rtnOK = false;
      } //not safe
    return rtnOK;
  } //checkSafeToCastTo

  s32 NodeUnaryOp::resultBitsize(UTI uti)
  {
    s32 newbs = m_state.getBitSize(uti);
    assert(m_state.okUTItoContinue(uti));
    ULAMCLASSTYPE ct = m_state.getUlamTypeByIndex(uti)->getUlamClassType();

    if(ct == UC_QUARK)
      newbs = MAXBITSPERINT; //32

    return newbs;
  } //resultBitsize

  //no atoms, elements as operand
  bool NodeUnaryOp::checkForPrimitiveType(UTI uti)
  {
    if(!m_state.getUlamTypeByIndex(uti)->isPrimitiveType())
      {
	std::ostringstream msg;
	msg << "Non-primitive type <";
	msg << m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	msg << "> is not supported for unary ";
	msg << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }
    return checkNotVoidType(uti);
  } //checkForPrimitiveType

  bool NodeUnaryOp::checkNotVoidType(UTI uti)
  {
    bool rtnOK = true;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum();
    if(typEnum == Void)
      {
	std::ostringstream msg;
	msg << "Void is not a supported type for unary ";
	msg << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnOK = false;
      }
    return rtnOK;
  } //checkNotVoidTypes

  bool NodeUnaryOp::checkForNumericType(UTI uti)
  {
    bool rtnOK = true;
    bool isnum = m_state.getUlamTypeByIndex(uti)->isNumericType();
    if(!isnum)
      {
	std::ostringstream msg;
	msg << "Incompatible type for unary ";
	msg << getName() << " : ";
	msg << m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	msg << "; Suggest casting to a numeric type first";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnOK = false;
      }
    return rtnOK;
  } //checkForNumericType

  void NodeUnaryOp::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //missing
    m_node->countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //no need to count self?
  }

  UTI NodeUnaryOp::constantFold()
  {
    u64 val = U64_MAX;
    UTI nuti = getNodeType();

    if(nuti == Nav) return Nav; //nothing to do yet

    //if(nuti == Hzy) return Hzy; //nothing to do yet TRY?

    // if here, must be a constant..
    assert(isAConstant());

    NNO pno = Node::getYourParentNo();
    assert(pno);
    Node * parentNode = m_state.findNodeNoInThisClassForParent(pno); //t3767
    assert(parentNode);

    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(nuti); //offset a constant expression
    EvalStatus evs = eval();
    if( evs == NORMAL)
      {
	UlamValue cnstUV = m_state.m_nodeEvalStack.popArg();
	u32 wordsize = m_state.getTotalWordSize(nuti);
	if(wordsize <= MAXBITSPERINT)
	  val = cnstUV.getImmediateData(m_state);
	else if(wordsize <= MAXBITSPERLONG)
	  val = cnstUV.getImmediateDataLong(m_state);
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for unary op" << getName();
	msg << " is erroneous while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    if(evs == NOTREADY)
      {
	std::ostringstream msg;
	msg << "Constant value expression for unary op" << getName();
	msg << " is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	setNodeType(Hzy);
	m_state.setGoAgain(); //for compiler counts
	return Hzy;
      }

    //replace ourselves (and kids) with a node terminal; new NNO unlike template's
    NodeTerminal * newnode = new NodeTerminal(val, nuti, m_state);
    assert(newnode);
    newnode->setNodeLocation(getNodeLocation());

    AssertBool swapOk = parentNode->exchangeKids(this, newnode);
    assert(swapOk);

    std::ostringstream msg;
    msg << "Exchanged kids! for unary " << getName();
    msg << ", with a constant == " << newnode->getName();
    msg << " while compiling class: ";
    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

    newnode->setYourParentNo(pno);
    newnode->resetNodeNo(getNodeNo());

    delete this; //suicide is painless..

    return newnode->checkAndLabelType();
  } //constantFold

  bool NodeUnaryOp::assignClassArgValueInStubCopy()
  {
    return m_node->assignClassArgValueInStubCopy();
  }

  EvalStatus NodeUnaryOp::eval()
  {
    assert(m_node);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    evalNodeProlog(0); //new current frame pointer
    u32 slots = makeRoomForNodeType(nuti);
    EvalStatus evs = m_node->eval();

    if(evs == NORMAL)
      if(!doUnaryOperation(1,slots))
	evs = ERROR;

    evalNodeEpilog();
    return evs;
  } //eval

  bool NodeUnaryOp::doUnaryOperation(s32 slot, u32 nslots)
  {
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti))  //not an array
      {
	return doUnaryOperationImmediate(slot, nslots);
      }
    else
      {
	//arrays not supported at this time
	std::ostringstream msg;
	msg << "Unsupported unary operation " << getName();
	msg << ", with an array type <";
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return false;
  } //dobinaryop

  bool NodeUnaryOp::doUnaryOperationImmediate(s32 slot, u32 nslots)
  {
    assert(nslots == 1);
    UTI nuti = getNodeType();
    u32 len = m_state.getTotalBitSize(nuti);

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slot); //immediate valueg

    if((uv.getUlamValueTypeIdx() == Nav) || (nuti == Nav))
      return false;

    if((uv.getUlamValueTypeIdx() == Hzy) || (nuti == Hzy))
      return false;

    UlamValue rtnUV;
    u32 wordsize = m_state.getUlamTypeByIndex(nuti)->getTotalWordSize();
    if(wordsize <= MAXBITSPERINT)
      {
	u32 data = uv.getImmediateData(len, m_state);
	rtnUV = makeImmediateUnaryOp(nuti, data, len);
      }
    else if(wordsize <= MAXBITSPERLONG)
      {
	u64 data = uv.getImmediateDataLong(len, m_state); //t3849
	rtnUV = makeImmediateLongUnaryOp(nuti, data, len);
      }
    else
      m_state.abortGreaterThanMaxBitsPerLong();

    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);
    return true;
  } //dounaryopImmediate

  void NodeUnaryOp::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    assert(m_node);
    m_node->calcMaxDepth(depth, maxdepth, base); //funccall?
    return; //work done by NodeStatements and NodeBlock
  }

  void NodeUnaryOp::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_node);
    m_node->genCode(fp, uvpass);

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64..
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPREGISTER).c_str());
    fp->write(" = ");

    fp->write(methodNameForCodeGen().c_str());
    fp->write("(");
    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    fp->write(", ");
    fp->write_decimal(nut->getBitSize());
    fp->write(");"); GCNL;

    uvpass = UVPass::makePass(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, 0); //POS 0 rightjustified.
  } //genCode

  void NodeUnaryOp::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    genCode(fp,uvpass);
  }

} //end MFM
