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

    UlamType * lut = m_state.getUlamTypeByIndex(leftType);
    if((lut->getUlamTypeEnum() == Class))
      {
	Node * newnode = buildOperatorOverloadFuncCallNode(); //virtual
	if(newnode)
	  {
	    AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
	    assert(swapOk);

	    m_nodeLeft = NULL; //recycle as memberselect
	    m_nodeRight = NULL; //recycle as func call arg

	    delete this; //suicide is painless..

	    return newnode->checkAndLabelType();
	  }
	//else should fail again as non-primitive;
      } //done

    UTI newType = calcNodeType(leftType, rightType); //Bits, or Nav error
    setNodeType(newType);
    if(newType == Hzy)
      m_state.setGoAgain();

    Node::setStoreIntoAble(TBOOL_FALSE);

    if(m_state.isComplete(newType))
      {
	UlamType * lut = m_state.getUlamTypeByIndex(leftType);
	ULAMTYPE letyp = lut->getUlamTypeEnum();
	if(letyp != Bits)
	  {
	    s32 lbs = UNKNOWNSIZE, wordsize = UNKNOWNSIZE;
	    NodeBinaryOp::calcBitsizeForResultInBits(leftType, lbs, wordsize);
	    //auto cast to Bits, a downhill cast. use LHS bitsize (not result bitsize
	    //to avoid exhaustive casting-up to 32 bits, e.g. ulamexports).
	    UlamKeyTypeSignature newleftkey(m_state.m_pool.getIndexForDataString("Bits"), lbs);
	    UTI newleftType = m_state.makeUlamType(newleftkey, Bits, UC_NOTACLASS);
	    if(!Node::makeCastingNode(m_nodeLeft, newleftType, m_nodeLeft))
	      {
		newType = Nav;
		setNodeType(Nav);
	      }
	  }
	//shift by unsigned type; cast if need be. safety checked by calcNodeType.
	if(m_state.getUlamTypeByIndex(rightType)->getUlamTypeEnum() != Unsigned)
	  {
	    if(!Node::makeCastingNode(m_nodeRight, Unsigned, m_nodeRight))
	      {
		newType = Nav;
		setNodeType(Nav);
	      }
	  }
      } //complete

    if((newType != Nav) && isAConstant() && m_nodeLeft->isReadyConstant() && m_nodeRight->isReadyConstant())
      return constantFold();

    return newType;
  } //checkAndLabelType

  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  bool NodeBinaryOpShift::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti)) //not an array
      {
	return doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	// leverage case when both are packed, for shift operations
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

  UTI NodeBinaryOpShift::calcNodeType(UTI lt, UTI rt)
  {
    if(!m_state.neitherNAVokUTItoContinue(lt, rt))
      return Nav;

    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
      return Hzy;

    //no atoms, elements, nor void as either operand
    if(!NodeBinaryOp::checkForPrimitiveNotVoidTypes(lt, rt))
	return Nav;

    UTI newType = Nav; //init to Nav
    // LHS must be Bits..
    if(NodeBinaryOp::checkScalarTypesOnly(lt, rt))
      {
	bool bok = true;
	//s32 lbs = resultBitsize(lt, rt);
	//e.g. t3432, t3463, t3467
	s32 lbs = UNKNOWNSIZE, wordsize = UNKNOWNSIZE;
	NodeBinaryOp::calcBitsizeForResultInBits(lt, lbs, wordsize);

	//will auto cast to Bits, a downhill cast. using LHS bitsize (not result size).
	UlamKeyTypeSignature newleftkey(m_state.m_pool.getIndexForDataString("Bits"), lbs);
	UTI newleftType = m_state.makeUlamType(newleftkey, Bits, UC_NOTACLASS);

	FORECAST lscr = m_nodeLeft->safeToCastTo(newleftType);
	if(lscr != CAST_CLEAR)
	  {
	    std::ostringstream msg;
	    msg << "Bits is the supported type for shift operation ";
	    msg << getName() << "; Suggest casting ";
	    msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str();
	    msg << " to Bits";
	    if(lbs > 0)
	      msg << "(" << lbs << ")";
	    if(lscr == CAST_HAZY)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		newType = Hzy;
		bok = false;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		bok = false; //Nav;
	      }
	  }

	// RHS of shift must be Unsigned, or positive constant.
	//note: C implementations typically shift by the lower 5 bits (6 for 64-bits) only.
	FORECAST rscr = m_nodeRight->safeToCastTo(Unsigned);
	if(rscr != CAST_CLEAR)
	  {
	    std::ostringstream msg;
	    msg << "Unsigned is the supported type for shift distance, operation ";
	    msg << getName() << "; Suggest casting ";
	    msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	    msg << " to Unsigned";
	    if(rscr == CAST_HAZY)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		newType = Hzy;
		bok = false;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		bok = false; //Nav;
	      }
	  }

	//check for big shift values
	if(m_nodeRight->isAConstant() && m_nodeRight->isReadyConstant())
	  {
	    if(m_nodeRight->isWordSizeConstant())
	      {
		std::ostringstream msg;
		msg << "Shift distance greater than data width, operation ";
		msg << getName();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	      }
	  }

	if(bok)
	  {
	    //e.g. t3432, t3463, t3467, t3200, t3201, t3372, t3507, t3509
	    s32 newbs = resultBitsize(lt, rt);
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bits"), newbs);
	    newType = m_state.makeUlamType(newkey, Bits, UC_NOTACLASS);
	  }
	//else newType = Nav or Hzy
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
	m_state.abortUndefinedUlamPrimitiveType();
	methodname << "NAV";
	break;
      };
    methodname << nut->getTotalWordSize();
    return methodname.str();
  } // methodNameForCodeGen

} //end MFM
