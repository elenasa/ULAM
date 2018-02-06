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

  UTI NodeListEmpty::checkAndLabelType()
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

  UTI NodeListEmpty::constantFold()
  {
    m_state.abortShouldntGetHere();
    return Node::getNodeType();
  }

  void NodeListEmpty::genCode(File * fp, UVPass& uvpass)
  {
    //save before wipe out with each init dm; for local vars (o.w. empty)
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;

    m_state.indent(fp);
    fp->write("{ /* ");
    fp->write(getName());
    fp->write(" */}");
    GCNL;
  }

  void NodeListEmpty::genCodeConstantArrayInitialization(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void NodeListEmpty::generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly)
  {
    m_state.abortShouldntGetHere();
  }

  bool NodeListEmpty::initDataMembersConstantValue(BV8K& bvref)
  {
    return true;
  }

} //MFM
