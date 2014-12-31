#include "NodeBinaryOpLogicalOr.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpLogicalOr::NodeBinaryOpLogicalOr(Node * left, Node * right, CompilerState & state) : NodeBinaryOpLogical(left,right,state) {}

  NodeBinaryOpLogicalOr::~NodeBinaryOpLogicalOr(){}


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
    std::ostringstream methodname;
    methodname << "_BinOpLogicalOr" << NodeBinaryOpLogical::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen


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
    u32 ldata = luv.getImmediateData(len);
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
  }  //eval


  UlamValue NodeBinaryOpLogicalOr::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    assert(0);
#if 0
    UTI nuti = getNodeType(); //Bool
    u32 nodelen = m_state.getBitSize(nuti);

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum(); // left/right node type
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpLogicalOrInt32(ldata, rdata, len), nodelen);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpLogicalOrUnsigned32(ldata, rdata, len), nodelen);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpLogicalOrBool32(ldata, rdata, len), nodelen);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(nuti, _BinOpLogicalOrUnary32(ldata, rdata, len), nodelen);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
#endif
    return rtnUV;
  } //makeImmediateBinaryOp


  void NodeBinaryOpLogicalOr::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //not implemented yet!
#if 0
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpLogicalOrInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpLogicalOrUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpLogicalOrBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpLogicalOrUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
#endif
    return;
  }


  //short-circuit when lhs is true
  void NodeBinaryOpLogicalOr::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    //    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr;  //*************
    assert(m_state.m_currentObjSymbolsForCodeGen.empty());     //*************

#ifdef TMPVARBRACES
    m_state.indent(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;
#endif

    //initialize node result to false
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    //fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString(&m_state).c_str()); //e.g. u32, s32, u64..
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti,tmpVarNum).c_str());
    fp->write(" = false;\n");

    //process lhs first
    UlamValue luvpass;
    m_nodeLeft->genCode(fp, luvpass);     //updates m_currentObjSymbol
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);                  //terminals read into tmpvar
    luti = luvpass.getPtrTargetType();
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    assert(lut->getUlamTypeEnum() == Bool);

    //fp->write("\n");

    m_state.indent(fp);
    fp->write("if(!");    //lhs is false
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

    //set node's tmp var to whatever rhs value
    m_state.indent(fp);
    fp->write(m_state.getTmpVarAsString(nuti,tmpVarNum).c_str());
    fp->write(" = ");
    fp->write(m_state.getTmpVarAsString(ruvpass.getPtrTargetType(), ruvpass.getPtrSlotIndex()).c_str());
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

    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0);  //P

#ifdef TMPVARBRACES
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");  //close for tmpVar
#endif

    //    m_state.m_currentObjPtr = saveCurrentObjectPtr;        //restore current object ptr
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************
  } //genCode


} //end MFM
