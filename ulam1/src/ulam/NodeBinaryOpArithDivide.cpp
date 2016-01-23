#include "NodeBinaryOpArithDivide.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithDivide::NodeBinaryOpArithDivide(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}

NodeBinaryOpArithDivide::NodeBinaryOpArithDivide(const NodeBinaryOpArithDivide& ref) : NodeBinaryOpArith(ref) {}

  NodeBinaryOpArithDivide::~NodeBinaryOpArithDivide(){}

  Node * NodeBinaryOpArithDivide::instantiate()
  {
    return new NodeBinaryOpArithDivide(*this);
  }

  const char * NodeBinaryOpArithDivide::getName()
  {
    return "/";
  }

  const std::string NodeBinaryOpArithDivide::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpArithDivide::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpDivide" << NodeBinaryOpArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  UTI NodeBinaryOpArithDivide::castThyselfToResultType(UTI rt, UTI lt, UTI newType)
  {
    UTI nuti = newType;
    //because the result bitsize for division should be the left bitsize
    // create a cast! combining newType's base type and left resultbitsize.
    // could be the same, or "unsafe".
    //if((newType != Nav) && (newType != Hzy) && m_state.isComplete(newType))
    if(m_state.okUTItoContinue(newType) && m_state.isComplete(newType))
      {
	UlamType * newut = m_state.getUlamTypeByIndex(newType);
	ULAMTYPE typEnum = newut->getUlamTypeEnum();
	u32 convertSize = m_state.getUlamTypeByIndex(lt)->bitsizeToConvertTypeTo(typEnum);
	u32 enumStrIdx = m_state.m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(typEnum));
	UlamKeyTypeSignature tokey(enumStrIdx, convertSize, NONARRAYSIZE);
	nuti = m_state.makeUlamType(tokey, typEnum);

	if(UlamType::compare(nuti, newType, m_state) != UTIC_SAME) //not same, or dontknow
	  {
	    NNO pno = Node::getYourParentNo(); //save
	    assert(pno);
	    //not using use makeCastingNode since don't want recursive c&l call
	    Node * castNode = new NodeCast(this, nuti, NULL, m_state);
	    assert(castNode);
	    castNode->setNodeLocation(getNodeLocation());

	    Node * parentNode = m_state.findNodeNoInThisClass(pno);
	    if(!parentNode)
	      {
		std::ostringstream msg;
		msg << "Division cast cannot be exchanged at this time while compiling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		msg << " Parent required";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		assert(0); //parent required
	      }

	    AssertBool swapOk = parentNode->exchangeKids(this, castNode);
	    assert(swapOk);

	    std::ostringstream msg;
	    msg << "Exchanged kids! of parent of binary operator" << getName();
	    msg << ", with a cast to type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    castNode->setYourParentNo(pno); //inverts normal update lineage
	    setYourParentNo(castNode->getNodeNo());
	  }
      }
    return nuti;
  } //castThyselfToResultType

  UlamValue NodeBinaryOpArithDivide::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;

    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	rtnUV.setUlamValueTypeIdx(Nav);
	setNodeType(Nav); //compiler counts
	return rtnUV;
      }

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpArithDivide::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;

    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	rtnUV.setUlamValueTypeIdx(Nav);
	setNodeType(Nav); //compiler counts
	return rtnUV;
      }

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpDivideInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpDivideUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpDivideBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpDivideUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpArithDivide::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	refUV.setUlamValueTypeIdx(Nav);
	setNodeType(Nav); //compiler counts
	return;
      }

    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpDivideInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpDivideUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpDivideBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpDivideUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  } //appendBinaryOp

} //end MFM
