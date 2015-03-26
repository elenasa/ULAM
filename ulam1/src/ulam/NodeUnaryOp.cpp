#include <stdio.h>
#include "NodeUnaryOp.h"
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
  }//updateLineage

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

  void NodeUnaryOp::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
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

    printOp(fp);  //operators last
  } //printPostfix

  void NodeUnaryOp::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }

  const std::string NodeUnaryOp::methodNameForCodeGen()
  {
    assert(0);
    return "_UNARY_NOOP";
  }

  bool NodeUnaryOp::isAConstant()
  {
    assert(m_node);
    return m_node->isAConstant();
  }

  bool NodeUnaryOp::isReadyConstant()
  {
    //needs constant folding
    return false;
  }

  UTI NodeUnaryOp::checkAndLabelType()
  {
    assert(0);  //see unary operators..
    assert(m_node);
    UTI ut = m_node->checkAndLabelType();
    UTI newType = ut;         // init to stay the same

    if(newType != Nav && m_state.isComplete(newType))
      {
	if(!m_state.isScalar(ut)) //array unsupported at this time
	  {
	    std::ostringstream msg;
	    msg << "Incompatible (nonscalar) type: ";
	    msg << m_state.getUlamTypeNameByIndex(ut).c_str();
	    msg << " for unary operator" << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
	else
	  {
	    ULAMTYPE eut = m_state.getUlamTypeByIndex(ut)->getUlamTypeEnum();
	    if(eut == Bool)
	      {
		newType = Int;
		m_node = makeCastingNode(m_node, newType);  //insert node/s
	      }
	  }
      } //not nav

    setNodeType(newType);
    setStoreIntoAble(false);

    if(isAConstant() && m_node->isReadyConstant())
      return constantFold();

    return newType;
  } //checkAndLabelType

  void NodeUnaryOp::countNavNodes(u32& cnt)
  {
    m_node->countNavNodes(cnt);
  }

  UTI NodeUnaryOp::constantFold()
  {
    u32 val;
    UTI nuti = getNodeType();

    if(m_state.m_parsingInProgress)
      return nuti;

    if(nuti == Nav) return Nav; //nothing to do yet

    // if here, must be a constant..
    assert(isAConstant());

    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(nuti); //offset a constant expression
    EvalStatus evs = eval();
    if( evs == NORMAL)
      {
	UlamValue cnstUV = m_state.m_nodeEvalStack.popArg();
	val = cnstUV.getImmediateData(m_state);
      }

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for unary op" << getName();
	msg << " is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	return Nav;
      }

    //replace ourselves (and kids) with a node terminal; new NNO unlike template's
    NodeTerminal * newnode = new NodeTerminal(val, nuti, m_state);
    assert(newnode);
    newnode->setNodeLocation(getNodeLocation());

    NNO pno = Node::getYourParentNo();
    assert(pno);
    Node * parentNode = m_state.findNodeNoInThisClass(pno);
    assert(parentNode);

    assert(parentNode->exchangeKids(this, newnode));

    std::ostringstream msg;
    msg << "Exchanged kids! for unary operator" << getName();
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

    UTI nut = getNodeType();
    evalNodeProlog(0); //new current frame pointer
    u32 slots = makeRoomForNodeType(nut);
    EvalStatus evs = m_node->eval();

    if(evs == NORMAL)
      doUnaryOperation(1,slots);

    evalNodeEpilog();
    return evs;
  } //eval

  void NodeUnaryOp::doUnaryOperation(s32 slot, u32 nslots)
  {
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doUnaryOperationImmediate(slot, nslots);
      }
    else
      { //arrays not supported at this time
	assert(0);
      }
  } //end dobinaryop

  void NodeUnaryOp::doUnaryOperationImmediate(s32 slot, u32 nslots)
  {
    assert(nslots == 1);
    UTI nuti = getNodeType();
    u32 len = m_state.getTotalBitSize(nuti);

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slot); //immediate value
    u32 data = uv.getImmediateData(len);
    UlamValue rtnUV = makeImmediateUnaryOp(nuti, data, len);
    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);
  } //end dounaryopImmediate

  void NodeUnaryOp::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_node);
    m_node->genCode(fp, uvpass);

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64..
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(getNodeType(),tmpVarNum).c_str());
    fp->write(" = ");

    fp->write(methodNameForCodeGen().c_str());
    fp->write("(");

    UTI uti = uvpass.getUlamValueTypeIdx();
    assert(uti == Ptr);

    fp->write(m_state.getTmpVarAsString(uvpass.getPtrTargetType(), uvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");
    fp->write_decimal(nut->getBitSize());

    fp->write(");\n");

    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0);  //POS 0 rightjustified.
  } //genCode

  void NodeUnaryOp::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    genCode(fp,uvpass);
  }

} //end MFM
