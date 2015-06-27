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
    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
	return Nav;

    //no atoms, elements, nor void as either operand
    if(!NodeBinaryOp::checkForPrimitiveTypes(lt, rt))
	return Nav;

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
	if(etyp == Bits)
	  return lt; //includes array of bits
      }

    if(NodeBinaryOp::checkScalarTypesOnly(lt, rt))
      {
	SAFECAST scr = SAFE; //avoid constant error msg if neither var is Bits

	ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

	//if not both Bits ERR, except for both constants
	bool lconst = m_nodeLeft->isAConstant();
	bool rconst = m_nodeRight->isAConstant();

	//auto cast when both constants, or both Bits. constant fold later.
	if((lconst && rconst) || (ltypEnum == Bits && rtypEnum == Bits))
	  {
	    s32 newbs = NodeBinaryOp::maxBitsize(lt, rt);
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bits"), newbs);
	    newType = m_state.makeUlamType(newkey, Bits);
	  }
	else if(lconst ^ rconst)
	  {
	    //one or the other but not both are constants; use
	    if(lconst && rtypEnum == Bits)
	      {
		newType = rt;
		scr = m_nodeLeft->safeToCastTo(newType);
	      }
	    else if(rconst && ltypEnum == Bits)
	      {
		newType = lt;
		scr = m_nodeLeft->safeToCastTo(newType);
	      }
	    //else could fail if neither var is Bits

	    if(newType != Nav && scr != SAFE)
	      {
		std::ostringstream msg;
		msg << "Constant <";
		if(lconst)
		  msg << m_nodeLeft->getName();
		if(rconst)
		  msg << m_nodeRight->getName();

		msg <<  "> is not representable as: ";
		msg<< m_state.getUlamTypeNameByIndex(newType).c_str();
		msg << ", for binary operator" << getName() << " ";
		if(scr == UNSAFE)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		else //hazy
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		newType = Nav;
	      }
	  }

	if(newType == Nav && !(ltypEnum == Bits && rtypEnum == Bits))
	  {
	    s32 mbs = NodeBinaryOp::maxBitsize(lt, rt);
	    std::ostringstream msg;
	    msg << "Bits is the supported type for bitwise operator";
	    msg << getName() << "; Suggest casting ";
	    msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str() << " and ";
	    msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	    msg << " to Bits";
	    if(mbs > 0)
	      msg<< "(" << mbs << ")";
	    if(scr == HAZY)
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
      } //both scalars
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
