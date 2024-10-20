#include <stdlib.h>
#include "NodeConstantClassArray.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstantClassArray::NodeConstantClassArray(const Token& tok, SymbolWithValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state) : Node(state), m_token(tok), m_nodeTypeDesc(typedesc), m_constSymbol(symptr), m_constType(Nouti), m_currBlockNo(0), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  {
    NODE_ASSERT(symptr);
    setBlockNo(symptr->getBlockNoOfST());
    m_constType = m_constSymbol->getUlamTypeIdx();
  }

  NodeConstantClassArray::NodeConstantClassArray(const Token& tok, NNO stblockno, UTI constantType, NodeTypeDescriptor * typedesc, CompilerState & state) : Node(state), m_token(tok), m_nodeTypeDesc(typedesc), m_constSymbol(NULL), m_constType(constantType), m_currBlockNo(stblockno), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  { }

  NodeConstantClassArray::NodeConstantClassArray(const NodeConstantClassArray& ref) : Node(ref), m_token(ref.m_token), m_nodeTypeDesc(NULL), m_constSymbol(NULL), m_constType(ref.m_constType), m_currBlockNo(ref.m_currBlockNo), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  {
    //can we use the same address for a constant symbol?
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeConstantClassArray::~NodeConstantClassArray()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;

    delete m_tmpvarSymbol;
    m_tmpvarSymbol = NULL;
  }

  Node * NodeConstantClassArray::instantiate()
  {
    return new NodeConstantClassArray(*this);
  }

  void NodeConstantClassArray::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeConstantClassArray::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeConstantClassArray::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }

  const char * NodeConstantClassArray::getName()
  {
    return m_state.getTokenDataAsString(m_token).c_str();
  }

  const std::string NodeConstantClassArray::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeConstantClassArray::clearSymbolPtr()
  {
    //if symbol is in a stub, there's no guarantee the stub
    // won't be replace by another duplicate class once its
    // pending args have been resolved.
    m_constSymbol = NULL;
    setBlock(NULL);
  }

  bool NodeConstantClassArray::getSymbolPtr(const Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  bool NodeConstantClassArray::hasASymbolDataMember()
  {
    NODE_ASSERT(m_constSymbol);
    return m_constSymbol->isDataMember();
  }

  bool NodeConstantClassArray::isReadyConstant()
  {
    return m_constSymbol && m_constSymbol->isReady(); //m_ready;
  }

  TBOOL NodeConstantClassArray::isAConstant()
  {
    return TBOOL_TRUE;
  }

  bool NodeConstantClassArray::isAConstantClassArray()
  {
    return true; //t41273
  }

  FORECAST NodeConstantClassArray::safeToCastTo(UTI newType)
  {
    if(isReadyConstant())
      {
	UlamType * newut = m_state.getUlamTypeByIndex(newType);
	return newut->safeCast(getNodeType());
      }
    return CAST_HAZY;
  } //safeToCastTo

  UTI NodeConstantClassArray::checkAndLabelType(Node * thisparentnode)
  {
    UTI it = Nav;

    bool astub = m_state.isClassAStub(m_state.getCompileThisIdx());

    //instantiate, look up in class block; skip if stub copy and already ready.
    //    if(!stubcopy && m_constSymbol == NULL)
    if(m_constSymbol == NULL)
      checkForSymbol();
    else
      astub = m_state.hasClassAStubInHierarchy(m_state.getCompileThisIdx()); //includes ancestors

    if(m_constSymbol)
      it = checkUsedBeforeDeclared(); //m_constSymbol->getUlamTypeIdx();
    else if(astub)
      {
	NODE_ASSERT(m_state.okUTItoContinue(m_constType));
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
		  msg << ": " << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << " used with constant symbol name '" << getName() << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		//wait until updateConstant tried.
	      }
	  }
      }

    if(!isReadyConstant())
      {
	it = Hzy;
	//if(!astub)
	//m_constSymbol = NULL; //lookup again too! (e.g. inherited template instances)
      }

    setNodeType(it);
    Node::setStoreIntoAble(TBOOL_FALSE);

    if(it == Hzy)
      {
	if(m_constSymbol && !((SymbolConstantValue *) m_constSymbol)->isALocalConstantDef())
	  clearSymbolPtr();
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
	      m_state.addCompleteUlamTypeToThisContextSet(cbuti); //t41637
	  }
      }

    return it;
  } //checkAndLabelType

  void NodeConstantClassArray::checkForSymbol()
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
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) '" << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "' is not a constant class array, and cannot be used as one with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Named Constant Array '" << m_state.getTokenDataAsString(m_token).c_str();
	msg << "' is not defined, or was used before declared in a function";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //was debug
      }
    m_state.popClassContext(); //restore
  } //checkForSymbol

  UTI NodeConstantClassArray::checkUsedBeforeDeclared()
  {
    NODE_ASSERT(m_constSymbol);
    UTI rtnuti = m_constSymbol->getUlamTypeIdx();

    if(((SymbolConstantValue *) m_constSymbol)->isALocalConstantDef() && (m_constSymbol->getDeclNodeNo() > getNodeNo()))
      {
	NodeBlock * currBlock = getBlock();
	currBlock = currBlock->getPreviousBlockPointer();
	std::ostringstream msg;
	msg << "Named constant class array '" << getName();
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
	    rtnuti = Nav;
	  }
      }
    return rtnuti;
  } //checkUsedBeforeDeclared

  void NodeConstantClassArray::setBlockNo(NNO n)
  {
    NODE_ASSERT(n > 0);
    m_currBlockNo = n;
    m_currBlockPtr = NULL; //not owned, just clear
  }

  NNO NodeConstantClassArray::getBlockNo() const
  {
    return m_currBlockNo;
  }

  void NodeConstantClassArray::setBlock(NodeBlock * ptr)
  {
    m_currBlockPtr = ptr;
  }

  NodeBlock * NodeConstantClassArray::getBlock()
  {
    NODE_ASSERT(m_currBlockNo);

    if(m_currBlockPtr)
      return m_currBlockPtr;

    NodeBlock * currBlock = NULL;
    if(m_nodeTypeDesc && m_state.isAClass(m_nodeTypeDesc->getNodeType()))
      currBlock = (NodeBlock *) m_state.findNodeNoInAClassOrLocalsScope(m_currBlockNo, m_nodeTypeDesc->getNodeType()); //t41555
    else
      currBlock = (NodeBlock *) m_state.findNodeNoInThisClassOrLocalsScope(m_currBlockNo);

    if(!currBlock)
      {
	UTI anotherclassuti = m_state.findAClassByNodeNo(m_currBlockNo);
	if(anotherclassuti != Nouti)
	  {
	    currBlock = m_state.getAClassBlock(anotherclassuti);
	    NODE_ASSERT(currBlock);
	    if(currBlock->getNodeNo() != m_currBlockNo)
	      currBlock = NULL;
	  }
	//try local scope
	if(!currBlock)
	  currBlock = m_state.findALocalsScopeByNodeNo(m_currBlockNo);
      }
    NODE_ASSERT(currBlock);
    return currBlock;
  } //getBlock

  bool NodeConstantClassArray::getClassArrayValue(BV8K& bvtmp)
  {
    bool brtn = false;
    if(isReadyConstant())
      brtn = m_constSymbol->getValue(bvtmp);
    return brtn;
  }

  bool NodeConstantClassArray::getConstantValue(BV8K& bval)
  {
    return getClassArrayValue(bval);
  }

  TBOOL NodeConstantClassArray::checkVarUsedBeforeDeclared(u32 id, NNO declblockno)
  {
    if(m_token.m_dataindex != id)
      return TBOOL_FALSE; //ok

    if(!m_constSymbol)
      return TBOOL_HAZY;

    // error if use comes before end of decl;
    //  called by NodeVarDecl or NodeConstantDef.
    if(((SymbolConstantValue *) m_constSymbol)->isALocalConstantDef())
      {
	if(getBlockNo() < declblockno )
	  return TBOOL_FALSE; //ok symbol w same name not in same block

	//and try previous block (t41685); if symbol BlockNo is the same as current block no;
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
	msg << "Named constant class array '" << getName();
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
    return TBOOL_FALSE; //ok moot
  } //checkVarUsedBeforeDeclared

  EvalStatus NodeConstantClassArray::eval()
  {
    if(!isReadyConstant()) return evalStatusReturnNoEpilog(NOTREADY);

    UTI nuti = getNodeType();
    if(!m_state.isComplete(nuti)) return evalErrorReturn();

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getBitSize() > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE);

    NODE_ASSERT(m_constSymbol);

    if(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() == 0)
      return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0); //new current node eval frame pointer, t3897

    UlamValue rtnUVPtr = makeUlamValuePtr();
    Node::assignReturnValueToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeConstantClassArray::evalToStoreInto()
  {
    //possible access of constant array item (t3881)
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    //    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
    if((classtype == UC_TRANSIENT) && (nut->getBitSize() > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE); //t41269, t41632

    NODE_ASSERT(m_constSymbol);

    if(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() == 0)
      return evalStatusReturnNoEpilog(NOTREADY);;

    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeConstantClassArray::makeUlamValuePtr()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    NODE_ASSERT(m_constSymbol);
    NODE_ASSERT(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() > 0);

    UlamValue absptr = UlamValue::makePtr(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex(), CNSTSTACK, nuti, nut->getPackable(), m_state, 0, m_constSymbol->getId());
    absptr.setUlamValueTypeIdx(PtrAbs);

    return absptr;
  }

  void NodeConstantClassArray::genCode(File * fp, UVPass& uvpass)
  {
    NODE_ASSERT(isReadyConstant()); //must be

    //return the ptr for an array; square bracket will resolve down to the immediate data
    makeUVPassForCodeGen(uvpass);

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol); //*********UPDATED GLOBAL;

    Node::genCodeReadFromAConstantClassIntoATmpVar(fp, uvpass);
  } //genCode

  void NodeConstantClassArray::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    NODE_ASSERT(isReadyConstant()); //must be
    makeUVPassForCodeGen(uvpass);

    //******UPDATED GLOBAL; no restore!!!**************************
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol);
  } //genCodeToStoreInto

  void NodeConstantClassArray::makeUVPassForCodeGen(UVPass& uvpass)
  {
    NODE_ASSERT(m_constSymbol);
    s32 tmpnum = m_state.getNextTmpVarNumber();
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    uvpass = UVPass::makePass(tmpnum, nut->getTmpStorageTypeForTmpVar(), nuti, m_state.determinePackable(nuti), m_state, 0, m_constSymbol->getId());
  }

} //end MFM
