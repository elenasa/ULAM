#include <stdlib.h>
#include "NodeTerminalProxy.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "NodeMemberSelectByBaseClassType.h"
#include "NodePositionofRef.h"
#include "CompilerState.h"

namespace MFM {

  NodeTerminalProxy::NodeTerminalProxy(Node * membernode, UTI memberType, const Token& funcTok, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeTerminal( (u64) UNKNOWNSIZE, memberType, state), m_nodeOf(membernode), m_uti(memberType), m_funcTok(funcTok), m_ready(false), m_nodeTypeDesc(nodetype)
  {
    Node::setNodeLocation(funcTok.m_locator);
    // memberType is corrected for sizeof during c&l
  }

  NodeTerminalProxy::NodeTerminalProxy(const NodeTerminalProxy& ref) : NodeTerminal(ref), m_nodeOf(NULL), m_uti(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_uti,ref.getNodeLocation())), m_funcTok(ref.m_funcTok), m_ready(ref.m_ready), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
    if(ref.m_nodeOf)
      m_nodeOf = ref.m_nodeOf->instantiate();
  }

  NodeTerminalProxy::~NodeTerminalProxy()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
    delete m_nodeOf;
    m_nodeOf = NULL;
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
    if(m_nodeOf)
      m_nodeOf->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeTerminalProxy::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeOf == oldnptr)
      {
	m_nodeOf = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeTerminalProxy::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    if(m_nodeOf && m_nodeOf->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTerminalProxy::printPostfix(File * fp)
  {
    fp->write(" ");
    if(isReadyConstant())
      {
	if(m_funcTok.m_type == TOK_KW_CLASSIDOF)
	  {
	    if(m_nodeOf)
	      fp->write(m_nodeOf->getName());
	    else
	      fp->write(m_state.getUlamTypeNameBriefByIndex(m_uti).c_str());
	    fp->write("[");
	    fp->write(NodeTerminal::getName());
	    fp->write("]");
	  }
	else
	  fp->write(NodeTerminal::getName());
      }
    else
      {
	if(m_nodeOf)
	  fp->write(m_nodeOf->getName());
	else
	  fp->write(m_state.getUlamTypeNameBriefByIndex(m_uti).c_str());
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

  u32 NodeTerminalProxy::getNameId()
  {
    return m_funcTok.getTokenStringId();
  }

  const std::string NodeTerminalProxy::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeTerminalProxy::isAConstant()
  {
    if((m_funcTok.m_type == TOK_KW_LENGTHOF) && m_nodeOf)
      {
	//lengthof a scalar String variable is not constant, t3984; but,
	//lengthof a String array is, t3985;
	UTI ofnodeType = m_nodeOf->getNodeType();
	if(m_state.isAStringType(ofnodeType) && m_state.isScalar(ofnodeType))
	  return m_nodeOf->isAConstant();
      }
    //else, e.g. length of String type has null m_nodeOf, and is constant STRINGIDXBITS (t3933).
    return true;
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

  UTI NodeTerminalProxy::checkAndLabelType(Node * thisparentnode)
  {
    //when minmaxsizeof a selected member; and for clones,
    //when m_uti is a String, we must c&l m_nodeOf to find its symbol (t3960)
    if((!m_state.okUTItoContinue(m_uti) || m_state.isAStringType(m_uti)) && m_nodeOf)
      {
	bool savCnstInitFlag = m_state.m_initSubtreeSymbolsWithConstantsOnly;
	//terminal proxy (e.g. classidof, sizeof) are constants, regardless of lhs (t41381)
	m_state.m_initSubtreeSymbolsWithConstantsOnly = false;


	UTI ofuti = m_nodeOf->checkAndLabelType(this);

	m_state.m_initSubtreeSymbolsWithConstantsOnly = savCnstInitFlag; //restore

	if(m_state.okUTItoContinue(ofuti))
	  {
	    std::ostringstream msg;
	    msg << "Determined type for member '";
	    msg << m_nodeOf->getName();
	    msg << "' Proxy, as type: ";
	    msg << m_state.getUlamTypeNameByIndex(ofuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_uti = ofuti;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Undetermined type for missing member '";
	    msg << m_nodeOf->getName();
	    msg << "' Proxy";
	    if(ofuti == Nav)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav); //missing
		return Nav;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		setNodeType(Hzy); //missing
		m_state.setGoAgain(); //too
		return Hzy;
	      }
	  }
      }

    //attempt to map UTI; may not have a node type descriptor
    if(m_state.okUTItoContinue(m_uti) && !m_state.isComplete(m_uti))
      {
	//non-virtual functions, returns of-value of referenced type
	if(m_state.isReference(m_uti)) //e.g. selftyperef
	  m_uti = m_state.getUlamTypeAsDeref(m_uti); //resets m_uti here!

	UTI cuti = m_state.getCompileThisIdx();
	UTI mappedUTI = Nav;
	if(m_state.mappedIncompleteUTI(cuti, m_uti, mappedUTI))
	  {
	    std::ostringstream msg;
	    msg << "Substituting Mapped UTI" << mappedUTI;
	    msg << ", " << m_state.getUlamTypeNameByIndex(mappedUTI).c_str();
	    msg << " for incomplete Proxy type: ";
	    msg << m_state.getUlamTypeNameByIndex(m_uti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_uti = mappedUTI;
	  }

	if(!m_state.isComplete(m_uti)) //reloads to recheck
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Terminal Proxy for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	    if(m_nodeOf)
	      {
		msg << ", of member '";
		msg << m_nodeOf->getName() << "'";
	      }
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	  }
      }

    UTI nodeType = Nav;
    if(!updateProxy()) //sets m_uti
      {
	if(m_funcTok.m_type == TOK_KW_POSOF) //t41619
	  {
	    UTI ofuti = m_nodeOf->getNodeType();
	    if(m_state.okUTItoContinue(ofuti))
	      {
		std::ostringstream msg;
		msg << "Incomplete Terminal Proxy for type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
		if(m_nodeOf)
		  {
		    msg << ", of member '";
		    msg << m_nodeOf->getName() << "'";
		  }
		msg << " operator " << getName();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		setNodeType(Hzy);
		nodeType = Hzy;
		m_state.setGoAgain();
	      }
	    else
	      setNodeType(Nav); //invalid func
	  }
	else
	  setNodeType(Nav); //invalid func
      }
    else
      {
	nodeType = setConstantTypeForNode(m_funcTok); //enough info to set this constant node's type
	if((m_funcTok.m_type == TOK_KW_LENGTHOF))
	  {
	    if(replaceOurselvesLengthOf(thisparentnode))
	      {
		m_state.setGoAgain();

		delete this; //suicide is painless..

		return Hzy;
	      }
	    else
	      nodeType = getNodeType(); //t41382
	  }
	else if((m_funcTok.m_type == TOK_KW_POSOF))
	  {
	    if(replaceOurselvesPositionofRef(thisparentnode))
	      {
		m_state.setGoAgain();

		delete this; //suicide is painless..

		return Hzy;
	      }
	    else
	      nodeType = getNodeType();
	  }
	//else
      }
    return nodeType; //getNodeType(); //updated to Unsigned, hopefully
  } //checkandLabelType

  bool NodeTerminalProxy::replaceOurselvesLengthOf(Node * parentnode)
  {
    bool rtnb = false;
    if(m_state.isAClass(m_uti) && m_state.isClassACustomArray(m_uti) && m_state.hasAClassCustomArrayLengthof(m_uti))
      {
	//replace node with func call to 'alengthof'
	Node * newnode = buildAlengthofFuncCallNode();
	NODE_ASSERT(newnode);
	AssertBool swapOk = Node::exchangeNodeWithParent(newnode, parentnode);
	NODE_ASSERT(swapOk);

	m_nodeOf = NULL; //recycled
	rtnb = true;
      }
    else if(isAConstant() && isReadyConstant())
      {
	//constantFold, like NodeBinaryOp (e.g. t3985)
	//replace with a NodeTerminal; might not be ready (t41065, t41382)
	Node * newnode = constantFoldLengthofConstantString();
	if(newnode)
	  {
	    AssertBool swapOk = Node::exchangeNodeWithParent(newnode, parentnode);
	    NODE_ASSERT(swapOk);

	    rtnb = true;
	  } //else t41382
      }
    //else didn't replace us
    return rtnb;
  } //replaceOurselvesLengthOf

  Node * NodeTerminalProxy::buildAlengthofFuncCallNode()
  {
    Token identTok;
    u32 alenofId = m_state.getCustomArrayLengthofFunctionNameId();
    identTok.init(TOK_IDENTIFIER, getNodeLocation(), alenofId);

    //fill in func symbol during type labeling;
    Node * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
    NODE_ASSERT(fcallNode);
    fcallNode->setNodeLocation(identTok.m_locator);
    Node * mselectNode = new NodeMemberSelect(m_nodeOf, fcallNode, m_state);
    NODE_ASSERT(mselectNode);
    mselectNode->setNodeLocation(identTok.m_locator);

    //redo check and type labeling done by caller!!
    return mselectNode; //replace right node with new branch
  } //buildAlengthofFuncCallNode

  Node * NodeTerminalProxy::constantFoldLengthofConstantString()
  {
    u64 val = 0;
    UTI nuti = getNodeType();
    NODE_ASSERT(m_state.okUTItoContinue(nuti)); //nothing to do yet

    // if here, must be a constant..
    NODE_ASSERT(isAConstant());

    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(nuti); //offset a constant expression
    EvalStatus evs = eval();
    if( evs == NORMAL)
      {
	UlamValue cnstUV = m_state.m_nodeEvalStack.popArg();
	u32 wordsize = m_state.getTotalWordSize(nuti);
	if(wordsize == MAXBITSPERINT)
	  val = cnstUV.getImmediateData(m_state);
	else if(wordsize == MAXBITSPERLONG)
	  val = cnstUV.getImmediateDataLong(m_state);
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for ";
	msg << "proxy Type '" << m_funcTok.getTokenString() << "' for scalar constant: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	msg << " is erroneous";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return NULL;
      }

    if(evs == NOTREADY)
      {
	std::ostringstream msg;
	msg << "Constant value expression for ";
	msg << "proxy Type '" << m_funcTok.getTokenString() << "' for scalar constant: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	msg << " is not yet ready";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	setNodeType(Hzy);
	m_state.setGoAgain(); //for compiler counts
	return NULL;
      }

    //replace ourselves (and kids) with a node terminal; new NNO unlike template's
    NodeTerminal * newnode = new NodeTerminal(val, nuti, m_state);
    NODE_ASSERT(newnode);
    newnode->setNodeLocation(getNodeLocation());

    return newnode;
  } //constantFoldLengthofConstantString

  bool NodeTerminalProxy::replaceOurselvesPositionofRef(Node * parentnode)
  {
    bool rtnb = false;
    if(m_nodeOf->hasASymbolReference())
      {
	//replace node with func call to 'alengthof'
	Node * newnode = buildApositionofrefNode();
	NODE_ASSERT(newnode);
	AssertBool swapOk = Node::exchangeNodeWithParent(newnode, parentnode);
	NODE_ASSERT(swapOk);

	m_nodeOf = NULL; //recycled
	rtnb = true;
      }
    //else didn't replace us
    return rtnb;
  } //replaceOurselvesPositionofRef

  Node * NodeTerminalProxy::buildApositionofrefNode()
  {
    NodePositionofRef * newNode = new NodePositionofRef(m_nodeOf, m_state);
    NODE_ASSERT(newNode);
    newNode->setNodeLocation(m_funcTok.m_locator);

    //redo check and type labeling done by caller!!
    return newNode; //replace right node with new branch
  } //buildApositionofNode

  void NodeTerminalProxy::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavNodes

  EvalStatus NodeTerminalProxy::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    if(!m_state.isComplete(m_uti)) return evalStatusReturnNoEpilog(NOTREADY);

    EvalStatus evs = NORMAL; //init ok
    evalNodeProlog(0); //new current frame pointer

    if((m_funcTok.m_type == TOK_KW_LENGTHOF))
      {
	UTI noftype;
	if(m_nodeOf && m_state.isAStringType(noftype = m_nodeOf->getNodeType()) && m_state.isScalar(noftype))
	  {
	    //String or String array item (t3933, t3949, t3984)
	    evalNodeProlog(0); //new current frame pointer
	    makeRoomForSlots(1); //upool index is a constant expression
	    evs = m_nodeOf->eval();
	    if(evs == NORMAL)
	      {
		UlamValue stringUV = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
		u32 ustringidx = stringUV.getImmediateData(m_state);
		if(m_state.isValidUserStringIndex(ustringidx))
		  m_constant.uval = m_state.getUserStringLength(ustringidx); //reset here!!
		else
		  evs = ERROR;
	      }
	    //else
	    evalNodeEpilog(); //second epilog
	  }
	else if(m_state.isClassACustomArray(m_uti))
	  {
	    std::ostringstream msg;
	    msg << "Custom Array '" << m_funcTok.getTokenString() << "' proxy requires "; //lengthof
	    msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayLengthofFunctionNameId()).c_str();
	    msg << " function; Unsupported for eval";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    evs = UNEVALUABLE;
	    m_state.abortShouldntGetHere(); //replaced with func call when provided, o.w. parse err
	  }
      }

    if(evs == NORMAL)
      {
	UlamValue rtnUV;
	evs = NodeTerminal::makeTerminalValue(rtnUV);

	//copy result UV to stack, -1 relative to current frame pointer
	if(evs == NORMAL)
	  Node::assignReturnValueToStack(rtnUV);
      }

    if(evs != NORMAL) return evalStatusReturn(evs);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  void NodeTerminalProxy::genCode(File * fp, UVPass& uvpass)
  {
    if(m_funcTok.m_type == TOK_KW_LENGTHOF)
      {
	if(m_nodeOf && m_state.isAStringType(m_uti))
	  {
	    //String or String array item (t3933, t3949)
	    return genCodeForUserStringLength(fp, uvpass); //t3929
	  }
	else if(m_state.isClassACustomArray(m_uti))
	  {
	    std::ostringstream msg;
	    msg << "Custom Array '" << m_funcTok.getTokenString() << "' proxy requires "; //lengthof
	    msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayLengthofFunctionNameId()).c_str();
	    msg << " function; Unsupported for genCode";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.abortShouldntGetHere(); //replaced with func call when provided, o.w. parse err
	  }
      }

    return NodeTerminal::genCode(fp, uvpass);
  }

  void NodeTerminalProxy::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    return NodeTerminal::genCodeToStoreInto(fp, uvpass);
  }

  void NodeTerminalProxy::genCodeForUserStringLength(File * fp, UVPass& uvpass)
  {
    NODE_ASSERT(m_state.isAStringType(m_uti));
    NODE_ASSERT(m_nodeOf);
    UTI nuti = getNodeType();
    UVPass ofpass;
    m_nodeOf->genCode(fp, ofpass);

    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(m_state.getUlamTypeByIndex(nuti)->getTmpStorageTypeAsString().c_str()); //u32
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(ASCII, tmpVarNum, TMPREGISTER).c_str());
    fp->write(" = ");
    fp->write(m_state.getGetStringLengthFunctionName());
    fp->write("(");
    fp->write(ofpass.getTmpVarAsString(m_state).c_str());
    fp->write(");");
    GCNL; //t3949

    uvpass = UVPass::makePass(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, 0); //POS 0 rightjustified (atom-based).

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeForUserStringLength

  bool NodeTerminalProxy::setConstantValue(const Token& tok)
  {
    bool rtnB = false;
    UlamType * cut = m_state.getUlamTypeByIndex(m_uti);
    NODE_ASSERT(cut->isComplete());

    switch(tok.m_type)
      {
      case TOK_KW_SIZEOF:
	{
	  //User String length must wait until after c&l; sizeof string == 32 (t3929)
	  //consistent with C; (not array size if non-scalar)
	  if(checkForClassIdOfType())
	    m_constant.uval = m_state.getClassIdBits();
	  else
	    m_constant.uval =  cut->getSizeofUlamType(); //unsigned
	  rtnB = true;
	}
	break;
      case TOK_KW_LENGTHOF:
	{
	  //User String length must wait until after c&l; sizeof string == 32 (t3929)
	  //consistent with C; (not array size if non-scalar)
	  rtnB = true;
	  if(!cut->isScalar())
	    {
	      m_constant.uval = cut->getArraySize(); //number of items, not custom arrays
	    }
	  else if(cut->getUlamTypeEnum() == String)
	    {
	      m_constant.uval = cut->getSizeofUlamType(); //tmp for proxy, t3985
	    }
	  else if(m_state.isClassACustomArray(m_uti))
	    {
	      if(!m_state.hasAClassCustomArrayLengthof(m_uti))
		{
		  std::ostringstream msg;
		  msg << "Proxy Type '" << m_funcTok.getTokenString() << "' is not supported ";
		  msg << "for custom array: ";
		  msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
		  msg << "; Function '";
		  msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayLengthofFunctionNameId()).c_str();
		  msg << "()' is not defined";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  rtnB = false; //not provided
		}
	      //else a runtime request..
	    }
	  else
	    {
	      std::ostringstream msg;
	      msg << "Proxy Type '" << m_funcTok.getTokenString() << "' is not supported ";
	      msg << "for scalar: ";
	      msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      rtnB = false; //not allowed (e.g. custom arrays, scalars)
	    }
	}
	break;
      case TOK_KW_MAXOF:
	{
	  if(cut->isMinMaxAllowed())
	    {
	      if(checkForClassIdOfType()) //t41537,t41549
		m_constant.uval = MAX_REGISTRY_NUMBER; //m_state.getMaxNumberOfRegisteredUlamClasses()
	      else
		m_constant.uval = cut->getMax();
	      rtnB = true;
	    }
	  else
	    {
	      std::ostringstream msg;
	      msg << "Proxy Type '" << m_funcTok.getTokenString() << "' is not supported ";
	      msg << "for type: ";
	      msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //t41407
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
	  else
	    {
	      std::ostringstream msg;
	      msg << "Proxy Type '" << m_funcTok.getTokenString() << "' is not supported ";
	      msg << "for type: ";
	      msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //t41407
	    }
	}
	break;
      case TOK_KW_CLASSIDOF:
	{
	  if((rtnB = checkForClassType()))
	    {
	      m_constant.uval = m_state.getAClassRegistrationNumber(m_uti);
	    }
	  break;
	}
      case TOK_KW_POSOF:
	{
	  u32 pos = UNRELIABLEPOS;
	  if(m_nodeOf->hasASymbolReference() && !m_nodeOf->hasASymbolSelf()) //t41621
	    {
	      //build NodePositionofRef for runtime determination of pos
	      rtnB = true;
	    }
	  else if(m_nodeOf->hasASymbolDataMember())
	    {
	      pos = m_nodeOf->getPositionOf(); //t41615,6
	    }
	  else if(checkForClassType())
	    {
	      pos = m_nodeOf->getPositionOf();
	    }
	  else
	    m_state.abortNotImplementedYet();

	  if(pos != UNRELIABLEPOS)
	    {
	      m_constant.uval = pos;
	      rtnB = true;
	    }

	  break;
	}
      case TOK_KW_FLAG_INSERTCLASSSIGNATURE:
	{
	  if((rtnB = checkForClassType()))
	    {
	      u32 cid = m_state.getUlamTypeNameIdByIndex(m_uti);
	      SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(cid);
	      NODE_ASSERT(cnsym);

	      std::string sig = cnsym->generatePrettyNameOrSignature(m_uti,true,false);
	      m_constant.uval = m_state.formatAndGetIndexForDataUserString(sig);
	    }
	  break;
	}
      case TOK_KW_FLAG_INSERTCLASSNAMESIMPLE:
	{
	  if((rtnB = checkForClassType()))
	    {
	      u32 cid = m_state.getUlamTypeNameIdByIndex(m_uti);
	      SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(cid);
	      NODE_ASSERT(cnsym);

	      std::string simple = cnsym->generatePrettyNameOrSignature(m_uti,false,true);
	      m_constant.uval = m_state.formatAndGetIndexForDataUserString(simple);
	    }
	  break;
	}
      case TOK_KW_FLAG_INSERTCLASSNAMEPRETTY:
	{
	  if((rtnB = checkForClassType()))
	    {
	      u32 cid = m_state.getUlamTypeNameIdByIndex(m_uti);
	      SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(cid);
	      NODE_ASSERT(cnsym);

	      std::string pretty = cnsym->generatePrettyNameOrSignature(m_uti,true,true);
	      m_constant.uval = m_state.formatAndGetIndexForDataUserString(pretty);
	    }
	  break;
	}
      case TOK_KW_FLAG_INSERTCLASSNAMEMANGLED:
	{
	  if((rtnB = checkForClassType()))
	    {
	      std::string mangled = m_state.getUlamTypeByIndex(m_uti)->getUlamTypeMangledName();
	      m_constant.uval = m_state.formatAndGetIndexForDataUserString(mangled);
	    }
	}
	break;
      default:
	m_state.abortShouldntGetHere();
	break;
      };
    return rtnB;
  } //setConstantValue

  //minof, maxof use type of lhs, sizeof is just unsigned32
  UTI NodeTerminalProxy::setConstantTypeForNode(const Token& tok)
  {
    UTI newType = Nav; //init
    switch(tok.m_type)
      {
      case TOK_KW_LENGTHOF:
      case TOK_KW_SIZEOF:
      case TOK_KW_CLASSIDOF:
      case TOK_KW_POSOF:
	newType = Unsigned;
	break;
      case TOK_KW_MAXOF:
      case TOK_KW_MINOF:
	newType = m_uti; // use type of the lhs
	break;
      case TOK_KW_FLAG_INSERTCLASS:
      case TOK_KW_FLAG_INSERTCLASSSIGNATURE:
      case TOK_KW_FLAG_INSERTCLASSNAMESIMPLE:
      case TOK_KW_FLAG_INSERTCLASSNAMEPRETTY:
      case TOK_KW_FLAG_INSERTCLASSNAMEMANGLED:
	newType = String;
	break;
      default:
	m_state.abortShouldntGetHere();
	break;
      };

    setNodeType(newType);
    Node::setStoreIntoAble(TBOOL_FALSE);
    return newType;
  } //setConstantTypeForNode

  bool NodeTerminalProxy::updateProxy()
  {
    bool rtnb = true;

    if(m_nodeTypeDesc)
      {
	m_uti = m_nodeTypeDesc->checkAndLabelType(this);
      }

    if(isReadyConstant())
      return true;

    if(!m_state.okUTItoContinue(m_uti) || !m_state.isComplete(m_uti))
      {
	std::ostringstream msg;
	msg << "Proxy Type";
	if(m_state.okUTItoContinue(m_uti) && !m_state.isHolder(m_uti))
	  msg << ": " << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	if(m_uti == Nav)
	  msg << " is invalid for its '";
	else
	  msg << " is still incomplete and unknown for its '";
	msg << m_funcTok.getTokenString() << "'";
	if(m_state.okUTItoContinue(m_uti) || m_state.isStillHazy(m_uti))
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //error/t3298,t41204
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false; //don't want to stop after parsing.???
	  }
      }
    else
      {
	//depending on the of-func, update our constant
	if(!setConstantValue(m_funcTok))
	  {
	    //err msg output already!
	    rtnb = false; //turns into a nav! error/t3937,t3938,t41066
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Yippee! Proxy Type: ";
	    msg << m_state.getUlamTypeNameByIndex(m_uti).c_str();
	    msg << " (UTI" << getNodeType() << ") is KNOWN (=" << m_constant.uval;
	    msg << ") while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(&m_funcTok, msg.str().c_str(), DEBUG);
	    m_ready = true; //set ready here
	  }
      }
    return rtnb;
  } //updateProxy

  bool NodeTerminalProxy::checkForClassType()
  {
    NODE_ASSERT(m_state.okUTItoContinue(m_uti)); //is complete too!
    if(!m_state.isAClass(m_uti))
      {
	std::ostringstream msg;
	msg << "Proxy Type '" << m_funcTok.getTokenString() << "' is not supported ";
	msg << "for non-class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_uti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false; //not allowed (e.g. primitives..)
      }
    return true;
  } //checkForClassType

  bool NodeTerminalProxy::checkForClassIdOfType()
  {
    //special case of classidof.maxof returns max classid (t41537);
    //and, classidof.sizeof (consistently) returns bits for classidof.maxof
    NODE_ASSERT(m_state.okUTItoContinue(m_uti)); //is complete too!
    if(m_nodeOf)
      {
	u32 ofnameid = m_nodeOf->getNameId();
	Token classidtok(TOK_KW_CLASSIDOF, getNodeLocation(), 0);
	return (classidtok.getTokenStringId() == ofnameid);
      }
    return false; //t3783..
  } //checkForClassIdOfType

} //end MFM
