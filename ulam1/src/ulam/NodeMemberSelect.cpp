#include "NodeMemberSelect.h"
#include "CompilerState.h"

namespace MFM {

  NodeMemberSelect::NodeMemberSelect(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeMemberSelect::~NodeMemberSelect(){}


  void NodeMemberSelect::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }


  const char * NodeMemberSelect::getName()
  {
    return ".";
  }


  const std::string NodeMemberSelect::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeMemberSelect::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UTI luti = m_nodeLeft->checkAndLabelType(); //side-effect
    UlamType * lut = m_state.getUlamTypeByIndex(luti);

    if(lut->getUlamClass() == UC_NOTACLASS)
      {
	//error!
	// must be a 'Class' type, either quark or element
	setNodeType(Nav);
	return getNodeType();
      }
    
    std::string className = m_state.getUlamTypeNameBriefByIndex(luti); //help me debug

    SymbolClass * csym = NULL;
    assert(m_state.alreadyDefinedSymbolClass(lut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym));

    NodeBlockClass * memberClassNode = csym->getClassBlockNode();
    assert(memberClassNode);

    //set up compiler state to use the member class block for symbol searches
    m_state.m_currentMemberClassBlock = memberClassNode;
    m_state.m_useMemberBlock = true;

    UTI rightType = m_nodeRight->checkAndLabelType();
    
    //clear up compiler state to no longer use the member class block for symbol searches
    m_state.m_useMemberBlock = false;
    m_state.m_currentMemberClassBlock = NULL;

    setNodeType(rightType);

    setStoreIntoAble(m_nodeRight->isStoreIntoAble());
    return getNodeType();
  }


  EvalStatus NodeMemberSelect::eval()
  {
    assert(m_nodeLeft && m_nodeRight);

    evalNodeProlog(0); //new current frame pointer on node eval stack

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //UPDATE selected member (i.e. element or quark) before eval of rhs (i.e. data member or func call)
    //UlamValue pluv = m_state.m_nodeEvalStack.popArg(); //Ptr to atom  ???which way???
    m_state.m_currentObjPtr = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1); //e.g. Ptr to atom


    u32 slot = makeRoomForNodeType(getNodeType());
    evs = m_nodeRight->eval();   //a Node Function Call here, or data member eval
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }


    //Symbol * rsymptr = NULL;
    //if(m_nodeRight->getSymbolPtr(rsymptr) && rsymptr->isFunction())

    //assigns rhs to lhs UV pointer (handles arrays);  
    //also copy result UV to stack, -1 relative to current frame pointer

    if(slot)   //avoid Void's
      doBinaryOperation(1, 1+slot, slot);  //????????

    m_state.m_currentObjPtr = saveCurrentObjectPtr;  //restore current object ptr
    evalNodeEpilog();
    return NORMAL;
  }


  //for eval, want the value of the rhs 
  void NodeMemberSelect::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    //the return value of a function call, or value of a data member
    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot); 

    UlamValue rtnUV;    
    UTI ruti = getNodeType();
    PACKFIT packFit = m_state.determinePackable(ruti);

    if(m_state.isScalar(ruti) || WritePacked(packFit))
      {
	rtnUV = ruv; 
      }
    else
      {
	//make a ptr to an unpacked array, base[0] ? [pls test]
	rtnUV = UlamValue::makePtr(rslot, EVALRETURN, ruti, UNPACKED, m_state);
      }

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(rtnUV);
  }

  

  EvalStatus NodeMemberSelect::evalToStoreInto()
  {
    evalNodeProlog(0);

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();  
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //UPDATE selected member (i.e. element or quark) before eval of rhs (i.e. data member or func call)
    //m_state.m_currentObjPtr = m_state.m_nodeEvalStack.popArg(); //e.g. Ptr to atom
    m_state.m_currentObjPtr = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1); //e.g. Ptr to atom

    makeRoomForSlots(1); //always 1 slot for ptr
    evs = m_nodeRight->evalToStoreInto();  
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //UlamValue ruvPtr = m_state.m_nodeEvalStack.popArg();
    UlamValue ruvPtr = m_state.m_nodeEvalStack.loadUlamValueFromSlot(2);

    assignReturnValuePtrToStack(ruvPtr);

    m_state.m_currentObjPtr = saveCurrentObjectPtr;  //restore current object ptr *************
    
    evalNodeEpilog();
    return NORMAL;
  }


  UlamValue NodeMemberSelect::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    assert(0); //unused
    return UlamValue();
  }


  void NodeMemberSelect::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //unused
  }


  //differs from NodeBinaryOp, no spaces surrounding the operator .
  void NodeMemberSelect::genCode(File * fp)
  {
    assert(m_nodeLeft && m_nodeRight);
    m_nodeLeft->genCode(fp);
    fp->write(getName());
    m_nodeRight->genCode(fp);
  }

} //end MFM
