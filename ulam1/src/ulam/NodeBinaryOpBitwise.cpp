#include <stdio.h>
#include "NodeBinaryOpBitwise.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwise::NodeBinaryOpBitwise(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpBitwise::~NodeBinaryOpBitwise()
  {}


  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  void NodeBinaryOpBitwise::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
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



  UTI NodeBinaryOpBitwise::calcNodeType(UTI lt, UTI rt)  //bitwise
  {
    UTI newType = Nav;  //init

    if(rt == lt)
      {
	//maintain type
	newType = rt;
      }
    else
      {
	if(m_state.isScalar(lt) && m_state.isScalar(rt))
	  {
	    newType = Int;  //default is Int
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Incompatible (nonscalar) types, LHS: <" << m_state.getUlamTypeNameByIndex(lt).c_str() << ">, RHS: <" << m_state.getUlamTypeNameByIndex(rt).c_str() << "> for binary bitwise operator" << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }	
      }
    return newType;
  }

} //end MFM
