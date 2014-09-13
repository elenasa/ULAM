#include <stdio.h>
#include "NodeCast.h"
#include "CompilerState.h"

namespace MFM {

  NodeCast::NodeCast(Node * n, UTI typeToBe, CompilerState & state): NodeUnaryOp(n, state) 
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


  UTI NodeCast::checkAndLabelType()
  { 
    // unlike the other nodes, nodecast knows its type at construction time;
    // this is for checking for errors, before eval happens.
    UTI tobeType = getNodeType();
    UTI nodeType = m_node->checkAndLabelType(); //user cast 

    if(tobeType == Nav)
      MSG(getNodeLocationAsString().c_str(), "Cannot cast to Nav.", ERR);
    else
      {
	if(!m_state.isScalar(tobeType))
	  {
	    MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts, elena", ERR);

	    if(!m_state.isScalar(nodeType))
	      MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Cannot cast scalar into array", ERR);

	    if(m_state.getArraySize(tobeType) != m_state.getArraySize(nodeType))
	      MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Array sizes differ", ERR);
	  }
	else
	  {
	    if(!m_state.isScalar(nodeType))
	      MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Cannot cast array into scalar", ERR);
	  }
      }

    return getNodeType(); 
  }


  EvalStatus NodeCast::eval()
  {
    assert(m_node); //has to be

    evalNodeProlog(0); //new current frame pointer
    UTI tobeType = getNodeType();
    UTI nodeType = m_node->getNodeType(); //uv.getUlamValueType()

    makeRoomForNodeType(nodeType);
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //do we believe these to be scalars, only?
    //possibly an array that needs to be casted, per elenemt
    if(m_state.isScalar(tobeType))
      assert(m_state.isScalar(nodeType));
    else
      {
	//both arrays the same dimensions
	//assert(!nodeType->isScalar());
	if(m_state.getArraySize(tobeType) != m_state.getArraySize(nodeType))
	  MSG(getNodeLocationAsString().c_str(), "Considering implementing array casts!!!", ERR);
	assert(0);
      }

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

    if(nodeType != tobeType)
      {
	if(!(m_state.getUlamTypeByIndex(tobeType)->cast(uv, m_state)))
	  {
	    std::ostringstream msg;
	    msg << "Cast problem! Value type <" << m_state.getUlamTypeNameByIndex(uv.getUlamValueTypeIdx()).c_str() << "> failed to be cast as type: <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  }


  UlamValue NodeCast::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    assert(0); // n/a
    return UlamValue();
  }


  void NodeCast::genCode(File * fp)
  {
    fp->write("(");
    fp->write(m_state.getUlamTypeByIndex(getNodeType())->getUlamTypeMangledName(&m_state).c_str());
    fp->write(") ");
    fp->write("(");
    m_node->genCode(fp);
    fp->write(")");
  }

} //end MFM
