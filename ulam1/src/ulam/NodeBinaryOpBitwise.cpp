#include <stdio.h>
#include "NodeBinaryOpBitwise.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpBitwise::NodeBinaryOpBitwise(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpBitwise::NodeBinaryOpBitwise(const NodeBinaryOpBitwise& ref) : NodeBinaryOp(ref) {}

  NodeBinaryOpBitwise::~NodeBinaryOpBitwise() {}

  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  bool NodeBinaryOpBitwise::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti))  //not an array
      {
	return doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	// leverage case when both are packed, for bitwise operations
	if(m_state.determinePackable(nuti) == PACKEDLOADABLE)
	  {
	    return doBinaryOperationImmediate(lslot, rslot, slots);
	  }
	else
	  {
	    return doBinaryOperationArray(lslot, rslot, slots);
	  }
      }
    return false;
  } //dobinaryoperation

  UTI NodeBinaryOpBitwise::calcNodeType(UTI lt, UTI rt)  //bitwise
  {
    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
	return Hzy;

    //no atoms, elements, nor void as either operand
    if(!NodeBinaryOp::checkForPrimitiveTypes(lt, rt))
	return Nav;

    UTI newType = Nav;  //init
    if(NodeBinaryOp::checkScalarTypesOnly(lt, rt))
      {
	s32 newbs = resultBitsize(lt, rt);
	UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bits"), newbs);
	newType = m_state.makeUlamType(newkey, Bits);

	if(!NodeBinaryOp::checkSafeToCastTo(newType))
	  newType = Nav; //outputs error msg

      } //both scalars
    return newType;
  } //calcNodeType

  s32 NodeBinaryOpBitwise::resultBitsize(UTI lt, UTI rt)
  {
    s32 lbs = UNKNOWNSIZE, rbs = UNKNOWNSIZE, wordsize = UNKNOWNSIZE;
    NodeBinaryOp::resultBitsizeCalcInBits(lt, rt, lbs, rbs, wordsize);

    return (lbs > rbs ? lbs : rbs);
  } //resultBitsize

  const std::string NodeBinaryOpBitwise::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_BitwiseOr";  determined by each op

    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());

    // common part of name
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
      case Unary:
	methodname << UlamType::getUlamTypeEnumAsString(etyp);
	break;
      default:
	methodname << "Bits";
	break;
      };
    methodname << nut->getTotalWordSize();
    return methodname.str();
  } // methodNameForCodeGen

} //end MFM
