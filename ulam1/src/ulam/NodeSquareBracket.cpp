#include "NodeSquareBracket.h"
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

    if(rightType != Int)
    {
      std::ostringstream msg;
      msg << "Invalid Type: <" << m_state.getUlamTypeNameByIndex(rightType).c_str() << "> used within " << getName();
      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      errorCount++;
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
    assert(offset.getUlamValueTypeIdx() == Int);
    u32 arraysize = m_state.getArraySize(ltype);
    u32 offsetInt = offset.getImmediateData(m_state);

    if(offsetInt >= (s32) arraysize)
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
    assert(offset.getUlamValueTypeIdx() == Int);
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
  bool NodeSquareBracket::installSymbolTypedef(Token atok, u32 bitsize, u32 arraysize, Symbol *& asymptr)
  {
    assert(m_nodeLeft && m_nodeRight);

    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build symbol", ERR);
	return false;
      }
    
    u32 newarraysize = 0;
    if(getArraysizeInBracket(newarraysize))
      return m_nodeLeft->installSymbolTypedef(atok, bitsize, newarraysize, asymptr);
   
    return false;  //error getting array size
  }


  //see also NodeTerminalIdent
  bool NodeSquareBracket::installSymbolVariable(Token atok, u32 bitsize, u32 arraysize, Symbol *& asymptr)
  {
    assert(m_nodeLeft && m_nodeRight);

    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build symbol", ERR);
	return false;
      }
    
    u32 newarraysize = 0;
    if(getArraysizeInBracket(newarraysize))
      return m_nodeLeft->installSymbolVariable(atok, bitsize, newarraysize, asymptr);
   
    return false;  //error getting array size
  }


  bool NodeSquareBracket::getArraysizeInBracket(u32 & rtnArraySize)
  {
    // since square brackets determine the constant size for this type, else error
    u32 newarraysize = 1;
    UTI sizetype = m_nodeRight->checkAndLabelType();
    if(sizetype == Int)
      {
	evalNodeProlog(0); //new current frame pointer
	makeRoomForNodeType(sizetype); //offset a constant expression
	m_nodeRight->eval();  
	UlamValue arrayUV = m_state.m_nodeEvalStack.popArg();
	evalNodeEpilog(); 

	newarraysize = arrayUV.getImmediateData(m_state);
	if(newarraysize == 0)
	  {
	    MSG(getNodeLocationAsString().c_str(), "Array element specifier in [] is not a constant expression", ERR);
	    return false;
	  }
      }
    else
      {
	MSG(getNodeLocationAsString().c_str(), "Array element specifier in [] is not an integer", ERR);
	return false;
      }

    rtnArraySize = newarraysize;
    return true;
  } //getArraysizeInBracket 


  void NodeSquareBracket::genCode(File * fp)
  {
    assert(m_nodeLeft && m_nodeRight);
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());

    m_nodeLeft->genCode(fp);
    fp->write(".");
    fp->write(nut->getUlamTypeAsSingleLowercaseLetter());
    fp->write("[");
    m_nodeRight->genCode(fp);
    fp->write("]");
  }

} //end MFM
