#include <stdio.h>
#include "NodeBinaryOpArith.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArith::NodeBinaryOpArith(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpArith::~NodeBinaryOpArith()
  { }


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
    if(m_state.isScalar(getNodeType()))  //not an array
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
    UTI newType = Nav; //init

    // except for 2 Unsigned, all arithmetic operations are performed as Int.32.-1
    // if one is unsigned, and the other isn't -> output warning, but Signed Int wins.
    // Class (i.e. quark) + anything goes to Int.32
 
    if( m_state.isScalar(lt) && m_state.isScalar(rt))
      {
	//return constant expressions as constants; 
	if(lt == rt && m_state.isConstant(lt))
	  return lt;   // or perhaps return constant Int? what's an unsigned constant?

	newType = Int;

	ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

	if(ltypEnum == Unsigned && rtypEnum == Unsigned)
	  {
	    newType = Unsigned;   //check if isConstant?
	  }
	else
	  {
	    // not both unsigned, but one is, so mixing signed and
	    // unsigned gets a warning, but still uses signed Int.
	    if(ltypEnum == Unsigned || rtypEnum == Unsigned)
	      {
		std::ostringstream msg;
		msg << "Attempting to mix signed and unsigned types, LHS: <" << m_state.getUlamTypeNameByIndex(lt).c_str() << ">, RHS: <" << m_state.getUlamTypeNameByIndex(rt).c_str() << "> for binary operator" << getName() << " ";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
		//output warning
	      }
	  }
      } //both scalars
    else
      { 
	//#define SUPPORT_ARITHMETIC_ARRAY_OPS
#ifdef SUPPORT_ARITHMETIC_ARRAY_OPS
	// Conflicted: we don't like the idea that the type might be
	// different for arrays than scalars; casting occurring differently.
	// besides, for arithmetic ops, unlike logical ops, we have to do each 
	// op separately anyway, so no big win (let ulam programmer do the loop).
	// let arrays of same types through ??? Is SO for op equals, btw.
	if(lt == rt)
	  {
	    return lt;  
	  }
#endif //SUPPORT_ARITHMETIC_ARRAY_OPS

	//array op scalar: defer since the question of matrix operations is unclear at this time.
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types, LHS: <" << m_state.getUlamTypeNameByIndex(lt).c_str() << ">, RHS: <" << m_state.getUlamTypeNameByIndex(rt).c_str() << "> for binary operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return newType;
  }

} //end MFM
