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
    
    if(m_state.isScalar(leftType))
      {
	std::ostringstream msg;
	msg << "Invalid Type: <" << m_state.getUlamTypeNameByIndex(leftType).c_str() << "> used with " << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	errorCount++;
      }

    UTI rightType = m_nodeRight->checkAndLabelType();

    //must be some kind of Int..of any bit size
    if(m_state.getUlamTypeByIndex(rightType)->getUlamTypeEnum() != Int)
      {

	m_nodeRight = makeCastingNode(m_nodeRight, Int);  //refactored

#if 0
	//"cast" Quark to return an Int by
	//inserting NodeMemberSelect and NodeFunctionCall to 'castToInt' method.
	if(m_state.getUlamTypeByIndex(rightType)->getUlamClass() == UC_QUARK)
	  {
	    Token identTok;	 
	    u32 castId = m_state.m_pool.getIndexForDataString("toInt");	 
	    identTok.init(TOK_IDENTIFIER, getNodeLocation(), castId);
	    
	    //fill in func symbol during type labeling;
	    Node * fcallNode = new NodeFunctionCall(identTok,NULL, m_state);
	    fcallNode->setNodeLocation(identTok.m_locator);
	    Node * mselectNode = new NodeMemberSelect(m_nodeRight,fcallNode,m_state);
	    mselectNode->setNodeLocation(identTok.m_locator);
	    
	    m_nodeRight=mselectNode;  //replace right node with new branch
	    
	    //redo check and type labeling
	    rightType = m_nodeRight->checkAndLabelType(); 
	    assert(m_state.getUlamTypeByIndex(rightType)->getUlamTypeEnum() == Int);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid Type: <" << m_state.getUlamTypeNameByIndex(rightType).c_str() << "> used within " << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    errorCount++;
	  }
#endif

      }
    
    if(errorCount == 0)
      {
	// sq bracket purpose in life is to account for array elements;    
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

    assert(!m_state.isScalar(ltype));                //already checked, must be array

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

    if(offsetInt >= arraysize)
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

    //adjust pos by offset * len, according to its scalar type 
    UlamValue scalarPtr = UlamValue::makeScalarPtr(pluv, m_state);
    scalarPtr.incrementPtr(m_state, offsetInt);
   
    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(scalarPtr);

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
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build symbol", ERR);
	return false;
      }
    
    s32 newarraysize = 0;
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
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build symbol", ERR);
	return false;
      }
    
    s32 newarraysize = NONARRAYSIZE;
    if(getArraysizeInBracket(newarraysize))
      return m_nodeLeft->installSymbolVariable(atok, bitsize, newarraysize, asymptr);
   
    return false;  //error getting array size
  }


  bool NodeSquareBracket::getArraysizeInBracket(s32 & rtnArraySize)
  {
    // since square brackets determine the constant size for this type, else error
    s32 newarraysize = NONARRAYSIZE;
    UTI sizetype = m_nodeRight->checkAndLabelType();
    
    // expect a constant integer
    if(sizetype == m_state.getUlamTypeOfConstant(Int))
      {
	evalNodeProlog(0); //new current frame pointer
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

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************
    //Symbol * saveCurrentObjectSymbol = m_state.m_currentObjSymbolForCodeGen; //********

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);
    m_state.m_currentObjPtr = luvpass; //updated by lhs

#if 0
    //UPDATE selected member (i.e. element or quark) before eval of rhs (i.e. data member or func call)
    Symbol * lsym = NULL;
    if(!getSymbolPtr(lsym))
      {
	//error!
	assert(0);
      }
    //m_state.m_currentObjSymbolForCodeGen = lsym;   //***********************
    m_state.m_currentObjSymbolsForCodeGen.push_back(lsym);
#endif

    UlamValue nextlptr = UlamValue::makeScalarPtr(luvpass,m_state);  //for incrementPtr

    UlamValue offset;
    //m_nodeRight->genCode(fp, offset);
    m_nodeRight->genCodeToStoreInto(fp, offset); //for immediate value

    s32 offsetInt = offset.getImmediateData(m_state);
    nextlptr.incrementPtr(m_state, offsetInt);
    nextlptr.setPtrNameId(luvpass.getPtrNameId());

    genCodeReadIntoATmpVar(fp, nextlptr);  // more consistent, ok?
    uvpass = nextlptr;

    m_state.m_currentObjPtr = saveCurrentObjectPtr;  //restore current object ptr
    //m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol;  //restore *******
  } //genCode


  void NodeSquareBracket::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft && m_nodeRight);

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass);
    m_state.m_currentObjPtr = luvpass; //updated by lhs ********** NO RESTORE

    //UPDATE selected member (i.e. element or quark) before eval of rhs (i.e. data member or func call)
#if 0
    Symbol * lsym = NULL;
    if(!getSymbolPtr(lsym))
      {
	//error!
	assert(0);
      }
    //m_state.m_currentObjSymbolForCodeGen = lsym;   //***********************
    m_state.m_currentObjSymbolsForCodeGen.push_back(lsym);   //***********************
#endif

    UlamValue nextlptr = UlamValue::makeScalarPtr(luvpass,m_state);  //for incrementPtr

    UlamValue offset;
    //m_nodeRight->genCode(fp, offset);  
    m_nodeRight->genCodeToStoreInto(fp, offset); //for immediate value

    s32 offsetInt = offset.getImmediateData(m_state);
    nextlptr.incrementPtr(m_state, offsetInt);

    uvpass = nextlptr; //return
    uvpass.setPtrNameId(luvpass.getPtrNameId());

    // NO RESTORE -- up to caller for lhs.
  } //genCodeToStoreInto

} //end MFM
