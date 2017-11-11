#include "NodeBinaryOpEqualArith.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualArith::NodeBinaryOpEqualArith(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpEqualArith::NodeBinaryOpEqualArith(const NodeBinaryOpEqualArith& ref) : NodeBinaryOpEqual(ref) {}

  NodeBinaryOpEqualArith::~NodeBinaryOpEqualArith(){}

  UTI NodeBinaryOpEqualArith::checkAndLabelType()
  {
    //copied from NodeBinaryOpEqual::checkandlabeltype..
    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();

    if(!m_state.neitherNAVokUTItoContinue(leftType, rightType))
      {
	std::ostringstream msg;
	msg << "Assignment is invalid";
	msg << "; LHS: ";
	msg << m_state.getUlamTypeNameBriefByIndex(leftType);
	msg << "; RHS: ";
	msg << m_state.getUlamTypeNameBriefByIndex(rightType);

	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    if(!m_state.isComplete(leftType) || !m_state.isComplete(rightType))
      {
    	setNodeType(Hzy);
	m_state.setGoAgain(); //for compiler counts
    	return Hzy; //not quietly
      }

    TBOOL stor = checkStoreIntoAble();
    if(stor == TBOOL_FALSE)
      {
	setNodeType(Nav);
	return Nav;
      }
    else if(stor == TBOOL_HAZY)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
      }

    if(!NodeBinaryOp::checkNotVoidTypes(leftType, rightType, false))
      {
    	setNodeType(Nav);
    	return Nav;
      }

    if(m_nodeRight->isExplicitReferenceCast())
      {
	std::ostringstream msg;
	msg << "Explicit Reference cast of assignment is invalid";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    if(m_state.isScalar(leftType) ^ m_state.isScalar(rightType))
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types: ";
	msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
	msg << " and ";
	msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
	msg << " used with binary " << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
      }

    UTI newType = leftType;
    UlamType * lut = m_state.getUlamTypeByIndex(leftType);
    if(lut->getUlamTypeEnum() == Class)
      {
	//try for operator overload first (e.g. (pre) +=,-=, (post) ++,-- ) t41117,8
	Node * newnode = buildOperatorOverloadFuncCallNode(); //virtual
	if(newnode)
	  {
	    AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
	    assert(swapOk);

	    m_nodeLeft = NULL; //recycle as memberselect
	    m_nodeRight = NULL; //recycle as func call arg

	    delete this; //suicide is painless..

	    return newnode->checkAndLabelType();
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Incompatible class type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
	    msg << " and ";
	    msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
	    msg << " used with binary " << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav; //error
	  }
      }
    else
      {
	//LHS not class; cast RHS if necessary and safe (t3388)
	if(UlamType::compareForAssignment(newType, rightType, m_state) != UTIC_SAME)
	  {
	    UTI derefLeft = m_state.getUlamTypeAsDeref(leftType); //tmp deref type
	    if(checkSafeToCastTo(rightType, derefLeft))
	      {
		if(!Node::makeCastingNode(m_nodeRight, derefLeft, m_nodeRight))
		  newType = Nav; //error
	      } //else not safe, error msg, newType changed
	  } //else the same
      }

    //specifically for equal arith's, no classes left at this point..
    if(m_state.okUTItoContinue(newType))
      {
	UlamType * nut = m_state.getUlamTypeByIndex(newType);
	if(!nut->isNumericType())
	  {
	    //no bool, bits, or atoms..(t3211, t3212, t3213, t41156)
	    std::ostringstream msg;
	    msg << "Arithmetic Operation " << getName();
	    msg << " is invalid on '";
	    msg << nut->getUlamTypeNameOnly().c_str() << "' types";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }

	if((newType != Nav) && !nut->isScalar())
	  {
	    std::ostringstream msg;
	    msg << "Non-scalars require a loop for operation " << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav; //t3140, t3150
	  }
      }

    setNodeType(newType);
    return newType;
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
	m_state.abortUndefinedUlamPrimitiveType();
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

  void NodeBinaryOpEqualArith::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    assert(m_state.m_currentObjSymbolsForCodeGen.empty());

    // generate rhs first; may update current object globals (e.g. function call)
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
    uvpass = luvpass; //keep luvpass slot untouched
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

    uvpass = UVPass::makePass(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, uvpass.getPassPos(), uvpass.getPassNameId()); //P

    // current object globals should pertain to lhs for the write
    genCodeWriteFromATmpVar(fp, luvpass, uvpass); //uses rhs' tmpvar; orig lhs

    assert(m_state.m_currentObjSymbolsForCodeGen.empty());
  } //genCode

} //end MFM
