#include "NodeBinaryOpMultiplyEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpMultiplyEqual::NodeBinaryOpMultiplyEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpMultiplyEqual::~NodeBinaryOpMultiplyEqual(){}


  const char * NodeBinaryOpMultiplyEqual::getName()
  {
    return "*=";
  }


  const std::string NodeBinaryOpMultiplyEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  void NodeBinaryOpMultiplyEqual::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	doBinaryOperationArray(lslot, rslot, slots);
      }
  } //end dobinaryop


  UlamValue NodeBinaryOpMultiplyEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, (s32) ldata * (s32) rdata, len);
  }


  void NodeBinaryOpMultiplyEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, (s32) ldata * (s32) rdata);
  }


} //end MFM
