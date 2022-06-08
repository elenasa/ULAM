#include <stdio.h>
#include "NodeBlockFunctionDefinition.h"
#include "CompilerState.h"
#include "SymbolVariable.h"
#include "SymbolVariableStack.h"
#include "SymbolFunctionName.h"
#include "NodeVarDecl.h"

namespace MFM {

  NodeBlockFunctionDefinition::NodeBlockFunctionDefinition(SymbolFunction * fsym, NodeBlock * prevBlockNode, NodeTypeDescriptor * nodetype, CompilerState & state, NodeStatements * s) : NodeBlock(prevBlockNode, state, s), m_funcSymbol(fsym), m_isDefinition(false), m_maxDepth(0), m_native(false), m_nodeTypeDesc(nodetype)
  {
    m_nodeParameterList = new NodeList(state);
    assert(m_nodeParameterList);
  }

  NodeBlockFunctionDefinition::NodeBlockFunctionDefinition(const NodeBlockFunctionDefinition& ref) : NodeBlock(ref), m_funcSymbol(NULL), m_isDefinition(ref.m_isDefinition), m_maxDepth(ref.m_maxDepth), m_native(ref.m_native), m_nodeTypeDesc(NULL)
 {
   m_nodeParameterList = (NodeList *) ref.m_nodeParameterList->instantiate();
   if(ref.m_nodeTypeDesc)
     m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
 }

  NodeBlockFunctionDefinition::~NodeBlockFunctionDefinition()
  {
    // nodes deleted by SymbolTable in BlockClass
    delete m_nodeParameterList;
    m_nodeParameterList = NULL;

    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  Node * NodeBlockFunctionDefinition::instantiate()
  {
    return new NodeBlockFunctionDefinition(*this);
  }

  void NodeBlockFunctionDefinition::updateLineage(NNO pno)
  {
    NodeBlock::updateLineage(pno);
    m_state.pushCurrentBlock(this);
    if(m_nodeTypeDesc)
      {
	m_nodeTypeDesc->updateLineage(getNodeNo());
      }
    if(m_nodeParameterList)
      {
	m_nodeParameterList->updateLineage(getNodeNo());
      }
    m_state.popClassContext();
  } //updateLineage

  bool NodeBlockFunctionDefinition::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeBlock::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    if(m_nodeParameterList && m_nodeParameterList->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeBlockFunctionDefinition::checkAbstractInstanceErrors()
  {
    if(m_nodeParameterList)
      m_nodeParameterList->checkAbstractInstanceErrors();
    if(m_nodeNext)
      m_nodeNext->checkAbstractInstanceErrors();
  }

  void NodeBlockFunctionDefinition::setNodeLocation(Locator loc)
  {
    m_nodeParameterList->setNodeLocation(loc);
    Node::setNodeLocation(loc);
  }

  void NodeBlockFunctionDefinition::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    //parameters in symbol

    // if just a declaration, m_nodeNext is NULL
    if(isDefinition())
      m_nodeNext->print(fp);

    sprintf(id,"maxdepth=%d ----------------%s\n", m_maxDepth, prettyNodeName().c_str());
    fp->write(id);
  } //print

  void NodeBlockFunctionDefinition::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(getNodeType()).c_str()); //short type name
    fp->write(" ");
    fp->write(getName());
    // has no m_node!
    // declaration has no m_nodeNext!!
    fp->write("(");
    u32 numparams = getNumberOfParameters();

    for(u32 i = 0; i < numparams; i++)
      {
	if(i > 0)
	  fp->write(", ");
	m_nodeParameterList->printPostfix(fp, i);
      }
    fp->write(")");

    if(isDefinition())
      {
	fp->write(" { ");
	m_nodeNext->printPostfix(fp);
	fp->write(" }");
      }
    else
      fp->write(";");
  } //printPostfix


  u32 NodeBlockFunctionDefinition::getParameterNameId(u32 n)
  {
    return m_nodeParameterList->getNameId(n);
  }

  u32 NodeBlockFunctionDefinition::getParameterTypeNameId(u32 n)
  {
    return m_nodeParameterList->getTypeNameId(n);
  }

  const char * NodeBlockFunctionDefinition::getName()
  {
    return m_state.m_pool.getDataAsString(getNameId()).c_str();
  }

  u32 NodeBlockFunctionDefinition::getNameId()
  {
    assert(m_funcSymbol);
    return m_funcSymbol->getFunctionNameId();
  }

  u32 NodeBlockFunctionDefinition::getTypeNameId()
  {
    if(m_nodeTypeDesc)
      return m_nodeTypeDesc->getTypeNameId();

    UTI nuti = getNodeType();
    assert(m_state.okUTItoContinue(nuti));
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    //skip bitsize if default size
    if(nut->getBitSize() == ULAMTYPE_DEFAULTBITSIZE[nut->getUlamTypeEnum()])
      return m_state.m_pool.getIndexForDataString(nut->getUlamTypeNameOnly());
    return m_state.m_pool.getIndexForDataString(nut->getUlamTypeNameBrief());
  } //getTypeNameId

  const std::string NodeBlockFunctionDefinition::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBlockFunctionDefinition::checkAndLabelType(Node * thisparentnode)
  {
    assert(m_funcSymbol);
    UTI fit = m_funcSymbol->getUlamTypeIdx();
    UTI it = fit;
    UTI cuti = m_state.getCompileThisIdx();

    // don't want to leave Nav dangling
    assert(m_nodeTypeDesc);
    if(m_nodeTypeDesc)
      it = m_nodeTypeDesc->checkAndLabelType(this);

    if(it == Nav)
      {
	std::ostringstream msg;
	msg << "Invalid Function Return type ";
	msg << "used with function name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    else
      {
	checkParameterNodeTypes();

	if(!m_state.isComplete(it))
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Function Return type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", used with function name '" << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    it = Hzy; //t3653
	  }
	else
	  {
	    if(m_state.okUTItoContinue(it) && (fit != it)) //exact UTI match
	      {
		std::ostringstream msg;
		msg << "Resetting function symbol UTI" << fit;
		msg << ", " << m_state.getUlamTypeNameByIndex(fit).c_str();
		msg << " by type descriptor type: ";
		msg << m_state.getUlamTypeNameByIndex(it).c_str();
		msg << "' UTI" << it;
		msg << " used with function name '" << getName();
		msg <<  " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		//m_state.mapTypesInCurrentClass(fit, it);
		m_funcSymbol->resetUlamType(it); //consistent!
		//m_state.updateUTIAliasForced(fit, it); //Mon Jun  6 13:45:15 2016 ?
	      }
	  }
      }

    setNodeType(it);

    if(it == Nav)
      return Nav; //bail for this iteration

    if(it == Hzy)
      {
	m_state.setGoAgain();
	return Hzy; //bail for this iteration
      }

    assert(m_state.okUTItoContinue(it));

    bool isref = m_state.isAltRefType(it) && !m_state.isConstantRefType(it);
    if(m_state.isAClass(it) || isref)
      setStoreIntoAble(TBOOL_TRUE); //t3912 (class)

    if(!isref)
      setReferenceAble(TBOOL_FALSE); //set after storeintoable t3661,2; t3630

    m_state.pushCurrentBlock(this);

    //set self slot just below return value
    u32 selfid = m_state.m_pool.getIndexForDataString("self");
    Symbol * selfsym = NULL;
    bool hazyKin = false; //return is always false?
    AssertBool isDefined = m_state.alreadyDefinedSymbolHere(selfid, selfsym, hazyKin) && !hazyKin;
    assert(isDefined);
    s32 newslot = -2 - m_state.slotsNeeded(it); //2nd hidden arg
    ((SymbolVariable *) selfsym)->setStackFrameSlotIndex(newslot);

    m_state.m_currentFunctionReturnNodes.clear(); //vector of return nodes
    m_state.m_currentFunctionReturnType = it;

    makeSuperSymbol(-(m_state.slotsNeeded(it) + 1)); //same as self?);

    if(m_nodeNext) //non-empty function
      {
	m_nodeNext->checkAndLabelType(this); //side-effect
	if(!m_state.checkFunctionReturnNodeTypes(m_funcSymbol)) //gives some errors
	  setNodeType(Nav); //tries to avoid assert in resolving loop; return sets goagain
      }
    else
      {
	std::ostringstream msg;
	msg << "Undefined function block '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
      }
    m_state.popClassContext(); //restores previous block ptr
    return getNodeType();
  } //checkAndLabelType

  bool NodeBlockFunctionDefinition::checkParameterNodeTypes()
  {
    UTI puti = m_nodeParameterList->checkAndLabelType(this);
    return m_state.okUTItoContinue(puti);
  }

  void NodeBlockFunctionDefinition::addParameterNode(Node * nodeArg)
  {
    m_nodeParameterList->addNodeToList(nodeArg);
  }

  Node * NodeBlockFunctionDefinition::getParameterNode(u32 pidx)
  {
    assert(m_nodeParameterList);
    return m_nodeParameterList->getNodePtr(pidx);
  }

  UTI NodeBlockFunctionDefinition::getParameterNodeGivenType(u32 pidx)
  {
    NodeVarDecl * parmdef = (NodeVarDecl *) getParameterNode(pidx);
    assert(parmdef);
    UTI puti = parmdef->getTypeDescriptorGivenType();
    return puti;
  }

  u32 NodeBlockFunctionDefinition::getNumberOfParameters()
  {
    return m_nodeParameterList->getNumberOfNodes();
  }

  bool NodeBlockFunctionDefinition::isAConstantParameter(u32 pidx)
  {
    return m_nodeParameterList->isAConstantFunctionParameter(pidx);
  }

  void NodeBlockFunctionDefinition::makeSuperSymbol(s32 slot)
  {
    UTI cuti = m_state.getCompileThisIdx();
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    UTI superuti = csym->getBaseClass(0);
    SymbolVariableStack * supersym = NULL;
    u32 superid = m_state.m_pool.getIndexForDataString("super");
    if(!NodeBlock::isIdInScope(superid, (Symbol *&) supersym))
      {
	if(m_state.okUTItoContinue(superuti) && !m_state.isHolder(superuti)) //t41013
	  {
	    Token superTok(TOK_KW_SUPER, getNodeLocation(), 0);
	    supersym = new SymbolVariableStack(superTok, m_state.getUlamTypeAsRef(superuti, ALT_REF), slot, m_state);
	    assert(supersym);
	    supersym->setAutoLocalType(ALT_REF);
	    supersym->setIsSuper();
	    m_state.addSymbolToCurrentScope(supersym); //ownership goes to the block
	  }
	//else ok
      }
    else
      {
	if(superuti == Nouti)
	  {
	    AssertBool isGone = NodeBlock::removeIdFromScope(superid, (Symbol *&) supersym);
	    assert(isGone);
	    delete supersym;
	    supersym = NULL;
	  }
	else //ok
	  {
	    s32 oldslot = supersym->getStackFrameSlotIndex();
	    if(oldslot != slot)
	      {
		std::ostringstream msg;
		msg << "'" << m_state.m_pool.getDataAsString(superid).c_str();
		msg << "' was at slot: " << oldslot << ", new slot is " << slot;
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		supersym->setStackFrameSlotIndex(slot);
		//t3704, t3706, t3707, t3709, t3710
	      }
	  }
      }
  } //makeSuperSymbol

  void NodeBlockFunctionDefinition::printUnresolvedLocalVariables(u32 fid)
  {
    if(m_nodeParameterList)
      m_nodeParameterList->printUnresolvedLocalVariables(fid);
    m_nodeNext->printUnresolvedLocalVariables(fid);
  } //printUnresolvedLocalVariables

  void NodeBlockFunctionDefinition::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeBlock::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if(m_nodeParameterList)
      m_nodeParameterList->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  EvalStatus NodeBlockFunctionDefinition::eval()
  {
    assert(isDefinition());
    assert(m_nodeNext);

    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    //for eval, native function blocks (NodeBlockEmpty) return Normal. t3942
    if(isNative() && getNodeType() != Void) return evalStatusReturnNoEpilog(UNEVALUABLE);

    //for eval, virtual functions, init globals (e.g. t3611)
    m_state.m_currentAutoObjPtr = UlamValue(); //wipeout
    m_state.m_currentAutoStorageType = Nouti; //clear (was Nav)

    m_state.pushCurrentBlock(this); //push func def

    // m_currentObjPtr set up by caller
    assert(m_state.okUTItoContinue(m_state.m_currentObjPtr.getPtrTargetType()));
    m_state.m_currentFunctionReturnType = nuti; //to help find hidden first arg

    evalNodeProlog(0); //new current frame pointer on node eval stack
    makeRoomForNodeType(nuti); //place for return vals node eval stack

    m_state.m_funcCallStack.addFrameSlots(getMaxDepth()); //local variables on callstack!

    EvalStatus evs = m_nodeNext->eval();

    m_state.popClassContext(); //restore

    PACKFIT packRtn = m_state.determinePackable(nuti);
    UlamValue rtnUV;

    if(evs == RETURN)
      {
	if(Node::returnValueOnStackNeededForEval(nuti))
	  {
	    //t3189 returns a class (non-ref);
	    //t3630 return a reference to a primitive;
	    rtnUV = m_state.m_funcCallStack.loadUlamValueFromSlot(-1); //popArg();
	  }
	else
	  {
	    // save results in the stackframe for caller;
	    // copies each element of the array by value,
	    //in reverse order ([0] is last at bottom)
	    //negative to current stack frame pointer
	    s32 slot = m_state.slotsNeeded(nuti);
	    rtnUV = UlamValue::makePtr(-slot, STACK, nuti, packRtn, m_state);
	  }
      }
    else if (evs == NORMAL)  //no explicit return statement
      {
	// 1 for base of array or scalar
	//positive to current frame pointer
	rtnUV = UlamValue::makePtr(1, EVALRETURN, nuti, packRtn, m_state);
      }
    else
      {
	m_state.m_funcCallStack.returnFrame(m_state);
	return evalStatusReturn(evs);
      }

    // save results in the node eval stackframe for function caller, returning
    // each element of array by value, in reverse order ([0] is last at bottom);
    // Could be a reference if called from evalToStoreInto (e.g. t3630)
    if(m_state.isAltRefType(nuti))
      Node::assignReturnValuePtrToStack(rtnUV);
    else
      Node::assignReturnValueToStack(rtnUV);

    m_state.m_funcCallStack.returnFrame(m_state);
    evalNodeEpilog();
    return NORMAL; //t3896
  } //eval

  EvalStatus NodeBlockFunctionDefinition::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    // m_currentObjPtr set up by caller
    assert(m_state.okUTItoContinue(m_state.m_currentObjPtr.getPtrTargetType()));
    assert(m_state.isAClass(nuti) || (m_state.isAltRefType(nuti) && !m_state.isConstantRefType(nuti)) || m_funcSymbol->isConstructorFunction()); //sanity?
    return eval();
  } //evalToStoreInto

  void NodeBlockFunctionDefinition::setDefinition()
  {
    m_isDefinition = true;
  }

  bool NodeBlockFunctionDefinition::isDefinition() const
  {
    return m_isDefinition;
  }

  void NodeBlockFunctionDefinition::setMaxDepth(u32 depth)
  {
    m_maxDepth = depth;

#if 0
    std::ostringstream msg;
    msg << getName() << ": Max Depth is <" << m_maxDepth << ">";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
#endif
  }

  u32 NodeBlockFunctionDefinition::getMaxDepth()
  {
    return m_maxDepth;
  }

  void NodeBlockFunctionDefinition::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    m_state.pushCurrentBlock(this);
    //base starts with slots for: return type, args, and hidden
    u32 max1 = 0;
    u32 nomaxdepth = 0;
    if(m_nodeParameterList) //slot indices negative to frame
      m_nodeParameterList->calcMaxDepth(max1, nomaxdepth, base);

    NodeBlock::calcMaxDepth(depth, maxdepth, 1); // one for the frame ptr offset
    m_state.popClassContext();
  } //calcMaxDepth

  void NodeBlockFunctionDefinition::calcMaxIndexOfVirtualFunctionInOrderOfDeclaration(SymbolClass* csym, s32& maxidx)
  {
    m_state.abortShouldntGetHere(); //not in parse tree..see NodeFuncDecl
  }

  void NodeBlockFunctionDefinition::setNative()
  {
    m_native = true;
  }

  bool NodeBlockFunctionDefinition::isNative() const
  {
    return m_native;
  }

  SymbolFunction * NodeBlockFunctionDefinition::getFuncSymbolPtr()
  {
    return m_funcSymbol;
  }

  void NodeBlockFunctionDefinition::setFuncSymbolPtr(SymbolFunction * fsymptr)
  {
    assert(fsymptr);
    m_funcSymbol = fsymptr;
  }

  void NodeBlockFunctionDefinition::genCode(File * fp, UVPass& uvpass)
  {
    // m_currentObjSymbol set up by caller
    //    assert(m_state.m_currentObjSymbolForCodeGen != NULL);
    m_state.pushCurrentBlock(this);

    //"self" belongs to func def block that we're currently gencoding
    u32 selfid = m_state.m_pool.getIndexForDataString("self");
    Symbol * selfsym = NULL;
    bool hazykin = false; //unused
    AssertBool gotSelf = m_state.alreadyDefinedSymbolHere(selfid, selfsym, hazykin);
    assert(gotSelf);
    m_state.m_currentSelfSymbolForCodeGen = selfsym;

    assert(isDefinition());
    assert(m_nodeNext);

    assert(!isNative());

    if(m_funcSymbol->isVirtualFunction())
      {
	m_state.m_gencodingAVirtualFunctionInThisOriginatingClass = m_funcSymbol->getVirtualMethodOriginatingClassUTI(); //t41318
      }
    else
      {
	assert(m_state.m_gencodingAVirtualFunctionInThisOriginatingClass == Nouti); //sanity chk
      }

    fp->write("\n");
    m_state.indentUlamCode(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_nodeNext->genCode(fp, uvpass);

    m_state.m_currentIndentLevel--;

    m_state.indentUlamCode(fp);
    fp->write("} // ");
    fp->write(m_funcSymbol->getMangledName().c_str()); //end of function
    fp->write("\n");

    m_state.m_gencodingAVirtualFunctionInThisOriginatingClass = Nouti; //clear
    m_state.popClassContext(); //restores NodeBlock::getPreviousBlockPointer()
  } //genCode

} //end MFM
