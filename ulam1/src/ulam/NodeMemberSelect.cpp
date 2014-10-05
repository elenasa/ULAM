#include "NodeMemberSelect.h"
#include "CompilerState.h"

namespace MFM {

  NodeMemberSelect::NodeMemberSelect(Node * left, Node * right, CompilerState & state) : NodeBinaryOpEqual(left,right,state) {}

  NodeMemberSelect::~NodeMemberSelect(){}


  void NodeMemberSelect::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }


  const char * NodeMemberSelect::getName()
  {
    return ".";
  }


  const std::string NodeMemberSelect::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeMemberSelect::checkAndLabelType()
  { 
    assert(m_nodeLeft && m_nodeRight);

    UTI luti = m_nodeLeft->checkAndLabelType(); //side-effect
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE classtype = lut->getUlamClass();
    if(classtype == UC_NOTACLASS || classtype == UC_INCOMPLETE)
      {
	//error!
	// must be a 'Class' type, either quark or element
	setNodeType(Nav);
	return getNodeType();
      }
    
    std::string className = m_state.getUlamTypeNameBriefByIndex(luti); //help me debug

    SymbolClass * csym = NULL;
    assert(m_state.alreadyDefinedSymbolClass(lut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym));

    NodeBlockClass * memberClassNode = csym->getClassBlockNode();
    assert(memberClassNode);  // e.g. forgot the closing brace on quark definition
    //set up compiler state to use the member class block for symbol searches
    m_state.m_currentMemberClassBlock = memberClassNode;
    m_state.m_useMemberBlock = true;
    
    UTI rightType = m_nodeRight->checkAndLabelType();
    
    //clear up compiler state to no longer use the member class block for symbol searches
    m_state.m_useMemberBlock = false;
    m_state.m_currentMemberClassBlock = NULL;
    
    setNodeType(rightType);
    
    setStoreIntoAble(m_nodeRight->isStoreIntoAble());
    return getNodeType();
  }


  EvalStatus NodeMemberSelect::eval()
  {
    assert(m_nodeLeft && m_nodeRight);

    evalNodeProlog(0); //new current frame pointer on node eval stack

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //UPDATE selected member (i.e. element or quark) before eval of rhs (i.e. data member or func call)
    //UlamValue pluv = m_state.m_nodeEvalStack.popArg(); //Ptr to atom  ???which way???
    m_state.m_currentObjPtr = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1); //e.g. Ptr to atom


    u32 slot = makeRoomForNodeType(getNodeType());
    evs = m_nodeRight->eval();   //a Node Function Call here, or data member eval
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }


    //Symbol * rsymptr = NULL;
    //if(m_nodeRight->getSymbolPtr(rsymptr) && rsymptr->isFunction())

    //assigns rhs to lhs UV pointer (handles arrays);  
    //also copy result UV to stack, -1 relative to current frame pointer

    if(slot)   //avoid Void's
      doBinaryOperation(1, 1+slot, slot);  //????????

    m_state.m_currentObjPtr = saveCurrentObjectPtr;  //restore current object ptr
    evalNodeEpilog();
    return NORMAL;
  }


  //for eval, want the value of the rhs 
  void NodeMemberSelect::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    //the return value of a function call, or value of a data member
    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot); 

    UlamValue rtnUV;    
    UTI ruti = getNodeType();
    PACKFIT packFit = m_state.determinePackable(ruti);

    if(m_state.isScalar(ruti) || WritePacked(packFit))
      {
	rtnUV = ruv; 
      }
    else
      {
	//make a ptr to an unpacked array, base[0] ? [pls test]
	rtnUV = UlamValue::makePtr(rslot, EVALRETURN, ruti, UNPACKED, m_state);
      }

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(rtnUV);
  }

  

  EvalStatus NodeMemberSelect::evalToStoreInto()
  {
    evalNodeProlog(0);

    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();  
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //UPDATE selected member (i.e. element or quark) before eval of rhs (i.e. data member or func call)
    //m_state.m_currentObjPtr = m_state.m_nodeEvalStack.popArg(); //e.g. Ptr to atom
    m_state.m_currentObjPtr = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1); //e.g. Ptr to atom

    makeRoomForSlots(1); //always 1 slot for ptr
    evs = m_nodeRight->evalToStoreInto();  
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //UlamValue ruvPtr = m_state.m_nodeEvalStack.popArg();
    UlamValue ruvPtr = m_state.m_nodeEvalStack.loadUlamValueFromSlot(2);

    assignReturnValuePtrToStack(ruvPtr);

    m_state.m_currentObjPtr = saveCurrentObjectPtr;  //restore current object ptr *************
    
    evalNodeEpilog();
    return NORMAL;
  }


  UlamValue NodeMemberSelect::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    assert(0); //unused
    return UlamValue();
  }


  void NodeMemberSelect::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    assert(0); //unused
  }


  bool NodeMemberSelect::getSymbolPtr(Symbol *& symptrref)
  {
    if(m_nodeLeft)
      return m_nodeLeft->getSymbolPtr(symptrref);
    
    MSG(getNodeLocationAsString().c_str(), "No symbol", ERR);
    return false;
  }


  //differs from NodeBinaryOp, no spaces surrounding the operator .
  void NodeMemberSelect::genCode(File * fp)
  {
    assert(m_nodeLeft && m_nodeRight);

    Symbol * saveCurrentObjectSymbol = m_state.m_currentObjSymbolForCodeGen; //*************
    //UPDATE selected member (i.e. element or quark) before eval of rhs (i.e. data member or func call)
    Symbol * lsym = NULL;
    if(!getSymbolPtr(lsym))
      {
	//error!
	assert(0);
      }
    m_state.m_currentObjSymbolForCodeGen = lsym;   //***********************

    //m_nodeLeft->genCode(fp);
    //fp->write(getName());
    //m_nodeRight->genCode(fp);

    //create a tmp local variable big enough to hold the lhs membe
    //std::string tmpVar = genCodeReadIntoATmpVar(fp);

    m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol;  //restore *******
  } //genCode


  std::string NodeMemberSelect::genCodeReadIntoATmpVar(File * fp)
  {
    Symbol * saveCurrentObjectSymbol = m_state.m_currentObjSymbolForCodeGen; //*************
    //UPDATE selected member (i.e. element or quark) before eval of rhs (i.e. data member or func call)
    Symbol * lsym = NULL;
    if(!getSymbolPtr(lsym))
      {
	//error!
	assert(0);
      }
    m_state.m_currentObjSymbolForCodeGen = lsym;   //***********************

    //casting..need to pass to right as current, but its not a symbol???
    //std::string tmpVar = m_nodeLeft->genCodeReadIntoATmpVar(fp);   //the terminal

    std::string tmpVar = m_nodeRight->genCodeReadIntoATmpVar(fp);

    m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol;  //restore *******    
    return tmpVar;
  }

  void NodeMemberSelect::genCodeWriteFromATmpVar(File * fp, std::string tmpVar)
  {
    Symbol * saveCurrentObjectSymbol = m_state.m_currentObjSymbolForCodeGen; //*************
    //UPDATE selected member (i.e. element or quark) before eval of rhs (i.e. data member or func call)
    Symbol * lsym = NULL;
    if(!getSymbolPtr(lsym))
      {
	//error!
	assert(0);
      }
    m_state.m_currentObjSymbolForCodeGen = lsym;   //***********************

    m_nodeRight->genCodeWriteFromATmpVar(fp, tmpVar);
    
    m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol;  //restore *******    
  }


#if 0
  std::string NodeMemberSelect::genCodeReadIntoATmpVar(File * fp)
  {
    std::string tmpVar;
    assert(m_nodeLeft && m_nodeRight);
    //make a tmp local variable big enough to hold the lhs member
    Symbol * lsym = NULL;
    Symbol * rsym = NULL;
    if(!m_nodeLeft->getSymbolPtr(lsym))
      {
	MSG(getNodeLocationAsString().c_str(), "No L symbol", ERR);
      }
    if(!m_nodeRight->getSymbolPtr(rsym))
      {
	MSG(getNodeLocationAsString().c_str(), "No R symbol", ERR);
      }

    if(lsym && rsym)
      {
	UTI luti = m_nodeLeft->getNodeType();
	UlamType * lut = m_state.getUlamTypeByIndex(luti);
	//PACKFIT lpacked = m_state.determinePackable(luti);

	UTI ruti = rsym->getUlamTypeIdx();
	UlamType * rut = m_state.getUlamTypeByIndex(ruti);
	m_state.indent(fp);
	
	tmpVar = "UH_tmp_loadable";
	fp->write(rut->getImmediateTypeAsString(&m_state)); //e.g. u32, s32, u64, etc.
	fp->write(" ");

	fp->write(tmpVar.c_str());
	fp->write(" = ");
	fp->write(lut->getUlamTypeImmediateMangledName(&m_state).c_str());
	fp->write("::");
	// we need the atomicparametertype for this quark's data member m_val_b!!!
	fp->write(rsym->getMangledNameForParameterType().c_str());
	fp->write("::");
	fp->write(readMethodForCodeGen(ruti).c_str());
	fp->write("(");

	fp->write(HIDDEN_ARG_NAME);  //???
	fp->write(".GetBits());\n");
      }
    else
      {
	tmpVar = "tmpVarError";
	MSG(getNodeLocationAsString().c_str(), "Cannot Read without both symbols", ERR);
      }
    return tmpVar;
  } //genCodeReadIntoTmp
#endif


#if 0
  void NodeMemberSelect::genCodeWriteFromATmpVar(File * fp, std::string tmpVar)
  {
    assert(m_nodeLeft && m_nodeRight);
    //make a tmp local variable big enough to hold the lhs member
    Symbol * lsym = NULL;
    Symbol * rsym = NULL;
    if(!m_nodeLeft->getSymbolPtr(lsym))
      {
	MSG(getNodeLocationAsString().c_str(), "No L symbol", ERR);
      }
    if(!m_nodeRight->getSymbolPtr(rsym))
      {
	MSG(getNodeLocationAsString().c_str(), "No R symbol", ERR);
      }

    if(lsym && rsym)
      {
	UTI luti = m_nodeLeft->getNodeType();
	UlamType * lut = m_state.getUlamTypeByIndex(luti);
	//PACKFIT lpacked = m_state.determinePackable(luti);

	UTI ruti = rsym->getUlamTypeIdx();
	UlamType * rut = m_state.getUlamTypeByIndex(ruti);
	s32 sizeByIntBits = rut->getTotalSizeByInts();
	m_state.indent(fp);

	fp->write(lut->getUlamTypeImmediateMangledName(&m_state).c_str());
	fp->write("::");
	// we need the atomicparametertype for this quark's data member m_val_b!!!
	fp->write(rsym->getMangledNameForParameterType().c_str());
	fp->write("::");

	fp->write(writeMethodForCodeGen(ruti).c_str());
	fp->write("(");
	
	fp->write(HIDDEN_ARG_NAME);
	fp->write(".GetBits(), ");
	fp->write(tmpVar.c_str());
	fp->write(");\n");
      }
    else
      MSG(getNodeLocationAsString().c_str(), "Cannot Write without both symbols", ERR);
  } //genCodeWriteFromATmpVar
#endif


} //end MFM
