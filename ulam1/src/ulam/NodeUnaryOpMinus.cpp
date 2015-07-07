#include "NodeUnaryOpMinus.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpMinus::NodeUnaryOpMinus(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpMinus::NodeUnaryOpMinus(const NodeUnaryOpMinus& ref) : NodeUnaryOp(ref) {}

  NodeUnaryOpMinus::~NodeUnaryOpMinus(){}

  Node * NodeUnaryOpMinus::instantiate()
  {
    return new NodeUnaryOpMinus(*this);
  }

  const char * NodeUnaryOpMinus::getName()
  {
    return "-";
  }

  const std::string NodeUnaryOpMinus::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeUnaryOpMinus::methodNameForCodeGen()
  {
    u32 sizeByInts = m_state.getTotalWordSize(getNodeType());

    std::ostringstream methodname;
    methodname << "_UnaryMinus" << "Int" << sizeByInts ;
    return methodname.str();
  } //methodNameForCodeGen

  UTI NodeUnaryOpMinus::calcNodeType(UTI uti)
  {
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum();
    if(typEnum == Bits)
      {
	std::ostringstream msg;
	msg << "Incompatible Bits type for unary operator";
	msg << getName() << ". Suggest casting to a numeric type first";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(typEnum == Bool)
      {
	std::ostringstream msg;
	msg << "Incompatible Bool type for unary operator";
	msg << getName() << ". Suggest casting to a numeric type first";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(!NodeUnaryOp::checkForPrimitiveType(uti))
      return Nav; //outputs error msg

    s32 newbs = NodeUnaryOp::maxBitsize(uti);
    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Int"), newbs);
    UTI newType = m_state.makeUlamType(newkey, Int);

    if(!NodeUnaryOp::checkSafeToCastTo(newType))
      newType = Nav; //outputs error msg

    return newType;
  } //calcNodeType

  UlamValue NodeUnaryOpMinus::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    return UlamValue::makeImmediate(type, _UnaryMinusInt32(data, len), len);
  }

} //end MFM
