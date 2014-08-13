#include "NodeFunctionCall.h"
#include "CompilerState.h"
#include "NodeBlockFunction.h"
#include "NodeProgram.h"
#include "SymbolFunction.h"
#include "CallStack.h"

namespace MFM {

  NodeFunctionCall::NodeFunctionCall(Token tok, SymbolFunction * fsym, CompilerState & state) : Node(state), m_functionNameTok(tok), m_funcSymbol(fsym) {}

  NodeFunctionCall::~NodeFunctionCall()
  {
    //may need to delete the nodes
    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	delete m_argumentNodes[i];
      }
    m_argumentNodes.clear();
  }


  void NodeFunctionCall::printPostfix(File * fp)
  {
    fp->write(" (");
    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	m_argumentNodes[i]->printPostfix(fp);
      }

    fp->write(" )");
    fp->write(getName());
  }


  const char * NodeFunctionCall::getName()
  {
    return m_state.getDataAsString(&m_functionNameTok).c_str();
  }


  const std::string NodeFunctionCall::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeFunctionCall::checkAndLabelType()
  {
    UlamType * it = m_state.getUlamTypeByIndex(Nav);  //Nav
    u32 numErrorsFound = 0;

    //call was before def, look up in class block
    if(m_funcSymbol == NULL)
      {
	Symbol * asymptr;
	if(m_state.m_classBlock->isFuncIdInScope(m_functionNameTok.m_dataindex,asymptr))
	  {
	    if(asymptr->isFunction())
	      m_funcSymbol = (SymbolFunction *) asymptr;
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.m_pool.getDataAsString(m_functionNameTok.m_dataindex).c_str() << "> is not a function, and cannot be called";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		numErrorsFound++;		
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) <" << m_state.m_pool.getDataAsString(m_functionNameTok.m_dataindex).c_str() << "> is not a defined function, and cannot be called";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    numErrorsFound++;		
	  }	  
      }

    //cannot continue
    if(numErrorsFound > 0)
      {
	setNodeType(it);
	return it;
      }

    u32 numArgs   = m_argumentNodes.size();
    u32 numParams = m_funcSymbol->getNumberOfParameters();

    if(numArgs != numParams)
      {
	std::ostringstream msg;
	msg << "(3) <" << m_state.m_pool.getDataAsString(m_functionNameTok.m_dataindex).c_str() << "> has " << numArgs << " arguments for a defined function, that has " << numParams << " parameters";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	numErrorsFound++;		
      }

    u32 shorterList = (numArgs > numParams ? numParams : numArgs);

    //validate each arg against the corresponding parameter in the SymbolFunction.
    for(u32 i = 0; i < shorterList; i++)
      {	
	Symbol * sym = m_funcSymbol->getParameterSymbolPtr(i);
	UlamType * paramtype = sym->getUlamType();
	UlamType * argtype = m_argumentNodes[i]->checkAndLabelType();  //plus side-effect
	if(paramtype != argtype)
	  {
	    std::ostringstream msg;
	    msg << "(4) Mismatch Types: Function call <" << m_state.m_pool.getDataAsString(m_functionNameTok.m_dataindex).c_str() << "> argument " << i << " has type <" << argtype->getUlamTypeName().c_str() << ">; parameter is defined as type: <" << paramtype->getUlamTypeName().c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    numErrorsFound++;		//castable???
	  }
      }


    if(!numErrorsFound)
      {
	 it = m_funcSymbol->getUlamType();
      }

    setNodeType(it);
    return it;
  }


  void NodeFunctionCall::eval()
  {
    assert(m_funcSymbol);
    NodeBlockFunction * func = m_funcSymbol->getFunctionNode();
    assert(func);    

    evalNodeProlog(0); //new current frame pointer on node eval stack
    
    u32 argsPushed = 0;
    // place values of arguments on call stack (reverse order) before calling function
    for(s32 i= m_argumentNodes.size() - 1; i >= 0; i--)
      {
	UlamType * argType = m_argumentNodes[i]->getNodeType();
	// arrays are handled by callstack, and passed by value
	u32 slots = makeRoomForNodeType(argType); //for eval return
	
	m_argumentNodes[i]->eval();

	// transfer to call stack
	if(slots==1)
	  {
	    UlamValue auv = m_state.m_nodeEvalStack.popArg();
	    argsPushed += m_state.m_funcCallStack.pushArg(auv, m_state);
	  }
	else
	  {
	    //array to transfer without reversing order again
	    u32 baseSlot = m_state.m_funcCallStack.getRelativeTopOfStackNextSlot();
	    makeRoomForNodeType(argType, STACK);  
	    UlamValue basePtr(argType, baseSlot, true, STACK);  

	    UlamValue auvPtr(argType, 1, true, EVALRETURN);  //positive to current frame pointer

	    m_state.m_funcCallStack.assignUlamValue(basePtr,auvPtr, m_state);
	    m_state.m_nodeEvalStack.popArgs(slots);
	  }
      }

    //push return value last (on both STACKS for now) 
    UlamType * rtnType = m_funcSymbol->getUlamType();
    u32 rtnslots = makeRoomForNodeType(rtnType);
    //makeRoomForNodeType(rtnType, STACK);

    //assert(rtnslots == rtnType->getArraySize()); not true for scalar
    assert(rtnType->getArraySize() > 0 ? rtnslots == rtnType->getArraySize() : rtnslots == 1); 
    //********************************************    

    func->eval();   //NodeBlockFunction..

    //**********************************************

    UlamValue rtnPtr(rtnType, 1, true, EVALRETURN);  //positive to current frame pointer
    assignReturnValueToStack(rtnPtr);  //in return space on eval stack; didn't need callstack space!!!

    //m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args
    m_state.m_funcCallStack.popArgs(argsPushed); //drops all the args

    evalNodeEpilog();  //clears out the node eval stack
  }


  void NodeFunctionCall::addArgument(Node * n)
  {
    //check types later
    m_argumentNodes.push_back(n);
    return;
  }


  u32 NodeFunctionCall::getNumberOfArguments()
  {
    return m_argumentNodes.size();
  }

} //end MFM
