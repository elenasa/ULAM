#include <stdio.h>
#include "NodeControlWhile.h"
#include "UlamTypeBool.h"
#include "CompilerState.h"

namespace MFM {

  NodeControlWhile::NodeControlWhile(Node * condNode, Node * trueNode, CompilerState & state): NodeControl(condNode, trueNode, state) {}
  NodeControlWhile::NodeControlWhile(const NodeControlWhile& ref) : NodeControl(ref) {}
  NodeControlWhile::~NodeControlWhile() {}

  Node * NodeControlWhile::clone()
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
    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(getNodeType());
    EvalStatus evs = m_nodeCondition->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue cuv = m_state.m_nodeEvalStack.popArg();

    while((bool) cuv.getImmediateData(m_state) == true)
      {
	u32 slots = makeRoomForNodeType(m_nodeBody->getNodeType());
 	evs = m_nodeBody->eval();  //side-effect
	if(evs == BREAK)
	  break;  //use C to break out of this loop
	else if(evs == RETURN || evs == ERROR)
	  {
	    evalNodeEpilog();
	    return evs;
	  }
	assert(evs == NORMAL || evs == CONTINUE);

	//continue continues as normal
	m_state.m_nodeEvalStack.popArgs(slots);

	makeRoomForNodeType(getNodeType());

	evs = m_nodeCondition->eval();
	if(evs != NORMAL)
	  {
	    evalNodeEpilog();
	    return evs;
	  }

	cuv = m_state.m_nodeEvalStack.popArg();
      }

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(cuv); //always false

    evalNodeEpilog();
    return NORMAL;
  }


  void NodeControlWhile::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeCondition && m_nodeBody);

#ifdef TMPVARBRACES
    m_state.indent(fp);
    fp->write("{\n");  //for overall tmpvars
    m_state.m_currentIndentLevel++;
#endif

    //while true..
    m_state.indent(fp);
    fp->write(getName());
    fp->write("(true)\n");

    m_state.indent(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;

    //if !condition break;
    m_nodeCondition->genCode(fp, uvpass);

    bool isTerminal = false;
    UTI cuti = uvpass.getUlamValueTypeIdx();

    if(cuti == Ptr)
      {
	cuti = uvpass.getPtrTargetType();
      }
    else
      {
	isTerminal = true;
      }

    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    fp->write("\n");
    m_state.indent(fp);
    fp->write("if(!");

    if(isTerminal)
      {
	// fp->write("(bool) ");
	// write out terminal explicitly
	u32 data = uvpass.getImmediateData(m_state);
	char dstr[40];
	cut->getDataAsString(data, dstr, 'z');
	fp->write(dstr);
      }
    else
      {
	assert(cut->getUlamTypeEnum() == Bool);
	fp->write(((UlamTypeBool *) cut)->getConvertToCboolMethod().c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(cuti, uvpass.getPtrSlotIndex()).c_str());
	fp->write(", ");
	fp->write_decimal(cut->getBitSize());
	fp->write(")");
      }
    fp->write(")\n");
    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("break;\n");
    m_state.m_currentIndentLevel--;

    //else do the body
    m_nodeBody->genCode(fp, uvpass);

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} // end ");
    fp->write(getName()); //end while
    fp->write("\n");

#ifdef TMPVARBRACES
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");  //close for tmpVar
#endif

  } //genCode

} //end MFM
