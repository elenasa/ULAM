#include <stdio.h>
#include "NodeBinaryOpCompare.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpCompare::NodeBinaryOpCompare(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}


  NodeBinaryOpCompare::~NodeBinaryOpCompare()
  {}


  UTI NodeBinaryOpCompare::checkAndLabelType()
  {
    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();
    UTI newType = calcNodeType(leftType, rightType); //for casting

    if(newType != Nav)
      {
	if(rightType != newType)
	  {
	    m_nodeRight = makeCastingNode(m_nodeRight, newType);
	  }

	if(leftType != newType)
	  {
	    m_nodeLeft = makeCastingNode(m_nodeLeft, newType);
	  }

	newType = Bool;  //always Bool (default size) for node
      }

    setNodeType(newType);
    setStoreIntoAble(false);
    return newType;
  } //checkAndLabelType


  // same as Arith Ops for casting lhs & rhs, however node type is Bool
  // punt on arrays at this time..
  UTI NodeBinaryOpCompare::calcNodeType(UTI lt, UTI rt)
  {
    UTI newType = Nav; //init

    // except for 2 Unsigned, all comparison operations are performed as Int.32.-1
    // if one is unsigned, and the other isn't -> output warning, but Signed Int wins.
    // Class (i.e. quark) + anything goes to Int.32

    if( m_state.isScalar(lt) && m_state.isScalar(rt))
      {
	newType = Int;

	ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

	if(ltypEnum == Unsigned && rtypEnum == Unsigned)
	  {
	    newType = Unsigned;        //constants are not unsigned
	  }
	else if(m_state.isConstant(lt) || m_state.isConstant(rt))
	  {
	    // if one is a constant, check for value to fit in bits.
	    bool doErrMsg = true;
	    if(m_state.isConstant(lt) && m_nodeLeft->fitsInBits(rt))
	      doErrMsg = false;

	    if(m_state.isConstant(rt) && m_nodeRight->fitsInBits(lt))
	      doErrMsg = false;

	    if(doErrMsg)
	      {
		std::ostringstream msg;
		msg << "Attempting to fit a constant <";
		if(m_state.isConstant(lt))
		  msg << m_nodeLeft->getName() <<  "> into a smaller bit size type, RHS: <" << m_state.getUlamTypeNameByIndex(rt).c_str();
		else
		  msg << m_nodeRight->getName() <<  "> into a smaller bit size type, LHS: <" << m_state.getUlamTypeNameByIndex(lt).c_str();
		msg << "> for binary comparison operator" << getName() << " ";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);    //output warning
	      }
	  } //a constant
	else if(ltypEnum == Unsigned || rtypEnum == Unsigned)
	  {
	    // not both unsigned, but one is, so mixing signed and
	    // unsigned gets a warning, but still uses signed Int.
	    // if one is a constant, check for value to fit in bits.
	    std::ostringstream msg;
	    msg << "Attempting to mix signed and unsigned types, LHS: <" << m_state.getUlamTypeNameByIndex(lt).c_str() << ">, RHS: <" << m_state.getUlamTypeNameByIndex(rt).c_str() << "> for binary comparison operator" << getName() << " ";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);	    //output warning
	  }
      } //both scalars
    else
      {
	//#define SUPPORT_ARITHMETIC_ARRAY_OPS
#ifdef SUPPORT_ARITHMETIC_ARRAY_OPS
	// Conflicted: we don't like the idea that the type might be
	// different for arrays than scalars; casting occurring differently.
	// besides, for arithmetic ops, unlike logical ops, we have to do each
	// op separately anyway, so no big win (let ulam programmer do the loop).
	// let arrays of same types through ??? Is SO for op equals, btw.
	if(lt == rt)
	  {
	    return lt;
	  }
#endif //SUPPORT_ARITHMETIC_ARRAY_OPS

	//array op scalar: defer since the question of matrix operations is unclear at this time.
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types, LHS: <" << m_state.getUlamTypeNameByIndex(lt).c_str() << ">, RHS: <" << m_state.getUlamTypeNameByIndex(rt).c_str() << "> for binary comparison operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return newType;
  } //calcNodeType


  const std::string NodeBinaryOpCompare::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_BitwiseOr";  determined by each op
    UlamType * lut = m_state.getUlamTypeByIndex(m_nodeLeft->getNodeType());

    // common part of name
    ULAMTYPE etyp = lut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	methodname << "Int";
	break;
      case Unsigned:
	methodname << "Unsigned";
	break;
      default:
	assert(0);
	methodname << "NAV";
	break;
      };
    methodname << lut->getTotalWordSize();
    return methodname.str();
  } // methodNameForCodeGen


  void NodeBinaryOpCompare::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
#ifdef SUPPORT_ARITHMETIC_ARRAY_OPS
	doBinaryOperationArray(lslot, rslot, slots);
#else
	assert(0);
#endif //defined below...
      }
  } //end dobinaryop


  //unlike NodeBinaryOp, NodeBinaryOpCompare has a node type that's different from
  // its nodes, where left and right nodes are casted to be the same.
  void NodeBinaryOpCompare::doBinaryOperationImmediate(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots == 1);
    UTI luti = m_nodeLeft->getNodeType();
    u32 len = m_state.getTotalBitSize(luti);

    UlamValue luv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(lslot); //immediate value
    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot); //immediate value

    u32 ldata = luv.getImmediateData(len);
    u32 rdata = ruv.getImmediateData(len);
    UlamValue rtnUV = makeImmediateBinaryOp(luti, ldata, rdata, len);
    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);
  } //end dobinaryopImmediate


  //unlike NodeBinaryOp, NodeBinaryOpCompare has a node type that's different from
  // its nodes, where left and right nodes are casted to be the same.
  void NodeBinaryOpCompare::doBinaryOperationArray(s32 lslot, s32 rslot, u32 slots)
  {
    assert(0); //not implemented yet.
    UlamValue rtnUV;
    UTI nuti = getNodeType(); //Bool, same array size as lhs/rhs

    UTI luti = m_nodeLeft->getNodeType();
    UTI ruti = m_nodeRight->getNodeType();
    s32 arraysize = m_state.getArraySize(luti);

    assert(arraysize = m_state.getArraySize(nuti)); //node is same array size as lhs/rhs

    s32 bitsize = m_state.getBitSize(luti);
    UTI scalartypidx = m_state.getUlamTypeAsScalar(luti);
    PACKFIT packRtn = m_state.determinePackable(nuti);

    if(WritePacked(packRtn))
      {
	// pack result too. (slot size known ahead of time)
	rtnUV = UlamValue::makeAtom(nuti); //accumulate result here
      }

    // point to base array slots, packedness determines its 'pos'
    UlamValue lArrayPtr = UlamValue::makePtr(lslot, EVALRETURN, luti, packRtn, m_state);
    UlamValue rArrayPtr = UlamValue::makePtr(rslot, EVALRETURN, ruti, packRtn, m_state);

    // to use incrementPtr(), 'pos' depends on packedness
    UlamValue lp = UlamValue::makeScalarPtr(lArrayPtr, m_state);
    UlamValue rp = UlamValue::makeScalarPtr(rArrayPtr, m_state);

    //make immediate result for each element inside loop
    for(s32 i = 0; i < arraysize; i++)
      {
	UlamValue luv = m_state.getPtrTarget(lp);
	UlamValue ruv = m_state.getPtrTarget(rp);

	u32 ldata = luv.getData(lp.getPtrPos(), bitsize); //'pos' doesn't vary for unpacked
	u32 rdata = ruv.getData(rp.getPtrPos(), bitsize); //'pos' doesn't vary for unpacked

	if(WritePacked(packRtn))
	  // use calc position where base [0] is furthest from the end.
	  appendBinaryOp(rtnUV, ldata, rdata, (BITSPERATOM-(bitsize * (arraysize - i))), bitsize);
	else
	  {
	    rtnUV = makeImmediateBinaryOp(scalartypidx, ldata, rdata, bitsize);

	    //copy result UV to stack, -1 (first array element deepest) relative to current frame pointer
	    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -slots + i);
	  }

	lp.incrementPtr(m_state);
	rp.incrementPtr(m_state);
      } //forloop

    if(WritePacked(packRtn))
      m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);  //store accumulated packed result

  } //end dobinaryoparray


  void NodeBinaryOpCompare::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty());     //*************

#ifdef TMPVARBRACES
    m_state.indent(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;
#endif

    // generate rhs first; may update current object globals (e.g. function call)
    UlamValue ruvpass;
    m_nodeRight->genCode(fp, ruvpass);

    // restore current object globals
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************

    UlamValue luvpass;
    m_nodeLeft->genCode(fp, luvpass);     //updates m_currentObjSymbol

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString(&m_state).c_str()); //e.g. u32, s32, u64..
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(nuti,tmpVarNum).c_str());
    fp->write(" = ");

    fp->write(methodNameForCodeGen().c_str());
    fp->write("(");

    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType();  //reset
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");

    UTI ruti = ruvpass.getUlamValueTypeIdx();
    assert(ruti == Ptr);
    fp->write(m_state.getTmpVarAsString(ruvpass.getPtrTargetType(), ruvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");

    fp->write_decimal(m_state.getUlamTypeByIndex(luti)->getTotalBitSize());  //compare needs size of left/right nodes (only difference!)

    fp->write(");\n");

    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0);  //P

#ifdef TMPVARBRACES
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");  //close for tmpVar
#endif
    assert(m_state.m_currentObjSymbolsForCodeGen.empty()); //*************
  } //genCode


} //end MFM
