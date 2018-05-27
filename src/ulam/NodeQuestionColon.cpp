#include <stdio.h>
#include "NodeQuestionColon.h"
#include "CompilerState.h"
#include "UlamTypePrimitiveBool.h"

namespace MFM {

  NodeQuestionColon::NodeQuestionColon(Node * condNode, Node * trueNode, Node * falseNode, CompilerState & state): NodeBinaryOp(trueNode, falseNode, state), m_nodeCondition(condNode) {}

  NodeQuestionColon::NodeQuestionColon(const NodeQuestionColon& ref) : NodeBinaryOp(ref)
  {
    m_nodeCondition = ref.m_nodeCondition->instantiate();
  }

  NodeQuestionColon::~NodeQuestionColon()
  {
    delete m_nodeCondition;
    m_nodeCondition = NULL;
  }

  Node * NodeQuestionColon::instantiate()
  {
    return new NodeQuestionColon(*this);
  }

  void NodeQuestionColon::updateLineage(NNO pno)
  {
    m_nodeCondition->updateLineage(getNodeNo());
    NodeBinaryOp::updateLineage(pno);
  }

  bool NodeQuestionColon::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeCondition == oldnptr)
      {
	m_nodeCondition = newnptr;
	return true;
      }
    return NodeBinaryOp::exchangeKids(oldnptr,newnptr);
  } //exhangeKids

  bool NodeQuestionColon::findNodeNo(NNO n, Node *& foundNode)
  {
    if(m_nodeCondition->findNodeNo(n, foundNode))
      return true;
    return NodeBinaryOp::findNodeNo(n,foundNode);
  } //findNodeNo

  void NodeQuestionColon::checkAbstractInstanceErrors()
  {
    m_nodeCondition->checkAbstractInstanceErrors();
    NodeBinaryOp::checkAbstractInstanceErrors();
  }

  void NodeQuestionColon::resetNodeLocations(Locator loc)
  {
    m_nodeCondition->resetNodeLocations(loc);
    NodeBinaryOp::resetNodeLocations(loc);
  }

  void NodeQuestionColon::print(File * fp)
  {
    assert(m_nodeCondition && m_nodeLeft && m_nodeRight);

    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n",prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    fp->write("condition:\n");
    m_nodeCondition->print(fp);

    fp->write(" ? ");
    m_nodeLeft->print(fp);
    fp->write(" : ");
    m_nodeRight->print(fp);
  } //print

  void NodeQuestionColon::printPostfix(File * fp)
  {
    assert(m_nodeCondition && m_nodeLeft && m_nodeRight);
    m_nodeCondition->printPostfix(fp);

    fp->write(" ? ");

    m_nodeLeft->printPostfix(fp);

    fp->write(" : ");

    m_nodeRight->printPostfix(fp);

  } //printPostfix

  void NodeQuestionColon::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname,"%s", getName());
    fp->write(myname);
  } //printOp

  const std::string NodeQuestionColon::methodNameForCodeGen()
  {
    m_state.abortShouldntGetHere();
    return "NoMethodNameForQuestionColon";
  }

  const char * NodeQuestionColon::getName()
  {
    return "?:";
  }

  const std::string NodeQuestionColon::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeQuestionColon::isAConstant()
  {
    return m_nodeLeft->isAConstant() || m_nodeRight->isAConstant();
  }

  UTI NodeQuestionColon::calcNodeType(UTI lt, UTI rt)
  {
    if(!m_state.neitherNAVokUTItoContinue(lt, rt))
      return Nav;

    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
      return Hzy; //short-circuit

    if(UlamType::compare(lt, rt, m_state) == UTIC_SAME)
      return lt; //incl classes, atoms, refs, void, strings, arrays

    //if different non-primitive types require explicit casting:
    //atom-quark,atom-element,element-quark,quark-quark',transient-quark
    if(!NodeBinaryOp::checkForPrimitiveTypes(lt,rt,true))
      {
	//neither same, nor primitive (or void), requires explicit cast
	std::ostringstream msg;
	msg << "Use explicit cast to convert ";
	msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	msg << " and ";
	msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	msg << " to be same type for binary " << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    UTI newType = Nav; //init
    ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
    ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

    //if either is Bits, cast to Bits
    if(ltypEnum == Bits || rtypEnum == Bits)
      {
	if(NodeBinaryOp::checkScalarTypesOnly(lt, rt, true))
	  {
	    s32 lbs = UNKNOWNSIZE, rbs = UNKNOWNSIZE, wordsize = UNKNOWNSIZE;
	    NodeBinaryOp::resultBitsizeCalcInBits(lt, rt, lbs, rbs, wordsize);
	    s32 newbs = (lbs > rbs ? lbs : rbs);

	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bits"), newbs);
	    newType = m_state.makeUlamType(newkey, Bits, UC_NOTACLASS);
         }
	//else array of Bits with different bit sizes-- Nav.
	return newType; //done
      }

    //o.w. revert to ordered comparison rules
    //return NodeBinaryOpCompare::calcNodeType(lt,rt);
    //what if different bit sizes? take the larger
    if(NodeBinaryOp::checkScalarTypesOnly(lt, rt, true))
      {
	s32 newbs = NodeBinaryOp::resultBitsize(lt, rt); //default
	ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

	if(ltypEnum == rtypEnum)
	  {
	    //use same base type
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(ltypEnum)), newbs);
	    newType = m_state.makeUlamType(newkey, ltypEnum, UC_NOTACLASS);
	  }
	else
	  {
	    // treat Unary using Unsigned rules
	    if(ltypEnum == Unary)
	      ltypEnum = Unsigned;

	    if(rtypEnum == Unary)
	      rtypEnum = Unsigned;

	    if(ltypEnum == Unsigned && rtypEnum == Unsigned)
	      {
		UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Unsigned"), newbs);
		newType = m_state.makeUlamType(newkey, Unsigned, UC_NOTACLASS);
	      }
	    else
	      {
		UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Int"), newbs);
		newType = m_state.makeUlamType(newkey, Int, UC_NOTACLASS);
	      }
	  }
      }
    //else don't cast arrays!
    return newType;
  } //calcNodeType

  UTI NodeQuestionColon::checkAndLabelType()
  {
    assert(m_nodeCondition && m_nodeLeft && m_nodeRight);

    // condition should be a bool, safely cast
    UTI condType = m_nodeCondition->checkAndLabelType();
    UTI newcondtype = Bool;

    if(m_state.okUTItoContinue(condType) && m_state.isComplete(condType))
      {
	assert(m_state.isScalar(condType));
	UlamType * cut = m_state.getUlamTypeByIndex(condType);
	ULAMTYPE ctypEnum = cut->getUlamTypeEnum();
	if(ctypEnum != Bool)
	  {
	    if(NodeBinaryOp::checkSafeToCastTo(condType, newcondtype))
	      {
		if(!Node::makeCastingNode(m_nodeCondition, Bool, m_nodeCondition))
		  newcondtype = Nav;
	      }
	    //else //not safe, newcondType changed
	  }
	else
	  {
	    //caution: c&l called multiple times
	    //always cast: Bools are maintained as unsigned in gen code,
	    //until c-bool is needed
	    if(condType != Bool)
	      {
		if(m_nodeCondition->safeToCastTo(newcondtype) == CAST_CLEAR)
		  {
		    if(!Node::makeCastingNode(m_nodeCondition, newcondtype, m_nodeCondition))
		      newcondtype = Nav;
		  }
		//else //not safe, newcondType changed
	      }
	  }
      }
    else
      {
	newcondtype = Hzy;
      }

    UTI trueType = m_nodeLeft->checkAndLabelType(); //side-effect

    UTI falseType = m_nodeRight->checkAndLabelType(); //side-effect

    UTI newType = calcNodeType(trueType, falseType);

    if(!m_state.okUTItoContinue(newcondtype))
      newType = newcondtype;

    if(m_state.okUTItoContinue(newType))
      {
	if(UlamType::compareForMakingCastingNode(trueType, newType, m_state) == UTIC_NOTSAME)
	  {
	    if(NodeBinaryOp::checkSafeToCastTo(trueType, newType)) //Nav, Hzy or no change; outputs error msg
	      {
		if(!Node::makeCastingNode(m_nodeLeft, newType, m_nodeLeft, false))
		  newType = Nav;
	      }
	  }
	if(m_state.okUTItoContinue(newType) && (UlamType::compareForMakingCastingNode(falseType, newType, m_state) == UTIC_NOTSAME))
	  {
	    if(NodeBinaryOp::checkSafeToCastTo(falseType, newType)) //Nav, Hzy or no change; outputs error msg
	      {
		if(!Node::makeCastingNode(m_nodeRight, newType, m_nodeRight, false))
		  newType = Nav;
	      }
	  }
      }

    setNodeType(newType);  //stays the same
    if(newType == Hzy)
      m_state.setGoAgain();

    Node::setStoreIntoAble(isAConstant() ? TBOOL_FALSE : TBOOL_TRUE);

    if(m_state.okUTItoContinue(newType) && m_nodeCondition->isAConstant())
      {
	return constantFold();
      }
    return newType;
  } //checkAndLabelType

  void NodeQuestionColon::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    assert(m_nodeCondition && m_nodeLeft && m_nodeRight);
    m_nodeCondition->calcMaxDepth(depth, maxdepth, base); //function call?

    u32 maxtrue = depth;
    m_nodeLeft->calcMaxDepth(maxtrue, maxdepth, base);

    u32 maxfalse = depth;
    m_nodeRight->calcMaxDepth(maxfalse, maxdepth, base);

    depth += maxtrue > maxfalse ? maxtrue : maxfalse;
  } //calcMaxDepth

  void NodeQuestionColon::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    assert(m_nodeCondition && m_nodeLeft && m_nodeRight);
    m_nodeCondition->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    return NodeBinaryOp::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  UTI NodeQuestionColon::constantFold()
  {
    if(!m_nodeCondition->isAConstant())
      return Nav; //t41059

    //if(!m_nodeCondition->isReadyConstant())
    //  return Hzy;

    bool condbool = false;

    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(Bool);
    EvalStatus evs = m_nodeCondition->eval();
    if(evs == NORMAL)
      {
	UlamValue cuv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
	condbool = (bool) cuv.getImmediateData(m_state);
      }

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for binary op" << getName();
	msg << " is erroneous while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    if(evs == NOTREADY)
      {
	std::ostringstream msg;
	msg << "Constant value expression for binary op" << getName();
	msg << " is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	setNodeType(Hzy);
	m_state.setGoAgain(); //for compiler counts
	return Hzy;
      }

    Node * newnode = NULL;
    if(condbool == false)
      {
	newnode = m_nodeRight;
	m_nodeRight = NULL; //recycling
      }
    else
      {
	newnode = m_nodeLeft;
	m_nodeLeft = NULL; //recycling
      }

    AssertBool isSwap = Node::exchangeNodeWithParent(newnode);
    assert(isSwap);

    delete this; //suicide is painless..

    return newnode->getNodeType(); //already known
  } //constantFold

  EvalStatus NodeQuestionColon::eval()
  {
    assert(m_nodeCondition && m_nodeLeft && m_nodeRight);

    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0); //new current frame pointer

    makeRoomForNodeType(Bool);
    EvalStatus evs = m_nodeCondition->eval();
    if(evs != NORMAL) return evalStatusReturn(evs);//what if RETURN

    makeRoomForNodeType(nuti);

    UlamValue cuv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
    if((bool) cuv.getImmediateData(m_state) == false)
      {
	evs = m_nodeRight->eval();
      }
    else
      {
	evs = m_nodeLeft->eval();
      }

    if(evs != NORMAL) return evalStatusReturn(evs);

    if(nuti != Void)
      {
	UlamValue auv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(2);

	// unlike if-else, the type of this node is the result that must be
	// the same type/size. the type of the condition is Bool.
	//Results need to be stored in a variable somewhere.

	//also copy result UV to stack, -1 relative to current frame pointer
	Node::assignReturnValueToStack(auv);
      }

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeQuestionColon::evalToStoreInto()
  {
    //t41071, nuti not ALT_REF; comes from cast to ref
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0);

    makeRoomForNodeType(Bool);
    EvalStatus evs = m_nodeCondition->eval();
    if(evs != NORMAL) return evalStatusReturn(evs);

    makeRoomForNodeType(nuti);

    UlamValue cuv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);
    if((bool) cuv.getImmediateData(m_state) == false)
      {
	evs = m_nodeRight->evalToStoreInto();
      }
    else
      {
	evs = m_nodeLeft->evalToStoreInto();
      }

    if(evs != NORMAL) return evalStatusReturn(evs);

    //should always return value as ptr to stack.
    UlamValue rtnUV = m_state.m_nodeEvalStack.loadUlamValueFromSlot(2);
    assert(rtnUV.isPtr());

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUV);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  void NodeQuestionColon::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeCondition && m_nodeLeft && m_nodeRight);

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(nut->isReference())
      {
	genCodeToStoreInto(fp, uvpass); //t41062
	if(nut->isPrimitiveType())
	  Node::genCodeReadAutorefIntoATmpVar(fp, uvpass); //t41140
	return;
      }

    TMPSTORAGE stor = nut->getTmpStorageTypeForTmpVar();
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    if(nuti != Void)
      {
	//similar to code generated by NodeControlIf
	m_state.indentUlamCode(fp);
	////fp->write("const ");
	fp->write(nut->getTmpStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, stor).c_str());
	fp->write(";"); GCNL;
      }

    genCodeConditionalExpression(fp, uvpass);

    m_state.indentUlamCode(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;

    UVPass luvpass;
    m_nodeLeft->genCode(fp, luvpass);

    if(nuti != Void)
      {
	m_state.indentUlamCode(fp);
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, stor).c_str());
	fp->write(" = ");
	fp->write(luvpass.getTmpVarAsString(m_state).c_str());
	if(luvpass.getPassStorage() == TMPBITVAL)
	  fp->write(".read()");
	fp->write(";"); GCNL;
      }

    m_state.m_currentIndentLevel--;
    m_state.indentUlamCode(fp);
    fp->write("}\n");

    m_state.indentUlamCode(fp);
    fp->write("else\n");

    m_state.indentUlamCode(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;

    UVPass ruvpass;
    m_nodeRight->genCode(fp, ruvpass);

    if(nuti != Void)
      {
	m_state.indentUlamCode(fp);
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, stor).c_str());
	fp->write(" = ");
	fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
	if(ruvpass.getPassStorage() == TMPBITVAL)
	  fp->write(".read()");
	fp->write(";"); GCNL;
      }

    m_state.m_currentIndentLevel--;
    m_state.indentUlamCode(fp);
    fp->write("} //ends");
    fp->write(getName());
    fp->write("\n\n");

    uvpass = UVPass::makePass(tmpVarNum, stor, nuti, m_state.determinePackable(nuti), m_state, 0, 0);

    //similar to NodeFunctionCall
    // return classes as bitvectors, primitives as tmpregisters (t41140)
    if(nuti != Void)
      {
	if(!nut->isPrimitiveType() && (uvpass.getPassStorage() != TMPBITVAL))
	  {
	    Node::genCodeConvertATmpVarIntoBitVector(fp, uvpass); //t41140
	  }
	else if(nut->isPrimitiveType() && (uvpass.getPassStorage() == TMPBITVAL))
	  {
	    Node::genCodeConvertABitVectorIntoATmpVar(fp, uvpass); //inc uvpass slot
	  }
	//else ok
      }

    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //************
  } //genCode

  void NodeQuestionColon::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    assert(m_nodeCondition && m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(m_state.getReferenceType(nuti) != ALT_REF)
      nuti = m_state.getUlamTypeAsRef(nuti); //e.g. called by NodeCast t41071
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    s32 tmpVarNum = m_state.getNextTmpVarNumber();
    //similar to code generated by NodeControlIf
    m_state.indentUlamCode(fp);
    fp->write(nut->getLocalStorageTypeAsString().c_str());
    fp->write(" * ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPAUTOREF).c_str());
    fp->write(" = NULL;"); GCNL;

    genCodeConditionalExpression(fp, uvpass); //if true

    m_state.indentUlamCode(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;

    UVPass luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    genCodeToStoreIntoExpression(fp, luvpass, tmpVarNum);

    m_state.m_currentIndentLevel--;
    m_state.indentUlamCode(fp);
    fp->write("}\n");

    m_state.indentUlamCode(fp);
    fp->write("else\n");

    m_state.indentUlamCode(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;

    UVPass ruvpass;
    m_nodeRight->genCodeToStoreInto(fp, ruvpass);

    genCodeToStoreIntoExpression(fp, ruvpass, tmpVarNum);

    m_state.m_currentIndentLevel--;
    m_state.indentUlamCode(fp);
    fp->write("} //ends");
    fp->write(getName());
    fp->write("\n\n");

    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    m_state.indentUlamCode(fp);
    fp->write(nut->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, TMPAUTOREF).c_str());
    fp->write(" = (*");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPAUTOREF).c_str());
    fp->write(");"); GCNL;

    uvpass = UVPass::makePass(tmpVarNum2, TMPAUTOREF, nuti, m_state.determinePackable(nuti), m_state, 0, 0);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //************
  } //genCodeToStoreInto

  void NodeQuestionColon::genCodeConditionalExpression(File * fp, UVPass& uvpass)
  {
    m_nodeCondition->genCode(fp, uvpass);

    bool isTerminal = (uvpass.getPassStorage() == TERMINAL);
    UTI cuti = uvpass.getPassTargetType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    m_state.indentUlamCode(fp);
    fp->write("if(");

    if(isTerminal)
      fp->write(m_state.m_pool.getDataAsString(uvpass.getPassNameId()).c_str());
    else
      {
	//regular condition
	assert(cut->getUlamTypeEnum() == Bool);
	fp->write(((UlamTypePrimitiveBool *) cut)->getConvertToCboolMethod().c_str());
	fp->write("(");
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(", ");
	fp->write_decimal(cut->getBitSize());
	fp->write(")");
      }
    fp->write(")"); GCNL;
  } //genCodeConditionalExpression

  void NodeQuestionColon::genCodeToStoreIntoExpression(File * fp, UVPass& uvpass, s32 tmpVarNum)
  {
    UTI nuti = getNodeType();
    if(!m_state.isAltRefType(nuti))
      nuti = m_state.getUlamTypeAsRef(nuti); //e.g. called by NodeCast t41071
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	Symbol * cossym = m_state.m_currentObjSymbolsForCodeGen.back(); //or [0]?
	UTI cosuti = cossym->getUlamTypeIdx();
	if((UlamType::compareForString(cosuti, m_state) == UTIC_SAME) && !m_state.isAltRefType(cosuti))
	  {
	    //special case String ref (t41068)
	    u32 tmpvarstr = m_state.getNextTmpVarNumber();
	    m_state.indentUlamCode(fp);
	    fp->write(nut->getLocalStorageTypeAsString().c_str());
	    fp->write(" ");
	    fp->write(m_state.getTmpVarAsString(nuti, tmpvarstr, TMPAUTOREF).c_str());
	    fp->write("(");
	    fp->write(cossym->getMangledName().c_str());
	    fp->write(", 0u, uc);"); GCNL;

	    m_state.indentUlamCode(fp);
	    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPAUTOREF).c_str());
	    fp->write(" = &"); //tmp ptr = address of cossym
	    fp->write(m_state.getTmpVarAsString(nuti, tmpvarstr, TMPAUTOREF).c_str());
	    fp->write(";"); GCNL;
	  }
	else if(m_state.getReferenceType(cosuti) != ALT_REF)
	  {
	      //called by NodeCast t41071
	    {
	      // similar to NodeReturn genCodeToStoreInto code
	      u32 id = cossym->getId();
	      s32 tmprefnum = m_state.getNextTmpVarNumber();

	      UVPass fuvpass = UVPass::makePass(tmprefnum, TMPAUTOREF, nuti, m_state.determinePackable(nuti), m_state, 0, id);
	      SymbolTmpVar * tmpvarsym = Node::makeTmpVarSymbolForCodeGen(fuvpass, cossym);
	      assert(tmpvarsym);

	      Node::genCodeReferenceInitialization(fp, uvpass, tmpvarsym);
	      delete tmpvarsym;
	      uvpass = fuvpass;
	    }

	    m_state.indentUlamCode(fp);
	    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPAUTOREF).c_str());
	    fp->write(" = &");
	    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(";"); GCNL;
	  }
	else
	  {
	    m_state.indentUlamCode(fp);
	    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPAUTOREF).c_str());
	    fp->write(" = &"); //tmp ptr = address of cossym
	    fp->write(cossym->getMangledName().c_str());
	    fp->write(";"); GCNL;
	  }
	m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of rhs ?
      }
    else //no symbol
      {
	//t41067
	m_state.indentUlamCode(fp);
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPAUTOREF).c_str());
	fp->write(" = &"); //tmp ptr = address of cossym
	fp->write(uvpass.getTmpVarAsString(m_state).c_str());
	fp->write(";"); GCNL;
      }
    return;
  } //genCodeToStoreIntoExpression

  bool NodeQuestionColon::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    m_state.abortShouldntGetHere();
    return false;
  }

  UlamValue NodeQuestionColon::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    m_state.abortShouldntGetHere();
    UlamValue uv;
    return uv;
  }

  UlamValue NodeQuestionColon::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    m_state.abortShouldntGetHere();
    UlamValue uv;
    return uv;
  }

  void NodeQuestionColon::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    m_state.abortShouldntGetHere();
  }

} //end MFM
