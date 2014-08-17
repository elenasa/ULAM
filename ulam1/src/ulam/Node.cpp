#include "CompilerState.h"
#include <iostream>

namespace MFM {

  Node::Node(CompilerState & state): m_state(state), m_storeIntoAble(false), m_nodeUType(NULL) {}  


  void Node::print(File * fp)
  {
    printNodeLocation(fp);
    UlamType * myut = getNodeType();
    char id[255];
    if(!myut)    
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), myut->getUlamTypeName().c_str());
    fp->write(id);

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }

  // any node above assignexpr is not storeintoable;
  // and has no type (e.g. statements, statement, block, program)
  UlamType * Node::checkAndLabelType()
  { 
    m_nodeUType = m_state.getUlamTypeByIndex(Nav);
    m_storeIntoAble = false;
    return m_nodeUType; 
  }


  UlamType * Node::getNodeType()
  {
    return m_nodeUType;
  }


  void Node::setNodeType(UlamType * ut)
  {
    m_nodeUType = ut;
  }


  bool Node::isStoreIntoAble()
  {
    return m_storeIntoAble;
  }


  void Node::setStoreIntoAble(bool s)
  {
    m_storeIntoAble = s;
  }

  // any node above assignexpr is not storeintoable
  EvalStatus Node::evalToStoreInto()
  {
    std::ostringstream msg;
    msg << "Not storeIntoAble: " << m_storeIntoAble;
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
    assert(!isStoreIntoAble());
    return ERROR;
  }


  Locator Node::getNodeLocation()
  {
    return m_nodeLoc;
  }


  void Node::setNodeLocation(Locator loc)
  {
    m_nodeLoc = loc;
  }


  void Node::printNodeLocation(File * fp)
  {
    fp->write(getNodeLocationAsString().c_str());
  }

  std::string Node::getNodeLocationAsString()
  {
    return m_state.getFullLocationAsString(m_nodeLoc);
  }


  bool Node::getSymbolPtr(Symbol *& symptrref)
  {
    return false;
  }

  bool Node::installSymbolTypedef(Token atok, u32 bitsize, u32 arraysize, Symbol *& asymptr)
  {
    return false;
  }

  bool Node::installSymbol(Token atok, u32 arraysize, Symbol *& asymptr)
  {
    return false;
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


  void Node::evalNodeProlog(u32 depth)
  {
    //space for local variables on node eval stack; adjusts current fp;
    m_state.m_nodeEvalStack.addFrameSlots(depth); 
  }


  void Node::evalNodeEpilog()
  {
    //includes any return value and args; adjusts current fp;
    m_state.m_nodeEvalStack.returnFrame();  
  }
  
  //default storage type is the EVALRETURN stack
  u32 Node::makeRoomForNodeType(UlamType * type, STORAGE where)
  {
    if(type == m_state.getUlamTypeByIndex(Void)) 
      return 0;

    UlamType * scalarType = m_state.getUlamTypeAsScalar(type);
    
    //push copies of individual array element singletons
    UlamValue rtnUV(scalarType, 0, IMMEDIATE);  

    u32 pushsize = type->getArraySize();
    pushsize = (pushsize > 0 ? pushsize : 1); //=1 for scalar
   
    for(s32 j = pushsize - 1; j >=0; j--)
      {
	if(where == EVALRETURN)
	  m_state.m_nodeEvalStack.pushArg(rtnUV, m_state);    //last ones
	else if (where == STACK)
	  m_state.m_funcCallStack.pushArg(rtnUV, m_state);    //last ones
	else
	  assert(0);
      }
    return pushsize;
  }
 

  u32 Node::makeRoomForSlots(u32 slots)
  {
    //push copies of temporary UV (e.g. UVPtr)
    UlamValue tmpUV(m_state.getUlamTypeByIndex(Nav), 0, IMMEDIATE);  
    for(u32 j = 0; j < slots; j++)
      {
	m_state.m_nodeEvalStack.pushArg(tmpUV, m_state);
      }
    return slots;
  }


  //in case of arrays, rtnUV is a ptr.
  void Node::assignReturnValueToStack(UlamValue rtnUV, STORAGE where)
  {
    UlamType * rtnUVtype = rtnUV.getUlamValueType(); //==node type
    assert(rtnUVtype == getNodeType());

    if(rtnUVtype == m_state.getUlamTypeByIndex(Void)) 
      return;

    u32 rtnUVarraysize = rtnUVtype->getArraySize();
	  
    // save results in the stackframe for caller;
    // copies each element of the array by value, 
    // in reverse order ([0] is last at bottom)
    s32 arraysize = (rtnUVarraysize > 0 ? -rtnUVarraysize : -1);

    //where to put the return value..'return' statement uses STACK
    UlamValue rtnPtr;

    if(where == EVALRETURN)
      {
	rtnPtr.init(rtnUVtype, arraysize, true, EVALRETURN);
	m_state.m_nodeEvalStack.assignUlamValue(rtnPtr,rtnUV, m_state); //to,from,state
      }
    else if(where == STACK)
      {
      rtnPtr.init(rtnUVtype, arraysize, true, STACK);
      m_state.m_funcCallStack.assignUlamValue(rtnPtr,rtnUV, m_state); //to,from,state
      }
    else
      assert(0);
  }


  //in case of arrays, rtnUV is a ptr.
  void Node::assignReturnValuePtrToStack(UlamValue rtnUVptr)
  {
    UlamType * rtnUVtype = rtnUVptr.getUlamValueType(); //stack or atom PTR

    if(rtnUVtype == m_state.getUlamTypeByIndex(Void)) 
      return;

    UlamValue rtnPtr(rtnUVtype, -1, true, EVALRETURN);
    m_state.m_nodeEvalStack.assignUlamValuePtr(rtnPtr,rtnUVptr, m_state);
  }


  void Node::genCode(File * fp)
  {
    m_state.indent(fp);
    fp->write("virtual void ");
    fp->write(prettyNodeName().c_str());
    fp->write("::genCode(File * fp){} is needed!!\n");
  }

} //end MFM
