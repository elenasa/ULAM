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

  bool NodeListEmpty::initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask)
  {
    return true;
  }

  bool NodeListEmpty::buildArrayValueInitialization(BV8K& bvtmp)
  {
    UTI nuti = Node::getNodeType();
    assert(m_state.okUTItoContinue(nuti));
    if(nuti == Void)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
	return false;
      }
    return true; //all zeros for default primitives
  } //buildArrayValueInitialization

  bool NodeListEmpty::buildClassArrayValueInitialization(BV8K& bvtmp)
  {
    UTI nuti = Node::getNodeType();
    assert(m_state.okUTItoContinue(nuti));
    if(nuti == Void)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
	return false;
      }

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 arraysize = nut->getArraySize();
    assert(arraysize >= 0); //t3847

    u32 n = m_nodes.size();
    assert(n==0);

    bool rtnok = true;
    //fill in default class if nothing provided for a non-empty array
    if((arraysize > 0))
      {
	rtnok = m_state.getDefaultClassValue(nuti, bvtmp); //uses scalar uti
	n = 1; //ready to fall thru and propagate as needed
      }

    if(rtnok)
      {
	//propagate last value for any remaining items not initialized
	if((n > 0) && (n < (u32) arraysize))
	  {
	    u32 itemlen = nut->getBitSize();
	    BV8K lastbv;
	    bvtmp.CopyBV((n - 1) *  itemlen, 0, itemlen, lastbv); //frompos, topos, len, destBV
	    for(s32 i = n; i < arraysize; i++)
	      {
		lastbv.CopyBV(0, i * itemlen, itemlen, bvtmp);
	      }
	  }
      }
    return rtnok;
  } //buildClassArrayValueInitialization


} //MFM
