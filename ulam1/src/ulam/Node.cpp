#include <iostream>
#include "Node.h"
#include "CompilerState.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"


namespace MFM {

  Node::Node(CompilerState & state): m_state(state), m_storeIntoAble(false), m_nodeUType(Nav) {}  


  void Node::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)    
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }

  // any node above assignexpr is not storeintoable;
  // and has no type (e.g. statements, statement, block, program)
  UTI Node::checkAndLabelType()
  { 
    m_nodeUType = Nav;
    m_storeIntoAble = false;
    return m_nodeUType; 
  }


  Node * Node::makeCastingNode(Node * node, UTI tobeType)
  {
    Node * rtnNode = NULL;
    UTI nuti = node->getNodeType();
    ULAMCLASSTYPE nclasstype = m_state.getUlamTypeByIndex(nuti)->getUlamClass();

    if(nclasstype == UC_NOTACLASS)
      {
	rtnNode = new NodeCast(node, tobeType, m_state);
	rtnNode->setNodeLocation(getNodeLocation());
	rtnNode->checkAndLabelType();
      }
    else if (nclasstype == UC_QUARK)
      {
	Token identTok;
	u32 castId = m_state.m_pool.getIndexForDataString("toInt");	 
	identTok.init(TOK_IDENTIFIER, getNodeLocation(), castId);
	assert(m_state.getUlamTypeByIndex(tobeType)->getUlamTypeEnum() == Int);

	//fill in func symbol during type labeling;
	Node * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
	fcallNode->setNodeLocation(identTok.m_locator);
	Node * mselectNode = new NodeMemberSelect(node, fcallNode, m_state);
	mselectNode->setNodeLocation(identTok.m_locator);
	
	//address the case of different byte sizes here
	//before asserts start hitting later during assignment
	if(tobeType != nuti)
	  {
	    rtnNode = new NodeCast(mselectNode, tobeType, m_state);
	    rtnNode->setNodeLocation(getNodeLocation());
	  }
	else
	  rtnNode = mselectNode;  //replace right node with new branch
	    
	//redo check and type labeling
	UTI newType = rtnNode->checkAndLabelType(); 
	//assert(m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum() == Int);
	assert(newType == tobeType);
      }
    else
      {
	std::ostringstream msg;
	msg << "Cannot CAST type <" << m_state.getUlamTypeNameByIndex(nuti).c_str() << "> as a <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return rtnNode;
  } //make casting node


  bool Node::warnOfNarrowingCast(UTI nodeType, UTI tobeType)
  {
    bool rtnB = false;
    s32 nodesize = m_state.getBitSize(nodeType);
    s32 tobesize = m_state.getBitSize(tobeType);

    // warn of narrowing cast
    if(nodesize > tobesize)
      {
	rtnB = true;
	std::ostringstream msg;
	msg << "Narrowing CAST: type <" << m_state.getUlamTypeNameByIndex(nodeType).c_str() << "> to a <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << "> may cause data loss";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
      }
    return rtnB;
  } //x`warnOfNarrowingCast

    
  UTI Node::getNodeType()
  {
    return m_nodeUType;
  }


  void Node::setNodeType(UTI ut)
  {
    m_nodeUType = ut;
  }


  bool Node::isStoreIntoAble()
  {
    return m_storeIntoAble;
  }


  void Node::setStoreIntoAble(bool s)
  {
    m_storeIntoAble = s;
  }

  // any node above assignexpr is not storeintoable
  EvalStatus Node::evalToStoreInto()
  {
    std::ostringstream msg;
    msg << "Invalid lefthand value (not storeIntoAble): <" << getName() << ">";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    assert(!isStoreIntoAble());
    return ERROR;
  }


  Locator Node::getNodeLocation()
  {
    return m_nodeLoc;
  }


  void Node::setNodeLocation(Locator loc)
  {
    m_nodeLoc = loc;
  }


  void Node::printNodeLocation(File * fp)
  {
    fp->write(getNodeLocationAsString().c_str());
  }

  std::string Node::getNodeLocationAsString()
  {
    return m_state.getFullLocationAsString(m_nodeLoc);
  }


  bool Node::getSymbolPtr(Symbol *& symptrref)
  {
    return false;
  }

  bool Node::installSymbolTypedef(Token atok, s32 bitsize, s32 arraysize, Symbol *& asymptr)
  {
    return false;
  }


  bool Node::installSymbolVariable(Token atok, s32 bitsize, s32 arraysize, Symbol *& asymptr)
  {
    return false;
  }


  const std::string Node::nodeName(const std::string& prettyFunction)
  {
    size_t colons = prettyFunction.find("::");
    if (colons == std::string::npos)
      return "::";
    size_t begin = colons + 2;
    size_t colons2 = prettyFunction.find("::", begin);
    size_t end = colons2 - colons - 2;
    std::string nodename = prettyFunction.substr(begin,end);
    return nodename;
  }


  void Node::evalNodeProlog(u32 depth)
  {
    //space for local variables on node eval stack; adjusts current fp;
    m_state.m_nodeEvalStack.addFrameSlots(depth); 
  }


  void Node::evalNodeEpilog()
  {
    //includes any return value and args; adjusts current fp;
    m_state.m_nodeEvalStack.returnFrame();  
  }
  

  //default storage type is the EVALRETURN stack
  u32 Node::makeRoomForNodeType(UTI type, STORAGE where)
  {
    if(type == Void)
      return 0;

    u32 slots = m_state.slotsNeeded(type);
    makeRoomForSlots(slots, where);  //=1 for scalar or packed array
    return slots;
  }


  u32 Node::makeRoomForSlots(u32 slots, STORAGE where)
  {
    //push copies of temporary UV (e.g. UVPtr)
    UlamValue tmpUV = UlamValue::makeImmediate(Nav, 0, 1);  
    for(u32 j = 0; j < slots; j++)
      {
	if(where == EVALRETURN)
	  m_state.m_nodeEvalStack.pushArg(tmpUV);
	else if (where == STACK)
	  m_state.m_funcCallStack.pushArg(tmpUV);
	else
	  assert(0);
      }
    return slots;
  }


  //in case of arrays, rtnUV is a ptr; default STORAGE is EVALRETURN
  void Node::assignReturnValueToStack(UlamValue rtnUV, STORAGE where)
  {
    UTI rtnUVtype = rtnUV.getUlamValueTypeIdx(); //==node type

    if(rtnUVtype == Ptr)  //unpacked array
      {
	rtnUVtype = rtnUV.getPtrTargetType();
      }

    if(rtnUVtype == Void)  //check after Ptr target type
      return;

    assert(rtnUVtype == getNodeType());

    // save results in the stackframe for caller;
    // copies each element of the 'unpacked' array by value, 
    // in reverse order ([0] is last at bottom)
    s32 slots = m_state.slotsNeeded(rtnUVtype);

    //where to put the return value..'return' statement uses STACK
    UlamValue rtnPtr = UlamValue::makePtr(-slots, where, rtnUVtype, m_state.determinePackable(rtnUVtype), m_state);
    m_state.assignValue(rtnPtr, rtnUV);
  }


  //in case of arrays, rtnUV is a ptr.
  void Node::assignReturnValuePtrToStack(UlamValue rtnUVptr)
  {
    UTI rtnUVtype = rtnUVptr.getPtrTargetType(); //target ptr type

    if(rtnUVtype == Void) 
      return;

    UlamValue rtnPtr = UlamValue::makePtr(-1, EVALRETURN, rtnUVtype, rtnUVptr.isTargetPacked(), m_state);
    m_state.m_nodeEvalStack.assignUlamValuePtr(rtnPtr, rtnUVptr);
  }


  void Node::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert(0);
  }


  void Node::genCode(File * fp, UlamValue& uvpass)
  {
    m_state.indent(fp);
    fp->write("virtual void ");
    fp->write(prettyNodeName().c_str());
    fp->write("::genCode(File * fp){} is needed!!\n");  //sweet.
  }


  void Node::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args
   
    // write out intermediate tmpVar (i.e. terminal) as temp BitVector arg
    if(uvpass.getPtrNameId() == 0)
      {
	genCodeConvertATmpVarIntoBitVector(fp, uvpass);
	return;
      }
  
    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    vuti = uvpass.getPtrTargetType();  //replaces vuti w target type
    assert(vuti != Void);

    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.m_currentSelfSymbolForCodeGen;
      }
    else
      {	
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    //    UTI cosuti = m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx();
    UTI cosuti = cos->getUlamTypeIdx();
    //ULAMCLASSTYPE cosclasstype = m_state.getUlamTypeByIndex(cosuti)->getUlamClass();		

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    ULAMCLASSTYPE stgcosclasstype = m_state.getUlamTypeByIndex(stgcosuti)->getUlamClass();		

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	m_state.indent(fp);
	fp->write("const ");
	
	fp->write("u32"); //always read into a u32
	fp->write(" ");
	
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum).c_str());
	fp->write(" = ");

	genMemberNameOfMethod(fp, uvpass);
	
	// the READ method
	fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
	fp->write("(");
	
	// a data member quark, or the element itself should both getBits from self
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");

	if(!m_state.isScalar(cosuti))
	  {
	    fp->write(", ");
	    fp->write_decimal(m_state.getTotalBitSize(cosuti)); //array size
	    fp->write("u, ");
	    fp->write_decimal(uvpass.getPtrLen()); //BITS_PER_ITEM
	    fp->write("u, ");
	    fp->write_decimal(uvpass.getPtrPos()); //item POS (last like others)
	    fp->write("u");
	  }
	
	fp->write(");\n");

	// specifically to sign extend Int's (a cast)
	vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);
      } 
    else
      {
	m_state.indent(fp);   //local
	fp->write("const ");
		
	fp->write(vut->getTmpStorageTypeAsString(&m_state).c_str()); //e.g. u32, s32, u64, etc.
	
	fp->write(" ");
		
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum).c_str());
	fp->write(" = ");
		
	genLocalMemberNameOfMethod(fp, uvpass);

	//read method based on last cos
	fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());

	//first arg depends on stgcos
	// elements have a 'T' atom storage
	if(stgcosclasstype == UC_ELEMENT)
	  {
	    fp->write("(");
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".GetBits()");

	    if(!m_state.isScalar(cosuti))
	      {
		fp->write(", ");
		fp->write_decimal(m_state.getTotalBitSize(cosuti)); //array size
		fp->write("u, ");
		fp->write_decimal(uvpass.getPtrLen()); //BITS_PER_ITEM
		fp->write("u, ");
		fp->write_decimal(uvpass.getPtrPos()); //item POS (last like others)
		fp->write("u");
	      }

	    fp->write(");\n");

	    // specifically to sign extend Int's (a cast)
	    vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);
	  }
	else
	  {
	    // local quark or primitive (i.e. 'notaclass'); has an immediate type:
	    // uses local variable name, and immediate read method
	    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
	    if(cosSize == 1)
	      {
		fp->write("(");

		if(isCurrentObjectAPieceOfAnArray(cosuti, uvpass))
		  {
		    fp->write_decimal(uvpass.getPtrLen()); //BITS_PER_ITEM
		    fp->write("u, ");
		    fp->write_decimal(uvpass.getPtrPos()); //item POS (last like others)
		    fp->write("u");
		  }
		
		fp->write(");\n"); //done
	      }
	    else
	      {
		// storage-cos and cos-for-read are different:
		// use this instance of storage-cos to specify 
		// its non-static read method
		fp->write("(");
		fp->write(stgcos->getMangledName().c_str());
	
		assert(stgcosclasstype == UC_QUARK);
		fp->write(".getBits()");  //m_stg.GetBits
		
		// use immediate read() for entire array
		if(isCurrentObjectAPieceOfAnArray(cosuti, uvpass))
		  {
		    fp->write(", ");
		    fp->write_decimal(uvpass.getPtrLen()); //BITS_PER_ITEM
		    fp->write("u, ");
		    fp->write_decimal(uvpass.getPtrPos()); //item POS (last like others)
		    fp->write("u");
		  }

		fp->write(");\n");
		    
		// specifically to sign extend Int's (a cast)
		vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state); 
	      }
	  }
      }

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadIntoTmp
  

  // write out intermediate tmpVar, or immediate terminal, as temp BitVector
  void Node::genCodeConvertATmpVarIntoBitVector(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();
    bool isTerminal = false;

    if(vuti == Ptr)
      {	
	vuti = uvpass.getPtrTargetType();
      }
    else
      {
	isTerminal = true;
      }

    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    // write out intermediate tmpVar, or immediate terminal, as temp BitVector arg
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    
    m_state.indent(fp);
    fp->write("const ");

    fp->write(vut->getImmediateStorageTypeAsString(&m_state).c_str()); //e.g. BitVector<32> exception
    fp->write(" ");
    
    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, TMPBITVAL).c_str());
    fp->write("("); // use constructor (not equals)
    if(isTerminal)
      {
	u32 data = uvpass.getImmediateData(m_state);
	char dstr[40];
	vut->getDataAsString(data, dstr, 'z', m_state);
	fp->write(dstr);
      }
    else
      {
	fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex()).c_str()); 
      }
    fp->write(");\n");

    u32 pos = 0;  //pos calculated by makePtr(atom-based) (e.g. quark)
    if(vut->getUlamClass() == UC_NOTACLASS)
      {
	s32 wordsize = vut->getTotalWordSize();
	pos = wordsize - vut->getTotalBitSize();
      }

    uvpass = UlamValue::makePtr(tmpVarNum2, TMPBITVAL, vuti, m_state.determinePackable(vuti), m_state, pos);  //POS rightjustified.

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeConvertATmpVarIntoBitVector


  // write out immediate tmp BitValue as an intermediate tmpVar
  void Node::genCodeConvertABitVectorIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();
    assert(vuti == Ptr);
    vuti = uvpass.getPtrTargetType();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    assert(uvpass.getPtrStorage() == TMPBITVAL);

    // write out immediate tmp BitValue as an intermediate tmpVar
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    
    m_state.indent(fp);
    fp->write("const ");

    fp->write(vut->getTmpStorageTypeAsString(&m_state).c_str()); //e.g. s32, u32, etc
    //fp->write("u32");  //always read into a u32
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2).c_str());
    fp->write(" = ");
 
    fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex(), TMPBITVAL).c_str()); 
    fp->write(".");
    fp->write(readMethodForImmediateBitValueForCodeGen(vuti, uvpass).c_str());
    
    fp->write("(");
    // use immediate read for entire array
    if(isCurrentObjectAPieceOfAnArray(vuti, uvpass))
      {
	fp->write_decimal(uvpass.getPtrLen()); //BITS_PER_ITEM
	fp->write("u, ");
	fp->write_decimal(uvpass.getPtrPos()); //item POS (last like others)
	fp->write("u");
      }
    fp->write(");\n");

    uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, vuti, m_state.determinePackable(vuti), m_state, 0);  //POS 0 rightjustified (atom-based).
    uvpass.setPtrPos(0); //entire register

    // specifically to sign extend Int's (a cast)
    //vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);    
  } //genCodeConvertABitVectorIntoATmpVar


  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    assert(luvpass.getUlamValueTypeIdx() == Ptr);
    
    bool isTerminal = false;
    //UTI luti = luvpass.getPtrTargetType();
    UTI ruti = ruvpass.getUlamValueTypeIdx();
    
    if(ruti == Ptr)
      {
	ruti = ruvpass.getPtrTargetType();
      }
    else
      {
	isTerminal = true;
      }
    
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    
    //rhs could be a constant; or previously cast from Int to Unary variables.
    //UTI cosuti = m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx();
    
    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.m_currentSelfSymbolForCodeGen;
      }
    else
      {	
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }
    
    //    UTI cosuti = m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx();
    UTI cosuti = cos->getUlamTypeIdx();
    //ULAMCLASSTYPE cosclasstype = m_state.getUlamTypeByIndex(cosuti)->getUlamClass();		
    
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    ULAMCLASSTYPE stgcosclasstype = m_state.getUlamTypeByIndex(stgcosuti)->getUlamClass();		
    
    // a data member quark, or the element itself should both getBits from self;
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	m_state.indent(fp);
	
	genMemberNameOfMethod(fp, luvpass);
	
	// the WRITE method
	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");
	
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", ");  //...rest of args
      }
    else
      {
	//local
	m_state.indent(fp);
	
	genLocalMemberNameOfMethod(fp, luvpass);
	
	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());  //was luti
	fp->write("(");
	
	// allow for immediate quarks
	if(stgcosclasstype == UC_ELEMENT)
	  {
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".GetBits()");
	    fp->write(", ");   //...rest of args
	  }
	else if(stgcosclasstype == UC_QUARK)
	  {
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".getBits()");  //m_stg.GetBits
	    fp->write(", ");   //...rest of args
	  }
	else
	  {
	    //NOT A CLASS
	  }
      }

    //value to be written:
    if(isTerminal)
      {
	// write out terminal explicitly
	u32 data = ruvpass.getImmediateData(m_state);
	char dstr[40];
	rut->getDataAsString(data, dstr, 'z', m_state);
	fp->write(dstr);
	fp->write(");\n");
      }
    else
      {
	ULAMCLASSTYPE vclasstype = rut->getUlamClass();
	
	// with immediate quarks, they are read into a tmpreg as other immediates
	if(vclasstype == UC_ELEMENT)
	  {
	    //need to read from the tmpVar ??? (need a test case!)
	    //fp->write(rut->getUlamTypeImmediateMangledName(&m_state).c_str());
	    ruvpass.genCodeBitField(fp, m_state);
	    fp->write("::");
	    fp->write(readMethodForCodeGen(ruti, ruvpass).c_str());
	    fp->write("(");
	    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
	    fp->write(")");
	  }
	else
	  {
	    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
	  }
	

	// rest of array write args..
	//if(!m_state.isScalar(ruti))
	if(!m_state.isScalar(cosuti))
	  {
	    if(cos->isDataMember())
	      {
		fp->write(", ");
		fp->write_decimal(m_state.getTotalBitSize(cosuti)); //array size
		fp->write("u, ");
		fp->write_decimal(luvpass.getPtrLen()); //BITS_PER_ITEM
		fp->write("u, ");
		fp->write_decimal(luvpass.getPtrPos()); //item POS (last like others)
		fp->write("u");
	      }
	    // use immediate read() for entire array
	    else if(ruvpass.getPtrLen() != (s32) m_state.getTotalBitSize(cosuti))
	      {
		fp->write(", ");
		fp->write_decimal(luvpass.getPtrLen()); //BITS_PER_ITEM
		fp->write("u, ");
		fp->write_decimal(luvpass.getPtrPos()); //item POS (last like others)
		fp->write("u");
	      }
	  }
	fp->write(");\n");
      } //not terminal

    m_state.m_currentObjSymbolsForCodeGen.clear();

  } //genCodeWriteFromATmpVar


  void Node::genMemberNameOfMethod(File * fp, UlamValue uvpass)
  {
    assert(!isCurrentObjectALocalVariableOrArgument());

    //nothing if current object is self
    //if(m_state.m_currentObjSymbolsForCodeGen.empty())
    //  return;

    //iterate over COS vector; empty if current object is self
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(u32 i = 0; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
      }

    //if last cos is a quark, for Read/WriteRaw to work it needs an
    // atomic Parameter type (i.e. Up_Us);
    if(cosSize > 0)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen.back();
	UlamType * sut = m_state.getUlamTypeByIndex(sym->getUlamTypeIdx());
	// scalar quarks are typedefs and need atomic parametization; 
	// arrays are already atomic parameters
	if(sut->isScalar() && sut->getUlamClass() == UC_QUARK)
	  {
	    fp->write("Up_Us::");  //gives quark an atomicparameter type for write
	  }
      }
  } //genMemberNameOfMethod


  void Node::genLocalMemberNameOfMethod(File * fp, UlamValue uvpass)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[0]; 

    if(cosSize == 1)
      {
	fp->write(cos->getMangledName().c_str());
	fp->write(".");
	return;
      }
    
    UTI uti = cos->getUlamTypeIdx();
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    ULAMCLASSTYPE classtype = ut->getUlamClass();

    // if local element, first arg of read is all that's req'd?
    if(classtype == UC_QUARK)
      {
	fp->write(ut->getImmediateStorageTypeAsString(&m_state).c_str());
	fp->write("::");
	fp->write("Us::");   //typedef
      }

    for(u32 i = 1; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
      }    
  } //genLocalMemberNameOfMethod
  

  void Node::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    std::ostringstream msg;
    msg << "genCodeToStoreInto called on Node type <" << m_state.getUlamTypeNameByIndex(getNodeType()).c_str() << "> and failed.";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
    assert(0);
    return;
  } //genCodeToStoreInto


  const std::string Node::readMethodForCodeGen(UTI nuti, UlamValue uvpass)
  {
    std::string method;
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(!isCurrentObjectALocalVariableOrArgument())
      { 
	method = nut->readMethodForCodeGen();
      }
    else
      {
	//local storage case
	ULAMCLASSTYPE classtype = nut->getUlamClass();	//last cos

	// allow for immediate quarks
	if(classtype == UC_ELEMENT)
	  {
	    method = nut->readMethodForCodeGen();
	  }
	else
	  {
	    if(m_state.m_currentObjSymbolsForCodeGen.size() == 1) // not refining
	      method = readMethodForImmediateBitValueForCodeGen(nuti, uvpass);
	    else
	      method = nut->readMethodForCodeGen(); //refining implies data member within local  
	  }
      }
    return method;
  } //readMethodForCodeGen


  const std::string Node::writeMethodForCodeGen(UTI nuti, UlamValue uvpass)
  {
    std::string method;
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(!isCurrentObjectALocalVariableOrArgument())
      {
	method = nut->writeMethodForCodeGen();
      }
    else
      {
	//local storage case
	ULAMCLASSTYPE classtype = nut->getUlamClass();	//last cos

	if(classtype == UC_ELEMENT)
	  {
	    method = nut->writeMethodForCodeGen();
	  }
	else
	  {
	    if(m_state.m_currentObjSymbolsForCodeGen.size() == 1) // not refining
	      method = writeMethodForImmediateBitValueForCodeGen(nuti, uvpass);
	    else
	      method = nut->writeMethodForCodeGen(); //refining implies data member within local
	  }
      }
    return method;
  } //writeMethodForCodeGen


  const std::string Node::readMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass)
  {
    if(isCurrentObjectAPieceOfAnArray(nuti, uvpass))
      return "readArray"; //TBD

    return "read";
  } //readMethodForImmediateBitValueForCodeGen


  const std::string Node::writeMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass)
  {
    if(isCurrentObjectAPieceOfAnArray(nuti, uvpass))
      return "writeArray"; //TBD

    return "write";
  } //writeMethodForImmediateBitValueForCodeGen


  bool Node::isCurrentObjectALocalVariableOrArgument()
  {
    return !(m_state.m_currentObjSymbolsForCodeGen.empty() || m_state.m_currentObjSymbolsForCodeGen[0]->isDataMember());
  }


  //false means its the entire array or not an array at all (use read())
  bool Node::isCurrentObjectAPieceOfAnArray(UTI cosuti, UlamValue uvpass)
  {
    return(!m_state.isScalar(cosuti) && uvpass.getPtrLen() != (s32) m_state.getTotalBitSize(cosuti));
  }


  std::string Node::allCAPS(const char * s) //static method
  {
    u32 len = strlen(s);
    std::ostringstream up;
    
    for(u32 i = 0; i < len; ++i)
      {
      std::string c(1,(s[i] >= 'a' && s[i] <= 'z') ? s[i]-('a'-'A') : s[i]);
      up << c;
    }
    return up.str();
  } //allCAPS

} //end MFM
