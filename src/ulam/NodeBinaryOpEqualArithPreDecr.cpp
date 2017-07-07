#include "NodeBinaryOpEqualArithPreDecr.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArithPreDecr::NodeBinaryOpEqualArithPreDecr(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualArith(left,right,state) {}

  NodeBinaryOpEqualArithPreDecr::NodeBinaryOpEqualArithPreDecr(const NodeBinaryOpEqualArithPreDecr& ref) : NodeBinaryOpEqualArith(ref) {}

  NodeBinaryOpEqualArithPreDecr::~NodeBinaryOpEqualArithPreDecr(){}

  Node * NodeBinaryOpEqualArithPreDecr::instantiate()
  {
    return new NodeBinaryOpEqualArithPreDecr(*this);
  }

  const char * NodeBinaryOpEqualArithPreDecr::getName()
  {
    return "-=";
  }

  const std::string NodeBinaryOpEqualArithPreDecr::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpEqualArithPreDecr::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpSubtract" << NodeBinaryOpEqualArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  Node * NodeBinaryOpEqualArithPreDecr::buildOperatorOverloadFuncCallNode()
  {
    Token identTok;
    TokenType opTokType = Token::getTokenTypeFromString("--");
    assert(opTokType != TOK_LAST_ONE);
    Token opTok(opTokType, getNodeLocation(), 0);
    u32 opolId = Token::getOperatorOverloadFullNameId(opTok, &m_state);
    assert(opolId != 0);

    identTok.init(TOK_IDENTIFIER, getNodeLocation(), opolId);

    //fill in func symbol during type labeling;
    NodeFunctionCall * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
    assert(fcallNode);
    fcallNode->setNodeLocation(identTok.m_locator);

    //pre incr/decr has no argument
    //    fcallNode->addArgument(m_nodeRight);
    delete m_nodeRight;
    m_nodeRight = NULL;

    NodeMemberSelect * mselectNode = new NodeMemberSelect(m_nodeLeft, fcallNode, m_state);
    assert(mselectNode);
    mselectNode->setNodeLocation(identTok.m_locator);

    //redo check and type labeling done by caller!!
    return mselectNode; //replace right node with new branch
  } //buildOperatorOverloadFuncCallNode

  UlamValue NodeBinaryOpEqualArithPreDecr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpSubtractUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpEqualArithPreDecr::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpSubtractUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpEqualArithPreDecr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpSubtractInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpSubtractUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpSubtractBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpSubtractUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
