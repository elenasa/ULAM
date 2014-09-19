#include "NodeBinaryOpBitwiseOrEqual.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwiseOrEqual::NodeBinaryOpBitwiseOrEqual(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpBitwiseOrEqual::~NodeBinaryOpBitwiseOrEqual(){}


  const char * NodeBinaryOpBitwiseOrEqual::getName()
  {
    return "|=";
  }


  const std::string NodeBinaryOpBitwiseOrEqual::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  void NodeBinaryOpBitwiseOrEqual::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
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
  

  UlamValue NodeBinaryOpBitwiseOrEqual::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    return UlamValue::makeImmediate(type, ldata | rdata, len);
  }


  void NodeBinaryOpBitwiseOrEqual::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not used, though could be
    refUV.putData(pos, len, ldata | rdata);
  }

} //end MFM
