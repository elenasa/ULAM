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

  NNO Node::getYourParentNo()
  {
    return m_parentNo;
  }

  void Node::updateLineage(NNO pno)
  {
    setYourParentNo(pno); //walk the tree..a leaf.
  }

  NNO Node::getNodeNo()
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
  } //exhangeKids

  bool Node::findNodeNo(NNO n, Node *& foundNode)
  {
    if(m_no == n) //leaf
      {
	foundNode = this;
	return true;
      }
    return false;
  } //findNodeNo

  void Node::checkAbstractInstanceErrors()
  {
    return; //default
  } //checkAbstractInstanceErrors

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

  UTI Node::getNodeType()
  {
    return m_utype;
  }

  void Node::setNodeType(UTI ut)
  {
    m_utype = ut;
  }

  TBOOL Node::getStoreIntoAble()
  {
    return m_storeIntoAble;
  }

  void Node::setStoreIntoAble(TBOOL s)
  {
    m_storeIntoAble = s;
    setReferenceAble(s); //usually the case, except custom arrays
  }

  TBOOL Node::getReferenceAble()
  {
    return m_referenceAble;
  }

  void Node::setReferenceAble(TBOOL s)
  {
    m_referenceAble = s;
  }

  Locator Node::getNodeLocation()
  {
    return m_loc;
  }

  void Node::setNodeLocation(Locator loc)
  {
    m_loc = loc;
  }

  void Node::printNodeLocation(File * fp)
  {
    fp->write(getNodeLocationAsString().c_str());
  }

  std::string Node::getNodeLocationAsString()
  {
    return m_state.getFullLocationAsString(m_loc);
  }

  bool Node::getSymbolPtr(Symbol *& symptrref)
  {
    return false;
  }

  void Node::constantFoldAToken(const Token& tok)
  {
    assert(0); //only NodeTerminal has this defined; NodeConstant bypasses
  }

  bool Node::isAConstant()
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
    assert(0);
    return false;
  }

  bool Node::isWordSizeConstant()
  {
    assert(0);
    return false;
  }

  bool Node::isFunctionCall()
  {
    return false;
  }

  bool Node::isExplicitReferenceCast()
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
  } //safeToCastTo

  bool Node::checkSafeToCastTo(UTI fromType, UTI& newType)
  {
    bool rtnOK = true;
    UlamType * tobe = m_state.getUlamTypeByIndex(newType);
    FORECAST scr = tobe->safeCast(fromType);
    if(scr != CAST_CLEAR)
      {
	std::ostringstream msg;
	if(tobe->getUlamTypeEnum() == Bool)
	  msg << "Use a comparison operator";
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
	    m_state.setGoAgain();
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
    assert(0);
    return false;
  }

  void Node::genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos)
  {
    assert(0);
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
	else
	  assert(0);
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

  //in case of arrays, rtnUV is a ptr.
  void Node::assignReturnValuePtrToStack(UlamValue rtnUVptr)
  {
    UTI rtnUVtype = rtnUVptr.getPtrTargetType(); //target ptr type

    if(rtnUVtype == Void)
      return;

    UlamValue rtnPtr = UlamValue::makePtr(-1, EVALRETURN, rtnUVtype, rtnUVptr.isTargetPacked(), m_state);
    m_state.m_nodeEvalStack.assignUlamValuePtr(rtnPtr, rtnUVptr);
  }

  void Node::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert(0);
  }

  void Node::printUnresolvedVariableDataMembers()
  {
    assert(0);
  } //printUnresolvedVariableDataMembers

  void Node::printUnresolvedLocalVariables(u32 fid)
  {
    //override
  } //printUnresolvedLocalVariables

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
    assert(0);
    return;
  } //genCodeToStoreInto

  void Node::genCodeReadIntoATmpVar(File * fp, UVPass & uvpass)
  {
    UTI vuti = uvpass.getPassTargetType(); //replaces vuti w target type
    assert(vuti != Void);

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI cosuti = cos->getUlamTypeIdx();

    // split off reading array items
    if(isCurrentObjectAnArrayItem(cosuti, uvpass))
      return genCodeReadArrayItemIntoATmpVar(fp, uvpass);

    // split if custom array, that requires an 'aref' function call
    // immediate types no longer have an array method to call for CA's.
    if(isCurrentObjectACustomArrayItem(cosuti, uvpass))
      return genCodeReadCustomArrayItemIntoATmpVar(fp, uvpass);

    // write out intermediate tmpVar (i.e. terminal) as temp BitVector arg
    // e.g. when func call is rhs of secondary member select
    if(uvpass.getPassNameId() == 0)
      return genCodeConvertATmpVarIntoBitVector(fp, uvpass);

    if((m_state.getUlamTypeByIndex(cosuti)->getUlamClassType() == UC_TRANSIENT))
      return genCodeReadTransientIntoATmpVar(fp, uvpass);

    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
    fp->write(" ");
    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
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
	    genMemberNameOfMethod(fp);

	    // the READ method
	    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
	    fp->write("(");

	    // a data member quark, or the element itself should both GetBits from self
	    // now, quark's self is treated as the entire atom/element storage
	    fp->write(");"); GCNL; //PLS generate name of var here!!!
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
		genLocalMemberNameOfMethod(fp);

		//read method based on last cos
		fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());

		// local quark or primitive (i.e. 'notaclass'); has an immediate type:
		// uses local variable name, and immediate read method
		fp->write("();"); GCNL;
	      }
	  }
      }
    // note: Ints not sign extended until used/cast
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadIntoATmpVar

  void Node::genCodeReadSelfIntoATmpVar(File * fp, UVPass & uvpass)
  {
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    else
     stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

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
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      {
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
      }

    AssertBool cosSizeOneOrZero = (stgcos == cos);
    assert(cosSizeOneOrZero); //sanity check, pls!

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    // NOT just ur.Read() (e.g. big array of quarks: 3707, 3708)
    fp->write("UlamRef<EC>(");
    fp->write(m_state.getHiddenArgName()); //ur
    fp->write(", 0u, "); // (already + 25) e.g. t3407
    fp->write_decimal_unsigned(stgcosut->getTotalBitSize());
    fp->write("u, &");
    fp->write(m_state.getEffectiveSelfMangledNameByIndex(stgcosuti).c_str());
    fp->write(", ");
    fp->write(genUlamRefUsageAsString(stgcosuti).c_str());
    if(!stgcosut->isReference())
      fp->write(", uc");
    fp->write(").");
  } //genSelfNameOfMethod

  void Node::genCodeReadTransientIntoATmpVar(File * fp, UVPass & uvpass)
  {
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

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
    else
      genLocalMemberNameOfMethod(fp);

    if(m_state.isReference(stgcosuti))
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

  void Node::genCodeReadArrayItemIntoATmpVar(File * fp, UVPass & uvpass)
  {
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data
    UTI vuti = uvpass.getPassTargetType(); //replaces vuti w target type
    assert(vuti != Void);
    assert(m_state.getUlamTypeByIndex(vuti)->isNumericType());

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    ULAMCLASSTYPE classtype = cosut->getUlamClassType();
    UTI	scalarcosuti = m_state.getUlamTypeAsScalar(cosuti); //ALT_ARRAYITEM
    scalarcosuti = m_state.getUlamTypeAsDeref(scalarcosuti);
    UlamType * scalarcosut = m_state.getUlamTypeByIndex(scalarcosuti);
    TMPSTORAGE cstor = scalarcosut->getTmpStorageTypeForTmpVar();

    bool isLocal = isCurrentObjectALocalVariableOrArgument();
    assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid

    u32 itemlen = cosut->getBitSize();
    s32 pos = Node::calcPosOfCurrentObjects();

    // all the cases where '=' is used; else BitVector constructor for converting a tmpvar
    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(tmpStorageTypeForReadArrayItem(cosuti, uvpass).c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(scalarcosuti, tmpVarNum2, cstor).c_str());
    fp->write(" = ");
    fp->write("UlamRef<EC>("); //wrapper for array item

    if(!isLocal)
      {
	//i.e. data member
	fp->write(m_state.getHiddenArgName()); //ur first arg
	fp->write(", ");
      }
    else if(stgcosut->isReference())
      {
	fp->write(stgcos->getMangledName().c_str()); //reference first
	fp->write(", ");
      }

    fp->write("("); //start index calc

    //still adds ATOM_FIRST_STATE_BIT when writing the entire element, usage ELEMENTAL, when cos is an element
    if(needAdjustToStateBits(cosuti))
      fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3832

    fp->write_decimal_unsigned(pos); //rel offset of array base (e.g.t3147, t3148, t3153, t3177..)
    fp->write("u + ");

    fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //INDEX
    fp->write(" * ");
    if(classtype == UC_ELEMENT)
      {
    	fp->write("T::BPA), "); //atom-based item length (e.g. t3832)
      }
    else
      {
	fp->write_decimal_unsigned(itemlen); //BITS_PER_ITEM
	fp->write("u), ");
      }
    fp->write_decimal_unsigned(itemlen); //BITS_PER_ITEM
    fp->write("u, ");

    if(!stgcosut->isReference() && isLocal)
      {
	fp->write(stgcos->getMangledName().c_str()); //storage
	fp->write(", ");
      }

    if(cosut->isPrimitiveType())
      fp->write("NULL"); //eff self, not needed for classes since write isn't virtual
    else if(m_state.isAtom(cosuti))
      {
	fp->write("NULL"); //t3709, t3710
      }
    else if(!stgcosut->isReference())
      {
	fp->write("&");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(cosuti).c_str());
      }
    else if(classtype == UC_ELEMENT)
      {
	fp->write("NULL"); //element ref may rely on uc.Lookup (t3832)
      }
    else
      {
	//CLASSIC ref cannot rely on uc.Lookup
	//array's effective self is NULL. can't get it this way. (e.g. t3819)
	fp->write("&");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(cosuti).c_str());
      }

    fp->write(", ");
    fp->write(genUlamRefUsageAsString(scalarcosuti).c_str()); //t3230,1
    if(!stgcosut->isReference() && isLocal)
      fp->write(", uc");

    fp->write(")."); //close wrapper

    // the READ method
    fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());
    fp->write("();"); GCNL; // done read array item

    //update uvpass
    uvpass = UVPass::makePass(tmpVarNum2, cstor, scalarcosuti, m_state.determinePackable(scalarcosuti), m_state, 0, 0); //POS 0 rightjustified (atom-based).
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeReadArrayItemIntoTmp

  void Node::genCodeReadCustomArrayItemIntoATmpVar(File * fp, UVPass & uvpass)
  {
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data
    UTI vuti = uvpass.getPassTargetType(); //replaces vuti w target type
    assert(vuti != Void);

    //vut (index) may not be numeric when custom array
    //here, cos is symbol used to determine read method: either self or last of cos.
    //stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      cos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      cos = m_state.m_currentObjSymbolsForCodeGen.back();

    UTI cosuti = cos->getUlamTypeIdx();

    assert(isCurrentObjectACustomArrayItem(cosuti, uvpass));

    UTI itemuti = m_state.getAClassCustomArrayType(cosuti);

    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    UVPass auvpass = uvpass;
    genCodeConvertATmpVarIntoBitVector(fp, auvpass); //clears stack
    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore vector after args******

    u32 urtmpnum = 0;
    std::string hiddenarg2str = genHiddenArg2(uvpass, urtmpnum);
    if(urtmpnum > 0)
      {
	m_state.indentUlamCode(fp);
	fp->write(hiddenarg2str.c_str());
	fp->write("\n");
      }

    //non-const tmp ur for this function call
    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(localStorageTypeAsString(itemuti).c_str()); //e.g. BitVector<32> exception
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(itemuti, tmpVarNum2, TMPBITVAL).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genCustomArrayMemberNameOfMethod(fp);
	// the READ method
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());
	genCustomArrayHiddenArgs(fp, urtmpnum);
	fp->write(", "); //rest of arg's
      }
    else  //local var
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid
	//read method based on last cos
	genCustomArrayLocalMemberNameOfMethod(fp);
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());
	genCustomArrayHiddenArgs(fp, urtmpnum);
	fp->write(", "); //rest of args
      }
    //index is immediate Index arg of targettype in uvpass
    fp->write(auvpass.getTmpVarAsString(m_state).c_str()); //INDEX
    fp->write(");"); GCNL;

    //update uvpass
    uvpass = UVPass::makePass(tmpVarNum2, TMPBITVAL, itemuti, m_state.determinePackable(itemuti), m_state, 0, 0); //POS 0 rightjustified (atom-based).
    genCodeConvertABitVectorIntoATmpVar(fp, uvpass); //updates uvpass again (e.g. t3505 'is')
  } //genCodeReadCustomArrayItemIntoATmpVar

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
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI cosuti = cos->getUlamTypeIdx();

    // split if writing an array item or custom array item, here
    if(isCurrentObjectAnArrayItem(cosuti, luvpass))
      return genCodeWriteArrayItemFromATmpVar(fp, luvpass, ruvpass);

    if(isCurrentObjectACustomArrayItem(cosuti, luvpass))
      return genCodeWriteCustomArrayItemFromATmpVar(fp, luvpass, ruvpass); //like a func call

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
	genMemberNameOfMethod(fp);

	// the WRITE method
	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");
      }
    else
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid

	//local
	genLocalMemberNameOfMethod(fp);

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
    fp->write(m_state.getHiddenArgName()); //ur
    fp->write(".");
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
    genLocalMemberNameOfMethod(fp);

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
	genLocalMemberNameOfMethod(fp); //includes the dot
      }
    else
      {
	fp->write(luvpass.getTmpVarAsString(m_state).c_str());
	fp->write(".");
      }

    fp->write(writeMethodForCodeGen(luti, luvpass).c_str());
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
	genMemberNameOfMethod(fp);
	// the GetType WRITE method
	fp->write("ReadTypeField(");
      }
    else
      {
	//local
	genLocalMemberNameOfMethod(fp);
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
	genMemberNameOfMethod(fp);

	// the GetType WRITE method
	fp->write("WriteTypeField(");
      }
    else
      {
	//local
	genLocalMemberNameOfMethod(fp);
	fp->write("writeTypeField(");
      }

    fp->write(m_state.getTmpVarAsString(Unsigned, tmpVarType, TMPREGISTER).c_str());;
    fp->write("); //restore type"); GCNL;
    fp->write("\n");
  } //restoreElementTypeForAncestorCasting

  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteArrayItemFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass)
  {
    UTI ruti = ruvpass.getPassTargetType();
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    TMPSTORAGE rstor = ruvpass.getPassStorage();

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    ULAMCLASSTYPE cosclasstype = cosut->getUlamClassType();
    UTI	scalarcosuti = m_state.getUlamTypeAsScalar(cosuti); //ALT_ARRAYITEM
    scalarcosuti = m_state.getUlamTypeAsDeref(scalarcosuti);

    bool isLocal = isCurrentObjectALocalVariableOrArgument();
    assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid

    u32 itemlen = cosut->getBitSize();
    u32 pos = Node::calcPosOfCurrentObjects();

    //drive by UlamRef..
    m_state.indentUlamCode(fp);
    fp->write("UlamRef<EC>("); //wrapper for array item

    if(!isLocal)
      {
	//i.e. data member
	fp->write(m_state.getHiddenArgName()); //ur first arg
	fp->write(", ");
      }
    else if(stgcosut->isReference())
      {
	fp->write(stgcos->getMangledName().c_str()); //reference first
	fp->write(", ");
      }
    fp->write("("); //start index calc

    //still adding ATOM_FIRST_STATE_BIT to write the entire element, when cos is an element; uses ELEMENTAL //t3436, t3706, t3710
    if(needAdjustToStateBits(cosuti))
      fp->write("T::ATOM_FIRST_STATE_BIT + ");
    fp->write_decimal_unsigned(pos); //rel offset
    fp->write("u + ");
    fp->write(luvpass.getTmpVarAsString(m_state).c_str()); //INDEX
    fp->write(" * ");
    if((cosclasstype == UC_ELEMENT))
      fp->write("BPA), "); //t3670, t3841
    else
      {
	fp->write_decimal_unsigned(itemlen); //BITS_PER_ITEM
	fp->write("u), ");
      }
    fp->write_decimal_unsigned(itemlen); //BITS_PER_ITEM
    fp->write("u, ");

    if(!stgcosut->isReference() && isLocal)
      {
	fp->write(stgcos->getMangledName().c_str()); //storage
	fp->write(", ");
      }

    if(cosut->isPrimitiveType())
      fp->write("NULL"); //eff self, not needed for classes since write isn't virtual
    else if(m_state.isAtom(cosuti))
      {
	fp->write("NULL"); //3435
      }
    else if(!stgcosut->isReference())
      {
	fp->write("&");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(cosuti).c_str());
      }
    else if(cosclasstype == UC_ELEMENT)
      {
	fp->write("NULL"); //element ref may rely on uc.Lookup
      }
    else
      {
	//CLASSIC ref cannot rely on uc.Lookup
	//array's effective self is NULL. can't get it this way. (e.g. t3844)
	fp->write("&");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(cosuti).c_str());
      }

    fp->write(", ");
    fp->write(genUlamRefUsageAsString(scalarcosuti).c_str()); //t3230,1
    if(!stgcosut->isReference() && isLocal)
      fp->write(", uc");

    fp->write(")."); //close wrapper

    fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
    fp->write("(");
    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too! value not a terminal
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    if(rstor == TMPBITVAL)
      {
	fp->write(".read()"); //e.g. t3172
      }
    else if(rstor == TMPATOMBS)
      {
	fp->write(".");
	fp->write(rut->readMethodForCodeGen().c_str()); //ReadAtom
	fp->write("()");
      }
    fp->write(");"); GCNL;
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeWriteArrayItemFromATmpVar

  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteCustomArrayItemFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass)
  {
    //rhs could be a constant; or previously cast from Int to Unary variables.
    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine "hidden" args
    Symbol * cos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      cos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      cos = m_state.m_currentObjSymbolsForCodeGen.back();

    UTI cosuti = cos->getUlamTypeIdx();
    assert(isCurrentObjectACustomArrayItem(cosuti, luvpass));

    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    UVPass auvpass = luvpass;
    genCodeConvertATmpVarIntoBitVector(fp, auvpass); //clears stack
    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore vector after args******

    saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    assert(m_state.getUlamTypeByIndex(cosuti)->getUlamTypeEnum() == Class);
    UTI catype = m_state.getAClassCustomArrayType(cosuti);
    UVPass valuvpass = ruvpass;
    valuvpass.setPassTargetType(catype);
    genCodeConvertATmpVarIntoBitVector(fp, valuvpass); //clears stack
    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore vector after args******

    //non-const tmp ur for this function call
    u32 urtmpnum = 0;
    std::string hiddenarg2str = genHiddenArg2(luvpass, urtmpnum);
    if(urtmpnum > 0)
      {
	m_state.indentUlamCode(fp);
	fp->write(hiddenarg2str.c_str());
	fp->write("\n");
      }

    // a data member quark, or the element itself should both getBits from self;
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	m_state.indentUlamCode(fp);

	genCustomArrayMemberNameOfMethod(fp);
	// the WRITE method
	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	genCustomArrayHiddenArgs(fp, urtmpnum);
	fp->write(", "); //rest of args
      }
    else
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid
	//local
	m_state.indentUlamCode(fp);

	genCustomArrayLocalMemberNameOfMethod(fp);

	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());

	genCustomArrayHiddenArgs(fp, urtmpnum);
	fp->write(", "); //rest of args
      }

    //index is immediate Int arg
    fp->write(auvpass.getTmpVarAsString(m_state).c_str()); //INDEX
    fp->write(", ");

    //VALUE TO BE WRITTEN:
    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too!  value not a terminal
    // aset requires its custom array type (e.g. an atom) as its value:
    fp->write(valuvpass.getTmpVarAsString(m_state).c_str()); //VALUE
    fp->write(");"); GCNL;

    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeWriteCustomArrayItemFromATmpVar

  // write out intermediate tmpVar as temp BitVector, func args
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
    fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //VALUE

    u32 pos = uvpass.getPassPos(); //pos adjusted for Element stg in NodeIdent

    if(m_state.isAtom(vuti))
      {
	//for ANY immediate atom arg from a T
	//needs effective self from T's type
	if(m_state.isAtomRef(vuti))
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(pos); //Sun Jun 19 08:36:22 2016
	    fp->write("u, uc");
	  }
	//else nothing
      }
    else
      {
	if(vut->getUlamClassType() == UC_NOTACLASS)
	  {
	    //no longer atom-based primitives Thu Jun 23 15:51:15 2016
	    //pos = BITSPERATOM - vut->getTotalBitSize(); //right-justified atom-based
	    //pos = 0u;
	  }

	if(m_state.isReference(vuti))
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(pos); //position for constructor
	    fp->write("u");
	  }
      }
    fp->write("); //func arg&"); GCNL;

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
    fp->write(".read(");

    // use immediate read for entire array
    if(isCurrentObjectAnArrayItem(vuti, uvpass))
      {
	assert(0); //doesn't make sense for custom arrays?
	fp->write_decimal(uvpass.getPassLen()); //BITS_PER_ITEM
	fp->write("u, ");
	fp->write_decimal(adjustedImmediateArrayItemPassPos(vuti, uvpass)); //item POS (last like others) ?
	fp->write("u");
      }
    fp->write(");"); GCNL;

    uvpass = UVPass::makePass(tmpVarNum2, tmp2stor, vuti, m_state.determinePackable(vuti), m_state, 0, 0); //POS 0 rightjustified (atom-based).
    uvpass.setPassPos(0); //entire register
    m_state.clearCurrentObjSymbolsForCodeGen(); //forgotten?
  } //genCodeConvertABitVectorIntoATmpVar

  // write out auto ref tmpVar as temp BitVector; uvpass has variable w posOffset,
  // or array item index; uses stack for symbol; default is hidden arg.
  void Node::genCodeConvertATmpVarIntoAutoRef(File * fp, UVPass & uvpass)
  {
    UTI vuti = uvpass.getPassTargetType(); //offset or another autoref

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(cosSize == 0) //empty
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else if(cosSize == 1)
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    // write out auto ref constuctor
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    s32 pos = Node::calcPosOfCurrentObjects();

    // write out next chain using auto ref constuctor
    if(uvpass.getPassStorage() == TMPAUTOREF)
      {
	assert(m_state.isReference(vuti));
	m_state.indentUlamCode(fp);
	//can't be const and chainable
	fp->write(cosut->getLocalStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum2, TMPAUTOREF).c_str());
	fp->write("("); //use constructor (not equals)
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(", ");
	fp->write_decimal_unsigned(pos); //rel offset
	assert( pos == (s32) uvpass.getPassPos()); //Fri Jun 24 07:27:05 2016

	fp->write("u");
	fp->write(");"); GCNL;
      }
    else
      {
	//first array item, with item in uvpass
	assert(cosut->getUlamTypeEnum() == Class);
	assert(!cosut->isScalar());

	UTI scalarcosuti = m_state.getUlamTypeAsScalar(cosuti);
	UTI scalarrefuti = m_state.getUlamTypeAsRef(scalarcosuti, ALT_REF);
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
	    if(cosSize > 1)
	      {
		fp->write(stgcos->getMangledName().c_str()); //t3702
		stgisaref = stgcosut->isReference();
	      }
	    else
	      {
		fp->write(m_state.getHiddenArgName()); //t3543
		stgisaref = true;
	      }
	  }
	else
	  {
	    fp->write(cos->getMangledName().c_str()); //local array
	    stgisaref = cosut->isReference();
	  }

	fp->write(", ");
	fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //pos variable 0-based

	if((cosclasstype == UC_QUARK))
	  {
	    fp->write(" * ");
	    fp->write_decimal_unsigned(cosut->getBitSize());
	    fp->write("u");

	  }
	else if((cosclasstype == UC_ELEMENT))
	  {
	    fp->write(" * BPA"); //t3670

	    if(needAdjustToStateBits(cosuti))
	      fp->write(" + T::ATOM_FIRST_STATE_BIT"); //t3814
	  }
	else if((cosclasstype == UC_TRANSIENT)) //t3810
	  {
	    fp->write(" * ");
	    fp->write_decimal_unsigned(cosut->getBitSize());
	    fp->write("u");
	  }
	else
	  assert(0);

	if(cosSize > 0)
	  {
	    fp->write(" + ");
	    fp->write_decimal_unsigned(pos); //rel offset (t3512, t3543, t3648, t3702, t3776, t3668, t3811)
	    fp->write("u");
	  }

	fp->write(", &");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(scalarcosuti).c_str()); //t3495, t3512, t3543, t3648, t3702, t3776, t3668, t3811

	if(!stgisaref)
	  fp->write(", uc"); //t3512, t3543, t3648, t3702, t3776, t3668, t3811

	fp->write(");"); GCNL;

	cosuti = scalarrefuti; //for the uvpass
      }

    uvpass = UVPass::makePass(tmpVarNum2, TMPAUTOREF, cosuti, m_state.determinePackable(cosuti), m_state, 0, cos->getId()); //POS left-justified by default.
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCodeConvertATmpVarIntoAutoRef

  void Node::genCodeARefFromARefStorage(File * fp, UVPass stguvpass, UVPass uvpass)
  {
    // write out auto ref constuctor
    s32 tmpVarNum = stguvpass.getPassVarNum();

    UTI vuti = uvpass.getPassTargetType();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(m_state.isReference(vuti));

    UTI stgrefuti = stguvpass.getPassTargetType();
    assert(m_state.isReference(stgrefuti));

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

  //used both by NodeVarRef and NodeFunctionCall for reference args
  void Node::genCodeReferenceInitialization(File * fp, UVPass& uvpass, Symbol * vsymptr)
  {
    // lhs is third arg, vsymptr (m_varSymbol for NodeVarRef; tmprefsymbol for NodeFuncCall)
    // caller gets the right-hand side, stgcos
    // can be same type (e.g. class or primitive),
    // or ancestor quark if a class.

    UTI vuti = vsymptr->getUlamTypeIdx(); //i.e. this ref node
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    //Wed Jun 22 15:38:10 2016 atoms no longer a special case (t3484)

    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
      }

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    //do we need to check both stgcos && cos, or just cos?
    if(!cosut->isScalar() && !vut->isScalar())
      return genCodeArrayRefInit(fp, uvpass, vsymptr); //t3666

    if(!cosut->isScalar() && vut->isScalar())
      return genCodeArrayItemRefInit(fp, uvpass, vsymptr); //t3811

    ULAMTYPE vetyp = vut->getUlamTypeEnum();
    //assert(vetyp == cosetyp); t3684 vetyp is an atom, cosetyp is a class

    u32 pos = 0;

    m_state.indentUlamCode(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(vsymptr->getMangledName().c_str());
    fp->write("("); //pass ref in constructor (ref's not assigned with =)

    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	//WHY is this different than NodeFunctionCall::genCodeAnonymousReferenceArg??
	//local var (no currentObjSymbols, 1 arg since same type) e.g. t3617, t3779
	assert(UlamType::compare(uvpass.getPassTargetType(), vuti, m_state) == UTIC_SAME);
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(");"); GCNL;
      }
    else if(!m_state.isReference(stgcosuti))
      {
	pos = Node::calcPosOfCurrentObjects();
	assert(pos == uvpass.getPassPos()); //Fri Jun 24 07:27:05 2016

	if(stgcos->isDataMember()) //may be an element/atom in a TRANSIENT
	  fp->write(m_state.getHiddenArgName());
	else
	  fp->write(stgcos->getMangledName().c_str());
	fp->write(", ");

	if(needAdjustToStateBits(cosuti))
	  fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3803
	fp->write_decimal_unsigned(pos); //rel offset
	fp->write("u");
	if(vetyp == Class)
	  {
	    fp->write(", &"); //left just
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(vuti).c_str());
	  }

	if(!stgcos->isDataMember()) //technically stg is self (ur) if dm, so no uc (t3695)
	  fp->write(", uc"); //t3820, t3613, t3615
	fp->write(");"); GCNL;
      }
    else
      {
	//a reference
	pos = Node::calcPosOfCurrentObjects();
	assert(pos == uvpass.getPassPos()); //Sat Jun 25 12:13:43 2016

	fp->write(stgcos->getMangledName().c_str());
	if(vetyp == Class) //also not an atom
	  {
	    fp->write(", ");
	    if(needAdjustToStateBits(cosuti))
	      fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3819
	    fp->write_decimal_unsigned(pos); //rel offset t3819
	    fp->write("u, ");
	    if(cos->isDataMember())
	      {
		fp->write("&");
		fp->write(m_state.getEffectiveSelfMangledNameByIndex(vuti).c_str());
	      }
	    else
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".GetEffectiveSelf()");
	      }
	  }
	else if(vetyp == UAtom)
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(pos); //rel offset
	    fp->write("u"); //t3820
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

    // get the right-hand side, stgcos
    // can be same type (e.g. element, quark, or primitive),
    // or ancestor quark if a class.
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    UTI vuti = vsymptr->getUlamTypeIdx(); //i.e. this ref node
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(!vut->isScalar()); //entire array

    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();
    ULAMTYPE vetyp = vut->getUlamTypeEnum();
    assert(vetyp == cosut->getUlamTypeEnum());

    u32 pos = Node::calcPosOfCurrentObjects(); //t3832
    //u32 pos = uvpass.getPassPos(); //testing bug with Stale TYPE (t3832)

    m_state.indentUlamCode(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(vsymptr->getMangledName().c_str());
    fp->write("("); //pass ref in constructor (ref's not assigned with =)

    if(stgcos->isDataMember()) //rhs may be an element/atom in a transient; not a reference
      {
	fp->write(m_state.getHiddenArgName());
	fp->write(", ");
	fp->write_decimal_unsigned(pos); //relative off array base
	fp->write("u");

	if(vetyp == Class)
	  {
	    if(stgcosut->isScalar() && needAdjustToStateBits(stgcosuti))
	      fp->write(" + T::ATOM_FIRST_STATE_BIT"); //?
	    assert(!cosut->isReference());
	    fp->write(", NULL");
	  }
	fp->write(");"); GCNL;
      }
    else
      {
	//local stg
	fp->write(stgcos->getMangledName().c_str());
	if(cos->isDataMember())
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(pos); //rel offset array base (t3832)

	    fp->write("u");

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
		fp->write(", 0u");
	      }
	    else if(vetyp == Class)
	      {
		if(!stgcosut->isReference())
		  {
		    //effective self for arrays is NULL
		    fp->write(", 0u, NULL"); //t3670 3668, t3669, t3670
		  }
	      }
	    else
	      assert(0);
	  }
	if(!stgcosut->isReference())
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
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    UTI vuti = vsymptr->getUlamTypeIdx(); //i.e. this ref node
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(vut->isScalar()); //item of array

    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();
    ULAMTYPE vetyp = vut->getUlamTypeEnum();
    assert(vetyp == cosut->getUlamTypeEnum());

    bool isLocal = isCurrentObjectALocalVariableOrArgument();
    u32 pos = Node::calcPosOfCurrentObjects(); //array base rel offset; uvpass is index (e.g. t3653, t3654, t3810, t3811, t3812)

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
    if(needAdjustToStateBits(cosuti))
      fp->write("T::ATOM_FIRST_STATE_BIT + "); //t3816
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
	if(!stgcosut->isReference())
	  {
	    fp->write(", &"); //left just
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(vuti).c_str());
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
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(cosuti).c_str());
	  }
      }

    if(!stgcosut->isReference() && isLocal)
      fp->write(", uc"); //t3818, t3653, t3654

    fp->write(");"); GCNL;
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
  } //genCodeArrayItemRefInit

  void Node::genCodeExtern(File * fp, bool declOnly)
  {
    // no externs
  }

  void Node::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    assert(0); //fufilled by NodeVarDecl, NodeBlock; bypassed by NodeTypedef and NodeConstDef
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
	if(m_state.isAtom(nuti) && !((tclasstype == UC_ELEMENT) || m_state.isAtom(tobeType) || ((tclasstype == UC_QUARK) && tobe->isReference())))
	  doErrMsg = true;

	else if(nuti == Void)
	  doErrMsg = true; //cannot cast a void into anything else (reverse is fine)
	//else if reference to non-ref of same type?
	else
	  {
	    assert(!isExplicit);
	    doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
	  }
      }
    else if(nclasstype == UC_QUARK)
      {
	if(node->isFunctionCall())
	  {
	    if(tobe->isReference())
	      doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
	    else
	      {
		// a function call is not a valid lhs !!!
		// 'node' is a function call that returns a quark (it's not storeintoable);
		// build a toIntHelper function that takes the return value of 'node'
		// as its arg and returns toInt
		doErrMsg = buildCastingFunctionCallNode(node, tobeType, rtnNode);
	      }
	  }
	else if(tclasstype != UC_NOTACLASS)
	  {
	    //handle possible inheritance (u.1.2.2) here
	    if(m_state.isClassASubclassOf(nuti, tobeType))
	      doErrMsg = newCastingNodeWithCheck(node, tobeType, rtnNode);
	    else if(m_state.isClassASubclassOf(tobeType, nuti))
	      {
		//ok to cast a quark ref to an element, if its superclass
		if(!m_state.isReference(nuti))
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
	else
	  {
	    rtnNode = buildToIntCastingNode(node);

	    //redo check and type labeling; error msg if not same
	    UTI newType = rtnNode->checkAndLabelType();
	    doErrMsg = (UlamType::compareForMakingCastingNode(newType, tobeType, m_state) == UTIC_NOTSAME);
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
	    m_state.setGoAgain();
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
  } //makecastingnode

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

    //negative indicates parameter for symbol install
    m_state.m_currentFunctionBlockDeclSize = -3; //slots for return + 1 hidden arg self(+ uc)
    m_state.m_currentFunctionBlockMaxDepth = 0;

    SymbolVariableStack * selfsym = new SymbolVariableStack(selfTok, UAtom, m_state.m_currentFunctionBlockDeclSize, m_state);
    selfsym->setIsSelf();
    m_state.addSymbolToCurrentScope(selfsym); //ownership goes to the fblock

    UTI nodeType = node->getNodeType(); //quark type..
    UlamType * nut = m_state.getUlamTypeByIndex(nodeType);
    assert(nut->getUlamClassType() == UC_QUARK);
    u32 quid = nut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId();
    Token typeTok(TOK_TYPE_IDENTIFIER, loc, quid);

    TypeArgs typeargs;
    typeargs.init(typeTok);
    typeargs.m_bitsize = nut->getBitSize();
    typeargs.m_arraysize = nut->getArraySize();
    typeargs.m_classInstanceIdx = nodeType;

    u32 argid = m_state.m_pool.getIndexForDataString("arg");
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
    ((SymbolVariableStack *) argSym)->setDeclNodeNo(argIdentNode->getNodeNo());

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
	//starts with positive one for local variables
	m_state.m_currentFunctionBlockDeclSize = 1;
	m_state.m_currentFunctionBlockMaxDepth = 0;

	/* like a typical quark toInt cast expression */
	Node * mselectNode = buildToIntCastingNode(argIdentNode);
	//the body: single statement return arg.toInt();
	NodeReturnStatement * returnNode =  new NodeReturnStatement(mselectNode, m_state);
	assert(returnNode);
	returnNode->setNodeLocation(loc);

	NodeStatements * sNode = new NodeStatements(returnNode, m_state);
	assert(sNode);
	sNode->setNodeLocation(loc);

	fblock->setNextNode(sNode);
	fblock->setDefinition();
	fblock->setMaxDepth(0); //no local variables, except params
      }

    //this block's ST is no longer in scope
    m_state.popClassContext(); //= prevBlock;

    m_state.m_currentFunctionBlockDeclSize = 0; //default zero for datamembers
    m_state.m_currentFunctionBlockMaxDepth = 0; //reset

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

  void Node::genMemberNameOfMethod(File * fp, bool endingdot)
  {
    assert(!isCurrentObjectALocalVariableOrArgument());

    //nothing if current object is self, unless a subclass!!!
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      return;

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    UTI cosclassuti = cos->getDataMemberClass();
    UlamType * cosclassut = m_state.getUlamTypeByIndex(cosclassuti);
    assert(!cosclassut->isReference());

    u32 pos = Node::calcPosOfCurrentObjects();
    fp->write("UlamRef<EC>("); //wrapper for dm
    fp->write(m_state.getHiddenArgName()); //ur first arg
    fp->write(", ");
    fp->write_decimal_unsigned(pos); //rel offset
    fp->write("u, ");
    fp->write_decimal_unsigned(cosut->getTotalBitSize());
    fp->write("u, ");
    if(cosut->getUlamTypeEnum() == Class)
      {
	fp->write("&");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(cosclassuti).c_str()); //t3175
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
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(stgcosuti).c_str());
	fp->write(".");
      }
    else
      assert(0);

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

    fp->write(m_state.getEffectiveSelfMangledNameByIndex(stgcosuti).c_str());
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
    assert(!caclassut->isReference());
    fp->write(m_state.getEffectiveSelfMangledNameByIndex(caclassuti).c_str());
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

    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    // first "hidden" arg is the context, then
    //"hidden" ref self (ur) arg
    if(!Node::isCurrentObjectALocalVariableOrArgument())
      {
	if(m_state.m_currentObjSymbolsForCodeGen.empty())
	  hiddenarg2 << m_state.getHiddenArgName(); //same ur
	else if(stgcos->isSelf() && (stgcos == cos)) //t3831, t3274, t3275
	  {
	    hiddenarg2 << m_state.getHiddenArgName(); //same ur
	  }
	else
	  {
	    sameur = false;
	    hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";
	    //update ur to reflect "effective" self for this funccall
	    hiddenarg2 << m_state.getHiddenArgName(); //ur
	    hiddenarg2 << ", " << Node::calcPosOfCurrentObjectClassesAsString(uvpass); //relative off;
	    hiddenarg2 << ", " << cosut->getTotalBitSize() << "u, &"; //len

	    hiddenarg2 << m_state.getEffectiveSelfMangledNameByIndex(cosuti).c_str(); //cos->isSuper rolls as cosuti
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
	    assert(0);
	    //hiddenarg2 << genModelParameterHiddenArgs(epi).c_str();
	  }
	else //local var
	  {
	    if(m_state.m_currentObjSymbolsForCodeGen.empty())
	      {
		hiddenarg2 << m_state.getHiddenArgName(); //same ur
	      }
	    else if(stgcosut->isReference()) //t3751, t3752, (ALT_AS: t3249, t3255)
	      {
		sameur = false;
		//update ur to reflect "effective" self for this funccall
		hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";
		hiddenarg2 << stgcos->getMangledName().c_str() << ", ";

		if(cos->isDataMember()) //dm of local stgcos
		  {
		    hiddenarg2 << Node::calcPosOfCurrentObjectClassesAsString(uvpass); //relative off;
		    hiddenarg2 << ", " << cosut->getTotalBitSize() << "u, "; //len

		    hiddenarg2 << "&"; //effective self of dm (t3804 check -10)
		    hiddenarg2 << m_state.getEffectiveSelfMangledNameByIndex(cosuti).c_str();
		    hiddenarg2 << ", " << genUlamRefUsageAsString(cosuti).c_str();
		    hiddenarg2 << ");";
		  }
		else
		  {
		    //ancestor takes on effective self of sub
		    //uses UlamRef 2 arg copy constructor to maintain EffSelf and UsageType of ref
		    hiddenarg2 << cosut->getTotalBitSize() << "u);" ; //len t3637, t3746
		  }
	      }
	    else
	      {
		sameur = false;
		//new ur to reflect "effective" self and storage for this funccall
		hiddenarg2 << "UlamRef<EC> " << m_state.getUlamRefTmpVarAsString(tmpvar).c_str() << "(";

		hiddenarg2 << Node::calcPosOfCurrentObjectClassesAsString(uvpass); //relative off;
		hiddenarg2 << ", " << cosut->getTotalBitSize() << "u, "; //len

		hiddenarg2 << stgcos->getMangledName().c_str() << ", &"; //storage
		hiddenarg2 << m_state.getEffectiveSelfMangledNameByIndex(cosuti).c_str();

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

  void Node::genLocalMemberNameOfMethod(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());
    // model parameter has its own storage, like a local
    assert(isCurrentObjectsContainingAModelParameter() == -1);
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    if(cosSize > 1)
      return genLocalMemberNameOfMethodByUsTypedef(fp);

    assert(cosSize == 1);
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[0];
    fp->write(cos->getMangledName().c_str());
    fp->write(".");
    return;
  } //genLocalMemberNameOfMethod

  void Node::genLocalMemberNameOfMethodByUsTypedef(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());
    // model parameter has its own storage, like a local
    assert(isCurrentObjectsContainingAModelParameter() == -1);
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    assert(cosSize > 1); //see caller
    //therefore cos must be a data member!!
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    // handle inheritance, when data member is in superclass, not current class obj
    // now for both immediate elements and quarks..
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    //cosclass is related to type left of its dot
    UTI cosclassuti = cos->getDataMemberClass();
    assert(cosclassuti != Nouti);
    UlamType * cosclassut = m_state.getUlamTypeByIndex(cosclassuti);
    assert(!cosclassut->isReference());

    assert(cosclassut->isScalar()); //cos array (t3147-48)

    u32 pos = Node::calcPosOfCurrentObjects();

    fp->write("UlamRef<EC>("); //wrapper for local storage
    if(stgcosut->isReference())
      {
	fp->write(stgcos->getMangledName().c_str()); //ref
	fp->write(", ");
      }

    //reading entire thing, using ELEMENTAL, t.f. adjust (t3735)
    if(needAdjustToStateBits(cosuti))
      fp->write("T::ATOM_FIRST_STATE_BIT + ");

    fp->write_decimal_unsigned(pos); //rel offset
    fp->write("u, ");
    if((stgcosut->getUlamClassType() == UC_TRANSIENT) && (cosut->getUlamClassType() == UC_ELEMENT))
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

    if(!stgcosut->isReference())
      {
	fp->write(stgcos->getMangledName().c_str()); //local storage (not ref)
	fp->write(", ");
      }

    if(cosut->getUlamTypeEnum() == Class)
      {
	//cos is a data member
	fp->write("&");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(cosclassuti).c_str());
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

  void Node::genCustomArrayLocalMemberNameOfMethod(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    // handle inheritance, when data member is in superclass, not current class obj
    // now for both immediate elements and quarks..
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    if(cosut->isReference())
      cosuti = m_state.getUlamTypeAsDeref(cosuti); //search for funcid

    Symbol * fnsymptr = NULL;
    bool hazyKin = false;
    AssertBool isDefinedFunc = m_state.isFuncIdInAClassScope(cosuti, m_state.getCustomArrayGetFunctionNameId(),fnsymptr, hazyKin); //searches class of cos
    assert(isDefinedFunc);
    assert(!hazyKin);

    UTI futi = fnsymptr->getDataMemberClass();
    fp->write(m_state.getEffectiveSelfMangledNameByIndex(futi).c_str());
    fp->write(".");
  } //genCustomArrayLocalMemberNameOfMethod

  const std::string Node::localStorageTypeAsString(UTI nuti)
  {
    return m_state.getUlamTypeByIndex(nuti)->getLocalStorageTypeAsString();
  } //localStorageTypeAsString

  const std::string Node::tmpStorageTypeForRead(UTI nuti, UVPass uvpass)
  {
    return m_state.getUlamTypeByIndex(nuti)->getTmpStorageTypeAsString();
  } //tmpStorageTypeForRead

  const std::string Node::tmpStorageTypeForReadArrayItem(UTI nuti, UVPass uvpass)
  {
    return m_state.getUlamTypeByIndex(nuti)->getArrayItemTmpStorageTypeAsString();
  } //tmpStorageTypeForReadArrayItem

  const std::string Node::readMethodForCodeGen(UTI nuti, UVPass uvpass)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    std::string method;
    if(isCurrentObjectsContainingAModelParameter() >= 0)
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
    if(!isCurrentObjectALocalVariableOrArgument())
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
      method = m_state.getCustomArraySetMangledFunctionName();
    else
      {
	//packed and unpacked arrays both
	UTI scalarnuti = m_state.getUlamTypeAsScalar(nuti);
	method = m_state.getUlamTypeByIndex(scalarnuti)->writeMethodForCodeGen(); //for UlamRef
      }
    return method;
  } //writeArrayItemMethodForCodeGen

  bool Node::isCurrentObjectALocalVariableOrArgument()
  {
    //include model parameters as LocalVariableOrArgument, since more alike;
    //an element's "self" as obj[0] is like it isn't there for purposes of this discovery.
    //quark's self is an atom or immediate, and should be treated like a local arg.
    // notes: self is not a data member. references cannot be data members.
    //        func def's are dm, but func calls are not.
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      return false; //must be self, t.f. not local

    s32 modelparamidx = isCurrentObjectsContainingAModelParameter();
    if(m_state.m_currentObjSymbolsForCodeGen[0]->isDataMember() && (modelparamidx == -1))
      return false; //data member, not model parameter

    UTI stgcosuti = m_state.m_currentObjSymbolsForCodeGen[0]->getUlamTypeIdx();
    if(m_state.m_currentObjSymbolsForCodeGen[0]->isSelf() && !(m_state.isAtom(stgcosuti) || m_state.getUlamTypeByIndex(stgcosuti)->getUlamClassType() == UC_TRANSIENT) && (modelparamidx == -1))
      return false; //self, neither atom nor transient, not modelparameter

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
  } //isCurrentObjectsContainingAModelgParameter

  // used by genHiddenArg2 for function calls; uvpass may contain the index
  // of an array item, o.w. the current arg's tmp var (unneeded here).
  std::string Node::calcPosOfCurrentObjectClassesAsString(UVPass uvpass)
  {
    s32 pos = calcPosOfCurrentObjectClasses();
    std::ostringstream posStr;
    posStr << pos << "u";
    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
	UTI cosuti = cos->getUlamTypeIdx();
	UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
	ULAMCLASSTYPE classtype = cosut->getUlamClassType();
	//last one is element and not a ref, add offset to state bits;
	// unlike for read/write element.
	if(needAdjustToStateBits(cosuti))
	  {
	    posStr << " + T::ATOM_FIRST_STATE_BIT";
	  }
	if((classtype == UC_ELEMENT) && !cosut->isScalar())
	  {
	    posStr << " + T::BPA * " << uvpass.getTmpVarAsString(m_state);
	  }
      }
    return posStr.str();
  } //calcPosOfCurrentObjectClassesAsString

  // returns accumulated relative positions of data members
  s32 Node::calcPosOfCurrentObjectClasses()
  {
    return calcPosOfCurrentObjects(true);
  } //calcPosOfCurrentObjectClasses

  // returns accumulated relative positions of data members
  s32 Node::calcPosOfCurrentObjects(bool onlyClasses)
  {
    // default for onlyClasses is false;
    // self must be subclass, i.e. inherited quark always at pos 0
    if(m_state.m_currentObjSymbolsForCodeGen.empty()) return 0;
    s32 cosSize = (s32) m_state.m_currentObjSymbolsForCodeGen.size();

    u32 pos = 0;
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    if((cosSize > 1) && !stgcos->isSelf() && needAdjustToStateBits(stgcosuti))
      pos = 25u; //skip Type (e.g. t3262), not for self ref (t3408), not ele ref (t3637)

    for(s32 i = 0; i < cosSize; i++)
      {
    	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	assert(sym);
	if(sym->isDataMember() && !sym->isFunction())
	  {
	    UTI suti = sym->getUlamTypeIdx();
	    UlamType * sut = m_state.getUlamTypeByIndex(suti);
	    if(!onlyClasses || (sut->getUlamTypeEnum() == Class))
	      {
		pos += ((SymbolVariableDataMember *) sym)->getPosOffset();
		if((sut->getUlamClassType() == UC_ELEMENT) && (i < cosSize - 1))
		  {
		    //dm in transient; not the last one being written to.
		    pos += ATOMFIRSTSTATEBITPOS;
		    assert(sut->isScalar()); //atom-based, skip an atom
		  }
	      }
	  }
      }
    return pos;
  } //calcPosOfCurrentObjects

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
    return(m_state.isScalar(cosuti) && m_state.isClassACustomArray(cosuti) && uvpass.getPassTargetType() != cosuti);
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

  //seems to be unused (convertABitVectorIntoATmpVar for custom array)
  u32 Node::adjustedImmediateArrayItemPassPos(UTI cosuti, UVPass uvpass)
  {
    u32 pos = uvpass.getPassPos();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    if(cosut->getUlamClassType() == UC_NOTACLASS)
      {
	assert(!m_state.isAtom(cosuti)); //atom too? let's find out..
	u32 wordsize = cosut->getTotalWordSize();
	pos = wordsize - (BITSPERATOM - pos); //cosut->getTotalBitSize();
      }
    return pos;
  } //adjustedImmediateArrayItemPassPos

  bool Node::needAdjustToStateBits(UTI cuti)
  {
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    if(cut->getUlamClassType() != UC_ELEMENT)
      return false;
    if(!cut->isScalar())
      return true;
    if(cut->isReference())
      return false;
    return true;
  } //needAdjustToStateBits

  SymbolTmpRef * Node::makeTmpRefSymbolForCodeGen(UVPass uvpass)
  {
    UTI tuti = uvpass.getPassTargetType(); //possibly not a ref, e.g. array item.
    std::string tmpvarname = m_state.getTmpVarAsString(tuti, uvpass.getPassVarNum(), TMPAUTOREF);
    Token tidTok(TOK_IDENTIFIER, Node::getNodeLocation(), m_state.m_pool.getIndexForDataString(tmpvarname));
    SymbolTmpRef * rtnsym = new SymbolTmpRef(tidTok, tuti, m_state);
    assert(rtnsym);
    rtnsym->setAutoLocalType(m_state.getReferenceType(tuti));
    assert(rtnsym);
    return rtnsym;
  } //makeTmpRefSymbolForCodeGen

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
	  assert(0);
      }
    return usageStr.str();
  } //genUlamRefUsageAsString

} //end MFM
