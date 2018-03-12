#include <stdlib.h>
#include "NodeConstant.h"
#include "NodeConstantArray.h"
#include "NodeConstantClass.h"
#include "NodeModelParameter.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstant::NodeConstant(const Token& tok, SymbolWithValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state) : NodeTerminal(state), m_token(tok), m_nodeTypeDesc(typedesc), m_constSymbol(symptr), m_ready(false), m_constType(Nouti), m_currBlockNo(0), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  {
    if(symptr)
      {
	setBlockNo(symptr->getBlockNoOfST());
	m_ready = updateConstant(); //sets ready here
	m_constType = m_constSymbol->getUlamTypeIdx();
      }
    //else t41148
  }

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
    if(isReadyConstant())
      return NodeTerminal::getName();
    return m_state.getTokenDataAsString(m_token).c_str();
  }

  const std::string NodeConstant::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeConstant::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  bool NodeConstant::hasASymbolDataMember()
  {
    assert(m_constSymbol);
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

  UTI NodeConstant::checkAndLabelType()
  {
    UTI it = getNodeType(); //was Nav; but could be repeated c&l call.

    if(m_nodeTypeDesc)
      {
	UTI duti = m_nodeTypeDesc->checkAndLabelType(); //clobbers any expr it
	if(!m_state.okUTItoContinue(duti))
	  {
	    setNodeType(duti);
	    return duti;
	  }
	assert(m_state.getUlamTypeByIndex(duti)->getUlamTypeEnum() == Class);
      }

    setupBlockNo(); //in case zero, may use nodetypedesc

    bool stubcopy = m_state.isClassAStub(m_state.getCompileThisIdx());

    //instantiate, look up in class block; skip if stub copy and already ready.
    if(!stubcopy && m_constSymbol == NULL)
      {
	checkForSymbol();
	if(m_constSymbol)
	  {
	    UTI suti = m_constSymbol->getUlamTypeIdx();
	    if(m_state.isAClass(suti))
	      {
		NodeConstantClass * newnode = new NodeConstantClass(m_token, (SymbolConstantValue *) m_constSymbol, m_nodeTypeDesc, m_state);
		assert(newnode);

		AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
		assert(swapOk);

		m_nodeTypeDesc = NULL; //tfr to new node
		delete this; //suicide is painless..

		return newnode->checkAndLabelType();
	      }
	    else if(!m_state.isScalar(suti))
	      {
		NodeConstantArray * newnode = new NodeConstantArray(m_token, (SymbolConstantValue *) m_constSymbol, m_nodeTypeDesc, m_state);
		assert(newnode);

		AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
		assert(swapOk);

		m_nodeTypeDesc = NULL; //tfr to new node
		delete this; //suicide is painless..

		return newnode->checkAndLabelType();
	      }
	    else if(m_constSymbol->isModelParameter())
	      {
		// replace ourselves with a parameter node instead;
		// same node no, and loc
		NodeModelParameter * newnode = new NodeModelParameter(m_token, (SymbolModelParameterValue*) m_constSymbol, m_nodeTypeDesc, m_state);
		assert(newnode);

		AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
		assert(swapOk);

		m_nodeTypeDesc = NULL; //tfr to new node
		delete this; //suicide is painless..

		return newnode->checkAndLabelType();
	      }
	    //else
	  }
      }
    else
      {
	stubcopy = m_state.hasClassAStub(m_state.getCompileThisIdx()); //includes ancestors
      }

    if(m_constSymbol)
      {
	it = checkUsedBeforeDeclared(); //m_constSymbol->getUlamTypeIdx();
      }
    else if(isReadyConstant() && stubcopy)
      {
	assert(m_state.okUTItoContinue(m_constType));
	setNodeType(m_constType); //t3565, t3640, t3641, t3642, t3652
	//stub copy case: still wants uti mapping
	it = NodeTerminal::checkAndLabelType();
      }
    else if(stubcopy)
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
	    if(m_state.findaUTIAlias(it, mappedUTI))
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
		msg << "Incomplete " << prettyNodeName().c_str() << " for type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << ", used with constant symbol name '";
		msg << m_state.getTokenDataAsString(m_token).c_str() << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		//wait until updateConstant tried.
	      }
	  }
	assert(m_state.isScalar(it));
      }

    //copy m_constant from Symbol into NodeTerminal parent.
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here
    if(!isReadyConstant())
      {
	std::ostringstream msg;
	msg << "Not ready constant for type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << ", used with constant symbol name '";
	msg << m_state.getTokenDataAsString(m_token).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);  //t41192
	it = Hzy;
	if(!stubcopy)
	  {
	    m_constSymbol = NULL; //lookup again too! (e.g. inherited template instances)
	    setBlock(NULL);
	  }
      }

    setNodeType(it);
    Node::setStoreIntoAble(TBOOL_FALSE);

    if(getNodeType() == Hzy)
      m_state.setGoAgain();
    return getNodeType(); //it; just to be sure..
  } //checkAndLabelType

  void NodeConstant::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    setBlock(currBlock);

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
	    //temporarily, before surgery
	    m_constSymbol = (SymbolConstantValue *) asymptr;
	    return;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg <<"(1) ";
	    msg << "<" << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "> is not a constant, and cannot be used as one with a class type: ";
	    msg << getBlock()->getName(); //t41148
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Named Constant <" << m_state.getTokenDataAsString(m_token).c_str();
	msg << "> is not defined, or was used before declared in a function";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    m_state.popClassContext(); //restore
  } //checkForSymbol

  UTI NodeConstant::checkUsedBeforeDeclared()
  {
    assert(m_constSymbol);
    UTI rtnuti = m_constSymbol->getUlamTypeIdx();

    if(!m_constSymbol->isDataMember() && !m_constSymbol->isLocalsFilescopeDef() && !m_constSymbol->isClassArgument() && !m_constSymbol->isClassParameter() && (m_constSymbol->getDeclNodeNo() > getNodeNo()))
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
	    m_constSymbol = NULL; //t3323
	    rtnuti = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    m_constSymbol = NULL;
	    setBlock(NULL);
	    rtnuti = Nav;
	  }
      }
    //else t41215 (e.g. class parameter)
    return rtnuti;
  } //checkUsedBeforeDeclared

  //borrowed from NodeIdent
  void NodeConstant::setupBlockNo()
  {
    //define before used, start search with current block
    if(m_currBlockNo == 0)
      {
	if(m_nodeTypeDesc)
	  {
	    UTI duti = m_nodeTypeDesc->getNodeType();
	    assert(m_state.okUTItoContinue(duti));
	    assert(m_state.isAClass(duti)); //t3796, t3861,2,5,8..
	    SymbolClass * acsym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(duti, acsym);
	    assert(isDefined);

	    NodeBlockClass * memberClassNode = acsym->getClassBlockNode();
	    assert(memberClassNode); //e.g. forgot the closing brace on quark def once; or UNSEEN

	    //set up compiler state to use the member class block for symbol searches
	    m_state.pushClassContextUsingMemberClassBlock(memberClassNode);
	  }

	if(m_state.useMemberBlock())
	  {
	    NodeBlockClass * memberclass = m_state.getCurrentMemberClassBlock();
	    assert(memberclass);
	    setBlockNo(memberclass->getNodeNo());
	  }
	else
	  setBlockNo(m_state.getCurrentBlockNo());

	if(m_nodeTypeDesc)
	  m_state.popClassContext(); //restore
      }
  } //setupBlockNo

  void NodeConstant::setBlockNo(NNO n)
  {
    assert(n > 0);
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
    assert(m_currBlockNo);
    if(m_currBlockPtr)
      return m_currBlockPtr;

    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClassOrLocalsScope(m_currBlockNo); //t3328, t3329, t3330, t3332 (not using StubFirst version)
    if(!currBlock)
      {
	UTI anotherclassuti = m_state.findAClassByNodeNo(m_currBlockNo);
	if(anotherclassuti != Nouti) //could be hzy (t41149)
	  {
	    currBlock = m_state.getAClassBlock(anotherclassuti);
	    assert(currBlock);
	    if(currBlock->getNodeNo() != m_currBlockNo)
	      currBlock = NULL;
	  }
	//try local scope (for constants)
	if(!currBlock)
	  currBlock = m_state.findALocalsScopeByNodeNo(m_currBlockNo);
      }
    assert(currBlock);
    return currBlock;
  }

  //class context set prior to calling us; purpose is to get
  // the value of this constant from the context before
  // constant folding happens.
  bool NodeConstant::assignClassArgValueInStubCopy()
  {
    // insure current block NNOs match
    if(m_currBlockNo != m_state.getCurrentBlockNo())
      {
	std::ostringstream msg;
	msg << "Block NNO " << m_currBlockNo << " for <";
	msg << m_state.getTokenDataAsString(m_token).c_str();
	msg << "> does not match the current block no ";
	msg << m_state.getCurrentBlockNo();
	msg << "; its value cannot be used in stub copy, with class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return false;
      }

    if(m_ready)
      return true; //nothing to do

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
      {
	assert(hazyKin); //always hazy, right?
	if(asymptr->isConstant() && ((SymbolConstantValue *) asymptr)->isReady()) //???
	  {
	    u64 val = 0;
	    ((SymbolConstantValue *) asymptr)->getValue(val);
	    m_constant.uval = val;
	    m_ready = true;
	    //note: m_constSymbol may be NULL; ok in this circumstance (i.e. stub copy).
	  }
      }
    return m_ready;
  } //assignClassArgValueInStubCopy

  EvalStatus NodeConstant::eval()
  {
    if(!isReadyConstant())
      m_ready = updateConstant();
    if(!isReadyConstant())
      return NOTREADY;
    if(!m_state.isComplete(getNodeType()))
      return ERROR;
    return NodeTerminal::eval();
  } //eval

  EvalStatus NodeConstant::evalToStoreInto()
  {
    //possible constant array item (t3881)
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    assert(m_constSymbol);

    if(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() == 0)
      return NOTREADY;

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

    assert(m_constSymbol);
    assert(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() > 0);

    UlamValue absptr = UlamValue::makePtr(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex(), CNSTSTACK, nuti, nut->getPackable(), m_state, 0, m_constSymbol->getId());
    absptr.setUlamValueTypeIdx(PtrAbs);

    return absptr;
  }

  void NodeConstant::genCode(File * fp, UVPass& uvpass)
  {
    if(!isReadyConstant())
      m_ready = updateConstant(); //sets ready here
    assert(isReadyConstant()); //must be
    NodeTerminal::genCode(fp, uvpass);
  } //genCode

  void NodeConstant::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    assert(isReadyConstant()); //must be

    genCode(fp, uvpass);

    Node::genCodeConvertATmpVarIntoBitVector(fp, uvpass);

    m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL);
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
    //******UPDATED GLOBAL; no restore!!!**************************
  } //genCodeToStoreInto

  bool NodeConstant::updateConstant()
  {
    u64 val;
    if(!m_constSymbol)
      return false;

    if(m_constSymbol->getValue(val))
	m_constant.uval = val; //value fits type per its constantdef
    //else don't want default value here

    return m_constSymbol->isReady();
  } //updateConstant

} //end MFM
