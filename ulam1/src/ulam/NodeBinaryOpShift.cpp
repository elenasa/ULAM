#include <stdio.h>
#include "NodeBinaryOpShift.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpShift::NodeBinaryOpShift(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpShift::NodeBinaryOpShift(const NodeBinaryOpShift& ref) : NodeBinaryOp(ref) {}

  NodeBinaryOpShift::~NodeBinaryOpShift() {}

  UTI NodeBinaryOpShift::checkAndLabelType()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();
    UTI newType = calcNodeType(leftType, rightType); //left, or nav error
    setNodeType(newType);
    setStoreIntoAble(false);

    if(isAConstant() && m_nodeLeft->isReadyConstant() && m_nodeRight->isReadyConstant())
      return constantFold();

    if(newType != Nav && m_state.isComplete(newType))
      {
	assert(newType == leftType);
	if(UlamType::compare(rightType, Int, m_state) != UTIC_SAME) //notsame or dontknow
	  {
	    //m_nodeRight = makeCastingNode(m_nodeRight, Int);
	    if(!makeCastingNode(m_nodeRight, Int, m_nodeRight))
	      newType = Nav;
	  }
      }
    return newType;
  } //checkAndLabelType

  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  void NodeBinaryOpShift::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti)) //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	// leverage case when both are packed, for shift operations
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

  UTI NodeBinaryOpShift::calcNodeType(UTI lt, UTI rt)  //shift
  {
    if(lt == Nav || rt == Nav)
      {
	return Nav;
      }

    UTI newType = Nav; //init
    // all shift operations are performed as lhs type
    if(m_state.isScalar(lt) && m_state.isScalar(rt))
      {
	newType = lt;

	//these "helpful" checks do not consider the possibility of a constant expression XXX
	bool rconst = m_nodeRight->isAConstant();
	if(rconst && m_nodeRight->isReadyConstant() && m_nodeRight->isNegativeConstant())
	  {
	    std::ostringstream msg;
	    msg << "Negative shift! Recommend shifting in the opposite direction, LHS: ";
	    msg << m_state.getUlamTypeNameByIndex(lt).c_str() << ", RHS: ";
	    msg << m_state.getUlamTypeNameByIndex(rt).c_str() << " for binary shift operator";
	    msg << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }
	else if(rconst && m_nodeRight->isReadyConstant() && m_nodeRight->isWordSizeConstant())
	  {
	    std::ostringstream msg;
	    msg << "Bitwise shift by any number greater than or equal to 32 is undefined. LHS: ";
	    msg << m_state.getUlamTypeNameByIndex(lt).c_str() << ", RHS: ";
	    msg << m_state.getUlamTypeNameByIndex(rt).c_str() << " for binary shift operator";
	    msg << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }

	//if Unary or Bool ERR.
	ULAMTYPE etyp = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	if(etyp == Unary || etyp == Bool)
	  {
	    std::ostringstream msg;
	    msg << "Bool and Unary are not currently supported for bitwise shift operator";
	    msg << getName() << "; suggest casting " << m_state.getUlamTypeNameByIndex(lt).c_str();
	    msg << " to Bits";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
      } //both scalars
    else
      {
	//array op scalar: defer since the question of matrix operations is unclear at this time.
	std::ostringstream msg;
	msg << "Unsupported (nonscalar) types, LHS: " << m_state.getUlamTypeNameByIndex(lt).c_str();
	msg << ", RHS: " << m_state.getUlamTypeNameByIndex(rt).c_str() << " for bitwise shift operator";
	msg << getName() << " ; suggest writing a loop";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return newType;
  } //calcNodeType

  const std::string NodeBinaryOpShift::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_ShiftLeft";  determined by each op
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());

    // common part of name
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
      case Unsigned:
      case Bool:
      case Unary:
      case Bits:
	methodname << UlamType::getUlamTypeEnumAsString(etyp);
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
