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

  Node::Node(CompilerState & state): m_state(state), m_storeIntoAble(false), m_utype(Nouti), m_parentNo(0), m_no(m_state.getNextNodeNo()) {}

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

  void Node::checkAbstractInstanceErrors()
  {
    return; //default
  } //checkAbstractInstanceErrors

  void Node::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n",prettyNodeName().c_str());
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

  bool Node::checkSafeToCastTo(UTI fromType, UTI& newType)
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
	msg << " for '" << getName() << "'";
	if(scr == CAST_HAZY)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.setGoAgain();
	    newType = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
	rtnOK = false;
      } //not safe
    return rtnOK;
  } //checkSafeToCastTo

  // any node above assignexpr is not storeintoable;
  // and has no type (e.g. statements, statement, block, program)
  UTI Node::checkAndLabelType()
  {
    m_utype = Nouti;
    m_storeIntoAble = false;
    return m_utype;
  }

  void Node::checkForSymbol()
  {
    //for Nodes with Symbols
  }

  void Node::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    if(getNodeType() == Nav)
      {
	ncnt += 1;
	std::ostringstream msg;
	msg << "Erroneous ";
	msg << "'" << getName() << "'";
	msg << " (#" << ncnt << ")";
	//msg << " [" << prettyNodeName().c_str() << "] ";  //ugly!
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }
    else if(getNodeType() == Hzy)
      {
	hcnt += 1;
	std::ostringstream msg;
	msg << "Unresolved ";
	msg << "'" << getName() << "'";
	msg << " (#" << hcnt << ")";
	//msg << " [" << prettyNodeName().c_str() << "] ";  //ugly!
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }
    else if(getNodeType() == Nouti)
      {
	nocnt += 1;
	std::ostringstream msg;
	msg << "Untyped ";
	msg << "'" << getName() << "'";
	msg << " (#" << nocnt << ")";
	//msg << " [" << prettyNodeName().c_str() << "] ";  //ugly!
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
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
  } //countNavHzyNoutiNodes

  UTI Node::constantFold()
  {
    return Nav; //parent required
  }

  bool Node::buildDefaultQuarkValue(u32& dqref)
  {
    assert(0);
    return false;
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
    msg << "Invalid lefthand value (not storeIntoAble) '" << getName() << "'";
    msg << "; Cannot eval";
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
    m_state.m_nodeEvalStack.returnFrame(m_state);
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
    UlamValue tmpUV = UlamValue::makeImmediate(Nouti, 0, 1);
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

    assert((UlamType::compareForUlamValueAssignment(rtnUVtype, getNodeType(), m_state) == UTIC_SAME) || (m_state.getUlamTypeByIndex(rtnUVtype)->getUlamTypeEnum() == UAtom) || (m_state.getUlamTypeByIndex(getNodeType())->getUlamTypeEnum() == UAtom));

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
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI cosuti = cos->getUlamTypeIdx();

    // split off reading array items
    if(isCurrentObjectAnArrayItem(cosuti, uvpass))
      return genCodeReadArrayItemIntoATmpVar(fp, uvpass);

    // split if custom array, that requires an 'aref' function call
    // immediate types no longer have an array method to call for CA's.
    if(isCurrentObjectACustomArrayItem(cosuti, uvpass))
      return genCodeReadCustomArrayItemIntoATmpVar(fp, uvpass);

    // write out intermediate tmpVar (i.e. terminal) as temp BitVector arg
    // e.g. when func call is rhs of secondary member select
    if(uvpass.getPtrNameId() == 0)
      return genCodeConvertATmpVarIntoBitVector(fp, uvpass);

    m_state.indent(fp);
    fp->write("const ");
    fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, uvpass.getPtrStorage()).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	if(stgcos->isSelf() && (stgcos == cos))
	  {
	    genCodeReadSelfIntoATmpVar(fp, uvpass);
	  }
	else
	  {
	    genMemberNameOfMethod(fp);

	    // the READ method
	    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
	    fp->write("(");

	    // a data member quark, or the element itself should both GetBits from self
	    // now, quark's self is treated as the entire atom/element storage
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
	    if(stgcos->isSelf() && (stgcos == cos))
	      {
		genCodeReadSelfIntoATmpVar(fp, uvpass);
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
    // note: Ints not sign extended until used/cast
    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadIntoATmpVar

  void Node::genCodeReadIntoATmpVarUsingBitVector(File * fp, UlamValue & uvpass)
  {
    //note: arrays, custom arrays, autoref reads already split off
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
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI cosuti = cos->getUlamTypeIdx();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(tmpStorageTypeForRead(cosuti, uvpass).c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, uvpass.getPtrStorage()).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	if(stgcos->isSelf() && (stgcos == cos))
	  {
	    genCodeReadSelfIntoATmpVar(fp, uvpass);
	  }
	else
	  {
	    // a data member quark, or the element itself should both GetBits from self
	    // now, quark's self is treated as the entire atom/element storage
	    fp->write(m_state.getHiddenArgName());
	    fp->write(".GetBits().");

	    // the READ method
	    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
	    fp->write("(");

	    fp->write_decimal_unsigned(uvpass.getPtrPos()); //start
	    fp->write("u, ");
	    fp->write_decimal_unsigned(uvpass.getPtrLen()); //length
	    fp->write("u);\n");
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
	    if(stgcos->isSelf() && (stgcos == cos))
	      {
		genCodeReadSelfIntoATmpVar(fp, uvpass);
	      }
	    else
	      {
		// local quark or primitive (i.e. 'notaclass'); has an immediate type:
		// uses local variable name, and immediate read method
		u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
		if(cosSize > 1)
		  {
		    // storage-cos and cos-for-read are different:
		    // use this instance of storage-cos to specify
		    // its non-static read method
		    fp->write(stgcos->getMangledName().c_str());
		    fp->write(".getBits().");
		    //read method based on last cos
		    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
		    fp->write("(");
		    fp->write_decimal_unsigned(uvpass.getPtrPos()); //start
		    fp->write("u, ");
		    fp->write_decimal_unsigned(uvpass.getPtrLen()); //length
		    fp->write("u);\n");
		  }
		else
		  {
		    genLocalMemberNameOfMethod(fp);
		    fp->write(".");
		    //read method based on last cos
		    fp->write(readMethodForCodeGen(cosuti, uvpass).c_str());
		    fp->write("(true);\n"); //slow read
		  }
	      }
	  }
      }
    // note: Ints not sign extended until used/cast
    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadIntoATmpVarUsingBitVector

  void Node::genCodeReadSelfIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClass();
    if((stgclasstype == UC_QUARK) && stgcos->getId() == m_state.m_pool.getIndexForDataString("self"))
      {
	fp->write(stgcosut->getUlamTypeMangledName().c_str());
	fp->write("<EC, POS>::Up_Us::");
	fp->write(readMethodForCodeGen(stgcosuti, uvpass).c_str()); //or just 'Read' ?
	fp->write("(");
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits());\n"); //stand-alone 'self'
      }
    else
      {
	fp->write(m_state.getHiddenArgName());
	fp->write(";\n"); //stand-alone 'atom'
      }
  } //genCodeReadSelfIntoATmpVar

  void Node::genCodeReadAutorefIntoATmpVar(File * fp, UlamValue& uvpass)
  {
    //unlike the others, here, uvpass is the autoref (stg);
    //cos tell us where to go within the selected member
    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data

    UTI vuti = uvpass.getUlamValueTypeIdx();
    if(vuti == Ptr)
      vuti = uvpass.getPtrTargetType();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(tmpStorageTypeForRead(vuti, uvpass).c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2).c_str());
    fp->write(" = ");

    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, uvpass.getPtrStorage()).c_str());
    fp->write(".read();\n");

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, vuti, m_state.determinePackable(vuti), m_state, 0); //POS 0 justified (atom-based).

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadAutorefIntoATmpVar

  void Node::genCodeReadArrayItemIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data
    vuti = uvpass.getPtrTargetType(); //replaces vuti w target type
    assert(vuti != Void);
    assert(m_state.getUlamTypeByIndex(vuti)->isNumericType());

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI cosuti = cos->getUlamTypeIdx();
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

	// a data member quark, or the element itself should both GetBits from self
	// now, quark's hidden arg is treated as the entire atom/element storage
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", "); //rest of arg's
      }
    else //local var
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid

	genLocalMemberNameOfMethod(fp);

	//read method based on last cos
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

	if(stgcosclasstype == UC_ELEMENT)
	  {
	    fp->write("(");
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
	      fp->write("(");
	    else
	      {
		// storage-cos and cos-for-read are different:
		// use this instance of storage-cos to specify
		// its non-static read method
		fp->write("(");
		fp->write(stgcos->getMangledName().c_str());

		assert(stgcosclasstype == UC_QUARK);
		fp->write(".getBits()");
		fp->write(", ");
	      }
	  }
      }
    fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write(", ");
    fp->write_decimal_unsigned(m_state.getBitSize(cosuti)); //BITS_PER_ITEM
    fp->write("u");
    fp->write(");\n"); //done read array item

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, scalarcosuti, m_state.determinePackable(scalarcosuti), m_state, 0); //POS 0 rightjustified (atom-based).

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadArrayItemIntoTmp

  void Node::genCodeReadArrayItemIntoATmpVarUsingBitVector(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data
    vuti = uvpass.getPtrTargetType(); //replaces vuti w target type
    assert(vuti != Void);
    assert(m_state.getUlamTypeByIndex(vuti)->isNumericType());

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI cosuti = cos->getUlamTypeIdx();
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
	// a data member quark, or the element itself should both GetBits from self
	// now, quark's hidden arg is treated as the entire atom/element storage
	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
      }
    else //local var
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid

	if(stgcosclasstype == UC_ELEMENT)
	  {
	    fp->write("(");
	    fp->write(stgcos->getMangledName().c_str());
	    fp->write(".getBits()");
	  }
	else
	  {
	    // local quark or primitive (i.e. 'notaclass'); has an immediate type:
	    // uses local variable name, and immediate read method
	    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
	    if(cosSize == 1)
		genLocalMemberNameOfMethod(fp);
	    else
	      {
		// storage-cos and cos-for-read are different:
		// use this instance of storage-cos to specify
		// its non-static read method
		fp->write("(");
		fp->write(stgcos->getMangledName().c_str());

		assert(stgcosclasstype == UC_QUARK);
		fp->write(".getBits()");
	      }
	  }
      }

    //read method based on last cos
    fp->write(".");
    fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());

    //rest of args
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write(", ");
    fp->write_decimal_unsigned(m_state.getBitSize(cosuti)); //BITS_PER_ITEM
    fp->write("u");
    fp->write(");\n"); //done read array item

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarNum2, TMPREGISTER, scalarcosuti, m_state.determinePackable(scalarcosuti), m_state, 0); //POS 0 rightjustified (atom-based).
    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeReadArrayItemIntoTmpUsingBitVector

  void Node::genCodeReadCustomArrayItemIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();

    assert(vuti == Ptr); //terminals handled in NodeTerminal as BitVector for args

    s32 tmpVarNum2 = m_state.getNextTmpVarNumber(); //tmp for data
    vuti = uvpass.getPtrTargetType(); //replaces vuti w target type
    assert(vuti != Void);

    //vut (index) may not be numeric when custom array
    //here, cos is symbol used to determine read method: either self or last of cos.
    //stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      cos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      cos = m_state.m_currentObjSymbolsForCodeGen.back();

    UTI cosuti = cos->getUlamTypeIdx();

    assert(isCurrentObjectACustomArrayItem(cosuti, uvpass));

    UTI itemuti = m_state.getAClassCustomArrayType(cosuti);

    m_state.indent(fp);
    fp->write("const ");
    fp->write(localStorageTypeAsString(itemuti).c_str()); //e.g. BitVector<32> exception
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(itemuti, tmpVarNum2, TMPBITVAL).c_str());
    fp->write(" = ");

    // all the cases where = is used; else BitVector constructor for converting a tmpvar
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genCustomArrayMemberNameOfMethod(fp);
	// the READ method
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());
	genCustomArrayHiddenArgs(fp);
	fp->write(", "); //rest of arg's
      }
    else  //local var
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid
	//read method based on last cos
	genCustomArrayLocalMemberNameOfMethod(fp);
	fp->write(readArrayItemMethodForCodeGen(cosuti, uvpass).c_str());
	genCustomArrayHiddenArgs(fp);
	fp->write(", "); //rest of args
      }
    //index is immediate Index arg of targettype in uvpass
    fp->write(localStorageTypeAsString(vuti).c_str()); //e.g. BitVector<32> exception
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
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType();
    UlamType * lut = m_state.getUlamTypeByIndex(luti);

    UTI ruti = ruvpass.getUlamValueTypeIdx();
    assert(ruti == Ptr); //terminals handled in NodeTerminal
    ruti = ruvpass.getPtrTargetType();

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
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

    // split if writing an array item or custom array item, here
    if(isCurrentObjectAnArrayItem(cosuti, luvpass))
      return genCodeWriteArrayItemFromATmpVar(fp, luvpass, ruvpass);

    if(isCurrentObjectACustomArrayItem(cosuti, luvpass))
      return genCodeWriteCustomArrayItemFromATmpVar(fp, luvpass, ruvpass); //like a func call

    // split off autoref stg/member selected
    if(luvpass.getPtrStorage() == TMPAUTOREF)
      return genCodeWriteToAutorefFromATmpVar(fp, luvpass, ruvpass);

    if(stgcos->isSelf() && (stgcos == cos))
      return genCodeWriteToSelfFromATmpVar(fp, luvpass, ruvpass);

    bool isElementAncestorCast = (lut->getUlamClass() == UC_ELEMENT) && m_state.isClassASuperclassOf(ruti, luti);

    UlamValue typuvpass;
    if(isElementAncestorCast)
      {
	//readTypefield of lhs before the write!
	// pass as rhs uv to restore method afterward;
	// avoids making default atom.
	genCodeReadElementTypeField(fp, typuvpass);
      }

    m_state.indent(fp);

    // a data member quark, or the element itself should both GetBits from self;
    // now, quark's self is treated as the entire atom/element storage
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
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid

	//local
	genLocalMemberNameOfMethod(fp);

	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");

	if(cos->isDataMember())
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
	  }
      }

    //VALUE TO BE WRITTEN:
    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too! value is not a terminal
    STORAGE rstor = m_state.getUlamTypeByIndex(ruti)->getUlamClass() == UC_QUARK ? TMPREGISTER : ruvpass.getPtrStorage();
  fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), rstor).c_str());
    fp->write(");\n");

    // inheritance cast needs the lhs type restored after the generated write
    if(isElementAncestorCast)
      restoreElementTypeForAncestorCasting(fp, typuvpass);

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteFromATmpVar

  void Node::genCodeWriteFromATmpVarUsingBitVector(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    bool needsBVflag = false;
    UTI luti = luvpass.getUlamValueTypeIdx();
    AssertBool isPtrL = (luti == Ptr);
    assert(isPtrL);

    UTI ruti = ruvpass.getUlamValueTypeIdx();
    AssertBool isPtrR = (ruti == Ptr);
    assert(isPtrR); //terminals handled in NodeTerminal
    ruti = ruvpass.getPtrTargetType();

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
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

    m_state.indent(fp);

    // a data member quark, or the element itself should both GetBits from self;
    // now, quark's self is treated as the entire atom/element storage
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {

	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits().");
	//The WRITE method
	fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");
	fp->write_decimal_unsigned(luvpass.getPtrPos()); //start
	fp->write("u, ");
	fp->write_decimal_unsigned(luvpass.getPtrLen()); //length
	fp->write("u, "); //rest of args..
      }
    else
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid
	//local
	if(cos->isDataMember())
	  {
	    // allow for immediate quarks; not element parameters
	    if(stgcosclasstype == UC_ELEMENT)
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".getBits().");
	      }
	    else if(stgcosclasstype == UC_QUARK)
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".getBits().");
	      }
	    else
	      {
		//NOT A CLASS
	      }
	    //The WRITE method
	    fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("(");
	    fp->write_decimal_unsigned(luvpass.getPtrPos()); //start
	    fp->write("u, ");
	    fp->write_decimal_unsigned(luvpass.getPtrLen()); //length
	    fp->write("u, "); //rest of args..
	  }
	else
	  {
	    needsBVflag = true;
	    fp->write(cos->getMangledName().c_str());
	    fp->write(".");
	    //The WRITE method
	    fp->write(writeMethodForCodeGen(cosuti, luvpass).c_str());
	    fp->write("("); //rest of args..
	  }
      }

    //VALUE TO BE WRITTEN:
    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too! value is not a terminal
    STORAGE rstor = m_state.getUlamTypeByIndex(ruti)->getUlamClass() == UC_QUARK ? TMPREGISTER : ruvpass.getPtrStorage();
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), rstor).c_str());

    if(needsBVflag)
      fp->write(", true"); //local flag

    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteFromATmpVarUsingBitVector

  void Node::genCodeWriteToSelfFromATmpVar(File * fp, UlamValue & luvpass, UlamValue & ruvpass)
  {
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType();
    UlamType * lut = m_state.getUlamTypeByIndex(luti);

    UTI ruti = ruvpass.getUlamValueTypeIdx();
    assert(ruti == Ptr);
    ruti = ruvpass.getPtrTargetType();

    bool isElementAncestorCast = (lut->getUlamClass() == UC_ELEMENT) && m_state.isClassASuperclassOf(ruti, luti);

    UlamValue typuvpass;
    if(isElementAncestorCast)
      {
	//readTypefield of lhs before the write!
	// pass as rhs uv to restore method afterward;
	// avoids making default atom.
	genCodeReadElementTypeField(fp, typuvpass);
      }

    m_state.indent(fp);
    fp->write(m_state.getHiddenArgName());
    fp->write(" = ");
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
    fp->write(";\n");

    // inheritance cast needs the lhs type restored after the generated write
    if(isElementAncestorCast)
      restoreElementTypeForAncestorCasting(fp, typuvpass);

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteToSelfFromATmpVar

  void Node::genCodeWriteToAutorefFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    //unlike the others, here, uvpass is the autoref (stg);
    //cos tell us where to go within the selected member
    s32 tmpVarNum = luvpass.getPtrSlotIndex();

    m_state.indent(fp);
    fp->write(m_state.getTmpVarAsString(luvpass.getPtrTargetType(), tmpVarNum, TMPAUTOREF).c_str());
    fp->write(".write(");
    //VALUE TO BE WRITTEN:
    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too! value is not a terminal
    fp->write(m_state.getTmpVarAsString(ruvpass.getPtrTargetType(), ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
    fp->write(");\n");
    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteToAutorefFromATmpVar

  void Node::genCodeReadElementTypeField(File * fp, UlamValue & uvpass)
  {
    s32 tmpVarType = m_state.getNextTmpVarNumber();
    m_state.indent(fp);
    fp->write("const u32 ");
    fp->write(m_state.getTmpVarAsString(Unsigned, tmpVarType).c_str());;
    fp->write(" = ");

    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp);
	// the GetType WRITE method
	fp->write("ReadTypeField(");

	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
      }
    else
      {
	//local
	genLocalMemberNameOfMethod(fp);
	fp->write("readTypeField(");
      }
    fp->write("); //save type\n\n");

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarType, TMPREGISTER, Unsigned, m_state.determinePackable(Unsigned), m_state, 0); //POS 0 rightjustified (atom-based).
  } //genCodeReadElementTypeField

  void Node::restoreElementTypeForAncestorCasting(File * fp, UlamValue & uvpass)
  {
    // inheritance cast needs the lhs type restored after the generated write
    s32 tmpVarType = uvpass.getPtrSlotIndex();

    m_state.indent(fp);

    if(!isCurrentObjectALocalVariableOrArgument())
      {
	genMemberNameOfMethod(fp);

	// the GetType WRITE method
	fp->write("WriteTypeField(");

	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	fp->write(", "); //rest of args
      }
    else
      {
	//local
	genLocalMemberNameOfMethod(fp);
	fp->write("writeTypeField(");
      }

    fp->write(m_state.getTmpVarAsString(Unsigned, tmpVarType).c_str());;
    fp->write("); //restore type\n\n");
  } //restoreElementTypeForAncestorCasting

  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteArrayItemFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    assert(luvpass.getUlamValueTypeIdx() == Ptr);
    UTI luti = luvpass.getPtrTargetType();
    UTI ruti = ruvpass.getUlamValueTypeIdx();

    if(ruti == Ptr)
      ruti = ruvpass.getPtrTargetType();

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
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

    // a data member quark, or the element itself should both getBits from self;
    // now, quark's self is treated as the entire atom/element storage
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
	fp->write(", "); //rest of args
      }
    else
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid

	//local
	m_state.indent(fp);

	genLocalMemberNameOfMethod(fp);

	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	fp->write("(");

	if(cos->isDataMember())
	  {
	    // allow for immediate quarks; not model parameters
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
    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too! value not a terminal
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());

    // rest of array write args..
    fp->write(", ");
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write(", ");
    fp->write_decimal_unsigned(m_state.getBitSize(cosuti)); //BITS_PER_ITEM
    fp->write("u");
    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteArrayItemFromATmpVar

  void Node::genCodeWriteArrayItemFromATmpVarUsingBitVector(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    bool valueLastArg = false;

    assert(luvpass.getUlamValueTypeIdx() == Ptr);
    UTI luti = luvpass.getPtrTargetType();
    UTI ruti = ruvpass.getUlamValueTypeIdx();

    if(ruti == Ptr)
      ruti = ruvpass.getPtrTargetType();

    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine first "hidden" arg
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
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

    // a data member quark, or the element itself should both getBits from self;
    // now, quark's self is treated as the entire atom/element storage
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	m_state.indent(fp);

	fp->write(m_state.getHiddenArgName());
	fp->write(".GetBits()");
	valueLastArg = true;
      }
    else
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid
	//local
	m_state.indent(fp);

	if(cos->isDataMember())
	  {
	    valueLastArg = true;
	    // allow for immediate quarks; not model parameters
	    if(stgcosclasstype == UC_ELEMENT)
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".getBits()");
	      }
	    else if(stgcosclasstype == UC_QUARK)
	      {
		fp->write(stgcos->getMangledName().c_str());
		fp->write(".getBits()");
	      }
	    else
	      {
		//NOT A CLASS
	      }
	    // (what if array?)
	  }
	else
	  genLocalMemberNameOfMethod(fp);
      }

    fp->write(".");
    // the WRITE method
    fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
    fp->write("(");

    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too! value not a terminal
    if(!valueLastArg)
      {
	fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
	fp->write(", ");
      }

    // rest of array write args..
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write(", ");
    fp->write_decimal_unsigned(m_state.getBitSize(cosuti)); //BITS_PER_ITEM
    fp->write("u");

    if(valueLastArg)
      {
	fp->write(", ");
	fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
      }

    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteArrayItemFromATmpVarUsingBitVector

  // two arg's luvpass fine-tunes the current symbol in case of member selection;
  // ruvpass is the ptr to value to write
  void Node::genCodeWriteCustomArrayItemFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass)
  {
    assert(luvpass.getUlamValueTypeIdx() == Ptr);

    UTI luti = luvpass.getPtrTargetType();
    UTI ruti = ruvpass.getUlamValueTypeIdx();

    assert(ruti == Ptr); //terminals handled in NodeTerminal
    ruti = ruvpass.getPtrTargetType();

    //rhs could be a constant; or previously cast from Int to Unary variables.
    // here, cos is symbol used to determine read method: either self or last of cos.
    // stgcos is symbol used to determine "hidden" args
    Symbol * cos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      cos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      cos = m_state.m_currentObjSymbolsForCodeGen.back();

    UTI cosuti = cos->getUlamTypeIdx();
    assert(isCurrentObjectACustomArrayItem(cosuti, luvpass));

    // a data member quark, or the element itself should both getBits from self;
    // getbits needed to go from-atom to-BitVector
    if(!isCurrentObjectALocalVariableOrArgument())
      {
	m_state.indent(fp);

	genCustomArrayMemberNameOfMethod(fp);
	// the WRITE method
	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());
	genCustomArrayHiddenArgs(fp);
	fp->write(", "); //rest of args
      }
    else
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid
	//local
	m_state.indent(fp);

	genCustomArrayLocalMemberNameOfMethod(fp);

	fp->write(writeArrayItemMethodForCodeGen(cosuti, luvpass).c_str());

	genCustomArrayHiddenArgs(fp);
	fp->write(", "); //rest of args
      }

    //index is immediate Int arg
    fp->write(localStorageTypeAsString(luti).c_str()); //e.g. BitVector<32> exception
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(luti, luvpass.getPtrSlotIndex()).c_str()); //INDEX
    fp->write("), ");

    //VALUE TO BE WRITTEN:
    // with immediate quarks, they are read into a tmpreg as other immediates
    // with immediate elements, too!  value not a terminal
    // aset requires its custom array type (e.g. an atom) as its value:
    assert(m_state.getUlamTypeByIndex(cosuti)->getUlamTypeEnum() == Class);
    UTI catype = m_state.getAClassCustomArrayType(cosuti);
    fp->write(localStorageTypeAsString(catype).c_str()); //e.g. BitVector<32> exception
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(ruti, ruvpass.getPtrSlotIndex(), ruvpass.getPtrStorage()).c_str());
    fp->write(") );\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeWriteCustomArrayItemFromATmpVar

  // write out intermediate tmpVar as temp BitVector
  void Node::genCodeConvertATmpVarIntoBitVector(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();
    assert(vuti == Ptr); //terminals handled in NodeTerminal
    vuti = uvpass.getPtrTargetType();
    assert(m_state.okUTItoContinue(vuti));
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    // write out intermediate tmpVar, or immediate terminal, as temp BitVector arg
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");

    fp->write(localStorageTypeAsString(vuti).c_str()); //e.g. BitVector<32> exception
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, TMPBITVAL).c_str());
    fp->write("("); // use constructor (not equals)
    fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPtrSlotIndex(), uvpass.getPtrStorage()).c_str()); //VALUE

    u32 pos = uvpass.getPtrPos(); //pos calculated by makePtr(atom-based) (e.g. quark, atom)
    if(vut->getUlamClass() == UC_NOTACLASS)
      {
	//what if atom? pos == 0
	pos = BITSPERATOM - vut->getTotalBitSize(); //right-justified atom-based ?
      }

    if(m_state.isReference(vuti))
      {
	fp->write(", ");
	fp->write_decimal_unsigned(pos); //position for constructor
	fp->write("u");
      }
    fp->write(");\n");


    uvpass = UlamValue::makePtr(tmpVarNum2, TMPBITVAL, vuti, m_state.determinePackable(vuti), m_state, pos); //POS left-justified for quarks; right for primitives.

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeConvertATmpVarIntoBitVector

  // write out immediate tmp BitValue as an intermediate tmpVar
  void Node::genCodeConvertABitVectorIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();
    assert(vuti == Ptr);
    vuti = uvpass.getPtrTargetType();
    assert(m_state.okUTItoContinue(vuti));
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    assert(uvpass.getPtrStorage() == TMPBITVAL);

    // write out immediate tmp BitValue as an intermediate tmpVar
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
    STORAGE tmp2stor = (((vut->getUlamClass() == UC_ELEMENT) || (vut->getUlamTypeEnum() == UAtom)) ?  TMPBITVAL : TMPREGISTER);

    m_state.indent(fp);
    fp->write("const ");

    fp->write(vut->getTmpStorageTypeAsString().c_str()); //u32
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, tmp2stor).c_str());
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

    uvpass = UlamValue::makePtr(tmpVarNum2, tmp2stor, vuti, m_state.determinePackable(vuti), m_state, 0); //POS 0 rightjustified (atom-based).
    uvpass.setPtrPos(0); //entire register
  } //genCodeConvertABitVectorIntoATmpVar

  // write out auto ref tmpVar as temp BitVector; uvpass has variable w posOffset;
  // use stack for symbol? default is hidden arg
  void Node::genCodeConvertATmpVarIntoAutoRef(File * fp, UlamValue & uvpass)
  {
    UTI vuti = uvpass.getUlamValueTypeIdx();
    assert(vuti == Ptr);
    vuti = uvpass.getPtrTargetType(); //offset or another autoref

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(cosSize == 0) //empty
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else if(cosSize == 1)
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    // write out auto ref constuctor
    s32 tmpVarNum = uvpass.getPtrSlotIndex();
    s32 tmpVarNum2 = m_state.getNextTmpVarNumber();

    // write out next chain using auto ref constuctor
    if(uvpass.getPtrStorage() == TMPAUTOREF)
      {
	assert(m_state.isReference(vuti));
	m_state.indent(fp);
	//can't be const and chainable
	fp->write(cosut->getLocalStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum2, TMPAUTOREF).c_str());
	fp->write("("); //use constructor (not equals)
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, TMPAUTOREF).c_str());
	fp->write(", ");
	fp->write_decimal_unsigned(cos->getPosOffset());
	fp->write("u");
      }
    else
      {
	//first array item, with item in uvpass
	assert(cosut->getUlamTypeEnum() == Class);
	assert(!cosut->isScalar());

	cosuti = m_state.getUlamTypeAsScalar(cosuti);
	cosut = m_state.getUlamTypeByIndex(cosuti);

	m_state.indent(fp);
	//can't be const and chainable
	fp->write(cosut->getLocalStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(cosuti, tmpVarNum2, TMPAUTOREF).c_str());
	fp->write("("); // use constructor (not equals)

	if(cos->isDataMember())
	  {
	    if(cosSize > 1)
	      fp->write(stgcos->getMangledName().c_str());
	    else
	      fp->write(m_state.getHiddenArgName());
	  }
	else
	  {
	    fp->write(cos->getMangledName().c_str()); //local array
	    fp->write(".getRef()");
	  }

	fp->write(", (");
	fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum, uvpass.getPtrStorage()).c_str()); //pos variable 0-based
	fp->write(" * ");
	fp->write_decimal_unsigned(cosut->getBitSize());
	fp->write("u)");

	if(cosSize > 0)
	  {
	    fp->write(" + ");
	    fp->write_decimal_unsigned(cos->getPosOffset());
	    fp->write("u");
	  }
      }
    fp->write(");\n");

    uvpass = UlamValue::makePtr(tmpVarNum2, TMPAUTOREF, cosuti, m_state.determinePackable(cosuti), m_state, 0, cos->getId()); //POS left-justified by default.

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeConvertATmpVarIntoAutoRef

  void Node::genCodeARefFromARefStorage(File * fp, UlamValue stguvpass, UlamValue uvpass)
  {
    // write out auto ref constuctor
    s32 tmpVarNum = stguvpass.getPtrSlotIndex();
    s32 tmpVarNum2 = uvpass.getPtrSlotIndex();

    UTI vuti = uvpass.getPtrTargetType();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(m_state.isReference(vuti));

    UTI stgrefuti = stguvpass.getPtrTargetType();
    assert(m_state.isReference(stgrefuti));

    m_state.indent(fp);
    //can't be const and chainable
    fp->write(vut->getLocalStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(vuti, tmpVarNum2, TMPAUTOREF).c_str());
    fp->write("("); //use constructor (not equals)
    fp->write(m_state.getTmpVarAsString(stgrefuti, tmpVarNum, TMPAUTOREF).c_str());
    fp->write(".getRef(), ");
    fp->write(m_state.getTmpVarAsString(stgrefuti, tmpVarNum, TMPAUTOREF).c_str());
    fp->write(".getPosOffset() + ");
    fp->write_decimal_unsigned(uvpass.getPtrPos());
    fp->write("u");
    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear();
  } //genCodeARefFromARefStorage

  void Node::genCodeExtern(File * fp, bool declOnly)
  {
    // no externs
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

    if(nuti == Nav)
      {
	std::ostringstream msg;
	msg << "Cannot make casting node for an erronous type: " ;
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	msg << " (UTI" << nuti << ")";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnNode = node;
	return false; //short-circuit
      }

    if(nuti == Hzy)
      {
	std::ostringstream msg;
	msg << "Cannot make casting node for a nonready type: " ;
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	msg << " (UTI" << nuti << ")";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	rtnNode = node;
	return false; //short-circuit
      }

    ULAMTYPECOMPARERESULTS uticr = UlamType::compare(nuti, tobeType, m_state);
    if(uticr == UTIC_SAME)
      {
	//happens too often with Bool.1.-1 for some reason;
	//and Quark toInt special case -- handle quietly
	rtnNode = node;
	return true;
      }

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UlamType * tobe = m_state.getUlamTypeByIndex(tobeType);
    ULAMCLASSTYPE nclasstype = nut->getUlamClass();
    ULAMCLASSTYPE tclasstype = tobe->getUlamClass();

    if(nclasstype == UC_NOTACLASS)
      {
	if((nut->getUlamTypeEnum() == UAtom) && (tclasstype != UC_ELEMENT))
	  doErrMsg = true;
	else if(nuti == Void)
	  doErrMsg = true; //cannot cast a void into anything else (reverse is fine)
	//else if reference to non-ref of same type?
	else
	  {
	    assert(!isExplicit);
	    rtnNode = new NodeCast(node, tobeType, NULL, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(getNodeLocation());
	    rtnNode->updateLineage(getNodeNo());

	    //redo check and type labeling; error msg if not same
	    UTI newType = rtnNode->checkAndLabelType();
	    doErrMsg = (UlamType::compareForMakingCastingNode(newType, tobeType, m_state) == UTIC_NOTSAME);
	  }
      }
    else if(nclasstype == UC_QUARK)
      {
	if(node->isFunctionCall())
	  {
	    if(tobe->isReference())
	      {
		rtnNode = new NodeCast(node, tobeType, NULL, m_state);
		assert(rtnNode);
		rtnNode->setNodeLocation(getNodeLocation());
		rtnNode->updateLineage(getNodeNo());
	      }
	    else
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
	  }
	else if(tclasstype == UC_QUARK)
	  {
	    //handle possible inheritance (u.1.2.2) here
	    if(m_state.isClassASuperclassOf(nuti, tobeType))
	      {
		rtnNode = new NodeCast(node, tobeType, NULL, m_state);
		assert(rtnNode);
		rtnNode->setNodeLocation(getNodeLocation());
		rtnNode->updateLineage(getNodeNo());
	      }
	    else if((UlamType::compareForMakingCastingNode(nuti, tobeType, m_state) == UTIC_SAME))
	      {
		//cast arrayitems as their deref types
		rtnNode = new NodeCast(node, tobeType, NULL, m_state);
		assert(rtnNode);
		rtnNode->setNodeLocation(getNodeLocation());
		rtnNode->updateLineage(getNodeNo());
	      }
	    else if(m_state.isARefTypeOfUlamType(nuti, tobeType))
	      {
		//cast ref to deref type
		rtnNode = new NodeCast(node, tobeType, NULL, m_state);
		assert(rtnNode);
		rtnNode->setNodeLocation(getNodeLocation());
		rtnNode->updateLineage(getNodeNo());
	      }
	    else if(m_state.isARefTypeOfUlamType(tobeType, nuti))
	      {
		//cast non-ref to its ref type; constants & funccalls
		// not legal for initialization; ok for assignment.
		rtnNode = new NodeCast(node, tobeType, NULL, m_state);
		assert(rtnNode);
		rtnNode->setNodeLocation(getNodeLocation());
		rtnNode->updateLineage(getNodeNo());
	      }
	    else
	      doErrMsg = true;
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
	      doErrMsg = (UlamType::compareForMakingCastingNode(newType, tobeType, m_state) == UTIC_NOTSAME);
	      if(doErrMsg)
		return makeCastingNode(rtnNode, tobeType, rtnNode, false); //recurse
	    }
      }
    else if (nclasstype == UC_ELEMENT)
      {
	if((UlamType::compareForMakingCastingNode(nuti, tobeType, m_state) == UTIC_SAME))
	  {
	    rtnNode = new NodeCast(node, tobeType, NULL, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(getNodeLocation());
	    rtnNode->updateLineage(getNodeNo());

	    //redo check and type labeling; error msg if not same
	    UTI newType = rtnNode->checkAndLabelType();
	    doErrMsg = (UlamType::compareForMakingCastingNode(newType, tobeType, m_state) == UTIC_NOTSAME);
	  }
	else if((UlamType::compareForMakingCastingNode(tobeType, UAtom, m_state) != UTIC_SAME))
	  doErrMsg = true;
	else
	  {
	    rtnNode = new NodeCast(node, tobeType, NULL, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(getNodeLocation());
	    rtnNode->updateLineage(getNodeNo());

	    //redo check and type labeling; error msg if not same
	    UTI newType = rtnNode->checkAndLabelType();
	    doErrMsg = (UlamType::compareForMakingCastingNode(newType, tobeType, m_state) == UTIC_NOTSAME);
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
	    m_state.setGoAgain();
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

    //create "atom" symbol whose index is "hidden" first arg (i.e. a Ptr to an Atom);
    //immediately below the return value(s); and belongs to the function definition scope.
    u32 selfid = m_state.m_pool.getIndexForDataString("atom"); //was "self"
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
	fnSym = new SymbolFunctionName(funcidentTok, Nouti, m_state);

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

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    u32 startcos = 0;
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    Symbol * stgcos = m_state.getCurrentSelfSymbolForCodeGen();

    //what happens when a quark is both a DM as well as ancestor to CurrentSelfSymbol?
    if(cosSize > 1)
      stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    UTI stgcosuti = stgcos->getUlamTypeIdx(); //more general instead of current class
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    // when data member belongs to its (or a DM's) superclass, specify its class name first
    NNO cosBlockNo = cos->getBlockNoOfST();
    NNO stgcosBlockNo = stgcos->getBlockNoOfST(); //m_state.getAClassBlockNo(stgcosuti);

    if(cosBlockNo != stgcosBlockNo)
      {
	s32 subcos = isCurrentObjectsContainingASubClass();
	if(subcos >= 0)
	  {
	    startcos = subcos + 1;

	    UTI cosclassuti = cosuti;
	    UlamType * cosclassut = cosut;

	    if(cosut->getUlamTypeEnum() != Class)
	      {
		cosclassuti = findTypeOfAncestorAndBlockNo(cosBlockNo, subcos);
		assert(m_state.isComplete(cosclassuti) || m_state.isClassATemplate(cosclassuti));
		cosclassut = m_state.getUlamTypeByIndex(cosclassuti);
	      }
	    if(cosut->isReference())
	      {
		UTI cosclassrefuti = m_state.getUlamTypeAsRef(cosclassuti, cosut->getReferenceType());
		UlamType * cosclassrefut = m_state.getUlamTypeByIndex(cosclassrefuti);
		fp->write(cosclassrefut->getLocalStorageTypeAsString().c_str());
		fp->write("::Us::");
	      }
	    else
	      {
		fp->write(cosclassut->getUlamTypeMangledName().c_str());
		if(cosclassut->getUlamClass() == UC_ELEMENT)
		  fp->write("<EC>::");
		else
		  {
		    fp->write("<EC, ");
		    if(stgcos->isDataMember())
		      {
			fp->write_decimal_unsigned(stgcos->getPosOffset());
			fp->write("u + ");
		      }
		    fp->write("T::ATOM_FIRST_STATE_BIT");
		    fp->write(">::");
		  }
	      }
	  }
	else if(m_state.isClassASubclass(stgcosuti) != Nouti) //self is subclass
	  {
	    //could be in a super-super class
	    Node * foundnode = m_state.findNodeNoInAClass(cosBlockNo, stgcosuti);
	    assert(foundnode);
	    UTI superuti = foundnode->getNodeType();
	    UlamType * superut = m_state.getUlamTypeByIndex(superuti);

	    if(stgcosut->isReference())
	      {
		UTI superrefuti = m_state.getUlamTypeAsRef(superuti, stgcosut->getReferenceType());
		UlamType * superrefut = m_state.getUlamTypeByIndex(superrefuti);
		fp->write(superrefut->getLocalStorageTypeAsString().c_str());
		fp->write("::Us::");
	      }
	    else
	      {
		fp->write(superut->getUlamTypeMangledName().c_str());
		assert(superut->getUlamClass() != UC_ELEMENT);
		//super is a quark, stg might be either:
		fp->write("<EC, ");
		if(stgcosut->getUlamClass() == UC_ELEMENT)
		  fp->write("T::ATOM_FIRST_STATE_BIT>::");
		else
		  fp->write("POS>::"); //quarks know this
	      }
	  }
      }
    //else do nothing for inheritance

    //iterate over COS vector; empty if current object is self
    for(u32 i = startcos; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	if(sym->isSelf())
	  continue;
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
	//if(i == 0)
	//  fp->write("THE_INSTANCE."); not with parameter types!!!
      }

    //if last cos is a quark, for Read/Write to work it needs an
    // atomic Parameter type (i.e. Up_Us); not so for custom arrays
    // which are more like a function call
    // scalar quarks are typedefs and need atomic parametization;
    // arrays are already atomic parameters
    // ancestor issues up for grabs?
    bool isCA = cosut->isCustomArray();
    if(cosut->isScalar() && cosut->getUlamClass() == UC_QUARK)
      {
	if(!isCA)
	  fp->write("Up_Us::"); //gives quark an atomicparameter type for write
	else if(isCA)
	  fp->write("THE_INSTANCE.");
	else if(cos->isDataMember())
	  fp->write("THE_INSTANCE."); //Ut_..
      }
  } //genMemberNameOfMethod

  // "static" data member, a mixture of local variable and dm;
  // NEW approach as extern immediate; [XXXits mangled name includes the
  // element/quark mangled name; thus,XXX] unique MP per ulam class.
  // No longer a template data member; must be initialized (in .cpp),
  // so only primitive types allowed.
  void Node::genModelParameterMemberNameOfMethod(File * fp, s32 epi)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    assert(epi >= 0);

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen[epi]; //***

    // the MP (only primitive!, no longer quark or element):
    assert(isHandlingImmediateType());

    Symbol * stgcos = NULL;
    if(epi == 0)
      stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      stgcos = m_state.m_currentObjSymbolsForCodeGen[epi - 1]; //***

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    NNO cosBlockNo = cos->getBlockNoOfST();
    NNO stgcosBlockNo = m_state.getAClassBlockNo(stgcosuti);
    if((stgcosBlockNo != cosBlockNo) && (m_state.isClassASubclass(stgcosuti) != Nouti))
      {
	Node * foundnode = m_state.findNodeNoInAClass(cosBlockNo, stgcosuti);
	assert(foundnode);
	stgcosuti = foundnode->getNodeType(); //reset stgcosuti here!!
      }
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClass();

    if(stgclasstype == UC_ELEMENT)
      {
	if(stgcosut->isReference())
	  {
	    fp->write(stgcosut->getUlamTypeImmediateMangledName().c_str());
	    fp->write("<EC>::Us::");
	  }
	else
	  {
	    fp->write(stgcosut->getUlamTypeMangledName().c_str());
	    fp->write("<EC>::");
	  }
	    fp->write("THE_INSTANCE.");
      }
    else
      assert(0);

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
      stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      stgcos = m_state.m_currentObjSymbolsForCodeGen[epi - 1]; //***

    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();

    Symbol * epcos = m_state.m_currentObjSymbolsForCodeGen[epi]; //***
    UTI epcosuti = epcos->getUlamTypeIdx();
    UlamType * epcosut = m_state.getUlamTypeByIndex(epcosuti);
    ULAMCLASSTYPE epcosclasstype = epcosut->getUlamClass();

    if(m_state.isClassACustomArray(cosuti))
      fp->write("uc, "); //not for regular READs and WRITEs

    fp->write(stgcosut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::THE_INSTANCE");
    fp->write(".");

    // the MP (only primitive!, no longer quark or element):
    fp->write(epcos->getMangledName().c_str());

    if(epcosclasstype != UC_NOTACLASS)
      {
	if(m_state.isClassACustomArray(cosuti))
	  fp->write(".getRef()");
	else
	  fp->write(".getBits()");
      }
  } //genModelParameterHiddenArgs

  void Node::genCustomArrayMemberNameOfMethod(File * fp)
  {
    assert(!isCurrentObjectALocalVariableOrArgument());

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    //find class for cos' custom array method; and blockNo (may be inherited, unlike cos)
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    NNO cosBlockNo = cos->getBlockNoOfST();

    Symbol * fnsymptr = NULL;
    bool hazyKin = false;
    AssertBool isDefinedFunc = m_state.isFuncIdInAClassScope(cosuti, m_state.getCustomArrayGetFunctionNameId(),fnsymptr, hazyKin); //searches class of cos
    assert(isDefinedFunc);
    assert(!hazyKin);
    NNO caBlockNo = fnsymptr->getBlockNoOfST(); //block of aref
    UTI caclassuti = m_state.findAClassByNodeNo(caBlockNo);
    assert(m_state.isComplete(caclassuti) || m_state.isClassATemplate(caclassuti));

    //currently, only regular classes may have subclasses.
    if((cosBlockNo != caBlockNo) && (cosuti != caclassuti) && !m_state.isClassATemplate(caclassuti))
      {
	assert((m_state.isClassASubclass(cosuti) != Nouti) || (m_state.isARefTypeOfUlamType(cosuti, caclassuti) == UTIC_SAME));
	UlamType * caclassut = m_state.getUlamTypeByIndex(caclassuti);

	fp->write(caclassut->getUlamTypeMangledName().c_str());
	if(caclassut->getUlamClass() == UC_ELEMENT)
	  fp->write("<EC>::THE_INSTANCE.");
	else
	  {
	    fp->write("<EC,");
	    fp->write("T::ATOM_FIRST_STATE_BIT");
	    fp->write(">::");
	    fp->write("THE_INSTANCE."); //quarks need an object
	  }
	return;
      }
    //otherwise normal data member name..
    return genMemberNameOfMethod(fp);
  } //genCustomArrayMemberNameOfMethod

  void Node::genCustomArrayHiddenArgs(File * fp)
  {
    Symbol * cos = NULL;
    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      {
	stgcos = cos = m_state.getCurrentSelfSymbolForCodeGen();
      }
    else
      {
	cos = m_state.m_currentObjSymbolsForCodeGen.back();
	stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
      }
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMCLASSTYPE stgcosclasstype =  stgcosut->getUlamClass();

    fp->write("(");

    // first "hidden" arg is the context, then
    //"hidden" self (atom) arg
    if(!Node::isCurrentObjectALocalVariableOrArgument())
      {
	if(m_state.m_currentObjSymbolsForCodeGen.empty())
	  fp->write(m_state.getHiddenContextArgName()); //same uc
	else
	  {
	    //update uc to reflect "effective" self for this funccall
	    fp->write("UlamContext<EC>(uc, &");
	    fp->write(stgcosut->getUlamTypeMangledName().c_str());
	    fp->write("<EC");
	    if(stgcosclasstype == UC_QUARK)
	      {
		fp->write(", ");
		fp->write_decimal_unsigned(stgcos->getPosOffset());
		fp->write("u + T::ATOM_FIRST_STATE_BIT");
	      }
	    fp->write(">::THE_INSTANCE)");
	  }
	fp->write(", ");
	fp->write(m_state.getHiddenArgName()); //atom
      }
    else
      {
	assert(isCurrentObjectsContainingAModelParameter() == -1); //MP invalid
	//local var
	if(m_state.m_currentObjSymbolsForCodeGen.empty())
	  fp->write(m_state.getHiddenContextArgName()); //same uc
	else if(stgcos->getAutoLocalType() == ALT_AS)
	  {
	    fp->write(m_state.getAutoHiddenContextArgName()); //uc_
	    fp->write(stgcos->getMangledName().c_str());
	  }
	else
	  {
	    //use possible dereference type for mangled name
	    UTI derefuti = m_state.getUlamTypeAsDeref(stgcosuti);
	    UlamType * derefut = m_state.getUlamTypeByIndex(derefuti);

	    //update uc to reflect "effective" self for this funccall
	    fp->write("UlamContext<EC>(uc, &");
	    fp->write(derefut->getUlamTypeMangledName().c_str());
	    fp->write("<EC");
	    if(stgcosclasstype == UC_QUARK)
	      {
		fp->write(", ");
		if(cos->isDataMember()) //dm of local stgcos
		      {
			fp->write_decimal_unsigned(stgcos->getPosOffset());
			fp->write("u + ");
		      }
		fp->write("T::ATOM_FIRST_STATE_BIT");
	      }
	    fp->write(">::THE_INSTANCE)");
	  }
	fp->write(", ");
	fp->write(stgcos->getMangledName().c_str());
	// for both immediate quarks and elements now..not self.
	if(!stgcos->isSelf())
	  fp->write(".getRef()"); //the T storage within the struct for immediate quarks
      }
    return;
  } //genCustomArrayHiddenArgs

  void Node::genLocalMemberNameOfMethod(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    // model parameter has its own storage, like a local
    assert(isCurrentObjectsContainingAModelParameter() == -1);

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    if(cosSize > 1)
      return genLocalMemberNameOfMethodByUsTypedef(fp);

    assert(cosSize == 1);
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    fp->write(stgcos->getMangledName().c_str());
    fp->write(".");
    return;
  } //genLocalMemberNameOfMethod

  void Node::genLocalMemberNameOfMethodByUsTypedef(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());
    // model parameter has its own storage, like a local
    assert(isCurrentObjectsContainingAModelParameter() == -1);
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    u32 startcos = 1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    // handle inheritance, when data member is in superclass, not current class obj
    // now for both immediate elements and quarks..
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    NNO cosBlockNo = cos->getBlockNoOfST();
    NNO stgcosBlockNo = m_state.getAClassBlockNo(stgcosuti);
    s32 subcos = -1;

    // when cos and stgcos are different, check inheritance
    //if(stgcosBlockNo != cosBlockNo)
    if((cosSize > 1) && (stgcosBlockNo != cosBlockNo))
      {
	subcos = isCurrentObjectsContainingASubClass();
	if(subcos >= 0)
	  {
	    startcos = subcos + 1; //for loop later

	    UTI cosclassuti = cosuti;
	    UlamType * cosclassut = cosut;

	    if(cosut->getUlamTypeEnum() != Class)
	      {
		cosclassuti = findTypeOfAncestorAndBlockNo(cosBlockNo, subcos);
		assert(m_state.isComplete(cosclassuti) || m_state.isClassATemplate(cosclassuti));
		cosclassut = m_state.getUlamTypeByIndex(cosclassuti);
	      }

	    if(stgcosut->isReference())
	      {
		fp->write(cosclassut->getLocalStorageTypeAsString().c_str());
		fp->write("::");
		if(cosclassut->getUlamClass() == UC_QUARK)
		  fp->write("Us::"); //missing
	      }
	    else
	      {
		//note: can't use immediate name since we have multiple dots
		// (i.e. data members) at specific POS (e.g. t3606)
		fp->write(cosclassut->getUlamTypeMangledName().c_str());
		if(cosclassut->getUlamClass() == UC_ELEMENT)
		  fp->write("<EC>::");
		else
		  {
		    fp->write("<EC, ");
		    u32 posoff = 0;
		    for(u32 i = subcos; i > 0; i--)
		      {
			Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
			UTI suti = sym->getUlamTypeIdx();
			UlamType * sut = m_state.getUlamTypeByIndex(suti);
			if(sym->isDataMember() && sut->getUlamTypeEnum() == Class)
			  posoff += sym->getPosOffset();
		      }

		    if(posoff > 0)
		      {
			fp->write_decimal_unsigned(posoff);
			fp->write("u + ");
		      }

		    fp->write("T::ATOM_FIRST_STATE_BIT");
		    fp->write(">::");
		  } //a quark
	      } //not reference
	  } //sub class

      } //blocks same

    //stgcos is not the base of the type (possibly remove code? No, t3249)
    if(subcos < 0)
      {
	fp->write(localStorageTypeAsString(stgcosuti).c_str());
	fp->write("::");
	fp->write("Us::"); //typedef
	if(cosSize == 1)
	  fp->write("THE_INSTANCE."); //only for elements, except w funccalls
      }

    for(u32 i = startcos; i < cosSize; i++)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	UTI suti = sym->getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	//not the model parameter, but a data member..
	fp->write(sym->getMangledNameForParameterType().c_str());
	fp->write("::");
	// if its the last cos, a quark, and scalar
	if((i + 1 == cosSize) && sut->isScalar() && (sut->getUlamClass() == UC_QUARK))
	  {
	    bool isCA = m_state.isClassACustomArray(suti);
	    if(!isCA)
	      {
		//not a custom array...
		fp->write("Up_Us::"); //atomic parameter needed
	      }
	    else if(isCA)
	      fp->write("THE_INSTANCE."); //custom array
	    else if(sym->isDataMember())
	      fp->write("THE_INSTANCE."); //Ut_Um...
	  }
      }
      } //genLocalMemberNameOfMethodByUsTypedef

  void Node::genCustomArrayLocalMemberNameOfMethod(File * fp)
  {
    assert(isCurrentObjectALocalVariableOrArgument());

    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());

    // handle inheritance, when data member is in superclass, not current class obj
    // now for both immediate elements and quarks..
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    NNO cosBlockNo = cos->getBlockNoOfST();

    Symbol * fnsymptr = NULL;
    bool hazyKin = false;
    AssertBool isDefinedFunc = m_state.isFuncIdInAClassScope(cosuti, m_state.getCustomArrayGetFunctionNameId(),fnsymptr, hazyKin); //searches class of cos
    assert(isDefinedFunc);
    assert(!hazyKin);
    NNO caBlockNo = fnsymptr->getBlockNoOfST(); //block of aref
    UTI caclassuti = m_state.findAClassByNodeNo(caBlockNo);
    assert(m_state.isComplete(caclassuti) || m_state.isClassATemplate(caclassuti));

    //currently, only regular classes may have subclasses.
    if((cosBlockNo != caBlockNo) && (cosuti != caclassuti) && !m_state.isClassATemplate(caclassuti))
      {
	assert((m_state.isClassASubclass(cosuti) != Nouti) || (m_state.isARefTypeOfUlamType(cosuti, caclassuti) == UTIC_SAME));
	UlamType * caclassut = m_state.getUlamTypeByIndex(caclassuti);

	fp->write(caclassut->getUlamTypeMangledName().c_str());
	if(caclassut->getUlamClass() == UC_ELEMENT)
	  fp->write("<EC>::THE_INSTANCE.");
	else
	  {
	    fp->write("<EC, ");
	    fp->write("T::ATOM_FIRST_STATE_BIT");
	    fp->write(">::");
	    fp->write("THE_INSTANCE."); //quarks need an object
	  }
	return;
      }

    //otherwise normal data member name..
    return genLocalMemberNameOfMethodByUsTypedef(fp);
  } //genCustomArrayLocalMemberNameOfMethod

  const std::string Node::localStorageTypeAsString(UTI nuti)
  {
    return m_state.getUlamTypeByIndex(nuti)->getLocalStorageTypeAsString();
  } //localStorageTypeAsString

  const std::string Node::tmpStorageTypeForRead(UTI nuti, UlamValue uvpass)
  {
    return m_state.getUlamTypeByIndex(nuti)->getTmpStorageTypeAsString();
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
    bool isCArray = isCurrentObjectACustomArrayItem(nuti, uvpass);
    assert(isCurrentObjectAnArrayItem(nuti, uvpass) || isCArray);

    if(isHandlingImmediateType() && !isCArray)
      method = readMethodForImmediateBitValueForCodeGen(nuti, uvpass);
    else if(isCArray)
      method = m_state.getCustomArrayGetMangledFunctionName();
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
    bool isCArray = isCurrentObjectACustomArrayItem(nuti, uvpass);
    assert(isCurrentObjectAnArrayItem(nuti, uvpass) || isCArray);

    if(isHandlingImmediateType() && !isCArray)
      method = writeMethodForImmediateBitValueForCodeGen(nuti, uvpass);
    else if(isCArray)
      method = m_state.getCustomArraySetMangledFunctionName();
    else
      method = m_state.getUlamTypeByIndex(nuti)->writeArrayItemMethodForCodeGen();
    return method;
  } //writeArrayItemMethodForCodeGen

  //each knows which Read method to use based on their key(arraysize, bitsize, type);
  const std::string Node::readMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass)
  {
    if(isCurrentObjectAnArrayItem(nuti, uvpass))
	return "readArrayItem"; //piece

    assert(!isCurrentObjectACustomArrayItem(nuti, uvpass));

    //note: for the entire array, ie not just a piece, we want to use 'read'
    //      rather then readArrayItem; unless we're talking UNPACKED.
    //      For Int's, there is no automatic sign extension for entire arrays.
    return "read";
  } //readMethodForImmediateBitValueForCodeGen

  const std::string Node::writeMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass)
  {
    if(isCurrentObjectAnArrayItem(nuti, uvpass))
      return "writeArrayItem";

    assert(!isCurrentObjectACustomArrayItem(nuti, uvpass));

    return "write";
  } //writeMethodForImmediateBitValueForCodeGen

  bool Node::isCurrentObjectALocalVariableOrArgument()
  {
    //include model parameters as LocalVariableOrArgument, since more alike;
    //an element's "self" as obj[0] is like it isn't there for purposes of this discovery.
    //quark's self is an atom, and should be treated like a local arg.
    // note: self is not a data member.
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      return false; //must be self, t.f. not local

    s32 modelparamidx = isCurrentObjectsContainingAModelParameter();
    if(m_state.m_currentObjSymbolsForCodeGen[0]->isDataMember() && (modelparamidx == -1))
      return false; //data member, not model parameter

    UTI stgcosuti = m_state.m_currentObjSymbolsForCodeGen[0]->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    if(m_state.m_currentObjSymbolsForCodeGen[0]->isSelf() && (stgcosut->getUlamTypeEnum() != UAtom) && (modelparamidx == -1))
      return false; //self, not atom, not modelparameter
    return true;
  } //isCurrentObjectALocalVariableOrArgument

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

  // returns the index to the last object that's a subclass; o.w. -1 none found;
  // preceeding object is the "owner", others before it are irrelevant;
  s32 Node::isCurrentObjectsContainingASubClass()
  {
    s32 indexOfLastSubClass = -1;
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    for(s32 i = cosSize - 1; i >= 0; i--)
      {
	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
	UTI suti = sym->getUlamTypeIdx();
	if(m_state.isClassASubclass(suti) != Nouti)
	  {
	    indexOfLastSubClass = i;
	    break;
	  }
      }
    return indexOfLastSubClass;
  } //isCurrentObjectsContainingASubClass

  UTI Node::findTypeOfAncestorAndBlockNo(NNO bno, s32 subcosidx)
  {
    Symbol * subcos = NULL;
    if(subcosidx < 0)
      subcos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      subcos = m_state.m_currentObjSymbolsForCodeGen[subcosidx];
    UTI subcosuti = subcos->getUlamTypeIdx();

    //compare blockclassuti with cosnameuti (in case of templates)
    UTI blockclassuti = m_state.findAClassByNodeNo(bno); //regular or template
    assert(m_state.isComplete(blockclassuti) || m_state.isClassATemplate(blockclassuti));

    UTI cosclassuti = m_state.getUlamTypeAsScalar(subcosuti); //init as scalar
    do{
      SymbolClassName * coscnsym = NULL;
      UlamType * cosclassut = m_state.getUlamTypeByIndex(cosclassuti);
      u32 cosid = cosclassut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId();
      AssertBool isDefined = m_state.alreadyDefinedSymbolClassName(cosid, coscnsym);
      assert(isDefined);
      UTI cosnameuti = coscnsym->getUlamTypeIdx();

      if(blockclassuti == cosnameuti) break;

      subcosuti = cosclassuti;
      cosclassuti = m_state.isClassASubclass(subcosuti); //returns superuti, an instance!

      if(cosclassuti == Nouti) //no superclass
	return subcosuti;

    } while(cosclassuti != Nouti); //while superclass

    return cosclassuti;
  } //findTypeOfAncestorAndBlockNo

  // returns the index to the last object that's a subclass; o.w. -1 none found;
  // preceeding object is the "owner"
  s32 Node::calcPosOfCurrentObjectsContainingASubClass(bool isLocal)
  {
    //assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    //self must be subclass, i.e. inherited quark always at pos 0
    if(m_state.m_currentObjSymbolsForCodeGen.empty()) return 0;

    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    s32 cosSize = (s32) m_state.m_currentObjSymbolsForCodeGen.size();
    s32 subcos = isCurrentObjectsContainingASubClass();
    if(subcos < 0)
      subcos = cosSize - 1; //e.g. custom array is in subclass, not cos

    ULAMCLASSTYPE ct = m_state.getUlamTypeByIndex(stgcos->getUlamTypeIdx())->getUlamClass();
    u32 pos = ((ct == UC_ELEMENT || !isLocal) ? ATOMFIRSTSTATEBITPOS : 0) ;

    for(s32 i = 0; i <= subcos; i++)
      {
    	Symbol * sym = m_state.m_currentObjSymbolsForCodeGen[i];
    	pos += sym->getPosOffset();
      }
    return pos;
  } //calcPosOfCurrentObjectsContainingASubClass

  //false means its the entire array or not an array at all (use read() if PACKEDLOADABLE)
  bool Node::isCurrentObjectAnArrayItem(UTI cosuti, UlamValue uvpass)
  {
    //uvpass would be an array index (an int of sorts), not an array;
    //types would not be the same;
    return(!m_state.isScalar(cosuti) && m_state.isScalar(uvpass.getPtrTargetType()));
  } //isCurrentObjectAnArrayItem

  bool Node::isCurrentObjectACustomArrayItem(UTI cosuti, UlamValue uvpass)
  {
    // a cosuti as a scalar, customarray, may be used as a regular array,
    //     but at this point cosuti would be a scalar in either case (sigh);
    // uvpass would be an array index (an int of sorts), not an array;
    // types would not be the same;
    return(m_state.isScalar(cosuti) && m_state.isClassACustomArray(cosuti) && uvpass.getPtrTargetType() != cosuti);
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
	assert(cosut->getUlamTypeEnum() != UAtom); //atom too? let's find out..
	u32 wordsize = cosut->getTotalWordSize();
	pos = wordsize - (BITSPERATOM - pos); //cosut->getTotalBitSize();
      }
    return pos;
  } //adjustedImmediateArrayItemPtrPos

} //end MFM
