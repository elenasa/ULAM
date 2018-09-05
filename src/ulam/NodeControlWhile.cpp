#include <stdio.h>
#include "NodeControlWhile.h"
#include "UlamTypePrimitiveBool.h"
#include "CompilerState.h"

namespace MFM {

  NodeControlWhile::NodeControlWhile(Node * condNode, Node * trueNode, CompilerState & state): NodeControl(condNode, trueNode, state) {}

  NodeControlWhile::NodeControlWhile(const NodeControlWhile& ref) : NodeControl(ref) {}

  NodeControlWhile::~NodeControlWhile() {}

  Node * NodeControlWhile::instantiate()
  {
    return new NodeControlWhile(*this);
  }

  void NodeControlWhile::print(File * fp)
  {
    NodeControl::print(fp);

    fp->write("-----------------NodeControlwhile\n");
  }

  const char * NodeControlWhile::getName()
  {
    return "while";
  }

  const std::string NodeControlWhile::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  EvalStatus  NodeControlWhile::eval()
  {
    assert(m_nodeCondition && m_nodeBody);
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(nuti);
    EvalStatus evs = m_nodeCondition->eval();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue cuv = m_state.m_nodeEvalStack.popArg();

    while((bool) cuv.getImmediateData(m_state) == true)
      {
	u32 slots = makeRoomForNodeType(m_nodeBody->getNodeType());
 	evs = m_nodeBody->eval(); //side-effect
	if(evs == BREAK)
	  break; //use C to break out of this loop
	else if((evs == RETURN) || (evs == ERROR) || (evs == UNEVALUABLE))
	  return evalStatusReturn(evs);

	assert((evs == NORMAL) || (evs == CONTINUE));

	//continue continues as normal
	m_state.m_nodeEvalStack.popArgs(slots);

	makeRoomForNodeType(nuti);

	evs = m_nodeCondition->eval();
	if(evs != NORMAL) return evalStatusReturn(evs);

	cuv = m_state.m_nodeEvalStack.popArg();
      } //end while

    //also copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(cuv); //always false

    evalNodeEpilog();
    return NORMAL;
  } //eval

  void NodeControlWhile::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeCondition && m_nodeBody);

    //while true..
    m_state.indentUlamCode(fp);
    fp->write(getName());
    fp->write("(true)\n");

    m_state.indentUlamCode(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;

    //if !condition break;
    m_nodeCondition->genCode(fp, uvpass);

    bool isTerminal = (uvpass.getPassStorage() == TERMINAL);
    UTI cuti = uvpass.getPassTargetType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    fp->write("\n");
    m_state.indentUlamCode(fp);
    fp->write("if(!");

    if(isTerminal)
      {
	fp->write(m_state.m_pool.getDataAsString(uvpass.getPassNameId()).c_str());
      }
    else
      {
	assert(cut->getUlamTypeEnum() == Bool);
	fp->write(((UlamTypePrimitiveBool *) cut)->getConvertToCboolMethod().c_str());
	fp->write("(");
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(", ");
	fp->write_decimal(cut->getBitSize());
	fp->write(")");
      }
    fp->write(")\n");
    m_state.m_currentIndentLevel++;

    m_state.indentUlamCode(fp);
    fp->write("break;"); GCNL;
    m_state.m_currentIndentLevel--;

    //else do the body block, within another block for end-of-loop label (t41002)
    m_nodeBody->genCode(fp, uvpass);

    m_state.m_currentIndentLevel--;
    m_state.indentUlamCode(fp);
    fp->write("} // end ");
    fp->write(getName()); //end while
    GCNL;
  } //genCode

} //end MFM
