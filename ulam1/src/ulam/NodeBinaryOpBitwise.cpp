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
	// leverage case when both are packed, for bitwise operations
	if(m_state.determinePackable(nuti) == PACKEDLOADABLE)
	  {
	    doBinaryOperationImmediate(lslot, rslot, slots);
	  }
	else
	  {
	    doBinaryOperationArray(lslot, rslot, slots);
	  }
      }
  } //dobinaryoperation

  UTI NodeBinaryOpBitwise::calcNodeType(UTI lt, UTI rt)  //bitwise
  {
    UTI newType = Nav;  //init
    ULAMTYPECOMPARERESULTS uticr = UlamType::compare(lt, rt, m_state);
    if(uticr == UTIC_DONTKNOW)
      {
	std::ostringstream msg;
	msg << "Calculating 'incomplete' bitwise node types: ";
	msg << m_state.getUlamTypeNameByIndex(lt).c_str() << " and ";
	msg << m_state.getUlamTypeNameByIndex(rt).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return Nav;
      }

    if(uticr == UTIC_SAME)
      {
	ULAMTYPE etyp = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	if(!m_state.isScalar(lt) && (etyp == Unary || etyp == Bool))
	  {
	    std::ostringstream msg;
	    msg << "Nonscalar types Bool and Unary are not currently supported ";
	    msg << "for binary bitwise operator" << getName();
	    msg << "; suggest writing a loop for: ";
	    msg << m_state.getUlamTypeNameByIndex(lt).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	else
	  newType = rt; //maintain type
      }
    else
      {
	if(m_state.isScalar(lt) && m_state.isScalar(rt))
	  {
	    newType = Bits; //default is Bits
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Incompatible (nonscalar) types, LHS: ";
	    msg << m_state.getUlamTypeNameByIndex(lt).c_str();
	    msg << ", RHS: " << m_state.getUlamTypeNameByIndex(rt).c_str();
	    msg << " for binary bitwise operator" << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    return newType;
  } //calcNodeType

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
