#include "NodeUnaryOpBang.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpBang::NodeUnaryOpBang(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpBang::NodeUnaryOpBang(const NodeUnaryOpBang& ref) : NodeUnaryOp(ref) {}

  NodeUnaryOpBang::~NodeUnaryOpBang(){}

  Node * NodeUnaryOpBang::instantiate()
  {
    return new NodeUnaryOpBang(*this);
  }

  const char * NodeUnaryOpBang::getName()
  {
    return "!";
  }

  const std::string NodeUnaryOpBang::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeUnaryOpBang::methodNameForCodeGen()
  {
    s32 sizeByInts = m_state.getUlamTypeByIndex(getNodeType())->getTotalWordSize();

    std::ostringstream methodname;
    methodname << "_LogicalBang" << "Bool" << sizeByInts;
    return methodname.str();
  } //methodNameForCodeGen

  UTI NodeUnaryOpBang::checkAndLabelType()
  {
    assert(m_node);
    UTI ut = m_node->checkAndLabelType();
    UTI newType = ut;         // init to stay the same

    if(newType != Nav && m_state.isComplete(newType))
      {
	if(!m_state.isScalar(ut)) //array unsupported at this time
	  {
	    std::ostringstream msg;
	    msg << "Incompatible (nonscalar) type: ";
	    msg << m_state.getUlamTypeNameByIndex(ut).c_str();
	    msg << " for unary operator" << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
	else
	  {
	    ULAMTYPE eut = m_state.getUlamTypeByIndex(ut)->getUlamTypeEnum();
	    if(eut != Bool)
	      {
		newType = Bool;
		m_node = makeCastingNode(m_node, newType);  //insert node/s
	      }
	  }
      }
    setNodeType(newType);
    setStoreIntoAble(false);

    if(isAConstant() && m_node->isReadyConstant())
      return constantFold();

    return newType;
  } //checkAndLabelType

  UlamValue NodeUnaryOpBang::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    //return UlamValue::makeImmediate(type, !((bool) data), len);
    return UlamValue::makeImmediate(type, _LogicalBangBool32(data, len), len);
  }

} //end MFM
