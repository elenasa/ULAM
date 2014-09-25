#include "NodeBinaryOpEqualBitwise.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualBitwise::NodeBinaryOpEqualBitwise(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpEqualBitwise::~NodeBinaryOpEqualBitwise(){}


  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  void NodeBinaryOpEqualBitwise::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	// leverage case when both are packed, for logical bitwise operations
	if(m_state.determinePackable(nuti) == PACKEDLOADABLE)
	  {
	    doBinaryOperationImmediate(lslot, rslot, slots);
	  }
	else
	  { 
	    doBinaryOperationArray(lslot, rslot, slots);
	  }
      }
  } //end dobinaryop
  

} //end MFM
