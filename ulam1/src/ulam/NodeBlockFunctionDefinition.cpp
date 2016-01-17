#include <stdio.h>
#include "NodeBlockFunctionDefinition.h"
#include "CompilerState.h"
#include "SymbolVariable.h"
#include "SymbolFunctionName.h"

namespace MFM {

  NodeBlockFunctionDefinition::NodeBlockFunctionDefinition(SymbolFunction * fsym, NodeBlock * prevBlockNode, NodeTypeDescriptor * nodetype, CompilerState & state, NodeStatements * s) : NodeBlock(prevBlockNode, state, s), m_funcSymbol(fsym), m_isDefinition(false), m_maxDepth(0), m_native(false), m_nodeTypeDesc(nodetype)
  {
    m_nodeParameterList = new NodeList(state);
    assert(m_nodeParameterList);
  }

  NodeBlockFunctionDefinition::NodeBlockFunctionDefinition(const NodeBlockFunctionDefinition& ref) : NodeBlock(ref), m_funcSymbol(NULL), m_isDefinition(ref.m_isDefinition), m_maxDepth(ref.m_maxDepth), m_native(ref.m_native)
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
    m_state.pushCurrentBlock(this); //before?
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
  } //checkAbstractInstanceErrors

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
    fp->write(m_state.getUlamTypeNameBriefByIndex(m_funcSymbol->getUlamTypeIdx()).c_str()); //short type name
    fp->write(" ");
    fp->write(getName());
    // has no m_node!
    // declaration has no m_nodeNext!!
    fp->write("(");
    u32 numparams = m_funcSymbol->getNumberOfParameters();

    for(u32 i = 0; i < numparams; i++)
      {
	if(i > 0)
	  fp->write(", ");

	Symbol * asym = m_funcSymbol->getParameterSymbolPtr(i);
	assert(asym);
	fp->write(m_state.getUlamTypeNameBriefByIndex(asym->getUlamTypeIdx()).c_str()); //short type name
	fp->write(" ");
	fp->write(m_state.m_pool.getDataAsString(asym->getId()).c_str());

	s32 arraysize = 0;
	if(asym->isDataMember() && !asym->isFunction())
	  {
	    arraysize = m_state.getArraySize( ((SymbolVariable *) asym)->getUlamTypeIdx());
	  }

	if(arraysize > NONARRAYSIZE)
	  {
	    fp->write("[");
	    fp->write_decimal(arraysize);
	    fp->write("]");
	  }
	else if(arraysize == UNKNOWNSIZE)
	  {
	    fp->write("[UNKNOWN]");
	  }

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

  const char * NodeBlockFunctionDefinition::getName()
  {
    return m_state.m_pool.getDataAsString(m_funcSymbol->getId()).c_str();
  }

  const std::string NodeBlockFunctionDefinition::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBlockFunctionDefinition::checkAndLabelType()
  {
    assert(m_funcSymbol);
    UTI fit = m_funcSymbol->getUlamTypeIdx();
    UTI it = fit;
    UTI cuti = m_state.getCompileThisIdx();

    // don't want to leave Nav dangling
    if(m_nodeTypeDesc)
      {
	it = m_nodeTypeDesc->checkAndLabelType();
      }

    checkParameterNodeTypes();

    if(!m_state.isComplete(it))
      {
	std::ostringstream msg;
	msg << "Incomplete Function Return type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << " used with function name '" << getName();
	msg << "' UTI" << it << " while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	it = Hzy;
      }
    else
      {
	if((fit != Nav) && (fit != Hzy) && (fit != it)) //exact UTI match
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
	  m_state.mapTypesInCurrentClass(fit, it);
	  m_funcSymbol->resetUlamType(it); //consistent!
	}

	PACKFIT packed = m_state.determinePackable(it);
	if(!WritePacked(packed) && !m_state.isScalar(it))
	  {
	    std::ostringstream msg;
	    msg << "Function Definition <" << getName();
	    msg << "> return type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << " requires UNPACKED array support";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    it = Nav;
	  }
      }

    setNodeType(it);

    if(it == Nav)
      return Nav;; //bail for this iteration

    if(it == Hzy)
      return Hzy;; //bail for this iteration

    m_state.pushCurrentBlock(this);

    m_state.m_currentFunctionReturnNodes.clear(); //vector of return nodes
    m_state.m_currentFunctionReturnType = it;

    if(m_nodeNext) //non-empty function
      {
	m_nodeNext->checkAndLabelType(); //side-effect
	if(!m_state.checkFunctionReturnNodeTypes(m_funcSymbol)) //gives some errors
	  setNodeType(Nav); //tries to avoid assert in resolving loop; return sets goagain
      }
    else
      {
	std::ostringstream msg;
	msg << "Undefined function block <" << getName() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
      }
    m_state.popClassContext(); //restores previous block ptr
    return getNodeType();
  } //checkAndLabelType

  bool NodeBlockFunctionDefinition::checkParameterNodeTypes()
  {
    return m_nodeParameterList->checkAndLabelType();
  }

  void NodeBlockFunctionDefinition::addParameterNode(Node * nodeArg)
  {
    m_nodeParameterList->addNodeToList(nodeArg);
  }

  void NodeBlockFunctionDefinition::countNavNodes(u32& cnt)
  {
    NodeBlock::countNavNodes(cnt);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavNodes(cnt);
    if(m_nodeParameterList)
      m_nodeParameterList->countNavNodes(cnt);
  } //countNavNodes

  EvalStatus NodeBlockFunctionDefinition::eval()
  {
    assert(isDefinition());
    assert(m_nodeNext);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    m_state.pushCurrentBlock(this); //push func def

    // m_currentObjPtr set up by caller
    assert(m_state.m_currentObjPtr.getUlamValueTypeIdx() != Nav);
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
	if(m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum() == UAtom)
	  {
	    //avoid pointer to atom situation
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
	evalNodeEpilog();
	return evs;
      }

    //save results in the node eval stackframe for function caller
    //return each element of the array by value,
    //in reverse order ([0] is last at bottom)
    Node::assignReturnValueToStack(rtnUV);

    m_state.m_funcCallStack.returnFrame(m_state);
    evalNodeEpilog();
    return NORMAL;
  } //eval

  void NodeBlockFunctionDefinition::setDefinition()
  {
    m_isDefinition = true;
  }

  bool NodeBlockFunctionDefinition::isDefinition()
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

    //set self slot just below return value
    u32 selfid = m_state.m_pool.getIndexForDataString("atom");
    Symbol * selfsym = NULL;
    bool hazyKin = false; //return is always false?
    AssertBool isDefined = m_state.alreadyDefinedSymbol(selfid, selfsym, hazyKin) && !hazyKin;
    assert(isDefined);
    s32 newslot = -1 - m_state.slotsNeeded(getNodeType());
    s32 oldslot = ((SymbolVariable *) selfsym)->getStackFrameSlotIndex();
    if(oldslot != newslot)
      {
	std::ostringstream msg;
	msg << "'" << m_state.m_pool.getDataAsString(selfid).c_str();
	msg << "' was at slot: " << oldslot << ", new slot is " << newslot;
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	((SymbolVariable *) selfsym)->setStackFrameSlotIndex(newslot);
      }

    NodeBlock::calcMaxDepth(depth, maxdepth, 1); // one for the frame ptr offset
    m_state.popClassContext();

    // special case test function:
    u32 testid = m_state.m_pool.getIndexForDataString("test");
    if(m_funcSymbol->getId() == testid)
      maxdepth += 1; //add spot for return since no caller does
  } //calcMaxDepth

  void NodeBlockFunctionDefinition::setNative()
  {
    m_native = true;
  }

  bool NodeBlockFunctionDefinition::isNative()
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

  void NodeBlockFunctionDefinition::genCode(File * fp, UlamValue& uvpass)
  {
    // m_currentObjSymbol set up by caller
    //    assert(m_state.m_currentObjSymbolForCodeGen != NULL);
    m_state.pushCurrentBlock(this);

    assert(isDefinition());
    assert(m_nodeNext);

    assert(!isNative());

    fp->write("\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_nodeNext->genCode(fp, uvpass);

    m_state.m_currentIndentLevel--;

    fp->write("\n");
    m_state.indent(fp);
    fp->write("} // ");
    fp->write(m_funcSymbol->getMangledName().c_str()); //end of function
    fp->write("\n\n\n");

    m_state.popClassContext(); //restores NodeBlock::getPreviousBlockPointer()
  } //genCode

} //end MFM
