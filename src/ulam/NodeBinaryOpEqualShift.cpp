#include "NodeBinaryOpEqualShift.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpEqualShift::NodeBinaryOpEqualShift(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeBinaryOpEqualShift::NodeBinaryOpEqualShift(const NodeBinaryOpEqualShift& ref) : NodeBinaryOpEqual(ref) {}

  NodeBinaryOpEqualShift::~NodeBinaryOpEqualShift(){}

  UTI NodeBinaryOpEqualShift::checkAndLabelType()
  {
    //UTI nodeType = NodeBinaryOp::checkAndLabelType(); //dup Shift calcNodeType

    assert(m_nodeLeft && m_nodeRight);

    UTI leftType = m_nodeLeft->checkAndLabelType();
    UTI rightType = m_nodeRight->checkAndLabelType();

    if(!m_state.okUTItoContinue(leftType))
      {
	//left type possibly a class w overload operator; no need to check right type here;
	setNodeType(leftType);
	return getNodeType();
      }

    // efficiency bites! no sooner, need left and right side-effects
    // (e.g. NodeControl condition is Bool at start; stubs need Symbol ptrs)
    if(m_state.isComplete(getNodeType()))
      return getNodeType();

    //replace node with func call to matching function overload operator for class
    // of left, with argument of right;
    UlamType * lut = m_state.getUlamTypeByIndex(leftType);
    if((lut->getUlamTypeEnum() == Class))
      {
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
	//else should fail again as non-primitive;
      } //done

    UTI newType = calcNodeType(leftType, rightType); //does safety check

    setNodeType(newType); //could still change to nav

    if(m_state.okUTItoContinue(newType))
      {
	TBOOL stor = NodeBinaryOpEqual::checkStoreIntoAble();
	if(stor == TBOOL_FALSE)
	  {
	    setNodeType(Nav);
	    return Nav; //newType
	  }
	else if(stor == TBOOL_HAZY)
	  {
	    setNodeType(Hzy);
	    m_state.setGoAgain();
	  }

	if(!NodeBinaryOpEqual::checkNotUnpackedArray())
	  {
	    setNodeType(Nav);
	    return Nav;
	  }
      }

    if(m_state.isComplete(newType))
      {
	//lhs must be Bits; can't cast lhs of equal.

	//shift by unsigned type; cast if need be. safety checked by calcNodeType.
	if(m_state.getUlamTypeByIndex(rightType)->getUlamTypeEnum() != Unsigned)
	  {
	    if(!Node::makeCastingNode(m_nodeRight, Unsigned, m_nodeRight))
	      {
		newType = Nav;
		setNodeType(Nav);
	      }
	  }
      } //complete

    if(getNodeType() == Hzy)
      m_state.setGoAgain();

    return getNodeType();
  } //checkandlabeltype

  UTI NodeBinaryOpEqualShift::calcNodeType(UTI lt, UTI rt)  //shift
  {
    if(!m_state.neitherNAVokUTItoContinue(lt, rt))
      return Nav;

    if(!m_state.isComplete(lt) || !m_state.isComplete(rt))
	return Hzy;

    //no atoms, elements nor voids as either operand
    if(!NodeBinaryOp::checkForPrimitiveNotVoidTypes(lt, rt))
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
	    msg << "Bits is the supported type for shift operation "; //equal
	    msg << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    bOK = false;
	  }

	newType = lt;

	FORECAST rscr = m_nodeRight->safeToCastTo(Unsigned);
	if(rscr != CAST_CLEAR)
	  {
	    std::ostringstream msg;
	    msg << "Righthand type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(rt).c_str();
	    msg << " is not representable as Unsigned";
	    msg << "; the supported type for rhs of shift operation ";
	    msg << getName();
	    if(rscr == CAST_BAD)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		bOK = false;
	      }
	    else //hazy
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		newType = Hzy;
	      }
	  }

	if(!bOK)
	  newType = Nav;
      } //both scalars
    return newType;
  } //calcNodeType

  const std::string NodeBinaryOpEqualShift::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_Shift";  determined by each op

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
	m_state.abortUndefinedUlamPrimitiveType();
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
  bool NodeBinaryOpEqualShift::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    UTI nuti = getNodeType();
    if(m_state.isScalar(nuti))  //not an array
      {
	return doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
	// leverage case when both are packed, for logical shift operations
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

  void NodeBinaryOpEqualShift::genCode(File * fp, UVPass& uvpass)
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
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //may update m_currentObjSymbol

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
