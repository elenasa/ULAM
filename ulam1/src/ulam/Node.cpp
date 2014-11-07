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
     
    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    vuti = uvpass.getPtrTargetType();  //replaces vuti w target type

    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    UTI cosuti = m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx();

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	m_state.indent(fp);
	fp->write("const ");
	
	fp->write("u32"); //always read into a u32
	fp->write(" ");
	
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum).c_str());
	fp->write(" = ");

#if 0
	UTI selfuti = m_state.m_currentSelfSymbolForCodeGen->getUlamTypeIdx();
	ULAMCLASSTYPE vclasstype = vut->getUlamClass();
	
	// initialize to default type of self's element
	if(vclasstype == UC_QUARK || vclasstype == UC_ELEMENT)
	  {
	    fp->write(m_state.getUlamTypeByIndex(selfuti)->getUlamTypeMangledName(&m_state).c_str());
	    fp->write("<CC>");
	    fp->write("::");
	    fp->write("GetDefaultAtom();\n");
	    m_state.indent(fp);
	    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum).c_str());
	    fp->write(" = ");
	  }
#endif

	genMemberNameOfMethod(fp, uvpass);
	
	// the READ method
	fp->write(readMethodForCodeGen(cosuti).c_str());
	fp->write("("); 	    
	
	// a data member quark, or the element itself should both getBits from self
	if(!isCurrentObjectALocalVariableOrArgument())
	  {
	    fp->write(m_state.getHiddenArgName());
	    fp->write(".GetBits()");
	  }
	else
	  {
	    //local 
	    assert(uvpass.getPtrNameId() > 0);
	    Symbol * sym;
	    assert(m_state.alreadyDefinedSymbol(uvpass.getPtrNameId(), sym));
	    fp->write(sym->getMangledName().c_str());
	  }

	if(!m_state.isScalar(cosuti))
	  {
	    fp->write(", ");
	    fp->write_decimal(m_state.getTotalBitSize(cosuti)); //array size
	    fp->write(", ");
	    fp->write_decimal(uvpass.getPtrLen()); //BITS_PER_ITEM
	    fp->write(", ");
	    fp->write_decimal(uvpass.getPtrPos()); //item POS (last like others)
	  }
	
	fp->write(");\n");

	// specifically to sign extend Int's (a cast)
	vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);
      } 
    else
      {
	//local
	if(uvpass.getPtrNameId() > 0)
	  {
	    m_state.indent(fp);
	    fp->write("const ");
	    
	    fp->write(vut->getTmpStorageTypeAsString(&m_state).c_str()); //e.g. u32, s32, u64, etc.
	    //fp->write("u32");  //always read into a u32
	    
	    fp->write(" ");
	    
	    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum).c_str());
	    fp->write(" = ");

	    genLocalMemberNameOfMethod(fp, uvpass);

	    ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(cosuti)->getUlamClass();
	    
	    // allow for immediate quarks
	    //if(classtype == UC_NOTACLASS)
	    if(classtype != UC_ELEMENT)
	      {
		fp->write(readMethodForCodeGen(vuti).c_str());
		fp->write("();\n");
	      }
	    else
	      {
		fp->write(readMethodForCodeGen(vuti).c_str());
		fp->write("(");
		fp->write(m_state.m_currentObjSymbolForCodeGen->getMangledName().c_str());
		fp->write(".GetBits()");
		fp->write(");\n");

		// specifically to sign extend Int's (a cast)
		vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);		
	      }
	  }
	else
	  {	  
	    // write out intermediate tmpVar as temp BitVector arg
	    genCodeConvertATmpVarIntoBitVector(fp, uvpass);
	  }
      }
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

    uvpass = UlamValue::makePtr(tmpVarNum2, TMPBITVAL, vuti, m_state.determinePackable(vuti), m_state, 0);  //POS 0 rightjustified.
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
    fp->write(readMethodForImmediateBitValueForCodeGen(vuti).c_str());
    fp->write("();\n");

    uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, vuti, m_state.determinePackable(vuti), m_state, 0);  //POS 0 rightjustified.

    // specifically to sign extend Int's (a cast)
    //vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);    
  } //genCodeConvertABitVectorIntoATmpVar


  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    assert(luvpass.getUlamValueTypeIdx() == Ptr);

    bool isTerminal = false;
    UTI luti = luvpass.getPtrTargetType();
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
    UTI cosuti = m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx();

    // a data member quark, or the element itself should both getBits from self;
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	m_state.indent(fp);
	
	genMemberNameOfMethod(fp, luvpass);

	// the WRITE method
	//fp->write(writeMethodForCodeGen(ruti).c_str());
	fp->write(writeMethodForCodeGen(cosuti).c_str());
	fp->write("(");
	
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", ");
      }
    else
      {
	//local
	m_state.indent(fp);
	
	genLocalMemberNameOfMethod(fp, luvpass);

	ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(cosuti)->getUlamClass();

	// allow for immediate quarks
	//if(classtype == UC_NOTACLASS)
	if(classtype != UC_ELEMENT)
	  {
	    fp->write(writeMethodForCodeGen(luti).c_str());
	    fp->write("(");
	  }
	else
	  {
	    fp->write(writeMethodForCodeGen(luti).c_str());
	    fp->write("(");
	    fp->write(m_state.m_currentObjSymbolForCodeGen->getMangledName().c_str());
	    fp->write(".GetBits()");
	    fp->write(", ");
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

	// with immediate quarks, they are read into a tmpreg as others
	//if(vclasstype == UC_QUARK || vclasstype == UC_ELEMENT)
	if(vclasstype == UC_ELEMENT)
	  {
	    //need to read from the tmpVar ??? (need a test case!)
	    //fp->write(rut->getUlamTypeImmediateMangledName(&m_state).c_str());
	    ruvpass.genCodeBitField(fp, m_state);
	    fp->write("::");
	    fp->write(readMethodForCodeGen(ruti).c_str());
	    fp->write("(");
	    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
	    fp->write(")");
	  }
	else
	  {
	    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
	  }
	
	// rest of array write args 
	//if(!m_state.isScalar(ruti))
	if(!m_state.isScalar(cosuti))
	  {
	    fp->write(", ");
	    fp->write_decimal(m_state.getTotalBitSize(cosuti)); //array size
	    fp->write(", ");
	    fp->write_decimal(luvpass.getPtrLen()); //BITS_PER_ITEM
	    fp->write(", ");
	    fp->write_decimal(luvpass.getPtrPos()); //item POS (last like others)	
	  }
	
	fp->write(");\n");
      } //not terminal
  } //genCodeWriteFromATmpVar


  void Node::genMemberNameOfMethod(File * fp, UlamValue uvpass)
  {
    assert(!isCurrentObjectALocalVariableOrArgument());

    UlamType * ut = m_state.getUlamTypeByIndex(m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx());
    ULAMCLASSTYPE classtype = ut->getUlamClass();
	
    if(classtype == UC_NOTACLASS)
      {
	//assert(0);
	// we need the atomicparametertype for this primitive data member
	fp->write(m_state.m_currentObjSymbolForCodeGen->getMangledNameForParameterType().c_str());
	fp->write("::");
      }
    else
      {
	if(m_state.m_currentObjSymbolForCodeGen->isDataMember()) //within an element
	  {             
	    // Use CSS self symbol? e.g. when COS is an array
	    // of quarks, we're NOT looking for the primitive within
	    // the quark, rather the member of the element (CSS). 
	    // here, it-> does not refer to the 'self'
	    NodeBlockClass * memberClassNode = NULL;
	    UlamType * sut = m_state.getUlamTypeByIndex(m_state.m_currentSelfSymbolForCodeGen->getUlamTypeIdx());
	    ULAMCLASSTYPE selfclasstype = sut->getUlamClass();

	    //e.g. array of quarks
	    //if(ut->isScalar())
	    if(selfclasstype == UC_QUARK)
	      {
		SymbolClass * csym = NULL;
		assert(m_state.alreadyDefinedSymbolClass(ut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym));
		
		memberClassNode = csym->getClassBlockNode();	  
		assert(memberClassNode);  // e.g. forgot the closing brace on quark definition
	      }
	    else
	      {
		memberClassNode = ((SymbolClass *) m_state.m_currentSelfSymbolForCodeGen)->getClassBlockNode(); 
	      }

	    m_state.m_currentMemberClassBlock = memberClassNode;
	    m_state.m_useMemberBlock = true;
	    
	    Symbol * sym = NULL;
	    assert(m_state.alreadyDefinedSymbol(uvpass.getPtrNameId(),sym)); //failed last time
	    
	    m_state.m_useMemberBlock = false;
	    m_state.m_currentMemberClassBlock = NULL;
	    
	    // we need the atomicparametertype for this quark's data member m_val_b!!!
	    // unless COS is an array of quarks!!!
	    fp->write(sym->getMangledNameForParameterType().c_str());
	    fp->write("::");
	    fp->write("Up_Us::");  //entire quark atomic parameter for ReadRaw/WriteRaw
	  }
	else  //cos == css (self) 
	  {
	    //within self so no need for member name ?
	    fp->write(m_state.m_currentSelfSymbolForCodeGen->getMangledNameForParameterType().c_str());
	    fp->write("::");
	  }
      }    
  } //genMemberNameOfMethod


  void Node::genLocalMemberNameOfMethod(File * fp, UlamValue uvpass)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    UTI uti = m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx();
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    ULAMCLASSTYPE classtype = ut->getUlamClass();

    // hence a primitive type local variable
    //if(classtype == UC_NOTACLASS)
    if(classtype == UC_NOTACLASS || (classtype == UC_QUARK && ut->isScalar()))
      {
	Symbol * sym;
	assert(m_state.alreadyDefinedSymbol(uvpass.getPtrNameId(), sym));

	fp->write(sym->getMangledName().c_str());
	fp->write(".");
      }
    else
      {
	// since neither a data member nor self:
	// COS is a local element or array of quarks; 
	// use uvpass to get symbol id.
	assert(!m_state.m_currentObjSymbolForCodeGen->isDataMember()); 
	
	NodeBlockClass * memberClassNode = NULL;
	UlamType * sut = m_state.getUlamTypeByIndex(m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx());
	SymbolClass * csym = NULL;
	assert(m_state.alreadyDefinedSymbolClass(sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym));
		
	memberClassNode = csym->getClassBlockNode();	  
	assert(memberClassNode);  // e.g. forgot the closing brace on quark definition

	m_state.m_currentMemberClassBlock = memberClassNode;
	m_state.m_useMemberBlock = true;
	    
	assert(uvpass.getPtrNameId()); //not zero!
	Symbol * sym = NULL;
	assert(m_state.alreadyDefinedSymbol(uvpass.getPtrNameId(),sym)); //failed last time
	
	m_state.m_useMemberBlock = false;
	m_state.m_currentMemberClassBlock = NULL;
	    
	// we need the atomicparametertype for this quark's data member m_val_b!!!
	// unless COS is an array of quarks!!!
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


  const std::string Node::readMethodForCodeGen(UTI nuti)
  {
    std::string method;
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(!isCurrentObjectALocalVariableOrArgument())
      { 
	method = nut->readMethodForCodeGen();
      }
    else
      {
	//local
	UTI cosuti = m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx();
	UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
	ULAMCLASSTYPE classtype = cosut->getUlamClass();	

	// allow for immediate quarks
	//if(classtype == UC_NOTACLASS)
	if(classtype != UC_ELEMENT)
	  {
	    method = readMethodForImmediateBitValueForCodeGen(nuti);
	  }
	else
	  {
	    method = nut->readMethodForCodeGen();
	  }
      }
    return method;
  } //readMethodForCodeGen


  const std::string Node::writeMethodForCodeGen(UTI nuti)
  {
    std::string method;
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(!isCurrentObjectALocalVariableOrArgument())
      {
	method = nut->writeMethodForCodeGen();
      }
    else
      {
	//local
	UTI cosuti = m_state.m_currentObjSymbolForCodeGen->getUlamTypeIdx();
	UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
	ULAMCLASSTYPE classtype = cosut->getUlamClass();	

	// allow for immediate quarks
	//if(classtype == UC_NOTACLASS)
	if(classtype != UC_ELEMENT)
	  {
	    method = writeMethodForImmediateBitValueForCodeGen(nuti);
	  }
	else
	  {
	    method = nut->writeMethodForCodeGen();
	  }
      }
    return method;
  } //writeMethodForCodeGen


  const std::string Node::readMethodForImmediateBitValueForCodeGen(UTI nuti)
  {
    std::string method;
    if(m_state.isScalar(nuti))
      {     
	method = "read";
      }
    else
      {
	method = "readArray"; //TBD
      }
    return method;
  } //readMethodForImmediateBitValueForCodeGen


  const std::string Node::writeMethodForImmediateBitValueForCodeGen(UTI nuti)
  {
    std::string method;
	if(m_state.isScalar(nuti))
	  {     
	    method = "write";
	  }
	else
	  {
	    method = "writeArray";
	  }
	return method;
  } //writeMethodForImmediateBitValueForCodeGen


  bool Node::isCurrentObjectALocalVariableOrArgument()
  {
    return !(m_state.m_currentObjSymbolForCodeGen->isDataMember() || (m_state.m_currentObjSymbolForCodeGen == m_state.m_currentSelfSymbolForCodeGen));
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
