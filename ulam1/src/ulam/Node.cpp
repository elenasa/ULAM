#include <iostream>
#include "Node.h"
#include "CompilerState.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "UlamTypeInt.h"

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

  // only for constants (NodeTerminal)
  bool Node::fitsInBits(UTI fituti)
  {
    assert(0);
    return false;
  }


  Node * Node::makeCastingNode(Node * node, UTI tobeType)
  {
    Node * rtnNode = NULL;
    UTI nuti = node->getNodeType();
    ULAMCLASSTYPE nclasstype = m_state.getUlamTypeByIndex(nuti)->getUlamClass();

    if(nclasstype == UC_NOTACLASS)
      {
	assert(nuti != Atom);
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
  } //warnOfNarrowingCast


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
    assert(vuti != Void);

    //UlamType * vut = m_state.getUlamTypeByIndex(vuti);

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

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    //ULAMCLASSTYPE cosclasstype = cosut->getUlamClass();

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype = stgcosut->getUlamClass();


    // split off reading array items
    if(isCurrentObjectAnArrayItem(cosuti, uvpass) || isCurrentObjectACustomArrayItem(cosuti, uvpass))
      return genCodeReadArrayItemIntoATmpVar(fp, uvpass);


    // write out intermediate tmpVar (i.e. terminal) as temp BitVector arg
    if(uvpass.getPtrNameId() == 0)
      {
	genCodeConvertATmpVarIntoBitVector(fp, uvpass);
	return;
      }

    m_state.indent(fp);
    fp->write("const ");

    //fp->write("u32"); //always read into a u32, after read does sign extend for ints, etc.
    fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp);

	// the READ method
	fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
	fp->write("(");

	// a data member quark, or the element itself should both getBits from self
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
      }
    else if(isCurrentObjectsContainingAnElementParameter())
      {
	genElementParameterMemberNameOfMethod(fp);

	//read method based on last cos
	fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());

	//element parameter is not the cos, rather some preceeding cos,
	// so, the first arg is different depending on the storage cos.
	fp->write("(");
	if(cos->isDataMember() && !cos->isElementParameter())
	  {
	    Symbol * css = m_state.m_currentSelfSymbolForCodeGen;
	    UTI selfuti = css->getUlamTypeIdx();
	    UlamType * selfut = m_state.getUlamTypeByIndex(selfuti);
	    fp->write(selfut->getUlamTypeMangledName(&m_state).c_str());
	    fp->write("<CC>::THE_INSTANCE");
	    fp->write(".");
	    fp->write(stgcos->getMangledName().c_str());

	    if(stgcosclasstype == UC_ELEMENT)
	      {
		//fp->write(".GetBits()"); //like quark?
		fp->write(".getBits()");
	      }
	    else if(stgcosclasstype == UC_QUARK)
	      {
		fp->write(".getBits()");  //m_stg.GetBits
	      }
	    else
	      {
		//NOT A CLASS
	      }
	  }
      }
    else  //local var
      {
	genLocalMemberNameOfMethod(fp);

	//read method based on last cos
	fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());

	// allow for immediate quarks; not element parameters
	// first arg depends on stgcos
	// elements have a 'T' atom storage
#if 0
	if(stgcosclasstype == UC_ELEMENT)
	  {
	    fp->write("(");  //no args to local element read()
	    //fp->write(stgcos->getMangledName().c_str());
	    ////fp->write(".GetBits()");
	    //fp->write(".getBits()");
	  }
	else
#endif
	  {
	    // local quark or primitive (i.e. 'notaclass'); has an immediate type:
	    // uses local variable name, and immediate read method
	    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
	    fp->write("(");
	    if(cosSize > 1)
	      {
		// storage-cos and cos-for-read are different:
		// use this instance of storage-cos to specify
		// its non-static read method
		fp->write(stgcos->getMangledName().c_str());

		//assert(stgcosclasstype == UC_QUARK);
		fp->write(".getBits()");  //m_stg.GetBits
	      }
	  }
      }

    fp->write(");\n");

    // specifically to sign extend Int's (a cast)
    // problem! for arrays, the vut is an Int, regardless of the array typeXXX
    // but not arrays here. hmm..

    //vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);
    cosut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadIntoTmp


  void Node::genCodeReadArrayItemIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    //s32 tmpVarNum = uvpass.getPtrSlotIndex();  //tmp with index
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();  //tmp for data
    vuti = uvpass.getPtrTargetType();  //replaces vuti w target type
    assert(vuti != Void);

    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(vut->getUlamTypeEnum() == Int);

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

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    // split if custom array; like a function call instead
    if(cosut->isCustomArray() && !isHandlingImmediateType())
      return genCodeReadCustomArrayItemIntoATmpVar(fp, uvpass); 

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype = stgcosut->getUlamClass();

    UTI	scalarcosuti = m_state.getUlamTypeAsScalar(cosuti);
    UlamType *	scalarcosut = m_state.getUlamTypeByIndex(scalarcosuti);

    m_state.indent(fp);
    fp->write("const ");

    //fp->write("u32"); //always read into a u32
    fp->write(tmpStorageTypeForReadArrayItem(cosuti, uvpass).c_str());
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(scalarcosuti, tmpVarNum2).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp);

	// the READ method
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());
	fp->write("(");

	// a data member quark, or the element itself should both getBits from self
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", ");  	//rest of arg's
      }
    else if (isCurrentObjectsContainingAnElementParameter())
      {
	if(isCurrentStorageObjectIrrelevantForElementParameter())
	  {
	    //local storage; irrelevant, so let next cos be the storage.
	    assert(m_state.m_currentObjSymbolsForCodeGen.size() > 1);
	    stgcos = m_state.m_currentObjSymbolsForCodeGen[1];
	    stgcosuti = stgcos->getUlamTypeIdx();
	    stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
	    stgcosclasstype = stgcosut->getUlamClass();
	  }

	genElementParameterMemberNameOfMethod(fp);

	//read method based on last cos
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	//element parameter (could be array?)
	fp->write("(");
	if(cos->isDataMember() && !cos->isElementParameter())
	  {
	    Symbol * css = m_state.m_currentSelfSymbolForCodeGen;
	    UTI selfuti = css->getUlamTypeIdx();
	    UlamType * selfut = m_state.getUlamTypeByIndex(selfuti);
	    fp->write(selfut->getUlamTypeMangledName(&m_state).c_str());
	    fp->write("<CC>::THE_INSTANCE");
	    fp->write(".");

	    fp->write(stgcos->getMangledName().c_str());

	    if(stgcosclasstype == UC_ELEMENT)
	      {
		//fp->write(".GetBits()");  like quark?
		fp->write(".getBits()");
		fp->write(", ");   //...rest of args
	      }
	    else if(stgcosclasstype == UC_QUARK)
	      {
		fp->write(".getBits()");  //m_stg.GetBits
		fp->write(", ");   //...rest of args
	      }
	    else
	      {
		//NOT A CLASS
	      }
	  }
      }
    else  //local var
      {
	genLocalMemberNameOfMethod(fp);

	//read method based on last cos
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	// allow for immediate quarks; not element parameters
	//first arg depends on stgcos
	// elements have a 'T' atom storage
	//???	if(cos->isDataMember() && !cos->isElementParameter())

	if(stgcosclasstype == UC_ELEMENT)
	  {
	    fp->write("(");
	    fp->write(stgcos->getMangledName().c_str());
	    //fp->write(".GetBits()");
	    fp->write(".getBits()");
	    fp->write(", ");
	  }
	else
	  {
	    // local quark or primitive (i.e. 'notaclass'); has an immediate type:
	    // uses local variable name, and immediate read method
	    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
	    if(cosSize == 1)
	      {
		fp->write("(");
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
		fp->write(", ");
	      }
	  }
      }

    fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write(", ");
    fp->write_decimal(m_state.getBitSize(cosuti)); //BITS_PER_ITEM
    fp->write("u");
    fp->write(");\n");    //done read element parameter

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, scalarcosuti, m_state.determinePackable(scalarcosuti), m_state, 0);  //POS 0 rightjustified (atom-based).

    // specifically to sign extend Int's (a cast)
    //vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);
    scalarcosut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadArrayItemIntoTmp


  void Node::genCodeReadCustomArrayItemIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    //s32 tmpVarNum = uvpass.getPtrSlotIndex();  //tmp with index
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();  //tmp for data
    vuti = uvpass.getPtrTargetType();  //replaces vuti w target type
    assert(vuti != Void);

    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(vut->getUlamTypeEnum() == Int);

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

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype = stgcosut->getUlamClass();

    assert(isCurrentObjectACustomArrayItem(cosuti, uvpass));

    UTI itemuti = ((UlamTypeClass *) cosut)->getCustomArrayType();
    UlamType * itemut = m_state.getUlamTypeByIndex(itemuti);

    m_state.indent(fp);
    fp->write("const ");

    fp->write(itemut->getImmediateStorageTypeAsString(&m_state).c_str()); //e.g. BitVector<32> exception
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(itemuti, tmpVarNum2, TMPBITVAL).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp);

	// the READ method
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());
	fp->write("(");

	// a data member quark, or the element itself should both getBits from self
	fp->write(m_state.getHiddenArgName());
	fp->write(", ");  	//rest of arg's
      }
    else if(isCurrentObjectsContainingAnElementParameter())
      {
	if(isCurrentStorageObjectIrrelevantForElementParameter())
	  {
	    //local storage; irrelevant, so let next cos be the storage.
	    assert(m_state.m_currentObjSymbolsForCodeGen.size() > 1);
	    stgcos = m_state.m_currentObjSymbolsForCodeGen[1];
	    stgcosuti = stgcos->getUlamTypeIdx();
	    stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
	    stgcosclasstype = stgcosut->getUlamClass();
	  }

	genElementParameterMemberNameOfMethod(fp);

	//read method based on last cos
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	//element parameter holds the cos (could be array?)
	fp->write("(");
	if(cos->isDataMember() && !cos->isElementParameter())
	  {
	    Symbol * css = m_state.m_currentSelfSymbolForCodeGen;
	    UTI selfuti = css->getUlamTypeIdx();
	    UlamType * selfut = m_state.getUlamTypeByIndex(selfuti);
	    fp->write(selfut->getUlamTypeMangledName(&m_state).c_str());
	    fp->write("<CC>::THE_INSTANCE");
	    fp->write(".");

	    fp->write(stgcos->getMangledName().c_str()); //XXXX
	    fp->write(".m_stg");     //immediate EP needs the T storage within the struct
	    fp->write(", ");         //...rest of args
	  }
      }
    else  //local var
      {
	genLocalMemberNameOfMethod(fp);

	//read method based on last cos
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	// allow for immediate quarks; not element parameters
	//first arg depends on stgcos
	// elements have a 'T' atom storage
	//???	if(cos->isDataMember() && !cos->isElementParameter())

	if(stgcosclasstype == UC_ELEMENT)
	  {
	    fp->write("(");
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".m_stg");     //immediate EP needs the T storage within the struct
	    //fp->write(".getBits()"); //???
	    fp->write(", ");
	  }
	else
	  {
	    // local quark or primitive (i.e. 'notaclass'); has an immediate type:
	    // uses local variable name, and immediate read method
	    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
	    if(cosSize == 1)
	      {
		fp->write("(");
	      }
	    else
	      {
		// storage-cos and cos-for-read are different:
		// use this instance of storage-cos to specify
		// its non-static read method
		fp->write("(");
		fp->write(stgcos->getMangledName().c_str());

		assert(stgcosclasstype == UC_QUARK);
		//assert(stgcosclasstype == UC_QUARK || stgcosclasstype == UC_ELEMENT);
		fp->write(", ");
	      }
	  }
      }

    //index is immediate Int arg
    UlamType * intut = m_state.getUlamTypeByIndex(Int);
    fp->write(intut->getImmediateStorageTypeAsString(&m_state).c_str()); //e.g. BitVector<32> exception
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write("));\n");

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarNum2, TMPBITVAL, itemuti, m_state.determinePackable(itemuti), m_state, 0);  //POS 0 rightjustified (atom-based).

    genCodeConvertABitVectorIntoATmpVar(fp, uvpass);  //updates uvpass again

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadCustomArrayItemIntoTmp


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

    u32 pos = 0;  //pos calculated by makePtr(atom-based) (e.g. quark, atom)
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
    if(isCurrentObjectAnArrayItem(vuti, uvpass))
      {
	fp->write_decimal(uvpass.getPtrLen()); //BITS_PER_ITEM
	fp->write("u, ");
	//fp->write_decimal(uvpass.getPtrPos()); //item POS (last like others)
	fp->write_decimal(adjustedImmediateArrayItemPtrPos(vuti, uvpass)); //item POS (last like others) ?
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

    UTI cosuti = cos->getUlamTypeIdx();
    //ULAMCLASSTYPE cosclasstype = m_state.getUlamTypeByIndex(cosuti)->getUlamClass();

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype =  stgcosut->getUlamClass();

    // split if writing an array item, here
    // split if writing a custom array item, here
    if(isCurrentObjectAnArrayItem(cosuti, luvpass) || isCurrentObjectACustomArrayItem(cosuti, luvpass))
      return genCodeWriteArrayItemFromATmpVar(fp, luvpass, ruvpass);


    m_state.indent(fp);

    // a data member quark, or the element itself should both getBits from self;
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp);

	// the WRITE method
	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");

	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", ");  //...rest of args
      }
    else if(isCurrentObjectsContainingAnElementParameter())
      {
	genElementParameterMemberNameOfMethod(fp);

	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());  //was luti
	fp->write("(");

	if(cos->isDataMember() && !cos->isElementParameter())
	  {
	    Symbol * css = m_state.m_currentSelfSymbolForCodeGen;
	    UTI selfuti = css->getUlamTypeIdx();
	    UlamType * selfut = m_state.getUlamTypeByIndex(selfuti);
	    fp->write(selfut->getUlamTypeMangledName(&m_state).c_str());
	    fp->write("<CC>::THE_INSTANCE");
	    fp->write(".");
	    fp->write(stgcos->getMangledName().c_str());

	    if(stgcosclasstype == UC_ELEMENT)
	      {
		//		fp->write(".GetBits()"); like quark?
		fp->write(".getBits()");
		fp->write(", ");   //...rest of args
	      }
	    else if(stgcosclasstype == UC_QUARK)
	      {
		fp->write(".getBits()");  //m_stg.GetBits
		fp->write(", ");          //...rest of args
	      }
	    else
	      {
		//NOT A CLASS
	      }
	  }
      }
    else
      {
	//local
	genLocalMemberNameOfMethod(fp);

	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());  //was luti
	fp->write("(");

	if(cos->isDataMember())
	  {
	    assert(!cos->isElementParameter());

	    // allow for immediate quarks; not element parameters
	    if(stgcosclasstype == UC_ELEMENT)
	      {
		fp->write(stgcos->getMangledName().c_str());
		//fp->write(".GetBits()");
		fp->write(".getBits()");
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
      }

    //VALUE TO BE WRITTEN:
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
	// with immediate quarks, they are read into a tmpreg as other immediates
	// with immediate elements, too!
#if 0
	ULAMCLASSTYPE vclasstype = rut->getUlamClass();
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
#endif
	  fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
      } //value not a terminal

    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();

  } //genCodeWriteFromATmpVar


  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteArrayItemFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
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

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    //ULAMCLASSTYPE cosclasstype = cosut->getUlamClass();

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype =  stgcosut->getUlamClass();

    //assert(isCurrentObjectAnArrayItem(cosuti, luvpass));
    //split if custom array, storage is a data member
    //if(cosut->isCustomArray() && !isCurrentObjectALocalVariableOrArgument())
    if(cosut->isCustomArray() && !isHandlingImmediateType())
      return genCodeWriteCustomArrayItemFromATmpVar(fp, luvpass, ruvpass); //like a function call instead

    // a data member quark, or the element itself should both getBits from self;
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	m_state.indent(fp);

	genMemberNameOfMethod(fp);

	// the WRITE method
	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");

	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", ");  //...rest of args
      }
    else if(isCurrentObjectsContainingAnElementParameter())
      {
	if(isCurrentStorageObjectIrrelevantForElementParameter())
	  {
	    //local storage; irrelevant, so let next cos be the storage.
	    assert(m_state.m_currentObjSymbolsForCodeGen.size() > 1);
	    stgcos = m_state.m_currentObjSymbolsForCodeGen[1];
	    stgcosuti = stgcos->getUlamTypeIdx();
	    stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
	    stgcosclasstype = stgcosut->getUlamClass();
	  }

	m_state.indent(fp);

	genElementParameterMemberNameOfMethod(fp);

	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());  //was luti
	fp->write("(");

	if(cos->isDataMember() && !cos->isElementParameter())
	  {
	    Symbol * css = m_state.m_currentSelfSymbolForCodeGen;
	    UTI selfuti = css->getUlamTypeIdx();
	    UlamType * selfut = m_state.getUlamTypeByIndex(selfuti);
	    fp->write(selfut->getUlamTypeMangledName(&m_state).c_str());
	    fp->write("<CC>::THE_INSTANCE");
	    fp->write(".");
	    fp->write(stgcos->getMangledName().c_str());

	    if(stgcosclasstype == UC_ELEMENT)
	      {
		//fp->write(".GetBits()"); like quark?
		fp->write(".getBits()");
		fp->write(", ");   //...rest of args
	      }
	    else if(stgcosclasstype == UC_QUARK)
	      {
		fp->write(".getBits()");  //m_stg.GetBits
		fp->write(", ");          //...rest of args
	      }
	    else
	      {
		//NOT A CLASS
	      }
	  }
	// (what if array?)
      }
    else
      {
	//local
	m_state.indent(fp);

	genLocalMemberNameOfMethod(fp);

	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());  //was luti
	fp->write("(");

	if(cos->isDataMember() && !cos->isElementParameter())
	  {
	    // allow for immediate quarks; not element parameters
	    if(stgcosclasstype == UC_ELEMENT)
	      {
		fp->write(stgcos->getMangledName().c_str());
		//fp->write(".GetBits()");
		fp->write(".getBits()");
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
	    // (what if array?)
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
	// with immediate quarks, they are read into a tmpreg as other immediates
	// with immediate elements, too!
#if 0
	ULAMCLASSTYPE vclasstype = rut->getUlamClass();
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
#endif
	  
	  fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
      } //value not a terminal

    // rest of array write args..
    //if(cos->isDataMember() && !cos->isElementParameter())
    fp->write(", ");
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write(", ");
    fp->write_decimal(m_state.getBitSize(cosuti)); //BITS_PER_ITEM
    fp->write("u");
    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteArrayItemFromATmpVar

  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteCustomArrayItemFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
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
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype =  stgcosut->getUlamClass();


    assert(isCurrentObjectACustomArrayItem(cosuti, luvpass));

    // a data member quark, or the element itself should both getBits from self;
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	m_state.indent(fp);

	genMemberNameOfMethod(fp);

	// the WRITE method
	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");

	fp->write(m_state.getHiddenArgName());
	fp->write(", ");  //...rest of args
      }
    else if(isCurrentObjectsContainingAnElementParameter())
      {
	if(isCurrentStorageObjectIrrelevantForElementParameter())
	  {
	    //local storage; irrelevant, so let next cos be the storage.
	    assert(m_state.m_currentObjSymbolsForCodeGen.size() > 1);
	    stgcos = m_state.m_currentObjSymbolsForCodeGen[1];
	    stgcosuti = stgcos->getUlamTypeIdx();
	    stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
	    stgcosclasstype = stgcosut->getUlamClass();
	  }

	m_state.indent(fp);

	genElementParameterMemberNameOfMethod(fp);

	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());  //was luti
	fp->write("(");

	if(cos->isDataMember() && !cos->isElementParameter())
	  {
	    Symbol * css = m_state.m_currentSelfSymbolForCodeGen;
	    UTI selfuti = css->getUlamTypeIdx();
	    UlamType * selfut = m_state.getUlamTypeByIndex(selfuti);
	    fp->write(selfut->getUlamTypeMangledName(&m_state).c_str());
	    fp->write("<CC>::THE_INSTANCE");
	    fp->write(".");
	    fp->write(stgcos->getMangledName().c_str());  //XXX
	    fp->write(".m_stg");     //immediate EP needs the T storage within the struct
	    fp->write(", ");         //...rest of args
	  }
	// (what if array?)
      }
    else
      {
	//local
	m_state.indent(fp);

	genLocalMemberNameOfMethod(fp);

	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());  //was luti
	fp->write("(");

	if(cos->isDataMember() && !cos->isElementParameter())
	  {
	    // allow for immediate quarks; not element parameters
	    if(stgcosclasstype == UC_ELEMENT)
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".m_stg");     //immediate EP needs the T storage within the struct
		//fp->write(".getBits()");  //???
		fp->write(", ");         //...rest of args
	      }
	    else if(stgcosclasstype == UC_QUARK)
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(", ");         //...rest of args
	      }
	    else
	      {
		//NOT A CLASS
	      }
	    // (what if array?)
	  }
      }

    //index is immediate Int arg
    UlamType * intut = m_state.getUlamTypeByIndex(Int);
    fp->write(intut->getImmediateStorageTypeAsString(&m_state).c_str()); //e.g. BitVector<32> exception
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write("), ");


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
	// with immediate quarks, they are read into a tmpreg as other immediates
	// with immediate elements, too!
#if 0

	ULAMCLASSTYPE vclasstype = rut->getUlamClass();
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
#endif
	  {
	    fp->write(rut->getImmediateStorageTypeAsString(&m_state).c_str()); //e.g. BitVector<32> exception
	    fp->write("(");
	    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());

	    fp->write(") );\n");
	  }
      } //value not a terminal

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteCustomArrayItemFromATmpVar


  void Node::genMemberNameOfMethod(File * fp)
  {
    assert(!isCurrentObjectALocalVariableOrArgument());

    //nothing if current object is self
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      return;

    //iterate over COS vector; empty if current object is self
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(u32 i = 0; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
      }

    //if last cos is a quark, for Read/Write to work it needs an
    // atomic Parameter type (i.e. Up_Us); not so for custom arrays
    // which are more like a function call
    Symbol * sym = m_state.m_currentObjSymbolsForCodeGen.back();
    UlamType * sut = m_state.getUlamTypeByIndex(sym->getUlamTypeIdx());
    // scalar quarks are typedefs and need atomic parametization;
    // arrays are already atomic parameters
    //if(sut->isScalar() && sut->getUlamClass() == UC_QUARK)
    if(sut->isScalar() && sut->getUlamClass() == UC_QUARK && !sut->isCustomArray())
      {
	fp->write("Up_Us::");  //gives quark an atomicparameter type for write
      }
  } //genMemberNameOfMethod


  // "static" data member, a mixture of local variable and dm;
  // requires THE_INSTANCE, and local variables are superfluous.
  void Node::genElementParameterMemberNameOfMethod(File * fp)
  {
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    if(cosSize == 1)
      {
	Symbol * css = m_state.m_currentSelfSymbolForCodeGen;
	UTI selfuti = css->getUlamTypeIdx();
	UlamType * selfut = m_state.getUlamTypeByIndex(selfuti);
	fp->write(selfut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("<CC>::THE_INSTANCE");
	fp->write(".");
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".");
	return;
      }

    u32 cosStart = 1;
    UTI uti = stgcos->getUlamTypeIdx();

    //skip first stgcos if a local variable of same type as the 'self' and not the element parameter
    if(isCurrentStorageObjectIrrelevantForElementParameter())
      {
	stgcos = m_state.m_currentObjSymbolsForCodeGen[1]; //use next one
	uti = stgcos->getUlamTypeIdx();
	cosStart++;
      }

    UlamType * ut = m_state.getUlamTypeByIndex(uti);

    // since elements are also immediate..
    fp->write(ut->getImmediateStorageTypeAsString(&m_state).c_str());
    fp->write("::");
    fp->write("Us::");   //typedef

#if 0
    ULAMCLASSTYPE classtype = ut->getUlamClass();
    if(classtype == UC_QUARK)
      {
	fp->write(ut->getImmediateStorageTypeAsString(&m_state).c_str());
	fp->write("::");
	fp->write("Us::");   //typedef
      }
    else if(classtype == UC_ELEMENT)
      {
	// if data member or element parameter (i.e. not a local var)
	//if(!isCurrentObjectALocalVariableOrArgument() || isCurrentObjectsContainingAnElementParameter())
	  {
	    fp->write(ut->getUlamTypeMangledName(&m_state).c_str());
	    fp->write("<CC>::");
	  }
	//otherwise a local var which we can skip since EP's are static
      }
#endif

    for(u32 i = cosStart; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	ULAMCLASSTYPE sclasstype = sut->getUlamClass();
	if(sym->isElementParameter())
	  {
	    if(sclasstype == UC_NOTACLASS)  //atom too???
	      {
		fp->write(sym->getMangledName().c_str());
		fp->write(".");
	      }
	    else
	      {
		//now for both immmediate elements and quarks..
		fp->write(sut->getImmediateStorageTypeAsString(&m_state).c_str());
		fp->write("::");
		if( ((i + 1) < cosSize))  //still another cos refiner, use
		  fp->write("Us::");      //typedef
	      }
#if 0
	    else if(sclasstype == UC_QUARK)
	      {
		fp->write(sut->getImmediateStorageTypeAsString(&m_state).c_str());
		fp->write("::");
		if( ((i + 1) < cosSize))  //still another cos refiner, use
		  fp->write("Us::");      //typedef
	      }
	    else
	      {
		//UC_ELEMENT
		fp->write(sut->getUlamTypeMangledName(&m_state).c_str());
		fp->write("<CC>::");
	      }
#endif

	  }
	else
	  {
	    //not the element parameter, but a data member..
	    fp->write(sym->getMangledNameForParameterType().c_str());
	    fp->write("::");
	    // if its the last cos, a quark, and not a custom array...
	    if(sclasstype == UC_QUARK && (i + 1 == cosSize) && sut->isScalar() && !sut->isCustomArray())
	      fp->write("Up_Us::");   //atomic parameter needed
	  }
      }
  } //genElementParameterMemberNameOfMethod


  void Node::genLocalMemberNameOfMethod(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    // element parameter has its own storage, like a local
    assert(!isCurrentObjectsContainingAnElementParameter());

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    if(cosSize == 1)
      {
	fp->write(stgcos->getMangledName().c_str());
	fp->write(".");
	return;
      }

    UTI uti = stgcos->getUlamTypeIdx();
    UlamType * ut = m_state.getUlamTypeByIndex(uti);

    // now for both immediate elements and quarks..
    fp->write(ut->getImmediateStorageTypeAsString(&m_state).c_str());
    fp->write("::");
    fp->write("Us::");   //typedef

#if 0
    ULAMCLASSTYPE classtype = ut->getUlamClass();
    // if local element, first arg of read is all that's req'd?
    if(classtype == UC_QUARK)
      {
	fp->write(ut->getImmediateStorageTypeAsString(&m_state).c_str());
	fp->write("::");
	fp->write("Us::");   //typedef
      }
    else if(classtype == UC_ELEMENT)
      {
	fp->write(ut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("<CC>::");
      }
    else
      {
	assert(0);
	//NOTACLASS
      }
#endif

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


  const std::string Node::tmpStorageTypeForRead(UTI nuti, UlamValue uvpass)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etyp = nut->getUlamTypeEnum();

    //special case, u32/u64 desired before AfterReadingIntoATmpVar
    if(etyp == Int)
      return ((UlamTypeInt *) nut)->getUnsignedTmpStorageTypeAsString(&m_state);

    return nut->getTmpStorageTypeAsString(&m_state);
  } //tmpStorageTypeForRead


  const std::string Node::tmpStorageTypeForReadArrayItem(UTI nuti, UlamValue uvpass)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etyp = nut->getUlamTypeEnum();

    //special case, u32/u64 desired before AfterReadingIntoATmpVar
    if(etyp == Int)
      return ((UlamTypeInt *) nut)->getArrayItemUnsignedTmpStorageTypeAsString(&m_state);

    return nut->getArrayItemTmpStorageTypeAsString(&m_state);
  } //tmpStorageTypeForReadArrayItem


  const std::string Node::readMethodForCodeGen(UTI nuti, UlamValue uvpass)
  {
    std::string method;
    if(isHandlingImmediateType())
      method = readMethodForImmediateBitValueForCodeGen(nuti, uvpass);
    else
      method = m_state.getUlamTypeByIndex(nuti)->readMethodForCodeGen();
    return method;
  } //readMethodForCodeGen


  const std::string Node::readArrayItemMethodForCodeGen(UTI nuti, UlamValue uvpass)
  {
    std::string method;
    assert(isCurrentObjectAnArrayItem(nuti, uvpass) || isCurrentObjectACustomArrayItem(nuti, uvpass));

    if(isHandlingImmediateType())
      method = readMethodForImmediateBitValueForCodeGen(nuti, uvpass);
    else
      method = m_state.getUlamTypeByIndex(nuti)->readArrayItemMethodForCodeGen();
    return method;
  } //readArrayItemMethodForCodeGen


  const std::string Node::writeMethodForCodeGen(UTI nuti, UlamValue uvpass)
  {
    std::string method;
    if(isHandlingImmediateType())
      method = writeMethodForImmediateBitValueForCodeGen(nuti, uvpass);
    else
      method = m_state.getUlamTypeByIndex(nuti)->writeMethodForCodeGen();
    return method;
  } //writeMethodForCodeGen


  const std::string Node::writeArrayItemMethodForCodeGen(UTI nuti, UlamValue uvpass)
  {
    std::string method;
    assert(isCurrentObjectAnArrayItem(nuti, uvpass) || isCurrentObjectACustomArrayItem(nuti, uvpass));

    if(isHandlingImmediateType())
      method = writeMethodForImmediateBitValueForCodeGen(nuti, uvpass);
    else
      method = m_state.getUlamTypeByIndex(nuti)->writeArrayItemMethodForCodeGen();
    return method;
  } //writeArrayItemMethodForCodeGen


  //each knows which ReadXXX method to use based on their key(arraysize, bitsize, type);
  const std::string Node::readMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass)
  {
    if(isCurrentObjectAnArrayItem(nuti, uvpass) || isCurrentObjectACustomArrayItem(nuti, uvpass))
      {
	return "readArrayItem"; //piece
      }

    //note: for the entire array, ie not just a piece, we want to use 'read'
    //      rather then readArrayXXX; unless we're talking UNPACKED.
    //      For Int's, there is automatich no sign extension for entire arrays.
    return "read";
  } //readMethodForImmediateBitValueForCodeGen


  const std::string Node::writeMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass)
  {
    if(isCurrentObjectAnArrayItem(nuti, uvpass) || isCurrentObjectACustomArrayItem(nuti, uvpass))
      return "writeArrayItem"; //TBD piece

    return "write";
  } //writeMethodForImmediateBitValueForCodeGen


  bool Node::isCurrentObjectALocalVariableOrArgument()
  {
    // include element parameters as LocalVariableOrArgument, since more alike XXX
    return !(m_state.m_currentObjSymbolsForCodeGen.empty() || (m_state.m_currentObjSymbolsForCodeGen[0]->isDataMember() && !isCurrentObjectsContainingAnElementParameter()));
  }


  bool Node::isCurrentObjectsContainingAnElementParameter()
  {
    bool hasEP = false;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(u32 i = 0; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(sym->isElementParameter())
	  {
	    hasEP = true;
	    break;
	  }
      }
    return hasEP;
  } //isCurrentObjectsContainingAnElementParameter


  bool Node::isCurrentStorageObjectIrrelevantForElementParameter()
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    return (!stgcos->isElementParameter() && stgcosuti == m_state.m_currentSelfSymbolForCodeGen->getUlamTypeIdx());
  }


  //false means its the entire array or not an array at all (use read() if PACKEDLOADABLE)
  bool Node::isCurrentObjectAnArrayItem(UTI cosuti, UlamValue uvpass)
  {
    //uvpass would be an array index (an int of sorts), not an array; types would not be the same;
    //return(!m_state.isScalar(cosuti) && uvpass.getPtrLen() != (s32) m_state.getTotalBitSize(cosuti));
    return(!m_state.isScalar(cosuti) && m_state.isScalar(uvpass.getPtrTargetType()) );
  }


  bool Node::isCurrentObjectACustomArrayItem(UTI cosuti, UlamValue uvpass)
  {
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    // a cosuti as a scalar, customarray, may be used as a regular array,
    //     but at this point cosuti would be a scalar in either case (sigh);
    // uvpass would be an array index (an int of sorts), not an array; types would not be the same;
    //    return(m_state.isScalar(cosuti) && isCustomArray && m_state.isScalar(uvpass.getPtrTargetType()) );
    return(m_state.isScalar(cosuti) && cosut->isCustomArray() && uvpass.getPtrTargetType() != cosuti);
  }


  bool Node::isHandlingImmediateType()
  {
    // a local var or arg that's either not refining or an element parameter
    return (isCurrentObjectALocalVariableOrArgument() && ( (m_state.m_currentObjSymbolsForCodeGen.size() == 1) || (m_state.m_currentObjSymbolsForCodeGen.back()->isElementParameter())));
  }


  u32 Node::adjustedImmediateArrayItemPtrPos(UTI cosuti, UlamValue uvpass)
  {
    u32 pos = uvpass.getPtrPos();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    if(cosut->getUlamClass() == UC_NOTACLASS)
      {
	assert(cosuti != Atom);  //atom too??? let's find out..
	s32 wordsize = cosut->getTotalWordSize();
	pos = wordsize - (BITSPERATOM - pos); //cosut->getTotalBitSize();
      }
    return pos;
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
