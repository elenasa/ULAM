#include <stdio.h>
#include "NodeBinaryOpArith.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArith::NodeBinaryOpArith(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpArith::NodeBinaryOpArith(const NodeBinaryOpArith& ref) : NodeBinaryOp(ref) {}

  NodeBinaryOpArith::~NodeBinaryOpArith() {}

  const std::string NodeBinaryOpArith::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_BitwiseOr";  determined by each op
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());

    // common part of name
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	methodname << "Int";
	break;
      case Unsigned:
	methodname << "Unsigned";
	break;
      default:
	assert(0);
	methodname << "NAV";
	break;
      };
    methodname << nut->getTotalWordSize();
    return methodname.str();
  } // methodNameForCodeGen

  void NodeBinaryOpArith::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType())) //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
#ifdef SUPPORT_ARITHMETIC_ARRAY_OPS
	doBinaryOperationArray(lslot, rslot, slots);
#else
	assert(0);
#endif //defined below...
      }
  } //end dobinaryop

  UTI NodeBinaryOpArith::calcNodeType(UTI lt, UTI rt)
  {
    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
      return Nav;

    UTI newType = Nav; //init

    // all operations are performed as Int(32) or Unsigned(32) in CastOps.h
    // if one is unsigned, and the other isn't -> output error if unsafe;
    // Signed Int wins, unless its a constant.
    // Class (i.e. quark) + anything goes to Int(32)
    if(checkScalarTypesOnly(lt, rt))
      {
	s32 newbs = NodeBinaryOp::maxBitsize(lt, rt);
	ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

	if(ltypEnum == Bits || rtypEnum == Bits)
	  {
	    std::ostringstream msg;
	    msg << "Incompatible Bits type for binary operator";
	    msg << getName() << ". Suggest casting to an ordered type first";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return Nav;
	  }

	// treat Bool and Unary using Unsigned rules
	if(ltypEnum == Bool || ltypEnum == Unary)
	  ltypEnum = Unsigned;

	if(rtypEnum == Bool || rtypEnum == Unary)
	  rtypEnum = Unsigned;

	if(ltypEnum == Unsigned && rtypEnum == Unsigned)
	  {
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Unsigned"), newbs);
	    newType = m_state.makeUlamType(newkey, Unsigned);
	    return newType;
	  }

	UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Int"), newbs);
	newType = m_state.makeUlamType(newkey, Int);

	if(!NodeBinaryOp::checkAnyConstantsFit(ltypEnum, rtypEnum, newType))
	  return newType; //outputs errors if not ok, Nav returned

	NodeBinaryOp::checkForMixedSignsOfVariables(ltypEnum, rtypEnum, lt, rt, newType); //ref
      } //both scalars
    return newType;
  } //calcNodeType

} //end MFM
