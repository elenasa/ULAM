#include <stdio.h>
#include "NodeClassId.h"
#include "CompilerState.h"

namespace MFM {

  NodeClassId::NodeClassId(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeStorageof(ofnode, nodetype, state), m_tmpvarSymbol(NULL) { }

  NodeClassId::NodeClassId(const NodeClassId& ref) : NodeStorageof(ref), m_tmpvarSymbol(NULL) { }

  NodeClassId::~NodeClassId()
  {
    delete m_tmpvarSymbol;
    m_tmpvarSymbol = NULL;
  }

  Node * NodeClassId::instantiate()
  {
    return new NodeClassId(*this);
  }

  void NodeClassId::printPostfix(File * fp)
  {
    std::ostringstream cidstr;
    cidstr << "[" << getClassId() << "]";
    u32 cid = m_state.m_pool.getIndexForDataString(cidstr.str());

    if(m_nodeOf)
      m_nodeOf->printPostfix(fp);
    else
      {
	fp->write(" ");
	fp->write(m_state.getUlamTypeNameBriefByIndex(getOfType()).c_str());
      }
    fp->write(m_state.m_pool.getDataAsString(cid).c_str());
  }

  const char * NodeClassId::getName()
  {
    return ".classid";
  }

  const std::string NodeClassId::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeClassId::safeToCastTo(UTI newType)
  {
    UTI nuti = getNodeType(); //UAtom for references; ow type of class/atom
    return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);
  } //safeToCastTo

  u32 NodeClassId::getClassId()
  {
    assert(getNodeType()==Unsigned);

    UTI oftype = NodeStorageof::getOfType();
    assert(m_state.okUTItoContinue(oftype));

    return m_state.getAClassRegistrationNumber(oftype); //assigned after c&l by culam
  } //getClassId

  UTI NodeClassId::checkAndLabelType()
  {
    UTI nuti = NodeStorageof::checkAndLabelType();

    UTI oftype = NodeStorageof::getOfType(); //deref'd
    if(m_state.okUTItoContinue(oftype))
      {
	if(!m_state.isAClass(oftype))
	    {
	      std::ostringstream msg;
	      msg << "Invalid non-class type provided: '";
	      if(m_nodeOf)
		msg << m_nodeOf->getName();
	      else
		msg << m_state.getUlamTypeNameBriefByIndex(getOfType()).c_str();
	      msg << getName() << "'";
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      nuti = Nav;
	    }
	else
	  nuti = Unsigned; //registry number assigned after c&l.
      }

    setNodeType(nuti);

    Node::setStoreIntoAble(TBOOL_FALSE);
    Node::setReferenceAble(TBOOL_FALSE);

    return getNodeType();
  } //checkAndLabelType

  EvalStatus NodeClassId::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    // quark or nonclass data member;
    evalNodeProlog(0); //new current node eval frame pointer

    makeRoomForSlots(1); //1 slot number

    u32 classid = getClassId();

    UlamValue uv = UlamValue::makeImmediate(Unsigned, classid, m_state);

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeClassId::evalToStoreInto()
  {
    return Node::evalToStoreInto();
  }

  void NodeClassId::genCode(File * fp, UVPass& uvpass)
  {
    UTI nuti = getNodeType();
    u32 classid = getClassId();

    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(m_state.getUlamTypeByIndex(nuti)->getTmpStorageTypeAsString().c_str()); //u32
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(Unsigned, tmpVarNum, TMPREGISTER).c_str());
    fp->write(" = ");
    fp->write_decimal_unsigned(classid);
    fp->write(";");
    GCNL;

    uvpass = UVPass::makePass(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, 0); //POS 0 rightjustified (atom-based).

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCode


  void NodeClassId::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    m_state.abortNotImplementedYet(); //when here??? see below..

    //lhs
    assert(getStoreIntoAble() == TBOOL_TRUE); //not so..why not!
    genCode(fp, uvpass); //t41085

    //tmp variable becomes the object of the constructor call (t41085)
    m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL);
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
  } //genCodeToStoreInto

  UlamValue NodeClassId::makeUlamValuePtr()
  {
    return Node::makeUlamValuePtr();
  }

} //end MFM
