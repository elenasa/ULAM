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
    UTI ut = m_node->checkAndLabelType();
    UTI newType = ut;         // init to stay the same
    
    if(!m_state.isScalar(ut)) //array unsupported at this time
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) type: <" << m_state.getUlamTypeNameByIndex(ut).c_str() << "> for unary operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);	
	newType = Nav;
      }
    else
      {
	ULAMTYPE eut = m_state.getUlamTypeByIndex(ut)->getUlamTypeEnum();
	if(eut != Bool)
	  {
	    newType = Bool;
	    m_node = makeCastingNode(m_node, newType);  //insert node/s
	  }
      }

    setNodeType(newType);

    setStoreIntoAble(false);

    return newType; 
  } //checkAndLabelType


#if 0
  UTI NodeUnaryOpBang::CHECKANDLABELTYPE()
  { 
    assert(m_node);

    UTI but = Bool;
    UTI ut = m_node->checkAndLabelType(); 
    
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
#endif


  UlamValue NodeUnaryOpBang::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    return UlamValue::makeImmediate(type, !((bool) data), len);
  }

} //end MFM
