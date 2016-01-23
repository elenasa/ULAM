#include <stdlib.h>
#include "NodeVarDeclDM.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"
#include "NodeTerminal.h"

namespace MFM {

  NodeVarDeclDM::NodeVarDeclDM(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarDecl(sym, nodetype, state) { }

  NodeVarDeclDM::NodeVarDeclDM(const NodeVarDeclDM& ref) : NodeVarDecl(ref) { }

  NodeVarDeclDM::~NodeVarDeclDM(){ }

  Node * NodeVarDeclDM::instantiate()
  {
    return new NodeVarDeclDM(*this);
  }

  void NodeVarDeclDM::updateLineage(NNO pno)
  {
    NodeVarDecl::updateLineage(pno);
  } //updateLineage

  bool NodeVarDeclDM::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeVarDecl::findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  // see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarDeclDM::printPostfix(File * fp)
  {
    NodeVarDecl::printTypeAndName(fp);

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if((nuti == Nav) || (nuti == Nouti))
      {
	fp->write("(");
	fp->write("unknown");
	fp->write("); ");
	return;
      }

    if(nuti == Hzy)
      {
	fp->write("(");
	fp->write("notready");
	fp->write("); ");
	return;
      }

    if(nut->getUlamClass() == UC_QUARK)
      {
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
	assert(isDefined);
	NodeBlockClass * classblock = csym->getClassBlockNode();
	assert(classblock);

	s32 arraysize = nut->getArraySize();
	//scalar has 'size=1'; empty array [0] is still '0'.
	s32 size = (arraysize > NONARRAYSIZE ? arraysize : 1);

	m_state.pushClassContext(csym->getUlamTypeIdx(), classblock, classblock, false, NULL);

	for(s32 i = 0; i < size; i++)
	  {
	    if(i > 0)
	      fp->write("), (");
	    else
	      fp->write("(");
	    classblock->printPostfixDataMembersParseTree(fp); //same default values
	  }
	m_state.popClassContext();
      }
    else
      {
	fp->write("(");

	if(m_nodeInitExpr)
	  {
	    // only for quarks and scalars
	    m_nodeInitExpr->printPostfix(fp);
	  }
	else
	  {
	    //default (uninitialized) values
	    char dstr[40];

	    nut->getDataAsString(0, dstr, 'z');
	    fp->write(dstr);

	    if(!m_state.isScalar(nuti))
	      {
		s32 arraysize = m_state.getArraySize(nuti);
		for(s32 i = 1; i < arraysize; i++)
		  {
		    nut->getDataAsString(0, dstr, ',');
		    fp->write(dstr);
		  }
	      }
	  }
      }
    fp->write(")");
    fp->write("; ");
  } //printPostfix

  const char * NodeVarDeclDM::getName()
  {
    return NodeVarDecl::getName();
  }

  const std::string NodeVarDeclDM::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeVarDeclDM::safeToCastTo(UTI newType)
  {
    UTI nuti = getNodeType();
    //cast RHS if necessary and safe
    //insure constant value fits in its declared type
    FORECAST rscr = m_nodeInitExpr->safeToCastTo(nuti);
    if(rscr != CAST_CLEAR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for data member (";
	msg << getName() << " = " << m_nodeInitExpr->getName();
	msg << ") initialization is not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	if(rscr == CAST_BAD)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.setGoAgain(); //since not error
	  }
      }
    else
      {
	AssertBool isDefined = Node::makeCastingNode(m_nodeInitExpr, nuti, m_nodeInitExpr); //we know it's safe!
	assert(isDefined);
      }
    return rscr;
  } //safeToCastTo

  UTI NodeVarDeclDM::checkAndLabelType()
  {
    NodeVarDecl::checkAndLabelType(); //sets node type

    //don't allow a subclass to shadow a superclass datamember
    UTI cuti = m_state.getCompileThisIdx();
    UTI superuti = m_state.isClassASubclass(cuti);
    if(superuti == Hzy)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
	return Hzy;
      }
    if(superuti != Nouti) //has ancestor
      {
	//is a subclass' DM..
	//check for shadowed superclass DM of same name
	NodeBlockClass * superclassblock = m_state.getAClassBlock(superuti);
	assert(superclassblock);
	m_state.pushClassContextUsingMemberClassBlock(superclassblock);

	Symbol * varSymbolOfSameName = NULL;
	bool hazyKin = false;
	if(m_state.alreadyDefinedSymbol(m_vid, varSymbolOfSameName, hazyKin))
	  {
	    std::ostringstream msg;
	    msg << "Data member '";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << "' is shadowing an ancestor";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    m_state.popClassContext();
	    return Nav; //short-circuit
	  }
	m_state.popClassContext();
      } //end subclass error checking

    if(m_nodeInitExpr)
      {
	if(!m_nodeInitExpr->isAConstant())
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for data member: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is not a constant";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	UTI it = m_nodeInitExpr->getNodeType();
	if(it == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for data member: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is invalid";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	if(it == Hzy)
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for data member: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is not ready";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.setGoAgain(); //since not error
	    setNodeType(Hzy);
	    return Hzy; //short-circuit
	  }

	//constant fold if possible, set symbol value
	if(m_varSymbol)
	  {
	    AssertBool isDefined = ((SymbolVariableDataMember *) m_varSymbol)->hasInitValue();
	    assert(isDefined);
	    if(!(((SymbolVariableDataMember *) m_varSymbol)->initValueReady()))
	      {
		foldInitExpression(); //sets init constant value
		if(!(((SymbolVariableDataMember *) m_varSymbol)->initValueReady()))
		  {
		    setNodeType(Hzy);
		    m_state.setGoAgain(); //since not error
		    return Hzy;
		  }
	      }
	  }
	else
	  assert(0);

	//insure constant value fits in its declared type,
	//done in NodeVarDecl c&l
	//safeToCastTo(nuti);
      } //finished init expr node

    return getNodeType();
  } //checkAndLabelType

  void NodeVarDeclDM::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeVarDecl::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  void NodeVarDeclDM::setInitExpr(Node * node)
  {
    NodeVarDecl::setInitExpr(node);
    if(m_varSymbol)
      ((SymbolVariableDataMember *) m_varSymbol)->setHasInitValue();
  }

  //from NodeConstantDef; applied here to init value.
  // called during parsing rhs of named constant;
  // Requires a constant expression, else error;
  // (scope of eval is based on the block of const def.)
  bool NodeVarDeclDM::foldInitExpression()
  {
    UTI nuti = getNodeType();

    if(nuti == Nav || !m_state.isComplete(nuti))
      return false; //e.g. not a constant

    assert(m_varSymbol);
    if(((SymbolVariableDataMember *) m_varSymbol)->initValueReady())
      return true; //short-circuit

    if(!m_nodeInitExpr)
      return false;

    // if here, must be a constant init value..
    u64 newconst = 0; //UlamType format (not sign extended)
    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(nuti); //offset a constant expression
    EvalStatus evs = m_nodeInitExpr->eval();
    if(evs == NORMAL)
      {
	UlamValue cnstUV = m_state.m_nodeEvalStack.popArg();
	u32 wordsize = m_state.getTotalWordSize(nuti);
	if(wordsize <= MAXBITSPERINT)
	  newconst = cnstUV.getImmediateData(m_state);
	else if(wordsize <= MAXBITSPERLONG)
	  newconst = cnstUV.getImmediateDataLong(m_state);
	else
	  assert(0);
      }

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for data member '";
	msg << m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str();
	msg << "' initialization is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	m_state.setGoAgain(); //since not error
	return false;
      }

    //insure constant value fits in its declared type
    FORECAST scr = m_nodeInitExpr->safeToCastTo(nuti);
    if(scr != CAST_CLEAR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for data member (";
	msg << getName() << " = " << m_nodeInitExpr->getName() ;
	msg << ") initialization is not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	if(scr == CAST_BAD)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.setGoAgain(); //since not error
	  }
	return false; //necessary if not just a warning.
      }

    //folded here..(but no checkandlabel yet)
    if(updateConstant(newconst))
      {
	NodeTerminal * newnode;
	if(m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum() == Int)
	  newnode = new NodeTerminal((s64) newconst, nuti, m_state);
	else
	  newnode = new NodeTerminal(newconst, nuti, m_state);

	newnode->setNodeLocation(getNodeLocation());
	delete m_nodeInitExpr;
	m_nodeInitExpr = newnode;
      }
    else
      return false;

    ((SymbolVariableDataMember *) m_varSymbol)->setInitValue(newconst); //isReady now!
    return true;
  } //foldInitExpression

  bool NodeVarDeclDM::updateConstant(u64 & newconst)
  {
    if(!m_varSymbol)
      return false;

    UTI nuti = getNodeType();
    if(!m_state.isComplete(nuti))
      {
	m_state.setGoAgain(); //since not error
	return false;
      }

    //store in UlamType format
    bool rtnb = true;
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    AssertBool isKnownSize = (nut->getBitSize() > 0);
    assert(isKnownSize);
    u32 wordsize = nut->getTotalWordSize();
    if(wordsize <= MAXBITSPERINT)
      rtnb = updateConstant32(newconst);
    else if(wordsize <= MAXBITSPERLONG)
      rtnb = updateConstant64(newconst);
    else
      assert(0);

    if(!rtnb)
      {
	std::ostringstream msg;
	msg << "Constant Type Unknown: ";
	msg <<  m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return rtnb;
  } //updateConstant

  bool NodeVarDeclDM::updateConstant32(u64 & newconst)
  {
    u64 val = newconst; //wo any sign extend to start
    //store in UlamType format
    bool rtnb = true;
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());
    s32 nbitsize = nut->getBitSize();
    u32 srcbitsize = m_nodeInitExpr ? m_state.getBitSize(m_nodeInitExpr->getNodeType()) : nbitsize; //was MAXBITSPERINT WRONG!

    ULAMTYPE etype = nut->getUlamTypeEnum();
    switch(etype)
      {
      case Int:
	newconst = _Int32ToInt32((u32) val, srcbitsize, nbitsize); //signextended
	break;
      case Unsigned:
	newconst = _Unsigned32ToUnsigned32((u32) val, srcbitsize, nbitsize);
	break;
      case Bool:
	//newconst = _Unsigned32ToBool32(val, MAXBITSPERINT, nbitsize);
	newconst = _CboolToBool32( (bool) val, nbitsize);
	break;
      case Unary:
	newconst =  _Unsigned32ToUnary32((u32) val, srcbitsize, nbitsize);
	break;
      case Bits:
	newconst = _Unsigned32ToBits32((u32) val, srcbitsize, nbitsize);
	break;
      default:
	rtnb = false;
      };
    return rtnb;
  } //updateConstant32

  bool NodeVarDeclDM::updateConstant64(u64 & newconst)
  {
    u64 val = newconst; //wo any sign extend to start
    //store in UlamType format
    bool rtnb = true;
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());
    s32 nbitsize = nut->getBitSize();
    u32 srcbitsize = m_nodeInitExpr ? m_state.getBitSize(m_nodeInitExpr->getNodeType()) : nbitsize; //was MAXBITSPERINT WRONG!
    ULAMTYPE etype = nut->getUlamTypeEnum();
    switch(etype)
      {
      case Int:
	newconst = _Int64ToInt64(val, srcbitsize, nbitsize); //signextended
	break;
      case Unsigned:
	newconst = _Unsigned64ToUnsigned64(val, srcbitsize, nbitsize);
	break;
      case Bool:
	//newconst = _Unsigned64ToBool64(val, MAXBITSPERLONG, nbitsize);
	newconst = _CboolToBool64( (bool) val, nbitsize);
	break;
      case Unary:
	newconst =  _Unsigned64ToUnary64(val, srcbitsize, nbitsize);
	break;
      case Bits:
	newconst = _Unsigned64ToBits64(val, srcbitsize, nbitsize);
	break;
      default:
	  rtnb = false;
      };
    return rtnb;
  } //updateConstant64

  //right-justified
  bool NodeVarDeclDM::buildDefaultQuarkValue(u32& dqref)
  {
    bool aok = false; //init as not ready
    UTI nuti = getNodeType(); //same as symbol uti
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClass();

    if(m_nodeInitExpr || classtype == UC_QUARK)
      {
	assert(m_varSymbol);
	assert(m_varSymbol->isDataMember());

	u32 mask = _GetNOnes32((u32) nut->getBitSize());
	u32 pos = m_varSymbol->getPosOffset();
	u32 valinposition = 0;
	s32 bitsize = m_state.getBitSize(nuti);
	s32 quarksize = m_state.getBitSize(m_state.getCompileThisIdx());

	if(classtype == UC_QUARK)
	  {
	    if(m_state.isScalar(nuti))
	      {
		SymbolClass * csym = NULL;
		if(m_state.alreadyDefinedSymbolClass(nuti, csym))
		  {
		    u32 qval = 0;
		    if(csym->getDefaultQuark(qval))
		      {
			valinposition = (qval & mask) << (quarksize - bitsize - pos);
			dqref |= valinposition;
			aok = true;
		      }
		  }
	      }
	    else
	      {
		//CAN'T take up more than u32 and be a quark DM.
		//array of quarks
		// first, get default value of its scalar quark
		UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
		u32 bitsize = m_state.getBitSize(nuti);
		SymbolClass * csym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClass(scalaruti, csym);
		assert(isDefined);

		u32 qval = 0;
		AssertBool isDefinedQuark = csym->getDefaultQuark(qval);
		assert(isDefinedQuark);

		//initialize each array item
		u32 arraysize = m_state.getArraySize(nuti);
		qval &= mask;

		for(u32 j = 1; j <= arraysize; j++)
		  {
		    dqref |= (qval << (quarksize - (pos + (j * bitsize))));
		  }
		aok = true;
	      }
	  }
	else
	  {
	    //arrays not initialized at this time
	    //primitive (not a quark!)
	    if(m_state.isScalar(nuti))
	      {
		u64 val = 0;
		if(((SymbolVariableDataMember *) m_varSymbol)->getInitValue(val))
		  {
		    valinposition = ((u32) val & mask) << (quarksize - pos - bitsize);
		    dqref |= valinposition;
		    aok = true;
		  }
	      }
	  }

	//fold quark here for node eval, printpostfix..
	if(classtype == UC_QUARK && aok)
	  foldDefaultQuark(dqref);
      }
    else
      aok = true; //no initialized value

    return aok;
  } //buildDefaultQuarkValue

  bool NodeVarDeclDM::foldDefaultQuark(u32 dq)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    u64 dqval = 0;
    //build dqval array
    if(!nut->isScalar())
      {
	//array of quarks
	s32 quarksize = nut->getBitSize();
	s32 arraysize = nut->getArraySize();
	s32 totalbitsize = quarksize * arraysize;
	s32 qwordsize = nut->getTotalWordSize();
	if(qwordsize > MAXBITSPERLONG) //64
	  {
	    std::ostringstream msg;
	    msg << "Not supported at this time, UNPACKED Quark array type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return false;
	  }

	for(s32 i = 0; i < arraysize; i++)
	  dqval |= (dq << (totalbitsize - quarksize - (i * quarksize)));
      }
    else
      dqval = dq;

    //folding into a terminal node
    NodeTerminal * newnode = new NodeTerminal(dqval, nuti, m_state);
    newnode->setNodeLocation(getNodeLocation());
    delete m_nodeInitExpr;
    m_nodeInitExpr = newnode;
    //(in this order)
    ((SymbolVariableDataMember *) m_varSymbol)->setHasInitValue();
    ((SymbolVariableDataMember *) m_varSymbol)->setInitValue(dqval);
    return true;
  } //foldDefaultQuark

  EvalStatus NodeVarDeclDM::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClass();

    assert(m_varSymbol->getAutoLocalType() == ALT_NOT);

    if(nut->getUlamTypeEnum() == UAtom || classtype == UC_ELEMENT)
      return NodeVarDecl::eval();

    // make terminal expression node so rest of eval works for quarks too
    if(classtype == UC_QUARK && m_nodeInitExpr == NULL)
      {
	u32 dqval = 0;
	if(m_state.getDefaultQuark(nuti, dqval))
	  {
	    if(!foldDefaultQuark(dqval))
	      return ERROR;
	  }
	else
	  return ERROR;
      }

    // quark or nonclass data member;
    if(((SymbolVariableDataMember *) m_varSymbol)->hasInitValue())
      {
	return NodeVarDecl::evalInitExpr();
      }
    return NORMAL;
  } //eval

  EvalStatus NodeVarDeclDM::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    // (from NodeIdent's makeUlamValuePtr)
    // return ptr to this data member within the m_currentObjPtr
    // 'pos' modified by this data member symbol's packed bit position
    UlamValue rtnUVPtr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  // parse tree in order declared, unlike the ST.
  void NodeVarDeclDM::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_varSymbol);
    assert(m_state.isComplete(getNodeType()));

    assert(m_varSymbol->isDataMember());

    return genCodedBitFieldTypedef(fp, uvpass);
  } //genCode

  // variable is a data member; cannot be an element
  void NodeVarDeclDM::genCodedBitFieldTypedef(File * fp, UlamValue& uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE nclasstype = nut->getUlamClass();
    ULAMCLASSTYPE classtype = m_state.getUlamClassForThisClass();

    m_state.indent(fp);
    if(nclasstype == UC_QUARK && nut->isScalar())
      {
	// use typedef rather than atomic parameter for quarks within elements,
	// except if an array of quarks.
	fp->write("typedef ");
	fp->write(nut->getUlamTypeMangledName().c_str()); //for C++
	fp->write("<EC, ");
	if(classtype == UC_ELEMENT)
	  {
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset() + ATOMFIRSTSTATEBITPOS);
	    fp->write("u");
	  }
	else
	  {
	    //inside a quark
	    fp->write("POS + ");
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset());
	    fp->write("u");
	  }
      }
    else
      {
	fp->write("typedef AtomicParameterType");
	fp->write("<EC"); //BITSPERATOM
	fp->write(", ");
	fp->write(nut->getUlamTypeVDAsStringForC().c_str());
	fp->write(", ");
	fp->write_decimal(nut->getTotalBitSize());   //include arraysize
	fp->write(", ");
	if(classtype == UC_QUARK)
	  {
	    fp->write("POS + ");
	    fp->write_decimal(m_varSymbol->getPosOffset());
	  }
	else
	  {
	    assert(classtype == UC_ELEMENT);
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset() + ATOMFIRSTSTATEBITPOS);
	    fp->write("u");
	  }
      }
    fp->write("> ");
    fp->write(m_varSymbol->getMangledNameForParameterType().c_str());
    fp->write(";\n"); //func call parameters aren't NodeVarDecl's
  } //genCodedBitFieldTypedef

  void NodeVarDeclDM::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    UTI nuti = getNodeType();

    //output a case of switch statement
    m_state.indent(fp);
    fp->write("case ");
    fp->write_decimal(dmcount);
    fp->write(": { static UlamClassDataMemberInfo i(\"");
    fp->write(m_state.getUlamTypeByIndex(nuti)->getUlamTypeMangledName().c_str());
    fp->write("\", \"");
    fp->write(m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str());
    fp->write("\", ");
    fp->write_decimal(m_varSymbol->getPosOffset());
    fp->write("u); return i; }\n");

    dmcount++; //increment data member count
  } //generateUlamClassInfo

} //end MFM
