#include <stdlib.h>
#include "NodeTerminalProxy.h"
#include "CompilerState.h"

namespace MFM {

  NodeTerminalProxy::NodeTerminalProxy(Token memberTok, UTI memberType, Token funcTok, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeTerminal( (u64) UNKNOWNSIZE, memberType, state), m_ofTok(memberTok), m_uti(memberType), m_funcTok(funcTok), m_ready(false), m_nodeTypeDesc(nodetype)
  {
    Node::setNodeLocation(funcTok.m_locator);
    // is memberType is corrected for sizeof during c&l
  }

  NodeTerminalProxy::NodeTerminalProxy(const NodeTerminalProxy& ref) : NodeTerminal(ref), m_ofTok(ref.m_ofTok), m_uti(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_uti)), m_funcTok(ref.m_funcTok), m_ready(ref.m_ready), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeTerminalProxy::~NodeTerminalProxy()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  Node * NodeTerminalProxy::instantiate()
  {
    return new NodeTerminalProxy(*this);
  }

  void NodeTerminalProxy::updateLineage(NNO pno)
  {
    Node::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeTerminalProxy::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTerminalProxy::printPostfix(File * fp)
  {
    fp->write(" ");
    if(isReadyConstant())
      fp->write(NodeTerminal::getName());
    else
      {
	fp->write(m_state.getTokenDataAsString(&m_ofTok).c_str());
	fp->write(" ");
	fp->write(m_funcTok.getTokenString());
	fp->write(" .");
      }
  } //printPostfix

  const char * NodeTerminalProxy::getName()
  {
    if(isReadyConstant())
      return NodeTerminal::getName();

    return m_funcTok.getTokenString();
  }

  const std::string NodeTerminalProxy::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeTerminalProxy::isReadyConstant()
  {
    return m_ready;
  }

  FORECAST NodeTerminalProxy::safeToCastTo(UTI newType)
  {
    if(isReadyConstant())
      return  NodeTerminal::safeToCastTo(newType);
    return CAST_HAZY;
  } //safeToCastTo

  UTI NodeTerminalProxy::checkAndLabelType()
  {
    //when minmaxsizeof a selected member
    Symbol * asymptr = NULL;
    if(m_uti == Nouti)
      {
	bool hazyKin = false;
	if(m_state.alreadyDefinedSymbol(m_ofTok.m_dataindex, asymptr, hazyKin) && !hazyKin)
	  {
	    m_uti = asymptr->getUlamTypeIdx();
	    std::ostringstream msg;
	    msg << "Determined type for member '";
	    msg << m_state.getTokenDataAsString(&m_ofTok).c_str();
	    msg << "' Proxy, as type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    //m_state.setGoAgain(); //since not error
	  }
	else
	  {
	    if(m_ofTok.m_type == TOK_IDENTIFIER)
	      {
		std::ostringstream msg;
		msg << "Undetermined type for missing member '";
		msg << m_state.getTokenDataAsString(&m_ofTok).c_str();
		msg << "' Proxy";
		if(!hazyKin)
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav); //mising
		    return Nav;
		  }
		else
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    setNodeType(Hzy); //mising
		    m_state.setGoAgain(); //too
		    return Hzy;
		  }
	      }
	  }
      }

    //attempt to map UTI; may not have a node type descriptor
    if(!m_state.isComplete(m_uti))
      {
	UTI cuti = m_state.getCompileThisIdx();
	UTI mappedUTI = Nav;
	if(m_state.mappedIncompleteUTI(cuti, m_uti, mappedUTI))
	  {
	    std::ostringstream msg;
	    msg << "Substituting Mapped UTI" << mappedUTI;
	    msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
	    msg << " for incomplete Proxy type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_uti = mappedUTI;
	  }

	if(!m_state.isComplete(m_uti)) //reloads to recheck
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Terminal Proxy for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	    msg << " of member '";
	    msg << m_state.getTokenDataAsString(&m_ofTok).c_str();
	    msg << "' while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }

    if(!updateProxy()) //sets m_uti
      setNodeType(Nav); //invalid func
    else
      setConstantTypeForNode(m_funcTok); //enough info to set this constant node's type

    return getNodeType(); //updated to Unsigned, hopefully
  } //checkandLabelType

  void NodeTerminalProxy::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavNodes(cnt);
  } //countNavNodes

  EvalStatus NodeTerminalProxy::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    EvalStatus evs = NORMAL; //init ok
    evalNodeProlog(0); //new current frame pointer

    if(!m_state.isComplete(m_uti))
      evs = NOTREADY;
    else
      {
	UlamValue rtnUV;
	evs = NodeTerminal::makeTerminalValue(rtnUV);

	//copy result UV to stack, -1 relative to current frame pointer
	if(evs == NORMAL)
	  Node::assignReturnValueToStack(rtnUV);
      }
    evalNodeEpilog();
    return evs;
  } //eval

  void NodeTerminalProxy::genCode(File * fp, UlamValue& uvpass)
  {
    return NodeTerminal::genCode(fp, uvpass);
  }

  void NodeTerminalProxy::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    return NodeTerminal::genCodeToStoreInto(fp, uvpass);
  }

  bool NodeTerminalProxy::setConstantValue(Token tok)
  {
    bool rtnB = false;
    UlamType * cut = m_state.getUlamTypeByIndex(m_uti);
    assert(cut->isComplete());

    switch(tok.m_type)
      {
      case TOK_KW_SIZEOF:
	{
	  m_constant.uval =  cut->getTotalBitSize(); //unsigned
	  rtnB = true;
	}
	break;
      case TOK_KW_MAXOF:
	{
	  if(cut->isMinMaxAllowed())
	    {
	      m_constant.uval = cut->getMax();
	      rtnB = true;
	    }
	}
	break;
      case TOK_KW_MINOF:
	{
	  if(cut->isMinMaxAllowed())
	    {
	      m_constant.sval = cut->getMin();
	      rtnB = true;
	    }
	}
	break;
      default:
	assert(0);
      };
    return rtnB;
  } //setConstantValue

  //minof, maxof use type of lhs, sizeof is just unsigned32
  UTI NodeTerminalProxy::setConstantTypeForNode(Token tok)
  {
    UTI newType = Nav; //init
    switch(tok.m_type)
      {
      case TOK_KW_SIZEOF:
	{
	  newType = Unsigned;
	}
	break;
      case TOK_KW_MAXOF:
      case TOK_KW_MINOF:
      {
	newType = m_uti; // use type of the lhs
	break;
      }
      default:
	assert(0);
      };

    setNodeType(newType);
    setStoreIntoAble(false);
    return newType;
  } //setConstantTypeForNode

  bool NodeTerminalProxy::updateProxy()
  {
    bool rtnb = true;

    if(m_nodeTypeDesc)
      {
	m_uti = m_nodeTypeDesc->checkAndLabelType();
      }

    if(isReadyConstant())
      return true;

    if(!m_state.isComplete(m_uti))
      {
	std::ostringstream msg;
	msg << "Proxy Type: " << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	msg << " is still incomplete and unknown for its '";
	msg << m_funcTok.getTokenString();
	msg << "' while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	m_state.setGoAgain(); //since not error; maybe no nodetypedesc
	//rtnb = false; don't want to stop after parsing.
      }
    else
      {
	//depending on the of-func, update our constant
	if(!setConstantValue(m_funcTok))
	  {
	    std::ostringstream msg;
	    msg << "Proxy Type: " << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	    msg << " constant value for its <";
	    msg << m_funcTok.getTokenString();
	    msg << "> is still incomplete and unknown while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(&m_funcTok, msg.str().c_str(), DEBUG);
	    m_state.setGoAgain(); //since not error
	    rtnb = false;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Yippee! Proxy Type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	    msg << " (UTI" << getNodeType() << ") is KNOWN (=" << m_constant.uval;
	    msg << ") while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(&m_funcTok, msg.str().c_str(), DEBUG);
	    m_ready = true; //set ready here
	  }
      }
    return rtnb;
  } //updateProxy

} //end MFM
