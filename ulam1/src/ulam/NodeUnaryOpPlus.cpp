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
    if(!m_state.isComplete(uti))
      return Hzy;

    if(!NodeUnaryOp::checkForPrimitiveType(uti))
      return Nav; //outputs error msg

    if(!NodeUnaryOp::checkForNumericType(uti))
      return Nav; //outputs error msg

    //else maintain numeric type for NO-OP (e.g. Unary, Unsigned, Int)
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
