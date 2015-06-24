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
    UTI newType = calcNodeType(leftType, rightType); //Bits, or Nav error

    setNodeType(newType);
    setStoreIntoAble(false);

    if(newType != Nav && m_state.isComplete(newType))
      {
	if(leftType != newType)
	  {
	    if(!makeCastingNode(m_nodeLeft, newType, m_nodeLeft))
	      {
		newType = Nav;
		setNodeType(Nav);
	      }
	  }
      } //complete

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

    UTI newType = Nav; //init
    // change! LHS must be Bits..up to ulam programmer to cast
    // (including quarks).
    if(NodeBinaryOp::checkScalarTypesOnly(lt, rt))
      {
	newType = lt;

	// RHS must be Unsigned, or positive constant.
	//shift ops take unsigned as second arg;
	//note: C implementations typically shift by the lower 5 bits (6 for 64-bits) only.
	UlamType * rut = m_state.getUlamTypeByIndex(rt);
	ULAMTYPE retype = rut->getUlamTypeEnum();
	bool rconstpos = (m_nodeRight->isAConstant() && m_nodeRight->isReadyConstant() && !m_nodeRight->isNegativeConstant());

	if(!rconstpos && retype != Unsigned)
	  {
	    //this is an unsafe cast, must be explicit!
	    std::ostringstream msg;
	    msg << "Unsigned is the supported type for RHS bitwise shift value, operator";
	    msg << getName() << "; Suggest casting ";
	    msg << m_state.getUlamTypeNameByIndex(rt).c_str();
	    msg << " to Unsigned";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return  Nav;
	  }

	//ERROR if LHS is not Bits, except for constants
	bool lconst = m_nodeLeft->isAConstant();
	if(!lconst)
	  {
	    ULAMTYPE etyp = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	    if(etyp != Bits)
	      {
		std::ostringstream msg;
		msg << "Bits is the supported type for bitwise shift operator";
		msg << getName() << "; Suggest casting ";
		msg << m_state.getUlamTypeNameByIndex(lt).c_str();
		msg << " to Bits(" << m_state.getBitSize(lt) << ")";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		newType = Nav;
	      }
	  }
	else
	  {
	    //auto for constants, downhill cast. use larger bitsize.
	    s32 newbs = NodeBinaryOp::maxBitsize(lt, rt);
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bits"), newbs);
	    newType = m_state.makeUlamType(newkey, Bits);
	  }
      } //both scalars
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
