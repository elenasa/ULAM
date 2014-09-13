#include "CompilerState.h"
#include <iostream>

namespace MFM {

  Node::Node(CompilerState & state): m_state(state), m_storeIntoAble(false), m_nodeUType(Nav) {}  


  void Node::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)    
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }

  // any node above assignexpr is not storeintoable;
  // and has no type (e.g. statements, statement, block, program)
  UTI Node::checkAndLabelType()
  { 
    m_nodeUType = Nav;
    m_storeIntoAble = false;
    return m_nodeUType; 
  }


  UTI Node::getNodeType()
  {
    return m_nodeUType;
  }


  void Node::setNodeType(UTI ut)
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

  bool Node::installSymbolVariable(Token atok, u32 arraysize, Symbol *& asymptr)
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
  u32 Node::makeRoomForNodeType(UTI type, STORAGE where)
  {
    if(type == Void)
      return 0;

    u32 slots;
    u32 arraysize = m_state.getArraySize(type);
    PACKFIT packRtn = m_state.determinePackable(type);
    if(WritePacked(packRtn))
      slots = makeRoomForSlots(1, where);  //=1 for scalar or packed array
    else
      slots = makeRoomForSlots((arraysize > 0 ? arraysize : 1), where);  //=1 for scalar

    return slots;
  }

  /*
#if 0
    // good stuff! but overkill for this function XXX
    // push a Ptr "header" for array first; assumption is unpacked here!
    if(arraysize > 0)
      {
	//get next relative slot for Ptr, and point to next one for base of array [0]
	u32 nextSlot;
	if(where == EVALRETURN)
	  nextSlot = m_state.m_nodeEvalStack.getRelativeTopOfStackNextSlot();
	else if (where == STACK)
	  nextSlot = m_state.m_funcCallStack.getRelativeTopOfStackNextSlot();
	else
	  assert(0);

	UlamValue ptrUV = UlamValue::makePtr(nextSlot+1, where, type, false);

	if(where == EVALRETURN)
	  m_state.m_nodeEvalStack.pushArg(ptrUV, m_state);
	else if (where == STACK)
	  m_state.m_funcCallStack.pushArg(ptrUV, m_state);
	else
	  assert(0);
      }

    //push copies of individual array element singletons
    UTI scalarTypidx = m_state.getUlamTypeAsScalar(type);
    u32 typeLen = m_state.getBitSize(type);
    UlamValue rtnUV = makeImmediate(scalarTypidx, 0, typeLen);  

    s32 pushsize = (arraysize > 0 ? arraysize : 1); //=1 for scalar
    while(pushsize-- > 0)
      {
	if(where == EVALRETURN)
	  m_state.m_nodeEvalStack.pushArg(rtnUV, m_state);    //last ones
	else if (where == STACK)
	  m_state.m_funcCallStack.pushArg(rtnUV, m_state);    //last ones
	else
	  assert(0);
      }

    return  (arraysize > 0 ? arraysize + 1 : 1);  //=1 for scalar; +Ptr for arrays
 }
#endif 
  */

  u32 Node::makeRoomForSlots(u32 slots, STORAGE where)
  {
    //push copies of temporary UV (e.g. UVPtr)
    UlamValue tmpUV = UlamValue::makeImmediate(Nav, 0, 1);  
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
    if(rtnUVtype == Void)
      return;

    if(rtnUVtype == Ptr)  //unpacked array
      {
	rtnUVtype = rtnUV.getPtrTargetType();
      }

    assert(rtnUVtype == getNodeType());

    u32 rtnUVarraysize = m_state.getArraySize(rtnUVtype);
    PACKFIT packedRtn = m_state.determinePackable(rtnUVtype);
    s32 arraysize;
    // save results in the stackframe for caller;
    // copies each element of the 'unpacked' array by value, 
    // in reverse order ([0] is last at bottom)
    if(WritePacked(packedRtn))
      arraysize = -1;
    else
      arraysize = (rtnUVarraysize > 0 ? -rtnUVarraysize : -1); 

    //where to put the return value..'return' statement uses STACK
    UlamValue rtnPtr = UlamValue::makePtr(arraysize, where, rtnUVtype, packedRtn, m_state);
    m_state.assignValue(rtnPtr, rtnUV);
  }


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


  void Node::genCode(File * fp)
  {
    m_state.indent(fp);
    fp->write("virtual void ");
    fp->write(prettyNodeName().c_str());
    fp->write("::genCode(File * fp){} is needed!!\n");
  }

} //end MFM
