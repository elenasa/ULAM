#include <stdlib.h>
#include "NodeConstantClassArray.h"
#include "CompilerState.h"

namespace MFM {

  NodeConstantClassArray::NodeConstantClassArray(const Token& tok, SymbolWithValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state) : Node(state), m_token(tok), m_nodeTypeDesc(typedesc), m_constSymbol(symptr), m_constType(Nouti), m_currBlockNo(0), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  {
    assert(symptr);
    setBlockNo(symptr->getBlockNoOfST());
    m_constType = m_constSymbol->getUlamTypeIdx();
  }

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

  bool NodeConstantClassArray::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  bool NodeConstantClassArray::hasASymbolDataMember()
  {
    assert(m_constSymbol);
    return m_constSymbol->isDataMember();
  }

  bool NodeConstantClassArray::isReadyConstant()
  {
    return m_constSymbol && m_constSymbol->isReady(); //m_ready;
  }

  bool NodeConstantClassArray::isAConstant()
  {
    return true;
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

  UTI NodeConstantClassArray::checkAndLabelType()
  {
    UTI it = Nav;

    bool stubcopy = m_state.isClassAStub(m_state.getCompileThisIdx());

    //instantiate, look up in class block; skip if stub copy and already ready.
    if(!stubcopy && m_constSymbol == NULL)
      checkForSymbol();
    else
      stubcopy = m_state.hasClassAStubInHierarchy(m_state.getCompileThisIdx()); //includes ancestors

    if(m_constSymbol)
      it = checkUsedBeforeDeclared(); //m_constSymbol->getUlamTypeIdx();
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
      }

    if(!isReadyConstant())
      {
	it = Hzy;
	if(!stubcopy)
	  m_constSymbol = NULL; //lookup again too! (e.g. inherited template instances)
      }

    setNodeType(it);
    Node::setStoreIntoAble(TBOOL_FALSE);

    if(it == Hzy) m_state.setGoAgain();
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
	    msg << "' is not a constant, and cannot be used as one with class: ";
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
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    m_state.popClassContext(); //restore
  } //checkForSymbol

  UTI NodeConstantClassArray::checkUsedBeforeDeclared()
  {
    assert(m_constSymbol);
    UTI rtnuti = m_constSymbol->getUlamTypeIdx();

    if(!m_constSymbol->isDataMember() && !m_constSymbol->isLocalsFilescopeDef() && !m_constSymbol->isClassArgument() && !m_constSymbol->isClassParameter() && (m_constSymbol->getDeclNodeNo() > getNodeNo()))
      {
	NodeBlock * currBlock = getBlock();
	currBlock = currBlock->getPreviousBlockPointer();
	std::ostringstream msg;
	msg << "Named constant array '" << getName();
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
    assert(n > 0);
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
    assert(currBlock);
    return currBlock;
  }

  //class context set prior to calling us; purpose is to get
  // the value of this constant from the context before constant folding happens.
  bool NodeConstantClassArray::assignClassArgValueInStubCopy()
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

  EvalStatus NodeConstantClassArray::eval()
  {
    if(!isReadyConstant()) return evalStatusReturnNoEpilog(NOTREADY);

    UTI nuti = getNodeType();
    if(!m_state.isComplete(nuti)) return evalErrorReturn();

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getBitSize() > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE);

    assert(m_constSymbol);

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
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE); //t41269

    assert(m_constSymbol);

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

    assert(m_constSymbol);
    assert(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() > 0);

    UlamValue absptr = UlamValue::makePtr(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex(), CNSTSTACK, nuti, nut->getPackable(), m_state, 0, m_constSymbol->getId());
    absptr.setUlamValueTypeIdx(PtrAbs);

    return absptr;
  }

  void NodeConstantClassArray::genCode(File * fp, UVPass& uvpass)
  {
    assert(isReadyConstant()); //must be

    //return the ptr for an array; square bracket will resolve down to the immediate data
    makeUVPassForCodeGen(uvpass);

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol); //*********UPDATED GLOBAL;

    Node::genCodeReadFromAConstantClassIntoATmpVar(fp, uvpass);
  } //genCode

  void NodeConstantClassArray::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    assert(isReadyConstant()); //must be
    makeUVPassForCodeGen(uvpass);

    //******UPDATED GLOBAL; no restore!!!**************************
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol);
  } //genCodeToStoreInto

  void NodeConstantClassArray::makeUVPassForCodeGen(UVPass& uvpass)
  {
    assert(m_constSymbol);
    s32 tmpnum = m_state.getNextTmpVarNumber();
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    uvpass = UVPass::makePass(tmpnum, nut->getTmpStorageTypeForTmpVar(), nuti, m_state.determinePackable(nuti), m_state, 0, m_constSymbol->getId());
  }

} //end MFM
