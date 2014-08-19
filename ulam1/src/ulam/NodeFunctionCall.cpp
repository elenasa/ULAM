#include "NodeFunctionCall.h"
#include "CompilerState.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeProgram.h"
#include "SymbolFunction.h"
#include "SymbolFunctionName.h"
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
    UlamType * it = m_state.getUlamTypeByIndex(Nav);  //Nav init return type
    u32 numErrorsFound = 0;

    //first label argument types; used to pinpoint the exact function symbol in case of overloading
    std::vector<UlamType *> argTypes;

    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	UlamType * argtype = m_argumentNodes[i]->checkAndLabelType();  //plus side-effect
	argTypes.push_back(argtype);
      }

    //look up in class block, and match argument types to parameters
    assert(m_funcSymbol == NULL);
    
    Symbol * fnsymptr;
    if(m_state.m_classBlock->isFuncIdInScope(m_functionNameTok.m_dataindex,fnsymptr))
      {
	// still need to pinpoint the SymbolFunction for m_funcSymbol! currently requires exact match
	if(!((SymbolFunctionName *) fnsymptr)->findMatchingFunction(argTypes, m_funcSymbol))
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.m_pool.getDataAsString(m_functionNameTok.m_dataindex).c_str() << "> has no defined function with " << m_argumentNodes.size() << " matching argument types, and cannot be called";
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


    if(!numErrorsFound)
      {
	it = m_funcSymbol->getUlamType();
      }
    
    setNodeType(it);
    return it;
  }



  EvalStatus NodeFunctionCall::eval()
  {
    assert(m_funcSymbol);
    NodeBlockFunctionDefinition * func = m_funcSymbol->getFunctionNode();
    assert(func);    

    evalNodeProlog(0); //new current frame pointer on node eval stack
    
    u32 argsPushed = 0;
    EvalStatus evs;
    // place values of arguments on call stack (reverse order) before calling function
    for(s32 i= m_argumentNodes.size() - 1; i >= 0; i--)
      {
	UlamType * argType = m_argumentNodes[i]->getNodeType();
	// arrays are handled by callstack, and passed by value
	u32 slots = makeRoomForNodeType(argType); //for eval return
	
	evs = m_argumentNodes[i]->eval();
	if(evs != NORMAL)
	  {
	    evalNodeEpilog();
	    return evs;
	  }

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
      } //done with args

    //push return value last (on both STACKS for now) 
    UlamType * rtnType = m_funcSymbol->getUlamType();
    u32 rtnslots = makeRoomForNodeType(rtnType);
    makeRoomForNodeType(rtnType, STACK);

    assert(rtnType->getArraySize() > 0 ? rtnslots == rtnType->getArraySize() : rtnslots == 1); 
    //********************************************    

    evs = func->eval();   //NodeBlockFunctionDefinition..
    if(evs != NORMAL)
      {
	assert(evs != RETURN);
	m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args and return slots on callstack
	evalNodeEpilog();
	return evs;
      }
    //**********************************************

    UlamValue rtnPtr(rtnType, 1, true, EVALRETURN);  //positive to current frame pointer
    assignReturnValueToStack(rtnPtr);                //in return space on eval stack;

    m_state.m_funcCallStack.popArgs(argsPushed+rtnslots); //drops all the args and return slots on callstack

    evalNodeEpilog();  //clears out the node eval stack
    return NORMAL;
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


  void NodeFunctionCall::genCode(File * fp)
  {
    fp->write(m_funcSymbol->getMangledName(&m_state).c_str());
    fp->write("(");
    for(u32 i = 0; i < m_argumentNodes.size(); i++)
      {
	if(i>0)
	  fp->write(", ");

	m_argumentNodes[i]->genCode(fp);	
      }
    fp->write(")");

  }
} //end MFM
