#include "NodeBinaryOpEqualArith.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArith::NodeBinaryOpEqualArith(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpEqualArith::NodeBinaryOpEqualArith(const NodeBinaryOpEqualArith& ref) : NodeBinaryOpEqual(ref) {}

  NodeBinaryOpEqualArith::~NodeBinaryOpEqualArith(){}

  UTI NodeBinaryOpEqualArith::checkAndLabelType()
  {
    UTI nodeType = NodeBinaryOpEqual::checkAndLabelType();
    UlamType * nut = m_state.getUlamTypeByIndex(nodeType);

    // common part of name
    ULAMTYPE enodetyp = nut->getUlamTypeEnum();
    if(enodetyp == Bits)
      {
	// can happen with op-equal operations when both sides are the same type
	MSG(getNodeLocationAsString().c_str(), "Arithmetic Operations are invalid on 'Bits' type", ERR);
	nodeType = Nav;
      }

    if(enodetyp == Bool)
      {
	// can happen with op-equal operations when both sides are the same type
	MSG(getNodeLocationAsString().c_str(), "Arithmetic Operations are invalid on 'Bool' type", ERR);
	nodeType = Nav;
      }

    if((nodeType != Nav) && !nut->isScalar())
      {
	std::ostringstream msg;
	msg << "Non-scalars require a loop for operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	nodeType = Nav;
      }

    setNodeType(nodeType);
    return nodeType;
  } //checkAndLabelType

  const std::string NodeBinaryOpEqualArith::methodNameForCodeGen()
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
	methodname << UlamType::getUlamTypeEnumAsString(etyp);
	break;
      case Bits:
      default:
	assert(0);
	methodname << "NAV";
	break;
      };
    methodname << nut->getTotalWordSize();
    return methodname.str();
  } //methodNameForCodeGen

  bool NodeBinaryOpEqualArith::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	return doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	return doBinaryOperationArray(lslot, rslot, slots);
      }
    return false;
  } //end dobinaryop

  void NodeBinaryOpEqualArith::genCode(File * fp, UlamValue& uvpass)
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
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);      //may update m_currentObjSymbol

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
