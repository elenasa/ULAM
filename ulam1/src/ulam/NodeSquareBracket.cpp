#include "NodeSquareBracket.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "NodeTerminalIdent.h"
#include "CompilerState.h"

namespace MFM {

  NodeSquareBracket::NodeSquareBracket(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state) {}

  NodeSquareBracket::~NodeSquareBracket(){}


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

    UlamType * lut = m_state.getUlamTypeByIndex(leftType);
    bool isCustomArray = lut->isCustomArray();

    //if(m_state.isScalar(leftType))
    if(m_state.isScalar(leftType))
      {
	if(!isCustomArray)
	  {
	    std::ostringstream msg;
	    msg << "Invalid Type: <" << m_state.getUlamTypeNameByIndex(leftType).c_str() << "> used with " << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    errorCount++;
	  }
      }

    //for example, f.chance[i] where i is local, same as f.func(i);
    bool saveUseMemberBlock = m_state.m_useMemberBlock;
    m_state.m_useMemberBlock = false;

    UTI rightType = m_nodeRight->checkAndLabelType();

    m_state.m_useMemberBlock = saveUseMemberBlock;

    //must be some kind of Int..of any bit size
    if(m_state.getUlamTypeByIndex(rightType)->getUlamTypeEnum() != Int)
      {
	m_nodeRight = makeCastingNode(m_nodeRight, Int);  //refactored
      }

    if(errorCount == 0)
      {
	// sq bracket purpose in life is to account for array elements;
	if(isCustomArray)
	  newType = ((UlamTypeClass *) lut)->getCustomArrayType();
	else
	  newType = m_state.getUlamTypeAsScalar(leftType);

	setNodeType(newType);

	// multi-dimensional possible
	setStoreIntoAble(true);
      }

    return newType;
  }


  UTI NodeSquareBracket::calcNodeType(UTI lt, UTI rt)
  {
    assert(0);
    return Int;
  }


  EvalStatus NodeSquareBracket::eval()
  {
    assert(m_nodeLeft && m_nodeRight);
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

    // could be a custom array which is a scalar quark. already checked.
    UlamType * lut = m_state.getUlamTypeByIndex(ltype);
    bool isCustomArray = lut->isCustomArray();

    //assert(!m_state.isScalar(ltype));                //already checked, must be array
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
    //assert(offset.getUlamValueTypeIdx() == m_state.getUlamTypeOfConstant(Int));
    s32 arraysize = m_state.getArraySize(ltype);
    s32 offsetInt = offset.getImmediateData(m_state);

    if((offsetInt >= arraysize) && !isCustomArray)
      {
	Symbol * lsymptr;
	u32 lid = 0;
	if(getSymbolPtr(lsymptr))
	  lid = lsymptr->getId();

	std::ostringstream msg;
	msg << "Array subscript [" << offsetInt << "] exceeds the size (" << arraysize << ") of array '" << m_state.m_pool.getDataAsString(lid).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	evalNodeEpilog();
	return ERROR;
      }

    assignReturnValueToStack(pluv.getValAt(offsetInt, m_state));

    evalNodeEpilog();
    return NORMAL;
  }


  EvalStatus NodeSquareBracket::evalToStoreInto()
  {
    assert(m_nodeLeft && m_nodeRight);
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
    //assert(offset.getUlamValueTypeIdx() == m_state.getUlamTypeOfConstant(Int));
    u32 offsetInt = offset.getImmediateData(m_state);

    UTI auti = pluv.getPtrTargetType();
    UlamType * aut = m_state.getUlamTypeByIndex(auti);
    if(aut->isCustomArray())
      {
	UTI caType = ((UlamTypeClass *) aut)->getCustomArrayType();
	UlamType * caut = m_state.getUlamTypeByIndex(caType);
	u32 pos = pluv.getPtrPos();
	if(caut->getBitSize() > 32)
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
	scalarPtr.incrementPtr(m_state, offsetInt);

	//copy result UV to stack, -1 relative to current frame pointer
	assignReturnValuePtrToStack(scalarPtr);
      }

    evalNodeEpilog();
    return NORMAL;
  }


  UlamValue NodeSquareBracket::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
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


  //see also NodeTerminalIdent
  bool NodeSquareBracket::installSymbolTypedef(Token atok, s32 bitsize, s32 arraysize, Symbol *& asymptr)
  {
    assert(m_nodeLeft && m_nodeRight);

    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build typedef symbol", ERR);
	return false;
      }

    if(arraysize > NONARRAYSIZE)
      {
	MSG(getNodeLocationAsString().c_str(), "Array size specified twice for typedef symbol", ERR);
	return false;
      }

    s32 newarraysize = NONARRAYSIZE;
    if(getArraysizeInBracket(newarraysize))
      return m_nodeLeft->installSymbolTypedef(atok, bitsize, newarraysize, asymptr);

    return false;  //error getting array size
  }


  //see also NodeTerminalIdent
  bool NodeSquareBracket::installSymbolVariable(Token atok, s32 bitsize, s32 arraysize, Symbol *& asymptr)
  {
    assert(m_nodeLeft && m_nodeRight);

    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build array symbol", ERR);
	return false;
      }

    if(arraysize > NONARRAYSIZE)
      {
	MSG(getNodeLocationAsString().c_str(), "Array size specified twice", ERR);
	return false;
      }

    s32 newarraysize = NONARRAYSIZE;
    if(getArraysizeInBracket(newarraysize))
      return m_nodeLeft->installSymbolVariable(atok, bitsize, newarraysize, asymptr);

    return false;  //error getting array size
  }


  // eval() performed even before check and label!
  bool NodeSquareBracket::getArraysizeInBracket(s32 & rtnArraySize)
  {
    // since square brackets determine the constant size for this type, else error
    s32 newarraysize = NONARRAYSIZE;
    UTI sizetype = m_nodeRight->checkAndLabelType();

    // expect a constant integer
    if(sizetype == m_state.getUlamTypeOfConstant(Int))
      {
	evalNodeProlog(0);             //new current frame pointer
	makeRoomForNodeType(sizetype); //offset a constant expression
	m_nodeRight->eval();
	UlamValue arrayUV = m_state.m_nodeEvalStack.popArg();
	evalNodeEpilog();

	newarraysize = arrayUV.getImmediateData(m_state);
	if(newarraysize == NONARRAYSIZE)
	  {
	    MSG(getNodeLocationAsString().c_str(), "Array size specifier in [] is not a positive integer", ERR);
	    return false;
	  }
      }
    else
      {
	MSG(getNodeLocationAsString().c_str(), "Array size specifier in [] is not a constant integer", ERR);
	return false;
      }

    rtnArraySize = newarraysize;
    return true;
  } //getArraysizeInBracket


  void NodeSquareBracket::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);

    //wipe out before getting item within sq brackets
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    m_state.m_currentObjSymbolsForCodeGen.clear();

    UlamValue offset;
    m_nodeRight->genCode(fp, offset); //read into tmp var

    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector;  //restore

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);

    uvpass = offset;

    Node::genCodeReadArrayItemIntoATmpVar(fp, uvpass);     //new!!!
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
