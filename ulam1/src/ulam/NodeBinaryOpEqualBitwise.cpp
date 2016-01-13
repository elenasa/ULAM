#include "NodeBinaryOpEqualBitwise.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualBitwise::NodeBinaryOpEqualBitwise(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpEqualBitwise::NodeBinaryOpEqualBitwise(const NodeBinaryOpEqualBitwise& ref) : NodeBinaryOpEqual(ref) {}

  NodeBinaryOpEqualBitwise::~NodeBinaryOpEqualBitwise(){}

  UTI NodeBinaryOpEqualBitwise::checkAndLabelType()
  {
    UTI nodeType = NodeBinaryOp::checkAndLabelType(); //dup Bitwise calcNodeType

    if((nodeType != Nav) && (nodeType != Hzy))
      {
	if(!NodeBinaryOpEqual::checkStoreIntoAble())
	  {
	    setNodeType(Nav);
	    return Nav; //newType
	  }

	if(!NodeBinaryOpEqual::checkNotUnpackedArray())
	  {
	    setNodeType(Nav);
	    return Nav;
	  }
      }
    return getNodeType();
  } //checkandlabeltype

  UTI NodeBinaryOpEqualBitwise::calcNodeType(UTI lt, UTI rt)  //bitwise
  {
    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
	return Hzy;

    //no atoms, elements nor voids as either operand
    if(!NodeBinaryOp::checkForPrimitiveTypes(lt, rt))
	return Nav;

    UTI newType = Nav;  //init
    if(NodeBinaryOp::checkScalarTypesOnly(lt, rt))
      {
	bool bOK = true;
	//can't cast lhs of equal
	ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	if(ltypEnum != Bits)
	  {
	    std::ostringstream msg;
	    msg << "Bits is the supported type for bitwise operator"; //equal
	    msg << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    bOK = false;
	  }

	newType = lt;

	FORECAST rscr = m_nodeRight->safeToCastTo(newType);
	if(rscr != CAST_CLEAR)
	  {
	    std::ostringstream msg;
	    msg << "Righthand type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	    msg << " is not representable as ";
	    msg<< m_state.getUlamTypeNameBriefByIndex(newType).c_str();
	    msg << ". Bits is the supported type for bitwise operator";
	    msg << getName();
	    if(rscr == CAST_BAD)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		bOK = false;
	      }
	    else //hazy
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		newType = Hzy;
	      }
	  }

	if(!bOK)
	  newType = Nav;
      } //both scalars
    return newType;
  } //calcNodeType

  const std::string NodeBinaryOpEqualBitwise::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_BitwiseOr";  determined by each op

    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());

    // common part of name
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
      case Unsigned:
      case Bool:
      case Unary:
      case Bits:
	methodname << UlamType::getUlamTypeEnumAsString(etyp);
	break;
      default:
	assert(0);
	break;
      };
    methodname << nut->getTotalWordSize();
    return methodname.str();
  } //methodNameForCodeGen

  // third arg is the slots for the rtype; slots for the left is
  // rslot-lslot; they should be equal, unless one is a packed array
  // and the other isn't; however, currently, according to
  // CompilerState determinePackable, they should always be the same
  // since their types must be identical.
  bool NodeBinaryOpEqualBitwise::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti))  //not an array
      {
	return doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	// leverage case when both are packed, for logical bitwise operations
	if(m_state.determinePackable(nuti) == PACKEDLOADABLE)
	  {
	    return doBinaryOperationImmediate(lslot, rslot, slots);
	  }
	else
	  {
	    return doBinaryOperationArray(lslot, rslot, slots);
	  }
      }
    return false;
  } //end dobinaryop

  void NodeBinaryOpEqualBitwise::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty());

    // generate rhs first; may update current object globals (e.g. function call)
    UlamValue ruvpass;
    m_nodeRight->genCode(fp, ruvpass);

    // restore current object globals
    assert(m_state.m_currentObjSymbolsForCodeGen.empty());

    // lhs should be the new current object: node member select updates them,
    // but a plain NodeIdent does not!!!  because genCodeToStoreInto has been repurposed
    // to mean "don't read into a TmpVar" (e.g. by NodeCast).
    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //may update m_currentObjSymbol

    //wiped out by left read; need to write back into left
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    uvpass = luvpass; //keep luvpass slot untouched
    Node::genCodeReadIntoATmpVar(fp, uvpass);
    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore vector after lhs read*************

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64..
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(nuti,tmpVarNum).c_str());
    fp->write(" = ");

    fp->write(methodNameForCodeGen().c_str());
    fp->write("(");

    UTI uti = uvpass.getUlamValueTypeIdx();
    assert(uti == Ptr);
    uti = uvpass.getPtrTargetType();
    fp->write(m_state.getTmpVarAsString(uti, uvpass.getPtrSlotIndex()).c_str());
    fp->write(", ");

    UTI ruti = ruvpass.getUlamValueTypeIdx();
    assert(ruti == Ptr);
    ruti = ruvpass.getPtrTargetType();
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");
    fp->write_decimal(nut->getBitSize());
    fp->write(");\n");

    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, uvpass.getPtrPos(), uvpass.getPtrNameId()); //P

    // current object globals should pertain to lhs for the write
    genCodeWriteFromATmpVar(fp, luvpass, uvpass); //uses rhs' tmpvar; orig lhs

    assert(m_state.m_currentObjSymbolsForCodeGen.empty());
  } //genCode

} //end MFM
