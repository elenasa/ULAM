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

    if(newType != Nav && m_state.isComplete(newType))
      {
	if(newType != leftType)
	  {
	    if(!makeCastingNode(m_nodeLeft, Int, m_nodeLeft))
	      newType = Nav;
	  }

	//shift ops take unsigned as second arg;
	//note: C implementations typically shift by the lower 5 bits (6 for 64-bits) only.
	UlamType * rut = m_state.getUlamTypeByIndex(rightType);
	ULAMTYPE retype = rut->getUlamTypeEnum();
	if(retype != Unsigned)
	  {
	    //first see if it happens to be a quark that can be cast toInt
	    if(retype == Class && rut->getUlamClass() == UC_QUARK)
	      {
		if(!makeCastingNode(m_nodeRight, Int, m_nodeRight))
		  newType = Nav;
	      }

	    if(!makeCastingNode(m_nodeRight, Unsigned, m_nodeRight))
	      newType = Nav;
	  }
      }

    if(newType != Nav && isAConstant() && m_nodeLeft->isReadyConstant() && m_nodeRight->isReadyConstant())
      return constantFold();

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


    ULAMCLASSTYPE lclass = m_state.getUlamTypeByIndex(lt)->getUlamClass();
    if(lclass == UC_ELEMENT || lt == UAtom)
      {
	std::ostringstream msg;
	msg << "Non-primitive type: <";
	msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	msg << "> is not supported for LHS bitwise shift operator";
	msg << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    ULAMCLASSTYPE rclass = m_state.getUlamTypeByIndex(rt)->getUlamClass();
    if(rclass == UC_ELEMENT || rt == UAtom)
      {
	std::ostringstream msg;
	msg << "Non-primitive type: <";
	msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	msg << "> is not supported for RHS bitwise shift operator";
	msg << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(lclass == UC_QUARK)
      lt = Int;

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

	//if Bool ERR.
	ULAMTYPE etyp = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	if(etyp == Bool)
	  {
	    std::ostringstream msg;
	    msg << "Bool is not currently supported for bitwise shift operator";
	    msg << getName() << "; suggest casting ";
	    msg << m_state.getUlamTypeNameByIndex(lt).c_str();
	    msg << " to Bits";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }

      } //both scalars
    else
      {
	//array op scalar: defer since the question of matrix operations
	// is unclear at this time.
	std::ostringstream msg;
	msg << "Unsupported (nonscalar) types, LHS: ";
	msg << m_state.getUlamTypeNameByIndex(lt).c_str();
	msg << ", RHS: " << m_state.getUlamTypeNameByIndex(rt).c_str();
	msg << " for bitwise shift operator";
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
