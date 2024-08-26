#include <stdio.h>
#include <assert.h>
#include "NodeInitDM.h"
#include "NodeListEmpty.h"
#include "CompilerState.h"

namespace MFM{

  NodeListEmpty::NodeListEmpty(CompilerState & state) : NodeList(state) { }

  NodeListEmpty::NodeListEmpty(const NodeListEmpty & ref) : NodeList(ref) { }

  NodeListEmpty::~NodeListEmpty() { }

  Node * NodeListEmpty::instantiate()
  {
    return new NodeListEmpty(*this);
  }

  void NodeListEmpty::printPostfix(File * fp)
  {
    fp->write(" { }");
  } //printPostfix

  void NodeListEmpty::print(File * fp)
  {
    fp->write("{}");
  } //print

  const char * NodeListEmpty::getName()
  {
    return "emptylist";
  }

  const std::string NodeListEmpty::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeListEmpty::isEmptyList() const
  {
    return true;
  }

  FORECAST NodeListEmpty::safeToCastTo(UTI newType)
  {
    return CAST_CLEAR;
  }

  UTI NodeListEmpty::checkAndLabelType(Node * thisparentnode)
  {
    setNodeType(Void);
    return Void;
  } //checkAndLabelType

  EvalStatus NodeListEmpty::evalToStoreInto(u32 n)
  {
    m_state.abortShouldntGetHere();
    return ERROR;
  }

  void NodeListEmpty::addNodeToList(Node * argNode)
  {
    m_state.abortShouldntGetHere(); //misses the point of empty!
  }

  void NodeListEmpty::printUnresolvedLocalVariables(u32 fid)
  {
    m_state.abortShouldntGetHere();
  } //printUnresolvedLocalVariables

  void NodeListEmpty::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return; //override NodeList
  } //calcMaxDepth

  bool NodeListEmpty::isAConstant()
  {
    return true;
  }

  UTI NodeListEmpty::foldConstantExpression()
  {
    return Node::getNodeType();
  }

  UTI NodeListEmpty::constantFold(Node * parentnode)
  {
    m_state.abortShouldntGetHere();
    return Node::getNodeType();
  }

  void NodeListEmpty::genCode(File * fp, UVPass& uvpass)
  {
    m_state.indentUlamCode(fp);
    fp->write("{ /* ");
    fp->write(getName());
    fp->write(" */}");
    GCNL;
  }

  void NodeListEmpty::genCodeConstantArrayInitialization(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void NodeListEmpty::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {
    m_state.abortShouldntGetHere();
  }

  bool NodeListEmpty::initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask)
  {
    return true;
  }

  bool NodeListEmpty::buildArrayValueInitialization(BV8K& bvtmp)
  {
    UTI nuti = Node::getNodeType();
    NODE_ASSERT(m_state.okUTItoContinue(nuti));
    if(nuti == Void)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
	return false;
      }
    if(m_state.isAClass(nuti))
      {
	return buildClassArrayValueInitialization(bvtmp); //t41262 (may be too soon)
      }
    return true; //all zeros for default primitives
  } //buildArrayValueInitialization

  //given bvtmp has arraysize copies of default class value;
  bool NodeListEmpty::buildClassArrayValueInitialization(BV8K& bvtmp)
  {
    return true;
  } //buildClassArrayValueInitialization

} //MFM
