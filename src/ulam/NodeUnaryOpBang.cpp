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
    u32 sizeByInts = m_state.getTotalWordSize(getNodeType());

    std::ostringstream methodname;
    methodname << "_LogicalBang" << "Bool" << sizeByInts;
    return methodname.str();
  } //methodNameForCodeGen

  UTI NodeUnaryOpBang::calcNodeType(UTI uti)
  {
    if(uti == Nav)
      return Nav;

    if(!m_state.isComplete(uti))
      return Hzy;

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum();
    if(typEnum == Bits)
      {
	std::ostringstream msg;
	msg << "Incompatible Bits type for unary ";
	msg << getName() << ". Suggest casting to a Bool first";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(!NodeUnaryOp::checkForPrimitiveType(uti))
      return Nav;

    UTI newType = Nav;
    s32 newbs = (typEnum == Bool ? m_state.getBitSize(uti) : 1);
    FORECAST scr = m_state.getUlamTypeByIndex(Bool)->safeCast(uti);
    if(scr != CAST_CLEAR)
      {
	std::ostringstream msg;
	msg << "Bool is the supported type for logical unary ";
	msg << getName() << "; Suggest casting ";
	msg << m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	msg << " to Bool";
	if(scr == CAST_HAZY)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    newType = Hzy;
	  }
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    else
      {
	// safe to cast. use a bool bitsize.
	UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bool"), newbs);
	newType = m_state.makeUlamType(newkey, Bool, UC_NOTACLASS);
      }
    return newType;
  } //calcNodeType

  UlamValue NodeUnaryOpBang::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    return UlamValue::makeImmediate(type, _LogicalBangBool32(data, len), len);
  }

  UlamValue NodeUnaryOpBang::makeImmediateLongUnaryOp(UTI type, u64 data, u32 len)
  {
    return UlamValue::makeImmediateLong(type, _LogicalBangBool64(data, len), len);
  }

} //end MFM
