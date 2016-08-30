#include "NodeBinaryOpEqualArithPostIncr.h"
#include "NodeBinaryOpArithAdd.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArithPostIncr::NodeBinaryOpEqualArithPostIncr(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqualArith(left,right,state) {}

  NodeBinaryOpEqualArithPostIncr::NodeBinaryOpEqualArithPostIncr(const NodeBinaryOpEqualArithPostIncr& ref) : NodeBinaryOpEqualArith(ref) {}

  NodeBinaryOpEqualArithPostIncr::~NodeBinaryOpEqualArithPostIncr(){}

  Node * NodeBinaryOpEqualArithPostIncr::instantiate()
  {
    return new NodeBinaryOpEqualArithPostIncr(*this);
  }

  const char * NodeBinaryOpEqualArithPostIncr::getName()
  {
    return "++";
  }

  const std::string NodeBinaryOpEqualArithPostIncr::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpEqualArithPostIncr::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpAdd" << NodeBinaryOpEqualArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen

  bool NodeBinaryOpEqualArithPostIncr::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType())) //not an array
      {
	UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(lslot); //a Ptr
	assert(m_state.isPtr(pluv.getUlamValueTypeIdx()) && (pluv.getPtrTargetType() == getNodeType()));

	assert(slots == 1);
	UlamValue luv = m_state.getPtrTarget(pluv);  //no eval!!
	if(doBinaryOperationImmediate(lslot, rslot, slots))
	  {
	    //re-save untouched result UV to stack, -1 relative to current frame pointer
	    m_state.m_nodeEvalStack.storeUlamValueInSlot(luv, -1);
	    return true;
	  }
      }
    //else array n/a/
    return false;
  } //end dobinaryop

  UlamValue NodeBinaryOpEqualArithPostIncr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpAddUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpEqualArithPostIncr::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpAddInt64(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpAddUnsigned64(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpAddBool64(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediateLong(type, _BinOpAddUnary64(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpEqualArithPostIncr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpAddInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpAddUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpAddBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpAddUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	m_state.abortUndefinedUlamPrimitiveType();
	break;
      };
    return;
  }

  void NodeBinaryOpEqualArithPostIncr::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty());

    // ==1 generate rhs first; may update current object globals (e.g. function call)
    UVPass ruvpass;
    m_nodeRight->genCode(fp, ruvpass);

    // restore current object globals
    assert(m_state.m_currentObjSymbolsForCodeGen.empty());

    // lhs should be the new current object: node member select updates them,
    // but a plain NodeIdent does not!!!  because genCodeToStoreInto has been repurposed
    // to mean "don't read into a TmpVar" (e.g. by NodeCast).
    UVPass luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);      //may update m_currentObjSymbol

    //wiped out by left read; need to write back into left
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    uvpass = luvpass; //keep luvpass slot untouched; and return uvpass (before add)
    Node::genCodeReadIntoATmpVar(fp, uvpass);
    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore vector after lhs read*************

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64..
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPREGISTER).c_str());
    fp->write(" = ");

    fp->write(methodNameForCodeGen().c_str());
    fp->write("(");
    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
    fp->write(", ");
    fp->write(ruvpass.getTmpVarAsString(m_state).c_str());
    fp->write(", ");
    fp->write_decimal(nut->getBitSize());
    fp->write(");"); GCNL;

    UVPass postuvpass = UVPass::makePass(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, uvpass.getPassPos(), uvpass.getPassNameId()); //P

    // current object globals should pertain to lhs for the write
    genCodeWriteFromATmpVar(fp, luvpass, postuvpass); //uses rhs' tmpvar; orig lhs

    assert(m_state.m_currentObjSymbolsForCodeGen.empty());
  } //genCode

} //end MFM
