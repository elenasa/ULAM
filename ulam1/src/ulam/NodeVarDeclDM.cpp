#include <stdlib.h>
#include "NodeVarDeclDM.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"
#include "NodeTerminal.h"

namespace MFM {

  NodeVarDeclDM::NodeVarDeclDM(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarDecl(sym, nodetype, state), m_nodeInitExpr(NULL)
  { }

  NodeVarDeclDM::NodeVarDeclDM(const NodeVarDeclDM& ref) : NodeVarDecl(ref), m_nodeInitExpr(NULL)
  {
    if(ref.m_nodeInitExpr)
      m_nodeInitExpr = (Node *) ref.m_nodeInitExpr->instantiate();
  }

  NodeVarDeclDM::~NodeVarDeclDM()
  {
    delete m_nodeInitExpr;
    m_nodeInitExpr = NULL;
  }

  Node * NodeVarDeclDM::instantiate()
  {
    return new NodeVarDeclDM(*this);
  }

  void NodeVarDeclDM::updateLineage(NNO pno)
  {
    NodeVarDecl::updateLineage(pno);
    if(m_nodeInitExpr)
      m_nodeInitExpr->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeVarDeclDM::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeInitExpr == oldnptr)
      {
	m_nodeInitExpr = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeVarDeclDM::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeVarDecl::findNodeNo(n, foundNode))
      return true;
    if(m_nodeInitExpr && m_nodeInitExpr->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  // see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarDeclDM::printPostfix(File * fp)
  {
    NodeVarDecl::printTypeAndName(fp);

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(nuti == Nav)
      {
	fp->write("(");
	//fp->write_decimal(0);
	fp->write("unknown");
	fp->write("); ");
	return;
      }

    if(nut->getUlamClass() == UC_QUARK)
      {
	SymbolClass * csym = NULL;
	assert(m_state.alreadyDefinedSymbolClass(nuti, csym));
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

  UTI NodeVarDeclDM::checkAndLabelType()
  {
    NodeVarDecl::checkAndLabelType(); //sets node type

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

	UTI it = m_nodeInitExpr->checkAndLabelType();
	if(it == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for data member: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is invalid";
	    if(m_nodeInitExpr->isReadyConstant())
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		m_state.setGoAgain(); //since not error
	      }
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	//constant fold if possible, set symbol value
	if(m_varSymbol)
	  {
	    assert(((SymbolVariableDataMember *) m_varSymbol)->hasInitValue());
	    if(!(((SymbolVariableDataMember *) m_varSymbol)->initValueReady()))
	      {
		foldConstantExpression(); //sets init constant value
		if(!(((SymbolVariableDataMember *) m_varSymbol)->initValueReady()))
		  setNodeType(Nav);
	      }
	  }
	else
	  assert(0);

	//insure constant value fits in its declared type
	UTI nuti = getNodeType();
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
	  }
	else
	  assert(Node::makeCastingNode(m_nodeInitExpr, nuti, m_nodeInitExpr)); //we know it's safe!
      } //finished init expr node

    return getNodeType();
  } //checkAndLabelType

  void NodeVarDeclDM::countNavNodes(u32& cnt)
  {
    if(m_nodeInitExpr)
      m_nodeInitExpr->countNavNodes(cnt);

    NodeVarDecl::countNavNodes(cnt);
  } //countNavNodes

  void NodeVarDeclDM::setConstantExpr(Node * node)
  {
    assert(node);
    m_nodeInitExpr = node;
    m_nodeInitExpr->updateLineage(getNodeNo()); //for unknown subtrees
    if(m_varSymbol)
      ((SymbolVariableDataMember *) m_varSymbol)->setHasInitValue();
  }

  //from NodeConstantDef; applied here to init value.
  // called during parsing rhs of named constant;
  // Requires a constant expression, else error;
  // (scope of eval is based on the block of const def.)
  bool NodeVarDeclDM::foldConstantExpression()
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
  } //foldConstantExpression

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
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize > 0);
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
		assert(m_state.alreadyDefinedSymbolClass(scalaruti, csym));

		u32 qval = 0;
		assert(csym->getDefaultQuark(qval));

		//initialize each array item
		u32 arraysize = m_state.getArraySize(nuti);
		qval &= mask;

		for(u32 j = 0; j < arraysize; j++)
		  {
		    dqref |= (qval << (quarksize - (pos + (j * bitsize))));
		  }
		aok = true;
	      }
	  }
	else
	  {
	    //primitive (not a quark!)
	    u64 val = 0;
	    if(((SymbolVariableDataMember *) m_varSymbol)->getInitValue(val))
	      {
		valinposition = ((u32) val & mask) << (quarksize - pos - bitsize);
		dqref |= valinposition;
		aok = true;
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

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClass();

    if(nuti == UAtom || classtype == UC_ELEMENT)
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

    EvalStatus evs = NORMAL; //init
    // quark or nonclass data member;
    if(((SymbolVariableDataMember *) m_varSymbol)->hasInitValue())
      {
	evalNodeProlog(0); //new current node eval frame pointer

	makeRoomForSlots(1); //always 1 slot for ptr

	evs = evalToStoreInto();
	if(evs != NORMAL)
	  {
	    evalNodeEpilog();
	    return evs;
	  }

	UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

	u32 slot = makeRoomForNodeType(nuti);

	evs = m_nodeInitExpr->eval();

	if(evs == NORMAL)
	  {
	    if(slot)
	      {
		UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slot+1); //immediate scalar
		m_state.assignValue(pluv,ruv);

		//also copy result UV to stack, -1 relative to current frame pointer
		assignReturnValueToStack(ruv);
	      }
	  } //normal
	evalNodeEpilog();
      } //has init value
    return evs;
  } //eval

  EvalStatus NodeVarDeclDM::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    // (from NodeIdent's makeUlamValuePtr)
    // return ptr to this data member within the m_currentObjPtr
    // 'pos' modified by this data member symbol's packed bit position
    UlamValue rtnUVPtr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  // parse tree in order declared, unlike the ST.
  void NodeVarDeclDM::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_varSymbol);
    assert(getNodeType() != Nav);

    if(m_varSymbol->isDataMember())
      {
	return genCodedBitFieldTypedef(fp, uvpass);
      }

    if(m_varSymbol->isAutoLocal())
      {
	return genCodedAutoLocal(fp, uvpass);
      }

    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indent(fp);
    if(!m_varSymbol->isDataMember())
      {
	fp->write(vut->getImmediateStorageTypeAsString().c_str()); //for C++ local vars
      }
    else
      {
	fp->write(vut->getUlamTypeMangledName().c_str()); //for C++
	assert(0); //doesn't happen anymore..
      }

    fp->write(" ");
    fp->write(m_varSymbol->getMangledName().c_str());

    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    //initialize T to default atom (might need "OurAtom" if data member ?)
    if(vclasstype == UC_ELEMENT)
      {
	fp->write(" = ");
	fp->write(m_state.getUlamTypeByIndex(vuti)->getUlamTypeMangledName().c_str());
	fp->write("<EC>");
	fp->write("::THE_INSTANCE");
	fp->write(".GetDefaultAtom()"); //returns object of type T
      }

    if(vclasstype == UC_QUARK)
      {
	//right-justified?
      }

    fp->write(";\n"); //func call parameters aren't NodeVarDeclDM's
  } //genCode

} //end MFM
