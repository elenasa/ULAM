#include <iostream>
#include "Node.h"
#include "CompilerState.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeFunctionCall.h"
#include "NodeIdent.h"
#include "NodeMemberSelect.h"
#include "NodeVarDecl.h"
#include "SymbolVariableStack.h"
#include "SymbolFunction.h"
#include "SymbolFunctionName.h"

#include "UlamTypeInt.h"

namespace MFM {

  Node::Node(CompilerState & state): m_state(state), m_storeIntoAble(false), m_utype(Nav), m_parentNo(0), m_no(m_state.getNextNodeNo()) {}

  Node::Node(const Node & ref) : m_state(ref.m_state), m_storeIntoAble(ref.m_storeIntoAble), m_utype(ref.m_utype), m_loc(ref.m_loc), m_parentNo(ref.m_parentNo), m_no(ref.m_no) /* same NNO */ {}

  void Node::setYourParentNo(NNO pno)
  {
    m_parentNo = pno;
  }

  NNO Node::getYourParentNo()
  {
    return m_parentNo;
  }

  void Node::updateLineage(NNO pno)
  {
    setYourParentNo(pno); //walk the tree..a leaf.
  }

  NNO Node::getNodeNo()
  {
    return m_no;
  }

  void Node::resetNodeNo(NNO no)
  {
    m_no = no;
  }

  bool Node::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    return false; //default
  } //exhangeKids

  bool Node::findNodeNo(NNO n, Node *& foundNode)
  {
    if(m_no == n) //leaf
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
    return m_utype;
  }

  void Node::setNodeType(UTI ut)
  {
    m_utype = ut;
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
    return m_loc;
  }

  void Node::setNodeLocation(Locator loc)
  {
    m_loc = loc;
  }

  void Node::printNodeLocation(File * fp)
  {
    fp->write(getNodeLocationAsString().c_str());
  }

  std::string Node::getNodeLocationAsString()
  {
    return m_state.getFullLocationAsString(m_loc);
  }

  bool Node::getSymbolPtr(Symbol *& symptrref)
  {
    return false;
  }

  void Node::constantFoldAToken(Token tok)
  {
    assert(0); //only NodeTerminal has this defined; NodeConstant bypasses
  }

  bool Node::isAConstant()
  {
    return false;
  }

  bool Node::isReadyConstant()
  {
    return false;
  }

  FORECAST Node::safeToCastTo(UTI newType)
  {
    std::ostringstream msg;
    msg << "virtual FORECAST " << prettyNodeName().c_str();
    msg << "::safeToCastTo(UTI newType){} is needed!!";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
    return CAST_HAZY;
  } //safeToCastTo

  bool Node::checkSafeToCastTo(UTI fromType, UTI newType)
  {
    bool rtnOK = true;
    UlamType * tobe = m_state.getUlamTypeByIndex(newType);
    FORECAST scr = tobe->safeCast(fromType);
    if(scr != CAST_CLEAR)
      {
	std::ostringstream msg;
	if(tobe->getUlamTypeEnum() == Bool)
	  msg << "Use a comparison operator";
	else
	  msg << "Use explicit cast";
	msg << " to convert "; // the real converting-message
	msg << m_state.getUlamTypeNameBriefByIndex(fromType).c_str();
	msg << " to ";
	msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
	msg << " for ";
	msg << getName();
	if(scr == CAST_HAZY)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnOK = false;
      } //not safe
    return rtnOK;
  } //checkSafeToCastTo

  // any node above assignexpr is not storeintoable;
  // and has no type (e.g. statements, statement, block, program)
  UTI Node::checkAndLabelType()
  {
    m_utype = Nav;
    m_storeIntoAble = false;
    return m_utype;
  }

  void Node::checkForSymbol()
  {
    //for Nodes with Symbols
  }

  void Node::countNavNodes(u32& cnt)
  {
    if(getNodeType() == Nav)
      {
	cnt += 1;
	std::ostringstream msg;
	msg << "Unresolved No." << cnt;
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
#if 0
    //debugg only
    if(m_loc.getLineNo() == 0)
      {
	std::ostringstream msg;
	msg << prettyNodeName().c_str() << " has NO LOC!!";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
#endif
  }

  UTI Node::constantFold()
  {
    return Nav;
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

  bool Node::isFunctionCall()
  {
    return false;
  }

  bool Node::installSymbolTypedef(TypeArgs& args, Symbol *& asymptr)
  {
    return false;
  }

  bool Node::installSymbolConstantValue(TypeArgs& args, Symbol*& asymptr)
  {
    return false;
  }

  bool Node::installSymbolParameterValue(TypeArgs& args, Symbol*& asymptr)
  {
    return false;
  }

  bool Node::installSymbolVariable(TypeArgs& args, Symbol *& asymptr)
  {
    return false;
  }

  bool Node::assignClassArgValueInStubCopy()
  {
    return true; //nothing to do
  }

  // any node above assignexpr is not storeintoable
  EvalStatus Node::evalToStoreInto()
  {
    std::ostringstream msg;
    msg << "Invalid lefthand value (not storeIntoAble): '" << getName() << "'";
    msg << ". Cannot eval";
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
    makeRoomForSlots(slots, where); //=1 for scalar or packed array
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

    if(rtnUVtype == Ptr) //unpacked array
      {
	rtnUVtype = rtnUV.getPtrTargetType();
      }

    if(rtnUVtype == Void) //check after Ptr target type
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

  void Node::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return; //work done by NodeStatements and NodeBlock
  }

  void Node::genCode(File * fp, UlamValue& uvpass)
  {
    m_state.indent(fp);
    fp->write("virtual void ");
    fp->write(prettyNodeName().c_str());
    fp->write("::genCode(File * fp){} is needed!!\n"); //sweet.
  }

  void Node::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    std::ostringstream msg;
    msg << "genCodeToStoreInto called on Node type: ";
    msg << m_state.getUlamTypeNameBriefByIndex(getNodeType()).c_str() << ", and failed.";
    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
    assert(0);
    return;
  } //genCodeToStoreInto

  void Node::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    vuti = uvpass.getPtrTargetType(); //replaces vuti w target type
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
    //UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

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

    //NOPE!!! after read does sign extend for ints, etc.
    fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, uvpass.getPtrStorage()).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp);

	if(stgcos->isSelf())
	  {
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(";\n");
	  }
	else
	  {
	    // the READ method
	    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
	    fp->write("(");

	    // a data member quark, or the element itself should both getBits from self
	    fp->write(m_state.getHiddenArgName());
	    fp->write(".GetBits()");
	    fp->write(");\n");
	  }
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    genModelParameterMemberNameOfMethod(fp, epi);

	    //read method based on last cos
	    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());

	    fp->write("(");

	    //storage based on epi-1
	    if(!isHandlingImmediateType())
	      {
		genModelParameterHiddenArgs(fp, epi);
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
    //    cosut->genCodeAfterReadingIntoATmpVar(fp, uvpass);

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadIntoTmp

  void Node::genCodeReadArrayItemIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data
    vuti = uvpass.getPtrTargetType(); //replaces vuti w target type
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
    // nor a local var that's an MP); o.w. immediate types have an array method
    // to call even for CA's.
    if(cosut->isCustomArray() && !isHandlingImmediateType())
      return genCodeReadCustomArrayItemIntoATmpVar(fp, uvpass);

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype = stgcosut->getUlamClass();
    UTI	scalarcosuti = m_state.getUlamTypeAsScalar(cosuti);

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

	if(cosut->isCustomArray()) //immediate CA
	  fp->write("uc, ");

	// a data member quark, or the element itself should both getBits from self
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", "); //rest of arg's
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    genModelParameterMemberNameOfMethod(fp, epi);

	    //read method based on last cos
	    fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	    //element parameter (could be array?)
	    fp->write("(");

	    if(!isHandlingImmediateType())
	      {
		genModelParameterHiddenArgs(fp, epi);
		fp->write(", "); //rest of arg's
	      }
	    else
	      {
		if(cosut->isCustomArray())
		  fp->write("uc, "); //rest of arg's
	      }
	  }
	else //local var
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
    fp->write(");\n"); //done read element parameter

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, scalarcosuti, m_state.determinePackable(scalarcosuti), m_state, 0); //POS 0 rightjustified (atom-based).

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadArrayItemIntoTmp

  void Node::genCodeReadCustomArrayItemIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data
    vuti = uvpass.getPtrTargetType(); //replaces vuti w target type
    assert(vuti != Void);

    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMTYPE vetype = vut->getUlamTypeEnum();
    assert( vetype == Int || vetype == Unsigned);

    //here, cos is symbol used to determine read method: either self or last of cos.
    //stgcos is symbol used to determine first "hidden" arg
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

	fp->write(m_state.getHiddenArgName()); //no getBits
	fp->write(", "); //rest of arg's
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    genModelParameterMemberNameOfMethod(fp, epi);

	    //read method based on last cos
	    fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	    fp->write("(");

	    //storage based on epi - 1
	    if(!isHandlingImmediateType())
	      {
		genModelParameterHiddenArgs(fp, epi);
		fp->write(", "); //rest of args
	      }
	    else
	      {
		if(cosut->isCustomArray())
		  fp->write("uc, "); //rest of arg's
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
		fp->write(".getRef()"); //immediate MP needs the T storage within the struct
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
    uvpass = UlamValue::makePtr(tmpVarNum2, TMPBITVAL, itemuti, m_state.determinePackable(itemuti), m_state, 0); //POS 0 rightjustified (atom-based).

    genCodeConvertABitVectorIntoATmpVar(fp, uvpass); //updates uvpass again

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
	fp->write(", "); //rest of args
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    assert(0); //shouldn't write into MP
	    genModelParameterMemberNameOfMethod(fp, epi);

	    fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(");

	    //storage based on epi - 1
	    if(!isHandlingImmediateType())
	      {
		genModelParameterHiddenArgs(fp, epi);
		fp->write(", "); //rest of arg's
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
		assert(!cos->isModelParameter());

		// allow for immediate quarks; not element parameters
		if(stgcosclasstype == UC_ELEMENT)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getBits()");
		    fp->write(", "); //rest of args
		  }
		else if(stgcosclasstype == UC_QUARK)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getBits()");
		    fp->write(", "); //rest of args
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
       s32 len = m_state.getBitSize(ruti);
       assert(len != UNKNOWNSIZE);
       if(len <= MAXBITSPERINT)
	 {
	   u32 data = ruvpass.getImmediateData(m_state);
	   char dstr[40];
	   rut->getDataAsString(data, dstr, 'z');
	   fp->write(dstr);
	 }
       else if(len <= MAXBITSPERLONG)
	 {
	   u64 data = ruvpass.getImmediateDataLong(m_state);
	   char dstr[70];
	   rut->getDataLongAsString(data, dstr, 'z');
	   fp->write(dstr);
	 }
       else
	 assert(0);
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
      return genCodeWriteCustomArrayItemFromATmpVar(fp, luvpass, ruvpass); //like a func call

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
	fp->write(", "); //rest of args
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    m_state.indent(fp);

	    genModelParameterMemberNameOfMethod(fp, epi);

	    fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(");

	    if(!isHandlingImmediateType())
	      {
		genModelParameterHiddenArgs(fp, epi);
		fp->write(", "); //rest of arg's
	      }
	    else
	      {
		if(cosut->isCustomArray())
		  fp->write("uc, "); //rest of arg's
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

	    if(cos->isDataMember() && !cos->isModelParameter())
	      {
		// allow for immediate quarks; not element parameters
		if(stgcosclasstype == UC_ELEMENT)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getBits()");
		    fp->write(", "); //rest of args
		  }
		else if(stgcosclasstype == UC_QUARK)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getBits()");
		    fp->write(", "); //rest of args
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
	s32 len = m_state.getBitSize(ruti);
	assert(len != UNKNOWNSIZE);
	if(len <= MAXBITSPERINT)
	  {
	    u32 data = ruvpass.getImmediateData(m_state);
	    char dstr[40];
	    rut->getDataAsString(data, dstr, 'z');
	    fp->write(dstr);
	  }
	else if(len <= MAXBITSPERLONG)
	  {
	    u64 data = ruvpass.getImmediateDataLong(m_state);
	    char dstr[70];
	    rut->getDataLongAsString(data, dstr, 'z');
	    fp->write(dstr);
	  }
	else
	  assert(0);

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
	fp->write(", "); //rest of args
      }
    else
      {
	s32 epi = isCurrentObjectsContainingAModelParameter();
	if(epi >= 0)
	  {
	    m_state.indent(fp);

	    genModelParameterMemberNameOfMethod(fp, epi);

	    fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(");

	    //storage based on epi - 1
	    if(!isHandlingImmediateType())
	      {
		genModelParameterHiddenArgs(fp, epi);
		fp->write(", "); //rest of arg's
	      }
	    else
	      {
		if(cosut->isCustomArray())
		  fp->write("uc, "); //rest of arg's
	      }
	  }
	else
	  {
	    //local
	    m_state.indent(fp);

	    genLocalMemberNameOfMethod(fp);

	    fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(uc, ");

	    if(cos->isDataMember() && !cos->isModelParameter())
	      {
		// allow for immediate quarks; not element parameters
		if(stgcosclasstype == UC_ELEMENT)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getRef()"); //immediate MP needs the T storage within the struct
		    fp->write(", "); //rest of args
		  }
		else if(stgcosclasstype == UC_QUARK)
		  {
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(", "); //rest of args
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
	s32 len = m_state.getBitSize(ruti);
	assert(len != UNKNOWNSIZE);
	if(len <= MAXBITSPERINT)
	  {
	    u32 data = ruvpass.getImmediateData(m_state);
	    char dstr[40];
	    rut->getDataAsString(data, dstr, 'z');
	    fp->write(dstr);
	  }
	else if(len <= MAXBITSPERLONG)
	  {
	    u64 data = ruvpass.getImmediateDataLong(m_state);
	    char dstr[70];
	    rut->getDataLongAsString(data, dstr, 'z');
	    fp->write(dstr);
	  }
	else
	  assert(0);
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
	// write out terminal explicitly
	s32 len = m_state.getBitSize(vuti);
	assert(len != UNKNOWNSIZE);
	if(len <= MAXBITSPERINT)
	  {
	    u32 data = uvpass.getImmediateData(m_state);
	    char dstr[40];
	    vut->getDataAsString(data, dstr, 'z');
	    fp->write(dstr);
	  }
	else if(len <= MAXBITSPERLONG)
	  {
	    u64 data = uvpass.getImmediateDataLong(m_state);
	    char dstr[70];
	    vut->getDataLongAsString(data, dstr, 'z');
	    fp->write(dstr);
	  }
	else
	  assert(0);
      }
    else
      {
	fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex()).c_str());
      }
    fp->write(");\n");

    u32 pos = 0; //pos calculated by makePtr(atom-based) (e.g. quark, atom)
    if(vut->getUlamClass() == UC_NOTACLASS)
      {
	u32 wordsize = vut->getTotalWordSize();
	pos = wordsize - vut->getTotalBitSize();
      }

    uvpass = UlamValue::makePtr(tmpVarNum2, TMPBITVAL, vuti, m_state.determinePackable(vuti), m_state, pos); //POS rightjustified.

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

    uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, vuti, m_state.determinePackable(vuti), m_state, 0); //POS 0 rightjustified (atom-based).
    uvpass.setPtrPos(0); //entire register

  } //genCodeConvertABitVectorIntoATmpVar

  void Node::genCodeExtern(File * fp, bool declOnly)
  {
    //e.g. NodeParameterDefs
  }

  void Node::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    assert(0); //fufilled by NodeVarDecl, NodeBlock; bypassed by NodeTypedef and NodeConstDef
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


  //PROTECTED METHODS:
  bool Node::makeCastingNode(Node * node, UTI tobeType, Node*& rtnNode, bool isExplicit)
  {
    bool doErrMsg = false;
    UTI nuti = node->getNodeType();

    ULAMTYPECOMPARERESULTS uticr = UlamType::compare(nuti, tobeType, m_state);
    if(uticr == UTIC_SAME)
      {
	//happens too often with Bool.1.-1 for some reason;
	//and Quark toInt special case -- handle quietly
	rtnNode = node;
	return true;
      }

    ULAMCLASSTYPE nclasstype = m_state.getUlamTypeByIndex(nuti)->getUlamClass();
    if(nclasstype == UC_NOTACLASS)
      {
	if((nuti == UAtom) && (m_state.getUlamTypeByIndex(tobeType)->getUlamClass() != UC_ELEMENT))
	  doErrMsg = true;
	else if(nuti == Void)
	  doErrMsg = true; //cannot cast a void into anything else (reverse is fine)
	else
	  {
	    assert(!isExplicit);
	    rtnNode = new NodeCast(node, tobeType, NULL, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(getNodeLocation());
	    rtnNode->updateLineage(getNodeNo());

	    //redo check and type labeling; error msg if not same
	    UTI newType = rtnNode->checkAndLabelType();
	    doErrMsg = (UlamType::compare(newType, tobeType, m_state) == UTIC_NOTSAME);
	  }
      }
    else if(nclasstype == UC_QUARK)
      {
	if(node->isFunctionCall())
	  {
	    // a function call is not a valid lhs !!!
	    // 'node' is a function call that returns a quark (it's not storeintoable);
	    // build a toIntHelper function that takes the return value of 'node'
	    // as its arg and returns toInt
	    NodeFunctionCall * castFunc = buildCastingFunctionCallNode(node, tobeType);
	    if(!castFunc)
	      doErrMsg = true;
	    else
	      rtnNode = castFunc;
	  }
	  else
	  {
	    Token identTok;
	    u32 castId = m_state.m_pool.getIndexForDataString("toInt");
	    identTok.init(TOK_IDENTIFIER, getNodeLocation(), castId);

	    //fill in func symbol during type labeling;
	    Node * fcallNode = new NodeFunctionCall(identTok, NULL, m_state);
	    assert(fcallNode);
	    fcallNode->setNodeLocation(identTok.m_locator);
	    Node * mselectNode = new NodeMemberSelect(node, fcallNode, m_state);
	    assert(mselectNode);
	    mselectNode->setNodeLocation(identTok.m_locator);
	    rtnNode = mselectNode; //replace right node with new branch
	  }

	  //redo check and type labeling; error msg if not same
	  if(!doErrMsg)
	    {
	      UTI newType = rtnNode->checkAndLabelType();
	      doErrMsg = (UlamType::compare(newType, tobeType, m_state) == UTIC_NOTSAME);
	      if(doErrMsg)
		return makeCastingNode(rtnNode, tobeType, rtnNode, false); //recurse
	    }
      }
    else if (nclasstype == UC_ELEMENT)
      {
	if(tobeType != UAtom)
	  doErrMsg = true;
	else
	  {
	    rtnNode = new NodeCast(node, tobeType, NULL, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(getNodeLocation());
	    rtnNode->updateLineage(getNodeNo());

	    //redo check and type labeling; error msg if not same
	    UTI newType = rtnNode->checkAndLabelType();
	    doErrMsg = (UlamType::compare(newType, tobeType, m_state) == UTIC_NOTSAME);
	  }
      }
    else
      doErrMsg = true;

    if(doErrMsg)
      {
	if(uticr == UTIC_DONTKNOW)
	  {
	    std::ostringstream msg;
	    msg << "Casting 'incomplete' types: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << "(UTI" << nuti << ") to be ";
	    msg << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    msg << "(UTI" << tobeType << ") in class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Cannot CAST " << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " as " << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    return !doErrMsg;
  } //makecastingnode

  NodeFunctionCall * Node::buildCastingFunctionCallNode(Node * node, UTI tobeType)
  {
    Locator loc = getNodeLocation(); //used throughout
    u32 castId = m_state.m_pool.getIndexForDataString("_toIntHelper");
    Token funcidentTok(TOK_IDENTIFIER, loc, castId);

    NodeBlockClass * currClassBlock = m_state.getClassBlock();

    //make the function def, with node (quark) type as its param, returns Int (always)
    SymbolFunction * fsymptr = new SymbolFunction(funcidentTok, Int /*tobeType*/, m_state);
    //No NodeTypeDescriptor needed for Int
    NodeBlockFunctionDefinition * fblock = new NodeBlockFunctionDefinition(fsymptr, currClassBlock, NULL, m_state);
    assert(fblock);
    fblock->setNodeLocation(loc);

    //symbol will have pointer to body (or just decl for 'use');
    fsymptr->setFunctionNode(fblock); //tfr ownership

    m_state.pushCurrentBlock(fblock); //before parsing the args

    //create "self" symbol whose index is "hidden" first arg (i.e. a Ptr to an Atom);
    //immediately below the return value(s); and belongs to the function definition scope.
    u32 selfid = m_state.m_pool.getIndexForDataString("self");
    Token selfTok(TOK_IDENTIFIER, loc, selfid);

    //negative indicates parameter for symbol install
    m_state.m_currentFunctionBlockDeclSize = -2; //slots for return + 1;

    m_state.m_currentFunctionBlockMaxDepth = 0;

    SymbolVariableStack * selfsym = new SymbolVariableStack(selfTok, UAtom, UNPACKED, m_state.m_currentFunctionBlockDeclSize, m_state);
    selfsym->setIsSelf();
    m_state.addSymbolToCurrentScope(selfsym); //ownership goes to the fblock

    UTI nodeType = node->getNodeType(); //quark type..
    UlamType * nut = m_state.getUlamTypeByIndex(nodeType);
    assert(nut->getUlamClass() == UC_QUARK);
    u32 quid = nut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId();
    Token typeTok(TOK_TYPE_IDENTIFIER, loc, quid);

    TypeArgs typeargs;
    typeargs.init(typeTok);
    typeargs.m_bitsize = nut->getBitSize();
    typeargs.m_arraysize = nut->getArraySize();
    typeargs.m_classInstanceIdx = nodeType;

    u32 argid = m_state.m_pool.getIndexForDataString("arg");
    Token argTok(TOK_IDENTIFIER, loc, argid);
    NodeIdent * argIdentNode = new NodeIdent(argTok, NULL, m_state);
    assert(argIdentNode);
    argIdentNode->setNodeLocation(loc);

    //symbol not a decl list, nor typedef from another class
    Symbol * argSym = NULL; //a place to put the new symbol;
    argIdentNode->installSymbolVariable(typeargs, argSym);
    assert(argSym);

    //NodeTypedescriptor for 'node' type?
    Node * argNode = new NodeVarDecl((SymbolVariable*) argSym, NULL, m_state);
    assert(argNode);
    argNode->setNodeLocation(loc);
    fsymptr->addParameterSymbol(argSym); //ownership stays w NodeBlockFunctionDefinition's ST

    //potentially needed to resolve its node type
    fblock->addParameterNode(argNode); //transfer owner

    //Now, look specifically for a function with the same given name defined
    Symbol * fnSym = NULL;
    if(!currClassBlock->isFuncIdInScope(funcidentTok.m_dataindex, fnSym))
      {
	//first time name used as a function..add symbol function name/typeNav
	fnSym = new SymbolFunctionName(funcidentTok, Nav, m_state);

	//ownership goes to the class block's ST
	currClassBlock->addFuncIdToScope(fnSym->getId(), fnSym);
      }

    bool isAdded = ((SymbolFunctionName *) fnSym)->overloadFunction(fsymptr); //tfrs owner
    if(!isAdded)
      {
	//this is a duplicate function definition with same parameters and given name!!
	//return types may differ
	std::ostringstream msg;
	msg << "Duplicate defined function '";
	msg << m_state.m_pool.getDataAsString(fsymptr->getId());
	msg << "' with the same parameters" ;
	MSG(&argTok, msg.str().c_str(), DEBUG);
	delete fsymptr; //also deletes the NodeBlockFunctionDefinition
	fsymptr = NULL;
	delete argIdentNode;
	argIdentNode = NULL;
      }
    else
      {
	//starts with positive one for local variables
	m_state.m_currentFunctionBlockDeclSize = 1;
	m_state.m_currentFunctionBlockMaxDepth = 0;

	/* like a typical quark toInt cast expression */
	u32 tointId = m_state.m_pool.getIndexForDataString("toInt");
	Token castidentTok(TOK_IDENTIFIER, loc, tointId);

	//fill in func symbol during type labeling;
	Node * fcallNode = new NodeFunctionCall(castidentTok, NULL, m_state);
	assert(fcallNode);
	fcallNode->setNodeLocation(loc);
	Node * mselectNode = new NodeMemberSelect(argIdentNode, fcallNode, m_state);
	assert(mselectNode);
	mselectNode->setNodeLocation(loc);

	//the body: single statement return arg.toInt();
	NodeReturnStatement * returnNode =  new NodeReturnStatement(mselectNode, m_state);
	assert(returnNode);
	returnNode->setNodeLocation(loc);

	NodeStatements * sNode = new NodeStatements(returnNode, m_state);
	assert(sNode);
	sNode->setNodeLocation(loc);

	fblock->setNextNode(sNode);
	fblock->setDefinition();
	fblock->setMaxDepth(0); //no local variables, except params
      }

    //this block's ST is no longer in scope
    m_state.popClassContext(); //= prevBlock;

    m_state.m_currentFunctionBlockDeclSize = 0; //default zero for datamembers
    m_state.m_currentFunctionBlockMaxDepth = 0; //reset

    //func call symbol to return to NodeCast; fsymptr maybe null
    NodeFunctionCall * rtnNode = new NodeFunctionCall(funcidentTok, fsymptr, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(loc);
    rtnNode->addArgument(node);

    return rtnNode;
  } //buildCastingFunctionCallNode

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
	msg << "Narrowing CAST: " << m_state.getUlamTypeNameBriefByIndex(nodeType).c_str();
	msg << " to " << m_state.getUlamTypeNameBriefByIndex(tobeType).c_str();
	msg << " may cause data loss";
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
	if(sym->isSelf())
	  continue;
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
	fp->write("Up_Us::"); //gives quark an atomicparameter type for write
      }
  } //genMemberNameOfMethod

  // "static" data member, a mixture of local variable and dm;
  // NEW approach as extern immediate; its mangled name includes the
  // element/quark mangled name; thus, unique MP per ulam class.
  // No longer a template data member; must be initialized (in .cpp),
  // so only primitive types allowed.
  void Node::genModelParameterMemberNameOfMethod(File * fp, s32 epi)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    assert(epi >= 0);

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[epi]; //***

    // the MP (only primitive!, no longer quark or element):
    assert(isHandlingImmediateType());

    fp->write(cos->getMangledName().c_str());
    fp->write(".");
    return;
  } //genModelParameterMemberNameOfMethod

  // "static" data member, a mixture of local variable and dm;
  // requires THE_INSTANCE, and local variables are superfluous.
  void Node::genModelParameterHiddenArgs(File * fp, s32 epi)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    assert(epi >= 0);

    if(isHandlingImmediateType())
      return; // no args

    Symbol * stgcos = NULL;
    if(epi == 0)
      stgcos = m_state.m_currentSelfSymbolForCodeGen;
    else
      stgcos = m_state.m_currentObjSymbolsForCodeGen[epi - 1]; //***

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    Symbol * epcos = m_state.m_currentObjSymbolsForCodeGen[epi]; //***
    UTI epcosuti = epcos->getUlamTypeIdx();
    UlamType * epcosut = m_state.getUlamTypeByIndex(epcosuti);
    ULAMCLASSTYPE epcosclasstype = epcosut->getUlamClass();

    if(cosut->isCustomArray())
      fp->write("uc, "); //not for regular READs and WRITEs

    fp->write(stgcosut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::THE_INSTANCE");
    fp->write(".");

    // the MP (only primitive!, no longer quark or element):
    fp->write(epcos->getMangledName().c_str());

    if(epcosclasstype != UC_NOTACLASS)
      {
	if(cosut->isCustomArray())
	  fp->write(".getRef()");
	else
	  fp->write(".getBits()");
      }
  } //genModelParameterHiddenArgs

  void Node::genLocalMemberNameOfMethod(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    // model parameter has its own storage, like a local
    assert(isCurrentObjectsContainingAModelParameter() == -1);

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
    fp->write("Us::"); //typedef

    for(u32 i = 1; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	ULAMCLASSTYPE sclasstype = sut->getUlamClass();
	//not the model parameter, but a data member..
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
	// if its the last cos, a quark, and not a custom array...
	if(sclasstype == UC_QUARK && (i + 1 == cosSize) && sut->isScalar() && !sut->isCustomArray())
	  fp->write("Up_Us::"); //atomic parameter needed
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
    return m_state.getUlamTypeByIndex(nuti)->getArrayItemTmpStorageTypeAsString();
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
    // include model parameters as LocalVariableOrArgument, since more alike XXX
    // an element's "self" as obj[0] is like it isn't there for purposes of this discovery.
    // quark's self is an atom, and should be treated like a local arg.
    // note: self is not a data member.
    return !(m_state.m_currentObjSymbolsForCodeGen.empty() || (m_state.m_currentObjSymbolsForCodeGen[0]->isDataMember() && isCurrentObjectsContainingAModelParameter() == -1) || (m_state.m_currentObjSymbolsForCodeGen[0]->isSelf() && m_state.m_currentObjSymbolsForCodeGen[0]->getUlamTypeIdx() != UAtom && isCurrentObjectsContainingAModelParameter() == -1));
  }

  // returns the index to the last object that's an MP; o.w. -1 none found;
  // preceeding object is the "owner", others before it are irrelevant;
  s32 Node::isCurrentObjectsContainingAModelParameter()
  {
    s32 indexOfLastMP = -1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = cosSize - 1; i >= 0; i--)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(sym->isModelParameter())
	  {
	    indexOfLastMP = i;
	    break;
	  }
      }
    return indexOfLastMP;
  } //isCurrentObjectsContainingAModelgParameter

  //false means its the entire array or not an array at all (use read() if PACKEDLOADABLE)
  bool Node::isCurrentObjectAnArrayItem(UTI cosuti, UlamValue uvpass)
  {
    //uvpass would be an array index (an int of sorts), not an array;
    //types would not be the same;
    return(!m_state.isScalar(cosuti) && m_state.isScalar(uvpass.getPtrTargetType()));
  } //isCurrentObjectAnArrayItem

  bool Node::isCurrentObjectACustomArrayItem(UTI cosuti, UlamValue uvpass)
  {
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    // a cosuti as a scalar, customarray, may be used as a regular array,
    //     but at this point cosuti would be a scalar in either case (sigh);
    // uvpass would be an array index (an int of sorts), not an array;
    // types would not be the same;
    return(m_state.isScalar(cosuti) && cosut->isCustomArray() && uvpass.getPtrTargetType() != cosuti);
  } //isCurrentObjectACustomArrayItem

  bool Node::isHandlingImmediateType()
  {
    // a local var that's not refining (i.e. cos[0]), or a var that's an MP
    return (isCurrentObjectALocalVariableOrArgument() && ( (m_state.m_currentObjSymbolsForCodeGen.size() == 1) || (m_state.m_currentObjSymbolsForCodeGen.back()->isModelParameter())));
  } //isHandlingImmediateType

  u32 Node::adjustedImmediateArrayItemPtrPos(UTI cosuti, UlamValue uvpass)
  {
    u32 pos = uvpass.getPtrPos();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    if(cosut->getUlamClass() == UC_NOTACLASS)
      {
	assert(cosuti != UAtom); //atom too? let's find out..
	u32 wordsize = cosut->getTotalWordSize();
	pos = wordsize - (BITSPERATOM - pos); //cosut->getTotalBitSize();
      }
    return pos;
  } //adjustedImmediateArrayItemPtrPos

} //end MFM
