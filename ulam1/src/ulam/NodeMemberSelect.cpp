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


  UlamType * NodeMemberSelect::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UlamType * lut = m_nodeLeft->checkAndLabelType(); //side-effect
    if(lut->getUlamClassType() == UC_NOTACLASS)
      {
	//error!
	// must be a 'Class' type, either quark or element
	setNodeType(m_state.getUlamTypeByIndex(Nav));
	return getNodeType();
      }
    
    std::string className = lut->getUlamTypeNameBrief(&m_state); //help me debug

    SymbolClass * csym = NULL;
    assert(m_state.alreadyDefinedSymbolClass(lut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym));

    NodeBlockClass * memberClassNode = csym->getClassBlockNode();
    assert(memberClassNode);

    //set up compiler state to use the member class block for symbol searches
    m_state.m_currentMemberClassBlock = memberClassNode;
    m_state.m_useMemberBlock = true;

    UlamType * rightType = m_nodeRight->checkAndLabelType();
    
    //clear up compiler state to no longer use the member class block for symbol searches
    m_state.m_useMemberBlock = false;
    m_state.m_currentMemberClassBlock = NULL;

    setNodeType(rightType);

    setStoreIntoAble(m_nodeRight->isStoreIntoAble());
    return getNodeType();
  }


  void NodeMemberSelect::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    UlamValue pluv = m_state.m_nodeEvalStack.getFrameSlotAt(lslot);
    UlamValue ruvPtr(getNodeType(), rslot, true, EVALRETURN);  //positive to current frame pointer
    //assignUlamValue(pluv,ruvPtr);

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(ruvPtr);
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
