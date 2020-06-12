#include "NodeBinaryOpEqualArithPreIncr.h"
#include "NodeBinaryOpArithAdd.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArithPreIncr::NodeBinaryOpEqualArithPreIncr(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualArith(left,right,state) {}

  NodeBinaryOpEqualArithPreIncr::NodeBinaryOpEqualArithPreIncr(const NodeBinaryOpEqualArithPreIncr& ref) : NodeBinaryOpEqualArith(ref) {}

  NodeBinaryOpEqualArithPreIncr::~NodeBinaryOpEqualArithPreIncr(){}

  Node * NodeBinaryOpEqualArithPreIncr::instantiate()
  {
    return new NodeBinaryOpEqualArithPreIncr(*this);
  }

  const char * NodeBinaryOpEqualArithPreIncr::getName()
  {
    return "+=";
  }

  const std::string NodeBinaryOpEqualArithPreIncr::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpEqualArithPreIncr::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpAdd" << NodeBinaryOpEqualArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  Node * NodeBinaryOpEqualArithPreIncr::buildOperatorOverloadFuncCallNode()
  {
    //pre incr/decr has no argument
    delete m_nodeRight;
    m_nodeRight = NULL;

    return Node::buildOperatorOverloadFuncCallNodeHelper(m_nodeLeft, NULL, "++");
  } //buildOperatorOverloadFuncCallNode

  UlamValue NodeBinaryOpEqualArithPreIncr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpEqualArithPreIncr::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpAddInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpAddUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpAddBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpAddUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpEqualArithPreIncr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpAddInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpAddUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpAddBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpAddUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return;
  }

} //end MFM
