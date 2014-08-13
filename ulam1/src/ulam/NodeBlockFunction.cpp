#include <stdio.h>
#include "NodeBlockFunction.h"
#include "CompilerState.h"
#include "SymbolVariable.h"

namespace MFM {

  NodeBlockFunction::NodeBlockFunction(SymbolFunction * fsym, NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s) : NodeBlock(prevBlockNode, state, s), m_funcSymbol(fsym), m_isDefinition(false), m_maxDepth(0) 
  {}

  NodeBlockFunction::~NodeBlockFunction()
  {
    // nodes deleted by SymbolTable in BlockClass 
  }

  void NodeBlockFunction::print(File * fp)
  {
    printNodeLocation(fp);
    UlamType * myut = getNodeType();
    char id[255];
    if(!myut)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), myut->getUlamTypeName().c_str());
    fp->write(id);

    //parameters in symbol

    // if just a declaration, m_nextNode is NULL
    if(isDefinition())
      m_nextNode->print(fp);

    sprintf(id,"maxdepth=%d ----------------%s\n", m_maxDepth, prettyNodeName().c_str());
    fp->write(id);
  }


  void NodeBlockFunction::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(m_funcSymbol->getUlamType()->getUlamTypeNameBrief()); //short type name
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
	fp->write(asym->getUlamType()->getUlamTypeNameBrief()); //short type name
	fp->write(" ");
	fp->write(m_state.m_pool.getDataAsString(asym->getId()).c_str());

	u32 arraysize = 0;
	//if(asym->hasStaticUlamValue())
	if(asym->isDataMember() && !asym->isFunction())
	  {
	    arraysize = ((SymbolVariable * ) asym)->getUlamType()->getUlamKeyTypeSignature().getUlamKeyTypeSignatureArraySize();
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


  const char * NodeBlockFunction::getName()
  {
    return m_state.m_pool.getDataAsString(m_funcSymbol->getId()).c_str();
  }


  const std::string NodeBlockFunction::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeBlockFunction::checkAndLabelType()
  { 
    UlamType * it = m_funcSymbol->getUlamType();
    setNodeType(it);

    if(m_nextNode) //non-empty function
      {
	UlamType * nextType = m_nextNode->checkAndLabelType();
	if(nextType != it)
	  {
	    ULAMTYPE nextBUT = nextType->getUlamTypeEnum();
	    ULAMTYPE itBUT = it->getUlamTypeEnum();
	    if(nextBUT != itBUT)
	      {
		std::ostringstream msg;
		msg << "Function '" << getName() << "''s Return type's <" << it->getUlamTypeName().c_str() << "> base type: <" << UlamType::getUlamTypeEnumAsString(itBUT) << "> does not match resulting type's <" << nextType->getUlamTypeName().c_str() << "> base type: <" << UlamType::getUlamTypeEnumAsString(nextBUT) << ">";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);

		//NodeBlockFunction.cpp:119:52: error: cannot convert ‘MFM::NodeCast*’ to ‘MFM::NodeStatements*’ in assignment
		//haven't considered array or bit sizes here
		//m_nextNode = new NodeCast(m_nextNode, it, m_state);
		//m_nextNode->setNodeLocation(getNodeLocation());
	      }
	    else
	      {
		if(nextType->getArraySize() != it->getArraySize())
		  {
		    std::ostringstream msg;
		    msg << "Function '" << getName() << "''s Return type's <" << it->getUlamTypeName().c_str() << "> array size: <" << it->getArraySize() << "> does not match resulting type's <" << nextType->getUlamTypeName().c_str() << "> array size: <" << nextType->getArraySize() << ">";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
		  }

		if(nextType->getBitSize() != it->getBitSize())
		  {
		    std::ostringstream msg;
		    msg << "Function '" << getName() << "''s Return type's <" << it->getUlamTypeName().c_str() << "> bit size: <" << it->getBitSize() << "> does not match resulting type's <" << nextType->getUlamTypeName().c_str() << "> bit size: <" << nextType->getBitSize() << ">";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
		  }

	      } //base types are the same..array and bit size might vary

	  } //they are the same ulamtype
      }
    else
      {
	std::ostringstream msg;
	msg << "Undefined function block: <" << getName() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    return getNodeType();
  }
  

  EvalStatus NodeBlockFunction::eval()
  {
    assert(isDefinition());
    assert(m_nextNode);

    evalNodeProlog(0);  //new current frame pointer on node eval stack
    makeRoomForNodeType(getNodeType()); //place for return vals node eval stack
    
    m_state.m_funcCallStack.addFrameSlots(getMaxDepth());  //local variables on callstack!!!

    EvalStatus evs = m_nextNode->eval();
    UlamValue rtnUV;
    if(evs == RETURN)
      {
	s32 arraysize = getNodeType()->getArraySize();
	// save results in the stackframe for caller;
	// copies each element of the array by value, 
	// in reverse order ([0] is last at bottom)
	arraysize = (arraysize > 0 ? -arraysize : -1);
	
	rtnUV.init(getNodeType(), arraysize, true, STACK);  //negative to current stack frame pointer
      }
    else if (evs == NORMAL)  //no explicit return statement
      {
	rtnUV.init(getNodeType(), 1, true, EVALRETURN);    //positive to current frame pointer
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


  void NodeBlockFunction::setDefinition()
  {
    m_isDefinition = true;
  }

  
  bool NodeBlockFunction::isDefinition()
  {
    return m_isDefinition;
  }


  void NodeBlockFunction::setMaxDepth(u32 depth)
  {
    m_maxDepth = depth;

    //std::ostringstream msg;
    //msg << "Max Depth is: <" << m_maxDepth << ">";
    //MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
  }


  u32 NodeBlockFunction::getMaxDepth()
  {
    return m_maxDepth;
  }


  SymbolFunction * NodeBlockFunction::getFuncSymbolPtr()
  {
    return m_funcSymbol;
  }

} //end MFM
