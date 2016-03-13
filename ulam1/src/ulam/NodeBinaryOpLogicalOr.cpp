#include "NodeBinaryOpLogicalOr.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpLogicalOr::NodeBinaryOpLogicalOr(Node * left, Node * right, CompilerState & state) : NodeBinaryOpLogical(left,right,state) {}

  NodeBinaryOpLogicalOr::NodeBinaryOpLogicalOr(const NodeBinaryOpLogicalOr& ref) : NodeBinaryOpLogical(ref) {}

  NodeBinaryOpLogicalOr::~NodeBinaryOpLogicalOr(){}

  Node * NodeBinaryOpLogicalOr::instantiate()
  {
    return new NodeBinaryOpLogicalOr(*this);
  }

  const char * NodeBinaryOpLogicalOr::getName()
  {
    return "||";
  }

  const std::string NodeBinaryOpLogicalOr::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpLogicalOr::methodNameForCodeGen()
  {
    assert(0);
    return "notapplicable_logicalOr";
  } //methodNameForCodeGen

  //overrides parent due to short-circuiting requirement
  EvalStatus NodeBinaryOpLogicalOr::eval()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();

    u32 len = m_state.getTotalBitSize(nuti);

    evalNodeProlog(0); //new current frame pointer

    u32 slot = makeRoomForNodeType(nuti);

    EvalStatus evs = m_nodeLeft->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //short-circuit if lhs is true
    UlamValue luv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slot); //immediate value
    u32 ldata = luv.getImmediateData(len, m_state);
    if(_Bool32ToCbool(ldata, len) == true)
      {
	//copies return UV to stack, -1 relative to current frame pointer
	m_state.m_nodeEvalStack.storeUlamValueInSlot(luv, -1);
      }
    else
      {
	u32 slot2 = makeRoomForNodeType(getNodeType());
	evs = m_nodeRight->eval();
	if(evs != NORMAL)
	  {
	    evalNodeEpilog();
	    return evs;
	  }

	UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slot+slot2); //immediate value
	//copies return UV to stack, -1 relative to current frame pointer
	m_state.m_nodeEvalStack.storeUlamValueInSlot(ruv, -1);
      }

    evalNodeEpilog();
    return NORMAL;
  } //eval

  UlamValue NodeBinaryOpLogicalOr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    assert(0); //overridden by eval
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpLogicalOr::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    assert(0); //overridden by eval
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpLogicalOr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not implemented yet!
    return;
  }

  //short-circuit when lhs is true
  void NodeBinaryOpLogicalOr::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************

    //initialize node result to false
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    //fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64..
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti,tmpVarNum).c_str());
    fp->write(" = false;\n");

    //process lhs first
    UlamValue luvpass;
    m_nodeLeft->genCode(fp, luvpass); //updates m_currentObjSymbol
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(m_state.isPtr(luti)); //terminals read into tmpvar
    luti = luvpass.getPtrTargetType();
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    assert(lut->getUlamTypeEnum() == Bool);

    //fp->write("\n");

    m_state.indent(fp);
    fp->write("if(!"); //lhs is false
    fp->write(((UlamTypeBool *) lut)->getConvertToCboolMethod().c_str());
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str());
    fp->write(", ");
    fp->write_decimal(lut->getBitSize());
    fp->write(")");
    fp->write(")\n");

    m_state.indent(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;

    UlamValue ruvpass;
    m_nodeRight->genCode(fp, ruvpass);
    UTI ruti = ruvpass.getUlamValueTypeIdx();
    assert(m_state.isPtr(ruti));
    ruti = ruvpass.getPtrTargetType();

    //set node's tmp var to whatever rhs value
    m_state.indent(fp);
    fp->write(m_state.getTmpVarAsString(nuti,tmpVarNum).c_str());
    fp->write(" = ");
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex()).c_str());
    fp->write(";\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");

    m_state.indent(fp);
    fp->write("else\n");
    m_state.indent(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;

    //set node's tmp var to lhs value (true)
    m_state.indent(fp);
    fp->write(m_state.getTmpVarAsString(nuti,tmpVarNum).c_str());
    fp->write(" = ");
    fp->write(m_state.getTmpVarAsString(luvpass.getPtrTargetType(), luvpass.getPtrSlotIndex()).c_str());
    fp->write(";\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");

    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0); //P

    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************
  } //genCode

} //end MFM
