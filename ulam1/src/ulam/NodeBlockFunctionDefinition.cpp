#include <stdio.h>
#include "NodeBlockFunctionDefinition.h"
#include "CompilerState.h"
#include "SymbolVariable.h"

namespace MFM {

  NodeBlockFunctionDefinition::NodeBlockFunctionDefinition(SymbolFunction * fsym, NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s) : NodeBlock(prevBlockNode, state, s), m_funcSymbol(fsym), m_isDefinition(false), m_maxDepth(0) 
  {}

  NodeBlockFunctionDefinition::~NodeBlockFunctionDefinition()
  {
    // nodes deleted by SymbolTable in BlockClass 
  }

  void NodeBlockFunctionDefinition::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut==Nav)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    //parameters in symbol

    // if just a declaration, m_nextNode is NULL
    if(isDefinition())
      m_nextNode->print(fp);

    sprintf(id,"maxdepth=%d ----------------%s\n", m_maxDepth, prettyNodeName().c_str());
    fp->write(id);
  }


  void NodeBlockFunctionDefinition::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(m_funcSymbol->getUlamTypeIdx()).c_str()); //short type name
    fp->write(" ");
    fp->write(getName());
    // has no m_node! 
    // declaration has no m_nextNode!!
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

	u32 arraysize = 0;
	if(asym->isDataMember() && !asym->isFunction())
	  {
	    arraysize = m_state.getArraySize( ((SymbolVariable *) asym)->getUlamTypeIdx());
	  }

	if(arraysize > 0)
	  {
	    fp->write("[");
	    fp->write_decimal(arraysize);
	    fp->write("]");
	  }
      }
    fp->write(")");

    if(isDefinition())
      {
	fp->write(" { ");
	m_nextNode->printPostfix(fp);
	fp->write(" }");
      }
    else
      fp->write(";");

  }


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
    UTI it = m_funcSymbol->getUlamTypeIdx();
    setNodeType(it);

    m_state.m_currentBlock = this;

    m_state.m_currentFunctionReturnNodes.clear(); //vector of return nodes
    m_state.m_currentFunctionReturnType = it;

    if(m_nextNode) //non-empty function
      {
	m_nextNode->checkAndLabelType();                     //side-effect
	m_state.checkFunctionReturnNodeTypes(m_funcSymbol);  //gives errors
      }
    else
      {
	std::ostringstream msg;
	msg << "Undefined function block: <" << getName() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    return getNodeType();
  }
  

  EvalStatus NodeBlockFunctionDefinition::eval()
  {
    assert(isDefinition());
    assert(m_nextNode);

    // m_currentObjPtr set up by caller
    assert(m_state.m_currentObjPtr.getUlamValueTypeIdx() != Nav);
    m_state.m_currentFunctionReturnType = getNodeType(); //to help find hidden first arg

    evalNodeProlog(0);                  //new current frame pointer on node eval stack
    makeRoomForNodeType(getNodeType()); //place for return vals node eval stack
    
    m_state.m_funcCallStack.addFrameSlots(getMaxDepth());  //local variables on callstack!

#define VERIFYHIDDENARG
#ifdef VERIFYHIDDENARG
    // Curious: is m_currentObjPtr the same as the "hidden" first arg on the STACK?
    UTI nuti = getNodeType();
    s32 arraysize = m_state.getArraySize(nuti);    
    s32 firstArgSlot = -1;
    if(m_state.determinePackable(nuti))
      firstArgSlot--;
    else
      firstArgSlot -= (arraysize > 0 ? arraysize: 1);

    UlamValue firstArg = m_state.m_funcCallStack.loadUlamValueFromSlot(firstArgSlot);
    assert(firstArg == m_state.m_currentObjPtr);
    //////////////////////////////////////////////////////
#endif

    EvalStatus evs = m_nextNode->eval();

    bool packRtn = m_state.determinePackable(getNodeType());
    UlamValue rtnUV;

    if(evs == RETURN)
      {	
	// save results in the stackframe for caller;
	// copies each element of the array by value, 
	// in reverse order ([0] is last at bottom)
	s32 arraysize = m_state.getArraySize(getNodeType());
	if(packRtn)
	  arraysize = -1;  
	else
	  arraysize = (arraysize > 0 ? -arraysize : -1);
	
	rtnUV = UlamValue::makePtr(arraysize, STACK, getNodeType(), packRtn, m_state); //negative to current stack frame pointer
      }
    else if (evs == NORMAL)  //no explicit return statement
      {
	// 1 for base of array or scalar
	rtnUV = UlamValue::makePtr(1, EVALRETURN, getNodeType(), packRtn, m_state); //positive to current frame pointer
      }
    else
      {
	m_state.m_funcCallStack.returnFrame();
	evalNodeEpilog();
	return evs;
      }

    //save results in the node eval stackframe for function caller
    //return each element of the array by value, in reverse order ([0] is last at bottom)
    assignReturnValueToStack(rtnUV);

    m_state.m_funcCallStack.returnFrame();
    evalNodeEpilog();
    return NORMAL;
  }


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

    //std::ostringstream msg;
    //msg << "Max Depth is: <" << m_maxDepth << ">";
    //MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
  }


  u32 NodeBlockFunctionDefinition::getMaxDepth()
  {
    return m_maxDepth;
  }


  SymbolFunction * NodeBlockFunctionDefinition::getFuncSymbolPtr()
  {
    return m_funcSymbol;
  }


  void NodeBlockFunctionDefinition::genCode(File * fp)
  {
    assert(isDefinition());
    assert(m_nextNode);

    fp->write("\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_nextNode->genCode(fp);
    m_state.m_currentIndentLevel--;
    
    m_state.indent(fp);
    fp->write("}\n\n");
  }


} //end MFM
