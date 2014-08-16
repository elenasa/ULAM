#include <stdio.h>
#include "NodeCast.h"
#include "CompilerState.h"

namespace MFM {

  NodeCast::NodeCast(Node * n, UlamType * typeToBe, CompilerState & state): NodeUnaryOp(n, state) 
  {
    setNodeType(typeToBe);
  }

  NodeCast::~NodeCast()
  { 
    delete m_node;
    m_node = NULL;
  }


  const char * NodeCast::getName()
  {
    return "cast";
  }


  const std::string NodeCast::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeCast::checkAndLabelType()
  { 
    // unlike the other nodes, nodecast is created by other nodes during checkAndLabelType, 
    // and knows at construction type what it is,
    // and what it needs to cast its child to be during eval.
    // this is for checking for errors, before eval happens.
    UlamType * tobeType = getNodeType();
    UlamType * nodeType = m_node->getNodeType(); 

    if(tobeType == m_state.getUlamTypeByIndex(Nav))
      MSG(getNodeLocationAsString().c_str(), "Cannot cast to Nav.", ERR);
    else
      {
	if(!tobeType->isScalar())
	  {
	    MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts, elena", ERR);

	    if(!nodeType->isScalar())
	      MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Cannot cast scalar into array", ERR);

	    if(tobeType->getArraySize() != nodeType->getArraySize())
	      MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Array sizes differ", ERR);
	  }
	else
	  {
	    if(!nodeType->isScalar())
	      MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Cannot cast array into scalar", ERR);
	  }
      }

    return getNodeType(); 
  }


  EvalStatus NodeCast::eval()
  {
    assert(m_node); //has to be

    evalNodeProlog(0); //new current frame pointer
    UlamType * tobeType = getNodeType();
    UlamType * nodeType = m_node->getNodeType(); //uv.getUlamValueType()

    makeRoomForNodeType(nodeType);
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //do we believe these to be scalars, only?
    //possibly an array that needs to be casted, per elenemt
    if(tobeType->isScalar())
      assert(nodeType->isScalar());
    else
      {
	//both arrays the same dimensions
	//assert(!nodeType->isScalar());
	if(tobeType->getArraySize() != nodeType->getArraySize())
	  MSG(getNodeLocationAsString().c_str(), "Considering implementing array casts!!!", ERR);
	assert(0);
      }

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

    if(nodeType != tobeType)
      {
	if(!(tobeType->cast(uv)))
	  {
	    std::ostringstream msg;
	    msg << "Cast problem! Value type <" << uv.getUlamValueType()->getUlamTypeName().c_str() << "> failed to be cast as type: <" << tobeType->getUlamTypeName().c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  }


  void NodeCast::genCode(File * fp)
  {
    fp->write("(");
    fp->write(getNodeType()->getUlamTypeAsStringForC().c_str());
    fp->write(") ");
    m_node->genCode(fp);
  }

} //end MFM
