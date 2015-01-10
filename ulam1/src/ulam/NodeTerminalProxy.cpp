#include <stdlib.h>
#include "NodeTerminalProxy.h"
#include "CompilerState.h"

namespace MFM {

  NodeTerminalProxy::NodeTerminalProxy(UTI memberType, Token funcTok, CompilerState & state) : NodeTerminal(UNKNOWNSIZE, state), m_uti(memberType), m_funcTok(funcTok), m_bitsizeConstExpr(NULL)
  {
    Node::setNodeLocation(funcTok.m_locator);
  }


  NodeTerminalProxy::~NodeTerminalProxy()
  {
    delete m_bitsizeConstExpr;
    m_bitsizeConstExpr = NULL;
  }


  const std::string NodeTerminalProxy::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  void NodeTerminalProxy::constantFold(Token tok)
  {
    assert(0);  //override NodeTerminal
  } //constantFold


  UTI NodeTerminalProxy::checkAndLabelType()
  {
    UlamType * cut = m_state.getUlamTypeByIndex(m_uti);

    if(m_constant.sval == UNKNOWNSIZE)
      {
	s32 bitsize = cut->getBitSize();
	if(bitsize == UNKNOWNSIZE)
	  {
	    if(m_bitsizeConstExpr)
	      {
		m_bitsizeConstExpr->getTypeBitSizeInParen(bitsize, cut->getUlamTypeEnum()); //eval

		if(bitsize != UNKNOWNSIZE)
		  {
		    m_state.setSizes(m_uti, bitsize, cut->getArraySize()); //update UlamType
		  }
	      }
	  }
      }

    if(!cut->isComplete())
      {
	std::ostringstream msg;
	msg << "Proxy Type: " << m_state.getUlamTypeNameByIndex(m_uti).c_str() << " is still incomplete and unknown";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }
    else
      {
	//depending on the of-func, update our constant
	if(!setConstantValue(m_funcTok))
	  {
	    std::ostringstream msg;
	    msg << "Unsupported request: '" << m_state.getTokenDataAsString(&m_funcTok).c_str() << "' of type: " << m_state.getUlamTypeNameByIndex(m_uti).c_str();
	    MSG(&m_funcTok, msg.str().c_str(), ERR);
	    setNodeType(Nav);
	  }
	else
	  setConstantTypeForNode(m_funcTok);
      }
    return getNodeType(); //updated to Unsigned, hopefully
  }  //checkandLabelType


  Node * NodeTerminalProxy::findANodeDeclWithType(UTI utype)
  {
    if(m_uti == utype && m_bitsizeConstExpr != NULL)
      return this;
    return NULL;  //not me
  }


  void NodeTerminalProxy::linkConstantExpression(NodeTypeBitsize * cenode)
  {
    m_bitsizeConstExpr = cenode;
  }


  void NodeTerminalProxy::linkConstantExpression(NodeSquareBracket * cenode)
  {
    //m_arraysizeConstExpr = cenode;
    assert(0);
  }


  bool NodeTerminalProxy::setConstantValue(Token tok)
  {
    bool rtnB = false;
    UlamType * cut = m_state.getUlamTypeByIndex(m_uti);
    assert(cut->isComplete());

    if(tok.m_dataindex == m_state.m_pool.getIndexForDataString("sizeof"))
      {
	m_constant.uval =  cut->getTotalBitSize(); //unsigned

	rtnB = true;
      }
    else if(tok.m_dataindex == m_state.m_pool.getIndexForDataString("maxof"))
      {
	if(cut->isMinMaxAllowed())
	  {
	    m_constant.uval = cut->getMax();
	    rtnB = true;
	  }
      }
    else if (tok.m_dataindex == m_state.m_pool.getIndexForDataString("minof"))
      {
	if(cut->isMinMaxAllowed())
	  {
	    m_constant.sval = cut->getMin();
	    rtnB = true;
	  }
      }
    return rtnB;
  } //setConstantValue


  //minof, maxof should be the same type as the original member token, unsigned for sizeof
  UTI NodeTerminalProxy::setConstantTypeForNode(Token tok)
  {
    UTI newType = Nav;  //init
    if(tok.m_dataindex == m_state.m_pool.getIndexForDataString("sizeof"))
	newType = m_state.getUlamTypeOfConstant(Unsigned);
    else
      {
	ULAMTYPE etype = m_state.getUlamTypeByIndex(m_uti)->getUlamTypeEnum();
	newType = m_state.getUlamTypeOfConstant(etype);
      }

    setNodeType(newType);
    setStoreIntoAble(false);
    return newType;
  } //setConstantTypeForNode

} //end MFM
