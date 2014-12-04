#include <stdio.h>
#include "NodeBinaryOpLogical.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpLogical::NodeBinaryOpLogical(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpLogical::~NodeBinaryOpLogical()
  {}


  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  void NodeBinaryOpLogical::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(0);
    assert(slots);
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	// leverage case when both are packed, for logical operations
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



  UTI NodeBinaryOpLogical::calcNodeType(UTI lt, UTI rt)  //logical
  {
    UTI newType = Nav; //init

    // all logical operations are performed as Bool.BITSPERBOOL.-1 
    if(m_state.isScalar(lt) && m_state.isScalar(rt))
      {
	newType = Bool;
      } //both scalars
    else
      { 
	//array op scalar: defer since the question of matrix operations is unclear at this time.
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types, LHS: <" << m_state.getUlamTypeNameByIndex(lt).c_str() << ">, RHS: <" << m_state.getUlamTypeNameByIndex(rt).c_str() << "> for binary logical operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return newType;
  } //calcNodeType


  const std::string NodeBinaryOpLogical::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_LogicalOr";  determined by each op

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


} //end MFM
