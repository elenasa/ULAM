#include "NodeSquareBracket.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "NodeIdent.h"
#include "CompilerState.h"

namespace MFM {

  NodeSquareBracket::NodeSquareBracket(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state)
  {
    m_nodeRight->updateLineage(getNodeNo()); //for unknown subtrees
  }

  NodeSquareBracket::NodeSquareBracket(const NodeSquareBracket& ref) : NodeBinaryOp(ref) {}

  NodeSquareBracket::~NodeSquareBracket(){}

  Node * NodeSquareBracket::instantiate()
  {
    return new NodeSquareBracket(*this);
  }

  void NodeSquareBracket::printOp(File * fp)
  {
    NodeBinaryOp::printOp(fp);
  }

  const char * NodeSquareBracket::getName()
  {
    return "[]";
  }

  const std::string NodeSquareBracket::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeSquareBracket::methodNameForCodeGen()
  {
    return "_SquareBracket_Stub";
  }

  // used to select an array element; not for declaration
  UTI NodeSquareBracket::checkAndLabelType()
  {
    assert(m_nodeLeft && m_nodeRight);
    u32 errorCount = 0;
    u32 hazyCount = 0;
    UTI newType = Nav; //init
    UTI idxuti = Nav;

    UTI leftType = m_nodeLeft->checkAndLabelType();
    bool isCustomArray = false;

    //for example, f.chance[i] where i is local, same as f.func(i);
    NodeBlock * currBlock = m_state.getCurrentBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock); //currblock doesn't change
    UTI rightType = m_nodeRight->checkAndLabelType();

    m_state.popClassContext();

    if(m_state.isComplete(leftType))
      {
	UlamType * lut = m_state.getUlamTypeByIndex(leftType);
	isCustomArray = m_state.isClassACustomArray(leftType);

	if(lut->isScalar())
	  {
	    if(lut->isHolder())
	      {
		std::ostringstream msg;
		msg << "Incomplete Type: " << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
		msg << " used with " << getName();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		hazyCount++;
	      }
	    else if(!isCustomArray)
	      {
		std::ostringstream msg;
		msg << "Invalid Type: " << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
		msg << " used with " << getName();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorCount++;
	      }
	    else
	      {
		//must be a custom array; t.f. lhs is a quark!
		assert(lut->getUlamTypeEnum() == Class);

		// can't substitute a function call node for square brackets to leverage
		// all the overload matching in func call node's c&l, because
		// we ([]) can't tell which side of = we are on, and whether we should
		// be a aref or aset.
		UTI caType = m_state.getAClassCustomArrayType(leftType);
		if(!m_state.isComplete(caType))
		  {
		    std::ostringstream msg;
		    msg << "Incomplete Custom Array Type: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(caType).c_str();
		    msg << " used with class: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
		    msg << getName();
		    if(lut->isComplete())
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			newType = Nav; //error!
			errorCount++;
		      }
		    else
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			newType = Hzy;
			m_state.setGoAgain();
			hazyCount++;
		      }
		  }
	      }
	  }

	//set up idxuti..RHS
	//cant proceed with custom array subscript if lhs is incomplete
	//if(errorCount == 0)
	if((errorCount == 0) && (hazyCount == 0))
	  {
	    if(isCustomArray)
	      {
		bool hasHazyArgs = false;
		u32 camatches = m_state.getAClassCustomArrayIndexType(leftType, m_nodeRight, idxuti, hasHazyArgs);
		if(camatches == 0)
		  {
		    std::ostringstream msg;
		    msg << "No defined custom array get function with";
		    msg << " matching argument type ";
		    msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
		    msg << "; and cannot be called";
		    if(hasHazyArgs)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			idxuti = Hzy;
			m_state.setGoAgain();
			hazyCount++;
		      }
		    else
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			idxuti = Nav; //error!
			errorCount++;
		      }
		  }
		else if(camatches > 1)
		  {
		    std::ostringstream msg;
		    msg << "Ambiguous matches (" << camatches;
		    msg << ") of custom array get function for argument type ";
		    msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
		    msg << "; Explicit casting required";
		    if(hasHazyArgs)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			idxuti = Hzy;
			m_state.setGoAgain();
			hazyCount++;
		      }
		    else
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			idxuti = Nav; //error!
			errorCount++;
		      }
		  }
	      }
	    else
	      {
		//not custom array
		//must be some kind of numeric type: Int, Unsigned, or Unary..of any bit size
		UlamType * rut = m_state.getUlamTypeByIndex(rightType);
		if(m_state.okUTItoContinue(rightType) && !rut->isNumericType())
		  {
		    std::ostringstream msg;
		    msg << "Array item specifier requires numeric type: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(rightType).c_str();
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    idxuti = Nav; //error!
		    errorCount++;
		  }
		else if(rut->getUlamTypeEnum() == Class)
		  idxuti = Int;
		else
		  idxuti = rightType; //default unless caarray
	      }
	  } //errorcount is zero

	if(m_state.okUTItoContinue(idxuti) && (UlamType::compare(idxuti, rightType, m_state) == UTIC_NOTSAME))
	  {
	    FORECAST rscr = m_nodeRight->safeToCastTo(idxuti);
	    if(rscr == CAST_CLEAR)
	      {
		if(!Node::makeCastingNode(m_nodeRight, idxuti, m_nodeRight))
		  errorCount++;
	      }
	    else if(rscr == CAST_HAZY)
	      hazyCount++;
	    else
	      errorCount++;
	  }
      } //lt not nav, might be Hzy
    else
      {
	if(leftType == Nav)
	  errorCount++;
	else if(leftType == Hzy)
	  hazyCount++;
	else
	  hazyCount++;
      }

    if((errorCount == 0) && (hazyCount == 0))
      {
	// sq bracket purpose in life is to account for array elements;
	if(isCustomArray)
	  newType = m_state.getAClassCustomArrayType(leftType);
	else
	  newType = m_state.getUlamTypeAsScalar(leftType);

	// multi-dimensional possible; MP not ok lhs.
	setStoreIntoAble(m_nodeLeft->isStoreIntoAble());
      }
    else
      {
	if(errorCount != 0)
	  newType = Nav;
	else if(hazyCount != 0)
	  {
	    newType = Hzy;
	    m_state.setGoAgain(); //covers non-error(debug) messages for incompletes
	  }
	else
	  assert(0);
      }
    setNodeType(newType);
    return newType;
  } //checkAndLabelType

  UTI NodeSquareBracket::calcNodeType(UTI lt, UTI rt)
  {
    assert(0);
    return Int;
  }

  EvalStatus NodeSquareBracket::eval()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.popArg();
    UTI ltype = pluv.getPtrTargetType();

    //could be a custom array which is a scalar quark. already checked.
    bool isCustomArray = m_state.isClassACustomArray(ltype);

    assert(!m_state.isScalar(ltype) || isCustomArray); //already checked, must be array

    makeRoomForNodeType(m_nodeRight->getNodeType()); //offset a constant expression
    evs = m_nodeRight->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue offset = m_state.m_nodeEvalStack.popArg();
    UlamType * offut = m_state.getUlamTypeByIndex(offset.getUlamValueTypeIdx());
    s32 offsetInt = 0;
    if(offut->isNumericType())
      {
	// constant expression only required for array declaration
	s32 arraysize = m_state.getArraySize(ltype);
	u32 offsetdata = offset.getImmediateData(m_state);
	offsetInt = offut->getDataAsCs32(offsetdata);

	if((offsetInt >= arraysize) && !isCustomArray)
	  {
	    Symbol * lsymptr;
	    u32 lid = 0;
	    if(getSymbolPtr(lsymptr))
	      lid = lsymptr->getId();

	    std::ostringstream msg;
	    msg << "Array subscript [" << offsetInt << "] exceeds the size (" << arraysize;
	    msg << ") of array '" << m_state.m_pool.getDataAsString(lid).c_str() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    evalNodeEpilog();
	    return ERROR;
	  }
      }
    else if(!isCustomArray)
      {
	    Symbol * lsymptr;
	    u32 lid = 0;
	    if(getSymbolPtr(lsymptr))
	      lid = lsymptr->getId();

	    std::ostringstream msg;
	    msg << "Array subscript of array '";
	    msg << m_state.m_pool.getDataAsString(lid).c_str();
	    msg << "' requires a numeric type";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    evalNodeEpilog();
	    return ERROR;
      }

    Node::assignReturnValueToStack(pluv.getValAt(offsetInt, m_state));
    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeSquareBracket::evalToStoreInto()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.popArg();

    makeRoomForNodeType(m_nodeRight->getNodeType()); //offset a constant expression
    evs = m_nodeRight->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue offset = m_state.m_nodeEvalStack.popArg();

    // constant expression only required for array declaration
    u32 offsetdata = offset.getImmediateData(m_state);
    s32 offsetInt = m_state.getUlamTypeByIndex(offset.getUlamValueTypeIdx())->getDataAsCs32(offsetdata);

    UTI auti = pluv.getPtrTargetType();
    if(m_state.isClassACustomArray(auti))
      {
	UTI caType = m_state.getAClassCustomArrayType(auti);
	UlamType * caut = m_state.getUlamTypeByIndex(caType);
	u32 pos = pluv.getPtrPos();
	if(caut->getBitSize() > MAXBITSPERINT)
	  pos = 0;
	//  itemUV = UlamValue::makeAtom(caType);
	//else
	//  itemUV = UlamValue::makeImmediate(caType, 0, m_state);  //quietly skip for now

	//use CA type, not the left ident's type
	UlamValue scalarPtr = UlamValue::makePtr(pluv.getPtrSlotIndex(),
						 pluv.getPtrStorage(),
						 caType,
						 m_state.determinePackable(caType), //PACKEDLOADABLE
						 m_state,
						 pos /* base pos of array */
						 );

	//copy result UV to stack, -1 relative to current frame pointer
	Node::assignReturnValuePtrToStack(scalarPtr);
      }
    else
      {
	//adjust pos by offset * len, according to its scalar type
	UlamValue scalarPtr = UlamValue::makeScalarPtr(pluv, m_state);
	if(scalarPtr.incrementPtr(m_state, offsetInt))
	  //copy result UV to stack, -1 relative to current frame pointer
	  Node::assignReturnValuePtrToStack(scalarPtr);
	else
	  {
	    s32 arraysize = m_state.getArraySize(pluv.getPtrTargetType());
	    Symbol * lsymptr;
	    u32 lid = 0;
	    if(getSymbolPtr(lsymptr))
	      lid = lsymptr->getId();

	    std::ostringstream msg;
	    msg << "Array subscript [" << offsetInt << "] exceeds the size (" << arraysize;
	    msg << ") of array '" << m_state.m_pool.getDataAsString(lid).c_str() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    evs = ERROR;
	  }
      }
    evalNodeEpilog();
    return evs;
  } //evalToStoreInto

  bool NodeSquareBracket::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    return false;
  }

  UlamValue NodeSquareBracket::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    assert(0); //unused
    return UlamValue();
  }

  UlamValue NodeSquareBracket::makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len)
  {
    assert(0); //unused
    return UlamValue();
  }

  bool NodeSquareBracket::getSymbolPtr(Symbol *& symptrref)
  {
    if(m_nodeLeft)
      return m_nodeLeft->getSymbolPtr(symptrref);

    MSG(getNodeLocationAsString().c_str(), "No symbol", ERR);
    return false;
  }

  //see also NodeIdent
  bool NodeSquareBracket::installSymbolTypedef(TypeArgs& args, Symbol *& asymptr)
  {
    assert(m_nodeLeft && m_nodeRight);

    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build typedef symbol", ERR);
	return false;
      }

    if(args.m_arraysize > NONARRAYSIZE)
      {
	MSG(getNodeLocationAsString().c_str(), "Array size specified twice for typedef", ERR);
	return false;
      }

    args.m_arraysize = UNKNOWNSIZE; // no eval yet
    return m_nodeLeft->installSymbolTypedef(args, asymptr);
  } //installSymbolTypedef

  //see also NodeIdent
  bool NodeSquareBracket::installSymbolConstantValue(TypeArgs& args, Symbol *& asymptr)
  {
    MSG(getNodeLocationAsString().c_str(), "Array size specified for named constant", ERR);
    return false;
  } //installSymbolConstantValue

  //see also NodeIdent
  bool NodeSquareBracket::installSymbolParameterValue(TypeArgs& args, Symbol *& asymptr)
  {
    MSG(getNodeLocationAsString().c_str(), "Array size specified for model parameter", ERR);
    return false;
  } //installSymbolParameterValue

  //see also NodeIdent
  bool NodeSquareBracket::installSymbolVariable(TypeArgs& args,  Symbol *& asymptr)
  {
    assert(m_nodeLeft && m_nodeRight);

    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build array symbol", ERR);
	return false;
      }

    if(args.m_arraysize > NONARRAYSIZE)
      {
	MSG(getNodeLocationAsString().c_str(), "Array size specified twice", ERR);
	return false;
      }

    args.m_arraysize = UNKNOWNSIZE; // no eval yet
    return m_nodeLeft->installSymbolVariable(args, asymptr);
  } //installSymbolVariable

  bool NodeSquareBracket::assignClassArgValueInStubCopy()
  {
    //return m_nodeRight->assignClassArgValueInStubCopy();
    return true;
  }

  // eval() no longer performed before check and label
  // returns false if error; UNKNOWNSIZE is not an error!
  bool NodeSquareBracket::getArraysizeInBracket(s32 & rtnArraySize, UTI& sizetype)
  {
    bool noerr = true;
    // since square brackets determine the constant size for this type, else error
    s32 newarraysize = NONARRAYSIZE;
    sizetype = m_nodeRight->checkAndLabelType();
    if((sizetype == Nav))
      {
	rtnArraySize = UNKNOWNSIZE;
	return false;
      }

    if((sizetype == Hzy))
      {
	rtnArraySize = UNKNOWNSIZE;
	return true;
      }

    UlamType * sizeut = m_state.getUlamTypeByIndex(sizetype);

    // expects a constant, numeric type within []
    if(sizeut->isNumericType() && m_nodeRight->isAConstant())
      {
	evalNodeProlog(0); //new current frame pointer
	makeRoomForNodeType(sizetype); //offset a constant expression
	EvalStatus evs = m_nodeRight->eval();
	if(evs == NORMAL)
	  {
	    UlamValue arrayUV = m_state.m_nodeEvalStack.popArg();
	    u32 arraysizedata = arrayUV.getImmediateData(m_state);
	    newarraysize = sizeut->getDataAsCs32(arraysizedata);
	    if(newarraysize < 0 && newarraysize != UNKNOWNSIZE) //NONARRAY or UNKNOWN
	      {
		MSG(getNodeLocationAsString().c_str(),
		    "Array size specifier in [] is not a positive number", ERR);
		sizetype = Nav;
		noerr = false;
	      }
	    //else unknown is not an error
	  }
	else if(evs == NOTREADY)
	  {
	    noerr = false;
	    sizetype = Hzy;
	  }
	else //newarraysize = UNKNOWNSIZE; //still true
	  {
	    noerr = false;
	    sizetype = Nav;
	  }
	evalNodeEpilog();
      }
    else
      {
	MSG(getNodeLocationAsString().c_str(),
	    "Array size specifier in [] is not a constant number", ERR);
	sizetype = Nav;
	noerr = false;
      }
    rtnArraySize = newarraysize;
    return noerr;
  } //getArraysizeInBracket

  void NodeSquareBracket::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    //wipe out before getting item within sq brackets
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    m_state.m_currentObjSymbolsForCodeGen.clear();

    UlamValue offset;
    m_nodeRight->genCode(fp, offset); //read into tmp var

    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    //special case index for non-custom array: numeric unary conversion to cu32
    UTI luti = luvpass.getPtrTargetType();
    if(!m_state.isClassACustomArray(luti))
      {
	UTI offuti = offset.getUlamValueTypeIdx();
	if(offuti == Ptr)
	  offuti = offset.getPtrTargetType();
	UlamType * offut = m_state.getUlamTypeByIndex(offuti);
	if(offut->getUlamTypeEnum() == Unary)
	  {
	    s32 tmpVarIdx = m_state.getNextTmpVarNumber();
	    m_state.indent(fp);
	    fp->write("const u32 ");
	    fp->write(m_state.getTmpVarAsString(Unsigned, tmpVarIdx).c_str());;
	    fp->write(" = ");
	    if(offut->getTotalWordSize() <= MAXBITSPERINT)
	      fp->write("_Unary32ToCu32(");
	    else //must be long
	      fp->write("_Unary64ToCu64(");

	    fp->write(m_state.getTmpVarAsString(offuti, offset.getPtrSlotIndex(), TMPREGISTER).c_str());
	    fp->write(", ");
	    fp->write_decimal(offut->getBitSize());
	    fp->write(");\n");
	    // new uvpass here
	    offset = UlamValue::makePtr(tmpVarIdx, TMPREGISTER, Unsigned, m_state.determinePackable(offuti), m_state, 0); //POS 0 rightjustified.
	  } //end unary special case
      } //for non custom arrays only!

    uvpass = offset;
    Node::genCodeReadIntoATmpVar(fp, uvpass); //splits on array item
  } //genCode

  void NodeSquareBracket::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);
    //wipe out before getting item within sq brackets
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    m_state.m_currentObjSymbolsForCodeGen.clear();

    UlamValue offset;
    m_nodeRight->genCode(fp, offset);

    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector;  //restore

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    uvpass = offset; //return
    // NO RESTORE -- up to caller for lhs.
  } //genCodeToStoreInto

} //end MFM
