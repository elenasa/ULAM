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


  void  NodeControlWhile::eval()
  {
    assert(m_nodeCondition && m_nodeBody);
    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(getNodeType());
    m_nodeCondition->eval();
    UlamValue cuv = m_state.m_nodeEvalStack.popArg();

    while(cuv.m_valBool == true)
      {
	u32 slots = makeRoomForNodeType(m_nodeBody->getNodeType());
 	m_nodeBody->eval();  //side-effect
	m_state.m_nodeEvalStack.popArgs(slots);

	makeRoomForNodeType(getNodeType());
	m_nodeCondition->eval();
	cuv = m_state.m_nodeEvalStack.popArg();
      }
    
    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(cuv); //always false

    evalNodeEpilog();
  }

} //end MFM
