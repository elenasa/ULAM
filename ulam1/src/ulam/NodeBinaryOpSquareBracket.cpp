#include "NodeBinaryOpSquareBracket.h"
#include "NodeTerminalIdent.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpSquareBracket::NodeBinaryOpSquareBracket(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left,right,state) {}

  NodeBinaryOpSquareBracket::~NodeBinaryOpSquareBracket(){}


  void NodeBinaryOpSquareBracket::printOp(File * fp)
  {
	NodeBinaryOp::printOp(fp);
  }


  const char * NodeBinaryOpSquareBracket::getName()
  {
    return "[]";
  }


  const std::string NodeBinaryOpSquareBracket::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeBinaryOpSquareBracket::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UlamType * newType = m_state.getUlamTypeByIndex(Nav);
    UlamType * leftType = m_nodeLeft->checkAndLabelType(); 

    if(leftType->isScalar())
    {
      std::ostringstream msg;
      msg << "Invalid Type: <" << m_state.getUlamTypeNameByIndex(leftType->getUlamTypeIndex()) << "> used with " << getName();
      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    }

    UlamType * rightType = m_nodeRight->checkAndLabelType();

    if(rightType->getUlamTypeIndex() != Int)
    {
      std::ostringstream msg;
      msg << "Invalid Type: <" << m_state.getUlamTypeNameByIndex(rightType->getUlamTypeIndex()) << "> used within " << getName();
      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    }

    // sq bracket purpose in life is to account for array elements;    
    newType = m_state.getUlamTypeAsScalar(leftType); 
    setNodeType(newType);
    
    // multi-dimensional possible    
    setStoreIntoAble(true);

    return newType;
  }


  EvalStatus NodeBinaryOpSquareBracket::eval()
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
    UlamType * ltype = pluv.getUlamValueType();

    assert(!ltype->isScalar());   //already checked

    makeRoomForNodeType(m_nodeRight->getNodeType()); //offset a constant expression
    evs = m_nodeRight->eval();  
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue offset = m_state.m_nodeEvalStack.popArg();
    assert(offset.getUlamValueType()->getUlamTypeIndex() == Int);
    u32 arraysize = ltype->getArraySize();
    if(offset.m_valInt >= (s32) arraysize)
      {
	Symbol * lsymptr;
	u32 lid = 0;
	if(getSymbolPtr(lsymptr))
	  lid = lsymptr->getId();
 
	std::ostringstream msg;
	msg << "Array subscript [" << offset.m_valInt << "] exceeds the size (" << arraysize << ") of array '" << m_state.m_pool.getDataAsString(lid).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	evalNodeEpilog();
	return ERROR;
      }

    assignReturnValueToStack(pluv.getValAt(offset.m_valInt, m_state));

    evalNodeEpilog();
    return NORMAL;
  }


  EvalStatus NodeBinaryOpSquareBracket::evalToStoreInto()
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
    assert(offset.getUlamValueType()->getUlamTypeIndex() == Int);

    //makeUlamValuePtrAt:
    UlamType * scalarType = m_state.getUlamTypeAsScalar(pluv.getUlamValueType());
    UlamValue rtnPtr(scalarType, pluv.m_baseArraySlotIndex + offset.m_valInt, true, pluv.m_storage);

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(rtnPtr);

    evalNodeEpilog();
    return NORMAL;
  }


  bool NodeBinaryOpSquareBracket::getSymbolPtr(Symbol *& symptrref)
  {
    if(m_nodeLeft)
      return m_nodeLeft->getSymbolPtr(symptrref);
    
    MSG(getNodeLocationAsString().c_str(), "No symbol", ERR);
    return false;  
  }


  //see also NodeTerminalIdent
  bool NodeBinaryOpSquareBracket::installSymbolTypedef(Token atok, u32 bitsize, u32 arraysize, Symbol *& asymptr)
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
  bool NodeBinaryOpSquareBracket::installSymbol(Token atok, u32 arraysize, Symbol *& asymptr)
  {
    assert(m_nodeLeft && m_nodeRight);

    if(!m_nodeLeft)
      {
	MSG(getNodeLocationAsString().c_str(), "No Identifier to build symbol", ERR);
	return false;
      }
    
    u32 newarraysize = 0;
    if(getArraysizeInBracket(newarraysize))
      return m_nodeLeft->installSymbol(atok, newarraysize, asymptr);
   
    return false;  //error getting array size
  }


  bool NodeBinaryOpSquareBracket::getArraysizeInBracket(u32 & rtnArraySize)
  {
    // since square brackets determine the constant size for this type, else error
    u32 newarraysize = 1;
    UlamType * sizetype = m_nodeRight->checkAndLabelType();
    if(sizetype->getUlamTypeIndex() == Int)
      {
	evalNodeProlog(0); //new current frame pointer
	makeRoomForNodeType(sizetype); //offset a constant expression
	m_nodeRight->eval();  
	UlamValue arrayUV = m_state.m_nodeEvalStack.popArg();
	evalNodeEpilog(); 

	newarraysize = arrayUV.m_valInt;
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
  }

} //end MFM
