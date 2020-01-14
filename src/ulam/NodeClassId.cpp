#include <stdio.h>
#include "NodeClassId.h"
#include "CompilerState.h"

namespace MFM {

  NodeClassId::NodeClassId(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeStorageof(ofnode, nodetype, state) { }

  NodeClassId::NodeClassId(const NodeClassId& ref) : NodeStorageof(ref) { }

  NodeClassId::~NodeClassId() { }

  Node * NodeClassId::instantiate()
  {
    return new NodeClassId(*this);
  }

  void NodeClassId::printPostfix(File * fp)
  {
    if(m_nodeOf)
      m_nodeOf->printPostfix(fp);
    else
      {
	fp->write(" ");
	fp->write(m_state.getUlamTypeNameBriefByIndex(getOfType()).c_str());
      }
    fp->write("[");
    fp->write_decimal_unsigned(getClassId());
    fp->write("u]");
  }

  const char * NodeClassId::getName()
  {
    return ".classid"; //for error msgs
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

  u32 NodeClassId::getClassIdAsStringIndex()
  {
    u32 cid = getClassId();

    std::ostringstream num;
    num << cid; //without u
    u32 cididx = m_state.m_pool.getIndexForDataString(num.str());
    return cididx; //like NodeTerminal
  } //getClassIdAsStringId

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
    return Node::evalToStoreInto(); //stub
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
    fp->write("u;");
    GCNL;

    uvpass = UVPass::makePass(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, 0); //POS 0 rightjustified (atom-based).
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCode

  void NodeClassId::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    m_state.abortNotImplementedYet(); //when here??? see NodeInstanceof..
  } //genCodeToStoreInto

  UlamValue NodeClassId::makeUlamValuePtr()
  {
    return Node::makeUlamValuePtr(); //stub
  }

} //end MFM
