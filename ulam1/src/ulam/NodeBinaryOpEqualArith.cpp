#include "NodeBinaryOpEqualArith.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArith::NodeBinaryOpEqualArith(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpEqualArith::~NodeBinaryOpEqualArith(){}


  void NodeBinaryOpEqualArith::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	doBinaryOperationArray(lslot, rslot, slots);
      }
  } //end dobinaryop


} //end MFM
