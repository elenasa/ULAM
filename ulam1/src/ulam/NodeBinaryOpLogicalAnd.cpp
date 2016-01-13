#include "NodeBinaryOpLogicalAnd.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpLogicalAnd::NodeBinaryOpLogicalAnd(Node * left, Node * right, CompilerState & state) : NodeBinaryOpLogical(left,right,state) {}

  NodeBinaryOpLogicalAnd::NodeBinaryOpLogicalAnd(const NodeBinaryOpLogicalAnd& ref) : NodeBinaryOpLogical(ref) {}

  NodeBinaryOpLogicalAnd::~NodeBinaryOpLogicalAnd(){}

  Node * NodeBinaryOpLogicalAnd::instantiate()
  {
    return new NodeBinaryOpLogicalAnd(*this);
  }

  const char * NodeBinaryOpLogicalAnd::getName()
  {
    return "&&";
  }

  const std::string NodeBinaryOpLogicalAnd::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeBinaryOpLogicalAnd::methodNameForCodeGen()
  {
    assert(0);
    return "notapplicable_logicalAnd";
  } //methodNameForCodeGen

  //overrides parent due to short-circuiting requirement
  EvalStatus NodeBinaryOpLogicalAnd::eval()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    u32 len = m_state.getTotalBitSize(nuti);

    evalNodeProlog(0); //new current frame pointer

    u32 slot = makeRoomForNodeType(nuti);

    EvalStatus evs = m_nodeLeft->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //short-circuit if lhs is false
    UlamValue luv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slot); //immediate value
    u32 ldata = luv.getImmediateData(len, m_state);
    if(_Bool32ToCbool(ldata, len) == false)
      {
	//copies return UV to stack, -1 relative to current frame pointer
	m_state.m_nodeEvalStack.storeUlamValueInSlot(luv, -1);
      }
    else
      {
	u32 slot2 = makeRoomForNodeType(nuti);
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

  UlamValue NodeBinaryOpLogicalAnd::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    assert(0); //overridden by eval
    return rtnUV;
  } //makeImmediateBinaryOp

  UlamValue NodeBinaryOpLogicalAnd::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    UlamValue rtnUV;
    assert(0); //overridden by eval
    return rtnUV;
  } //makeImmediateLongBinaryOp

  void NodeBinaryOpLogicalAnd::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not implemented for arrays
    return;
  }

  //short-circuit when lhs is false
  void NodeBinaryOpLogicalAnd::genCode(File * fp, UlamValue& uvpass)
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
    assert(luti == Ptr); //terminals read into tmpvar
    luti = luvpass.getPtrTargetType();
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    assert(lut->getUlamTypeEnum() == Bool);

    //fp->write("\n");

    m_state.indent(fp);
    fp->write("if(");
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
    assert(ruti == Ptr);
    ruti = ruvpass.getPtrTargetType();

    //set node tmp var to rhs value
    m_state.indent(fp);
    fp->write(m_state.getTmpVarAsString(nuti,tmpVarNum).c_str());
    fp->write(" = ");
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex()).c_str());
    fp->write(";\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");

    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0); //P

    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************
  } //genCode

} //end MFM
