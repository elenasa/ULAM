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

  bool NodeBinaryOpArith::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType())) //not an array
      {
	return doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
#ifdef SUPPORT_ARITHMETIC_ARRAY_OPS
	return doBinaryOperationArray(lslot, rslot, slots);
#else
	assert(0);
#endif //defined below...
      }
    return false;
  } //end dobinaryop

  UTI NodeBinaryOpArith::calcNodeType(UTI lt, UTI rt)
  {
    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
      return Hzy;

    //no atoms, elements nor void as either operand
    if(!NodeBinaryOp::checkForPrimitiveTypes(lt, rt))
      return Nav; //err output

    // only int, unsigned, unary types; not bool, bits, etc..
    if(!NodeBinaryOp::checkForNumericTypes(lt, rt))
      return Nav; //err output

    UTI newType = Nav; //init

    // all operations are performed as Int(32) or Unsigned(32) in CastOps.h
    // if one is unsigned, and the other isn't -> output error if unsafe;
    // Signed Int wins, unless its a constant.
    // Class (i.e. quark) + anything goes to Int(32)
    if(checkScalarTypesOnly(lt, rt))
      {
	s32 newbs = resultBitsize(lt, rt); //op-specific
	ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

	// treat Unary using Unsigned rules
	if(ltypEnum == Unary)
	  ltypEnum = Unsigned;

	if(rtypEnum == Unary)
	  rtypEnum = Unsigned;

	if(ltypEnum == Unsigned && rtypEnum == Unsigned)
	  {
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Unsigned"), newbs);
	    newType = m_state.makeUlamType(newkey, Unsigned);
	  }
	else
	  {
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Int"), newbs);
	    newType = m_state.makeUlamType(newkey, Int);
	  }

	NodeBinaryOp::checkSafeToCastTo(newType); //Nav, Hzy, or nochange; outputs error msg
      } //both scalars
    return newType;
  } //calcNodeType

} //end MFM
