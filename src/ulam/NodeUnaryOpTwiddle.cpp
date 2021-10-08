#include "NodeUnaryOpTwiddle.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOpTwiddle::NodeUnaryOpTwiddle(Node * n, CompilerState & state): NodeUnaryOp(n, state) {}

  NodeUnaryOpTwiddle::NodeUnaryOpTwiddle(const NodeUnaryOpTwiddle& ref) : NodeUnaryOp(ref) {}

  NodeUnaryOpTwiddle::~NodeUnaryOpTwiddle(){}

  Node * NodeUnaryOpTwiddle::instantiate()
  {
    return new NodeUnaryOpTwiddle(*this);
  }

  const char * NodeUnaryOpTwiddle::getName()
  {
    return "~";
  }

  const std::string NodeUnaryOpTwiddle::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeUnaryOpTwiddle::methodNameForCodeGen()
  {
    u32 sizeByInts = m_state.getTotalWordSize(getNodeType());

    std::ostringstream methodname;
    methodname << "_BitwiseComplement" << "Bits";

    if(sizeByInts <= MAXBITSPERLONG)
      methodname << sizeByInts;
    else
      methodname << "BV";  //for Big Bits, t41567
    return methodname.str();
  } //methodNameForCodeGen

  UTI NodeUnaryOpTwiddle::calcNodeType(UTI uti)
  {
    if(uti == Nav)
      return Nav;

    if(!m_state.isComplete(uti))
      return Hzy;

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum();
    if(typEnum == Class)
      {
	std::ostringstream msg;
	msg << "Incompatible type for unary ";
	msg << getName() << ". Suggest casting to Bits first";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(!NodeUnaryOp::checkForPrimitiveType(uti))
      return Nav;

    UTI newType = Nav;
    // safe to cast to same bitsize.
    s32 newbs = m_state.getBitSize(uti);
    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Bits"), newbs);
    newType = m_state.makeUlamType(newkey, Bits, UC_NOTACLASS);

    FORECAST scr = m_state.getUlamTypeByIndex(newType)->safeCast(uti);
    if(scr != CAST_CLEAR)
      {
	std::ostringstream msg;
	msg << "Bits is the supported type for bitwise unary ";
	msg << getName() << "; Suggest casting ";
	msg << m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	msg << " to Bits";
	if(scr == CAST_HAZY)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    newType = Hzy;
	  }
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
  return newType;
  } //calcNodeType

  UlamValue NodeUnaryOpTwiddle::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    return UlamValue::makeImmediate(type, _BitwiseComplementBits32(data, len), len);
  }

  UlamValue NodeUnaryOpTwiddle::makeImmediateLongUnaryOp(UTI type, u64 data, u32 len)
  {
    return UlamValue::makeImmediateLong(type, _BitwiseComplementBits64(data, len), len);
  }

} //end MFM
