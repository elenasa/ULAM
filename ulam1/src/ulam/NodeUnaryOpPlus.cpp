#include "NodeUnaryOpPlus.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpPlus::NodeUnaryOpPlus(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpPlus::NodeUnaryOpPlus(const NodeUnaryOpPlus& ref) : NodeUnaryOp(ref) {}

  NodeUnaryOpPlus::~NodeUnaryOpPlus(){}

  Node * NodeUnaryOpPlus::instantiate()
  {
    return new NodeUnaryOpPlus(*this);
  }

  const char * NodeUnaryOpPlus::getName()
  {
    return "+";
  }

  const std::string NodeUnaryOpPlus::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeUnaryOpPlus::calcNodeType(UTI uti)
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

    if(typEnum == Void)
      {
	std::ostringstream msg;
	msg << "Void is not supported for unary operator";
	msg << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(!NodeUnaryOp::checkForPrimitiveType(uti))
      return Nav;

    //else maintain type for NOOP (e.g. Unary, Unsigned, Int)
    return uti;
  } //calcNodeType

  UlamValue NodeUnaryOpPlus::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    return UlamValue::makeImmediate(type, data, len); //no change
  }

  void NodeUnaryOpPlus::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_node);
    m_node->genCode(fp, uvpass); //essentially a no-op
  } //genCode

  void NodeUnaryOpPlus::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    assert(m_node);
    m_node->genCodeToStoreInto(fp, uvpass); //essentially a no-op ?
  } //genCodeToStoreInto

} //end MFM
