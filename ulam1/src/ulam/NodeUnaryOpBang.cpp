#include "NodeUnaryOpBang.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpBang::NodeUnaryOpBang(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpBang::~NodeUnaryOpBang(){}


  const char * NodeUnaryOpBang::getName()
  {
    return "!";
  }


  const std::string NodeUnaryOpBang::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeUnaryOpBang::checkAndLabelType()
  { 
    assert(m_node);

    UTI but = Bool;
    UTI ut  = m_node->checkAndLabelType(); 
    
    assert(m_state.isScalar(ut));

    if(ut != but)
      {
	m_node = new NodeCast(m_node, but, m_state);
	m_node->setNodeLocation(getNodeLocation());
	m_node->checkAndLabelType();
      }
    
    setNodeType(but);
    setStoreIntoAble(false);

    return getNodeType();
  }


  UlamValue NodeUnaryOpBang::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    return UlamValue::makeImmediate(type, !((bool) data), len);
  }

} //end MFM
