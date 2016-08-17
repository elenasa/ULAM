#include <stdlib.h>
#include "NodeConstantDef.h"
#include "NodeConstant.h"
#include "NodeListArrayInitialization.h"
#include "NodeTerminal.h"
#include "CompilerState.h"


namespace MFM {

  NodeConstantDef::NodeConstantDef(SymbolWithValue * symptr, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_constSymbol(symptr), m_nodeExpr(NULL), m_currBlockNo(m_state.getCurrentBlockNo()), m_nodeTypeDesc(nodetype)
  {
    if(symptr)
      {
	// node uses current block no, not the one saved in the symbol
	// (e.g. pending class args)
	m_cid = symptr->getId();
	symptr->setDeclNodeNo(getNodeNo());
      }
    else
      m_cid = 0; //error
  }

  NodeConstantDef::NodeConstantDef(const NodeConstantDef& ref) : Node(ref), m_constSymbol(NULL), m_nodeExpr(NULL), m_cid(ref.m_cid), m_currBlockNo(ref.m_currBlockNo), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeExpr)
      m_nodeExpr = ref.m_nodeExpr->instantiate();

    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeConstantDef::~NodeConstantDef()
  {
    delete m_nodeExpr;
    m_nodeExpr = NULL;

    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  Node * NodeConstantDef::instantiate()
  {
    return new NodeConstantDef(*this);
  }

  void NodeConstantDef::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    assert(m_state.getCurrentBlockNo() == m_currBlockNo);
    if(m_nodeExpr)
      m_nodeExpr->updateLineage(getNodeNo());
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeConstantDef::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeExpr == oldnptr)
      {
	m_nodeExpr = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeConstantDef::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeExpr && m_nodeExpr->findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeConstantDef::checkAbstractInstanceErrors()
  {
    if(m_nodeExpr)
      m_nodeExpr->checkAbstractInstanceErrors();
  } //checkAbstractInstanceErrors

  void NodeConstantDef::printPostfix(File * fp)
  {
    //in case the node belongs to the template, use the symbol uti, o.w. 0Nav.
    UTI suti = m_constSymbol ? m_constSymbol->getUlamTypeIdx() : getNodeType();
    //see also SymbolConstantValue
    fp->write(" constant");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(suti).c_str());
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(m_cid).c_str());

    if(m_nodeExpr)
      {
	fp->write(" =");
	m_nodeExpr->printPostfix(fp);
      }
    fp->write("; ");
  } //printPostfix

  const char * NodeConstantDef::getName()
  {
    if(m_constSymbol)
      return m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
    return "CONSTDEF?";
  }

  const std::string NodeConstantDef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeConstantDef::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  void NodeConstantDef::setSymbolPtr(SymbolWithValue * cvsymptr)
  {
    assert(cvsymptr);
    m_constSymbol = cvsymptr;
    m_currBlockNo = cvsymptr->getBlockNoOfST();
    assert(m_currBlockNo);
  } //setSymbolPtr

  u32 NodeConstantDef::getSymbolId()
  {
    return m_cid;
  }

  bool NodeConstantDef::getNodeTypeDescriptorPtr(NodeTypeDescriptor *& nodetypedescref)
  {
    if(m_nodeTypeDesc)
      {
	nodetypedescref = m_nodeTypeDesc;
	return true;
      }
    return false;
  }

  UTI NodeConstantDef::checkAndLabelType()
  {
    UTI it = Nouti; //expression type

    // instantiate, look up in current block
    if(m_constSymbol == NULL)
      checkForSymbol(); //toinstantiate

    //short circuit, avoid assert
    if(!m_constSymbol)
      {
	setNodeType(Nav);
	return Nav;
      }

    UTI suti = m_constSymbol->getUlamTypeIdx();
    UTI cuti = m_state.getCompileThisIdx();

    // type of the constant
    if(m_nodeTypeDesc)
      {
	UTI duti = m_nodeTypeDesc->checkAndLabelType(); //clobbers any expr it
	if(m_state.okUTItoContinue(duti) && (suti != duti))
	  {
	    std::ostringstream msg;
	    msg << "REPLACING Symbol UTI" << suti;
	    msg << ", " << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << " used with " << prettyNodeName().c_str() << " symbol name '" << getName();
	    msg << "' with node type descriptor type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(duti).c_str();
	    msg << " UTI" << duti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << " UTI" << cuti;
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_constSymbol->resetUlamType(duti); //consistent!
	    m_state.mapTypesInCurrentClass(suti, duti);
	    //m_state.updateUTIAliasForced(suti, duti); //help? (not when we used it and it == 0, instead of suti)
	    suti = duti;
	  }
      }

    //move before m_nodeExpr "Void" check (t3883)
    if(!m_state.okUTItoContinue(suti) || !m_state.isComplete(suti))
      {
	std::ostringstream msg;
	msg << "Incomplete " << prettyNodeName().c_str() << " for type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	msg << ", used with symbol name '" << getName() << "'";
	if(m_state.okUTItoContinue(suti) || (suti == Hzy))
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    suti = Hzy;
	    //m_state.setGoAgain(); //since not error; wait until not Nav
	  }
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    ULAMTYPE etyp = m_state.getUlamTypeByIndex(suti)->getUlamTypeEnum();
    if(etyp == Void)
      {
	//void only valid use is as a func return type
	std::ostringstream msg;
	msg << "Invalid use of type ";
	msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	msg << " with symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //could be clobbered by Hazy node expr
	return Nav;
      }

    // NOASSIGN REQUIRED (e.g. for class parameters) doesn't have to have this!
    if(m_nodeExpr)
      {
	it = m_nodeExpr->checkAndLabelType();
	if(it == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is invalid";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	if(it == Hzy)
	  {
	    UTI cuti = m_state.getCompileThisIdx();
	    std::ostringstream msg;
	    msg << "Constant value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is not ready, still hazy while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    m_state.setGoAgain();
	    setNodeType(Hzy);
	    return Hzy; //short-circuit
	  }

	if(!m_nodeExpr->isAConstant())
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is not a constant";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit (error/t3453)
	  }

	//note: Void is flag that it's a list of constant initializers;
	// code lifted from NodeVarDecl.cpp c&l.
	if(it == Void)
	  {
	    //only possible if array type with initializers
	    //m_constSymbol->setHasInitValue(); //might not be ready yet
	    if(!m_state.okUTItoContinue(suti) && m_nodeTypeDesc)
	      {
		UTI duti = m_nodeTypeDesc->getNodeType();
		UlamType * dut = m_state.getUlamTypeByIndex(duti);
		if(m_state.okUTItoContinue(duti) && !dut->isComplete())
		  {
		    assert(!dut->isScalar());
		    assert(dut->isPrimitiveType());

		    //if here, assume arraysize depends on number of initializers
		    s32 bitsize = dut->getBitSize();
		    u32 n = ((NodeList *) m_nodeExpr)->getNumberOfNodes();
		    m_state.setUTISizes(duti, bitsize, n);

		    if(m_state.isComplete(duti))
		      {
			std::ostringstream msg;
			msg << "REPLACING Symbol UTI" << it;
			msg << ", " << m_state.getUlamTypeNameBriefByIndex(it).c_str();
			msg << " used with variable symbol name '" << getName();
			msg << "' with node type descriptor array type (with initializers): ";
			msg << m_state.getUlamTypeNameBriefByIndex(duti).c_str();
			msg << " UTI" << duti << " while labeling class: ";
			msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			m_constSymbol->resetUlamType(duti); //consistent!
			m_state.mapTypesInCurrentClass(suti, duti);
			m_state.updateUTIAliasForced(suti, duti); //help?
			m_nodeExpr->setNodeType(duti); //replace Void too!
			it = duti;
		      }
		  }
		//else
	      }
	    else if(m_state.okUTItoContinue(suti) && m_state.isComplete(suti))
	      {
		//arraysize specified, may have fewer initializers
		s32 arraysize = m_state.getArraySize(suti);
		assert(arraysize >= 0); //t3847
		u32 n = ((NodeList *) m_nodeExpr)->getNumberOfNodes();
		if((n > (u32) arraysize) && (arraysize > 0)) //not an error: t3847
		  {
		    std::ostringstream msg;
		    msg << "Too many initializers (" << n << ") specified for array '";
		    msg << getName() << "', size " << arraysize;
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		    it = Nav;
		  }
		else
		  {
		    m_nodeExpr->setNodeType(suti);
		    it = suti;
		  }
	      }
	    else
	      {
		assert(suti != Nav);
		it = Hzy;
		m_state.setGoAgain();
	      }
	  } //end array initializers (eit == Void)

	if(m_state.okUTItoContinue(it) && (m_state.isScalar(it) ^ m_state.isScalar(suti)))
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", array/scalar mismatch";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit (t3446)
	  }
      } //end node expression

    if(m_state.isComplete(suti)) //reloads
      {
	ULAMTYPE eit = m_state.getUlamTypeByIndex(it)->getUlamTypeEnum();
	ULAMTYPE esuti = m_state.getUlamTypeByIndex(suti)->getUlamTypeEnum();
	if(m_state.okUTItoContinue(it) && (eit != esuti))
	  {
	    std::ostringstream msg;
	    msg << prettyNodeName().c_str() << " '" << getName();
	    msg << "' type <" << m_state.getUlamTypeByIndex(suti)->getUlamTypeNameOnly().c_str();
	    msg << "> does not match its value type <";
	    msg << m_state.getUlamTypeByIndex(it)->getUlamTypeNameOnly().c_str() << ">";
	    msg << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << " UTI" << cuti;
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }

	//Moved: esuti == Void 	    //void only valid use is as a func return type
	// to be more like NodeVarDecl

	//note: Void is flag that it's a list of constant initializers.
	if((eit == Void))
	  {
	    //only possible if array type with initializers
	    //arraysize specified, may have fewer initializers
	    assert(m_state.okUTItoContinue(suti));
	    s32 arraysize = m_state.getArraySize(suti);
	    assert(arraysize >= 0); //t3847
	    u32 n = ((NodeList *) m_nodeExpr)->getNumberOfNodes();
	    if((n > (u32) arraysize) && (arraysize > 0)) //not an error: t3847
	      {
		std::ostringstream msg;
		msg << "Too many initializers (" << n << ") specified for array '";
		msg << getName() << "', size " << arraysize;
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav);
		return Nav;
	      }
	    m_nodeExpr->setNodeType(suti);
	  }
      } //end array initializers (eit == Void)

    setNodeType(suti);

    if(!m_constSymbol->isReady() && m_nodeExpr)
      {
	UTI foldrtn = foldConstantExpression();
	if(foldrtn == Nav)
	  setNodeType(Nav);
	else if(foldrtn == Hzy)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete " << prettyNodeName().c_str() << " for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << ", used with symbol name '" << getName() << "', after folding";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);

	    setNodeType(Hzy);
	    m_state.setGoAgain();
	  }
	else
	  {
	    if(!(m_constSymbol->isReady() || m_constSymbol->isInitValueReady()))
	      {
		std::ostringstream msg;
		msg << "Constant symbol '" << getName() << "' is not ready";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);

		setNodeType(Hzy);
		m_state.setGoAgain();
	      }
	  }
      }
    return getNodeType();
  } //checkAndLabelType

  void NodeConstantDef::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_cid, asymptr, hazyKin))
      {
	if(asymptr->isConstant())
	  {
	    m_constSymbol = (SymbolConstantValue *) asymptr;
	    m_constSymbol->setDeclNodeNo(getNodeNo());
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << "> is not a constant, and cannot be used as one";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) Named Constant <" << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "> is not defined, and cannot be used";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    m_state.popClassContext(); //restore
  } //checkForSymbols

  void NodeConstantDef::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if(m_nodeExpr)
      m_nodeExpr->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);

    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  NNO NodeConstantDef::getBlockNo()
  {
    return m_currBlockNo;
  }

  void NodeConstantDef::setBlockNo(NNO n)
  {
    m_currBlockNo = n;
  }

  NodeBlock * NodeConstantDef::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  void NodeConstantDef::setConstantExpr(Node * node)
  {
    m_nodeExpr = node;
    m_nodeExpr->updateLineage(getNodeNo()); //for unknown subtrees
  }

  bool NodeConstantDef::hasConstantExpr()
  {
    return (m_nodeExpr != NULL);
  }

  // called during parsing rhs of named constant;
  // Requires a constant expression, else error;
  // (scope of eval is based on the block of const def.)
  UTI NodeConstantDef::foldConstantExpression()
  {
    UTI uti = getNodeType();

    if(uti == Nav)
      return Nav;

    if(!m_state.isComplete(uti)) //not complete includes Hzy
      return Hzy; //e.g. not a constant; total word size (below) requires completeness

    assert(m_constSymbol);
    if(m_constSymbol->isReady())
      return uti;

    if(!m_nodeExpr)
      return Nav;

    if(!m_state.isScalar(uti))
      {
	// similar to NodeVarDecl (t3881)
	if(!(m_constSymbol->isReady() || m_constSymbol->isInitValueReady() || foldArrayInitExpression()))
	  {
	    if((getNodeType() == Nav) || m_nodeExpr->getNodeType() == Nav)
	      return Nav;

	    if(!(m_constSymbol->isReady() || m_constSymbol->isInitValueReady()))
	      {
		setNodeType(Hzy);
		m_state.setGoAgain(); //since not error
		return Hzy;
	      }
	  }
	return uti;
      }

    // if here, must be a constant..
    u64 newconst = 0; //UlamType format (not sign extended)

    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(uti); //offset a constant expression

    EvalStatus evs = m_nodeExpr->eval();
    UlamValue cnstUV;
    if( evs == NORMAL)
      cnstUV = m_state.m_nodeEvalStack.popArg();

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
	msg << "', is erronous while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(evs == NOTREADY)
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
	msg << "', is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	return Hzy;
      }

    //t3403, t3490
    //insure constant value fits in its declared type
    // no saturation without an explicit cast.
    FORECAST scr = m_nodeExpr->safeToCastTo(uti);
    if(scr != CAST_CLEAR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << getName() << "' is not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	if(scr == CAST_BAD)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return Nav;
	  }
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	return Hzy; //necessary if not just a warning.
      }

    //cast first, also does safeCast
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    if(ut->cast(cnstUV, uti))
      {
	u32 wordsize = m_state.getTotalWordSize(uti);
	if(wordsize == MAXBITSPERINT)
	  newconst = cnstUV.getImmediateData(m_state);
	else if(wordsize == MAXBITSPERLONG)
	  newconst = cnstUV.getImmediateDataLong(m_state);
	else
	  assert(0);
      }
    else
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << getName() << "', was not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

#if 1
    // BUT WHY when Symbol is all we need/want? because it indicates
    // there's a default value before c&l (see SCNT::getTotalParametersWithDefaultValues) (t3526)
    //then do the surgery
    NodeTerminal * newnode;
    if(m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum() == Int)
      newnode = new NodeTerminal((s64) newconst, uti, m_state);
    else
      newnode = new NodeTerminal(newconst, uti, m_state);
    newnode->setNodeLocation(getNodeLocation());
    delete m_nodeExpr;
    m_nodeExpr = newnode;
#else
    delete m_nodeExpr;
    m_nodeExpr = NULL;
#endif

    BV8K bvtmp;
    u32 len = m_state.getTotalBitSize(uti);
    bvtmp.WriteLong(0u, len, newconst); //is newconst packed?
    if(m_constSymbol->isClassParameter())
      m_constSymbol->setInitValue(bvtmp); //(isInitValueReady now)!
    else
      m_constSymbol->setValue(bvtmp); //isReady now! (e.g. ClassArgument, ModelParameter)
    return uti; //ok
  } //foldConstantExpression

  bool NodeConstantDef::foldArrayInitExpression()
  {
    //for arrays with constant expression initializers(local or dm)
    UTI nuti = getNodeType();
    if(!m_state.okUTItoContinue(nuti) || !m_state.isComplete(nuti))
      return false;

    assert(!m_state.isScalar(nuti));
    //assert(m_nodeExpr && (!m_state.isScalar(m_nodeExpr->getNodeType())); //NodeListArrayInitialization
    assert(m_constSymbol && !(m_constSymbol->isReady() || m_constSymbol->isInitValueReady()));
    assert(m_nodeExpr);

    if(((NodeListArrayInitialization *) m_nodeExpr)->foldArrayInitExpression())
      {
	BV8K bvtmp;
	if(((NodeListArrayInitialization *) m_nodeExpr)->buildArrayValueInitialization(bvtmp))
	  {
	    if(m_constSymbol->isClassParameter())
	      m_constSymbol->setInitValue(bvtmp);
	    else
	      m_constSymbol->setValue(bvtmp); //isReady now! (e.g. ClassArgument, ModelParameter)
	    return true;
	  }
      }
    return false;
  } //foldArrayInitExpression

  bool NodeConstantDef::buildDefaultValue(u32 wlen, BV8K& bvref)
  {
    return true; //pass on
  }

  void NodeConstantDef::genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos)
  {
    return;
  }

  void NodeConstantDef::fixPendingArgumentNode()
  {
    assert(m_constSymbol);
    // for unseen classes that needed their args "fixed" to proper param name
    // this fixes the saved m_cid while clonePendingClassArgumentsForStubClassInstance
    // (the m_cid is used during full instantiation).
    if(m_constSymbol->getId() != getSymbolId())
      {
	m_cid = m_constSymbol->getId();
      }
  } //fixPendingArgumentNode

  bool NodeConstantDef::assignClassArgValueInStubCopy()
  {
    assert(m_nodeExpr);
    return m_nodeExpr->assignClassArgValueInStubCopy();
  }

  EvalStatus NodeConstantDef::eval()
  {
    if(m_constSymbol->isReady())
      return NORMAL;
    return ERROR;
  } //eval

  void NodeConstantDef::packBitsInOrderOfDeclaration(u32& offset)
  {
    //do nothing, but override
  }

  void NodeConstantDef::printUnresolvedVariableDataMembers()
  {
    assert(m_constSymbol);
    UTI it = m_constSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(it))
      {
	std::ostringstream msg;
	msg << "Unresolved type <";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << "> used with constant def symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //compiler counts
      } //not complete
  } //printUnresolvedVariableDataMembers

  void NodeConstantDef::printUnresolvedLocalVariables(u32 fid)
  {
    assert(m_constSymbol);
    UTI it = m_constSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(it))
      {
	// e.g. error/t3298 Int(Fu.sizeof)
	std::ostringstream msg;
	msg << "Unresolved type <";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << "> used with local variable symbol name '" << getName() << "'";
	msg << " in function: " << m_state.m_pool.getDataAsString(fid);
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //compiler counts
      } //not complete
  } //printUnresolvedLocalVariables

  void NodeConstantDef::genCode(File * fp, UVPass& uvpass)
  {
    UTI nuti = getNodeType();
    if(!m_state.isScalar(nuti))
      {
	assert(m_constSymbol);
	assert(m_state.isComplete(nuti));
	assert(m_constSymbol->getUlamTypeIdx() == nuti); //sanity check
	UlamType * nut = m_state.getUlamTypeByIndex(nuti);

	if(m_constSymbol->isLocalFilescopeDef() ||  m_constSymbol->isDataMember())
	  {
	    //as a "data member", or locals filescope, will be initialized in no-arg constructor (non-const)
	    m_state.indentUlamCode(fp);
	    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write(";"); GCNL;
	  }
	else
	  {
	    //immediate use (also, non-const)
	    assert(m_nodeExpr);
	    m_nodeExpr->genCode(fp, uvpass);

	    m_state.indentUlamCode(fp);
	    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write("("); // use constructor (not equals)
	    fp->write(m_state.getTmpVarAsString(nuti, uvpass.getPassVarNum(), uvpass.getPassStorage()).c_str()); //VALUE
	    fp->write(");"); GCNL;
	    m_state.clearCurrentObjSymbolsForCodeGen();
	  }
      }
    return; //done
  } //genCode

  void NodeConstantDef::genCodeConstantArrayInitialization(File * fp)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(nut->isScalar())
      return;

    //no-arg constructor to initialize const arrays using special method
    // (based on: 'Init' + constants's mangled name)
    m_state.m_currentIndentLevel+=2;
    m_state.indent(fp);
    fp->write(",");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("(Init");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("())\n");
    m_state.m_currentIndentLevel-=2;
  } //genCodeConstantArrayInitialization

  void NodeConstantDef::generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(nut->isScalar())
      return;

    if(declOnly)
      {
	//unique typedef for this constant array (number of u32's)
	m_state.indent(fp);
	fp->write("typedef u32 TypeForInit");
	fp->write(m_constSymbol->getMangledName().c_str());
	fp->write("[");
	fp->write_decimal_unsigned(nut->getTotalNumberOfWords());
	fp->write("];\n");

	//unique function to initialize const array "data members" in class no-arg constructor
	m_state.indent(fp);
	fp->write("TypeForInit");
	fp->write(m_constSymbol->getMangledName().c_str());
	fp->write("& Init");
	fp->write(m_constSymbol->getMangledName().c_str());
	fp->write("();"); GCNL;
	fp->write("\n");
	return;
      }

    UTI cuti = m_state.getCompileThisIdx();
    //include the mangled class::
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    //    bool isLocalsFileScope = m_state.isLocalsFileScope();

    //unique function to initialize const array "data members" in class no-arg constructor
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("typename ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write("TypeForInit");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("& ");

    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write("Init");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("()\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("static ");
    fp->write("u32 ");
    fp->write("initVal[");
    fp->write_decimal_unsigned(nut->getTotalNumberOfWords());
    fp->write("] = ");
    m_constSymbol->printPostfixValue(fp);
    fp->write(";\n");

    m_state.indent(fp);
    fp->write("return initVal;\n");

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //Init");
    fp->write(m_constSymbol->getMangledName().c_str()); GCNL;
    fp->write("\n");
  } //generateBuiltinConstantArrayInitializationFunction

  void NodeConstantDef::cloneAndAppendNode(std::vector<Node *> & cloneVec)
  {
    if(!m_state.isScalar(getNodeType()))
      {
	NodeConstantDef * cloneofme = (NodeConstantDef *) this->instantiate();
	assert(cloneofme);
	SymbolConstantValue * csymptr = NULL;
	AssertBool isSym = this->getSymbolPtr((Symbol *&) csymptr);
	assert(isSym);
	((NodeConstantDef *) cloneofme)->setSymbolPtr(csymptr); //another ptr to same symbol
	cloneVec.push_back(cloneofme);
      }
  }

  void NodeConstantDef::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {}

} //end MFM
