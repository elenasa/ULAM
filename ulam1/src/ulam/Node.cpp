#include <iostream>
#include "Node.h"
#include "CompilerState.h"
#include "NodeFunctionCall.h"
#include "NodeMemberSelect.h"
#include "UlamTypeInt.h"

namespace MFM {

  Node::Node(CompilerState & state): m_state(state), m_storeIntoAble(false), m_nodeUType(Nav), m_parentNo(0), m_nodeNo(m_state.getNextNodeNo()) {}

  Node::Node(const Node & ref) : m_state(ref.m_state), m_storeIntoAble(ref.m_storeIntoAble), m_nodeUType(ref.m_nodeUType), m_nodeLoc(ref.m_nodeLoc), m_parentNo(ref.m_parentNo), m_nodeNo(ref.m_nodeNo) /* same NNO */ {}

  void Node::setYourParentNo(NNO pno)
  {
    if(m_parentNo > 0)
      assert(m_parentNo == pno);
    m_parentNo = pno;
  }

  void Node::updateLineage(NNO pno)
  {
    setYourParentNo(pno);  //walk the tree..a leaf.
  }

  NNO Node::getNodeNo()
  {
    return m_nodeNo;
  }

  bool Node::findNodeNo(NNO n, Node *& foundNode)
  {
    if(m_nodeNo == n) //leaf
      {
	foundNode = this;
	return true;
      }
    return false;
  } //findNodeNo

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
  } //print

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

  void Node::constantFold(Token tok)
  {
    assert(0);  //only NodeTerminal has this defined
  }

  // any node above assignexpr is not storeintoable;
  // and has no type (e.g. statements, statement, block, program)
  UTI Node::checkAndLabelType()
  {
    m_nodeUType = Nav;
    m_storeIntoAble = false;
    return m_nodeUType;
  }

  void Node::countNavNodes(u32& cnt)
  {
    if(getNodeType() == Nav)
      cnt += 1;
  }

  // only for constants (NodeTerminal)
  bool Node::fitsInBits(UTI fituti)
  {
    return true;
  }

  // only for constants (NodeTerminal)
  bool Node::isNegativeConstant()
  {
    assert(0);
    return false;
  }

  bool Node::isWordSizeConstant()
  {
    assert(0);
    return false;
  }

  bool Node::installSymbolTypedef(Token atok, s32 bitsize, s32 arraysize, UTI classInstanceIdx, Symbol *& asymptr)
  {
    return false;
  }

  bool Node::installSymbolConstantValue(Token aTok, s32 bitsize, s32 arraysize, Symbol *& asymptr)
  {
    return false;
  }

  bool Node::installSymbolVariable(Token atok, s32 bitsize, s32 arraysize, UTI classInstanceIdx, UTI declListScalarType, Symbol *& asymptr)
  {
    return false;
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

    assert((UlamType::compare(rtnUVtype, getNodeType(), m_state) == UTIC_SAME) || rtnUVtype == UAtom || getNodeType() == UAtom);

    // save results in the stackframe for caller;
    // copies each element of the 'unpacked' array by value,
    // in reverse order ([0] is last at bottom)
    s32 slots = m_state.slotsNeeded(rtnUVtype);

    //where to put the return value..'return' statement uses STACK
    UlamValue rtnPtr = UlamValue::makePtr(-slots, where, rtnUVtype, m_state.determinePackable(rtnUVtype), m_state);
    m_state.assignValue(rtnPtr, rtnUV);
  } //assignReturnValueToStack


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

  void Node::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    std::ostringstream msg;
    msg << "genCodeToStoreInto called on Node type: " << m_state.getUlamTypeNameByIndex(getNodeType()).c_str() << ", and failed.";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
    assert(0);
    return;
  } //genCodeToStoreInto

  void Node::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    vuti = uvpass.getPtrTargetType();  //replaces vuti w target type
    assert(vuti != Void);

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

    // split off reading array items
    if(isCurrentObjectAnArrayItem(cosuti, uvpass) || isCurrentObjectACustomArrayItem(cosuti, uvpass))
      return genCodeReadArrayItemIntoATmpVar(fp, uvpass);

    // write out intermediate tmpVar (i.e. terminal) as temp BitVector arg
    // e.g. when func call is rhs of secondary member select
    if(uvpass.getPtrNameId() == 0)
      {
	genCodeConvertATmpVarIntoBitVector(fp, uvpass);
	return;
      }

    m_state.indent(fp);
    fp->write("const ");

    // after read does sign extend for ints, etc.
    fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, uvpass.getPtrStorage()).c_str());
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
	fp->write(");\n");
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAnElementParameter();
	if(epi >= 0)
	  {
	    genElementParameterMemberNameOfMethod(fp, epi);

	    //read method based on last cos
	    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());

	    fp->write("(");

	    //storage based on epi-1
	    if(!isHandlingImmediateType())
	      {
		genElementParameterHiddenArgs(fp, epi);
	      }
	    fp->write(");\n");
	  }
	else  //local var
	  {
	    if(stgcos->isSelf())
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(";\n");
	      }
	    else
	      {
		genLocalMemberNameOfMethod(fp);

		//read method based on last cos
		fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());

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
		    fp->write(".getBits()");
		  }
		fp->write(");\n");
	      }
	  }
      }

    // specifically to sign extend Int's (a cast)
    // problem! for arrays, the vut is an Int, regardless of the array typeXXX
    // but not arrays here. hmm..
    //vut->genCodeAfterReadingIntoATmpVar(fp, uvpass, m_state);
    cosut->genCodeAfterReadingIntoATmpVar(fp, uvpass);

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
    ULAMTYPE vetype = vut->getUlamTypeEnum();
    assert( vetype == Int || vetype == Unsigned);

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

    // split if custom array, that requires an 'aref' function call
    // (that's when cos is neither a local var that's not refining, i.e. cos[0];
    // nor a local var that's an EP); o.w. immediate types have an array method
    // to call even for CA's.
    if(cosut->isCustomArray() && !isHandlingImmediateType())
      return genCodeReadCustomArrayItemIntoATmpVar(fp, uvpass);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype = stgcosut->getUlamClass();

    UTI	scalarcosuti = m_state.getUlamTypeAsScalar(cosuti);
    UlamType *	scalarcosut = m_state.getUlamTypeByIndex(scalarcosuti);

    m_state.indent(fp);
    fp->write("const ");

    fp->write(tmpStorageTypeForReadArrayItem(cosuti, uvpass).c_str());
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(scalarcosuti, tmpVarNum2).c_str());
    fp->write(" = ");

    // all the cases where '=' is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp);

	// the READ method
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());
	fp->write("(");

	if(cosut->isCustomArray())
	  fp->write("uc, ");

	// a data member quark, or the element itself should both getBits from self
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", ");  	//rest of arg's
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAnElementParameter();
	if(epi >= 0)
	  {
	    genElementParameterMemberNameOfMethod(fp, epi);

	    //read method based on last cos
	    fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	    //element parameter (could be array?)
	    fp->write("(");

	    if(!isHandlingImmediateType())
	      {
		genElementParameterHiddenArgs(fp, epi);
		fp->write(", ");  	//rest of arg's
	      }
	    else
	      {
		if(cosut->isCustomArray())
		  fp->write("uc, "); 	    //rest of arg's
	      }
	  }
	else  //local var
	  {
	    genLocalMemberNameOfMethod(fp);

	    //read method based on last cos
	    fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	    if(stgcosclasstype == UC_ELEMENT)
	      {
		fp->write("(");

		if(cosut->isCustomArray())
		  fp->write("uc, ");

		fp->write(stgcos->getMangledName().c_str());
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
		    if(cosut->isCustomArray())
		      fp->write("uc, ");
		  }
		else
		  {
		    // storage-cos and cos-for-read are different:
		    // use this instance of storage-cos to specify
		    // its non-static read method
		    fp->write("(");
		    if(cosut->isCustomArray())
		      fp->write("uc, ");

		    fp->write(stgcos->getMangledName().c_str());

		    assert(stgcosclasstype == UC_QUARK);
		    fp->write(".getBits()");
		    fp->write(", ");
		  }
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
    scalarcosut->genCodeAfterReadingIntoATmpVar(fp, uvpass);

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadArrayItemIntoTmp

  void Node::genCodeReadCustomArrayItemIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();  //tmp for data
    vuti = uvpass.getPtrTargetType();  //replaces vuti w target type
    assert(vuti != Void);

    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMTYPE vetype = vut->getUlamTypeEnum();
    assert( vetype == Int || vetype == Unsigned);

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

    fp->write(itemut->getImmediateStorageTypeAsString().c_str()); //e.g. BitVector<32> exception
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(itemuti, tmpVarNum2, TMPBITVAL).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp);

	// the READ method
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());
	fp->write("(uc, ");

	fp->write(m_state.getHiddenArgName());  // no getBits
	fp->write(", ");  	//rest of arg's
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAnElementParameter();
	if(epi >= 0)
	  {
	    genElementParameterMemberNameOfMethod(fp, epi);

	    //read method based on last cos
	    fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	    fp->write("(");

	    //storage based on epi - 1
	    if(!isHandlingImmediateType())
	      {
		genElementParameterHiddenArgs(fp, epi);
		fp->write(", ");       //rest of args
	      }
	    else
	      {
		if(cosut->isCustomArray())
		  fp->write("uc, "); 	    //rest of arg's
	      }
	  }
	else  //local var
	  {
	    genLocalMemberNameOfMethod(fp);

	    //read method based on last cos
	    fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	    if(stgcosclasstype == UC_ELEMENT)
	      {
		fp->write("(uc, ");
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".getRef()");     //immediate EP needs the T storage within the struct
		fp->write(", ");
	      }
	    else
	      {
		// local quark or primitive (i.e. 'notaclass'); has an immediate type:
		// uses local variable name, and immediate read method
		u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
		if(cosSize == 1)
		  {
		    fp->write("(uc, ");
		  }
		else
		  {
		    // storage-cos and cos-for-read are different:
		    // use this instance of storage-cos to specify
		    // its non-static read method
		    fp->write("(uc, ");
		    fp->write(stgcos->getMangledName().c_str());

		    assert(stgcosclasstype == UC_QUARK);
		    fp->write(", ");
		  }
	      }
	  }
      }

    //index is immediate Int arg
    UlamType * intut = m_state.getUlamTypeByIndex(Int);
    fp->write(intut->getImmediateStorageTypeAsString().c_str()); //e.g. BitVector<32> exception
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write("));\n");

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarNum2, TMPBITVAL, itemuti, m_state.determinePackable(itemuti), m_state, 0);  //POS 0 rightjustified (atom-based).

    genCodeConvertABitVectorIntoATmpVar(fp, uvpass);  //updates uvpass again

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadCustomArrayItemIntoTmp

  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    assert(luvpass.getUlamValueTypeIdx() == Ptr);

    bool isTerminal = false;
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
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype =  stgcosut->getUlamClass();

    // split if writing an array item, here
    // split if writing a custom array item, here
    if(isCurrentObjectAnArrayItem(cosuti, luvpass) || isCurrentObjectACustomArrayItem(cosuti, luvpass))
      return genCodeWriteArrayItemFromATmpVar(fp, luvpass, ruvpass);

    if(stgcos->isSelf())
      return genCodeWriteToSelfFromATmpVar(fp, luvpass, ruvpass);

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
	fp->write(", ");  //rest of args
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAnElementParameter();
	if(epi >= 0)
	  {
	    genElementParameterMemberNameOfMethod(fp, epi);

	    fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(");

	    //storage based on epi - 1
	    if(!isHandlingImmediateType())
	      {
		genElementParameterHiddenArgs(fp, epi);
		fp->write(", ");  	//rest of arg's
	      }
	  }
	else
	  {
	    //local
	    genLocalMemberNameOfMethod(fp);

	    fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(");

	    if(cos->isDataMember())
	      {
		assert(!cos->isElementParameter());

		// allow for immediate quarks; not element parameters
		if(stgcosclasstype == UC_ELEMENT)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getBits()");
		    fp->write(", ");   //rest of args
		  }
		else if(stgcosclasstype == UC_QUARK)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getBits()");
		    fp->write(", ");   //rest of args
		  }
		else
		  {
		    //NOT A CLASS
		  }
	      }
	  }
      }

    //VALUE TO BE WRITTEN:
    if(isTerminal)
      {
	// write out terminal explicitly
	u32 data = ruvpass.getImmediateData(m_state);
	char dstr[40];
	rut->getDataAsString(data, dstr, 'z');
	fp->write(dstr);
	fp->write(");\n");
      }
    else
      {
	// with immediate quarks, they are read into a tmpreg as other immediates
	// with immediate elements, too!
	fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
      } //value not a terminal

    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteFromATmpVar

  void Node::genCodeWriteToSelfFromATmpVar(File * fp, UlamValue & luvpass, UlamValue & ruvpass)
  {
    assert(luvpass.getUlamValueTypeIdx() == Ptr);
    UTI ruti = ruvpass.getUlamValueTypeIdx();

    assert(ruti == Ptr);
    ruti = ruvpass.getPtrTargetType();

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

    m_state.indent(fp);
    fp->write(stgcos->getMangledName().c_str());
    fp->write(" = ");
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
    fp->write(";\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteToSelfFromATmpVar

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
    ULAMCLASSTYPE stgcosclasstype =  stgcosut->getUlamClass();

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

	if(cosut->isCustomArray())
	  fp->write("uc, ");

	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", ");  //rest of args
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAnElementParameter();
	if(epi >= 0)
	  {
	    m_state.indent(fp);

	    genElementParameterMemberNameOfMethod(fp, epi);

	    fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(");

	    if(!isHandlingImmediateType())
	      {
		genElementParameterHiddenArgs(fp, epi);
		fp->write(", ");  	//rest of arg's
	      }
	    else
	      {
		if(cosut->isCustomArray())
		  fp->write("uc, "); 	//rest of arg's
	      }
	  }
	else
	  {
	    //local
	    m_state.indent(fp);

	    genLocalMemberNameOfMethod(fp);

	    fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(");

	    if(cosut->isCustomArray())
	      fp->write("uc, ");

	    if(cos->isDataMember() && !cos->isElementParameter())
	      {
		// allow for immediate quarks; not element parameters
		if(stgcosclasstype == UC_ELEMENT)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getBits()");
		    fp->write(", ");   //rest of args
		  }
		else if(stgcosclasstype == UC_QUARK)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getBits()");
		    fp->write(", ");   //rest of args
		  }
		else
		  {
		    //NOT A CLASS
		  }
		// (what if array?)
	      }
	  }
      }

    //VALUE TO BE WRITTEN:
    if(isTerminal)
      {
	// write out terminal explicitly
	u32 data = ruvpass.getImmediateData(m_state);
	char dstr[40];
	rut->getDataAsString(data, dstr, 'z');
	fp->write(dstr);
	fp->write(");\n");
      }
    else
      {
	// with immediate quarks, they are read into a tmpreg as other immediates
	// with immediate elements, too!
	fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
      } //value not a terminal

    // rest of array write args..
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
    ULAMCLASSTYPE cosclasstype = cosut->getUlamClass();

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
	fp->write("(uc, ");

	fp->write(m_state.getHiddenArgName());
	fp->write(", ");  //rest of args
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAnElementParameter();
	if(epi >= 0)
	  {
	    m_state.indent(fp);

	    genElementParameterMemberNameOfMethod(fp, epi);

	    fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(");

	    //storage based on epi - 1
	    if(!isHandlingImmediateType())
	      {
		genElementParameterHiddenArgs(fp, epi);
		fp->write(", ");  	//rest of arg's
	      }
	    else
	      {
		if(cosut->isCustomArray())
		  fp->write("uc, "); 	//rest of arg's
	      }
	  }
	else
	  {
	    //local
	    m_state.indent(fp);

	    genLocalMemberNameOfMethod(fp);

	    fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(uc, ");

	    if(cos->isDataMember() && !cos->isElementParameter())
	      {
		// allow for immediate quarks; not element parameters
		if(stgcosclasstype == UC_ELEMENT)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getRef()");  //immediate EP needs the T storage within the struct
		    fp->write(", ");         //rest of args
		  }
		else if(stgcosclasstype == UC_QUARK)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(", ");         //rest of args
		  }
		else
		  {
		    //NOT A CLASS
		  }
		// (what if array?)
	      }
	  }
      }

    //index is immediate Int arg
    UlamType * intut = m_state.getUlamTypeByIndex(Int);
    fp->write(intut->getImmediateStorageTypeAsString().c_str()); //e.g. BitVector<32> exception
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write("), ");

    //VALUE TO BE WRITTEN:
    if(isTerminal)
      {
	// write out terminal explicitly
	u32 data = ruvpass.getImmediateData(m_state);
	char dstr[40];
	rut->getDataAsString(data, dstr, 'z');
	fp->write(dstr);
	fp->write(");\n");
      }
    else
      {
	// with immediate quarks, they are read into a tmpreg as other immediates
	// with immediate elements, too!
	// aset requires its custom array type (e.g. an atom) as its value:
	assert(cosclasstype != UC_NOTACLASS);
	UTI catype = ((UlamTypeClass *) cosut)->getCustomArrayType();
	fp->write(m_state.getUlamTypeByIndex(catype)->getImmediateStorageTypeAsString().c_str()); //e.g. BitVector<32> exception
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
	fp->write(") );\n");
      } //value not a terminal

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteCustomArrayItemFromATmpVar

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

    fp->write(vut->getImmediateStorageTypeAsString().c_str()); //e.g. BitVector<32> exception
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, TMPBITVAL).c_str());
    fp->write("("); // use constructor (not equals)
    if(isTerminal)
      {
	u32 data = uvpass.getImmediateData(m_state);
	char dstr[40];
	vut->getDataAsString(data, dstr, 'z');
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

    fp->write(vut->getTmpStorageTypeAsString().c_str()); //u32
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
	fp->write_decimal(adjustedImmediateArrayItemPtrPos(vuti, uvpass)); //item POS (last like others) ?
	fp->write("u");
      }
    fp->write(");\n");

    uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, vuti, m_state.determinePackable(vuti), m_state, 0);  //POS 0 rightjustified (atom-based).
    uvpass.setPtrPos(0); //entire register

    // specifically to sign extend Int's (a cast)
    vut->genCodeAfterReadingIntoATmpVar(fp, uvpass); //why was this commented out?
  } //genCodeConvertABitVectorIntoATmpVar

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


  //PROTECTED METHODS:
  Node * Node::makeCastingNode(Node * node, UTI tobeType)
  {
    bool doErrMsg = false;
    Node * rtnNode = NULL;
    UTI nuti = node->getNodeType();

    ULAMTYPECOMPARERESULTS uticr = UlamType::compare(nuti, tobeType, m_state);
    if(uticr == UTIC_DONTKNOW)
      {
	std::ostringstream msg;
	msg << "Casting 'incomplete' types: " << m_state.getUlamTypeNameByIndex(nuti).c_str() << "(UTI" << nuti << ") to be " << m_state.getUlamTypeNameByIndex(tobeType).c_str() << "(UTI" << tobeType << ") in class: " << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	if(m_state.getUlamTypeByIndex(m_state.getCompileThisIdx())->isComplete())
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	// continue on..
      }

    if(uticr == UTIC_SAME)
      {
	//happens too often with Bool.1.-1 for some reason; and Quark toInt special case
	// handle quietly
	return node;
      }

    ULAMCLASSTYPE nclasstype = m_state.getUlamTypeByIndex(nuti)->getUlamClass();
    if(nclasstype == UC_NOTACLASS)
      {
	if((nuti == UAtom) && (m_state.getUlamTypeByIndex(tobeType)->getUlamClass() != UC_ELEMENT))
	  doErrMsg = true;
	else if(nuti == Void)
	  doErrMsg = true;  //cannot cast a void into anything else (reverse is fine)
	else
	  {
	    rtnNode = new NodeCast(node, tobeType, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(getNodeLocation());
	    rtnNode->checkAndLabelType();
	  }
      }
    else if (nclasstype == UC_QUARK)
      {
	Token identTok;
	u32 castId = m_state.m_pool.getIndexForDataString("toInt");
	identTok.init(TOK_IDENTIFIER, getNodeLocation(), castId);
	if(m_state.getUlamTypeByIndex(tobeType)->getUlamTypeEnum() != Int)
	  doErrMsg = true;
	else
	  {
	    //fill in func symbol during type labeling;
	    Node * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
	    assert(fcallNode);
	    fcallNode->setNodeLocation(identTok.m_locator);
	    Node * mselectNode = new NodeMemberSelect(node, fcallNode, m_state);
	    assert(mselectNode);
	    mselectNode->setNodeLocation(identTok.m_locator);

	    //address the case of different byte sizes here
	    //before asserts start hitting later during assignment
	    //quarks are likely unknown size at checkandlabel time
	    //if(tobeType != nuti)
	    if(uticr != UTIC_SAME)
	      {
		rtnNode = new NodeCast(mselectNode, tobeType, m_state);
		assert(rtnNode);
		rtnNode->setNodeLocation(getNodeLocation());
	      }
	    else
	      rtnNode = mselectNode;  //replace right node with new branch

	    //redo check and type labeling
	    UTI newType = rtnNode->checkAndLabelType();
	    //if(newType != tobeType)
	    if(UlamType::compare(newType, tobeType, m_state) == UTIC_NOTSAME)
	      doErrMsg = true; //error msg instead
	  }
      }
    else if (nclasstype == UC_ELEMENT)
      {
	if(tobeType != UAtom)
	  doErrMsg = true;
	else
	  {
	    rtnNode = new NodeCast(node, tobeType, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(getNodeLocation());
	    rtnNode->checkAndLabelType();
	  }
      }
    else
      doErrMsg = true;

    if(doErrMsg)
      {
	std::ostringstream msg;
	msg << "Cannot CAST type: " << m_state.getUlamTypeNameByIndex(nuti).c_str() << " as a " << m_state.getUlamTypeNameByIndex(tobeType).c_str();
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
	msg << "Narrowing CAST, type: " << m_state.getUlamTypeNameByIndex(nodeType).c_str() << " to a " << m_state.getUlamTypeNameByIndex(tobeType).c_str() << " may cause data loss";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
      }
    return rtnB;
  } //warnOfNarrowingCast

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
    if(sut->isScalar() && sut->getUlamClass() == UC_QUARK && !sut->isCustomArray())
      {
	fp->write("Up_Us::");  //gives quark an atomicparameter type for write
      }
  } //genMemberNameOfMethod

  // "static" data member, a mixture of local variable and dm;
  // requires THE_INSTANCE, and local variables are superfluous.
  void Node::genElementParameterMemberNameOfMethod(File * fp, s32 epi)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    assert(epi >= 0);

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[epi];  //***

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    ULAMCLASSTYPE cosclasstype = cosut->getUlamClass();

    if(isHandlingImmediateType())
      {
	fp->write(cos->getMangledName().c_str());
	fp->write(".");
	return;
      }

    // the EP:
    if(cosclasstype == UC_NOTACLASS)  //atom too?
      {
	fp->write(cos->getMangledName().c_str());
	fp->write(".");
      }
    else
      {
	//now for both immmediate elements and quarks..
	fp->write(cosut->getImmediateStorageTypeAsString().c_str());
	fp->write("::");
	if( ((u32) (epi + 1) < cosSize))  //still another cos refiner, use
	  fp->write("Us::");      //typedef
      }

    u32 cosStart = epi+1;
    for(u32 i = cosStart; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	ULAMCLASSTYPE sclasstype = sut->getUlamClass();
	//not the element parameter, but a data member..
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
	// if its the last cos, a quark, and not a custom array...
	if(sclasstype == UC_QUARK && (i + 1 == cosSize) && sut->isScalar() && !sut->isCustomArray())
	  fp->write("Up_Us::");   //atomic parameter needed
      }
  } //genElementParameterMemberNameOfMethod

  // "static" data member, a mixture of local variable and dm;
  // requires THE_INSTANCE, and local variables are superfluous.
  void Node::genElementParameterHiddenArgs(File * fp, s32 epi)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    assert(epi >= 0);

    if(isHandlingImmediateType())
      return;  // no args

    Symbol * stgcos = NULL;
    if(epi == 0)
      stgcos = m_state.m_currentSelfSymbolForCodeGen;
    else
      stgcos = m_state.m_currentObjSymbolsForCodeGen[epi - 1];  //***

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    Symbol * epcos = m_state.m_currentObjSymbolsForCodeGen[epi];  //***
    UTI epcosuti = epcos->getUlamTypeIdx();
    UlamType * epcosut = m_state.getUlamTypeByIndex(epcosuti);
    ULAMCLASSTYPE epcosclasstype = epcosut->getUlamClass();

    if(cosut->isCustomArray())
      fp->write("uc, ");  //not for regular READs and WRITEs

    fp->write(stgcosut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::THE_INSTANCE");
    fp->write(".");

    // the EP (an element, quark, or primitive):
    fp->write(epcos->getMangledName().c_str());

    if(epcosclasstype != UC_NOTACLASS)
      {
	if(cosut->isCustomArray())
	  fp->write(".getRef()");
	else
	  fp->write(".getBits()");
      }
  } //genElementParameterHiddenArgs

  void Node::genLocalMemberNameOfMethod(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    // element parameter has its own storage, like a local
    assert(isCurrentObjectsContainingAnElementParameter() == -1);

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
    fp->write(ut->getImmediateStorageTypeAsString().c_str());
    fp->write("::");
    fp->write("Us::");   //typedef

    for(u32 i = 1; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	ULAMCLASSTYPE sclasstype = sut->getUlamClass();
	//not the element parameter, but a data member..
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
	// if its the last cos, a quark, and not a custom array...
	if(sclasstype == UC_QUARK && (i + 1 == cosSize) && sut->isScalar() && !sut->isCustomArray())
	  fp->write("Up_Us::");   //atomic parameter needed
      }
  } //genLocalMemberNameOfMethod

  const std::string Node::tmpStorageTypeForRead(UTI nuti, UlamValue uvpass)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etyp = nut->getUlamTypeEnum();

    //special case, u32/u64 desired before AfterReadingIntoATmpVar
    if(etyp == Int)
      return ((UlamTypeInt *) nut)->getUnsignedTmpStorageTypeAsString();

    return nut->getTmpStorageTypeAsString();
  } //tmpStorageTypeForRead

  const std::string Node::tmpStorageTypeForReadArrayItem(UTI nuti, UlamValue uvpass)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etyp = nut->getUlamTypeEnum();

    //special case, u32/u64 desired before AfterReadingIntoATmpVar
    if(etyp == Int)
      return ((UlamTypeInt *) nut)->getArrayItemUnsignedTmpStorageTypeAsString();

    return nut->getArrayItemTmpStorageTypeAsString();
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
    return !(m_state.m_currentObjSymbolsForCodeGen.empty() || (m_state.m_currentObjSymbolsForCodeGen[0]->isDataMember() && isCurrentObjectsContainingAnElementParameter() == -1));
  }

  // returns the index to the last object that's an EP; o.w. -1 none found;
  // preceeding object is the "owner", others before it are irrelevant;
  s32 Node::isCurrentObjectsContainingAnElementParameter()
  {
    s32 indexOfLastEP = -1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = cosSize - 1; i >= 0; i--)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(sym->isElementParameter())
	  {
	    indexOfLastEP = i;
	    break;
	  }
      }
    return indexOfLastEP;
  } //isCurrentObjectsContainingAnElementParameter

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
    // a local var that's not refining (i.e. cos[0]), or a local var that's an EP
    return (isCurrentObjectALocalVariableOrArgument() && ( (m_state.m_currentObjSymbolsForCodeGen.size() == 1) || (m_state.m_currentObjSymbolsForCodeGen.back()->isElementParameter())));
  }

  u32 Node::adjustedImmediateArrayItemPtrPos(UTI cosuti, UlamValue uvpass)
  {
    u32 pos = uvpass.getPtrPos();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    if(cosut->getUlamClass() == UC_NOTACLASS)
      {
	assert(cosuti != UAtom);  //atom too? let's find out..
	s32 wordsize = cosut->getTotalWordSize();
	pos = wordsize - (BITSPERATOM - pos); //cosut->getTotalBitSize();
      }
    return pos;
  }

} //end MFM
