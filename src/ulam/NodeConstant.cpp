#include <stdlib.h>
#include "NodeConstant.h"
#include "NodeConstantArray.h"
#include "NodeConstantClass.h"
#include "NodeConstantClassArray.h"
#include "NodeModelParameter.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstant::NodeConstant(const Token& tok, SymbolWithValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state) : NodeTerminal(state), m_token(tok), m_nodeTypeDesc(typedesc), m_constSymbol(symptr), m_ready(false), m_constType(Nouti), m_currBlockNo(0), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  {
    if(symptr)
      {
	setBlockNo(symptr->getBlockNoOfST());
	m_ready = updateConstant(); //sets ready here
	m_constType = symptr->getUlamTypeIdx();
      }
    //else t41148
  }

  NodeConstant::NodeConstant(const Token& tok, NNO stblockno, UTI constantType, NodeTypeDescriptor * typedesc, CompilerState & state) : NodeTerminal(state), m_token(tok), m_nodeTypeDesc(typedesc), m_constSymbol(NULL), m_ready(false), m_constType(constantType), m_currBlockNo(stblockno), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  { }

  NodeConstant::NodeConstant(const NodeConstant& ref) : NodeTerminal(ref), m_token(ref.m_token), m_nodeTypeDesc(NULL), m_constSymbol(NULL), m_ready(false), m_constType(ref.m_constType), m_currBlockNo(ref.m_currBlockNo), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeConstant::~NodeConstant()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;

    delete m_tmpvarSymbol;
    m_tmpvarSymbol = NULL;
  }

  Node * NodeConstant::instantiate()
  {
    return new NodeConstant(*this);
  }

  void NodeConstant::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    //assert(m_state.getCurrentBlockNo() == m_currBlockNo); //may not be setup yet (t41146)
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeConstant::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeConstant::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }

  const char * NodeConstant::getName()
  {
    if(isReadyConstant() && m_constSymbol && !m_constSymbol->isClassParameter())
      return NodeTerminal::getName();

    return m_state.getTokenDataAsString(m_token).c_str();
  }

  const std::string NodeConstant::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeConstant::clearSymbolPtr()
  {
    //if symbol is in a stub, there's no guarantee the stub
    // won't be replace by another duplicate class once its
    // pending args have been resolved.
    m_constSymbol = NULL;
    setBlock(NULL);
  }

  bool NodeConstant::getSymbolPtr(const Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  bool NodeConstant::compareSymbolPtrs(Symbol * ptr)
  {
    return (m_constSymbol == ptr);
  }

  bool NodeConstant::hasASymbol()
  {
    return (m_constSymbol != NULL);
  }

  u32 NodeConstant::getSymbolId()
  {
    NODE_ASSERT(m_constSymbol);
    return m_constSymbol->getId();
  }

  bool NodeConstant::hasASymbolDataMember()
  {
    NODE_ASSERT(m_constSymbol);
    return m_constSymbol->isDataMember();
  }

  bool NodeConstant::isReadyConstant()
  {
    return m_ready;
  }

  void NodeConstant::setClassType(UTI cuti)
  {
    //noop - before surgery
  }

  bool NodeConstant::getConstantValue(BV8K& bval)
  {
    if(!m_constSymbol || !m_constSymbol->isReady())
      return false;
    return m_constSymbol->getValue(bval);
  }

  FORECAST NodeConstant::safeToCastTo(UTI newType)
  {
    if(!isReadyConstant())
      return CAST_HAZY;

    //constant to non-constant ref is not cast-able.
    if(m_state.isAltRefType(newType) && !m_state.isConstantRefType(newType))
      return CAST_BAD;

    UTI nuti = getNodeType();
    ULAMTYPE ntypEnum = m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum();
    UlamType * newut = m_state.getUlamTypeByIndex(newType);
    ULAMTYPE typEnum = newut->getUlamTypeEnum();

    //special cases: not a matter of fitting
    if((typEnum == Bool) || (ntypEnum == Bool) || (typEnum == UAtom) || (typEnum == Class) || (typEnum == Void))
      return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);

    //special case: FROM Bits, not just a matter of fitting, e.g. logical shifts (t3850)
    if((ntypEnum == Bits) && (typEnum != Bits))
      return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);

    //special case: FROM/TO String, not just a matter of fitting (t41164)
    if((ntypEnum == String) ^ (typEnum == String))
      return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);

    //unlike terminals, named constants may be used with constant refs; must be same size (t41254)
    if(m_state.isConstantRefType(newType))
      return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);

    //for non-bool terminal check for complete types and arrays before fits.
    FORECAST scr = m_state.getUlamTypeByIndex(newType)->UlamType::safeCast(nuti);
    if(scr != CAST_CLEAR)
      return scr;

    return NodeTerminal::fitsInBits(newType) ? CAST_CLEAR : CAST_BAD;
  } //safeToCastTo

  UTI NodeConstant::checkAndLabelType(Node * thisparentnode)
  {
    UTI it = getNodeType(); //was Nav; but could be repeated c&l call.

    if(m_nodeTypeDesc)
      {
	//NodeConstant uses its NodeTypeDescriptor to identify the specified class where it is defined (t41547)
	UTI duti = m_nodeTypeDesc->checkAndLabelType(this); //clobbers any expr it
	if(!m_state.okUTItoContinue(duti))
	  {
	    setNodeType(duti);
	    return duti;
	  }
	//NODE_ASSERT(m_state.getUlamTypeByIndex(duti)->getUlamTypeEnum() == Class); why a class? t41306
      }

    setupBlockNo(); //in case zero, may use nodetypedesc

    NodeBlockContext * currentContextBlock = m_state.getContextBlockForSearching();
    NODE_ASSERT(currentContextBlock);
    UTI cbuti = currentContextBlock->getNodeType(); //was getCompileThisIdx()
    if(!m_state.okUTItoContinue(cbuti)) cbuti = m_state.getCompileThisIdx(); //t3336
    bool astub = m_state.isClassAStub(cbuti);

    //instantiate, look up in class block;
    if(m_constSymbol == NULL) //t41440?
      {
	checkForSymbol();
	if(m_constSymbol)
	  {
	    TBOOL rtb = replaceOurselves(m_constSymbol, thisparentnode);
	    if(rtb == TBOOL_HAZY)
	      {
		clearSymbolPtr(); //lookup again too! (e.g. inherited template instances)
		m_state.setGoAgain();
		setNodeType(Hzy);
		return Hzy;
	      }
	    if(rtb == TBOOL_TRUE)
	      {
		m_state.setGoAgain();

		delete this; //suicide is painless..

		return Hzy; //t41266,t41274
	      }
	    //else keep it!
	  }
      }
    else
      {
	astub = m_state.hasClassAStubInHierarchy(cbuti); //includes ancestors
      }

    if(m_constSymbol)
      {
	it = checkUsedBeforeDeclared(); //m_constSymbol->getUlamTypeIdx();
      }
    else if(isReadyConstant() && astub)
      {
	NODE_ASSERT(m_state.okUTItoContinue(m_constType));
	setNodeType(m_constType); //t3565, t3640, t3641, t3642, t3652
	//stub copy case: still wants uti mapping
	it = NodeTerminal::checkAndLabelType(thisparentnode);
      }
    else if(astub)
      {
	// still need its symbol for a value
	// use the member class (unlike checkForSymbol)
      }

    if(m_state.okUTItoContinue(it))
      {
	// map incomplete UTI
	if(!m_state.isComplete(it)) //reloads to recheck
	  {
	    UTI mappedUTI = it;
	    if(m_state.findRootUTIAlias(it, mappedUTI))
	      {
		std::ostringstream msg;
		msg << "REPLACE " << prettyNodeName().c_str() << " for type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << ", used with alias type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		it = mappedUTI;
	      }

	    if(!m_state.isComplete(it)) //reloads to recheck
	      {
		std::ostringstream msg;
		msg << "Incomplete " << prettyNodeName().c_str() << " for type";
		if(m_state.okUTItoContinue(it) && !m_state.isHolder(it))
		  msg << ": " << m_state.getUlamTypeNameByIndex(it).c_str();
		msg << " used with constant symbol name '" << getName() << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		//wait until updateConstant tried.
	      }
	  }
	NODE_ASSERT(m_state.isScalar(it));
      }

    //copy m_constant from Symbol into NodeTerminal parent.
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here
    if(!isReadyConstant())
      {
	UTI cuti = m_state.getCompileThisIdx();
	std::ostringstream msg;
	msg << "Not ready ";
	if(isAConstant())
	  msg << "constant for type: ";
	else
	  msg << "Model Parameter for type: "; //t3443
	msg << m_state.getUlamTypeNameByIndex(it).c_str(); //t41456
	msg << ", used with symbol name '";
	msg << m_state.getTokenDataAsString(m_token).c_str() << "'";
	msg << ", while compiling " << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	msg << " (UTI " << cuti << ")";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);  //t41192
	it = Hzy;
      }
    setNodeType(it);
    Node::setStoreIntoAble(TBOOL_FALSE);
    if(getNodeType() == Hzy)
      {
	if(m_constSymbol)
	  {
	    if(!((SymbolConstantValue *) m_constSymbol)->isALocalConstantDef())
	      clearSymbolPtr(); //lookup again too! (e.g. inherited template instances) t3566;
	    //else except for, possibly nested, local constants (t41682)
	  }
	//else
	m_state.setGoAgain();
      }
    else
      {
	NodeBlock * currBlock = getBlock();
	NODE_ASSERT(currBlock);
	if(currBlock->isAClassBlock())
	  {
	    UTI cbuti = currBlock->getNodeType();
	    if(m_state.okUTItoContinue(cbuti))
	      m_state.addCompleteUlamTypeToThisContextSet(cbuti); //~t41605
	  }
      }
    return getNodeType(); //it; just to be sure..
  } //checkAndLabelType

  void NodeConstant::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    setBlock(currBlock);

    if(m_state.useMemberBlock())
      m_state.pushCurrentBlock(currBlock); //e.g. memberselect needed for already defined
    else if(m_nodeTypeDesc)
      m_state.pushClassContextUsingMemberClassBlock((NodeBlockClass *) currBlock); //t41547
    else
      m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
      {
	if(asymptr->isConstant())
	  {
	    m_constSymbol = (SymbolConstantValue *) asymptr;
	  }
	else if(asymptr->isModelParameter())
	  {
	    //temporarily, before surgery t41282
	    m_constSymbol = (SymbolConstantValue *) asymptr;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg <<"(1) ";
	    msg << "'" << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "' is not a constant, and cannot be used as one with ";
	    if(m_nodeTypeDesc && m_state.isAClass(m_nodeTypeDesc->getNodeType()))
	      {
		msg << "a class type: "; //t41148
		msg << m_state.m_pool.getDataAsString(m_nodeTypeDesc->getTypeNameId()).c_str();
	      }
	    else
	      {
		msg << "type: "; //t41306
		if(m_nodeTypeDesc)
		  msg << m_state.m_pool.getDataAsString(m_nodeTypeDesc->getTypeNameId()).c_str();
		else
		  msg << m_state.getUlamTypeNameByIndex(asymptr->getUlamTypeIdx()).c_str();
	      }
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Named Constant '" << m_state.getTokenDataAsString(m_token).c_str();
	msg << "' is not defined, or was used before declared in a function";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //was debug
      }
    m_state.popClassContext(); //restore
  } //checkForSymbol

  TBOOL NodeConstant::replaceOurselves(Symbol * symptr, Node * parentnode)
  {
    NODE_ASSERT(symptr); //don't pass on, may become stale (t41433)

    TBOOL rtb = TBOOL_FALSE;
    UTI suti = symptr->getUlamTypeIdx();
    NNO blocknoST = symptr->getBlockNoOfST();

    if(!m_state.okUTItoContinue(suti))
      return TBOOL_HAZY; //t3894

    if(m_state.isAClass(suti))
      {
	if(!m_state.isASeenClass(suti) || m_state.isAnonymousClass(suti))
	  return TBOOL_HAZY;

	Node * newnode = NULL;
	if(m_state.isScalar(suti))
	  newnode = new NodeConstantClass(m_token, blocknoST, suti, m_nodeTypeDesc, m_state);
	else
	  newnode = new NodeConstantClassArray(m_token, blocknoST, suti, m_nodeTypeDesc, m_state); //t41261

	NODE_ASSERT(newnode);
	AssertBool swapOk = Node::exchangeNodeWithParent(newnode, parentnode);
	NODE_ASSERT(swapOk);

	m_nodeTypeDesc = NULL; //tfr to new node
	rtb = TBOOL_TRUE;
      }
    else if(m_state.isAtom(suti)) //support for constant atom arrays (t41484)
      {
	Node * newnode = NULL;
	if(m_state.isScalar(suti))
	  newnode = new NodeConstantClass(m_token, blocknoST, suti, m_nodeTypeDesc, m_state);
	else
	  newnode = new NodeConstantClassArray(m_token, blocknoST, suti, m_nodeTypeDesc, m_state); //t41483
	NODE_ASSERT(newnode);
	AssertBool swapOk = Node::exchangeNodeWithParent(newnode, parentnode);
	NODE_ASSERT(swapOk);

	m_nodeTypeDesc = NULL; //tfr to new node
	rtb = TBOOL_TRUE;
      }
    else if(!m_state.isScalar(suti))
      {
	NodeConstantArray * newnode = new NodeConstantArray(m_token, blocknoST, suti, m_nodeTypeDesc, m_state); //t41261
	NODE_ASSERT(newnode);

	AssertBool swapOk = Node::exchangeNodeWithParent(newnode, parentnode);
	NODE_ASSERT(swapOk);

	m_nodeTypeDesc = NULL; //tfr to new node
	rtb = TBOOL_TRUE;
      }
    else if(symptr->isModelParameter())
      {
	// replace ourselves with a parameter node instead;
	// same node no, and loc
	NodeModelParameter * newnode = new NodeModelParameter(m_token, blocknoST, suti, m_nodeTypeDesc, m_state);
	NODE_ASSERT(newnode);

	AssertBool swapOk = Node::exchangeNodeWithParent(newnode, parentnode);
	NODE_ASSERT(swapOk);

	m_nodeTypeDesc = NULL; //tfr to new node
	rtb = TBOOL_TRUE;
      }
    //else did not replace ourselves
    return rtb;
  } //replaceOurselves

  UTI NodeConstant::checkUsedBeforeDeclared()
  {
    NODE_ASSERT(m_constSymbol);
    UTI rtnuti = m_constSymbol->getUlamTypeIdx();

    if(((SymbolConstantValue *) m_constSymbol)->isALocalConstantDef() && (m_constSymbol->getDeclNodeNo() > getNodeNo()))
      {
	NodeBlock * currBlock = getBlock();
	NodeBlock * pcurrBlock = currBlock->getPreviousBlockPointer();
	std::ostringstream msg;
	msg << "Named constant '" << getName();
	msg << "' was used before declared in a function scope";
	if(pcurrBlock)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setBlockNo(pcurrBlock->getNodeNo());
	    clearSymbolPtr(); //t3323
	    rtnuti = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    clearSymbolPtr();
	    rtnuti = Nav;
	  }
      }
    //else t41215 (e.g. class parameter)
    return rtnuti;
  } //checkUsedBeforeDeclared

  TBOOL NodeConstant::checkVarUsedBeforeDeclared(u32 id, NNO declblockno)
  {
    if(m_token.m_dataindex != id)
      return TBOOL_FALSE; //ok

    if(!m_constSymbol)
      return TBOOL_HAZY; //t41678

    // error if use comes before end of decl;
    //  called by NodeConstantDef (t41678), NodeVarDecl (t41674)
    if(((SymbolConstantValue *) m_constSymbol)->isALocalConstantDef())
      {
	if(getBlockNo() < declblockno )
	  return TBOOL_FALSE; //ok symbol w same name not in same block

	//and try previous block (t41682); if symbol BlockNo is the same as current block no;
	NodeBlock * currBlock = getBlock();
	currBlock = currBlock->getPreviousBlockPointer();
	if(currBlock)
	  {
	    setBlockNo(currBlock->getNodeNo());
	    clearSymbolPtr();
	    m_state.setGoAgain();
	    setNodeType(Hzy);
	  }

	std::ostringstream msg;
	msg << "Local named constant '" << getName();
	msg << "' was used before declaration completed";
	if(getNodeType() == Hzy)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    return TBOOL_HAZY;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return TBOOL_TRUE; //error
	  }
      }
    return TBOOL_FALSE;
  } //checkVarUsedBeforeDeclared

  //borrowed from NodeIdent
  void NodeConstant::setupBlockNo()
  {
    bool needsapop = false;
    //define before used, start search with current block
    if(m_currBlockNo == 0)
      {
	if(m_nodeTypeDesc)
	  {
	    UTI duti = m_nodeTypeDesc->getNodeType();
	    NODE_ASSERT(m_state.okUTItoContinue(duti));

	    //NODE_ASSERT(m_state.isAClass(duti)); //t3796, t3861,2,5,8..
	    if(m_state.isAClass(duti)) //t3796, t3861,2,5,8..
	      {
		SymbolClass * acsym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClass(duti, acsym);
		NODE_ASSERT(isDefined);

		NodeBlockClass * memberClassNode = acsym->getClassBlockNode();
		NODE_ASSERT(memberClassNode); //e.g. forgot the closing brace on quark def once; or UNSEEN

		//set up compiler state to use the member class block for symbol searches
		m_state.pushClassContextUsingMemberClassBlock(memberClassNode);
		needsapop = true;
	      }
	  }

	if(m_state.useMemberBlock())
	  {
	    NodeBlockClass * memberclass = m_state.getCurrentMemberClassBlock();
	    NODE_ASSERT(memberclass);
	    setBlockNo(memberclass->getNodeNo());
	  }
	else
	  setBlockNo(m_state.getCurrentBlockNo());

	if(needsapop)
	  m_state.popClassContext(); //restore
      }
  } //setupBlockNo

  void NodeConstant::setBlockNo(NNO n)
  {
    NODE_ASSERT(n > 0);
    m_currBlockNo = n;
    m_currBlockPtr = NULL; //not owned, just clear
  }

  NNO NodeConstant::getBlockNo() const
  {
    return m_currBlockNo;
  }

  void NodeConstant::setBlock(NodeBlock * ptr)
  {
    m_currBlockPtr = ptr;
  }

  NodeBlock * NodeConstant::getBlock()
  {
    NODE_ASSERT(m_currBlockNo);
    if(m_currBlockPtr)
      return m_currBlockPtr;

    NodeBlock * currBlock = NULL;
    if(m_nodeTypeDesc && m_state.isAClass(m_nodeTypeDesc->getNodeType()))
      currBlock = (NodeBlock *) m_state.findNodeNoInAClassOrLocalsScope(m_currBlockNo, m_nodeTypeDesc->getNodeType()); //t41284,5; t41306
    else
      currBlock = (NodeBlock *) m_state.findNodeNoInThisClassOrLocalsScope(m_currBlockNo); //t3328, t3329, t3330, t3332 (not using StubFirst version)

    if(!currBlock)
      {
	UTI anotherclassuti = m_state.findAClassByNodeNo(m_currBlockNo);
	if(anotherclassuti != Nouti) //could be hzy (t41149)
	  {
	    currBlock = m_state.getAClassBlock(anotherclassuti);
	    NODE_ASSERT(currBlock);
	    if(currBlock->getNodeNo() != m_currBlockNo)
	      currBlock = NULL;
	  }
	//try local scope (for constants)
	if(!currBlock)
	  currBlock = m_state.findALocalsScopeByNodeNo(m_currBlockNo);
      }
    NODE_ASSERT(currBlock);
    return currBlock;
  }

  EvalStatus NodeConstant::eval()
  {
    if(!isReadyConstant())
      m_ready = updateConstant();
    if(!isReadyConstant())
      return evalStatusReturnNoEpilog(NOTREADY);
    if(!m_state.isComplete(getNodeType()))
      return evalErrorReturn();
    return NodeTerminal::eval();
  } //eval

  EvalStatus NodeConstant::evalToStoreInto()
  {
    //possible constant array item (t3881)
    UTI nuti = getNodeType();
    if(nuti == Nav) evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    NODE_ASSERT(m_constSymbol);

    if(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() == 0)
      return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  }

  UlamValue NodeConstant::makeUlamValuePtr()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    NODE_ASSERT(m_constSymbol);
    NODE_ASSERT(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() > 0);

    UlamValue absptr = UlamValue::makePtr(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex(), CNSTSTACK, nuti, nut->getPackable(), m_state, 0, m_constSymbol->getId());
    absptr.setUlamValueTypeIdx(PtrAbs);

    return absptr;
  }

  void NodeConstant::genCode(File * fp, UVPass& uvpass)
  {
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here
    NODE_ASSERT(isReadyConstant()); //must be
    NodeTerminal::genCode(fp, uvpass);
  } //genCode

  void NodeConstant::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    NODE_ASSERT(isReadyConstant()); //must be

    genCode(fp, uvpass);

    Node::genCodeConvertATmpVarIntoBitVector(fp, uvpass);

    m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL);
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
    //******UPDATED GLOBAL; no restore!!!**************************
  } //genCodeToStoreInto

  bool NodeConstant::updateConstant()
  {
    bool brtn = false;
    u64 val;
    if(!m_constSymbol)
      return false;
    if(m_constSymbol->getValue(val))
      {
	m_constant.uval = val; //value fits type per its constantdef
	brtn = true;
	NODE_ASSERT(m_constSymbol->isReady()); //true;
      }
    //else don't want default value here

    return brtn;
  } //updateConstant

} //end MFM
