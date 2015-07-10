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
    UTI newType = Nav; //init
    UTI leftType = m_nodeLeft->checkAndLabelType();

    if(leftType != Nav)
      {
	UlamType * lut = m_state.getUlamTypeByIndex(leftType);
	bool isCustomArray = lut->isCustomArray();

	if(m_state.isScalar(leftType))
	  {
	    if(!isCustomArray)
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
		assert(lut->getUlamClass() != UC_NOTACLASS);
		UTI caType = ((UlamTypeClass *) lut)->getCustomArrayType();

		if(!m_state.isComplete(caType))
		  {
		    std::ostringstream msg;
		    msg << "Incomplete Custom Array Type: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(caType).c_str();
		    msg << " used with class: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(leftType).c_str();
		    msg << getName();
		    if(lut->isComplete())
		      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    else
		      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    newType = Nav; //error!
		    errorCount++;
		  }
	      }
	  }

	//for example, f.chance[i] where i is local, same as f.func(i);
	NodeBlock * currBlock = m_state.getCurrentBlock();
	m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock); //currblock doesn't change

	UTI rightType = m_nodeRight->checkAndLabelType();

	m_state.popClassContext();

	//must be some kind of numeric type: Int, Unsigned, or Unary..of any bit size
	ULAMTYPE retype = m_state.getUlamTypeByIndex(rightType)->getUlamTypeEnum();
	if(!(retype == Int || retype == Unsigned || retype == Unary))
	  {
	    if(Node::checkSafeToCastTo(rightType, Int) == CAST_CLEAR)
	      {
		if(!makeCastingNode(m_nodeRight, Int, m_nodeRight))
		  {
		    newType = Nav; //error!
		    errorCount++;
		  }
	      }
	    else
	      {
		newType = Nav; //error!
		errorCount++;
	      }
	  }

	if(errorCount == 0)
	  {
	    // sq bracket purpose in life is to account for array elements;
	    if(isCustomArray)
	      newType = ((UlamTypeClass *) lut)->getCustomArrayType();
	    else
	      newType = m_state.getUlamTypeAsScalar(leftType);

	    setNodeType(newType);
	    // multi-dimensional possible; MP not ok lhs.
	    setStoreIntoAble(m_nodeLeft->isStoreIntoAble());
	  }
      }
    return newType;
  } //checkAndLabelType

  void NodeSquareBracket::countNavNodes(u32& cnt)
  {
    m_nodeLeft->countNavNodes(cnt);
    m_nodeRight->countNavNodes(cnt);
  }

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
    UlamType * lut = m_state.getUlamTypeByIndex(ltype);
    bool isCustomArray = lut->isCustomArray();

    assert(!m_state.isScalar(ltype) || isCustomArray); //already checked, must be array

    makeRoomForNodeType(m_nodeRight->getNodeType()); //offset a constant expression
    evs = m_nodeRight->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue offset = m_state.m_nodeEvalStack.popArg();
    // constant expression only required for array declaration
    s32 arraysize = m_state.getArraySize(ltype);
    u32 offsetdata = offset.getImmediateData(m_state);
    s32 offsetInt = m_state.getUlamTypeByIndex(offset.getUlamValueTypeIdx())->getDataAsCs32(offsetdata);

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
    assignReturnValueToStack(pluv.getValAt(offsetInt, m_state));
    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeSquareBracket::evalToStoreInto()
  {
    assert(m_nodeLeft && m_nodeRight);
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

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
    UlamType * aut = m_state.getUlamTypeByIndex(auti);
    if(aut->isCustomArray())
      {
	UTI caType = ((UlamTypeClass *) aut)->getCustomArrayType();
	UlamType * caut = m_state.getUlamTypeByIndex(caType);
	u32 pos = pluv.getPtrPos();
	if(caut->getBitSize() > MAXBITSPERINT)
	  pos = 0;
	//  itemUV = UlamValue::makeAtom(caType);
	//else
	//  itemUV = UlamValue::makeImmediate(caType, 0, m_state);  //quietly skip for now XXX

	//use CA type, not the left ident's type
	UlamValue scalarPtr = UlamValue::makePtr(pluv.getPtrSlotIndex(),
						 pluv.getPtrStorage(),
						 caType,
						 m_state.determinePackable(caType), //PACKEDLOADABLE
						 m_state,
						 pos /* base pos of array */
						 );

	//copy result UV to stack, -1 relative to current frame pointer
	assignReturnValuePtrToStack(scalarPtr);
      }
    else
      {
	//adjust pos by offset * len, according to its scalar type
	UlamValue scalarPtr = UlamValue::makeScalarPtr(pluv, m_state);
	if(scalarPtr.incrementPtr(m_state, offsetInt))
	  //copy result UV to stack, -1 relative to current frame pointer
	  assignReturnValuePtrToStack(scalarPtr);
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

  // eval() performed even before check and label!
  // returns false if error; UNKNOWNSIZE is not an error!
  bool NodeSquareBracket::getArraysizeInBracket(s32 & rtnArraySize)
  {
    bool noerr = true;
    // since square brackets determine the constant size for this type, else error
    s32 newarraysize = NONARRAYSIZE;
    UTI sizetype = m_nodeRight->checkAndLabelType();
    UlamType * sizeut = m_state.getUlamTypeByIndex(sizetype);
    ULAMTYPE etype = sizeut->getUlamTypeEnum();

    // expects a constant, numeric type within []
    if( (etype == Int || etype == Unsigned || etype == Unary) && m_nodeRight->isAConstant())
      {
	evalNodeProlog(0); //new current frame pointer
	makeRoomForNodeType(sizetype); //offset a constant expression
	if(m_nodeRight->eval() == NORMAL)
	  {
	    UlamValue arrayUV = m_state.m_nodeEvalStack.popArg();

	    u32 arraysizedata = arrayUV.getImmediateData(m_state);
	    newarraysize = sizeut->getDataAsCs32(arraysizedata);
	    if(newarraysize < 0 && newarraysize != UNKNOWNSIZE) //NONARRAY or UNKNOWN
	      {
		MSG(getNodeLocationAsString().c_str(),
		    "Array size specifier in [] is not a positive number", ERR);
		noerr = false;
	      }
	  }
	else
	  {
	    newarraysize = UNKNOWNSIZE; //still true
	  }
	evalNodeEpilog();
      }
    else
      {
	MSG(getNodeLocationAsString().c_str(),
	    "Array size specifier in [] is not a constant number", ERR);
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

    uvpass = offset;

    Node::genCodeReadArrayItemIntoATmpVar(fp, uvpass); //new!!!
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
