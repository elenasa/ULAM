#include <stdio.h>
#include "NodeControlWhile.h"
#include "CompilerState.h"

namespace MFM {

  NodeControlWhile::NodeControlWhile(Node * condNode, Node * trueNode, CompilerState & state): NodeControl(condNode, trueNode, state) {}

  NodeControlWhile::~NodeControlWhile()
  { }


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
    m_state.indent(fp);
    fp->write("{\n");  //for overall tmpvars
    m_state.m_currentIndentLevel++;

    NodeControl::genCode(fp, uvpass);  //condition and body

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");  //overall tmpvar
  } //genCode

} //end MFM
