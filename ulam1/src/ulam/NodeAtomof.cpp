#include <stdio.h>
#include "NodeAtomof.h"
#include "CompilerState.h"
#include "SymbolVariableStack.h"

namespace MFM {

  NodeAtomof::NodeAtomof(Token tokatomof, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_nodeTypeDesc(nodetype), m_token(tokatomof), m_atomoftype(Nouti), m_varSymbol(NULL), m_currBlockNo(0)
  {
    Node::setNodeLocation(tokatomof.m_locator);
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeAtomof::NodeAtomof(const NodeAtomof& ref) : Node(ref), m_nodeTypeDesc(NULL), m_token(ref.m_token), m_atomoftype(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_atomoftype)), m_varSymbol(NULL), m_currBlockNo(ref.m_currBlockNo)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeAtomof::~NodeAtomof()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  Node * NodeAtomof::instantiate()
  {
    return new NodeAtomof(*this);
  }

  void NodeAtomof::updateLineage(NNO pno)
  {
    Node::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeAtomof::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeAtomof::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);
  }

  void NodeAtomof::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(m_state.getTokenDataAsString(&m_token).c_str());
    fp->write(getName());
  }

  const char * NodeAtomof::getName()
  {
    return ".atomof";
  }

  const std::string NodeAtomof::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeAtomof::safeToCastTo(UTI newType)
  {
    return (m_state.isAtom(newType) ? CAST_CLEAR : CAST_BAD);
  } //safeToCastTo

  UTI NodeAtomof::getAtomType()
  {
    return m_atomoftype;
  }

  UTI NodeAtomof::checkAndLabelType()
  {
    UTI nuti = Nouti;
    if(m_token.m_type == TOK_TYPE_IDENTIFIER)
      {
	assert(m_nodeTypeDesc);
	nuti = m_nodeTypeDesc->checkAndLabelType(); //sets goagain if hzy
      } // got type
    else if(m_token.m_type == TOK_IDENTIFIER)
      {
	if(m_varSymbol == NULL)
	  {
	    // like NodeIdent, in case of template instantiations
	    //used before defined, start search with current block
	    if(m_currBlockNo == 0)
	      {
		if(m_state.useMemberBlock())
		  {
		    NodeBlockClass * memberclass = m_state.getCurrentMemberClassBlock();
		    assert(memberclass);
		    m_currBlockNo = memberclass->getNodeNo();
		  }
		else
		  m_currBlockNo = m_state.getCurrentBlock()->getNodeNo();
	      }

	    NodeBlock * currBlock = getBlock();
	    if(m_state.useMemberBlock())
	      m_state.pushCurrentBlock(currBlock);
	    else
	      m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

	    Symbol * asymptr = NULL;
	    bool hazyKin = false; //useful?

	    //searched back through all block's ST for idx
	    if(m_state.alreadyDefinedSymbol(m_token.m_dataindex, asymptr, hazyKin))
	      {
		if(hazyKin)
		  nuti = Hzy;
		else
		  {
		    if(!asymptr->isFunction() && !asymptr->isTypedef() && !asymptr->isConstant() && !asymptr->isModelParameter())
		      {
			nuti = asymptr->getUlamTypeIdx();
			m_varSymbol = (SymbolVariable *) asymptr;
			m_currBlockNo = asymptr->getBlockNoOfST(); //refined
		      }
		    else
		      {
			std::ostringstream msg;
			msg << "(1) <" << m_state.getTokenDataAsString(&m_token).c_str();
			msg << "> is not a variable, and cannot be used as one with ";
			msg << getName();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			nuti = Nav;
		      }
		  }
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Unfound symbol variable for .atomof '";
		msg << m_state.getTokenDataAsString(&m_token).c_str();
		msg << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		nuti = Nav;
	      }
	    m_state.popClassContext(); //restore
	  }
	else
	  nuti = m_varSymbol->getUlamTypeIdx();
      }
    else
      assert(0); //shouldn't happen

    if(m_state.okUTItoContinue(nuti))
      {
	if(!m_state.isComplete(nuti)) //reloads
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Type for '";
	    msg << m_state.getTokenDataAsString(&m_token).c_str();
	    msg << getName();
	    msg << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    nuti = Hzy;
	    m_state.setGoAgain(); //since not error
	  }
	else
	  {
	    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
	    if(!((nut->getUlamTypeEnum() == Class) || m_state.isAtom(nuti)))
	      {
		std::ostringstream msg;
		msg << "Invalid non-class type provided: '";
		msg << m_state.getTokenDataAsString(&m_token).c_str();
		msg << getName();
		msg << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		nuti = Nav;
	      }

	    if(!nut->isScalar())
	      {
		std::ostringstream msg;
		msg << "Invalid non-scalar type provided: '";
		msg << m_state.getTokenDataAsString(&m_token).c_str();
		msg << getName();
		msg << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		nuti = Nav;
	      }
	  } //complete
      } //ok

    if(m_state.okUTItoContinue(nuti))
      {
	m_atomoftype = nuti; //set here!!
	//nuti = UAtomRef;
	if(m_token.m_type == TOK_IDENTIFIER)
	  {
	    Node::setStoreIntoAble(TBOOL_TRUE);
	    nuti = UAtomRef;
	  }
	else
	  {
	    Node::setStoreIntoAble(TBOOL_FALSE);
	    nuti = UAtom;
	  }
      }

    setNodeType(nuti);
    return nuti;
  } //checkAndLabelType

  NNO NodeAtomof::getBlockNo() const
  {
    return m_currBlockNo;
  }

  NodeBlock * NodeAtomof::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  EvalStatus NodeAtomof::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    // quark or nonclass data member;
    evalNodeProlog(0); //new current node eval frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr

    UlamValue uvp = makeUlamValuePtr();
    UlamValue uv = m_state.getPtrTarget(uvp);

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeAtomof::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    // return ptr to this local var (from NodeIdent's makeUlamValuePtr)
    UlamValue rtnUVPtr = makeUlamValuePtr();

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeAtomof::makeUlamValuePtr()
  {
    // (from NodeVarDecl's makeUlamValuePtr)
    UlamValue ptr;
    UlamValue atomuv;

    UTI auti = getAtomType();
    UlamType * aut = m_state.getUlamTypeByIndex(auti);
    ULAMCLASSTYPE aclasstype = aut->getUlamClass();

    if(m_token.m_type == TOK_TYPE_IDENTIFIER)
      {
	u32 atop = 1; //m_state.m_funcCallStack.getAbsoluteTopOfStackIndexOfNextSlot();
	if(m_state.isAtom(auti))
	  atomuv = UlamValue::makeAtom(auti);
	else if(aclasstype == UC_ELEMENT)
	  atomuv = UlamValue::makeDefaultAtom(auti, m_state);
	else if(aclasstype == UC_QUARK)
	  {
	    u32 dq = 0;
	    AssertBool isDefinedQuark = m_state.getDefaultQuark(auti, dq); //returns scalar dq
	    assert(isDefinedQuark);
	    atomuv = UlamValue::makeImmediate(auti, dq, m_state);
	  }
	else
	  assert(0);

	m_state.m_funcCallStack.storeUlamValueAtStackIndex(atomuv, atop); //stackframeslotindex ?
	ptr = UlamValue::makePtr(atop, STACK, auti, UNPACKED, m_state, 0);
	//ptr.setUlamValueTypeIdx(PtrAbs);
      }
    else if(m_token.m_type == TOK_IDENTIFIER)
      {
	assert(m_varSymbol);

	if(m_varSymbol->isSelf())
	  {
	    //when "self/atom" is a quark, we're inside a func called on a quark (e.g. dm or local)
	    //'atom' gets entire atom/element containing this quark; including its type!
	    //'self' gets type/pos/len of the quark from which 'atom' can be extracted
	    UlamValue selfuvp = m_state.m_currentSelfPtr;
	    UTI ttype = selfuvp.getPtrTargetType();
	    assert(m_state.okUTItoContinue(ttype));
	    if((m_state.getUlamTypeByIndex(ttype)->getUlamClass() == UC_QUARK))
	      {
		u32 vid = m_varSymbol->getId();
		if(vid == m_state.m_pool.getIndexForDataString("self"))
		  {
		    selfuvp = m_state.getAtomPtrFromSelfPtr();
		  }
		//else
	      }
	    return selfuvp;
	  } //done

	if(m_varSymbol->getAutoLocalType() == ALT_AS)
	  return ((SymbolVariableStack *) m_varSymbol)->getAutoPtrForEval(); //haha! we're done.

	if(aclasstype == UC_ELEMENT)
	  {
	    // ptr to explicit atom or element, (e.g.'f' in f.a=1) becomes new m_currentObjPtr
	    ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, auti, UNPACKED, m_state, 0, m_varSymbol->getId());
	  }
	else
	  {
	    if(m_varSymbol->isDataMember())
	      {
		// return ptr to the m_currentObjPtr that contains this data member within
		ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), auti, m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	      }
	    else
	      {
		//local variable on the stack; could be array ptr!
		ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, auti, m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	      }
	  }
      }
    else
      assert(0);

    return ptr;
  } //makeUlamValuePtr

  void NodeAtomof::genCode(File * fp, UlamValue& uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber(); //tmp for atomref
    u32 selfid = m_state.m_pool.getIndexForDataString("Self");
    if((m_token.m_type == TOK_TYPE_IDENTIFIER) && (m_token.m_dataindex == selfid))
      {
	m_state.indent(fp); //non-const
	fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write(";\n");

	u32 tmpuclass = m_state.getNextTmpVarNumber(); //only for this case
	m_state.indent(fp);
	fp->write("const UlamClass<EC> * ");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	fp->write(" = ur.GetEffectiveSelf();\n");

	m_state.indent(fp);
	fp->write("if(");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	fp->write("->AsUlamQuark() != NULL)\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write(".WriteAtom(");
	UlamType * aut = m_state.getUlamTypeByIndex(m_atomoftype);
	fp->write(aut->getLocalStorageTypeAsString().c_str()); //default constr
	fp->write("()");
	fp->write(".ReadAtom()");
	fp->write("); //default quark atomof\n");
	m_state.m_currentIndentLevel--;

	m_state.indent(fp);
	fp->write("else\n");

	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;

	m_state.indent(fp);
	fp->write("if(");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	fp->write(" == NULL) FAIL(ILLEGAL_ARGUMENT);\n");

	m_state.indent(fp);
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write(".WriteAtom(");
	fp->write("((UlamElement<EC> *) ");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	fp->write(")->GetDefaultAtom()); //default element atomof\n");

	m_state.m_currentIndentLevel--;

	m_state.indent(fp);
	fp->write("} \n");

#if 0
	m_state.indent(fp); //non-const
	fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write(";\n");

	m_state.indent(fp);
	fp->write("if(ur.GetType() == T::ATOM_UNDEFINED_TYPE)\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write(".WriteAtom(");
	UlamType * aut = m_state.getUlamTypeByIndex(m_atomoftype);
	fp->write(aut->getLocalStorageTypeAsString().c_str()); //default constr
	fp->write("()");
	fp->write(".ReadAtom()");
	fp->write(");\n");
	m_state.m_currentIndentLevel--;

	m_state.indent(fp);
	fp->write("else\n");

	m_state.indent(fp);
	fp->write("{\n");

	u32 tmpuclass = m_state.getNextTmpVarNumber(); //only for this case
	m_state.m_currentIndentLevel++;
	m_state.indent(fp);
	fp->write("const UlamClass<EC> * ");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	fp->write(" = uc.LookupElementTypeFromContext(ur.GetType());\n");

	m_state.indent(fp);
	fp->write("if(");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	fp->write(" == NULL) FAIL(ILLEGAL_ARGUMENT);\n");

	m_state.indent(fp);
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write(".WriteAtom(");
	fp->write("((UlamElement<EC> *) ");
	fp->write(m_state.getUlamClassTmpVarAsString(tmpuclass).c_str());
	fp->write(")->GetDefaultAtom()); //atomof\n");
	m_state.m_currentIndentLevel--;

	m_state.indent(fp);
	fp->write("} \n");
#endif

      }
    else
      {
	m_state.indent(fp); //non-const
	fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPBITVAL).c_str());
	fp->write("(");

	if(m_token.m_type == TOK_TYPE_IDENTIFIER)
	  {
	    UlamType * aut = m_state.getUlamTypeByIndex(m_atomoftype);
	    fp->write(aut->getLocalStorageTypeAsString().c_str()); //default constr
	    fp->write("()");
	  }
	else
	  {
	    assert(m_varSymbol);
	    //data member's storage is self
	    if(m_varSymbol->isDataMember())
	      fp->write("ur");
	    else if(m_varSymbol->isSelf())
	      fp->write("ur");
	    else
	      fp->write(m_varSymbol->getMangledName().c_str());
	  }

	if(getStoreIntoAble() == TBOOL_FALSE) //for isSelfType
	  fp->write(".ReadAtom()");
	else
	  fp->write(".GetStorage()"); //can't be const

	fp->write(", uc); //atomof \n");
      }

    uvpass = UlamValue::makePtr(tmpVarNum, TMPBITVAL, nuti, UNPACKED, m_state, 0, m_varSymbol ? m_varSymbol->getId() : 0);
  } //genCode

  void NodeAtomof::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    //lhs
    assert(getStoreIntoAble() == TBOOL_TRUE);
    NodeAtomof::genCode(fp, uvpass);
  } //genCodeToStoreInto

} //end MFM
