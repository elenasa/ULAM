#include <stdlib.h>
#include "NodeConstantClass.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstantClass::NodeConstantClass(const Token& tok, SymbolWithValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state) : Node(state), m_token(tok), m_nodeTypeDesc(typedesc), m_constSymbol(symptr), m_constType(Nouti), m_currBlockNo(0), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  {
    assert(symptr);
    setBlockNo(symptr->getBlockNoOfST());
    m_constType = m_constSymbol->getUlamTypeIdx();
  }

  NodeConstantClass::NodeConstantClass(const NodeConstantClass& ref) : Node(ref), m_token(ref.m_token), m_nodeTypeDesc(NULL), m_constSymbol(NULL), m_constType(ref.m_constType), m_currBlockNo(ref.m_currBlockNo), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  {
    //can we use the same address for a constant symbol?
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeConstantClass::~NodeConstantClass()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;

    delete m_tmpvarSymbol;
    m_tmpvarSymbol = NULL;
  }

  Node * NodeConstantClass::instantiate()
  {
    return new NodeConstantClass(*this);
  }

  void NodeConstantClass::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeConstantClass::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeConstantClass::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }

  const char * NodeConstantClass::getName()
  {
    return m_state.getTokenDataAsString(m_token).c_str();
  }

  const std::string NodeConstantClass::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeConstantClass::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  bool NodeConstantClass::hasASymbolDataMember()
  {
    assert(m_constSymbol);
    return m_constSymbol->isDataMember();
  }

  bool NodeConstantClass::isReadyConstant()
  {
    return m_constSymbol && (m_constSymbol->isReady() || m_constSymbol->isInitValueReady()); //t41209
  }

  bool NodeConstantClass::isAConstant()
  {
    return true;
  }

  bool NodeConstantClass::isAConstantClass()
  {
    return true;
  }

  void NodeConstantClass::setClassType(UTI cuti)
  {
    //noop
  }

  FORECAST NodeConstantClass::safeToCastTo(UTI newType)
  {
    if(isReadyConstant())
      {
	UlamType * newut = m_state.getUlamTypeByIndex(newType);
	return newut->safeCast(getNodeType());
      }
    return CAST_HAZY;
  } //safeToCastTo

  UTI NodeConstantClass::checkAndLabelType()
  {
    UTI it = Nouti;

    bool stubcopy = m_state.isClassAStub(m_state.getCompileThisIdx());

    //instantiate, look up in class block; skip if stub copy and already ready.
    if(!stubcopy && m_constSymbol == NULL)
      checkForSymbol();
    else
      stubcopy = m_state.hasClassAStub(m_state.getCompileThisIdx()); //includes ancestors

    if(m_constSymbol)
      {
	it = checkUsedBeforeDeclared(); //m_constSymbol->getUlamTypeIdx();
      }
    else if(stubcopy)
      {
	assert(m_state.okUTItoContinue(m_constType));
	setNodeType(m_constType); //t3565, t3640, t3641, t3642, t3652
	//stub copy case: still wants uti mapping
	it = m_constType;
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
		m_constType = it; //consistency
		m_constSymbol->resetUlamType(it); //consistency
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
      } //ok

    if(!isReadyConstant())
      {
	//foldConstantClassNodes(); //may reset nodetype
	//wait to refresh named constant value from constant def built after c&l
	//t41213,4,6 and t41220,4
	std::ostringstream msg;
	msg << "Cannot update <" << m_state.getTokenDataAsString(m_token).c_str();
	msg << "> class constant is not ready ";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	if(it != Nav)
	  it = Hzy;
      }

    setNodeType(it);
    Node::setStoreIntoAble(TBOOL_FALSE);

    if(it==Hzy)
      m_state.setGoAgain();

    return getNodeType(); //it
  } //checkAndLabelType

  void NodeConstantClass::checkForSymbol()
  {
    setupBlockNo(); //in case of 0
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    if(currBlock == NULL)
      return; //no symbol

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
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "> is not a constant, and cannot be used as one with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Named Constant Class <" << m_state.getTokenDataAsString(m_token).c_str();
	msg << "> is not defined, or was used before declared in a function";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    m_state.popClassContext(); //restore

  } //checkForSymbol

  UTI NodeConstantClass::checkUsedBeforeDeclared()
  {
    assert(m_constSymbol);
    UTI rtnuti = m_constSymbol->getUlamTypeIdx();

    if(!m_constSymbol->isDataMember() && !m_constSymbol->isLocalsFilescopeDef() && !m_constSymbol->isClassArgument() && !m_constSymbol->isClassParameter() && (m_constSymbol->getDeclNodeNo() > getNodeNo()))
      {
	NodeBlock * currBlock = getBlock();
	currBlock = currBlock->getPreviousBlockPointer();
	std::ostringstream msg;
	msg << "Named constant class '" << getName();
	msg << "' was used before declared in a function";
	if(currBlock)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setBlockNo(currBlock->getNodeNo());
	    m_constSymbol = NULL;
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
    return rtnuti;
  } //checkUsedBeforeDeclared

  void NodeConstantClass::setupBlockNo()
  {
    //define before used, start search with current block (copied from NodeIdent)
    if(m_currBlockNo == 0)
      {
	if(m_state.useMemberBlock())
	  {
	    NodeBlockClass * memberclass = m_state.getCurrentMemberClassBlock();
	    assert(memberclass);
	    setBlockNo(memberclass->getNodeNo());
	  }
	else
	  setBlockNo(m_state.getCurrentBlockNo());
      }
  } //setupBlockNo

  void NodeConstantClass::setBlockNo(NNO n)
  {
    assert(n > 0);
    m_currBlockNo = n;
    m_currBlockPtr = NULL; //not owned, just clear
  }

  NNO NodeConstantClass::getBlockNo() const
  {
    return m_currBlockNo;
  }

  void NodeConstantClass::setBlock(NodeBlock * ptr)
  {
    m_currBlockPtr = ptr;
  }

  NodeBlock * NodeConstantClass::getBlock()
  {
    assert(m_currBlockNo);

    if(m_currBlockPtr)
      return m_currBlockPtr;

    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClassOrLocalsScope(m_currBlockNo);
    if(!currBlock)
      {
	UTI anotherclassuti = m_state.findAClassByNodeNo(m_currBlockNo);
	if(anotherclassuti != Nouti)
	  {
	    currBlock = m_state.getAClassBlock(anotherclassuti);
	    assert(currBlock);
	    if(currBlock->getNodeNo() != m_currBlockNo)
	      currBlock = NULL;
	  }
	//try local scope
	if(!currBlock)
	  currBlock = m_state.findALocalsScopeByNodeNo(m_currBlockNo);
      }
    //assert(currBlock);
    return currBlock; //could be null
  }

  //class context set prior to calling us; purpose is to get
  // the value of this constant from the context before
  // constant folding happens.
  bool NodeConstantClass::assignClassArgValueInStubCopy()
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

    if(isReadyConstant())
      return true; //nothing to do

    bool isready = false;
    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
      {
	assert(hazyKin); //always hazy, right?
	if(asymptr->isConstant() && ((SymbolConstantValue *) asymptr)->isReady()) //???
	  {
	    isready = true;
	    //note: m_constSymbol may be NULL; ok in this circumstance (i.e. stub copy).
	  }
      }
    return isready;
  } //assignClassArgValueInStubCopy

  bool NodeConstantClass::getClassValue(BV8K& bvtmp)
  {
    return m_constSymbol->getValue(bvtmp);
  }

  bool NodeConstantClass::initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask)
  {
    //bvref contains default value at pos 0 of our m_forClassUTI.
    bool rtnok = getClassValue(bvref); //overwrites
    if(rtnok)
      bvmask.SetBits(0, m_state.getUlamTypeByIndex(getNodeType())->getSizeofUlamType()); //t41229, t41234
    return rtnok;
  }

  EvalStatus NodeConstantClass::eval()
  {
    if(!isReadyConstant())
      return NOTREADY;

    UTI nuti = getNodeType();
    if(!m_state.isComplete(nuti))
      return ERROR;

    if(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() == 0)
      return NOTREADY;

    evalNodeProlog(0); //new current node eval frame pointer, t3897

    UlamValue rtnUVPtr = makeUlamValuePtr();
    Node::assignReturnValueToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeConstantClass::evalToStoreInto()
  {
    //possible access of constant class item (t3881)
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
  } //evalToStoreInto

  UlamValue NodeConstantClass::makeUlamValuePtr()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    assert(m_constSymbol);
    assert(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() > 0);

    UlamValue absptr = UlamValue::makePtr(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex(), CNSTSTACK, nuti, nut->getPackable(), m_state, 0, m_constSymbol->getId());
    absptr.setUlamValueTypeIdx(PtrAbs);

    return absptr;
  }

  void NodeConstantClass::genCode(File * fp, UVPass& uvpass)
  {
    //return the ptr for an class; square bracket will resolve down to the immediate data
    makeUVPassForCodeGen(uvpass);

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol); //*********UPDATED GLOBAL;

    // UNCLEAR: should this be consistent with constants?
    Node::genCodeReadIntoATmpVar(fp, uvpass);
  } //genCode

  void NodeConstantClass::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    assert(isReadyConstant()); //must be

    makeUVPassForCodeGen(uvpass);
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol);

    // make a temporary immediate of class constant type that can be
    // referenced as a constant function parameter (t41238)
    Node::genCodeReadIntoATmpVar(fp, uvpass);
    Node::genCodeConvertATmpVarIntoBitVector(fp, uvpass);

    m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL);
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);

    //******UPDATED GLOBAL; no restore!!!**************************
  } //genCodeToStoreInto

  void NodeConstantClass::makeUVPassForCodeGen(UVPass& uvpass)
  {
    assert(m_constSymbol);
    s32 tmpnum = m_state.getNextTmpVarNumber();
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    uvpass = UVPass::makePass(tmpnum, nut->getTmpStorageTypeForTmpVar(), nuti, m_state.determinePackable(nuti), m_state, 0, m_constSymbol->getId());
  }

} //end MFM
