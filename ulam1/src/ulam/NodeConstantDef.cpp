#include <stdlib.h>
#include "NodeConstantDef.h"
#include "NodeConstant.h"
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
    //like SymbolConstantValue
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
    return true;
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
	    suti = duti;
	  }
      }

    // NOASSIGN REQUIRED (e.g. for class parameters) doesn't have to have this!
    if(m_nodeExpr)
      {
	//check constant before check-and-label to avoid handling not-ready-type Nav's
	if(!m_nodeExpr->isAConstant())
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is not a constant";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

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
	    std::ostringstream msg;
	    msg << "Constant value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is not ready";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //possibly still hazy
	    m_state.setGoAgain();
	    setNodeType(Hzy);
	    return Hzy; //short-circuit
	  }
      } //end node expression

    if(!m_state.isComplete(suti)) //reloads
      {
	std::ostringstream msg;
	msg << "Incomplete " << prettyNodeName().c_str() << " for type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	msg << " used with symbol name '" << getName();
	msg << "' UTI" << suti << " while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	msg << " UTI" << cuti;
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	//too soon! m_state.setGoAgain(); //might not have nodetypedesc

	UTI mappedUTI = Nouti;
	if(m_state.mappedIncompleteUTI(cuti, suti, mappedUTI))
	  {
	    std::ostringstream msg;
	    msg << "Substituting Mapped UTI" << mappedUTI;
	    msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
	    msg << " for incomplete list type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << "' UTI" << suti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.mapTypesInCurrentClass(suti, mappedUTI); //before setting equal?
	    m_constSymbol->resetUlamType(mappedUTI); //consistent!
	    suti = mappedUTI;
	  }

	if(!m_state.isComplete(suti)) //reloads to recheck for debug message
	  {
	    std::ostringstream msg;
	    msg << "Incomplete identifier for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << " used with symbol name '" << getName();
	    msg << "' UTI" << suti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }
    else
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

	if(esuti == Void)
	  {
	    //void only valid use is as a func return type
	    std::ostringstream msg;
	    msg << "Invalid use of type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << " with symbol name '" << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    suti = Nav;
	  }
      }

    setNodeType(suti);

    if(!(m_constSymbol->isReady()))
      {
	UTI foldrtn = foldConstantExpression();
	if(foldrtn == Nav)
	  setNodeType(Nav);
	else if(foldrtn == Hzy)
	  {
	    setNodeType(Hzy);
	    m_state.setGoAgain();
	  }
	else
	  {
	    if(!(m_constSymbol->isReady() || m_constSymbol->hasDefault()))
	      {
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
	msg << "' is erronous while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(evs == NOTREADY)
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
	msg << "' is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
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
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
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
	msg << getName() << "' was not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    //then do the surgery
    NodeTerminal * newnode;
    if(m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum() == Int)
      newnode = new NodeTerminal((s64) newconst, uti, m_state);
    else
      newnode = new NodeTerminal(newconst, uti, m_state);
    newnode->setNodeLocation(getNodeLocation());
    delete m_nodeExpr;
    m_nodeExpr = newnode;

    if(m_constSymbol->isParameter())
      m_constSymbol->setDefaultValue(newconst); //hasDefault (not isReady)!
    else
      m_constSymbol->setValue(newconst); //isReady now!
    return uti; //ok
  } //foldConstantExpression

  bool NodeConstantDef::buildDefaultQuarkValue(u32& dqref)
  {
    return true; //pass on
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

  void NodeConstantDef::genCode(File * fp, UlamValue& uvpass)
  {}

  void NodeConstantDef::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {}

} //end MFM
