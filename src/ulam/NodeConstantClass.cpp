#include <stdlib.h>
#include "NodeConstantClass.h"
#include "CompilerState.h"
#include "Parity2D_4x4.h"

namespace MFM {

  NodeConstantClass::NodeConstantClass(const Token& tok, SymbolWithValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state) : Node(state), m_token(tok), m_nodeTypeDesc(typedesc), m_constSymbol(symptr), m_constType(Nouti), m_currBlockNo(0), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  {
    NODE_ASSERT(symptr);
    setBlockNo(symptr->getBlockNoOfST());
    m_constType = m_constSymbol->getUlamTypeIdx();
  }

  NodeConstantClass::NodeConstantClass(const Token& tok, NNO stblockno, UTI constantType, NodeTypeDescriptor * typedesc, CompilerState & state) : Node(state), m_token(tok), m_nodeTypeDesc(typedesc), m_constSymbol(NULL), m_constType(constantType), m_currBlockNo(stblockno), m_currBlockPtr(NULL), m_tmpvarSymbol(NULL)
  { }

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

    //m_constSymbol is a pointer to a symbol that we do not own; hence, not deleted here.
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

  void NodeConstantClass::clearSymbolPtr()
  {
    //if symbol is in a stub, there's no guarantee the stub
    // won't be replace by another duplicate class once its
    // pending args have been resolved.
    m_constSymbol = NULL;
    setBlock(NULL);
  }

  bool NodeConstantClass::getSymbolPtr(const Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  bool NodeConstantClass::hasASymbol()
  {
    return (m_constSymbol != NULL);
  }

  u32 NodeConstantClass::getSymbolId()
  {
    NODE_ASSERT(m_constSymbol);
    return m_constSymbol->getId();
  }

  bool NodeConstantClass::hasASymbolDataMember()
  {
    NODE_ASSERT(m_constSymbol);
    return m_constSymbol->isDataMember();
  }

  u32 NodeConstantClass::getPositionOf()
  {
    NODE_ASSERT(m_constSymbol);
    if(hasASymbolDataMember())
      return m_constSymbol->getPosOffset();
    return 0u;
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

  UTI NodeConstantClass::checkAndLabelType(Node * thisparentnode)
  {
    UTI it = Nouti;

    bool astub = m_state.isClassAStub(m_state.getCompileThisIdx());

    //instantiate, look up in class block; skip if stub copy and already ready.
    //if(!astub && m_constSymbol == NULL)
    if(m_constSymbol == NULL)
      checkForSymbol();
    else
      astub = m_state.hasClassAStubInHierarchy(m_state.getCompileThisIdx()); //includes ancestors

    if(m_constSymbol)
      {
	it = checkUsedBeforeDeclared(); //m_constSymbol->getUlamTypeIdx();
      }
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
		m_constType = it; //consistency
		m_constSymbol->resetUlamType(it); //consistency
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
      } //ok

    if(!isReadyConstant())
      {
	//foldConstantClassNodes(); //may reset nodetype
	//wait to refresh named constant value from constant def built after c&l
	//t41213,4,6 and t41220,4
	std::ostringstream msg;
	msg << "Cannot update '" << m_state.getTokenDataAsString(m_token).c_str();
	msg << "' class constant is not ready ";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	if(it != Nav)
	  it = Hzy;
      }
    setNodeType(it);
    Node::setStoreIntoAble(TBOOL_FALSE);
    if(it==Hzy)
      {
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
	      m_state.addCompleteUlamTypeToThisContextSet(cbuti); //t41605-7
	  }
      }

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
	    msg << "(1) '" << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "' is not a constant class, and cannot be used as one with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Named Constant Class '" << m_state.getTokenDataAsString(m_token).c_str();
	msg << "' is not defined, or was used before declared in a function";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //was debug
      }
    m_state.popClassContext(); //restore

  } //checkForSymbol

  UTI NodeConstantClass::checkUsedBeforeDeclared()
  {
    NODE_ASSERT(m_constSymbol);
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
	    NODE_ASSERT(memberclass);
	    setBlockNo(memberclass->getNodeNo());
	  }
	else
	  setBlockNo(m_state.getCurrentBlockNo());
      }
  } //setupBlockNo

  void NodeConstantClass::setBlockNo(NNO n)
  {
    NODE_ASSERT(n > 0);
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
    NODE_ASSERT(m_currBlockNo);

    if(m_currBlockPtr)
      return m_currBlockPtr;

    NodeBlock * currBlock = NULL;
    if(m_nodeTypeDesc && m_state.isAClass(m_nodeTypeDesc->getNodeType()))
      currBlock = (NodeBlock *) m_state.findNodeNoInAClassOrLocalsScope(m_currBlockNo, m_nodeTypeDesc->getNodeType()); //t41554
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
    //NODE_ASSERT(currBlock);
    return currBlock; //could be null
  }

  bool NodeConstantClass::getClassValue(BV8K& bvtmp)
  {
    return m_constSymbol->getValue(bvtmp); //false, if not ready
  }

  bool NodeConstantClass::getConstantValue(BV8K& bval)
  {
    return getClassValue(bval);
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
    if(!isReadyConstant()) return evalStatusReturnNoEpilog(NOTREADY);

    UTI nuti = getNodeType();
    if(!m_state.isComplete(nuti)) return evalErrorReturn();

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE);

    if(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() == 0)
      return evalStatusReturnNoEpilog(NOTREADY);

    UTI effselfuti = setupStackWithClassForEval(); //t41230

    evalNodeProlog(0); //new current node eval frame pointer, t3897

    UlamValue rtnUVPtr = makeUlamValuePtr();
    rtnUVPtr.setPtrTargetEffSelfType(effselfuti, m_state); //t41494

    Node::assignReturnValueToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeConstantClass::evalToStoreInto()
  {
    //possible access of constant class item (t3881)
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    NODE_ASSERT(m_constSymbol);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE);

    if(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() == 0)
      return evalStatusReturnNoEpilog(NOTREADY);

    UTI effselfuti = setupStackWithClassForEval();

    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();
    rtnUVPtr.setPtrTargetEffSelfType(effselfuti, m_state);

    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeConstantClass::makeUlamValuePtr()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    NODE_ASSERT(m_constSymbol);
    NODE_ASSERT(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex() > 0);

    UlamValue absptr = UlamValue::makePtr(((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex(), CNSTSTACK, nuti, nut->getPackable(), m_state, 0, m_constSymbol->getId());
    absptr.setUlamValueTypeIdx(PtrAbs);

    return absptr;
  }

  UTI NodeConstantClass::setupStackWithClassForEval()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    bool isatom = m_state.isAtom(nuti); //t41483
    NODE_ASSERT(nut->getUlamTypeEnum()==Class || isatom);
    UTI rtneffself = Nav;

    //for eval purposes, a transient must fit into atom state bits, like an element
    // any class may be a data member (see NodeVarDeclDM)
    if(nut->isScalar())
      {
	BV8K bvclass;
	AssertBool gotVal = m_constSymbol->getValue(bvclass);
	u32 len = nut->getBitSize();
	UTI euti = Nouti;
	UlamValue atomUV = UlamValue::makeAtom();
	if(isatom)
	  {
	    u32 elewcorr = bvclass.Read(0u, ATOMFIRSTSTATEBITPOS);
	    u32 eletype = 0;
	    AssertBool gotele = Parity2D_4x4::Remove2DParity(elewcorr, eletype);
	    NODE_ASSERT(gotele);
	    ELE_TYPE ele = (ELE_TYPE) eletype;
	    euti = (ele != UNDEFINED_ELEMENT_TYPE) ? m_state.lookupClassByElementType(ele) : m_state.getEmptyElementUTI();
	    NODE_ASSERT(m_state.isAClass(euti));
	  }
	else
	  {
	    euti = m_constSymbol->getUlamTypeIdx();
	    atomUV.setUlamValueTypeIdx(euti);
	  }
	atomUV.setUlamValueEffSelfTypeIdx(euti);
	rtneffself = euti; //t41494

	if((nut->getUlamClassType()==UC_ELEMENT) || isatom) //support for constant atom (t41483,4)
	  {
	    len = BITSPERATOM; //t41487
	    atomUV.putDataBig(0u, len, bvclass);
	  }
	else
	  atomUV.putDataBig(ATOMFIRSTSTATEBITPOS, len, bvclass); //other classes skip elementtype field

	m_state.m_constantStack.storeUlamValueInSlot(atomUV, ((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex());
      } //not scalar
    else
      m_state.abortShouldntGetHere(); //see NodeConstantClassArray

    return rtneffself;
  } //setupStackWithClassForEval

  void NodeConstantClass::genCode(File * fp, UVPass& uvpass)
  {
    //return the ptr for an class; square bracket will resolve down to the immediate data
    makeUVPassForCodeGen(uvpass);

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol); //*********UPDATED GLOBAL;
    Node::genCodeReadFromAConstantClassIntoATmpVar(fp, uvpass); //not a BV
   } //genCode

  void NodeConstantClass::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    //immediate constants do not require fixing (Element Types and String RegNUM),
    //since it is done at the immediate declaration;
    //only data member and local filesscope (classarguments?);
    if(m_constSymbol->isDataMember() || m_constSymbol->isLocalsFilescopeDef())
      {
	//read in and fix; no further need for DM/Filesscope from symbol (t41198)
	genCode(fp, uvpass); //t41243?, t41273
	Node::genCodeConvertATmpVarIntoBitVector(fp,uvpass);
	m_tmpvarSymbol = Node::makeTmpVarSymbolForCodeGen(uvpass, NULL);
	m_tmpvarSymbol->setDivinedByConstantClass();
	m_state.m_currentObjSymbolsForCodeGen.push_back(m_tmpvarSymbol);
	return;
      }

    NODE_ASSERT(isReadyConstant()); //must be
    makeUVPassForCodeGen(uvpass);

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_constSymbol);
    //******UPDATED GLOBAL; no restore!!!**************************
  } //genCodeToStoreInto

  void NodeConstantClass::makeUVPassForCodeGen(UVPass& uvpass)
  {
    NODE_ASSERT(m_constSymbol);
    s32 tmpnum = m_state.getNextTmpVarNumber();
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    uvpass = UVPass::makePass(tmpnum, nut->getTmpStorageTypeForTmpVar(), nuti, m_state.determinePackable(nuti), m_state, 0, m_constSymbol->getId());
  }

} //end MFM
