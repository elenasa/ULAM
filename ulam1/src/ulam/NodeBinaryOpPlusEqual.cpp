#include "NodeBinaryOpPlusEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpPlusEqual::NodeBinaryOpPlusEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpPlusEqual::~NodeBinaryOpPlusEqual(){}


  const char * NodeBinaryOpPlusEqual::getName()
  {
    return "+=";
  }


  const std::string NodeBinaryOpPlusEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  void NodeBinaryOpPlusEqual::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
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


  UlamValue NodeBinaryOpPlusEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, (s32) ldata + (s32) rdata, len);
  }


  void NodeBinaryOpPlusEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    refUV.putData(pos, len, (s32) ldata + (s32) rdata);
  }

} //end MFM
