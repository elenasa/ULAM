#include <stdio.h>
#include "NodeBinaryOpLogical.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpLogical::NodeBinaryOpLogical(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}
  NodeBinaryOpLogical::NodeBinaryOpLogical(const NodeBinaryOpLogical& ref) : NodeBinaryOp(ref) {}
  NodeBinaryOpLogical::~NodeBinaryOpLogical()  {}

  // not used for logical op
  void NodeBinaryOpLogical::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(0);
  } //end dobinaryop



  UTI NodeBinaryOpLogical::calcNodeType(UTI lt, UTI rt)  //logical
  {
    UTI newType = Nav; //init

    // all logical operations are performed as Bool.BITSPERBOOL.-1
    if(m_state.isScalar(lt) && m_state.isScalar(rt))
      {
	if(m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum() == Bool)
	  newType = lt; //any size bool
	else if(m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum() == Bool)
	  newType = rt; //any size bool
	else
	  newType = Bool; //default
      } //both scalars
    else
      {
	//array op scalar: defer since the question of matrix operations is unclear at this time.
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types, LHS: " << m_state.getUlamTypeNameByIndex(lt).c_str() << ", RHS: " << m_state.getUlamTypeNameByIndex(rt).c_str() << " for binary logical operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return newType;
  } //calcNodeType


  const std::string NodeBinaryOpLogical::methodNameForCodeGen()
  {
    assert(0);
    return "notapplicable_logicalops";
  } // methodNameForCodeGen

} //end MFM
