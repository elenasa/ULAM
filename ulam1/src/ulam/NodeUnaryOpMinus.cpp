#include "NodeUnaryOpMinus.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpMinus::NodeUnaryOpMinus(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpMinus::~NodeUnaryOpMinus(){}


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
    s32 sizeByInts = m_state.getUlamTypeByIndex(getNodeType())->getTotalWordSize();

    std::ostringstream methodname;
    methodname << "_UnaryMinus" << "Int" << sizeByInts ;
    return methodname.str();
  }


  UTI NodeUnaryOpMinus::checkAndLabelType()
  {
    assert(m_node);
    UTI ut = m_node->checkAndLabelType();
    UTI newType = ut;         // init to stay the same

    if(!m_state.isScalar(ut)) //array unsupported at this time
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) type: " << m_state.getUlamTypeNameByIndex(ut).c_str() << " for unary operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }
    else
      {
	ULAMTYPE eut = m_state.getUlamTypeByIndex(ut)->getUlamTypeEnum();
	// implicit cast for Bool only
	if(eut == Bool)
	  {
	    newType = Int;
	    m_node = makeCastingNode(m_node, newType);  //insert node/s
	  }
	else if(eut != Int)
	  {
	    std::ostringstream msg;
	    msg << "Unary operator" << getName() << " applied to type: " << m_state.getUlamTypeNameByIndex(ut).c_str() << " requires an explicit cast";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
	else if(ut != Int) //bitsize < 32
	  {
	    newType = Int;
	    m_node = makeCastingNode(m_node, newType);  //insert node/s
	  }
      }

    setNodeType(newType);

    setStoreIntoAble(false);

    return newType;
  } //checkAndLabelType


  UlamValue NodeUnaryOpMinus::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    //return UlamValue::makeImmediate(type, (s32) -data, len);
    return UlamValue::makeImmediate(type, _UnaryMinusInt32(data, len), len);
  }

} //end MFM
