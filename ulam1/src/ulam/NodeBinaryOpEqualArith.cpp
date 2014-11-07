#include "NodeBinaryOpEqualArith.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArith::NodeBinaryOpEqualArith(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpEqualArith::~NodeBinaryOpEqualArith(){}


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
    methodname << nut->getTotalWordSize();
    return methodname.str();
  } // methodNameForCodeGen


  void NodeBinaryOpEqualArith::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	doBinaryOperationArray(lslot, rslot, slots);
      }
  } //end dobinaryop


  void NodeBinaryOpEqualArith::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr;                //*************
    Symbol * saveCurrentObjectSymbol = m_state.m_currentObjSymbolForCodeGen; //*************

#ifdef TMPVARBRACES
    m_state.indent(fp);
    fp->write("{\n");
    m_state.m_currentIndentLevel++;
#endif

    // generate rhs first; may update current object globals (e.g. function call)
    UlamValue ruvpass;
    m_nodeRight->genCode(fp, ruvpass);

    // restore current object globals
    m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol; // init to self
    m_state.m_currentObjPtr = saveCurrentObjectPtr;  //restore *******

    // lhs should be the new current object: node member select updates them, 
    // but a plain NodeTerminalIdent does not!!!  because genCodeToStoreInto has been repurposed
    // to mean "don't read into a TmpVar" (e.g. by NodeCast).
    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);      //may update m_currentObjSymbol
    //m_nodeLeft->genCode(fp, luvpass);      //may update m_currentObjSymbol
    Node::genCodeReadIntoATmpVar(fp, luvpass);

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
      
    fp->write(m_state.getTmpVarAsString(luvpass.getPtrTargetType(), luvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");

    UTI ruti = ruvpass.getUlamValueTypeIdx();
    assert(ruti == Ptr);
    fp->write(m_state.getTmpVarAsString(ruvpass.getPtrTargetType(), ruvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");

    fp->write_decimal(nut->getBitSize());

    fp->write(");\n");
  
    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0);  //P

    // current object globals should pertain to lhs for the write
    genCodeWriteFromATmpVar(fp, luvpass, uvpass);        //uses rhs' tmpvar

#ifdef TMPVARBRACES
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");  //close for tmpVar
#endif
    m_state.m_currentObjPtr = saveCurrentObjectPtr;  //restore current object ptr
    m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol;  //restore *******
  } //genCode

} //end MFM
