#include "NodeBinaryOpArithRemainder.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithRemainder::NodeBinaryOpArithRemainder(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}

  NodeBinaryOpArithRemainder::NodeBinaryOpArithRemainder(const NodeBinaryOpArithRemainder& ref) : NodeBinaryOpArith(ref) {}

  NodeBinaryOpArithRemainder::~NodeBinaryOpArithRemainder(){}

  Node * NodeBinaryOpArithRemainder::instantiate()
  {
    return new NodeBinaryOpArithRemainder(*this);
  }

  const char * NodeBinaryOpArithRemainder::getName()
  {
    return "%";
  }

  const std::string NodeBinaryOpArithRemainder::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBinaryOpArithRemainder::castThyselfToResultType(UTI rt, UTI lt, UTI newType, Node *& parentnoderef)
  {
    UTI nuti = newType;
    //because the result bitsize for mod should be the right bitsize
    // create a cast! combining newType's base type and right resultbitsize.
    // could be the same, or "unsafe".
    if(m_state.okUTItoContinue(newType) && m_state.isComplete(newType))
      {
	UlamType * newut = m_state.getUlamTypeByIndex(newType);
	ULAMTYPE typEnum = newut->getUlamTypeEnum();
	u32 convertSize = m_state.getUlamTypeByIndex(rt)->bitsizeToConvertTypeTo(typEnum);
	u32 enumStrIdx = m_state.m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(typEnum));
	UlamKeyTypeSignature tokey(enumStrIdx, convertSize, NONARRAYSIZE);
	ULAMCLASSTYPE newclasstype = newut->getUlamClassType();
	nuti = m_state.makeUlamType(tokey, typEnum, newclasstype);

	if(UlamType::compareForMakingCastingNode(nuti, newType, m_state) != UTIC_SAME) //not same, or dontknow
	  {
	    NNO pno = Node::getYourParentNo(); //save
	    NODE_ASSERT(pno);

	    //not using use makeCastingNode since don't want recursive c&l call
	    Node * castNode = Node::newCastingNode(this, nuti);

	    NODE_ASSERT(parentnoderef);
	    NODE_ASSERT(pno == parentnoderef->getNodeNo());

	    AssertBool swapOk = parentnoderef->exchangeKids(this, castNode);
	    NODE_ASSERT(swapOk);

	    std::ostringstream msg;
	    msg << "Exchanged kids! of parent of binary " << getName();
	    msg << ", with a cast to type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    castNode->updateLineage(pno);
	    parentnoderef = castNode; //t41384
	  }
	}
    return nuti;
  } //castThyselfToResultType

  const std::string NodeBinaryOpArithRemainder::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpMod" << NodeBinaryOpArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  UlamValue NodeBinaryOpArithRemainder::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;

    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Division By Zero Attempt in Modulus", ERR);
	rtnUV.setUlamValueTypeIdx(Nav);
	setNodeType(Nav); //compiler counts
	return rtnUV;
      }

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpModInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpModUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpModBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpModUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpArithRemainder::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;

    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Division By Zero Attempt in Modulus", ERR);
	rtnUV.setUlamValueTypeIdx(Nav);
	setNodeType(Nav); //compiler counts
	return rtnUV;
      }

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpModInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpModUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpModBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpModUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpArithRemainder::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Remainder By Zero Attempt", ERR);
	refUV.setUlamValueTypeIdx(Nav);
	setNodeType(Nav); //compiler counts
	return;
      }

    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpModInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpModUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpModBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpModUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
