#include <stdio.h>
#include "NodeBinaryOpLogical.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpLogical::NodeBinaryOpLogical(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpLogical::NodeBinaryOpLogical(const NodeBinaryOpLogical& ref) : NodeBinaryOp(ref) {}

  NodeBinaryOpLogical::~NodeBinaryOpLogical()  {}

  // not used for logical op
  bool NodeBinaryOpLogical::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(0);
    return false;
  } //dobinaryoperation

  UTI NodeBinaryOpLogical::calcNodeType(UTI lt, UTI rt)  //logical
  {
    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
      return Hzy;

    //no atoms, elements nor voids as either operand
    if(!NodeBinaryOp::checkForPrimitiveTypes(lt, rt))
	return Nav;

    UTI newType = Nav; //init
    // all logical operations are performed as Bool.BITSPERBOOL.-1
    if(NodeBinaryOp::checkScalarTypesOnly(lt, rt))
      {
	s32 maxbs = 1;
	FORECAST lscr = m_state.getUlamTypeByIndex(Bool)->safeCast(lt);
	FORECAST rscr = m_state.getUlamTypeByIndex(Bool)->safeCast(rt);

	//check for Bool, or safe Non-Bool to Bool casting cases:
	if(lscr != CAST_CLEAR || rscr != CAST_CLEAR)
	  {
	    std::ostringstream msg;
	    msg << "Bool is the supported type for logical operator";
	    msg << getName() << "; Suggest casting ";
	    msg << m_state.getUlamTypeNameBriefByIndex(lt).c_str() << " and ";
	    msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	    msg << " to Bool";
	    if(lscr == CAST_BAD || rscr == CAST_BAD)
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //hazy
	  }
	else
	  {
	    s32 lbs = m_state.getBitSize(lt);
	    s32 rbs = m_state.getBitSize(rt);
	    maxbs = (lbs > rbs ? lbs : rbs);
	    //both bool. ok to cast. use larger bool bitsize.
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bool"), maxbs);
	    newType = m_state.makeUlamType(newkey, Bool);
	  }
      } //both scalars
    return newType;
  } //calcNodeType

  const std::string NodeBinaryOpLogical::methodNameForCodeGen()
  {
    assert(0);
    return "notapplicable_logicalops";
  } // methodNameForCodeGen

} //end MFM
