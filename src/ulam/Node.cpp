#include <iostream>
#include "Node.h"
#include "CompilerState.h"
#include "NodeBlock.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeFunctionCall.h"
#include "NodeIdent.h"
#include "NodeFuncDecl.h"
#include "NodeMemberSelect.h"
#include "NodeVarDecl.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "SymbolFunction.h"
#include "SymbolFunctionName.h"
#include "UlamTypePrimitiveInt.h"

namespace MFM {

  Node::Node(CompilerState & state): m_state(state), m_utype(Nouti), m_storeIntoAble(TBOOL_FALSE), m_referenceAble(TBOOL_FALSE), m_parentNo(0), m_no(m_state.getNextNodeNo()) {}

  Node::Node(const Node & ref) : m_state(ref.m_state), m_utype(ref.m_utype), m_storeIntoAble(ref.m_storeIntoAble), m_referenceAble(ref.m_referenceAble), m_loc(ref.m_loc), m_parentNo(ref.m_parentNo), m_no(ref.m_no) /* same NNO */ {}

  void Node::setYourParentNo(NNO pno)
  {
    m_parentNo = pno;
  }

  NNO Node::getYourParentNo() const
  {
    return m_parentNo;
  }

  void Node::updateLineage(NNO pno)
  {
    setYourParentNo(pno); //walk the tree..a leaf.
  }

  NNO Node::getNodeNo() const
  {
    return m_no;
  }

  void Node::resetNodeNo(NNO no)
  {
    m_no = no;
  }

  bool Node::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    return false; //default
  }

  bool Node::findNodeNo(NNO n, Node *& foundNode)
  {
    if(m_no == n) //leaf
      {
	foundNode = this;
	return true;
      }
    return false;
  }

  void Node::checkAbstractInstanceErrors()
  {
    return; //default
  }

  void Node::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n",prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  } //print

  u32 Node::getTypeNameId()
  {
    m_state.abortShouldntGetHere(); //NodeVarDecl, NodeBlockFuncDef does work
    return 0;
  }

  void Node::noteTypeAndName(UTI cuti, s32 totalsize, u32& accumsize)
  {
    m_state.abortShouldntGetHere(); //NodeVarDeclDM does work; NodeConstantDef, NodeTypedef bypass
  }

  void Node::genTypeAndNameEntryAsComment(File * fp, s32 totalsize, u32& accumsize)
  {
    m_state.abortShouldntGetHere(); //NodeVarDeclDM does work; NodeConstantDef, NodeTypedef bypass
  }

  const std::string Node::nodeName(const std::string& prettyFunction)
  {
    size_t colons = prettyFunction.find("::");
    if (colons == std::string::npos)
      return "::";
    size_t begin = colons + 2;
    size_t colons2 = prettyFunction.find("::", begin);
    size_t end = colons2 - colons - 2;
    std::string nodename = prettyFunction.substr(begin,end);
    return nodename;
  }

  UTI Node::getNodeType() const
  {
    return m_utype;
  }

  void Node::setNodeType(UTI uti)
  {
    m_utype = uti;
    if(m_state.okUTItoContinue(uti))
      m_state.addCompleteUlamTypeToThisContextSet(uti);
  }

  void Node::resetOfClassType(UTI cuti)
  {
    return; //noop for all except NodeListClassInit, and NodeInitDM
  }

  void Node::setClassType(UTI cuti)
  {
    std::ostringstream msg;
    msg << "virtual void " << prettyNodeName().c_str();
    msg << "::setClassType(UTI cuti){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    return; //noop for all except NodeInitDM
  }

  bool Node::isClassInit()
  {
    return false; //default, except NodeListClassInit
  }

  TBOOL Node::getStoreIntoAble() const
  {
    return m_storeIntoAble;
  }

  void Node::setStoreIntoAble(TBOOL s)
  {
    m_storeIntoAble = s;
    setReferenceAble(s); //usually the case, except custom arrays and func calls
  }

  TBOOL Node::getReferenceAble() const
  {
    return m_referenceAble;
  }

  void Node::setReferenceAble(TBOOL s)
  {
    m_referenceAble = s;
  }

  TBOOL Node::minTBOOL(TBOOL atb, TBOOL btb)
  {
    TBOOL mintb = TBOOL_TRUE;
    if((atb == TBOOL_FALSE) || (btb == TBOOL_FALSE))
      mintb = TBOOL_FALSE;
    else if((atb == TBOOL_HAZY) || (btb == TBOOL_HAZY))
      mintb = TBOOL_HAZY;
    return mintb;
  }

  Locator Node::getNodeLocation() const
  {
    return m_loc;
  }

  void Node::setNodeLocation(Locator loc)
  {
    m_loc = loc;
  }

  void Node::resetNodeLocations(Locator loc)
  {
    m_loc = loc;
  }

  void Node::printNodeLocation(File * fp)
  {
    fp->write(getNodeLocationAsString().c_str());
  }

  std::string Node::getNodeLocationAsString() const
  {
    return m_state.getFullLocationAsString(m_loc);
  }

  bool Node::getSymbolPtr(Symbol *& symptrref)
  {
    return false;
  }

  bool Node::getStorageSymbolPtr(Symbol *& symptrref)
  {
    return false;
  }

  bool Node::hasASymbolDataMember()
  {
    return false;
  }

  bool Node::hasASymbolSuper()
  {
    return false;
  }

  bool Node::hasASymbolSelf()
  {
    return false;
  }

  bool Node::hasASymbolReference()
  {
    return false;
  }

  bool Node::hasASymbolReferenceConstant()
  {
    assert(hasASymbolReference());
    return false;
  }

  bool Node::belongsToVOWN(UTI vown)
  {
    std::ostringstream msg;
    msg << "virtual bool " << prettyNodeName().c_str();
    msg << "::belongsToVOWN(UTI vown){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    return false;
  }

  bool Node::isAConstant()
  {
    return false;
  }

  bool Node::isAConstantClass()
  {
    return false;
  }

  bool Node::isAConstantClassArray()
  {
    return false;
  }

  bool Node::isReadyConstant()
  {
    return false;
  }

  bool Node::isNegativeConstant()
  {
    m_state.abortShouldntGetHere(); // only for constants (NodeTerminal)
    return false;
  }

  bool Node::isWordSizeConstant()
  {
    m_state.abortShouldntGetHere();
    return false;
  }

  bool Node::isFunctionCall()
  {
    return false;
  }

  bool Node::isAConstructorFunctionCall()
  {
    return false;
  }

  bool Node::isArrayItem()
  {
    return false;
  }

  bool Node::isAList()
  {
    return false;
  }

  bool Node::isExplicitReferenceCast()
  {
    return false;
  }

  bool Node::isExplicitCast()
  {
    return false;
  }

  bool Node::asConditionalNode()
  {
    return false;
  }

  bool Node::isAMemberSelect()
  {
    return false;
  }

  bool Node::isTernaryExpression()
  {
    return false;
  }

  bool Node::getConstantValue(BV8K& bval)
  {
    std::ostringstream msg;
    msg << "virtual bool " << prettyNodeName().c_str();
    msg << "::getConstantValue(BV8K& bval){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    return false;
  }

  FORECAST Node::safeToCastTo(UTI newType)
  {
    std::ostringstream msg;
    msg << "virtual FORECAST " << prettyNodeName().c_str();
    msg << "::safeToCastTo(UTI newType){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    return CAST_HAZY;
  }

  bool Node::checkSafeToCastTo(UTI fromType, UTI& newType)
  {
    bool rtnOK = true;
    UlamType * tobe = m_state.getUlamTypeByIndex(newType);
    FORECAST scr = tobe->safeCast(fromType);
    if(scr != CAST_CLEAR)
      {
	ULAMTYPE etyp = tobe->getUlamTypeEnum();
	std::ostringstream msg;
	if(etyp == Bool)
	  msg << "Use a comparison operation";
	else if(etyp == String)
	  msg << "Invalid";
	else if(!m_state.isScalar(newType) || !m_state.isScalar(fromType))
	  msg << "Not possible";
	else
	  msg << "Use explicit cast";
	msg << " to convert "; // the real converting-message
	msg << m_state.getUlamTypeNameBriefByIndex(fromType).c_str();
	msg << " to ";
	msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
	msg << " for '" << getName() << "'";
	if(scr == CAST_HAZY)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    newType = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
	rtnOK = false;
      } //not safe

    return rtnOK;
  } //checkSafeToCastTo

  // any node above assignexpr is not storeintoable;
  // and has no type (e.g. statements, statement, block, program)
  UTI Node::checkAndLabelType()
  {
    m_utype = Nouti;
    m_storeIntoAble = TBOOL_FALSE;
    return m_utype;
  }

  //common to NodeIdent, NodeTerminalProxy, NodeSquareBracket
  bool Node::exchangeNodeWithParent(Node * newnode)
  {
    UTI cuti = m_state.getCompileThisIdx(); //for error messages
    NodeBlock * currBlock = m_state.getCurrentBlock(); //in NodeIdent, getBlock())

    NNO pno = Node::getYourParentNo();

    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock); //push again

    Node * parentNode = m_state.findNodeNoInThisClassForParent(pno);
    assert(parentNode);

    AssertBool swapOk = parentNode->exchangeKids(this, newnode);
    assert(swapOk);

    std::ostringstream msg;
    msg << "Exchanged kids! <" << getName();
    msg << "> func call (" << prettyNodeName().c_str();
    msg << "), within class: ";
    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

    m_state.popClassContext(); //restore

    //common to all new nodes:
    newnode->setNodeLocation(getNodeLocation());
    newnode->resetNodeNo(getNodeNo()); //moved before update lineage
    newnode->updateLineage(pno); //t3942

    return true;
  } //exchangeNodeWithParent

  bool Node::trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr)
  {
    std::ostringstream msg;
    msg << "virtual bool " << prettyNodeName().c_str();
    msg << "::trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    return false;
  }

  void Node::checkForSymbol()
  {
    //for Nodes with Symbols
  }

  void Node::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      {
	ncnt += 1;
	std::ostringstream msg;
	msg << "Erroneous ";
	msg << "'" << getName() << "'";
	msg << " (#" << ncnt << ")";
	//msg << " [" << prettyNodeName().c_str() << "] ";  //ugly!
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }
    else if(m_state.isStillHazy(nuti) || m_state.isHolder(nuti))
      {
	hcnt += 1;
	std::ostringstream msg;
	msg << "Unresolved ";
	msg << "'" << getName() << "'";
	msg << " (#" << hcnt << ")";
	//msg << " [" << prettyNodeName().c_str() << "] ";  //ugly!
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }
    else if(nuti == Nouti)
      {
	nocnt += 1;
	std::ostringstream msg;
	msg << "Untyped ";
	msg << "'" << getName() << "'";
	msg << " (#" << nocnt << ")";
	//msg << " [" << prettyNodeName().c_str() << "] ";  //ugly!
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

#if 0
    //debugg only
    if(m_loc.getLineNo() == 0)
      {
	std::ostringstream msg;
	msg << prettyNodeName().c_str() << " has NO LOC!!";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
#endif
  } //countNavHzyNoutiNodes

  UTI Node::constantFold()
  {
    if(!isAConstant())
      {
	std::ostringstream msg;
	msg << "Cannot constant fold non-constant ";
	msg << "'" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
      }
    //see NodeBinaryOp for the actual work
    return getNodeType(); //noop. parent required (was Nav)
  } //constantFold

  bool Node::buildDefaultValue(u32 wlen, BV8K& dvref)
  {
    std::ostringstream msg;
    msg << "virtual bool " << prettyNodeName().c_str();
    msg << "::buildDefaultValue(u32 wlen, BV8K& dvref){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    return false;
  }

  bool Node::buildDefaultValueForClassConstantDefs()
  {
    std::ostringstream msg;
    msg << "virtual bool " << prettyNodeName().c_str();
    msg << "::buildDefaultValueForClassConstantDefs(){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    return false;
  }

  bool Node::initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask)
  {
    std::ostringstream msg;
    msg << "virtual bool " << prettyNodeName().c_str();
    msg << "::initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    return false; //only for NodeListClassInit, NodeListArrayIniti (t41185)
  }

  bool Node::installSymbolTypedef(TypeArgs& args, Symbol *& asymptr)
  {
    return false;
  }

  bool Node::installSymbolConstantValue(TypeArgs& args, Symbol*& asymptr)
  {
    return false;
  }

  bool Node::installSymbolModelParameterValue(TypeArgs& args, Symbol*& asymptr)
  {
    return false;
  }

  bool Node::installSymbolVariable(TypeArgs& args, Symbol *& asymptr)
  {
    return false;
  }

  bool Node::assignClassArgValueInStubCopy()
  {
    return true; //nothing to do
  }

  EvalStatus Node::evalToStoreInto()
  {
    std::ostringstream msg;
    msg << "Invalid lefthand value (not storeIntoAble) '" << getName() << "'";
    msg << "; Cannot eval";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    assert(getStoreIntoAble() == TBOOL_FALSE);// any node above assignexpr is not storeintoable
    return evalErrorReturn();
  }

  __inline__ EvalStatus Node::evalErrorReturn()
  {
    return evalStatusReturnNoEpilog(ERROR);
  }

  __inline__ EvalStatus Node::evalStatusReturnNoEpilog(EvalStatus evs)
  {
    return evs; //bp for source of non-normal eval status (helps debugging)
  }

  __inline__ EvalStatus Node::evalStatusReturn(EvalStatus evs)
  {
    evalNodeEpilog(); //with epilog
    return evalStatusReturnNoEpilog(evs);
  }

  UlamValue Node::makeUlamValuePtr()
  {
    std::ostringstream msg;
    msg << "virtual UlamValue " << prettyNodeName().c_str();
    msg << "::makeUlamValuePtr(){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);

    UlamValue ptr;
    return ptr;
  } //makeUlamValuePtr

  void Node::evalNodeProlog(u32 depth)
  {
    //space for local variables on node eval stack; adjusts current fp;
    m_state.m_nodeEvalStack.addFrameSlots(depth);
  }

  void Node::evalNodeEpilog()
  {
    //includes any return value and args; adjusts current fp;
    m_state.m_nodeEvalStack.returnFrame(m_state);
  }

  u32 Node::makeRoomForNodeType(UTI type, STORAGE where)
  {
    if(type == Void)
      return 0;

    //default storage type, where, is the EVALRETURN stack
    u32 slots = m_state.slotsNeeded(type);
    makeRoomForSlots(slots, where); //=1 for scalar or packed array
    return slots;
  }

  u32 Node::makeRoomForSlots(u32 slots, STORAGE where)
  {
    //push copies of temporary UV (e.g. UVPtr)
    UlamValue tmpUV = UlamValue::makeImmediate(Nouti, 0, 1);
    for(u32 j = 0; j < slots; j++)
      {
	if(where == EVALRETURN)
	  m_state.m_nodeEvalStack.pushArg(tmpUV);
	else if (where == STACK)
	  m_state.m_funcCallStack.pushArg(tmpUV);
	else if (where == CNSTSTACK)
	  m_state.m_constantStack.pushArg(tmpUV);
	else
	  m_state.abortUndefinedCallStack();
      }
    return slots;
  }

  //in case of arrays, rtnUV is a ptr; default STORAGE is EVALRETURN
  void Node::assignReturnValueToStack(UlamValue rtnUV, STORAGE where)
  {
    UTI rtnUVtype = rtnUV.getUlamValueTypeIdx(); //==node type

    if(m_state.isPtr(rtnUVtype)) //unpacked array
      {
	rtnUVtype = rtnUV.getPtrTargetType();
      }

    if(rtnUVtype == Void) //check after Ptr target type
      return;

    assert((UlamType::compareForUlamValueAssignment(rtnUVtype, getNodeType(), m_state) == UTIC_SAME) || m_state.isAtom(rtnUVtype) || m_state.isAtom(getNodeType()));

    // save results in the stackframe for caller;
    // copies each element of the 'unpacked' array by value,
    // in reverse order ([0] is last at bottom)
    s32 slots = m_state.slotsNeeded(rtnUVtype);

    //where to put the return value..'return' statement uses STACK
    UlamValue rtnPtr = UlamValue::makePtr(-slots, where, rtnUVtype, m_state.determinePackable(rtnUVtype), m_state);
    m_state.assignValue(rtnPtr, rtnUV);
  } //assignReturnValueToStack

  //in case of arrays, rtnUV is a ptr; default storage is EVALRETURN
  void Node::assignReturnValuePtrToStack(UlamValue rtnUVptr, STORAGE where)
  {
    UTI rtnUVtype = rtnUVptr.getPtrTargetType(); //target ptr type

    if(rtnUVtype == Void)
      return;

    UlamValue rtnPtr = UlamValue::makePtr(-1, where, rtnUVtype, rtnUVptr.isTargetPacked(), m_state);
    m_state.assignValuePtr(rtnPtr, rtnUVptr);
  } //assignReturnValuePtrToStack

  //in case of func call returning a class, lhs of dot; default STORAGE is STACK
  UlamValue Node::assignAnonymousClassReturnValueToStack(UlamValue rtnUV)
  {
    UTI rtnUVtype = rtnUV.getUlamValueTypeIdx(); //==node type

    assert(!m_state.isPtr(rtnUVtype));

    assert(m_state.isAClass(rtnUVtype));

    // save in "uc" slot, below returning answer (if Void??? - 0?)
    s32 slots = m_state.slotsNeeded(getNodeType());

    //where to put the return value..'return' statement uses STACK
    UlamValue rtnPtr = UlamValue::makePtr(-slots - 1, STACK, rtnUVtype, m_state.determinePackable(rtnUVtype), m_state);
    m_state.assignValue(rtnPtr, rtnUV);
    return rtnPtr;
  } //assignAnonymousClassReturnValueToStack

  //called by NodeFuncCall, NodeReturnStmt, NodeBlockFuncDef eval
  bool Node::returnValueOnStackNeededForEval(UTI rtnType)
  {
    bool rtnb = false;
    if(m_state.isAltRefType(rtnType))
      rtnb = true;
    else if((m_state.isAtom(rtnType) && (m_state.isScalar(rtnType))))
      rtnb = true;
    else if(m_state.isAClass(rtnType) && (m_state.isScalar(rtnType)))
      rtnb = true;
    return rtnb;
  } //returnValueOnStackNeededForEval

  TBOOL Node::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_state.abortShouldntGetHere();
    return TBOOL_FALSE;
  }

  void Node::calcMaxIndexOfVirtualFunctionInOrderOfDeclaration(SymbolClass* csym, s32& maxidx)
  {
    return; //only NodeBlockFunctionDefinition does work!
  }

  void Node::printUnresolvedVariableDataMembers()
  {
    m_state.abortShouldntGetHere();
  }

  void Node::printUnresolvedLocalVariables(u32 fid)
  {
    //override
  }

  void Node::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return; //work done by NodeStatements and NodeBlock
  }

  void Node::genCode(File * fp, UVPass& uvpass)
  {
    m_state.indentUlamCode(fp);
    fp->write("virtual void ");
    fp->write(prettyNodeName().c_str());
    fp->write("::genCode(File * fp){} //is needed!!\n"); //sweet.
  }

  void Node::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    std::ostringstream msg;
    msg << "genCodeToStoreInto called on Node type: ";
    msg << m_state.getUlamTypeNameBriefByIndex(getNodeType()).c_str() << ", and failed.";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
    m_state.abortShouldntGetHere();
    return;
  }

  void Node::generateFunctionInDeclarationOrder(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    return; //work done by NodeFuncDecl
  }

  void Node::genCodeReadIntoATmpVar(File * fp, UVPass & uvpass)
  {
    UTI vuti = uvpass.getPassTargetType(); //replaces vuti w target type
    assert(vuti != Void);

    // No split if custom array, that requires an 'aref' function call;
    // handled as genCodeConvertATmpVarIntoCustomArrayAutoRef

    //no actual storage taken up by constant class, we have its initialized default value;
    // may need to read a data member (t41198)
    if(isCurrentObjectsContainingAConstantClass() >= 0)
      return genCodeReadFromAConstantClassIntoATmpVar(fp, uvpass);

    // write out intermediate tmpVar (i.e. terminal) as temp BitVector arg
    // e.g. when func call is rhs of secondary member select
    if(uvpass.getPassNameId() == 0)
      return genCodeConvertATmpVarIntoBitVector(fp, uvpass);

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI cosuti = cos->getUlamTypeIdx();
    //derefcosuti, after read, no longer a ref; effects cstor (t3691,t3682) and tmpvar names.
    cosuti = m_state.getUlamTypeAsDeref(cosuti);
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    TMPSTORAGE cstor = cosut->getTmpStorageTypeForTmpVar();

    if((cosut->getUlamClassType() == UC_TRANSIENT))
      return genCodeReadTransientIntoATmpVar(fp, uvpass);

    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum, cstor).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	if(stgcos->isSelf() && (stgcos == cos))
	  {
	    genCodeReadSelfIntoATmpVar(fp, uvpass);
	  }
	else
	  {
	    genMemberNameOfMethod(fp, uvpass);

	    // the READ method
	    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
	    fp->write("(");

	    // a data member quark, or the element itself should both GetBits from self
	    // now, quark's self is treated as the entire atom/element storage
	    fp->write(");"); GCNL; //PLS generate name of var here!!! (t3543, t3512)
	  }
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    genModelParameterMemberNameOfMethod(fp, epi);

	    //read method based on last cos
	    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());

	    fp->write("(");

	    //storage based on epi-1
	    genModelParameterHiddenArgs(fp, epi); //t3259
	    fp->write(");"); GCNL;
	  }
	else  //local var
	  {
	    if(stgcos->isSelf() && (stgcos == cos))
	      {
		genCodeReadSelfIntoATmpVar(fp, uvpass);
	      }
	    else
	      {
		genLocalMemberNameOfMethod(fp, uvpass);

		//read method based on last cos
		fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());

		// local quark or primitive (i.e. 'notaclass'); has an immediate type:
		// uses local variable name, and immediate read method
		fp->write("();"); GCNL;
	      }
	  }
      }
    uvpass = UVPass::makePass(tmpVarNum, cstor, cosuti, m_state.determinePackable(cosuti), m_state, 0, 0); //POS 0 justified (atom-based).

    s32 namedconstantidx = isCurrentObjectsContainingAConstant();
    if(namedconstantidx >= 0) //right-most (last) named constant (e.g. t41277)
      {
	SymbolWithValue * ncsym = (SymbolWithValue *) m_state.m_currentObjSymbolsForCodeGen[namedconstantidx];
	assert(ncsym);
      }
    // note: Ints not sign extended until used/cast
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadIntoATmpVar

  void Node::genCodeReadFromAConstantClassIntoATmpVar(File * fp, UVPass & uvpass)
  {
    UTI vuti = uvpass.getPassTargetType(); //replaces vuti w target type
    assert(vuti != Void);

    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);
    s32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    TMPSTORAGE cstor = cosut->getTmpStorageTypeForTmpVar();
    u32 coslen = cosut->getSizeofUlamType(); //96 for elements (t41230)
    //what if coslen is zero?
    u32 cospos = 0;
    ULAMCLASSTYPE cosclass = cosut->getUlamClassType();

    //named constant class/entire constant class array; (after loadStorageAndCurrent..)
    s32 namedconstantclassidx = isCurrentObjectsContainingAConstantClass();
    assert(namedconstantclassidx >= 0); //0==right-most (last)

    SymbolWithValue * ncsym = (SymbolWithValue *) m_state.m_currentObjSymbolsForCodeGen[namedconstantclassidx];
    assert(ncsym);
    UTI ncuti = ncsym->getUlamTypeIdx();
    UlamType * ncut = m_state.getUlamTypeByIndex(ncuti);
    ULAMCLASSTYPE nclasstype = ncut->getUlamClassType();

    bool cosIsTheConstantClass = (cos == ncsym);


    if(cosIsTheConstantClass) //cos is the constant class
      {
	assert(namedconstantclassidx == (cosSize - 1));
	//may be scalar or dm array (t41198), arrayitem (t41261), entire array (t41261,3,6)
	assert(m_state.isAClass(cosuti));
	//assert(stgcos==cos); //s.cc t41198
      }
    else
      {
	//that is cos is a data member of our constant class,
	assert(cos->isDataMember()); //namedconstantclassidx < (cosSize - 1))
	assert(namedconstantclassidx < (cosSize - 1));
	assert(((SymbolVariableDataMember *) cos)->isPosOffsetReliable());
	cospos = ((SymbolVariableDataMember *) cos)->getPosOffset();
	//assert(stgcos==ncsym); //s.cc.b
      }

    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp); //non-const (e.g. t41230)
    fp->write(cosut->getTmpStorageTypeAsString().c_str()); //u32, T, BV non-const
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum, cstor).c_str());
    fp->write(" = ");

    if(!cosIsTheConstantClass) //t41209
      {
	fp->write("UlamRef<EC>(");
	fp->write_decimal_unsigned(cospos);
	if(nclasstype == UC_ELEMENT)
	  fp->write("u + T::ATOM_FIRST_STATE_BIT, ");
	else
	  fp->write("u, ");
	fp->write_decimal_unsigned(coslen);
	fp->write("u, ");
      }

    genConstantClassMangledName(fp); //refactored

    //continue to gen the UlamRef to read the cos dm from its constant class
    if(!cosIsTheConstantClass)
      {
	fp->write(", NULL, ");
	if(cosclass == UC_ELEMENT)
	  fp->write("UlamRef<EC>::ATOMIC"); //entire atom (t41232)
	else
	  fp->write(genUlamRefUsageAsString(cosuti).c_str()); //eg. CLASSIC, PRIMITIVE, etc.
	fp->write(", uc).");
	fp->write(cosut->readMethodForCodeGen().c_str()); //t41232 (e.g. element dm in transient)
	fp->write("()");
      }
    else
      fp->write(".read()");

    //gen comment:
    fp->write("; // constant value of ");
    fp->write(m_state.m_pool.getDataAsString(ncsym->getId()).c_str());
    GCNL;

    uvpass = UVPass::makePass(tmpVarNum, cstor, cosuti, m_state.determinePackable(cosuti), m_state, 0, 0); //POS 0 justified (atom-based).

    // note: Ints not sign extended until used/cast
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadFromAConstantClassIntoATmpVar

  void Node::genCodeReadArrayItemFromAConstantClassIntoATmpVar(File * fp, UVPass & luvpass, UVPass & ruvpass)
  {
    //index is a variable, not known at compile time;
    UTI luti = luvpass.getPassTargetType(); //replaces vuti w target type
    assert(luti != Void);
    assert(!m_state.isScalar(luti));

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(stgcos && cos);

    s32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    UTI cosuti = cos->getUlamTypeIdx();
    u32 cospos = 0;

    //now for the array item read at index in ruvpass tmp var
    UTI scalarluti = m_state.getUlamTypeAsScalar(luti);
    UlamType * scalarlut = m_state.getUlamTypeByIndex(scalarluti);
    u32 itemlen = scalarlut->getSizeofUlamType(); //atom-based for elements
    TMPSTORAGE slstor = scalarlut->getTmpStorageTypeForTmpVar();

    s32 namedconstantclassidx = isCurrentObjectsContainingAConstantClass();
    assert(namedconstantclassidx >= 0); //right-most (last) named constant class

    SymbolWithValue * ncsym = (SymbolWithValue *) m_state.m_currentObjSymbolsForCodeGen[namedconstantclassidx];
    assert(ncsym);
    UTI ncuti = ncsym->getUlamTypeIdx();
    UlamType * ncut = m_state.getUlamTypeByIndex(ncuti);
    ULAMCLASSTYPE nclasstype = ncut->getUlamClassType();
    bool cosIsTheConstantClass = (cos == ncsym);

    if(cosIsTheConstantClass) //cos is the constant class
      {
	assert(namedconstantclassidx == (cosSize - 1));
	assert(!m_state.isScalar(cosuti)); //dm array (t41198), or arrayitem (t41261)
	assert(m_state.isAClass(cosuti));
	//assert(stgcos==cos);
      }
    else
      {
	//that is cos is a data member of our constant class,
	assert(cos->isDataMember()); //namedconstantclassidx < (cosSize - 1))
	assert(namedconstantclassidx < (cosSize - 1));
	assert(((SymbolVariableDataMember *) cos)->isPosOffsetReliable());
	cospos = ((SymbolVariableDataMember *) cos)->getPosOffset();
	//assert(stgcos==ncsym); //s.cc.b
      }

    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write(scalarlut->getTmpStorageTypeAsString().c_str()); //u32, u64, T, BV<x>
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(scalarluti, tmpVarNum, slstor).c_str());
    fp->write(" = ");

    if(!cosIsTheConstantClass) //t41198, t41209
      {
	fp->write("UlamRef<EC>(");
	fp->write_decimal_unsigned(cospos);
	fp->write("u + ");
	fp->write(ruvpass.getTmpVarAsString(m_state).c_str()); //index
	fp->write(" * ");
	fp->write_decimal_unsigned(itemlen);
	if((nclasstype == UC_ELEMENT) && !cosIsTheConstantClass) //t41263
	  fp->write("u + T::ATOM_FIRST_STATE_BIT, ");
	else
	  fp->write("u, ");
	fp->write_decimal_unsigned(itemlen);
	fp->write("u, ");
      }

    genConstantClassMangledName(fp); //t41198

    //continue to gen the UlamRef to read the cos dm from its constant class
    if(!cosIsTheConstantClass)
      {
	fp->write(", NULL, ");
	fp->write(genUlamRefUsageAsString(scalarluti).c_str()); //eg. CLASSIC, PRIMITIVE, etc.
 	fp->write(", uc).");
	fp->write(scalarlut->readMethodForCodeGen().c_str());
	fp->write("()");
      }
    else
      {
	fp->write(".readArrayItem(");
	fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
	fp->write(", ");
	fp->write_decimal_unsigned(itemlen);
	fp->write("u);"); //t41266,3,1
      }

    //gen comment:
    fp->write("; // array value in constant ");
    fp->write(m_state.m_pool.getDataAsString(ncsym->getId()).c_str());
    GCNL;

    luvpass = UVPass::makePass(tmpVarNum, slstor, scalarluti, m_state.determinePackable(scalarluti), m_state, 0, 0); //POS 0 justified (atom-based).

   // note: Ints not sign extended until used/cast
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadArrayItemFromAConstantClassIntoATmpVar

  void Node::genCodeReadSelfIntoATmpVar(File * fp, UVPass & uvpass)
  {
    Symbol * stgcos = NULL;
    Symbol * costmp = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, costmp);
    assert(stgcos && costmp);

    UTI stgcosuti = stgcos->getUlamTypeIdx();

    genSelfNameOfMethod(fp); // NOT just ur.Read() (e.g. big array of quarks: 3707, 3708)
    fp->write(readMethodForCodeGen(stgcosuti, uvpass).c_str());
    fp->write("();"); GCNL; //stand-alone 'self'

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadSelfIntoATmpVar

  void Node::genSelfNameOfMethod(File * fp)
  {
    Symbol * self = m_state.getCurrentSelfSymbolForCodeGen();
    if(self->getAutoLocalType() == ALT_AS)
      fp->write(self->getMangledName().c_str()); //t41359
    else
      fp->write(m_state.getHiddenArgName()); //t41120,t3707,8
    fp->write(".");
    return;
  } //genSelfNameOfMethod

  void Node::genCodeReadTransientIntoATmpVar(File * fp, UVPass & uvpass)
  {
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    TMPSTORAGE cstor = cosut->getTmpStorageTypeForTmpVar();
    UTI stgcosuti = stgcos->getUlamTypeIdx();

    s32 tmpVarNum = m_state.getNextTmpVarNumber(); //t41271
    bool isLocal = Node::isCurrentObjectALocalVariableOrArgument();

    if(isLocal && !cos->isDataMember())
      {
	//including TmpVarSymbol (t41272); not data member since needs UlamRef (t41269)
	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum, cstor).c_str());
	fp->write(" = ");
	//read method based on last cos
	genLocalMemberNameOfMethod(fp, uvpass);
	fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
	fp->write("();"); GCNL;
      }
    else
      {
	m_state.indentUlamCode(fp); //not const
	fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum, cstor).c_str());
	fp->write(";"); GCNL;

	m_state.indentUlamCode(fp); //not const
	if(stgcos->isSelf() && (stgcos == cos))
	  genSelfNameOfMethod(fp); //transient as-cond (t41359)
	else if(!isLocal)
	  genMemberNameOfMethod(fp, uvpass); //transient dm (t3967)
	else //local already taken care of
	  genLocalMemberNameOfMethod(fp, uvpass); //transient const ref func arg (t41271)

	//read method based on last cos
	fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
	//	if(cos->isDataMember())
	if(cos->isDataMember() || m_state.isAltRefType(stgcosuti))
	  {
	    fp->write("(0u, "); //pos part of local member name (UlamRef) (e.g. t3739, 3788, 3789, 3795, 3805)
	    fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum, cstor).c_str()); //t41269
	    fp->write(");"); GCNL;
	  }
	else
	  {
	    fp->write("("); //use immediate's read for gather among scattered bases (ulam-5); t41359
	    fp->write(");"); GCNL;
	  }
      }
    uvpass = UVPass::makePass(tmpVarNum, cstor, cosuti, m_state.determinePackable(cosuti), m_state, 0, 0); //POS 0 justified (atom-based).

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadTransientIntoATmpVar

  void Node::genCodeReadAutorefIntoATmpVar(File * fp, UVPass& uvpass)
  {
    //unlike the others, here, uvpass is the autoref (stg);
    //cos tell us where to go within the selected member
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data

    UTI vuti = uvpass.getPassTargetType();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    TMPSTORAGE vstor = vut->getTmpStorageTypeForTmpVar();

    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(tmpStorageTypeForRead(vuti, uvpass).c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, vstor).c_str());
    fp->write(" = ");

    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    fp->write(".read();"); GCNL;

    //update uvpass
    uvpass = UVPass::makePass(tmpVarNum2, vstor, vuti, m_state.determinePackable(vuti), m_state, 0, 0); //POS 0 justified (atom-based).
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadAutorefIntoATmpVar

  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass)
  {
    UTI luti = luvpass.getPassTargetType();
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    UTI ruti = ruvpass.getPassTargetType();

    // No split if custom array, that requires an 'aref' function call;
    // handled as genCodeConvertATmpVarIntoCustomArrayAutoRef t41006

    if(m_state.isAtomRef(luti) && m_state.isAtom(ruti))
      return genCodeWriteToAtomofRefFromATmpVar(fp, luvpass, ruvpass);

    // split off autoref stg/member selected
    if(luvpass.getPassStorage() == TMPAUTOREF)
      return genCodeWriteToAutorefFromATmpVar(fp, luvpass, ruvpass);

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    if(stgcos->isSelf() && (stgcos == cos))
      return genCodeWriteToSelfFromATmpVar(fp, luvpass, ruvpass);

    if((cosut->getUlamClassType() == UC_TRANSIENT))
      return genCodeWriteToTransientFromATmpVar(fp, luvpass, ruvpass);

    bool isElementAncestorCast = (lut->getUlamClassType() == UC_ELEMENT) && m_state.isClassASubclassOf(ruti, luti);

    UVPass typuvpass;
    if(isElementAncestorCast)
      {
	//readTypefield of lhs before the write!
	// pass as rhs uv to restore method afterward; avoids making default atom.
	genCodeReadElementTypeField(fp, typuvpass);
      }

    m_state.indentUlamCode(fp);

    // a data member quark, or the element itself should both GetBits from self;
    // now, quark's self is treated as the entire atom/element storage
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp, luvpass);

	// the WRITE method
	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");
      }
    else
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid

	//local
	genLocalMemberNameOfMethod(fp, luvpass);

	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");
      }

    //VALUE TO BE WRITTEN:
    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too! value is not a terminal
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    TMPSTORAGE rstor = ruvpass.getPassStorage();
    if((rstor == TMPBITVAL) || (rstor == TMPAUTOREF))
      fp->write(".read()");

    fp->write(");"); GCNL;

    // inheritance cast needs the lhs type restored after the generated write
    if(isElementAncestorCast)
      restoreElementTypeForAncestorCasting(fp, typuvpass);

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeWriteFromATmpVar

  void Node::genCodeWriteToSelfFromATmpVar(File * fp, UVPass & luvpass, UVPass & ruvpass)
  {
    UTI luti = luvpass.getPassTargetType();

    if(m_state.isAtomRef(luti))
      {
    	return genCodeWriteToAtomofRefFromATmpVar(fp, luvpass, ruvpass); //t3663, t3907
      }

    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    UTI ruti = ruvpass.getPassTargetType();

    bool isElementAncestorCast = (lut->getUlamClassType() == UC_ELEMENT) && m_state.isClassASubclassOf(ruti, luti);

    UVPass typuvpass;
    if(isElementAncestorCast)
      {
	//readTypefield of lhs before the write!
	// pass as rhs uv to restore method afterward; avoids making default atom.
	genCodeReadElementTypeField(fp, typuvpass);
      }

    m_state.indentUlamCode(fp);
    genSelfNameOfMethod(fp); // NOT just ur.Write(tmpvar) e.g. t41120
    fp->write(writeMethodForCodeGen(luti, luvpass).c_str());
    fp->write("(");
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    //if(ruvpass.getPassStorage() == TMPBITVAL)
    //  fp->write(".read()"); //t41359

    fp->write(");"); GCNL;

    // inheritance cast needs the lhs type restored after the generated write
    if(isElementAncestorCast)
      restoreElementTypeForAncestorCasting(fp, typuvpass);

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeWriteToSelfFromATmpVar

  void Node::genCodeWriteToTransientFromATmpVar(File * fp, UVPass & luvpass, UVPass & ruvpass)
  {
    Symbol * cos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
      }

    UTI cosuti = cos->getUlamTypeIdx();
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();

    m_state.indentUlamCode(fp);

    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp, luvpass);  //t41395
      }
    else
      {
	//local
	genLocalMemberNameOfMethod(fp, luvpass);
      }

    fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str()); //t3739
    if(cosSize > 1)
      {
	assert(cos->isDataMember()); //sanity
	fp->write("(0u, "); //pos part of local member name (UlamRef);t41355
      }
    else
      {
	fp->write("("); //use immediate's write for distrib among scattered bases (ulam-5); t41358
      }
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str()); //tmp var ref
    //if((ruvpass.getPassStorage() == TMPBITVAL) && m_state.isAClass(ruvpass.getPassTargetType()))
    //  fp->write(".read()"); //t41355, t3715 (primitive)
    fp->write(");"); GCNL;

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeWriteToTransientFromATmpVar

  void Node::genCodeWriteToAtomofRefFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass)
  {
    UTI luti = luvpass.getPassTargetType();

    m_state.indentUlamCode(fp);
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	//localvar for atoms
	genLocalMemberNameOfMethodForAtomof(fp, luvpass); //local or self; includes the dot
	fp->write("WriteAtom"); //t3907
      }
    else
      {
	fp->write(luvpass.getTmpVarAsString(m_state).c_str()); //e.g. t3663 atomref
	fp->write(".");
	fp->write(writeMethodForCodeGen(luti, luvpass).c_str());
      }

    fp->write("(");

    //VALUE TO BE WRITTEN:
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    if((ruvpass.getPassStorage() == TMPBITVAL) || (ruvpass.getPassStorage() == TMPAUTOREF))
      fp->write(".read()");
    fp->write("); //write into atomof ref"); GCNL;

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeWriteToAtomofRefFromATmpVar

  void Node::genCodeWriteToAutorefFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass)
  {
    //unlike the others, here, uvpass is the autoref (stg);
    //cos tell us where to go within the selected member
    UTI luti = luvpass.getPassTargetType();
    TMPSTORAGE rstor = ruvpass.getPassStorage();

    m_state.indentUlamCode(fp);
    fp->write(luvpass.getTmpVarAsString(m_state).c_str()); //TMPAUTOREF
    fp->write(".");
    fp->write(writeMethodForCodeGen(luti, luvpass).c_str());
    fp->write("(");
    //VALUE TO BE WRITTEN:
    // with immediate quarks/refs (t3708, t3172), they are read into a tmpreg as other immediates
    // with immediate elements, too! value is not a terminal
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    if((rstor == TMPBITVAL) || (rstor == TMPAUTOREF))
      fp->write(".read()");
    fp->write(");"); GCNL;

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeWriteToAutorefFromATmpVar

  void Node::genCodeReadElementTypeField(File * fp, UVPass & uvpass)
  {
    s32 tmpVarType = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const u32 ");
    fp->write(m_state.getTmpVarAsString(Unsigned, tmpVarType, TMPREGISTER).c_str());;
    fp->write(" = ");

    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp, uvpass);
	// the GetType WRITE method
	fp->write("ReadTypeField(");
      }
    else
      {
	//local
	genLocalMemberNameOfMethod(fp, uvpass);
	fp->write("readTypeField(");
      }
    fp->write("); //save type"); GCNL;
    fp->write("\n");

    //update uvpass
    uvpass = UVPass::makePass(tmpVarType, TMPREGISTER, Unsigned, m_state.determinePackable(Unsigned), m_state, 0, 0); //POS 0 rightjustified (atom-based).
  } //genCodeReadElementTypeField

  void Node::restoreElementTypeForAncestorCasting(File * fp, UVPass & uvpass)
  {
    // inheritance cast needs the lhs type restored after the generated write
    s32 tmpVarType = uvpass.getPassVarNum();

    m_state.indentUlamCode(fp);

    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp, uvpass);

	// the GetType WRITE method
	fp->write("WriteTypeField(");
      }
    else
      {
	//local
	genLocalMemberNameOfMethod(fp, uvpass);
	fp->write("writeTypeField(");
      }

    fp->write(m_state.getTmpVarAsString(Unsigned, tmpVarType, TMPREGISTER).c_str());;
    fp->write("); //restore type"); GCNL;
    fp->write("\n");
  } //restoreElementTypeForAncestorCasting

  // write out intermediate tmpVar as temp BitVector, e.g. func args, question-colon
  //for func args, the type of the funccall node isn't the type of the argument;
  //casts can mask whether the node is the same type as uvpass tmp var.
  void Node::genCodeConvertATmpVarIntoBitVector(File * fp, UVPass & uvpass)
  {
    UTI vuti = uvpass.getPassTargetType(); //terminals handled in NodeTerminal
    assert(m_state.okUTItoContinue(vuti));
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    // write out intermediate tmpVar, or immediate terminal, as temp BitVector arg
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    //non-constant and C++ & to avoid an extra copy

    fp->write(localStorageTypeAsString(vuti).c_str()); //e.g. BitVector<32> exception
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, TMPBITVAL).c_str());
    fp->write("("); // use constructor (not equals)

    u32 pos = uvpass.getPassPos(); //pos adjusted for Element stg in NodeIdent

    if(m_state.isAtom(vuti))
      {
	fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //VALUE

	//for ANY immediate atom arg from a T; needs effective self from Ts type
	if(m_state.isAtomRef(vuti))
	  {
	    assert(uvpass.getPassStorage() == TMPTATOM); // t41139
	    fp->write(", ");
	    fp->write_decimal_unsigned(pos); //Sun Jun 19 08:36:22 2016
	    fp->write("u, uc");
	  }
	//else nothing
      }
    else
      {
	fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //VALUE

	if(vut->getUlamClassType() == UC_NOTACLASS)
	  {
	    //no longer atom-based primitives
	  }
      }
    fp->write(");"); GCNL; //func arg& ?

    uvpass = UVPass::makePass(tmpVarNum2, TMPBITVAL, vuti, m_state.determinePackable(vuti), m_state, pos, 0); //POS left-justified for quarks; right for primitives.
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeConvertATmpVarIntoBitVector

  // write out immediate (LocalStorage) tmp var as an intermediate tmpVar
  void Node::genCodeConvertABitVectorIntoATmpVar(File * fp, UVPass & uvpass)
  {
    UTI vuti = uvpass.getPassTargetType();
    assert(m_state.okUTItoContinue(vuti));

    //e.g. could be primitive ref from aref (t3653, t41333,6)
    UTI derefvuti = m_state.getUlamTypeAsDeref(vuti); //type after read
    UlamType * derefvut = m_state.getUlamTypeByIndex(derefvuti);

    // write out immediate tmp BitValue as an intermediate tmpVar
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    TMPSTORAGE tmp2stor = derefvut->getTmpStorageTypeForTmpVar();

    m_state.indentUlamCode(fp);
    fp->write("const ");

    fp->write(derefvut->getTmpStorageTypeAsString().c_str()); //u32
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(derefvuti, tmpVarNum2, tmp2stor).c_str());
    fp->write(" = ");

    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    fp->write(".read();"); GCNL;

    // use immediate read for entire array; doesn't make sense for custom arrays
    assert(!isCurrentObjectAnArrayItem(vuti, uvpass)); //sanity

    uvpass = UVPass::makePass(tmpVarNum2, tmp2stor, derefvuti, m_state.determinePackable(derefvuti), m_state, 0, 0); //POS 0 rightjustified (atom-based).
    uvpass.setPassPos(0); //entire register
    m_state.clearCurrentObjSymbolsForCodeGen(); //forgotten?
  } //genCodeConvertABitVectorIntoATmpVar

  // write out auto ref tmpVar as temp BitVector; uvpass has variable w posOffset;
  // uses stack for symbol; default is hidden arg.
  void Node::genCodeConvertATmpVarAutoRefIntoAutoRef(File * fp, UVPass & uvpass)
  {
    // write out next chain using auto ref constuctor
    assert(uvpass.getPassStorage() == TMPAUTOREF);

    UTI vuti = uvpass.getPassTargetType(); //offset or another autoref
    assert(m_state.isReference(vuti)); //t3706, t370, t3817, t3906,8,9 (not isAltRefType)

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    // write out auto ref constuctor
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    s32 pos = uvpass.getPassPos();

    m_state.indentUlamCode(fp);
    //can't be const and chainable
    fp->write(cosut->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum2, TMPAUTOREF).c_str());
    fp->write("("); //use constructor (not equals)
    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    fp->write(", ");
    fp->write_decimal_unsigned(pos); //rel offset
    fp->write("u");
    fp->write(");"); GCNL;

    uvpass = UVPass::makePass(tmpVarNum2, TMPAUTOREF, cosuti, m_state.determinePackable(cosuti), m_state, pos, cos->getId()); //POS left-justified by default (t3832).
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeConvertATmpVarAutoRefIntoAutoRef

  // write out array item tmpVar as temp BitVector; luvpass has variable w posOffset,
  // ruvpass has array item index; uses stack for symbol; default is hidden arg.
  void Node::genCodeConvertATmpVarIntoAutoRef(File * fp, UVPass & luvpass, UVPass ruvpass)
  {
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    assert(m_state.okUTItoContinue(stgcosuti));
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    UTI cosuti = cos->getUlamTypeIdx();
    assert(m_state.okUTItoContinue(cosuti));
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    // write out auto ref constuctor
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    s32 pos = luvpass.getPassPos();

    bool askEffSelf = askEffectiveSelfAtRuntimeForRelPosOfBase(); //t41355
    u32 calcpos = calcDataMemberPosOfCurrentObjectClasses(false, Nouti);

    //first array item, with item in uvpass (e.g. t3147)
    assert(ruvpass.getPassStorage() == TMPARRAYIDX);

    assert((m_state.m_currentObjSymbolsForCodeGen.size() == 1) || (stgcosut->getUlamTypeEnum() == Class)); //?

    assert(!cosut->isScalar());

    assert(!cos->isConstant());

    UTI scalarcosuti = m_state.getUlamTypeAsScalar(cosuti);
    UTI scalarrefuti = m_state.getUlamTypeAsRef(scalarcosuti, ALT_ARRAYITEM); //t3147
    UlamType * scalarrefut = m_state.getUlamTypeByIndex(scalarrefuti);
    ULAMCLASSTYPE cosclasstype = cosut->getUlamClassType();

    bool stgisaref = false;

    bool adjstForEle = false;
    s32 edx = isCurrentObjectsContainingAnElement();
    if(edx >= 0)
      {
	UTI euti = m_state.m_currentObjSymbolsForCodeGen[edx]->getUlamTypeIdx(); //needAdjustToStateBits())
	UlamType * eut = m_state.getUlamTypeByIndex(euti);
	adjstForEle = !eut->isScalar() || !eut->isReference(); //neither array/item (t3832)
      }

    m_state.indentUlamCode(fp);
    //can't be const and chainable; needs to be a ref! (e.g. t3668)
    fp->write(scalarrefut->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(scalarrefuti, tmpVarNum2, TMPAUTOREF).c_str());
    fp->write("("); // use constructor (not equals)

    if(cos->isDataMember())
      {
	if(stgcos->isSelf()) //t3149, t3147
	  {
	    fp->write(m_state.getHiddenArgName()); //t3543
	    stgisaref = true; //was implicit self
	  }
	else //array item (dm && tmpvar), or self, super, ref, local stg
	  {
	    fp->write(stgcos->getMangledName().c_str()); //t3702, t3818
	    stgisaref = stgcosut->isReference(); //t3114 (not isAltRefType),incl ARRAYITEM,AS
	  }
      }
    else
      {
	fp->write(cos->getMangledName().c_str()); //local array
	stgisaref = cosut->isReference(); //not isAltRefType, incl ARRAYITEM,AS
      }

    fp->write(", ");
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str()); //item idx variable 0-based

    if((cosclasstype == UC_QUARK))
      {
	fp->write(" * ");
	fp->write_decimal_unsigned(cosut->getBitSize());
	fp->write("u");
      }
    else if((cosclasstype == UC_ELEMENT))
      {
	fp->write(" * BPA"); //t3670, t3814, t3908, t3832
      }
    else if((cosclasstype == UC_TRANSIENT)) //t3810
      {
	fp->write(" * ");
	fp->write_decimal_unsigned(cosut->getBitSize());
	fp->write("u");
      }
    else //not a class
      {
	fp->write(" * ");
	fp->write_decimal_unsigned(cosut->getBitSize());
	fp->write("u");
      }

    if(stgcos->isTmpVarSymbol())
      {
	fp->write(" + ");
	fp->write_decimal_unsigned(pos); //rel offset (t3512, t3543, t3648, t3702, t3776, t3668, t3811, t3946, t3832)
	fp->write("u");
      }
    else if(askEffSelf) //t41355
      {
	assert(cos->isDataMember()); //sanity?
	UTI cosclassuti = cos->getDataMemberClass();
	fp->write(" + ");
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".GetEffectiveSelf()->");
	fp->write(m_state.getGetRelPosMangledFunctionName(stgcosuti)); //nonatom
	fp->write("(");
	fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(cosclassuti)); //efficiency
	fp->write("u ");
	fp->write("/* ");
	fp->write(m_state.getUlamTypeNameBriefByIndex(cosclassuti).c_str());
	fp->write(" */");
	fp->write(") - ");
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".GetPosToEffectiveSelf() + ");
	fp->write_decimal_unsigned(pos); //dm offset (t41358)
	fp->write("u");
      }
    else if(cos->isDataMember())
      {
	fp->write(" + ");
	fp->write_decimal_unsigned(calcpos); //rel offset (t41182, ..)
	fp->write("u");
      }
    else
      fp->write(" + 0"); //in case no other arg in place

    if(adjstForEle)
      fp->write(" + T::ATOM_FIRST_STATE_BIT"); //t3615, t3436, t41177

    if(cosclasstype != UC_NOTACLASS) //is a class
      {
	if(!stgisaref)
	  fp->write(", 0u"); //extra ulamRef arg, pos-to-eff (t3436, t3605, t3832);
	fp->write(", &");
	fp->write(m_state.getTheInstanceMangledNameByIndex(scalarcosuti).c_str()); //t3495, t3512, t3543, t3648, t3702, t3776, t3668, t3811, t3621
      }
    //else skip primitives (t3114, t3605)

    if(!stgisaref)
      fp->write(", uc"); //t3512, t3543, t3648, t3702, t3776, t3668, t3811

    fp->write(");"); GCNL;

    cosuti = scalarrefuti; //for the uvpass

    luvpass = UVPass::makePass(tmpVarNum2, TMPAUTOREF, cosuti, m_state.determinePackable(cosuti), m_state, pos, cos->getId()); //POS left-justified by default (t3832).
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeConvertATmpVarIntoAutoRef

  // write out array item tmpVar as temp BitVector; luvpass has variable w posOffset,
  // ruvpass has array item index; uses stack for symbol; default is hidden arg.
  void Node::genCodeConvertATmpVarIntoConstantAutoRef(File * fp, UVPass & luvpass, UVPass ruvpass)
  {
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    // write out auto ref constuctor
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    s32 pos = luvpass.getPassPos();

    //first array item, with item in luvpass (e.g. t3147)
    assert(ruvpass.getPassStorage() == TMPARRAYIDX);

    assert(!cosut->isScalar());

    UTI scalarcosuti = m_state.getUlamTypeAsScalar(cosuti); //ALT_ARRAYITEM
    UlamType * scalarcosut = m_state.getUlamTypeByIndex(scalarcosuti);
    u32 itemlen = scalarcosut->getSizeofUlamType();

    m_state.indentUlamCode(fp); //non-const to update regnumber
    fp->write(scalarcosut->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(scalarcosuti, tmpVarNum2, TMPAUTOREF).c_str());
    fp->write("("); // use constructor (not equals)
    fp->write(((SymbolConstantValue *) cos)->getCompleteConstantMangledName().c_str()); //constant
    fp->write(", ");
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str()); //item idx variable 0-based
    fp->write(" * ");
    fp->write_decimal_unsigned(itemlen); //no rel offset
    fp->write("u, uc);"); GCNL;

    luvpass = UVPass::makePass(tmpVarNum2, TMPAUTOREF, scalarcosuti, m_state.determinePackable(scalarcosuti), m_state, pos, cos->getId()); //POS left-justified by default

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeConvertATmpVarIntoConstantAutoRef

  void Node::genCodeARefFromARefStorage(File * fp, UVPass stguvpass, UVPass uvpass)
  {
    // write out auto ref constuctor
    s32 tmpVarNum = stguvpass.getPassVarNum();

    UTI vuti = uvpass.getPassTargetType();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(m_state.isAltRefType(vuti));

    UTI stgrefuti = stguvpass.getPassTargetType();
    assert(m_state.isAltRefType(stgrefuti));

    assert(uvpass.getPassStorage() == TMPAUTOREF);

    m_state.indentUlamCode(fp);
    //can't be const and chainable
    fp->write(vut->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    fp->write("("); //use constructor (not equals)
    fp->write(m_state.getTmpVarAsString(stgrefuti, tmpVarNum, TMPAUTOREF).c_str());
    fp->write(", ");
    fp->write_decimal_unsigned(uvpass.getPassPos());
    fp->write("u");
    fp->write(");"); GCNL;

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeARefFromARefStorage

  //used both by NodeVarRef and NodeFunctionCall for reference args;
  //and NodeReturn for ref return value (t3630)
  void Node::genCodeReferenceInitialization(File * fp, UVPass& uvpass, Symbol * vsymptr)
  {
    assert(vsymptr);
    // lhs is third arg, vsymptr (m_varSymbol for NodeVarRef; tmpvarsymbol for NodeFuncCall)
    // caller gets the right-hand side, stgcos; can be same type (e.g. class or primitive),
    // or ancestor quark if a class. atoms no longer a special case (t3484).
    UTI vuti = vsymptr->getUlamTypeIdx(); //i.e. THIS REF TYPE!!
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    //do we need to check both stgcos && cos, or just cos?
    if(!cosut->isScalar() && !vut->isScalar())
      return genCodeArrayRefInit(fp, uvpass, vsymptr); //t3666

    if(!cosut->isScalar() && vut->isScalar())
      return genCodeArrayItemRefInit(fp, uvpass, vsymptr);

    //    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();
    ULAMTYPE vetyp = vut->getUlamTypeEnum();
    u32 pos = 0;

    bool adjstForEle = false;
    s32 edx = isCurrentObjectsContainingAnElement();
    if((edx >= 0) && needAdjustToStateBits(m_state.m_currentObjSymbolsForCodeGen[edx]->getUlamTypeIdx()))
      adjstForEle = true;

    bool askEffSelf = askEffectiveSelfAtRuntimeForRelPosOfBase();

    m_state.indentUlamCode(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(vsymptr->getMangledName().c_str());
    fp->write("("); //pass ref in constructor (ref's not assigned with =)

    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	//local var (no currentObjSymbols, 1 arg since same type) e.g. t3617, t3779
	assert(UlamType::compare(uvpass.getPassTargetType(), vuti, m_state) == UTIC_SAME);
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	if((vetyp == Class) && (uvpass.getPassStorage() == TMPBITVAL))
	  fp->write(", uc"); //not TMPAUTOREF, so need uc (t41071)
	fp->write(");"); GCNL;
      }
    else if(!m_state.isReference(stgcosuti)) //not isAltRefType (t3650);
      {
	pos = uvpass.getPassPos();

	if(stgcos->isConstant())
	  genConstantClassMangledName(fp); //t41238
	else
	  fp->write(stgcos->getMangledName().c_str()); //local var
	fp->write(", ");

	if((vetyp == Class) && m_state.isARefTypeOfUlamType(stgcosuti,vuti)==UTIC_SAME)
	  {
	    //shorthand cnstr: ref fm non-ref of same type. e.g. t41065, t3715
	    fp->write("uc);"); GCNL;
	    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
	    return; //done
	  }

	if(vetyp != UAtom) //t3907
	  {
	    if(adjstForEle)
	      fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3803, t3832, t3632

	    if(!askEffSelf && uvpass.getPassApplyDelta())
	      {
		//specifc base class member type, not including first relpos
		pos = calcDataMemberPosOfCurrentObjectClasses(false, Nouti); //reset pos
	      }

	    //if dm of a base, first relpos known thru effself at runtime..
	    // also cos is dm when cos==stgcos (or cosSize==1).
	    if(askEffSelf) //implicit-self
	      {
		//implicit self, data member may belong to a base, treat as a reference
		//implicit self (e.g. t41333, returned by e4.rboo())
		fp->write(m_state.getHiddenArgName()); //ur first arg
		fp->write(".GetEffectiveSelf()->");
		fp->write(m_state.getGetRelPosMangledFunctionName(stgcosuti)); //nonatom

		assert(cos->isDataMember());
		UTI cosclassuti = cos->getDataMemberClass();
		fp->write("(");
		fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(cosclassuti)); //efficiency
		fp->write("u ");
		fp->write("/* ");
		fp->write(m_state.getUlamTypeNameBriefByIndex(cosclassuti).c_str());
		fp->write(" */");
		fp->write(") - ");
		fp->write(m_state.getHiddenArgName()); //ur
		fp->write(".GetPosToEffectiveSelf() + "); //uvpass has cos dm pos (t41140)
	      }
	    //else (e.g. local var) known at compile time, 0u.
	  }
	fp->write_decimal_unsigned(pos); //rel offset
	fp->write("u");

	if(vetyp == Class)
	  {
	    //UlamRef extra arg for BitStorage (ulam-5)
	    if((cos != stgcos) || isCurrentObjectALocalVariableOrArgument())
	       {
		 fp->write(", ");
		 //in case cos is a sub of to-be-reftype (vuti), effself wouldnt necessarily
		 // be the lh type, nor would the pos-to-eff be 0. instead it would be the
		 // relative position of vuti within cosuti; implicit self (cos==stgcos) has
		 // no need extra UlamRef arg since it is not BitStorage (t41071); but, a
		 // local var does (t3657,8,9, t3829, t41140, t41199).
		 u32 relpos = 0;
		 if(m_state.getABaseClassRelativePositionInAClass(cosuti, vuti, relpos)) //handles refs
		   fp->write_decimal_unsigned(relpos); //UlamRef extra arg
		 else
		   fp->write_decimal_unsigned(0);
		fp->write("u");
	       } //end etra arg

	    //effective self of data member (or stg if not dm)
	    fp->write(", &"); //left just
	    fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str()); //was vuti t3969; t3801,4,5 (usage for quarks gone);
	  } //end for classes

	if(!stgcos->isSelf()) //technically stg is self (ur) if dm, so no uc (t3695)
	  fp->write(", uc"); //t3820, t3613, t3615
	fp->write(");"); GCNL;
      }
    else //rhs is a reference
      {
	//a ref that's not a tmpref has calc'd its pos (t3820, t3908, t3910);
	if(!cos->isTmpVarSymbol())
	  {
	    pos = uvpass.getPassPos();
	    fp->write(stgcos->getMangledName().c_str()); //t3819, t3908
	  }
	else
	  fp->write(cos->getMangledName().c_str()); //t3832, t3653

	ULAMCLASSTYPE cosclasstype = cosut->getUlamClassType(); //t3908
	if(vetyp == Class) //also lhs is not an atom
	  {
	    if(!(cos->isSelf() && cosclasstype == UC_QUARK))
	      {
		fp->write(", ");
		if(adjstForEle)
		  fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3819, t3814?
		fp->write_decimal_unsigned(pos); //rel offset t3819
		fp->write("u, ");
		if(cos->isDataMember())
		  {
		    fp->write("&");
		    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
		  }
		else
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".GetEffectiveSelf()");
		  }
	      }
	    //else quarkref init to self, use copy constructor for same usage (t41153)
	    fp->write(");"); GCNL;
	  }
	else if(vetyp == UAtom)
	  {
	    if(!m_state.isAtom(stgcosuti)) //skip pos for atomref t3709, t3820
	      {
		fp->write(", ");
		fp->write_decimal_unsigned(pos); //rel offset
		fp->write("u"); //t3820
		if((cosclasstype == UC_ELEMENT) || (cosclasstype == UC_QUARK))
		  fp->write(" - T::ATOM_FIRST_STATE_BIT"); //t3684, 3735, 3756, 3789, t3907, t41051
	      }
	    fp->write(");"); GCNL;
	  }
	else //non-class cos, stg is a ref..could be self, runtime rel pos for askEffSelf
	  {
	    if(!askEffSelf && !cos->isTmpVarSymbol())
	      {
		fp->write(", ");
		pos = calcDataMemberPosOfCurrentObjectClasses(false, Nouti); //reset pos
		//specific base class, stg is a ref (first relpos in stgref);
		//(t41333, see rbootwo()); or super (t41333 rboothree());
		fp->write_decimal_unsigned(pos); //rel offset
		fp->write("u);"); GCNL;
	      }
	    else if(askEffSelf)
	      {
		//default data member, stg is a ref (t41333, last set)
		assert(cos->isDataMember()); //sanity
		UTI cosclassuti = cos->getDataMemberClass();
		fp->write(", ");
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetEffectiveSelf()->");
		fp->write(m_state.getGetRelPosMangledFunctionName(stgcosuti)); //nonatom
		fp->write("(");
		fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(cosclassuti)); //efficiency
		fp->write("u ");
		fp->write("/* ");
		fp->write(m_state.getUlamTypeNameBriefByIndex(cosclassuti).c_str());
		fp->write(" */");
		fp->write(") - ");
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetPosToEffectiveSelf() ");
		fp->write("+ ");
		fp->write_decimal_unsigned(pos);
		fp->write("u);"); GCNL;
	      }
	    else
	      {
		//e.g. arrayitem (ie ref), primitive tf null effSelf (t3653)
		fp->write(");"); GCNL; //3630, 3653, t41071,73, t41100
	      }
	  } //end non-class
      } //end stg is ref
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
  } //genCodeReferenceInitialization

  void Node::genCodeArrayRefInit(File * fp, UVPass & uvpass, Symbol * vsymptr)
  {
    //reference always has initial value, unless func param
    assert(vsymptr);
    assert(vsymptr->isAutoLocal());
    assert(vsymptr->getAutoLocalType() != ALT_AS);

    // get the right-hand side, stgcos can be same type
    // (e.g. element, quark, or primitive), or ancestor quark if a class.
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    UTI vuti = vsymptr->getUlamTypeIdx(); //i.e. this ref node
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(!vut->isScalar()); //entire array

    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();
    ULAMTYPE vetyp = vut->getUlamTypeEnum();
    u32 pos = uvpass.getPassPos();

    //no actual storage taken up by constant class, we have its initialized default value;
    // may need to read a data member (t41198)
    if(isCurrentObjectsContainingAConstant() >= 0)
      {
	bool isConstantClass = false;
	UVPass cuvpass = uvpass;
	if(isCurrentObjectsContainingAConstantClass() >= 0)
	  {
	    isConstantClass = true; //t41271,2
	    genCodeReadFromAConstantClassIntoATmpVar(fp, cuvpass); //fixes strings and element types

	    UTI cuvuti = cuvpass.getPassTargetType();
	    UlamType * cuvut = m_state.getUlamTypeByIndex(cuvuti);
	    u32 tmpForImmed = m_state.getNextTmpVarNumber();
	    m_state.indentUlamCode(fp);
	    fp->write(cuvut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	    fp->write(" ");
	    fp->write(m_state.getTmpVarAsString(cuvuti, tmpForImmed, TMPBITVAL).c_str());
	    fp->write("(");
	    fp->write(cuvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(");"); GCNL;
	    cuvpass = UVPass::makePass(tmpForImmed, TMPBITVAL, cuvuti, m_state.determinePackable(cuvuti), m_state, 0, 0); //POS 0 justified (atom-based).
	  }

	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");

	//now for the array ref..
	fp->write(vsymptr->getMangledName().c_str());
	fp->write("("); //pass ref in constructor (ref's not assigned with =)
	if(isConstantClass)
	  {
	    fp->write(cuvpass.getTmpVarAsString(m_state).c_str());
	  }
	else
	  {
	    genConstantArrayMangledName(fp); //primitive constant array
	  }
	fp->write(", 0u, NULL"); //effective self for arrays is NULL
	fp->write(", uc"); //non-reference since constant
	fp->write(");"); GCNL;
	return;
      }

    m_state.indentUlamCode(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(vsymptr->getMangledName().c_str());
    fp->write("("); //pass ref in constructor (ref's not assigned with =)

    if(stgcos->isSelf()) //rhs may be an element/atom in a transient; not a reference (not isAltRefType?) excluding AS and ARRAYITEM too.
      {
	//t3634,t3672,3,t3946,8,t41251
	fp->write(m_state.getHiddenArgName());
	fp->write(", ");
	fp->write_decimal_unsigned(pos); //relative off array base
	fp->write("u");

	if(vetyp == Class)
	  {
	   if(stgcosut->isScalar() && needAdjustToStateBits(stgcosuti))
	     fp->write(" + T::ATOM_FIRST_STATE_BIT");
	   assert(!cosut->isReference()); //not isAltRefType
	   fp->write(", NULL");
	  }
	fp->write(");"); GCNL;
      }
    else
      {
	//local stg, reference, or constant class|array
	//array item is a tmpvar/reference (t3818)
	if(stgcos->isConstant())
	  {
	    m_state.abortShouldntGetHere();
	    if(vetyp == Class)
	      genConstantClassMangledName(fp); //t41271
	    else
	      genConstantArrayMangledName(fp);
	  }
	else
	  //local stg or reference
	  fp->write(stgcos->getMangledName().c_str());

	bool adjstForEle = false;
	if((vetyp != UAtom))
	  {
	    s32 edx = isCurrentObjectsContainingAnElement();
	    if((edx >= 0) && needAdjustToStateBits(m_state.m_currentObjSymbolsForCodeGen[edx]->getUlamTypeIdx()))
	      adjstForEle = true;
	  }

	if(cos->isDataMember())
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(pos); //rel offset array base (t3832)
	    fp->write("u");

	    if(adjstForEle)
	      fp->write(" + T::ATOM_FIRST_STATE_BIT"); //t3635

	    if(vetyp == Class)
	      {
		//element array ref pos NOT adjusted to T::ATOM_FIRST_STATE_BIT (t3814)
		fp->write(", NULL"); //arrays eff self is null
	      }
	    else if(m_state.isAtom(cosuti)) //in case of transient stg
	      {
		fp->write(", NULL"); //t3818
	      }
	  }
	else
	  {
	    // unpack and packed array use same code, except for Atom
	    if((vclasstype == UC_NOTACLASS) && (vetyp != UAtom) )
	      {
		fp->write(", 0u"); //non-atomic primitive (e.g. t3666)
	      }
	    else if((vetyp == UAtom))
	      {
		if(stgcosut->isReference()) //not isAltRefType?
		  {
		    ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClassType();
		    if((stgclasstype == UC_ELEMENT) || (stgclasstype == UC_QUARK))
		      fp->write(", - T::ATOM_FIRST_STATE_BIT"); //needs a test!!
		  }
		else
		  fp->write(", 0u");
	      }
	    else if(vetyp == Class)
	      {
		if(!stgcosut->isReference()) //not isAltRefType
		  {
		    if(adjstForEle)
		      fp->write(" + T::ATOM_FIRST_STATE_BIT"); //t3635

		    //effective self for arrays is NULL
		    fp->write(", 0u, NULL"); //t3670 3668, t3669, t3670
		  }
	      }
	    else
	      m_state.abortUndefinedUlamType();
	  }
	if(!stgcosut->isReference()) //not isAltRefType
	  fp->write(", uc");  //t3635
	fp->write(");"); GCNL;
      } //storage
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
  } //genCodeArrayRefInit

  void Node::genCodeArrayItemRefInit(File * fp, UVPass & uvpass, Symbol * vsymptr)
  {
    //reference always has initial value, unless func param
    assert(vsymptr);
    assert(vsymptr->isAutoLocal());
    assert(vsymptr->getAutoLocalType() != ALT_AS);

    // get the right-hand side, stgcos
    // can be same type (e.g. element, quark, or primitive),
    // or ancestor quark if a class.
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    UTI vuti = vsymptr->getUlamTypeIdx(); //i.e. this ref node
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(vut->isScalar()); //item of array

    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();
    ULAMTYPE vetyp = vut->getUlamTypeEnum();

    bool isLocal = isCurrentObjectALocalVariableOrArgument();
    u32 pos = uvpass.getPassPos();

    m_state.indentUlamCode(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(vsymptr->getMangledName().c_str());
    fp->write("("); //pass ref in constructor (ref's not assigned with =)

    //rhs may be an element or atom in a TRANSIENT!!
    if(!isLocal)
      fp->write(m_state.getHiddenArgName());
    else
      fp->write(stgcos->getMangledName().c_str());

    fp->write(", ");
    if(needAdjustToStateBits(cosuti) && (vetyp != UAtom))
      fp->write("+ T::ATOM_FIRST_STATE_BIT + "); //t3816
    else if(vetyp == UAtom)
      {
	if(stgcosut->isReference()) //not isAltRefType
	  {
	    ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClassType();
	    if((stgclasstype == UC_ELEMENT) || (stgclasstype == UC_QUARK))
	      {
		fp->write("- T::ATOM_FIRST_STATE_BIT + ");
		m_state.abortNeedsATest(); //needs a test!!
	      }
	  }
      }
    fp->write_decimal_unsigned(pos); //rel offset array base
    fp->write("u + (");
    fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //INDEX
    fp->write(" * ");
    if(vclasstype == UC_ELEMENT)
      fp->write("T::BPA)"); //t3811
    else
      {
	fp->write_decimal_unsigned(cosut->getBitSize()); //was stgcosut->
	fp->write("u)");
      }

    if((vclasstype != UC_NOTACLASS) && (vetyp != UAtom))
      {
	if(!stgcosut->isReference()) //not isAltRefType
	  {
	    fp->write(", &"); //left just
	    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	  }
	else if(vclasstype == UC_ELEMENT)
	  {
	    fp->write(", NULL"); //for lookup t3832, t3811
	  }
	else
	  {
	    //CLASSIC ref cannot rely on uc.Lookup
	    //array's effective self is NULL. can't get it this way
	    fp->write("&");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str());
	  }
      }

    if(!stgcosut->isReference() && isLocal) //not isAltRefType
      fp->write(", uc"); //t3818, t3653, t3654

    fp->write(");"); GCNL;
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
  } //genCodeArrayItemRefInit

  void Node::genCodeExtern(File * fp, bool declOnly)
  {
    // no externs
  }

  void Node::genCodeConstantArrayInitialization(File * fp)
  {
    m_state.abortShouldntGetHere(); //fufilled by NodeConstantDef
  }

  void Node::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {
    m_state.abortShouldntGetHere(); //fufilled by NodeConstantDef
  }

  void Node::cloneAndAppendNode(std::vector<Node *> & cloneVec)
  {
    m_state.abortShouldntGetHere(); //fufilled by NodeConstantDef
  }

  void Node::generateTestInstance(File * fp, bool runtest)
  {
    //fufilled by NodeVarDeclDM, NodeBlockClass/Locals;
    //bypassed by NodeTypedef and NodeConstDef, passed thru by NodeStatements
    m_state.abortShouldntGetHere();
  }

  void Node::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    //fufilled by NodeVarDecl, NodeBlock; bypassed by NodeTypedef and NodeConstDef
    m_state.abortShouldntGetHere();
  }

  void Node::addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers)
  {
    //fufilled by NodeVarDecl, NodeConstDef, NodeFuncDecl; bypassed by NodeTypedef
    m_state.abortShouldntGetHere();
  }

  std::string Node::allCAPS(const char * s) //static method
  {
    u32 len = strlen(s);
    std::ostringstream up;

    for(u32 i = 0; i < len; ++i)
      {
      std::string c(1,(s[i] >= 'a' && s[i] <= 'z') ? s[i]-('a'-'A') : s[i]);
      up << c;
    }
    return up.str();
  } //allCAPS


  //PROTECTED METHODS:
  bool Node::makeCastingNode(Node * node, UTI tobeType, Node*& rtnNode, bool isExplicit)
  {
    bool doErrMsg = false;
    UTI nuti = node->getNodeType();

    if(nuti == Nav)
      {
	std::ostringstream msg;
	msg << "Cannot make casting node for type: " ;
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnNode = node;
	return false; //short-circuit
      }

    if(m_state.isStillHazy(nuti))
      {
	std::ostringstream msg;
	msg << "Cannot make casting node for type: " ;
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	rtnNode = node;
	return false; //short-circuit
      }

    ULAMTYPECOMPARERESULTS uticr = UlamType::compareForMakingCastingNode(nuti, tobeType, m_state);
    if(uticr == UTIC_SAME)
      {
	//happens too often with Bool.1.-1 for some reason;
	//and Quark toInt special case -- handle quietly
	rtnNode = node;
	return true;
      }

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    ULAMCLASSTYPE nclasstype = nut->getUlamClassType();
    ULAMCLASSTYPE tclasstype = tobe->getUlamClassType();

    if(nclasstype == UC_NOTACLASS)
      {
	//allows atom ref casts; allows atom-to-quarkref casts
	if(m_state.isAtom(nuti) && !((tclasstype == UC_ELEMENT) || m_state.isAtom(tobeType) || ((tclasstype == UC_QUARK) && tobe->isAltRefType())))
	  doErrMsg = true;

	else if(nuti == Void)
	  doErrMsg = true; //cannot cast a void into anything else (reverse is fine), except cnstr(t41077)
	//else if reference to non-ref of same type?
	else
	  {
	    assert(!isExplicit); //why? (t3250)
	    //because we wouldn't be here making a casting node if the cast were explicit!
	    doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
	  }
      }
    else if(nclasstype == UC_QUARK)
     {
       if(tclasstype == UC_NOTACLASS)
	 {
	   if(node->isFunctionCall())
	     {
	       if(tobe->isAltRefType())
		 {
		   doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
		 }
	       else
		 {
		   // 'node' is a function call that returns a quark;
		   // build a toIntHelper function that takes the return value of 'node'
		   // as its arg and returns toInt
		   doErrMsg = buildCastingFunctionCallNode(node, tobeType, rtnNode);
		 }
	     }
	   else
	     {
	       rtnNode = buildToIntCastingNode(node);

	       //redo check and type labeling; error msg if not same
	       UTI newType = rtnNode->checkAndLabelType();
	       doErrMsg = (UlamType::compareForMakingCastingNode(newType, tobeType, m_state) == UTIC_NOTSAME);
	     }
	 }
       else //if(tclasstype != UC_NOTACLASS)
	 {
	   //handle possible inheritance (u.1.2.2) here
	   if(m_state.isClassASubclassOf(nuti, tobeType))
	     doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
	   else if(m_state.isClassASubclassOf(tobeType, nuti))
	     {
	       //ok to cast a quark ref to an element, if its superclass
	       if(!m_state.isAltRefType(nuti))
		 doErrMsg = true;
	       //else if(!isExplicit) //contradicts UlamTypeClass::safeCast
	       //  doErrMsg = true;
	       else
		 doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
	     }
	   else if(m_state.isARefTypeOfUlamType(nuti, tobeType))
	     {
	       //cast ref to deref type
	       //cast non-ref to its ref type; constants & funccalls
	       // not legal for initialization; ok for assignment.
	       doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
	     }
	   else
	     doErrMsg = true;
	 }

       // infinite loop t3756 (without explicit cast)
       //redo check and type labeling; error msg if not same
       // e.g. t3191 missing function symbol without c&l
       // e.g. t3463 "Cannot CAST Bar as Bits"
       if(doErrMsg && tobe->isPrimitiveType())
	 return makeCastingNode(rtnNode, tobeType, rtnNode, false); //recurse
     }
    else if (nclasstype == UC_ELEMENT)
      {
	if(!( m_state.isAtom(tobeType) || (tobe->getUlamTypeEnum() == Class)))
	  doErrMsg = true;
	else
	  doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
      }
    else if(nclasstype == UC_TRANSIENT)
      doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
    else
      doErrMsg = true;

    if(doErrMsg)
      {
	if(uticr == UTIC_DONTKNOW)
	  {
	    std::ostringstream msg;
	    msg << "Casting 'incomplete' types: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << "(UTI" << nuti << ") to be ";
	    msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    msg << "(UTI" << tobeType << ") in class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Cannot CAST " << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " as " << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    return !doErrMsg; //true is ok
  } //makeCastingNode

  Node * Node::newCastingNode(Node * node, UTI tobeType)
  {
    Node * rtnnode = new NodeCast(node, tobeType, NULL, m_state);
    assert(rtnnode);
    rtnnode->setNodeLocation(getNodeLocation());
    rtnnode->updateLineage(getNodeNo());
    return rtnnode;
  } //newCastingNode

  bool Node::newCastingNodeWithCheck(Node * node, UTI tobeType, Node*& rtnNode)
  {
    rtnNode = newCastingNode(node, tobeType);
    assert(rtnNode);

    //redo check and type labeling; error msg if not same
    UTI newType = rtnNode->checkAndLabelType();
    return (UlamType::compareForMakingCastingNode(newType, tobeType, m_state) == UTIC_NOTSAME);
  } //newCastingNodeWithCheck

  bool Node::buildCastingFunctionCallNode(Node * node, UTI tobeType, Node*& rtnNode)
  {
    assert(m_state.getUlamTypeByIndex(tobeType)->getUlamTypeEnum() == Int); //t3412
    Locator loc = getNodeLocation(); //used throughout

    u32 castId = m_state.m_pool.getIndexForDataString("_toIntHelper");
    Token funcidentTok(TOK_IDENTIFIER, loc, castId);

    NodeBlockContext * cblock = m_state.getContextBlock();
    if(!cblock->isAClassBlock())
      {
	std::ostringstream msg;
	msg << "Attempting to build a casting function call '";
	msg << m_state.m_pool.getDataAsString(castId);
	msg << "' outside a class scope" ;
	MSG(&funcidentTok, msg.str().c_str(), DEBUG);
	return false;
      }

    NodeBlockClass * currClassBlock = (NodeBlockClass *) cblock;

    //make the function def, with node (quark) type as its param, returns Int (always)
    SymbolFunction * fsymptr = new SymbolFunction(funcidentTok, Int /*tobeType*/, m_state);
    //No NodeTypeDescriptor needed for Int
    NodeBlockFunctionDefinition * fblock = new NodeBlockFunctionDefinition(fsymptr, currClassBlock, NULL, m_state);
    assert(fblock);
    fblock->setNodeLocation(loc);

    //symbol will have pointer to body (or just decl for 'use');
    fsymptr->setFunctionNode(fblock); //tfr ownership

    m_state.pushCurrentBlock(fblock); //before parsing the args

    //create "atom" symbol whose index is "hidden" first arg (i.e. a Pass to an Atom);
    //immediately below the return value(s); and belongs to the function definition scope.
    Token selfTok(TOK_KW_SELF, loc, 0);

    m_state.m_parsingVariableSymbolTypeFlag = STF_FUNCPARAMETER; //for symbol install
    SymbolVariableStack * selfsym = new SymbolVariableStack(selfTok, UAtom, m_state);
    selfsym->setIsSelf();
    m_state.addSymbolToCurrentScope(selfsym); //ownership goes to the fblock

    UTI nodeType = node->getNodeType(); //quark, or its memberselect
    UlamType * nut = m_state.getUlamTypeByIndex(nodeType);
    assert(nut->getUlamClassType() == UC_QUARK);
    u32 quid = nut->getUlamTypeNameId();
    Token typeTok(TOK_TYPE_IDENTIFIER, loc, quid);

    TypeArgs typeargs;
    typeargs.init(typeTok);
    typeargs.m_bitsize = nut->getBitSize();
    typeargs.m_arraysize = nut->getArraySize();
    typeargs.m_classInstanceIdx = nodeType;

    //a local arg/parameter
    u32 argid = m_state.m_pool.getIndexForDataString("_arg"); //t3411, t3412
    Token argTok(TOK_IDENTIFIER, loc, argid);
    NodeIdent * argIdentNode = new NodeIdent(argTok, NULL, m_state);
    assert(argIdentNode);
    argIdentNode->setNodeLocation(loc);
    argIdentNode->updateLineage(fblock->getNodeNo()); //t3411, need for toInt,etc

    //symbol not a decl list, nor typedef from another class
    Symbol * argSym = NULL; //a place to put the new symbol;
    argIdentNode->installSymbolVariable(typeargs, argSym);
    assert(argSym);

    //NodeTypedescriptor for 'node' type?
    Node * argNode = new NodeVarDecl((SymbolVariable*) argSym, NULL, m_state);
    assert(argNode);
    argNode->setNodeLocation(loc);
    fsymptr->addParameterSymbol(argSym); //ownership stays w NodeBlockFunctionDefinition's ST
    //uses "decl" nodeno of argIdentNode since it had to be created first (e.g. t3411, t3412, t3514)
    ((SymbolVariableStack *) argSym)->setDeclNodeNo(argIdentNode->getNodeNo()); //fix

    //potentially needed to resolve its node type
    fblock->addParameterNode(argNode); //transfer owner

    //Now, look specifically for a function with the same given name defined
    Symbol * fnSym = NULL;
    if(!currClassBlock->isFuncIdInScope(funcidentTok.m_dataindex, fnSym))
      {
	//first time name used as a function..add symbol function name/typeNav
	fnSym = new SymbolFunctionName(funcidentTok, Nouti, m_state);

	//ownership goes to the class block's ST
	currClassBlock->addFuncIdToScope(fnSym->getId(), fnSym);
      }

    bool isAdded = ((SymbolFunctionName *) fnSym)->overloadFunction(fsymptr); //tfrs owner
    if(!isAdded)
      {
	//this is a duplicate function definition with same parameters and given name!!
	//return types may differ
	std::ostringstream msg;
	msg << "Duplicate defined function '";
	msg << m_state.m_pool.getDataAsString(fsymptr->getId());
	msg << "' with the same parameters" ;
	MSG(&argTok, msg.str().c_str(), DEBUG);
	delete fsymptr; //also deletes the NodeBlockFunctionDefinition
	fsymptr = NULL;
	delete argIdentNode;
	argIdentNode = NULL;
      }
    else
      {
	m_state.m_parsingVariableSymbolTypeFlag = STF_FUNCLOCALVAR; //for symbol install

	/* like a typical quark toInt cast expression */
	Node * mselectNode = buildToIntCastingNode(argIdentNode);
	//the body: single statement return arg.toInt();
	NodeReturnStatement * returnNode =  new NodeReturnStatement(mselectNode, m_state);
	assert(returnNode);
	returnNode->setNodeLocation(loc);
	returnNode->updateLineage(node->getYourParentNo()); //t3411

	fblock->appendNextNode(returnNode);
	fblock->setDefinition();
	fblock->setMaxDepth(0); //no local variables, except params

	//append this little guy to tree to preserve order of function declarations, ulam-5
	NodeFuncDecl * funcdecl = new NodeFuncDecl(fsymptr, m_state); //t3412
	assert(funcdecl);
	funcdecl->setNodeLocation(loc);
	m_state.pushCurrentBlock(currClassBlock); //class scope
	m_state.appendNodeToCurrentBlock(funcdecl);
	m_state.popClassContext();
      }

    //this block's ST is no longer in scope
    m_state.popClassContext(); //= prevBlock;

    m_state.m_parsingVariableSymbolTypeFlag = STF_NEEDSATYPE;

    //func call symbol to return to NodeCast; fsymptr maybe null
    NodeFunctionCall * funccall = new NodeFunctionCall(funcidentTok, fsymptr, m_state);
    assert(funccall);
    funccall->setNodeLocation(loc);
    funccall->addArgument(node);

    //assume helper belongs to self (t3411)
    NodeIdent * selfIdentNode = new NodeIdent(selfTok, NULL, m_state);
    assert(selfIdentNode);
    selfIdentNode->setNodeLocation(loc);

    Node * mfuncselectNode = new NodeMemberSelect(selfIdentNode, funccall, m_state);
    assert(mfuncselectNode);
    mfuncselectNode->setNodeLocation(loc);
    mfuncselectNode->updateLineage(node->getYourParentNo()); //t3190

    rtnNode = mfuncselectNode;

    //redo check and type labeling; error msg if not same
    UTI newType = rtnNode->checkAndLabelType();
    return (UlamType::compareForMakingCastingNode(newType, tobeType, m_state) == UTIC_NOTSAME);
  } //buildCastingFunctionCallNode

  Node * Node::buildToIntCastingNode(Node * node)
  {
    Token identTok;
    u32 castId = m_state.m_pool.getIndexForDataString("toInt");
    identTok.init(TOK_IDENTIFIER, getNodeLocation(), castId);

    //fill in func symbol during type labeling;
    Node * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
    assert(fcallNode);
    fcallNode->setNodeLocation(identTok.m_locator);
    Node * mselectNode = new NodeMemberSelect(node, fcallNode, m_state);
    assert(mselectNode);
    mselectNode->setNodeLocation(identTok.m_locator);
    mselectNode->updateLineage(node->getYourParentNo()); //t3190

    //redo check and type labeling done by caller!!
    return mselectNode; //replace right node with new branch
  } //buildToIntCastingNode

  bool Node::warnOfNarrowingCast(UTI nodeType, UTI tobeType)
  {
    bool rtnB = false;
    s32 nodesize = m_state.getBitSize(nodeType);
    s32 tobesize = m_state.getBitSize(tobeType);

    // warn of narrowing cast
    if(nodesize > tobesize)
      {
	rtnB = true;
	std::ostringstream msg;
	msg << "Narrowing CAST: " << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
	msg << " to " << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	msg << " may cause data loss";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
      }
    return rtnB;
  } //warnOfNarrowingCast

  Node * Node::buildOperatorOverloadFuncCallNodeHelper(Node * selectNode, Node * argNode, const char * nameForTok)
  {
    Token identTok;
    TokenType opTokType = Token::getTokenTypeFromString(nameForTok);
    assert(opTokType != TOK_LAST_ONE);
    Token opTok(opTokType, getNodeLocation(), 0);
    u32 opolId = Token::getOperatorOverloadFullNameId(opTok, &m_state);
    if(opolId == 0)
      {
	std::ostringstream msg;
	msg << "Overload for operator " << getName();
	msg << " is not supported as operand for class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(selectNode->getNodeType()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return NULL;
      }

    identTok.init(TOK_IDENTIFIER, getNodeLocation(), opolId);

    //fill in func symbol during type labeling;
    NodeFunctionCall * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
    assert(fcallNode);
    fcallNode->setNodeLocation(identTok.m_locator);

    if(argNode)
      fcallNode->addArgument(argNode);

    NodeMemberSelect * mselectNode = new NodeMemberSelect(selectNode, fcallNode, m_state);
    assert(mselectNode);
    mselectNode->setNodeLocation(identTok.m_locator);

    //redo check and type labeling done by caller!!
    return mselectNode; //replace right node with new branch
  } //buildOperatorOverloadFuncCallNodeHelper

  void Node::genMemberNameOfMethod(File * fp, const UVPass& uvpass, bool endingdot)
  {
    assert(!isCurrentObjectALocalVariableOrArgument());

    //nothing if current object is self, unless a subclass!!!
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      return;

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(stgcos && cos);

    if(cos->isTmpVarSymbol())
      {
	assert(!((SymbolTmpVar *) cos)->isBaseClassRegNum());
	fp->write(cos->getMangledName().c_str());
	fp->write(".");
	return; //done
      }
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    UTI cosclassuti = cos->getDataMemberClass();
    UlamType * cosclassut = m_state.getUlamTypeByIndex(cosclassuti);
    assert(!cosclassut->isAltRefType());

    // stg as tmpvar, eventhough a data member, has a good 'pos'
    // for example, t3512 with ALT_ARRAYITEM as stg;
    bool askEffSelf = askEffectiveSelfAtRuntimeForRelPosOfBase();
    u32 calcpos = calcDataMemberPosOfCurrentObjectClasses(askEffSelf, Nouti);

    fp->write("UlamRef<EC>("); //wrapper for dm
    if(cosSize > 1 && stgcos->isTmpVarSymbol())
      fp->write(stgcos->getMangledName().c_str()); //first arg t3543, not t3512
    else
      fp->write(m_state.getHiddenArgName()); //ur first arg
    fp->write(", ");

    //explicit 'self' (t41311), dm of a base, neither stg nor currentself (t3541)
    if(stgcos->isSelf() || stgcos->isSuper())
      {
	//reading entire thing, using ELEMENTAL, t.f. adjust (t3880)
	s32 edx = isCurrentObjectsContainingAnElement();
	if((edx >= 0) && needAdjustToStateBits(m_state.m_currentObjSymbolsForCodeGen[edx]->getUlamTypeIdx()))
	  fp->write("T::ATOM_FIRST_STATE_BIT + "); //prettier than adding to pos, t3880, t3147

	if(askEffSelf)
	  {
	    //self, data member belongs to a base, treat as a reference
	    fp->write(m_state.getHiddenArgName()); //ur first arg
	    fp->write(".GetEffectiveSelf()->");
	    fp->write(m_state.getGetRelPosMangledFunctionName(stgcosuti)); //nonatom
	    fp->write("(");
	    if(stgcos->isSuper())
	      {
		fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(stgcosuti)); //efficiency
		fp->write("u ");
		fp->write("/* ");
		fp->write(m_state.getUlamTypeNameBriefByIndex(stgcosuti).c_str());
		fp->write(" */");
	      }
	    else if((cosSize > 2))
	      {
		Symbol * dmcos = m_state.m_currentObjSymbolsForCodeGen[1];
		if(dmcos->isDataMember() && !dmcos->isTmpVarSymbol())
		  {
		    UTI dmclassuti = dmcos->getDataMemberClass();
		    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(dmclassuti)); //efficiency
		    fp->write("u ");
		    fp->write("/* ");
		    fp->write(m_state.getUlamTypeNameBriefByIndex(dmclassuti).c_str());
		    fp->write(" */");
		  }
		else
		  {
		    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(cosclassuti)); //efficiency
		    fp->write("u ");
		    fp->write("/* ");
		    fp->write(m_state.getUlamTypeNameBriefByIndex(cosclassuti).c_str());
		    fp->write(" */");
		  }
	      }
	    else
	      {
		fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(cosclassuti)); //efficiency
		fp->write("u ");
		fp->write("/* ");
		fp->write(m_state.getUlamTypeNameBriefByIndex(cosclassuti).c_str());
		fp->write(" */");
	      }

	    fp->write(") - ");
	    fp->write(m_state.getHiddenArgName()); //ur first arg t41321
	    fp->write(".GetPosToEffectiveSelf() + ");
	  }
      }
    else
      {
	m_state.abortNeedsATest(); //gen member, not local..so???
      }
    //not asking effSelf, or list of dm, so okay to calc relpos
    //calc data member position at compile time, complete object (t3541)
    fp->write_decimal_unsigned(calcpos); //rel offset //t3143, t3543, t3147
    fp->write("u, "); //t3541, t3831

    //the len..
    if(cosut->getUlamClassType() == UC_ELEMENT)
      {
	//atom-based element as dm in transient (t3880)
	if(needAdjustToStateBits(cosuti))
	  {
	    assert(cosut->isScalar()); //sanity
	    fp->write_decimal_unsigned(cosut->getTotalBitSize());
	    fp->write("u, ");
	  }
	else
	  {
	    s32 carraysize = cosut->getArraySize();
	    carraysize = (carraysize == NONARRAYSIZE ? 1 : carraysize);
	    fp->write_decimal_unsigned(carraysize);
	    fp->write(" * T::BPA, ");
	  }
      }
    else
      {
	fp->write_decimal_unsigned(cosut->getTotalBitSize());
	fp->write("u, "); //len
      }

    if(cosut->getUlamTypeEnum() == Class)
      {
	fp->write("&");
	fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str()); //t3175
      }
    else if(m_state.isAtom(cosuti))
      fp->write("NULL"); //t3833
    else
      fp->write("NULL"); //primitive eff self (e.g. t3102)

    fp->write(", ");
    fp->write(genUlamRefUsageAsString(cosuti).c_str());

    fp->write(")"); //close wrapper

    if(endingdot)
      fp->write(".");
    return;
  } //genMemberNameOfMethod

  // "static" data member, a mixture of local variable and dm;
  // NEW approach as extern immediate; [XXXits mangled name includes the
  // element/quark mangled name; thus,XXX] unique MP per ulam class.
  // No longer a template data member; must be initialized (in .cpp),
  // so only primitive types allowed.
  void Node::genModelParameterMemberNameOfMethod(File * fp, s32 epi)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    assert(epi >= 0);

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[epi]; //***

    // the MP (only primitive!, no longer quark or element):
    assert(isHandlingImmediateType());

    UTI stgcosuti = cos->getDataMemberClass();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClassType();

    if(stgclasstype == UC_ELEMENT)
      {
	fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
	fp->write(".");
      }
    else
      m_state.abortShouldntGetHere();

    fp->write(cos->getMangledName().c_str());
    fp->write(".");
    return;
  } //genModelParameterMemberNameOfMethod

  // "static" data member, a mixture of local variable and dm;
  // requires THE_INSTANCE, and local variables are superfluous.
  void Node::genModelParameterHiddenArgs(File * fp, s32 epi)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    assert(epi >= 0);

    if(isHandlingImmediateType())
      {
	fp->write("uc");
	return; // 1 hidden arg
      }

    Symbol * stgcos = NULL;
    if(epi == 0)
      stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      stgcos = m_state.m_currentObjSymbolsForCodeGen[epi - 1]; //***

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();

    Symbol * epcos = m_state.m_currentObjSymbolsForCodeGen[epi]; //***
    UTI epcosuti = epcos->getUlamTypeIdx();
    UlamType * epcosut = m_state.getUlamTypeByIndex(epcosuti);
    ULAMCLASSTYPE epcosclasstype = epcosut->getUlamClassType();

    if(m_state.isClassACustomArray(cosuti))
      fp->write("uc, "); //not for regular READs and WRITEs

    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
    fp->write(".");

    // the MP (only primitive!, no longer quark or element):
    fp->write(epcos->getMangledName().c_str());

    if(epcosclasstype != UC_NOTACLASS)
      {
	if(m_state.isClassACustomArray(cosuti))
	  fp->write(".getRef()");
	else
	  fp->write(".getBits()");
      }
  } //genModelParameterHiddenArgs

  void Node::genCustomArrayMemberNameOfMethod(File * fp)
  {
    assert(!isCurrentObjectALocalVariableOrArgument());

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    //find class for cos' custom array method; and blockNo (may be inherited, unlike cos)
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();

    Symbol * fnsymptr = NULL;
    bool hazyKin = false;
    AssertBool isDefinedFunc = m_state.isFuncIdInAClassScopeOrAncestor(cosuti, m_state.getCustomArrayGetFunctionNameId(),fnsymptr, hazyKin); //searches class of cos, and hierarchy
    assert(isDefinedFunc);
    assert(!hazyKin);

    UTI caclassuti = fnsymptr->getDataMemberClass();
    assert(m_state.isComplete(caclassuti) || m_state.isClassATemplate(caclassuti));

    UlamType * caclassut = m_state.getUlamTypeByIndex(caclassuti);
    assert(!caclassut->isAltRefType());
    fp->write(m_state.getTheInstanceMangledNameByIndex(caclassuti).c_str());
    fp->write(".");
  } //genCustomArrayMemberNameOfMethod

  void Node::genCustomArrayHiddenArgs(File * fp, u32 urtmpnum)
  {
    // first "hidden" arg is the context, then "hidden" self (ur) arg
    fp->write("(");
    fp->write(m_state.getHiddenContextArgName()); //same uc
    fp->write(", ");

    if(urtmpnum > 0)
      fp->write(m_state.getUlamRefTmpVarAsString(urtmpnum).c_str());
    else
      fp->write(m_state.getHiddenArgName());; //same ur;
    return;
  } //genCustomArrayHiddenArgs

  // the second hidden arg, ur/self, may need modification for
  // function calls, including custom array accessors.
  // formerly in NodeFunctionCall; duplicated as genHiddenArg2ForCustomArray
  std::string Node::genHiddenArg2(const UVPass& uvpass, u32& urtmpnumref, UTI vownarg, UTI funcclassarg)
  {
    bool sameur = true;
    u32 tmpvar = m_state.getNextTmpVarNumber();

    std::ostringstream hiddenarg2;
    Symbol *stgcos = NULL;
    Symbol * cos = NULL;

    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClassType();
    UTI cosuti = cos->getUlamTypeIdx();

    bool cosIsVTableClassOrId = cos->isTmpVarSymbol() && (((SymbolTmpVar *) cos)->isBaseClassRef() || ((SymbolTmpVar *) cos)->isBaseClassRegNum());

    if(m_state.isAPrimitiveType(cosuti))
      {
	//primitive cannot own a func, so must be VTtable special case syntax w regid (ulam-5)
	assert(vownarg != Nouti); //error caught by NodeMemberSelect c&l t41388

	cos = stgcos;
	cosuti = stgcosuti;
      }

    bool askEffSelf = askEffectiveSelfAtRuntimeForRelPosOfBase(funcclassarg);
    UTI skipfuncclass = Nouti; //not funcclassarg; consider funcs of a base separately (e.g. t3831)
    //cos precedes .func()
    bool funcinbase = m_state.isClassASubclassOf(cosuti, funcclassarg); //not same
    u32 funcclassrelpos = 0;
    if(funcinbase)
      {
	AssertBool gotrelpos = m_state.getABaseClassRelativePositionInAClass(cosuti, funcclassarg, funcclassrelpos);
	assert(gotrelpos);
      }

    //e.g. element is a dm of a transient, and cos is dm of element,
    //then pos of cos needs the 25u, even if stg is a ref (i.e. to the transient), t3805;
    s32 edx = isCurrentObjectsContainingAnElement();
    bool adjstEle = ((edx >= 0) && needAdjustToStateBits(m_state.m_currentObjSymbolsForCodeGen[edx]->getUlamTypeIdx()));

    // first "hidden" arg is the context, then "hidden" ref self (ur) arg
    if(! Node::isCurrentObjectALocalVariableOrArgument())
      {
	if(m_state.m_currentObjSymbolsForCodeGen.empty() || (stgcos->isSelf() && (stgcos == cos)))
	  {
	    if(!funcinbase)
	      hiddenarg2 << m_state.getHiddenArgName(); //same ur
	  }
	else if(cos->isDataMember())
	  {
	    sameur = false;

	    hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";
	    //update ur to reflect "effective" self for this funccall
	    hiddenarg2 << m_state.getHiddenArgName(); //ur t3102,3,4,6,7,8,9,10,11
	    hiddenarg2 << ", " << calcPosOfCurrentObjectClassesAsString(uvpass, false, askEffSelf, skipfuncclass); //relative off;

	    if(askEffSelf)
	      {
		hiddenarg2 << " + " << m_state.getHiddenArgName(); //ur first arg
		hiddenarg2 << ".GetEffectiveSelf()->";
		hiddenarg2 << m_state.getGetRelPosMangledFunctionName(stgcosuti); //nonatom

		UTI cosclassuti = cos->getDataMemberClass();
		hiddenarg2 << "(";
		hiddenarg2 << m_state.getAClassRegistrationNumber(cosclassuti); //efficiency
		hiddenarg2 << "u ";
		hiddenarg2 << "/* ";
		hiddenarg2 << m_state.getUlamTypeNameBriefByIndex(cosclassuti).c_str();
		hiddenarg2 << " */";

		hiddenarg2 << ") -" << m_state.getHiddenArgName(); //ur first arg t41321
		hiddenarg2 << ".GetPosToEffectiveSelf()";
	      }

	    if(adjstEle)
	      hiddenarg2 << " + T::ATOM_FIRST_STATE_BIT";

	    //func belongs to baseclass of cos dm, add its offset,
	    //change effself to cos (t3831)
	    hiddenarg2 << ", " << getLengthOfMemberClassForHiddenArg(cosuti) << "u, &"; //len, t41120
	    hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(cosuti).c_str(); //cos->isSuper rolls as cosuti
	    hiddenarg2 << ", " << genUlamRefUsageAsString(cosuti).c_str();
	    hiddenarg2 << ");";
	  }
	else //super, and specific bases, i think (t41311, t41322, t41338)
	  {
	    // just for non-virtuals (t41338, t41322, t41386?)
	    if((vownarg == Nouti) && !funcinbase && !askEffSelf)
	      {
		sameur = false;
		hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";
		//donot update ur to reflect "effective" self for this funccall
		if(stgcos->isTmpVarSymbol())
		  hiddenarg2 << stgcos->getMangledName().c_str(); //t3811
		else
		  hiddenarg2 << m_state.getHiddenArgName(); //ur t3102,3,4,6,7,8,9,10,11
		hiddenarg2 << ", " << calcPosOfCurrentObjectClassesAsString(uvpass, false, askEffSelf, skipfuncclass); //rel offset;

		if(adjstEle)
		  hiddenarg2 << " + T::ATOM_FIRST_STATE_BIT";
		hiddenarg2 << ", " << getBaseLengthOfMemberClassForHiddenArg(cosuti); //baselen, t41120
		hiddenarg2 << "u, " << genUlamRefUsageAsString(cosuti).c_str();
		hiddenarg2 << ", true);"; //t41322, t41338 (always true!)
	      } //else sameur (t41353)
	  }
      }
    else
      {
	s32 epi = Node::isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    //model parameters no longer classes, deprecated
	    m_state.abortNotSupported();
	    //hiddenarg2 << genModelParameterHiddenArgs(epi).c_str();
	  }
	else //local var
	  {
	    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

	    //Not isAltRefType; ALT_AS:t3249,t3255; t3751,t3752; ALT_ARRAYITEM tmp (t3832)
	    if(stgcosut->isReference())
	      {
		sameur = false;
		//update ur to reflect "effective" self for this funccall
		hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";
		hiddenarg2 << stgcos->getMangledName().c_str() << ", ";

		if(cos->isDataMember()) //dm of local stgcos, cosSize==1 (e.g.tmp arrayitem)
		  {
		    if(askEffSelf)
		      {
			//e.g. ref to a base, but dm in base's base, a
			//shared base.  can't know effSelf at compile
			//time;t3648,t3751,2,3,4,5,t3811,t3832
			hiddenarg2 << stgcos->getMangledName().c_str();
			hiddenarg2 << ".GetEffectiveSelf()->";
			hiddenarg2 << m_state.getGetRelPosMangledFunctionName(stgcosuti); //nonatom
			UTI cosclassuti = cos->getDataMemberClass();
			hiddenarg2 << "(";
			hiddenarg2 << m_state.getAClassRegistrationNumber(cosclassuti); //efficiency
			hiddenarg2 << "u ";
			hiddenarg2 << "/* ";
			hiddenarg2 << m_state.getUlamTypeNameBriefByIndex(cosclassuti).c_str();
			hiddenarg2 << " */";
			hiddenarg2 << ") -" << stgcos->getMangledName().c_str();
			hiddenarg2 << ".GetPosToEffectiveSelf()";
		      }
		    else
		      hiddenarg2 << calcPosOfCurrentObjectClassesAsString(uvpass, false, askEffSelf, skipfuncclass); //reloffset;

		    if(adjstEle)
		      hiddenarg2 << " + T::ATOM_FIRST_STATE_BIT";

		    hiddenarg2 << ", " << getLengthOfMemberClassForHiddenArg(cosuti) << "u, "; //len, t41120
		    hiddenarg2 << "&"; //effself of dm (t3804 check -10)
		    hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(cosuti).c_str();
		    hiddenarg2 << ", " << genUlamRefUsageAsString(cosuti).c_str();
		    hiddenarg2 << ");";
		  }
		else
		  {
		    //ancestor keeps effective self of sub, more
		    //later..t3637, t3746 uses UlamRef 3-arg copy
		    //constr to keep pos (t3249), EffSelf and UsageType of ref
		    hiddenarg2 << "0, " << getBaseLengthOfMemberClassForHiddenArg(cosuti) << "u);" ; //pos, baselen
		  }
	      }
	    else
	      {
		sameur = false;
		// local variable: new ur to reflect "effective" self
		// and storage for this funccall; in 2 steps to
		// ascertain the correct posToEff in ulam-5 (t3605);
		u32 tmpvarstg = m_state.getNextTmpVarNumber();
		hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvarstg).c_str() << "(";

		if(vownarg == Nouti)
		  {
		    //NOT a virtual func, however, funcclassarg (where
		    //its defined); may be a baseclass
		    if(adjstEle)
		      hiddenarg2 << "T::ATOM_FIRST_STATE_BIT + ";
		    hiddenarg2 << calcPosOfCurrentObjectClassesAsString(uvpass, false, askEffSelf, skipfuncclass); //rel off; incl 25u

		    hiddenarg2 << ", " << getLengthOfMemberClassForHiddenArg(cosuti) << "u, "; //len
		    hiddenarg2 << "0u, "; //UlamRef extra arg for pos-to-Eff (t3605)
		    hiddenarg2 << stgcos->getMangledName().c_str() << ", &"; //storage
		    hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(cosuti).c_str(); //effself
		    hiddenarg2 << ", " << genUlamRefUsageAsString(cosuti).c_str(); //usage
		    hiddenarg2 << ", uc";
		    hiddenarg2 << "); ";

		    tmpvar = tmpvarstg; //not virtual
		  }
		else
		  {
		    //virtual func
		    if(stgclasstype == UC_ELEMENT)
		      hiddenarg2 << "T::ATOM_FIRST_STATE_BIT + ";

		    hiddenarg2 << "0u , " << getLengthOfMemberClassForHiddenArg(stgcosuti) << "u, "; //len
		    hiddenarg2 << "0u, "; //UlamRef extra arg for pos-to-Eff??
		    hiddenarg2 << stgcos->getMangledName().c_str() << ", &"; //storage
		    hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str(); //effself
		    hiddenarg2 << ", " << genUlamRefUsageAsString(stgcosuti).c_str();
		    hiddenarg2 << ", uc";
		    hiddenarg2 << "); "; //line wraps (ugly)

		    //virtual func: 2nd ulamref based on existing stg
		    // ulamref; no implicit 'self' anymore, "dot-chain" size > 1;
		    if((m_state.m_currentObjSymbolsForCodeGen.size() > 1) && !cosIsVTableClassOrId)
		      {
			hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";
			hiddenarg2 << m_state.getUlamRefTmpVarAsString(tmpvarstg).c_str() << ", "; //existing
			hiddenarg2 << calcPosOfCurrentObjectClassesAsString(uvpass, false, askEffSelf, skipfuncclass); //relpos (t3763, t41310)
			if(adjstEle && (stgclasstype != UC_ELEMENT))
			  hiddenarg2 << " + T::ATOM_FIRST_STATE_BIT";

			if(!cos->isDataMember())
			  {
			    m_state.abortShouldntGetHere();
			    //e.g. cos is a BaseType: t41307,8,9,10,16,21,27
			    //virtual func call keeps the eff self of stg,
			    // unless dm of local stg (e.g. t3804)
			    // possible also chk uvpass: applydelta == true?
			    hiddenarg2 << ", " << getBaseLengthOfMemberClassForHiddenArg(cosuti) << "u, &"; //baselen
			    hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str(); //same effself
			    hiddenarg2 << ", " << genUlamRefUsageAsString(cosuti).c_str(); //new usage
			  }
			else
			  {
			    hiddenarg2 << ", " << getLengthOfMemberClassForHiddenArg(cosuti) << "u, &"; //len
			    //data member uses its effective self: t3804,5
			    hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(cosuti).c_str(); //new effself
			    hiddenarg2 << ", " << genUlamRefUsageAsString(cosuti).c_str(); //new usage
			  }
			hiddenarg2 << ");";
		      }
		    else
		      {
			//t3357,8,t3361,t3531,t3600,t3719,20,21,2,t3743,5,7,8,
			//t3804,5,t3880,t41000,1,7,11,12,97,t41298,9
			//t41304,17,19,22,25,27,28,32.
			tmpvar = tmpvarstg; //cos size <= 1
		      }
		  }
	      }
	  }
      }

    u32 tmpvarbasefunc = m_state.getNextTmpVarNumber();
    //non-virtual functions that are in base classes
    if((askEffSelf || funcinbase) && (vownarg == Nouti))
      {
	if(!sameur)
	  hiddenarg2 << " "; //readability btn wrapping UlamRefs
	hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvarbasefunc).c_str() << "(";
	if(sameur)
	  hiddenarg2 << m_state.getHiddenArgName(); //same ur
	else
	  hiddenarg2 << m_state.getUlamRefTmpVarAsString(tmpvar).c_str();

	hiddenarg2 << ", ";

	if(askEffSelf)
	  {
	    // runtime rel pos of func baseclass (t3735)
	    if(sameur)
	      hiddenarg2 << m_state.getHiddenArgName(); //same ur
	    else
	      hiddenarg2 << m_state.getUlamRefTmpVarAsString(tmpvar).c_str();

	    hiddenarg2 << ".GetEffectiveSelf()->";
	    hiddenarg2 << m_state.getGetRelPosMangledFunctionName(stgcosuti); //nonatom
	    hiddenarg2 << "(";
	    hiddenarg2 << m_state.getAClassRegistrationNumber(funcclassarg); //efficiency
	    hiddenarg2 << "u ";
	    hiddenarg2 << "/* ";
	    hiddenarg2 << m_state.getUlamTypeNameBriefByIndex(funcclassarg).c_str();
	    hiddenarg2 << " */";
	    hiddenarg2 << "), ";
	  }
	else
	  hiddenarg2 << funcclassrelpos << "u, ";
	hiddenarg2 << getBaseLengthOfMemberClassForHiddenArg(funcclassarg);
	hiddenarg2 << "u, " << genUlamRefUsageAsString(funcclassarg).c_str();
	hiddenarg2 << ", true);"; //baselen (always true!)
      } //else funcinbase && virtual: t3600,1 3743,5,7, t3986,
    //t41005,6,7,11,12, t41153,61, t41298,9, t41304,7,18,19,20,22,23,25.

    if((askEffSelf || funcinbase) && (vownarg == Nouti))
      urtmpnumref = tmpvarbasefunc;
    else if(!sameur)
      urtmpnumref = tmpvar; //update arg

    return hiddenarg2.str();
  } //genHiddenArg2

  void Node::genLocalMemberNameOfMethod(File * fp, const UVPass& uvpass)
  {
    assert(isCurrentObjectALocalVariableOrArgument());
    // model parameter has its own storage, like a local
    assert(isCurrentObjectsContainingAModelParameter() == -1);
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    if(cosSize > 1)
      return genLocalMemberNameOfMethodByUsTypedef(fp, uvpass);

    assert(cosSize == 1);
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[0];

    if(cos->isConstant()) //t41152 from another class
      fp->write(((SymbolConstantValue *) cos)->getCompleteConstantMangledName().c_str()); //constant
    else
      fp->write(cos->getMangledName().c_str());
    fp->write(".");
    return;
  } //genLocalMemberNameOfMethod

  void Node::genLocalMemberNameOfMethodByUsTypedef(File * fp, const UVPass& uvpass)
  {
    assert(isCurrentObjectALocalVariableOrArgument());
    // model parameter has its own storage, like a local
    assert(isCurrentObjectsContainingAModelParameter() == -1);
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    assert(cosSize > 1); //see caller
    //therefore cos must be a data member!!
    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    if(cos->isTmpVarSymbol())
      {
	assert(!((SymbolTmpVar *) cos)->isBaseClassRegNum());
	fp->write(cos->getMangledName().c_str());
	fp->write(".");
	return; //done
      }

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    // handle inheritance, when data member is in superclass, not current class obj
    // now for both immediate elements and quarks..
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    ULAMCLASSTYPE cosclasstype = cosut->getUlamClassType();

    //cosclass is related to type left of its dot
    UTI cosclassuti = cos->getDataMemberClass();
    assert(cosclassuti != Nouti);
    UlamType * cosclassut = m_state.getUlamTypeByIndex(cosclassuti);
    assert(!cosclassut->isReference());

    assert(cosclassut->isScalar()); //cos array (t3147-48)

    bool askEffSelf = askEffectiveSelfAtRuntimeForRelPosOfBase();
    u32 pos = uvpass.getPassPos();

    //use calculated pos when not asking effself
    if(!askEffSelf)
      {
	u32 locpos = calcDataMemberPosOfCurrentObjectClasses(false, Nouti);
	pos = locpos; //t41318, t3826
      }

    fp->write("UlamRef<EC>("); //wrapper for local storage

    if(stgcosut->isReference()) //not isAltRefType, could be AS or ARRAYITEM (t3249, t41318)
      {
	fp->write(stgcos->getMangledName().c_str()); //ref
	fp->write(", ");

	// when stgref is same as cosclass, use pos; else askEffSelf
	// (not the same and ref).(t41323,t41268);
	if(askEffSelf)
	  {
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".GetEffectiveSelf()->");
	    fp->write(m_state.getGetRelPosMangledFunctionName(stgcosuti)); //nonatom
	    fp->write("(");
	    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(cosclassuti)); //efficiency
	    fp->write("u ");
	    fp->write("/* ");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(cosclassuti).c_str());
	    fp->write(" */");
	    fp->write(") - ");
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".GetPosToEffectiveSelf() ");
	    fp->write("+ ");
	  }
      }
    //else local var position is in uvpass (t41317, t41310, t3826)

    //t41184 already adjusted! but not t3541? MADDNESS!! (formerly adjustUVPassForElements)
    // reading entire thing, using ELEMENTAL, adjust? (t3735), no adjust (t3968)
    // reading member of a transient dm that's an element, adjust (t3968)
    bool adjstEle = false;
    s32 edx = /*(cosclasstype == UC_ELEMENT) ? -1 :*/ isCurrentObjectsContainingAnElement();
    if((edx >= 0) && needAdjustToStateBits(m_state.m_currentObjSymbolsForCodeGen[edx]->getUlamTypeIdx()))
      {
	fp->write("T::ATOM_FIRST_STATE_BIT + "); //prettier than adding 25 to pos (t3735,t3968)
	adjstEle = true;
      }

    fp->write_decimal_unsigned(pos); //rel offset
    fp->write("u, ");

    //len
    if(cosclasstype == UC_ELEMENT)
      {
	//elements too (t3735)..arrays of elements (t3735)?
	  if(adjstEle)
	  {
	    assert(cosut->isScalar()); //sanity
	    fp->write_decimal_unsigned(cosut->getTotalBitSize());
	    fp->write("u, ");
	  }
	else
	  {
	    s32 carraysize = cosut->getArraySize();
	    carraysize = (carraysize == NONARRAYSIZE ? 1 : carraysize);
	    fp->write_decimal_unsigned(carraysize);
	    fp->write(" * T::BPA, ");
	  }
      }
    else
      {
	fp->write_decimal_unsigned(cosut->getTotalBitSize());
	fp->write("u, "); //len
      }

    if(!stgcosut->isReference()) //not isAltRefType, includes ALT_AS (t3249)
      {
	//UlamRef extra arg pos-to-eff
	if(adjstEle)
	  fp->write("0u, ");

	if(cos->isConstant()) //t41152 from another class, constant
	  fp->write(((SymbolConstantValue *) cos)->getCompleteConstantMangledName().c_str());
	else
	  fp->write(stgcos->getMangledName().c_str()); //local storage (not ref)
	fp->write(", ");
      }

    if(cosut->getUlamTypeEnum() == Class)
      {
	//cos is a data member (cosclass could be transient)
	fp->write("&");
	fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str()); //t3735
      }
    else if(m_state.isAtom(cosuti))
      {
	fp->write("NULL"); //eff self t3802
      }
    else
      fp->write("NULL"); //primitive eff self

    fp->write(", ");
    fp->write(genUlamRefUsageAsString(cosuti).c_str());
    if(!stgcosut->isReference())
      fp->write(", uc");

    fp->write(")."); //close wrapper
    return;
  } //genLocalMemberNameOfMethodByUsTypedef

  void Node::genLocalMemberNameOfMethodForAtomof(File * fp, const UVPass& uvpass)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    //assert(isCurrentObjectALocalVariableOrArgument()); //might be self t3907
    assert(isCurrentObjectsContainingAModelParameter() == -1);

    //therefore cos, an element, must be a data member(hence stg is transient)!!
    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    // handle inheritance, when data member is in superclass, not current class obj
    // now for both immediate elements and quarks..
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    assert(cosut->isScalar()); //cos array

    //atom ref doesn't need another wrapper (t3818, t3820)
    if(m_state.isAtomRef(cosuti))
      {
	fp->write(cos->getMangledName().c_str());
	fp->write(".");
	return; //done
      }

    u32 pos = uvpass.getPassPos();

    fp->write("UlamRef<EC>("); //wrapper for local storage

    //possible both stgcos and cos are references!! (t3908)
    // for atomof we care about the cos; c&l trimmed excess element data members
    if(cosut->isReference()) //not AltRefType (t3908)
      {
	fp->write(cos->getMangledName().c_str()); //ref (t3908)
	fp->write(", ");
      }

    ////reading entire thing, using ELEMENTAL, t.f. adjust (t3735)
    // using ATOMIC for atomof (t3908, t3906)
    if((cosut->isReference()) && (cosut->getUlamTypeEnum() == Class)) //vs. atom
      fp->write("- T::ATOM_FIRST_STATE_BIT + 0u, "); //t3909
    else
      {
	fp->write_decimal_unsigned(pos); //rel offset
	fp->write("u, ");
      }

    fp->write("T::BPA, "); //len

    if(!cosut->isReference())
      {
	fp->write(cos->getMangledName().c_str()); //local storage (no refs)
	fp->write(", ");
      }

    fp->write("NULL"); //eff-self for atom lookup

    fp->write(", ");
    fp->write(genUlamRefUsageAsString(UAtom).c_str()); //t3908 For Atomof

    if(!cosut->isReference())
      fp->write(", uc");

    fp->write(")."); //close wrapper
    return;
  } //genLocalMemberNameOfMethodForAtomof

  void Node::genCustomArrayLocalMemberNameOfMethod(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    // handle inheritance, when data member is in superclass, not current class obj
    // now for both immediate elements and quarks..
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    if(cosut->isReference()) //not isAltRefType?
      cosuti = m_state.getUlamTypeAsDeref(cosuti); //search for funcid

    Symbol * fnsymptr = NULL;
    bool hazyKin = false;
    AssertBool isDefinedFunc = m_state.isFuncIdInAClassScopeOrAncestor(cosuti, m_state.getCustomArrayGetFunctionNameId(),fnsymptr, hazyKin); //searches class of cos, and hierarchy
    assert(isDefinedFunc);
    assert(!hazyKin);

    UTI futi = fnsymptr->getDataMemberClass();
    fp->write(m_state.getTheInstanceMangledNameByIndex(futi).c_str());
    fp->write(".");
  } //genCustomArrayLocalMemberNameOfMethod

  void Node::genConstantClassMangledName(File * fp, const char * const prefix)
  {
    s32 namedconstantclassidx = isCurrentObjectsContainingAConstantClass();
    assert(namedconstantclassidx >= 0); //right-most (last) named constant class

    SymbolWithValue * ncsym = (SymbolWithValue *) m_state.m_currentObjSymbolsForCodeGen[namedconstantclassidx];
    assert(ncsym);
    UTI ncuti = ncsym->getUlamTypeIdx();
    assert(m_state.isAClass(ncuti)); //sanity
    assert(ncsym->isConstant()); //sanity
    assert(ncsym->isReady());

    if(ncsym->isDataMember()) //named constant dm
      {
	UTI ncutidm = ncsym->getDataMemberClass();
	if(UlamType::compare(ncutidm, m_state.getCompileThisIdx(), m_state) != UTIC_SAME)
	  {
	    UlamType * ncutdm = m_state.getUlamTypeByIndex(ncutidm);
	    fp->write(ncutdm->getUlamTypeMangledName().c_str());
	    fp->write("<EC>::");
	  }
	//else just the static constant immediate name. (t41271)
      }
    else if(ncsym->isLocalsFilescopeDef())
      {
	u32 mangledclassid = m_state.getMangledClassNameIdForUlamLocalsFilescope(ncsym->getLocalsFilescopeType());
	fp->write(m_state.m_pool.getDataAsString(mangledclassid).c_str());
	fp->write("<EC>::");
      }
    else if(ncsym->isClassArgument())
      {
	UTI whosclassarg = ncsym->getClassArgumentOfClassInstance();
	if(m_state.isClassAStub(whosclassarg))
	  {
	    Symbol * namedconstantstg = NULL;
	    if(namedconstantclassidx > 0)
	      namedconstantstg = m_state.m_currentObjSymbolsForCodeGen[namedconstantclassidx - 1]; //left
	    else
	      namedconstantstg = ncsym;

	    assert(namedconstantstg);
	    UTI stgType = namedconstantstg->getUlamTypeIdx();
	    u32 nameid = m_state.getUlamTypeByIndex(whosclassarg)->getUlamTypeNameId();
	    UTI stgsuper = Nouti;
	    u32 matchingids = m_state.findClassAncestorWithMatchingNameid(stgType, nameid, stgsuper);
	    if(matchingids == 1)
	      fp->write(m_state.getUlamTypeByIndex(stgsuper)->getUlamTypeMangledName().c_str());
	    else if(matchingids > 1) //too many?
	      m_state.abortShouldntGetHere();
	    else //no match
	      m_state.abortShouldntGetHere();
	  }
	else
	  fp->write(m_state.getUlamTypeByIndex(whosclassarg)->getUlamTypeMangledName().c_str());
	fp->write("<EC>::");
      }
    else if(namedconstantclassidx == 0)
      {
	//local: stg is constant class
      }
    else
      {
	//use index to left of the namedconstantclassidx; we know it is not 0;
	// anything else to the left is irrelevent
	assert(namedconstantclassidx > 0);
	Symbol * leftofnamedconstant = m_state.m_currentObjSymbolsForCodeGen[namedconstantclassidx - 1];
	assert(leftofnamedconstant);
	UlamType * ncutl = m_state.getUlamTypeByIndex(leftofnamedconstant->getUlamTypeIdx());
	fp->write(ncutl->getUlamTypeMangledName().c_str());
	fp->write("<EC>::");
      }

    if(prefix)
      fp->write(prefix);
    fp->write(ncsym->getMangledName().c_str()); //constant name
  } //genConstantClassMangledName

  // (much like genConstantClassMangledName above)
  void Node::genConstantArrayMangledName(File * fp, const char * const prefix)
  {
    s32 namedconstantidx = isCurrentObjectsContainingAConstant();
    assert(namedconstantidx >= 0); //right-most (last) named constant

    SymbolWithValue * ncsym = (SymbolWithValue *) m_state.m_currentObjSymbolsForCodeGen[namedconstantidx];
    assert(ncsym);
    UTI ncuti = ncsym->getUlamTypeIdx();
    assert(!m_state.isAClass(ncuti)); //sanity
    assert(ncsym->isConstant()); //sanity
    assert(ncsym->isReady());

    if(ncsym->isDataMember()) //named constant dm
      {
	UTI ncutidm = ncsym->getDataMemberClass();
	if(UlamType::compare(ncutidm, m_state.getCompileThisIdx(), m_state) != UTIC_SAME)
	  {
	    UlamType * ncutdm = m_state.getUlamTypeByIndex(ncutidm);
	    fp->write(ncutdm->getUlamTypeMangledName().c_str());
	    fp->write("<EC>::");
	  }
	//else just the static constant immediate name. (t41271)
      }
    else if(ncsym->isLocalsFilescopeDef())
      {
	u32 mangledclassid = m_state.getMangledClassNameIdForUlamLocalsFilescope(ncsym->getLocalsFilescopeType());
	fp->write(m_state.m_pool.getDataAsString(mangledclassid).c_str());
	fp->write("<EC>::");
      }
    else if(ncsym->isClassArgument())
      {
	UTI whosclassarg = ncsym->getClassArgumentOfClassInstance();
	if(m_state.isClassAStub(whosclassarg))
	  {
	    Symbol * namedconstantstg = NULL;
	    if(namedconstantidx > 0)
	      namedconstantstg = m_state.m_currentObjSymbolsForCodeGen[namedconstantidx - 1]; //left
	    else
	      namedconstantstg = ncsym;

	    assert(namedconstantstg);
	    UTI stgType = namedconstantstg->getUlamTypeIdx();
	    u32 nameid = m_state.getUlamTypeByIndex(whosclassarg)->getUlamTypeNameId();
	    UTI stgsuper = Nouti;
	    u32 matchingids = m_state.findClassAncestorWithMatchingNameid(stgType, nameid, stgsuper);
	    if(matchingids == 1)
	      fp->write(m_state.getUlamTypeByIndex(stgsuper)->getUlamTypeMangledName().c_str());
	    else if(matchingids > 1) //too many
	      m_state.abortShouldntGetHere();
	    else //no match
	      m_state.abortShouldntGetHere();
	  }
	else
	  fp->write(m_state.getUlamTypeByIndex(whosclassarg)->getUlamTypeMangledName().c_str());
	fp->write("<EC>::");
      }
    else if(namedconstantidx == 0)
      {
	//local: stg is constant class
      }
    else
      {
	//use index to left of the namedconstantidx; we know it is not 0;
	// anything else to the left is irrelevent
	assert(namedconstantidx > 0);
	Symbol * leftofnamedconstant = m_state.m_currentObjSymbolsForCodeGen[namedconstantidx - 1];
	assert(leftofnamedconstant);
	UlamType * ncutl = m_state.getUlamTypeByIndex(leftofnamedconstant->getUlamTypeIdx());
	fp->write(ncutl->getUlamTypeMangledName().c_str());
	fp->write("<EC>::");
      }
    if(prefix)
      fp->write(prefix);
    fp->write(ncsym->getMangledName().c_str()); //constant array name
  } //genConstantArrayMangledName

  const std::string Node::localStorageTypeAsString(UTI nuti)
  {
    return m_state.getUlamTypeByIndex(nuti)->getLocalStorageTypeAsString();
  }

  const std::string Node::tmpStorageTypeForRead(UTI nuti, const UVPass& uvpass)
  {
    return m_state.getUlamTypeByIndex(nuti)->getTmpStorageTypeAsString();
  }

  const std::string Node::tmpStorageTypeForReadArrayItem(UTI nuti, const UVPass& uvpass)
  {
    return m_state.getUlamTypeByIndex(nuti)->getArrayItemTmpStorageTypeAsString();
  } //tmpStorageTypeForReadArrayItem

  const std::string Node::readMethodForCodeGen(UTI nuti, const UVPass& uvpass)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    std::string method;
    Symbol * cos = NULL;
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      cos = m_state.m_currentObjSymbolsForCodeGen.back();

    if(cos && cos->isTmpVarSymbol())
      method = "read";
    if(cos && cos->isSelf() && (cos->getAutoLocalType() == ALT_AS))
      method = "read"; //may be too specific? t41359
    else if(isCurrentObjectsContainingAModelParameter() >= 0)
      method = "read"; //an exception
    else if(!isCurrentObjectALocalVariableOrArgument())
      method = nut->readMethodForCodeGen(); //UlamRef
    else if(m_state.m_currentObjSymbolsForCodeGen.size() > 1)
      method = nut->readMethodForCodeGen(); //UlamRef
    else
      method = "read"; //local variable name, not a transient
    return method;
  } //readMethodForCodeGen

  const std::string Node::readArrayItemMethodForCodeGen(UTI nuti, const UVPass& uvpass)
  {
    std::string method;
    bool isCArray = isCurrentObjectACustomArrayItem(nuti, uvpass);
    assert(isCurrentObjectAnArrayItem(nuti, uvpass) || isCArray);
    if(isCArray)
      method = m_state.getCustomArrayGetMangledFunctionName();
    else
      {
	//packed and unpacked arrays both
	UTI scalarnuti = m_state.getUlamTypeAsScalar(nuti);
	method = m_state.getUlamTypeByIndex(scalarnuti)->readMethodForCodeGen(); //for UlamRef
      }
    return method;
  } //readArrayItemMethodForCodeGen

  const std::string Node::writeMethodForCodeGen(UTI nuti, const UVPass& uvpass)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    std::string method;

    Symbol * cos = NULL;
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      cos = m_state.m_currentObjSymbolsForCodeGen.back();

    if(cos && cos->isTmpVarSymbol())
      method = "write";
    if(cos && cos->isSelf() && (cos->getAutoLocalType() == ALT_AS))
      method = "write"; //may be too specific? t41359
    else if(!isCurrentObjectALocalVariableOrArgument())
      method =  nut->writeMethodForCodeGen(); //UlamRef
    else if(m_state.m_currentObjSymbolsForCodeGen.size() > 1)
      method = nut->writeMethodForCodeGen(); //UlamRef
    //else if(nut->getUlamClassType() == UC_TRANSIENT) //t41358
    //  method = nut->writeMethodForCodeGen(); //always WriteBV (t3739)
    else
      method = "write"; //local variable name, not a transient
    return method;
  } //writeMethodForCodeGen

  const std::string Node::writeArrayItemMethodForCodeGen(UTI nuti, const UVPass& uvpass)
  {
    std::string method;
    bool isCArray = isCurrentObjectACustomArrayItem(nuti, uvpass);
    assert(isCurrentObjectAnArrayItem(nuti, uvpass) || isCArray);

    if(isCArray)
      method = m_state.getCustomArrayGetMangledFunctionName(); //returns a ref
    else
      {
	//packed and unpacked arrays both
	UTI scalarnuti = m_state.getUlamTypeAsScalar(nuti);
	method = m_state.getUlamTypeByIndex(scalarnuti)->writeMethodForCodeGen(); //for UlamRef
      }
    return method;
  } //writeArrayItemMethodForCodeGen

  s32 Node::loadStorageAndCurrentObjectSymbols(Symbol *& stgcosref, Symbol *&cosref)
  {
    s32 rtnstgidx = -1; //-1 means empty, o.w. index of stgcos (usually 0)
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	//"self" belongs to func def block that we're currently gencoding
	//function calls are not on the "stack"; t41065, foofunc();
	//t3617 'self' already added to parse tree if applicable
	u32 selfid = m_state.m_pool.getIndexForDataString("self");
	Symbol * selfsym = NULL;
	bool hazykin = false; //unused
	AssertBool gotSelf = m_state.alreadyDefinedSymbolHere(selfid, selfsym, hazykin);
	assert(gotSelf);
	stgcosref = cosref = selfsym; //t41065, foofunc();
	return -1;
      }

    stgcosref = m_state.m_currentObjSymbolsForCodeGen[0];
    cosref = m_state.m_currentObjSymbolsForCodeGen.back();

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(u32 i = 0; i < cosSize; i++)
      {
	stgcosref = m_state.m_currentObjSymbolsForCodeGen[i];
	rtnstgidx = i;
	//two tmprefsymbols, already UlamRef gencoded, in a row; use the second one.
	if(stgcosref->isTmpVarSymbol())
	  {
	    if((cosSize - i <= 1) || !m_state.m_currentObjSymbolsForCodeGen[i + 1]->isTmpVarSymbol())
	      break;
	  }
	else
	  break;
      }
    return rtnstgidx;
  } //loadStorageAndCurrentObjectSymbols

  bool Node::isCurrentObjectALocalVariableOrArgument()
  {
    //include model parameters as LocalVariableOrArgument, since more alike;
    //an element's "self" as obj[0] is like it isn't there for purposes of this discovery.
    //quark's self is an atom or immediate, and should be treated like a local arg.
    // notes: self is not a data member. references cannot be data members.
    //        func def's are dm, but func calls are not.
    //        arrayitem is a tmpvar, also altref type, and sometimes dm.
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      return false; //must be self, t.f. not local; or uvpass is tmpvar?

    if(m_state.m_currentObjSymbolsForCodeGen[0]->isDataMember())
      {
	if(isCurrentObjectsContainingAConstant() > -1)
	  return true; //treat named constant/class array like local storage
	if(isCurrentObjectsContainingAModelParameter() > -1)
	  return true; //treat model parameter like local storage
	if(isCurrentObjectsContainingATmpVarSymbol() > -1)
	  return true; //treat tmpvar symbols like local storage (t3914?)
	return false; //implicit self, non-local
      }

    if(m_state.m_currentObjSymbolsForCodeGen[0]->isSelf()) //explcit self
      {
	UTI stgcosuti = m_state.m_currentObjSymbolsForCodeGen[0]->getUlamTypeIdx();
	assert(!m_state.isAtom(stgcosuti)); //parse error (t3284)
	if(isCurrentObjectsContainingAConstant() > -1)
	  return true; //treat named constant/class array like local storage
	if(isCurrentObjectsContainingAModelParameter() > -1)
	  return true; //treat model parameter like local storage (t3259)
	if(isCurrentObjectsContainingATmpVarSymbol() > -1)
	  return true; //treat tmpvar symbols like local storage (t3914?)
	return false;
      }

    if(m_state.m_currentObjSymbolsForCodeGen[0]->isSuper())
      return false; //t41311

    return true; //including references
  } //isCurrentObjectALocalVariableOrArgument

  // returns the index to the last object that's an MP; o.w. -1 none found;
  // preceding object is the "owner", others before it are irrelevant;
  s32 Node::isCurrentObjectsContainingAModelParameter()
  {
    s32 indexOfLastMP = -1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = cosSize - 1; i >= 0; i--)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(sym->isModelParameter())
	  {
	    indexOfLastMP = i;
	    break;
	  }
      }
    return indexOfLastMP;
  } //isCurrentObjectsContainingAModelParameter

  // returns the index to the last object that's a named constant; o.w. -1 none found;
  // preceding object is the "owner", others before it are irrelevant;
  s32 Node::isCurrentObjectsContainingAConstant()
  {
    s32 indexOfLast = -1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = cosSize - 1; i >= 0; i--)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(sym->isConstant())
	  {
	    indexOfLast = i;
	    break;
	  }
      }
    return indexOfLast;
  } //isCurrentObjectsContainingAConstant

  // returns the index to the last object that's a named constant class; o.w. -1 none found;
  // preceding object is the "owner", others before it are irrelevant;
  s32 Node::isCurrentObjectsContainingAConstantClass()
  {
    s32 indexOfLast = -1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = cosSize - 1; i >= 0; i--)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	UTI suti = sym->getUlamTypeIdx(); //possibly array of classes
	if(sym->isConstant() && m_state.isAClass(suti))
	  {
	    indexOfLast = i;
	    break;
	  }
      }
    return indexOfLast;
  } //isCurrentObjectsContainingAConstantClass

  // returns the index to the first object that's an element or ele ref; o.w. -1 none found;
  // e.g. element data member of transient (t3905)
  s32 Node::isCurrentObjectsContainingAnElement()
  {
    s32 indexOfFirst = -1;
    s32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = 0; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	UTI suti = sym->getUlamTypeIdx();
	if(m_state.getUlamTypeByIndex(suti)->getUlamClassType() == UC_ELEMENT)
	  {
	    indexOfFirst = i;
	    break;
	  }
      }
    return indexOfFirst;
  } //isCurrentObjectsContainingAnElement

  // returns the index to the last object that's a Base Type selector; o.w. -1 none found;
  // preceding object is the "owner", others before it are irrelevant;
  s32 Node::isCurrentObjectsContainingABaseTypeTmpSymbol()
  {
    s32 indexOfLastBT = -1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = cosSize - 1; i >= 0; i--)
      {
	Symbol * bsym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(bsym->isTmpVarSymbol() && ((SymbolTmpVar *) bsym)->isBaseClassRef())
	  {
	    indexOfLastBT = i;
	    break;
	  }
      }
    return indexOfLastBT;
  } //isCurrentObjectsContainingABaseTypeTmpSymbol

  // returns the index to the last object that's a Sub/Base ClassId (RegNum)
  //  selector; o.w. -1 none found; preceding object is its baseclass
  //  type, stg at 0 is the "owner"; used for virtual function calls.
  s32 Node::isCurrentObjectsContainingABaseRegNumTmpSymbol()
  {
    s32 indexOfLastBT = -1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = cosSize - 1; i >= 0; i--)
      {
	Symbol * bsym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(bsym->isTmpVarSymbol() && ((SymbolTmpVar *) bsym)->isBaseClassRegNum())
	  {
	    indexOfLastBT = i;
	    break;
	  }
      }
    return indexOfLastBT;
  } //isCurrentObjectsContainingABaseRegNumTmpSymbol

  // returns the index to the last object that's a tmp var symbol (not base type); o.w. -1 none found;
  // preceding object is the "owner", others before it are irrelevant;
  s32 Node::isCurrentObjectsContainingATmpVarSymbol()
  {
    s32 indexOfLastTmp = -1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = cosSize - 1; i >= 0; i--)
      {
	Symbol * bsym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(bsym->isTmpVarSymbol() && !(((SymbolTmpVar *) bsym)->isBaseClassRef() || ((SymbolTmpVar *) bsym)->isBaseClassRegNum()))
	  {
	    indexOfLastTmp = i;
	    break;
	  }
      }
    return indexOfLastTmp;
  } //isCurrentObjectsContainingATmpVarSymbol

  // used by genHiddenArg2 for function calls; uvpass may contain the index
  // of an array item, or a specific base type that is in a shared position,
  //o.w. the current arg's tmp var (unneeded here).
  std::string Node::calcPosOfCurrentObjectClassesAsString(const UVPass& uvpass, bool adjstEle, bool askeffselfarg, UTI funcclassarg)
  {
    s32 pos = uvpass.getPassPos();
    assert(pos >= 0); //UlamRef arg is unsigned

    std::ostringstream posStr;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	posStr << pos << "u";
	return posStr.str();
      }

    bool outputpos = true;
    Symbol *stgcos = NULL;
    Symbol * cos = NULL;

    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    ULAMCLASSTYPE classtype = cosut->getUlamClassType();

    if(uvpass.getPassApplyDelta())
      {
	assert(!(cos->isTmpVarSymbol() && ((SymbolTmpVar *) cos)->isBaseClassRegNum()));

	//handling specific baseclass type here, shared don't add up correctly (t41316)
	if(cos->isTmpVarSymbol() && ((SymbolTmpVar *) cos)->isBaseClassRef())
	  {
	    UTI myself = stgcosuti;
	    if(stgcos->isSuper()) //t41314 ?
	      {
		myself = m_state.getCurrentSelfSymbolForCodeGen()->getUlamTypeIdx();
	      }
	    u32 tmpsharedrelpos = 0;
	    bool shared = m_state.getASharedBaseClassRelativePositionInAClass(myself, cosuti, tmpsharedrelpos);
	    if(shared)
	      {
		s32 posfix = tmpsharedrelpos; // - pos; //already includes atomfirststatebit
		posStr << posfix;
		outputpos = false;
	      }
	  }
      }
    else if(cos->isDataMember()) //also uvpass target type is stgcosuti(t3821)
      {
	u32 newpos = calcDataMemberPosOfCurrentObjectClasses(askeffselfarg, funcclassarg);
	posStr << newpos << "u ";
	outputpos = false;
      }
    else if(stgcos->isSuper())
      {
	u32 newpos = calcDataMemberPosOfCurrentObjectClasses(askeffselfarg, funcclassarg); //t41338
	posStr << newpos << "u ";
	outputpos = false;
      }
    //else

    //save for after calcDataMemberPos includes it too (t3605)
    if(outputpos) //3604 (pos in uvpass, don't double!)
      {
	posStr << pos << "u";
      }

    //last one is element array item;
    // o.w. already adjusted by NodeMemberSelect. t3147, t3408, t3913, t3915
    if((classtype == UC_ELEMENT) && !cosut->isScalar())
      posStr << " + T::BPA * " << uvpass.getTmpVarAsString(m_state);

    if(adjstEle)
      {
	posStr << " - T::ATOM_FIRST_STATE_BIT"; //to read entire element
      }

    return posStr.str();
  } //calcPosOfCurrentObjectClassesAsString

  //called when (implicit self) data member is a complete class;
  // pos known at compile time (e.g. t3541)
  u32 Node::calcDataMemberPosOfCurrentObjectClasses(bool askingeffself, UTI funcclassarg)
  {
    s32 pos = 0;

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    assert(cosSize > 0);

    //self for stgcos to be a data member; (self could be "OFF", e.g.cond-as w self t3831)
    UTI cuti = m_state.getCurrentSelfSymbolForCodeGen()->getUlamTypeIdx();

    u32 firstdmindex = 0;

    for(u32 i = 0; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	assert(sym);
	UTI suti = sym->getUlamTypeIdx();
	UTI sdmclass = sym->getDataMemberClass();
	//tmpSymbols (e.g. a func return value) are not data members (t3914);
	if(sym->isSelf())
	  {
	    assert(suti == cuti);
	    firstdmindex++;
	  }
	else if(sym->isSuper())
	  {
	    //'super' offset needed when not asking effself (t3749, t41333)
	    u32 suprelpos = 0;
	    AssertBool gotsuprelpos = m_state.getABaseClassRelativePositionInAClass(cuti, suti, suprelpos);
	    pos += suprelpos;
	    firstdmindex++;
	  }
	else if(sdmclass == Nouti)
	  {
	    //local stg (t3541), tmpvar funccall result, or BaseType (t41319)
	    if(sym->isTmpVarSymbol())
	      {
		assert(!((SymbolTmpVar *) sym)->isBaseClassRegNum());
		if(((SymbolTmpVar *) sym)->isBaseClassRef())
		  suti = cuti; //bypass
	      }
	    firstdmindex++;
	  }
	else
	  {
	    //data member
	    if((i > firstdmindex))
	      {
		if(sym->isTmpVarSymbol())
		  {
		    assert(!((SymbolTmpVar *) sym)->isBaseClassRegNum());
		    if(!((SymbolTmpVar *) sym)->isBaseClassRef())
		      {
			u32 tmprelpos; //error/t41313
			AssertBool gottmp = m_state.getABaseClassRelativePositionInAClass(cuti, suti, tmprelpos);
			assert(gottmp);

			pos += tmprelpos;
			pos += sym->getPosOffset();
		      }
		    else //ignore BaseClassTypes (implicit in data member class of next symbol)
		      suti = cuti; //t41319
		    firstdmindex++;
		  }
		else
		  {
		    //not a tmpvar
		    u32 relpos;
		    AssertBool gotRelPos = m_state.getABaseClassRelativePositionInAClass(cuti, sdmclass, relpos);
		    assert(gotRelPos);

		    pos += relpos;
		    pos += sym->getPosOffset();
		  }
	      }
	    else if((i == firstdmindex))
	      {
		if(!sym->isTmpVarSymbol())
		  {
		    pos += sym->getPosOffset(); //not rel base pos (t3541,3)
		    if(!askingeffself)
		      {
			u32 relpos;
			AssertBool gotRelPos = m_state.getABaseClassRelativePositionInAClass(cuti, sdmclass, relpos);
			assert(gotRelPos);
			pos += relpos;
		      } //else askingeffself so don't add relpos
		  } //else a tmpvar (e.g. arrayitem) t3542
	      }
	    else
	      m_state.abortNotImplementedYet();
	  }
	cuti = suti; //next in line
      } //forloop

    if((funcclassarg != Nouti) && m_state.isClassASubclassOf(cuti, funcclassarg))
      {
	u32 funcclassrelpos = 0;
	AssertBool gotrelpos = m_state.getABaseClassRelativePositionInAClass(cuti, funcclassarg, funcclassrelpos);
	assert(gotrelpos);
	pos += funcclassrelpos;
      }
    return pos;
  } //calcDataMemberPosOfCurrentObjectClasses

  // true means we can't know rel pos of 'stg' until runtime; o.w. known at compile time.
  // invarients: pos points to the 'stg' type, refs cannot be dm.
  // argument is the owner of the function (may or maynot be virtual);
  bool Node::askEffectiveSelfAtRuntimeForRelPosOfBase(UTI funcclassarg)
  {
    bool askEffSelf = false;
    //self may change with as-conditional; not getCompileThisIdx() (t3831)
    UTI cuti = m_state.getUlamTypeAsDeref(m_state.getCurrentSelfSymbolForCodeGen()->getUlamTypeIdx());

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(stgcos && cos);
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    //    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    UTI derefstguti = m_state.getUlamTypeAsDeref(stgcosuti);

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    s32 BTtmpi = isCurrentObjectsContainingABaseTypeTmpSymbol();

    if(stgcos->isSuper())
      {
	if((funcclassarg == Nouti))
	  askEffSelf = ((cosSize > 1) && m_state.m_currentObjSymbolsForCodeGen[1]->isDataMember()) ? (UlamType::compare(m_state.m_currentObjSymbolsForCodeGen[1]->getDataMemberClass(), derefstguti, m_state) != UTIC_SAME) : true; //t3749, t41338
	else
	  //t41304,7,8,9,10,11,14,15,16,17,18,20,21,22,23,27,28, t41333,t41336,
	  askEffSelf = true; //was false; //t3743,4,5,6,t41097,t41161,t41298,9
      }
    else if(stgcos->isDataMember()) //implicit self,t3541,t3832(arrayitem==tmp&&ref&&dm);
      {
	assert(stgcos->isTmpVarSymbol()); //implicit self mostly gone; e.g. t3175 aitem
	askEffSelf = false;
      }
    else if(stgcos->isSelf() && (cosSize > 1) && m_state.m_currentObjSymbolsForCodeGen[1]->isDataMember()) //explicit self, not BT
      {
	askEffSelf = UlamType::compare(m_state.m_currentObjSymbolsForCodeGen[1]->getDataMemberClass(), cuti, m_state) != UTIC_SAME; //t3821
      }
    else if(m_state.isReference(stgcosuti) && (cosSize > 1) && m_state.m_currentObjSymbolsForCodeGen[1]->isDataMember()) //ref, not BT
      askEffSelf = (UlamType::compare(m_state.m_currentObjSymbolsForCodeGen[1]->getDataMemberClass(), derefstguti, m_state) != UTIC_SAME); //t3914
    else if((BTtmpi > -1) && (stgcos->isSelf() || m_state.isReference(stgcosuti)))
      {
	askEffSelf = true; //a specific BaseType with a ref or self, t41323
      }
    else //default
      {
	//funcs not included in dot-chain stack (t3735); arrayitems are refs (t3832), and so is 'self';
	askEffSelf = m_state.isReference(stgcos->getUlamTypeIdx()) && ((cosSize > 1) || ((funcclassarg != Nouti) && (UlamType::compare(funcclassarg, derefstguti, m_state) != UTIC_SAME)));
      }
    return askEffSelf;
  } //askEffectiveSelfAtRuntimeForRelPosOfBase

  //false means its the entire array or not an array at all (use read() if PACKEDLOADABLE)
  bool Node::isCurrentObjectAnArrayItem(UTI cosuti, const UVPass& uvpass)
  {
    //uvpass would be an array index (an int of sorts), not an array;
    //types would not be the same;
    return(!m_state.isScalar(cosuti) && m_state.isScalar(uvpass.getPassTargetType()));
  } //isCurrentObjectAnArrayItem

  bool Node::isCurrentObjectACustomArrayItem(UTI cosuti, const UVPass& uvpass)
  {
    // a cosuti as a scalar, customarray, may be used as a regular array,
    //     but at this point cosuti would be a scalar in either case (sigh);
    // uvpass would be an array index (an int of sorts), not an array;
    // types would not be the same;
    //a custom array item, would be the return type of cossymbols 'aref' (t41006,5)
    return(m_state.isScalar(cosuti) && m_state.isClassACustomArray(cosuti) && (UlamType::compareForCustomArrayItem(uvpass.getPassTargetType(), m_state.getAClassCustomArrayType(cosuti), m_state) == UTIC_SAME));
  } //isCurrentObjectACustomArrayItem

  bool Node::isCurrentObjectAnUnpackedArray(UTI cosuti, const UVPass& uvpass)
  {
    if(m_state.isScalar(cosuti))
      return false;

    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    //either total size is greater than a long, or each item is an element or atom
    return((cosut->getTotalBitSize() > MAXBITSPERLONG) || m_state.isAtom(cosuti));
  } //isCurrentObjectAnUnpackedArray

  bool Node::isHandlingImmediateType()
  {
    // a local var that's not refining (i.e. cos[0]), or a var that's an MP
    return (isCurrentObjectALocalVariableOrArgument() && ( (m_state.m_currentObjSymbolsForCodeGen.size() == 1) || (m_state.m_currentObjSymbolsForCodeGen.back()->isModelParameter())));
  } //isHandlingImmediateType

  bool Node::needAdjustToStateBits(UTI cuti)
  {
    if(cuti == Nouti)
      return false;
    assert(m_state.okUTItoContinue(cuti));
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    if(cut->getUlamClassType() != UC_ELEMENT)
      return false;
    if(!cut->isScalar())
      return false; //array of elements, wait for item to adjust
    if(cut->isReference()) //not isAltRefType
      return false;
    return true;
  } //needAdjustToStateBits

  SymbolTmpVar * Node::makeTmpVarSymbolForCodeGen(UVPass& uvpass, Symbol * symarg)
  {
    UTI tuti = uvpass.getPassTargetType(); //possibly not a ref, e.g. array item.
    std::string tmpvarname = m_state.getTmpVarAsString(tuti, uvpass.getPassVarNum(), uvpass.getPassStorage());
    Token tidTok(TOK_IDENTIFIER, Node::getNodeLocation(), m_state.m_pool.getIndexForDataString(tmpvarname));

    u32 pos = uvpass.getPassPos();

    SymbolTmpVar * rtnsym = new SymbolTmpVar(tidTok, tuti, pos, m_state);
    assert(rtnsym);
    rtnsym->setAutoLocalType(m_state.getReferenceType(tuti));
    if(symarg && !m_state.isClassACustomArray(symarg->getUlamTypeIdx()))
      {
	rtnsym->setDataMemberClass(symarg->getDataMemberClass()); //dm doesn't pass to carray item (e.g. t3223)
	if(symarg->isLocalsFilescopeDef()) //avoid assert
	  rtnsym->setLocalsFilescopeDef(symarg->getLocalsFilescopeType()); //important for locals constant class
      }
    uvpass.setPassNameId(tidTok.m_dataindex); //read into tmpvar, not into bitvector t3919
    return rtnsym;
  } //makeTmpVarSymbolForCodeGen

  std::string Node::genUlamRefUsageAsString(UTI uti)
  {
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    std::ostringstream usageStr;
    usageStr << "UlamRef<EC>::";
    if(!ut->isScalar())
      usageStr << "ARRAY";
    else if(ut->isPrimitiveType())
      usageStr << "PRIMITIVE";
    else if(m_state.isAtom(uti))
      usageStr << "ATOMIC";
    else
      {
	ULAMCLASSTYPE classtype = ut->getUlamClassType();
	if(classtype == UC_QUARK)
	  usageStr << "CLASSIC";
	else if(classtype == UC_TRANSIENT)
	  usageStr << "CLASSIC";
	else if(classtype == UC_ELEMENT)
	  usageStr << "ELEMENTAL";
	else
	  m_state.abortUndefinedUlamClassType();
      }
    return usageStr.str();
  } //genUlamRefUsageAsString

  u32 Node::getLengthOfMemberClassForHiddenArg(UTI uti)
  {
    return m_state.getTotalBitSize(uti);
  }

  u32 Node::getBaseLengthOfMemberClassForHiddenArg(UTI uti)
  {
    return m_state.getBaseClassBitSize(uti);
  }

} //end MFM
