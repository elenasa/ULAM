#include <iostream>
#include "Node.h"
#include "CompilerState.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeFunctionCall.h"
#include "NodeIdent.h"
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

  void Node::noteTypeAndName(s32 totalsize, u32& accumsize)
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

  bool Node::isAConstant()
  {
    return false;
  }

  bool Node::isAConstantClass()
  {
    return false;
  }

  bool Node::isReadyConstant()
  {
    return false;
  }

  // only for constants (NodeTerminal)
  bool Node::isNegativeConstant()
  {
    m_state.abortShouldntGetHere();
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
    NodeBlock * currBlock = m_state.getCurrentBlock(); //in NodeIdent, getBlock();

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
    //newnode->setYourParentNo(pno);
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
    else if((nuti == Hzy) || m_state.isHolder(nuti))
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
    //m_state.abortShouldntGetHere(); //only for NodeListClassInit, NodeListArrayIniti (t41185)
    return false; //for compiler
  }

  void Node::genCodeDefaultValueStringRegistrationNumber(File * fp, u32 startpos)
  {
    m_state.abortShouldntGetHere();
    return;
  }

  void Node::genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos)
  {
    m_state.abortShouldntGetHere();
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

  // any node above assignexpr is not storeintoable
  EvalStatus Node::evalToStoreInto()
  {
    std::ostringstream msg;
    msg << "Invalid lefthand value (not storeIntoAble) '" << getName() << "'";
    msg << "; Cannot eval";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    assert(getStoreIntoAble() == TBOOL_FALSE);
    return ERROR;
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

  //default storage type is the EVALRETURN stack
  u32 Node::makeRoomForNodeType(UTI type, STORAGE where)
  {
    if(type == Void)
      return 0;

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

  void Node::genCodeReadIntoATmpVar(File * fp, UVPass & uvpass)
  {
    UTI vuti = uvpass.getPassTargetType(); //replaces vuti w target type
    assert(vuti != Void);

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    TMPSTORAGE cstor = cosut->getTmpStorageTypeForTmpVar();

    // No split if custom array, that requires an 'aref' function call;
    // handled as genCodeConvertATmpVarIntoCustomArrayAutoRef
    //assert(!isCurrentObjectACustomArrayItem(cosuti, uvpass));

    //no actual storage taken up by constant class, we have its initialized default value
    if(isCurrentObjectsContainingAConstantClass() >= 0)
      return genCodeReadFromAConstantClassIntoATmpVar(fp, uvpass); //t41198

    // write out intermediate tmpVar (i.e. terminal) as temp BitVector arg
    // e.g. when func call is rhs of secondary member select
    if(uvpass.getPassNameId() == 0)
      return genCodeConvertATmpVarIntoBitVector(fp, uvpass);

    if((m_state.getUlamTypeByIndex(cosuti)->getUlamClassType() == UC_TRANSIENT))
      return genCodeReadTransientIntoATmpVar(fp, uvpass);

    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const ");
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

    UTI derefcosuti = m_state.getUlamTypeAsDeref(cosuti); //after read, no longer a ref
    uvpass = UVPass::makePass(tmpVarNum, cstor, derefcosuti, m_state.determinePackable(cosuti), m_state, 0, 0); //POS 0 justified (atom-based).

   // note: Ints not sign extended until used/cast
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadIntoATmpVar

  void Node::genCodeReadFromAConstantClassIntoATmpVar(File * fp, UVPass & uvpass)
  {
    UTI vuti = uvpass.getPassTargetType(); //replaces vuti w target type
    assert(vuti != Void);

    s32 namedconstantclassidx = isCurrentObjectsContainingAConstantClass();
    assert(namedconstantclassidx >= 0); //right-most (last) named constant class

    Symbol * ncsym = m_state.m_currentObjSymbolsForCodeGen[namedconstantclassidx];
    assert(ncsym);
    UTI ncuti = ncsym->getUlamTypeIdx();
    assert(m_state.isAClass(ncuti)); //sanity
    assert(ncsym->isConstant()); //sanity
    assert(((SymbolWithValue *) ncsym)->isReady());

    BV8K bvclass;
    AssertBool gotval = ((SymbolWithValue *) ncsym)->getValue(bvclass);
    assert(gotval);


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

    std::string ccstr;
    bool notZero = true; //bv contents

    if(cos == ncsym) //cos is the constant class
      {
	assert(namedconstantclassidx == (cosSize - 1));
	assert(m_state.isScalar(cosuti)); //t41198 dm array
	assert(m_state.isAClass(cosuti));
      }
    else
      {
	//that is cos is a data member of our constant class,
	assert(cos->isDataMember()); //namedconstantclassidx < (cosSize - 1))
	assert(namedconstantclassidx < (cosSize - 1));
	assert(((SymbolVariableDataMember *) cos)->isPosOffsetReliable());
	cospos = ((SymbolVariableDataMember *) cos)->getPosOffset();
      }

    if(coslen <= MAXBITSPERINT)
      {
	u32 val = bvclass.Read(cospos,coslen);
	SymbolWithValue::convertValueToANonPrettyString((u64) val, cosuti, ccstr, m_state);
      }
    else if(coslen <= MAXBITSPERLONG)
      {
	u64 val = bvclass.ReadLong(cospos,coslen);
	SymbolWithValue::convertValueToANonPrettyString(val, cosuti, ccstr, m_state);
      }
    else if(cosclass == UC_ELEMENT)
      {
	//copy state bits in position for atom-based element
	BV8K bvel;
	bvclass.CopyBV<8192>(cospos, ATOMFIRSTSTATEBITPOS, MAXSTATEBITS, bvel); //srcpos, dstpos, len, dest
	notZero = SymbolWithValue::getHexValueAsString(coslen, bvel, ccstr); //t41230
      }
    else
      {
	BV8K bvtmp;
	bvclass.CopyBV<8192>(cospos, 0, coslen, bvtmp);
	notZero = SymbolWithValue::getHexValueAsString(coslen, bvtmp, ccstr);
      }

    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    if(coslen > MAXBITSPERLONG)
      {
	//oddly enough, our constant class isn't a const!
	m_state.indentUlamCode(fp);
	fp->write(cosut->getTmpStorageTypeAsString().c_str()); //T, BV<x>
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum, cstor).c_str());

	//gen comment:
	fp->write("; // constant value of ");
	fp->write(m_state.m_pool.getDataAsString(ncsym->getId()).c_str());
	if(cos->isDataMember())
	  {
	    fp->write(".");
	    fp->write(m_state.m_pool.getDataAsString(cos->getId()).c_str());
	  }
	GCNL;

	m_state.indent(fp);
	fp->write("{\n"); //limit scope of 'dam' and 'vales'
	m_state.m_currentIndentLevel++;

	m_state.indent(fp);
	fp->write("const u32 vales");
	fp->write("[(");
	fp->write_decimal_unsigned(coslen); //== [nwords]
	fp->write(" + 31)/32] = { ");
	if(notZero)
	  fp->write(ccstr.c_str());
	else
	  fp->write_decimal_unsigned(0);
	fp->write("};"); GCNL;

	if(cosclass == UC_ELEMENT)
	  {
	    m_state.indent(fp);
	    fp->write("AtomBitStorage<EC> gda(");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(cosuti).c_str());
	    fp->write(".GetDefaultAtom());"); GCNL;

	    m_state.indent(fp);
	    fp->write("u32 typefield = gda.Read(0u, T::ATOM_FIRST_STATE_BIT);"); GCNL; //can't use GetType");
	  }

	m_state.indent(fp);
	fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum, cstor).c_str());
	if(cosclass == UC_ELEMENT)
	  fp->write(".GetBits()");
	fp->write(".FromArray(vales);"); GCNL;

	if(cosclass == UC_ELEMENT)
	  {
	    m_state.indent(fp);
	    fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum, cstor).c_str());
	    fp->write(".GetBits().Write(");
	    fp->write_decimal_unsigned(cospos);
	    fp->write("u, T::ATOM_FIRST_STATE_BIT, typefield);"); GCNL;
	  }

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("}\n");
      }
    else
      {
	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(cosut->getTmpStorageTypeAsString().c_str()); //u32
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum, cstor).c_str());
	fp->write(" = ");
	fp->write(ccstr.c_str());

	//gen comment:
	fp->write("; // constant value of ");
	fp->write(m_state.m_pool.getDataAsString(ncsym->getId()).c_str());
	if(cos->isDataMember())
	  {
	    fp->write(".");
	    fp->write(m_state.m_pool.getDataAsString(cos->getId()).c_str());
	  }
	GCNL;
      }

    uvpass = UVPass::makePass(tmpVarNum, cstor, cosuti, m_state.determinePackable(cosuti), m_state, 0, 0); //POS 0 justified (atom-based).

    // note: Ints not sign extended until used/cast
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadFromAConstantClassIntoATmpVar

  void Node::genCodeReadArrayItemFromAConstantClassIntoATmpVarWithConstantIndex(File * fp, UVPass & luvpass, s32 rindex)
  {
    UTI luti = luvpass.getPassTargetType(); //replaces vuti w target type
    assert(luti != Void);
    assert(!m_state.isScalar(luti));
    assert(rindex < m_state.getArraySize(luti)); //catchable during c&l

    s32 namedconstantclassidx = isCurrentObjectsContainingAConstantClass();
    assert(namedconstantclassidx >= 0); //right-most (last) named constant class

    Symbol * ncsym = m_state.m_currentObjSymbolsForCodeGen[namedconstantclassidx];
    assert(ncsym);
    UTI ncuti = ncsym->getUlamTypeIdx();
    assert(m_state.isAClass(ncuti)); //sanity
    assert(ncsym->isConstant()); //sanity
    assert(((SymbolWithValue *) ncsym)->isReady());

    BV8K bvclass;
    AssertBool gotval = ((SymbolWithValue *) ncsym)->getValue(bvclass);
    assert(gotval);

    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);
    s32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    u32 cositemlen = cosut->getBitSize(); //what if len is zero?
    u32 cospos = 0;

    UTI scalarcosuti = m_state.getUlamTypeAsScalar(cosuti);
    UlamType * scalarcosut = m_state.getUlamTypeByIndex(scalarcosuti);
    TMPSTORAGE cstor = scalarcosut->getTmpStorageTypeForTmpVar();

    std::string ccstr;
    bool notZero = true; //bv contents

    if(cos->isDataMember()) //namedconstantclassidx < (cosSize - 1))
      {
	//that is cos is a data member of our constant class,
	assert(namedconstantclassidx < (cosSize - 1));
	assert(((SymbolVariableDataMember *) cos)->isPosOffsetReliable());

	cospos = ((SymbolVariableDataMember *) cos)->getPosOffset();
      }
    else //cos is the constant
      {
	assert(namedconstantclassidx == (cosSize - 1));
      }

    cospos += (rindex * cositemlen);

    if(cositemlen <= MAXBITSPERINT)
      {
	u32 val = bvclass.Read(cospos,cositemlen);
	SymbolWithValue::convertValueToANonPrettyString((u64) val, scalarcosuti, ccstr, m_state);
      }
    else if(cositemlen <= MAXBITSPERLONG)
      {
	u64 val = bvclass.ReadLong(cospos,cositemlen);
	SymbolWithValue::convertValueToANonPrettyString(val, scalarcosuti, ccstr, m_state);
      }
    else
      {
	BV8K bvtmp;
	bvclass.CopyBV<8192>(cospos, 0, cositemlen, bvtmp);
	notZero = SymbolWithValue::getHexValueAsString(cositemlen, bvtmp, ccstr);
      }


    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(scalarcosut->getTmpStorageTypeAsString().c_str()); //u32, u64, T, BV<x>
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(scalarcosuti, tmpVarNum, cstor).c_str());
    if(cositemlen > MAXBITSPERLONG)
      {
	fp->write("[(");
	fp->write_decimal_unsigned(cositemlen); //== [nwords]
	fp->write(" + 31)/32] = { ");
	if(notZero)
	  fp->write(ccstr.c_str());
	else
	  fp->write_decimal_unsigned(0);
	fp->write(" }");
      }
    else
      {
	fp->write(" = ");
	fp->write(ccstr.c_str());
      }

    //gen comment:
    fp->write("; // constant value of ");
    fp->write(m_state.m_pool.getDataAsString(ncsym->getId()).c_str());
    if(cos->isDataMember())
      {
	fp->write(".");
	fp->write(m_state.m_pool.getDataAsString(cos->getId()).c_str());
      }
    else
      fp->write(" item");
    fp->write("[");
    fp->write_decimal_unsigned(rindex);
    fp->write("]");
    GCNL;

    luvpass = UVPass::makePass(tmpVarNum, cstor, scalarcosuti, m_state.determinePackable(scalarcosuti), m_state, 0, 0); //POS 0 justified (atom-based).

   // note: Ints not sign extended until used/cast
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadArrayItemFromAConstantClassIntoATmpVarWithConstantIndex

  void Node::genCodeReadArrayItemFromAConstantClassIntoATmpVar(File * fp, UVPass & luvpass, UVPass & ruvpass)
  {
    //index is a variable, not known at compile time;
    UTI luti = luvpass.getPassTargetType(); //replaces vuti w target type
    assert(luti != Void);
    assert(!m_state.isScalar(luti));

    UVPass cuvpass = luvpass;
    Node::genCodeReadFromAConstantClassIntoATmpVar(fp, cuvpass); //get entire array into a tmp var

    //stack is now empty!

    //first make a temporary immediate bitvector storage for entire array value
    genCodeConvertATmpVarIntoBitVector(fp, cuvpass);

    //make an immediate ref to our storage
    UTI refluti = m_state.getUlamTypeAsRef(luti, ALT_REF); //constref?
    UlamType * reflut = m_state.getUlamTypeByIndex(refluti);

    s32 tmpVarNum3 = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    //can't be const and chainable; needs to be a ref! (e.g. t3668)
    fp->write(reflut->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(refluti, tmpVarNum3, TMPAUTOREF).c_str());
    fp->write("("); // use constructor (not equals)
    fp->write(cuvpass.getTmpVarAsString(m_state).c_str()); //storage
    fp->write(", 0u, uc);"); GCNL; //index, uc

    //now for the array item read at index in ruvpass tmp var
    UTI scalarluti = m_state.getUlamTypeAsScalar(luti);
    UlamType * scalarlut = m_state.getUlamTypeByIndex(scalarluti);
    u32 itemlen = scalarlut->getBitSize();
    TMPSTORAGE slstor = scalarlut->getTmpStorageTypeForTmpVar();

    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(scalarlut->getTmpStorageTypeAsString().c_str()); //u32, u64, T, BV<x>
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(scalarluti, tmpVarNum, slstor).c_str());
    fp->write(" = ");
    fp->write(m_state.getTmpVarAsString(refluti, tmpVarNum3, TMPAUTOREF).c_str());
    fp->write(".readArrayItem(");
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    fp->write(", ");
    fp->write_decimal_unsigned(itemlen);
    fp->write("u);"); GCNL;

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
    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(stgcos && cos);

    AssertBool cosSizeOneOrZero = (stgcos == cos);
    assert(cosSizeOneOrZero); //sanity check, pls!

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    // NOT just ur.Read() (e.g. big array of quarks: 3707, 3708)
    fp->write("UlamRef<EC>(");
    fp->write(m_state.getHiddenArgName()); //ur
    fp->write(", 0u, "); // (already + 25) e.g. t3407
    fp->write_decimal_unsigned(getLengthOfMemberClassForHiddenArg(stgcosuti));
    fp->write("u, &");
    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
    fp->write(", ");
    fp->write(genUlamRefUsageAsString(stgcosuti).c_str());
    if(!stgcosut->isAltRefType())
      fp->write(", uc");
    fp->write(").");
  } //genSelfNameOfMethod

  void Node::genCodeReadTransientIntoATmpVar(File * fp, UVPass & uvpass)
  {
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI cosuti = cos->getUlamTypeIdx();
    UTI stgcosuti = stgcos->getUlamTypeIdx();

    m_state.indentUlamCode(fp); //not const
    fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
    fp->write(" ");
    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    fp->write(";"); GCNL;

    m_state.indentUlamCode(fp); //not const
    if(stgcos->isSelf() && (stgcos == cos))
      genSelfNameOfMethod(fp);
    else if(!Node::isCurrentObjectALocalVariableOrArgument())
      genMemberNameOfMethod(fp, uvpass); //transient dm (t3967)
    else
      genLocalMemberNameOfMethod(fp, uvpass);

    if(m_state.isAltRefType(stgcosuti))
      fp->write("GetStorage()."); //need storage to do ReadBV (e.g. t3737, t3739)

    //read method based on last cos
    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
    fp->write("(0u, "); //pos part of local member name (UlamRef) (e.g. t3739, 3788, 3789, 3795, 3805)
    fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //tmp var ref
    fp->write(");"); GCNL;

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

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    loadStorageAndCurrentObjectSymbols(stgcos, cos);
    assert(cos && stgcos);

    UTI cosuti = cos->getUlamTypeIdx();

    // No split if custom array, that requires an 'aref' function call;
    // handled as genCodeConvertATmpVarIntoCustomArrayAutoRef t41006
    //assert(!isCurrentObjectACustomArrayItem(cosuti, luvpass));

    // split off autoref stg/member selected
    if(luvpass.getPassStorage() == TMPAUTOREF)
      return genCodeWriteToAutorefFromATmpVar(fp, luvpass, ruvpass);

    if(stgcos->isSelf() && (stgcos == cos))
      return genCodeWriteToSelfFromATmpVar(fp, luvpass, ruvpass);

    if(m_state.isAtomRef(luti) && m_state.isAtom(ruti))
      return genCodeWriteToAtomofRefFromATmpVar(fp, luvpass, ruvpass);

    if((m_state.getUlamTypeByIndex(cosuti)->getUlamClassType() == UC_TRANSIENT))
      return genCodeWriteToTransientFromATmpVar(fp, luvpass, ruvpass);

    bool isElementAncestorCast = (lut->getUlamClassType() == UC_ELEMENT) && m_state.isClassASubclassOf(ruti, luti);

    UVPass typuvpass;
    if(isElementAncestorCast)
      {
	//readTypefield of lhs before the write!
	// pass as rhs uv to restore method afterward;
	// avoids making default atom.
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
    TMPSTORAGE rstor = ruvpass.getPassStorage();
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    if(rstor == TMPBITVAL)
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
	// pass as rhs uv to restore method afterward;
	// avoids making default atom.
	genCodeReadElementTypeField(fp, typuvpass);
      }

    m_state.indentUlamCode(fp);
    genSelfNameOfMethod(fp); // NOT just ur.Write(tmpvar) e.g. t41120
    fp->write(writeMethodForCodeGen(luti, luvpass).c_str());
    fp->write("(");
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    if(ruvpass.getPassStorage() == TMPBITVAL)
      fp->write(".read()");

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

    m_state.indentUlamCode(fp);

    //local
    genLocalMemberNameOfMethod(fp, luvpass);

    fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
    fp->write("(0u, "); //pos part of local member name (UlamRef)
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str()); //tmp var ref
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
    if(ruvpass.getPassStorage() == TMPBITVAL)
      fp->write(".read()");
    fp->write("); //write into atomof ref"); GCNL;

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeWriteToAtomofRefFromATmpVar

  void Node::genCodeWriteToAutorefFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass)
  {
    //unlike the others, here, uvpass is the autoref (stg);
    //cos tell us where to go within the selected member
    UTI luti = luvpass.getPassTargetType();

    m_state.indentUlamCode(fp);
    fp->write(luvpass.getTmpVarAsString(m_state).c_str()); //TMPAUTOREF
    fp->write(".");
    fp->write(writeMethodForCodeGen(luti, luvpass).c_str());
    fp->write("(");
    //VALUE TO BE WRITTEN:
    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too! value is not a terminal
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    if(ruvpass.getPassStorage() == TMPBITVAL)
      fp->write(".read()");
    fp->write(");\n");

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

	//for ANY immediate atom arg from a T
	//needs effective self from T's type
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
	if(vut->getUlamTypeEnum() == String)
	  {
	    //TMPSTORAGE vstor = uvpass.getPassStorage();
	    //assert(!((vstor == TMPBITVAL) || (vstor == TMPAUTOREF))); tis true!
	    const std::string stringmangledName = m_state.getUlamTypeByIndex(String)->getLocalStorageTypeAsString();
	    fp->write(stringmangledName.c_str());
	    fp->write("::getRegNum(");
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write("), "); //e.g. t3961, t3973
	    fp->write(stringmangledName.c_str());
	    fp->write("::getStrIdx(");
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //VALUE
	    fp->write(")");
	  }
	else
	  {
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //VALUE

	    if(vut->getUlamClassType() == UC_NOTACLASS)
	      {
		//no longer atom-based primitives
	      }

	    if(m_state.isReference(vuti)) //i'm back
	      {
		fp->write(", ");
		fp->write_decimal_unsigned(pos); //position for constructor
		fp->write("u");
	      }
	  }
      }
    fp->write(");"); GCNL; //func arg& ?

    uvpass = UVPass::makePass(tmpVarNum2, TMPBITVAL, vuti, m_state.determinePackable(vuti), m_state, pos, 0); //POS left-justified for quarks; right for primitives.
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeConvertATmpVarIntoBitVector

  // write out immediate tmp BitValue as an intermediate tmpVar
  void Node::genCodeConvertABitVectorIntoATmpVar(File * fp, UVPass & uvpass)
  {
    UTI vuti = uvpass.getPassTargetType();
    assert(m_state.okUTItoContinue(vuti));
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    assert(uvpass.getPassStorage() == TMPBITVAL);

    // write out immediate tmp BitValue as an intermediate tmpVar
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    TMPSTORAGE tmp2stor = vut->getTmpStorageTypeForTmpVar();

    m_state.indentUlamCode(fp);
    fp->write("const ");

    fp->write(vut->getTmpStorageTypeAsString().c_str()); //u32
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, tmp2stor).c_str());
    fp->write(" = ");

    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    fp->write(".read();"); GCNL;

    // use immediate read for entire array; doesn't make sense for custom arrays
    assert(!isCurrentObjectAnArrayItem(vuti, uvpass)); //sanity

    uvpass = UVPass::makePass(tmpVarNum2, tmp2stor, vuti, m_state.determinePackable(vuti), m_state, 0, 0); //POS 0 rightjustified (atom-based).
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
    m_state.indentUlamCode(fp);
    //can't be const and chainable; needs to be a ref! (e.g. t3668)
    fp->write(scalarrefut->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(scalarrefuti, tmpVarNum2, TMPAUTOREF).c_str());
    fp->write("("); // use constructor (not equals)

    if(cos->isDataMember())
      {
	if(stgcos->isDataMember() && !stgcos->isTmpVarSymbol()) //t3149, t3147
	  {
	    fp->write(m_state.getHiddenArgName()); //t3543
	    stgisaref = true;
	  }
	else
	  {
	    fp->write(stgcos->getMangledName().c_str()); //t3702, t3818
	    stgisaref = stgcosut->isReference(); //t3114 (not isAltRefType)
	  }
      }
    else
      {
	fp->write(cos->getMangledName().c_str()); //local array
	stgisaref = cosut->isReference(); //not isAltRefType
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
	fp->write(" * BPA"); //t3670
	fp->write(" + T::ATOM_FIRST_STATE_BIT"); //t3814, t3908, t3832
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

    fp->write(" + ");
    fp->write_decimal_unsigned(pos); //rel offset (t3512, t3543, t3648, t3702, t3776, t3668, t3811, t3946)
    fp->write("u");

    if(cosclasstype != UC_NOTACLASS)
      {
	fp->write(", &");
	fp->write(m_state.getTheInstanceMangledNameByIndex(scalarcosuti).c_str()); //t3495, t3512, t3543, t3648, t3702, t3776, t3668, t3811
      }
    //else skip (t3114)

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

    //first array item, with item in uvpass (e.g. t3147)
    assert(ruvpass.getPassStorage() == TMPARRAYIDX);

    assert(!cosut->isScalar());

    UTI scalarcosuti = m_state.getUlamTypeAsScalar(cosuti); //ALT_ARRAYITEM
    UlamType * scalarcosut = m_state.getUlamTypeByIndex(scalarcosuti);

    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(scalarcosut->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(scalarcosuti, tmpVarNum2, TMPAUTOREF).c_str());
    fp->write("("); // use constructor (not equals)

    fp->write(((SymbolConstantValue *) cos)->getCompleteConstantMangledName().c_str()); //constant

    fp->write(", ");
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str()); //item idx variable 0-based

    fp->write(" * ");
    fp->write_decimal_unsigned(cosut->getBitSize()); //no rel offset
    fp->write("u, uc);"); GCNL;

    cosuti = scalarcosuti;
    luvpass = UVPass::makePass(tmpVarNum2, TMPAUTOREF, cosuti, m_state.determinePackable(cosuti), m_state, pos, cos->getId()); //POS left-justified by default
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
    UTI vuti = vsymptr->getUlamTypeIdx(); //i.e. this ref node
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
      return genCodeArrayItemRefInit(fp, uvpass, vsymptr); //t3811

    ULAMTYPE vetyp = vut->getUlamTypeEnum();
    u32 pos = 0;

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
	fp->write(");"); GCNL;
      }
    else if(!m_state.isReference(stgcosuti)) //not isAltRefType (t3650)
      {
	pos = uvpass.getPassPos();

	if(stgcos->isDataMember()) //may be an element/atom in a TRANSIENT
	  fp->write(m_state.getHiddenArgName());
	else
	  fp->write(stgcos->getMangledName().c_str());
	fp->write(", ");

	if(needAdjustToStateBits(cosuti) && (vetyp != UAtom)) //t3907
	  fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3803, t3832
	fp->write_decimal_unsigned(pos); //rel offset
	fp->write("u");
	if(vetyp == Class)
	  {
	    fp->write(", &"); //left just
	    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	  }

	if(!stgcos->isDataMember()) //technically stg is self (ur) if dm, so no uc (t3695)
	  fp->write(", uc"); //t3820, t3613, t3615
	fp->write(");"); GCNL;
      }
    else
      {
	//a reference, that's not a tmpref calcs its position (t3820, t3908, t3910)
	if(!cos->isTmpVarSymbol())
	  {
	    pos = uvpass.getPassPos();
	    fp->write(stgcos->getMangledName().c_str()); //t3819, t3908
	  }
	else
	  fp->write(cos->getMangledName().c_str()); //t3832

	ULAMCLASSTYPE cosclasstype = cosut->getUlamClassType(); //t3908
	if(vetyp == Class) //also not an atom
	  {
	    if(!(cos->isSelf() && cosclasstype == UC_QUARK))
	      {
		fp->write(", ");
		if(needAdjustToStateBits(cosuti))
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
	  }
	// else non-class has no effective self
	fp->write(");"); GCNL;
      }
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

    m_state.indentUlamCode(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(vsymptr->getMangledName().c_str());
    fp->write("("); //pass ref in constructor (ref's not assigned with =)

    if(stgcos->isDataMember() && !stgcosut->isReference()) //rhs may be an element/atom in a transient; not a reference (not isAltRefType?)
      {
	//can be a reference when an array item (t3818)
	fp->write(m_state.getHiddenArgName());
	fp->write(", ");
	fp->write_decimal_unsigned(pos); //relative off array base
	fp->write("u");

	if(vetyp == Class)
	  {
	   if(stgcosut->isScalar() && needAdjustToStateBits(stgcosuti))
	     fp->write(" + T::ATOM_FIRST_STATE_BIT"); //?
	   assert(!cosut->isReference()); //not isAltRefType?
	   fp->write(", NULL");
	  }
	fp->write(");"); GCNL;
      }
    else
      {
	//local stg or reference
	fp->write(stgcos->getMangledName().c_str());
	if(cos->isDataMember())
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(pos); //rel offset array base (t3832)

	    fp->write("u");

	    if(vetyp == Class)
	      {
		if(needAdjustToStateBits(vuti))
		  fp->write(" + T::ATOM_FIRST_STATE_BIT");
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
		assert(0); //needs a test!!
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

  void Node::generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly)
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
    //fufilled by NodeVarDecl and NodeConstDef; bypassed by NodeTypedef
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

    if(nuti == Hzy)
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
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
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
    u32 selfid = m_state.m_pool.getIndexForDataString("self"); //was "self"
    Token selfTok(TOK_IDENTIFIER, loc, selfid);

    m_state.m_parsingVariableSymbolTypeFlag = STF_FUNCPARAMETER; //for symbol install
    SymbolVariableStack * selfsym = new SymbolVariableStack(selfTok, UAtom, m_state);
    selfsym->setIsSelf();
    m_state.addSymbolToCurrentScope(selfsym); //ownership goes to the fblock

    UTI nodeType = node->getNodeType(); //quark
    UlamType * nut = m_state.getUlamTypeByIndex(nodeType);
    assert(nut->getUlamClassType() == UC_QUARK);
    u32 quid = nut->getUlamTypeNameId();
    Token typeTok(TOK_TYPE_IDENTIFIER, loc, quid);

    TypeArgs typeargs;
    typeargs.init(typeTok);
    typeargs.m_bitsize = nut->getBitSize();
    typeargs.m_arraysize = nut->getArraySize();
    typeargs.m_classInstanceIdx = nodeType;

    u32 argid = m_state.m_pool.getIndexForDataString("_arg"); //t3411, t3412
    Token argTok(TOK_IDENTIFIER, loc, argid);
    NodeIdent * argIdentNode = new NodeIdent(argTok, NULL, m_state);
    assert(argIdentNode);
    argIdentNode->setNodeLocation(loc);

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

	fblock->appendNextNode(returnNode);
	fblock->setDefinition();
	fblock->setMaxDepth(0); //no local variables, except params
      }

    //this block's ST is no longer in scope
    m_state.popClassContext(); //= prevBlock;

    m_state.m_parsingVariableSymbolTypeFlag = STF_NEEDSATYPE;

    //func call symbol to return to NodeCast; fsymptr maybe null
    NodeFunctionCall * funccall = new NodeFunctionCall(funcidentTok, fsymptr, m_state);
    assert(funccall);
    funccall->setNodeLocation(loc);
    funccall->addArgument(node);
    rtnNode = funccall;

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

  void Node::genMemberNameOfMethod(File * fp, UVPass& uvpass, bool endingdot)
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
	fp->write(cos->getMangledName().c_str());
	fp->write(".");
	return; //done
      }

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    UTI cosclassuti = cos->getDataMemberClass();
    UlamType * cosclassut = m_state.getUlamTypeByIndex(cosclassuti);
    assert(!cosclassut->isAltRefType());

    if((cosSize == 1))
       stgcos = m_state.getCurrentSelfSymbolForCodeGen();

    u32 pos = uvpass.getPassPos();

    fp->write("UlamRef<EC>("); //wrapper for dm
    if(cosSize > 1 && stgcos->isTmpVarSymbol())
      fp->write(stgcos->getMangledName().c_str()); //first arg t3543, not t3512
    else
      fp->write(m_state.getHiddenArgName()); //ur first arg
    fp->write(", ");

    //reading entire thing, using ELEMENTAL, t.f. adjust (t3880)
    if((cosSize == 1) && (cosut->getUlamClassType() == UC_ELEMENT))
      {
	if(needAdjustToStateBits(cosuti))
	  fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3880
	fp->write("0u, "); //t3147
      }
    else
      {
	fp->write_decimal_unsigned(pos); //rel offset //t3143, t3543
	fp->write("u, ");
      }

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
    AssertBool isDefinedFunc = m_state.isFuncIdInAClassScope(cosuti, m_state.getCustomArrayGetFunctionNameId(),fnsymptr, hazyKin); //searches class of cos
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
  std::string Node::genHiddenArg2(UVPass uvpass, u32& urtmpnumref)
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
    UTI cosuti = cos->getUlamTypeIdx();

    // first "hidden" arg is the context, then "hidden" ref self (ur) arg
    if(!Node::isCurrentObjectALocalVariableOrArgument())
      {
	if(m_state.m_currentObjSymbolsForCodeGen.empty())
	  hiddenarg2 << m_state.getHiddenArgName(); //same ur
	else if(stgcos->isSelf() && (stgcos == cos)) //t3831, t3274, t3275
	  hiddenarg2 << m_state.getHiddenArgName(); //same ur
	else
	  {
	    sameur = false;
	    hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";
	    //update ur to reflect "effective" self for this funccall
	    if(stgcos->isTmpVarSymbol())
	      hiddenarg2 << stgcos->getMangledName().c_str(); //t3811
	    else
	      hiddenarg2 << m_state.getHiddenArgName(); //ur t3102,3,4,6,7,8,9,10,11
	    hiddenarg2 << ", " << calcPosOfCurrentObjectClassesAsString(uvpass); //relative off;
	    hiddenarg2 << ", " << getLengthOfMemberClassForHiddenArg(cosuti) << "u, &"; //len, t41120

	    hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(cosuti).c_str(); //cos->isSuper rolls as cosuti
	    hiddenarg2 << ", " << genUlamRefUsageAsString(cosuti).c_str();
	    hiddenarg2 << ");";
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
	    if(m_state.m_currentObjSymbolsForCodeGen.empty())
	      {
		hiddenarg2 << m_state.getHiddenArgName(); //same ur
	      }
	    else if(stgcosut->isReference()) //t3751, t3752, (ALT_AS: t3249, t3255) not isAltRefType
	      {
		sameur = false;
		//update ur to reflect "effective" self for this funccall
		hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";
		hiddenarg2 << stgcos->getMangledName().c_str() << ", ";

		if(cos->isDataMember()) //dm of local stgcos
		  {
		    hiddenarg2 << calcPosOfCurrentObjectClassesAsString(uvpass); //relative off;
		    hiddenarg2 << ", " << getLengthOfMemberClassForHiddenArg(cosuti) << "u, "; //len, t41120
		    hiddenarg2 << "&"; //effective self of dm (t3804 check -10)
		    hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(cosuti).c_str();
		    hiddenarg2 << ", " << genUlamRefUsageAsString(cosuti).c_str();
		    hiddenarg2 << ");";
		  }
		else
		  {
		    //ancestor takes on effective self of sub
		    //uses UlamRef 2 arg copy constructor to maintain EffSelf and UsageType of ref
		    hiddenarg2 << getLengthOfMemberClassForHiddenArg(cosuti) << "u);" ; //len t3637, t3746
		  }
	      }
	    else
	      {
		sameur = false;
		//new ur to reflect "effective" self and storage for this funccall
		hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";

		hiddenarg2 << calcPosOfCurrentObjectClassesAsString(uvpass); //relative off;
		hiddenarg2 << ", " << getLengthOfMemberClassForHiddenArg(cosuti) << "u, "; //len

		hiddenarg2 << stgcos->getMangledName().c_str() << ", &"; //storage
		hiddenarg2 << m_state.getTheInstanceMangledNameByIndex(cosuti).c_str();

		hiddenarg2 << ", " << genUlamRefUsageAsString(cosuti).c_str();
		hiddenarg2 << ", uc";
		hiddenarg2 << ");";
	      }
	  }
      }

    if(!sameur)
      urtmpnumref = tmpvar; //update arg

    return hiddenarg2.str();
  } //genHiddenArg2

  void Node::genLocalMemberNameOfMethod(File * fp, UVPass& uvpass)
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

  void Node::genLocalMemberNameOfMethodByUsTypedef(File * fp, UVPass& uvpass)
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

    u32 pos = uvpass.getPassPos();

    fp->write("UlamRef<EC>("); //wrapper for local storage

    if(stgcosut->isReference()) //not isAltRefType (t3249)
      {
	fp->write(stgcos->getMangledName().c_str()); //ref
	fp->write(", ");
      }

    //reading entire thing, using ELEMENTAL, t.f. adjust (t3735)
    if(needAdjustToStateBits(cosuti))
      fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3735

    fp->write_decimal_unsigned(pos); //rel offset
    fp->write("u, ");

    if(cosclasstype == UC_ELEMENT)
      {
	//elements too (t3735)..arrays of elements (t3735)?
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

    if(!stgcosut->isReference()) //not isAltRefType, includes ALT_AS (t3249)
      {
	if(cos->isConstant()) //t41152 from another class
	  fp->write(((SymbolConstantValue *) cos)->getCompleteConstantMangledName().c_str()); //constant
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

  void Node::genLocalMemberNameOfMethodForAtomof(File * fp, UVPass& uvpass)
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
    AssertBool isDefinedFunc = m_state.isFuncIdInAClassScope(cosuti, m_state.getCustomArrayGetFunctionNameId(),fnsymptr, hazyKin); //searches class of cos
    assert(isDefinedFunc);
    assert(!hazyKin);

    UTI futi = fnsymptr->getDataMemberClass();
    fp->write(m_state.getTheInstanceMangledNameByIndex(futi).c_str());
    fp->write(".");
  } //genCustomArrayLocalMemberNameOfMethod

  const std::string Node::localStorageTypeAsString(UTI nuti)
  {
    return m_state.getUlamTypeByIndex(nuti)->getLocalStorageTypeAsString();
  }

  const std::string Node::tmpStorageTypeForRead(UTI nuti, UVPass uvpass)
  {
    return m_state.getUlamTypeByIndex(nuti)->getTmpStorageTypeAsString();
  }

  const std::string Node::tmpStorageTypeForReadArrayItem(UTI nuti, UVPass uvpass)
  {
    return m_state.getUlamTypeByIndex(nuti)->getArrayItemTmpStorageTypeAsString();
  } //tmpStorageTypeForReadArrayItem

  const std::string Node::readMethodForCodeGen(UTI nuti, UVPass uvpass)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    std::string method;
    Symbol * cos = NULL;
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      cos = m_state.m_currentObjSymbolsForCodeGen.back();

    if(cos && cos->isTmpVarSymbol())
      method = "read";
    else if(isCurrentObjectsContainingAModelParameter() >= 0)
      method = "read"; //an exception
    else if(!isCurrentObjectALocalVariableOrArgument())
      method = nut->readMethodForCodeGen(); //UlamRef
    else if(m_state.m_currentObjSymbolsForCodeGen.size() > 1)
      method = nut->readMethodForCodeGen(); //UlamRef
    else if(nut->getUlamClassType() == UC_TRANSIENT)
      method = nut->readMethodForCodeGen(); //BitStorage ReadBV
    else
      method = "read"; //local variable name, not a transient
    return method;
  } //readMethodForCodeGen

  const std::string Node::readArrayItemMethodForCodeGen(UTI nuti, UVPass uvpass)
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

  const std::string Node::writeMethodForCodeGen(UTI nuti, UVPass uvpass)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    std::string method;

    Symbol * cos = NULL;
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      cos = m_state.m_currentObjSymbolsForCodeGen.back();

    if(cos && cos->isTmpVarSymbol())
      method = "write";
    else if(!isCurrentObjectALocalVariableOrArgument())
      method =  nut->writeMethodForCodeGen(); //UlamRef
    else if(m_state.m_currentObjSymbolsForCodeGen.size() > 1)
      method = nut->writeMethodForCodeGen(); //UlamRef
    else if(nut->getUlamClassType() == UC_TRANSIENT)
      method = nut->writeMethodForCodeGen(); //BitStorage ReadBV
    else
      method = "write"; //local variable name, not a transient
    return method;
  } //writeMethodForCodeGen

  const std::string Node::writeArrayItemMethodForCodeGen(UTI nuti, UVPass uvpass)
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
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	//"self" belongs to func def block that we're currently gencoding
	u32 selfid = m_state.m_pool.getIndexForDataString("self");
	Symbol * selfsym = NULL;
	bool hazykin = false; //unused
	AssertBool gotSelf = m_state.alreadyDefinedSymbol(selfid, selfsym, hazykin);
	assert(gotSelf);

	//stgcosref = cosref = m_state.getCurrentSelfSymbolForCodeGen();
	stgcosref = cosref = selfsym; //t41065, foofunc();
      }
    else if(cosSize == 1)
      {
	cosref = m_state.m_currentObjSymbolsForCodeGen.back();
      	stgcosref = m_state.m_currentObjSymbolsForCodeGen[0]; //same e.g. for locals
	rtnstgidx = 0;
      }
    else
      {
	cosref = m_state.m_currentObjSymbolsForCodeGen.back();
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
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      return false; //must be self, t.f. not local

    s32 namedconstantidx = isCurrentObjectsContainingAConstant();
    if(m_state.m_currentObjSymbolsForCodeGen[0]->isDataMember() && (namedconstantidx > -1))
      return true; //data member, and named constant array: treat like local storage

    s32 modelparamidx = isCurrentObjectsContainingAModelParameter();
    if(m_state.m_currentObjSymbolsForCodeGen[0]->isDataMember() && (modelparamidx == -1))
      return false; //data member, not model parameter

    UTI stgcosuti = m_state.m_currentObjSymbolsForCodeGen[0]->getUlamTypeIdx();
    if(m_state.m_currentObjSymbolsForCodeGen[0]->isSelf() && !(m_state.isAtom(stgcosuti) || m_state.getUlamTypeByIndex(stgcosuti)->getUlamClassType() == UC_TRANSIENT) && (modelparamidx == -1) && (namedconstantidx == -1))
      return false; //self, neither atom nor transient, not modelparameter, nor named constant array

    return true; //including references
  } //isCurrentObjectALocalVariableOrArgument

  // returns the index to the last object that's an MP; o.w. -1 none found;
  // preceeding object is the "owner", others before it are irrelevant;
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
  // preceeding object is the "owner", others before it are irrelevant;
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

  // returns the index to the last object that's a named constant; o.w. -1 none found;
  // preceeding object is the "owner", others before it are irrelevant;
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

  // used by genHiddenArg2 for function calls; uvpass may contain the index
  // of an array item, o.w. the current arg's tmp var (unneeded here).
  std::string Node::calcPosOfCurrentObjectClassesAsString(UVPass uvpass)
  {
    s32 pos = uvpass.getPassPos();

    std::ostringstream posStr;
    posStr << pos << "u";
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
	UTI cosuti = cos->getUlamTypeIdx();
	UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
	ULAMCLASSTYPE classtype = cosut->getUlamClassType();
	//last one is element array item;
	// o.w. already adjusted by NodeMemberSelect. t3147, t3408, t3913, t3915
	if((classtype == UC_ELEMENT) && !cosut->isScalar())
	  posStr << " + T::BPA * " << uvpass.getTmpVarAsString(m_state);
      }
    return posStr.str();
  } //calcPosOfCurrentObjectClassesAsString

  //false means its the entire array or not an array at all (use read() if PACKEDLOADABLE)
  bool Node::isCurrentObjectAnArrayItem(UTI cosuti, UVPass uvpass)
  {
    //uvpass would be an array index (an int of sorts), not an array;
    //types would not be the same;
    return(!m_state.isScalar(cosuti) && m_state.isScalar(uvpass.getPassTargetType()));
  } //isCurrentObjectAnArrayItem

  bool Node::isCurrentObjectACustomArrayItem(UTI cosuti, UVPass uvpass)
  {
    // a cosuti as a scalar, customarray, may be used as a regular array,
    //     but at this point cosuti would be a scalar in either case (sigh);
    // uvpass would be an array index (an int of sorts), not an array;
    // types would not be the same;
    //a custom array item, would be the return type of cossymbols 'aref' (t41006,5)
    return(m_state.isScalar(cosuti) && m_state.isClassACustomArray(cosuti) && (UlamType::compareForCustomArrayItem(uvpass.getPassTargetType(), m_state.getAClassCustomArrayType(cosuti), m_state) == UTIC_SAME));
  } //isCurrentObjectACustomArrayItem

  bool Node::isCurrentObjectAnUnpackedArray(UTI cosuti, UVPass uvpass)
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

  void Node::adjustUVPassForElements(UVPass & uvpass)
  {
    UTI puti = uvpass.getPassTargetType();
    if(needAdjustToStateBits(puti))
      {
	u32 lpos = uvpass.getPassPos();
	uvpass.setPassPosForElementType(lpos, m_state);
      }
  }

  SymbolTmpVar * Node::makeTmpVarSymbolForCodeGen(UVPass& uvpass, Symbol * sym)
  {
    UTI tuti = uvpass.getPassTargetType(); //possibly not a ref, e.g. array item.
    std::string tmpvarname = m_state.getTmpVarAsString(tuti, uvpass.getPassVarNum(), uvpass.getPassStorage());
    Token tidTok(TOK_IDENTIFIER, Node::getNodeLocation(), m_state.m_pool.getIndexForDataString(tmpvarname));

    u32 pos = uvpass.getPassPos();

    SymbolTmpVar * rtnsym = new SymbolTmpVar(tidTok, tuti, pos, m_state);
    assert(rtnsym);
    rtnsym->setAutoLocalType(m_state.getReferenceType(tuti));
    if(sym && !m_state.isClassACustomArray(sym->getUlamTypeIdx()))
      rtnsym->setDataMemberClass(sym->getDataMemberClass()); //dm doesn't pass to carray item (e.g. t3223)
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

  u32 Node::getLengthOfMemberClassForHiddenArg(UTI cosuti)
  {
    return m_state.getUlamTypeByIndex(cosuti)->getTotalBitSize();
  }

} //end MFM
