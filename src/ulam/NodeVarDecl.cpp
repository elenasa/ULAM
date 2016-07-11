#include <stdlib.h>
#include "NodeVarDecl.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"
#include "NodeListArrayInitialization.h"
#include "NodeVarRef.h"

namespace MFM {

  NodeVarDecl::NodeVarDecl(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_varSymbol(sym), m_vid(0), m_nodeInitExpr(NULL), m_currBlockNo(0), m_nodeTypeDesc(nodetype)
  {
    if(sym)
      {
	m_vid = sym->getId();
	m_currBlockNo = sym->getBlockNoOfST();
      }
  }

  NodeVarDecl::NodeVarDecl(const NodeVarDecl& ref) : Node(ref), m_varSymbol(NULL), m_vid(ref.m_vid), m_nodeInitExpr(NULL), m_currBlockNo(ref.m_currBlockNo), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();

    if(ref.m_nodeInitExpr)
      m_nodeInitExpr = (Node *) ref.m_nodeInitExpr->instantiate();
  }

  NodeVarDecl::~NodeVarDecl()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;

    delete m_nodeInitExpr;
    m_nodeInitExpr = NULL;
  }

  Node * NodeVarDecl::instantiate()
  {
    return new NodeVarDecl(*this);
  }

  void NodeVarDecl::updateLineage(NNO pno)
  {
    Node::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
    if(m_nodeInitExpr)
      m_nodeInitExpr->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeVarDecl::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeInitExpr == oldnptr)
      {
	m_nodeInitExpr = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeVarDecl::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    if(m_nodeInitExpr && m_nodeInitExpr->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeVarDecl::checkAbstractInstanceErrors()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    if(nut->getUlamTypeEnum() == Class)
      {
	UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(scalaruti, csym);
	assert(isDefined);
	if(csym->isAbstract())
	  {
	    std::ostringstream msg;
	    msg << "Instance of Abstract Class ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " used with variable symbol name '" << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	  }
      }
  } //checkAbstractInstanceErrors

  //see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarDecl::printPostfix(File * fp)
  {
    printTypeAndName(fp);
    if(m_nodeInitExpr)
      {
	fp->write(" =");
	m_nodeInitExpr->printPostfix(fp);
      }
    fp->write("; ");
  } //printPostfix

  void NodeVarDecl::printTypeAndName(File * fp)
  {
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(vuti);
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    fp->write(" ");
    if(vut->getUlamTypeEnum() != Class)
      fp->write(vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
    else
      fp->write(vut->getUlamTypeNameBrief().c_str());

    fp->write(" ");
    fp->write(getName());

    s32 arraysize = m_state.getArraySize(vuti);
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }
  } //printTypeAndName

  const char * NodeVarDecl::getName()
  {
    //return m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str();
    return m_state.m_pool.getDataAsString(m_vid).c_str();
  }

  const std::string NodeVarDecl::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeVarDecl::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return (m_varSymbol != NULL);
  }

  void NodeVarDecl::setInitExpr(Node * node)
  {
    assert(node);
    m_nodeInitExpr = node;
    m_nodeInitExpr->updateLineage(getNodeNo()); //for unknown subtrees
  }

  bool NodeVarDecl::hasInitExpr()
  {
    return (m_nodeInitExpr != NULL);
  }

  bool NodeVarDecl::foldInitExpression()
  {
    //for arrays with constant expression initializers(local or dm)
    UTI nuti = getNodeType();
    if(!m_state.okUTItoContinue(nuti) || !m_state.isComplete(nuti))
      return false;

    assert(!m_state.isScalar(nuti));
    assert(m_nodeInitExpr); //NodeListArrayInitialization
    assert(m_varSymbol && !(m_varSymbol->isInitValueReady()));

    if(((NodeListArrayInitialization *) m_nodeInitExpr)->foldInitExpression())
      {
	BV8K bvtmp;
	if(((NodeListArrayInitialization *) m_nodeInitExpr)->buildArrayValueInitialization(bvtmp))
	  {
	    m_varSymbol->setInitValue(bvtmp);
	    return true;
	  }
      }
    return false;
  } //foldInitExpression

  FORECAST NodeVarDecl::safeToCastTo(UTI newType)
  {
    assert(m_nodeInitExpr);

    FORECAST rscr = CAST_CLEAR;
    UTI nuti = getNodeType();
    //cast RHS if necessary and safe
    if(UlamType::compare(nuti, newType, m_state) != UTIC_SAME)
      {
	rscr = m_nodeInitExpr->safeToCastTo(nuti); //but we're the new type!
	if(rscr != CAST_CLEAR)
	  {
	    if(m_state.isAtom(nuti))
	      {
		UlamType * newt = m_state.getUlamTypeByIndex(newType);
		UlamType * nut = m_state.getUlamTypeByIndex(nuti);
		std::ostringstream msg;
		msg << "Atom variable " << getName() << "'s type ";
		msg << nut->getUlamTypeNameBrief().c_str();
		msg << ", and its initial value type ";
		msg << newt->getUlamTypeNameBrief().c_str();
		msg << ", are incompatible";
		if(newt->isReference() && newt->getUlamClassType() == UC_QUARK)
		  msg << "; .atomof may help";
		if(rscr == CAST_HAZY)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    else
	      {
		std::ostringstream msg;
		if(m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum() == Bool)
		  msg << "Use a comparison operator";
		else
		  msg << "Use explicit cast";
		msg << " to convert "; // the real converting-message
		msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
		msg << " to ";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		msg << " for variable initialization";
		if(rscr == CAST_BAD)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	  }
	else
	  {
	    //safe to cast, but can't be reference
	    if(m_nodeInitExpr->isExplicitReferenceCast())
	      {
		std::ostringstream msg;
		msg << "Explicit Reference cast for variable '";
		msg << m_state.m_pool.getDataAsString(m_vid).c_str();
		msg << "' initialization is invalid";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		rscr = CAST_BAD;
	      }
	    else if(!Node::makeCastingNode(m_nodeInitExpr, nuti, m_nodeInitExpr))
	      rscr = CAST_BAD; //error
	  } //safe cast
      }
    //else same type, clear to cast
    return rscr;
  } //safeToCastTo (virtual)

  bool NodeVarDecl::checkSafeToCastTo(UTI fromType, UTI& newType)
  {
    if(!m_state.isComplete(fromType) || !m_state.isComplete(newType)) //e.g. t3753
      {
	m_state.setGoAgain();
	newType = Hzy;
	return false;
      }

    if(UlamType::compare(fromType, newType, m_state) == UTIC_SAME)
      return true;

    bool rtnOK = true;
    FORECAST scr = safeToCastTo(fromType); //reversed
    if(scr == CAST_HAZY)
      {
	m_state.setGoAgain();
	newType = Hzy;
	rtnOK = false;
      }
    else if(scr == CAST_BAD)
      {
	newType = Nav;
	rtnOK = false;
      }
    return rtnOK;
  } //checkSafeToCastTo

  bool NodeVarDecl::checkReferenceCompatibility(UTI uti)
  {
    assert(m_state.okUTItoContinue(uti));
    if(m_state.getUlamTypeByIndex(uti)->isReference())
      {
	UTI cuti = m_state.getCompileThisIdx();
	std::ostringstream msg;
	msg << "Variable '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "', is a reference -- do surgery";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

	// replace ourselves with a ref node instead;
	// same node no, and loc (e.g. t3666,t3669, t3670-3, t3819)
	NodeVarRef * newnode = new NodeVarRef(m_varSymbol, NULL, m_state);
	assert(newnode);

	NNO pno = Node::getYourParentNo();
	Node * parentNode = m_state.findNodeNoInThisClass(pno);
	if(!parentNode)
	  {
	    std::ostringstream msg;
	    msg << "Reference variable '" << getName();
	    msg << "' cannot be exchanged at this time while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << " Parent required";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    assert(0); //parent required
	  }

	AssertBool swapOk = parentNode->exchangeKids(this, newnode);
	assert(swapOk);

	{
	  std::ostringstream msg;
	  msg << "Exchanged kids! <" << m_state.m_pool.getDataAsString(m_vid).c_str();
	  msg << "> a reference variable, in place of a variable within class: ";
	  msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}

	if(m_nodeTypeDesc)
	  newnode->m_nodeTypeDesc = (NodeTypeDescriptor *) m_nodeTypeDesc->instantiate();

	if(m_nodeInitExpr)
	  newnode->m_nodeInitExpr = (Node *) m_nodeInitExpr->instantiate();

	newnode->setNodeLocation(getNodeLocation());
	newnode->setYourParentNo(pno); //missing?
	newnode->resetNodeNo(getNodeNo()); //missing?

	delete this; //suicide is painless..

	// we must be the last thing called by checkandlabel
	// to return properly to our parent
	return newnode->checkAndLabelType();
      }
    return true; //ok
  } //checkReferenceCompatibility

UTI NodeVarDecl::checkAndLabelType()
  {
    UTI it = getNodeType();

    // instantiate, look up in current block
    if(m_varSymbol == NULL)
      checkForSymbol();

    if(m_varSymbol)
      {
	it = m_varSymbol->getUlamTypeIdx(); //base type has arraysize

	UTI cuti = m_state.getCompileThisIdx();
	if(m_nodeTypeDesc)
	  {
	    UTI duti = m_nodeTypeDesc->checkAndLabelType(); //sets goagain
	    if(m_state.okUTItoContinue(duti) && (duti != it))
	      {
		std::ostringstream msg;
		msg << "REPLACING Symbol UTI" << it;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << " used with variable symbol name '" << getName();
		msg << "' with node type descriptor type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(duti).c_str();
		msg << " UTI" << duti << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		m_varSymbol->resetUlamType(duti); //consistent!
		m_state.mapTypesInCurrentClass(it, duti);
		//m_state.updateUTIAliasForced(it, duti); //help?NOPE, t3808
		it = duti;
	      }
	  }

	if(!m_state.okUTItoContinue(it) || !m_state.isComplete(it))
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Variable Decl for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", used with variable symbol name '" << getName() << "'";
	    if(m_state.okUTItoContinue(it) || (it == Hzy))
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		it = Hzy;
		//m_state.setGoAgain(); //since not error; wait until not Nav
	      }
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      } //end var_symbol

    ULAMTYPE etyp = m_state.getUlamTypeByIndex(it)->getUlamTypeEnum();
    if(etyp == Void)
      {
	//void only valid use is as a func return type
	std::ostringstream msg;
	msg << "Invalid use of type ";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << " with variable symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	it = Nav;
      }

    if(m_nodeInitExpr)
      {
	UTI eit = m_nodeInitExpr->checkAndLabelType();
	if(eit == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Initial value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is invalid";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	if(eit == Hzy)
	  {
	    std::ostringstream msg;
	    msg << "Initial value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is not ready";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    m_state.setGoAgain(); //since not error
	    setNodeType(Hzy);
	    return Hzy; //short-circuit
	  }

	//note: Void is flag that it's a list of constant initializers.
	if((eit == Void))
	  {
	    //only possible if array type with initializers
	    m_varSymbol->setHasInitValue(); //might not be ready yet

	    if(!m_state.okUTItoContinue(it) && m_nodeTypeDesc)
	      {
		UTI duti = m_nodeTypeDesc->getNodeType();
		UlamType * dut = m_state.getUlamTypeByIndex(duti);
		if(m_state.okUTItoContinue(duti) && !dut->isComplete())
		  {
		    assert(!dut->isScalar());
		    assert(dut->isPrimitiveType());

		    //if here, assume arraysize depends on number of initializers
		    s32 bitsize = dut->getBitSize();
		    u32 n = ((NodeList *) m_nodeInitExpr)->getNumberOfNodes();
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
			m_varSymbol->resetUlamType(duti); //consistent!
			m_state.mapTypesInCurrentClass(it, duti);
			m_state.updateUTIAliasForced(it, duti); //help?
			m_nodeInitExpr->setNodeType(duti); //replace Void too!
			it = duti;
		      }
		  }
		//else
	      }
	    else
	      {
		//arraysize specified, may have fewer initializers
		assert(m_state.okUTItoContinue(it));
		s32 arraysize = m_state.getArraySize(it);
		assert(arraysize >= 0); //t3847
		u32 n = ((NodeList *) m_nodeInitExpr)->getNumberOfNodes();
		if((n > (u32) arraysize) && (arraysize > 0)) //not an error: t3847
		  {
		    std::ostringstream msg;
		    msg << "Too many initializers (" << n << ") specified for array '";
		    msg << getName() << "', size " << arraysize;
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		  }
		else
		  m_nodeInitExpr->setNodeType(it);
	      }
	    eit = it;
	  } //end array initializers (eit == Void)

	setNodeType(it); //needed before safeToCast, and folding

	if(!m_state.isScalar(eit))
	  {
	    if(m_state.okUTItoContinue(eit) && m_state.isComplete(eit))
	      {
		assert(m_varSymbol);
		//constant fold if possible, set symbol value
		if(m_varSymbol->hasInitValue())
		  {
		    //possibly move this to NodeListArrayInitialization
		    u32 bitsize = m_state.getBitSize(eit);
		    if(bitsize > MAXBITSPERLONG)
		      {
			std::ostringstream msg;
			msg << "Constant value expression for array: ";
			msg << m_state.m_pool.getDataAsString(m_vid).c_str();
			msg << ", initialization is currently limited to ";
			msg << MAXBITSPERLONG << " bits";
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			setNodeType(Nav);
			return Nav; //short-circuit
		      }

		    if(!(m_varSymbol->isInitValueReady()))
		      {
			if(!foldInitExpression()) //sets init constant value
			  {
			    if((getNodeType() == Nav) || m_nodeInitExpr->getNodeType() == Nav)
			      return Nav;

			    if(!(m_varSymbol->isInitValueReady()))
			      {
				setNodeType(Hzy);
				m_state.setGoAgain(); //since not error
				return Hzy;
			      }
			  }
		      }
		  }
	      }
	  } //array initialization

	if(m_state.okUTItoContinue(eit) && m_state.okUTItoContinue(it))
	  checkSafeToCastTo(eit, it); //may side-effect 'it'
      }

    Node::setStoreIntoAble(TBOOL_TRUE);
    setNodeType(it);

    if(it == Hzy)
      m_state.setGoAgain(); //since not error

    //checkReferenceCompatibilty must be called at the end since
    // NodeVarDecl may do surgery on itself
    if(m_state.okUTItoContinue(it) && !checkReferenceCompatibility(it))
      {
	it = Nav; //err msg by checkReferenceCompatibility
	setNodeType(Nav); //failed
      }
    return it; //in case of surgery don't call getNodeType();
  } //checkAndLabelType

  void NodeVarDecl::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_vid, asymptr, hazyKin) && !hazyKin)
      {
	if(!asymptr->isTypedef() && !asymptr->isConstant() && !asymptr->isModelParameter() && !asymptr->isFunction())
	  {
	    m_varSymbol = (SymbolVariable *) asymptr;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << "> is not a variable, and cannot be used as one";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) Variable <" << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "> is not defined, and cannot be used";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      } //alreadyDefined

    m_state.popClassContext(); //restore
  } //checkForSymbol

  NNO NodeVarDecl::getBlockNo()
  {
    return m_currBlockNo;
  }

  NodeBlock * NodeVarDecl::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  void NodeVarDecl::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    assert(m_varSymbol);
    s32 newslot = depth + base;
    s32 oldslot = ((SymbolVariable *) m_varSymbol)->getStackFrameSlotIndex();
    if(oldslot != newslot)
      {
	std::ostringstream msg;
	msg << "'" << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "' was at slot: " << oldslot << ", new slot is " << newslot;
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	((SymbolVariable *) m_varSymbol)->setStackFrameSlotIndex(newslot);
      }
    depth += m_state.slotsNeeded(getNodeType());

    if(m_nodeInitExpr)
      m_nodeInitExpr->calcMaxDepth(depth, maxdepth, base);
  } //calcMaxDepth

  void NodeVarDecl::printUnresolvedLocalVariables(u32 fid)
  {
    assert(m_varSymbol);
    UTI it = m_varSymbol->getUlamTypeIdx();
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

  void NodeVarDecl::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);

    if(m_nodeInitExpr)
      m_nodeInitExpr->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);

    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  EvalStatus NodeVarDecl::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    u32 len = nut->getTotalBitSize();

    if((classtype == UC_TRANSIENT) && (len > MAXSTATEBITS))
      return UNEVALUABLE;

    assert(m_varSymbol->getUlamTypeIdx() == nuti); //is it so? if so, some cleanup needed

    assert(!m_varSymbol->isAutoLocal()); //NodeVarRef::eval

    assert(!m_varSymbol->isDataMember()); //see NodeVarDeclDM

    u32 slots = Node::makeRoomForNodeType(nuti, STACK);

    if(m_state.isAtom(nuti))
      {
	UlamValue atomUV = UlamValue::makeAtom(m_varSymbol->getUlamTypeIdx());
	//scalar or UNPACKED array of atoms (t3709)
	u32 baseslot =  ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex();
	for(u32 j = 0; j < slots; j++)
	  m_state.m_funcCallStack.storeUlamValueInSlot(atomUV, baseslot + j);
      }
    else if(classtype == UC_NOTACLASS)
      {
	//local variable to a function;
	// t.f. must be SymbolVariableStack, not SymbolVariableDataMember
	setupStackWithPrimitiveForEval(slots);
      }
    else if((classtype == UC_ELEMENT) || (classtype == UC_TRANSIENT))
      setupStackWithClassForEval(slots);
    else
      setupStackWithQuarkForEval(slots);

    if(m_nodeInitExpr)
      return evalInitExpr();

    return NORMAL;
  } //eval

  void NodeVarDecl::setupStackWithPrimitiveForEval(u32 slots)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    assert(m_varSymbol->getUlamTypeIdx() == nuti);
    PACKFIT packFit = nut->getPackable();
    if(packFit == PACKEDLOADABLE)
      {
	u64 dval = 0;
	if(m_varSymbol->hasInitValue())
	  {
	    AssertBool gotInitVal = m_varSymbol->getInitValue(dval);
	    assert(gotInitVal);
	  }

	UlamValue immUV;
	u32 len = nut->getTotalBitSize();
	if(len <= MAXBITSPERINT)
	  immUV = UlamValue::makeImmediate(nuti, (u32) dval, m_state);
	else if(len <= MAXBITSPERLONG)
	  immUV = UlamValue::makeImmediateLong(nuti, dval, m_state);
	else
	  assert(0);
	//immUV = UlamValue::makePtr(((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex(), STACK, nuti, m_state.determinePackable(nuti), m_state, 0, m_varSymbol->getId()); //array ptr
	m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
      }
    else
      {
	//unpacked primitive array - uses eval
	UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	bool hasInitVal = m_varSymbol->hasInitValue();
	u32 baseslot =  ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex();

	UlamValue immUV;
	u32 itemlen = nut->getBitSize();
	if(itemlen <= MAXBITSPERINT)
	  immUV = UlamValue::makeImmediate(scalaruti, 0, m_state);
	else if(itemlen <= MAXBITSPERLONG)
	  immUV = UlamValue::makeImmediateLong(scalaruti, 0, m_state);
	else
	  assert(0);

	u32 n = slots;
	if(hasInitVal)
	  n = ((NodeList *) m_nodeInitExpr)->getNumberOfNodes(); //may be fewer

	for(u32 j = 0; j < slots; j++)
	  {
	    UlamValue itemUV;
	    if(hasInitVal)
	      {
		evalNodeProlog(0); //new current frame pointer
		makeRoomForNodeType(scalaruti); //offset a constant expression
		u32 k = j < n ? j : n - 1; //repeat last initializer if fewer
		EvalStatus evs = ((NodeListArrayInitialization *) m_nodeInitExpr)->eval(k);
		if(evs == NORMAL)
		  {
		    itemUV = m_state.m_nodeEvalStack.popArg();
		  }
		else
		  assert(0); //error msg?
		evalNodeEpilog();
	      }
	    else
	      itemUV = immUV;
	    m_state.m_funcCallStack.storeUlamValueInSlot(itemUV, baseslot + j);
	  }
      }
  } //setupStackWithPrimitiveForEval

  void NodeVarDecl::setupStackWithClassForEval(u32 slots)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    //for eval purposes, a transient must fit into atom state bits, like an element
    // any class may be a data member (see NodeVarDeclDM)
    if(nut->isScalar())
      {
	UlamValue atomUV = UlamValue::makeDefaultAtom(m_varSymbol->getUlamTypeIdx(), m_state);
	m_state.m_funcCallStack.storeUlamValueInSlot(atomUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
      }
    else
      {
	PACKFIT packFit = nut->getPackable();
	if(packFit == PACKEDLOADABLE)
	  {
	    u32 len = nut->getTotalBitSize();
	    u64 dval = 0;
	    AssertBool isPackLoadableScalar = m_state.getPackedDefaultClass(nuti, dval);
	    assert(isPackLoadableScalar);
	    u64 darrval = 0;
	    m_state.getDefaultAsPackedArray(nuti, dval, darrval); //3rd arg ref

	    if(len <= MAXBITSPERINT)
	      {
		UlamValue immUV = UlamValue::makeImmediateClass(nuti, (u32) darrval, len);
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
	      }
	    else if(len <= MAXBITSPERLONG) //t3710
	      {
		UlamValue immUV = UlamValue::makeImmediateLongClass(nuti, darrval, len);
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
	      }
	    else
	      assert(0); //not write load packable!
	  }
	else
	  {
	    //UNPACKED element array
	    UlamValue atomUV = UlamValue::makeDefaultAtom(m_varSymbol->getUlamTypeIdx(), m_state);
	    u32 baseslot =  ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex();
	    for(u32 j = 0; j < slots; j++)
	      {
		m_state.m_funcCallStack.storeUlamValueInSlot(atomUV, baseslot + j);
	      }
	  }
      }
  } //setupStackWithClassForEval

  void NodeVarDecl::setupStackWithQuarkForEval(u32 slots)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    u32 len = nut->getTotalBitSize();

    //must be a local quark! could be an array of them!!
    u32 dq = 0;
    AssertBool isDefinedQuark = m_state.getDefaultQuark(nuti, dq); //returns scalar dq
    assert(isDefinedQuark);
    if(nut->isScalar())
      {
	UlamValue immUV = UlamValue::makeImmediateClass(nuti, dq, len);
	m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
      }
    else
      {
	PACKFIT packFit = nut->getPackable();
	if(packFit == PACKEDLOADABLE)
	  {
	    u64 darrval = 0;
	    m_state.getDefaultAsPackedArray(nuti, (u64) dq, darrval); //3rd arg ref
	    if(len <= MAXBITSPERINT) //t3706
	      {
		UlamValue immUV = UlamValue::makeImmediateClass(nuti, (u32) darrval, len);
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
	      }
	    else if(len <= MAXBITSPERLONG) //t3708
	      {
		UlamValue immUV = UlamValue::makeImmediateLongClass(nuti, darrval, len);
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
	      }
	  }
	else
	  {
	    //UNPACKED array of quarks! t3649, t3707
	    UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	    UlamValue immUV = UlamValue::makeImmediateClass(scalaruti, dq, nut->getBitSize());
	    u32 baseslot =  ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex();
	    for(u32 j = 0; j < slots; j++)
	      {
		m_state.m_funcCallStack.storeUlamValueInSlot(immUV, baseslot + j);
	      }
	  }
      }
  } //setupStackWithQuarkForEval

  EvalStatus NodeVarDecl::evalInitExpr()
  {
    //also called by NodeVarDecDM for data members with initial constant values (t3514);
    // don't call m_nodeInitExpr->eval(), if constant initialized array (e.g.t3768, t3769);
    if(m_varSymbol->hasInitValue() && !m_state.isScalar(getNodeType()))
      {
	return NORMAL;
      }

    EvalStatus evs = NORMAL; //init
    // quark or non-class data member;
    evalNodeProlog(0); //new current node eval frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr

    evs = evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1);
    UTI nuti = getNodeType();
    u32 slots = makeRoomForNodeType(nuti);

    evs = m_nodeInitExpr->eval();

    if(evs == NORMAL)
      {
	if(slots == 1)
	  {
	    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slots+1); //immediate scalar
	    if(m_state.isScalar(nuti))
	      {
		m_state.assignValue(pluv,ruv);
		//also copy result UV to stack, -1 relative to current frame pointer
		Node::assignReturnValueToStack(ruv); //not when unpacked? how come?
	      }
	    else
	      {
		//(do same as scalar) t3419. t3425, t3708
		m_state.assignValue(pluv,ruv);
		Node::assignReturnValueToStack(ruv);
	      }
	  }
	else //unpacked
	  {
	    //t3704, t3706, t3707, t3709
	    UlamValue scalarPtr = UlamValue::makeScalarPtr(pluv, m_state);
	    for(u32 j = 0; j < slots; j++)
	      {
		UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1+1+j); //immediate scalar
		m_state.assignValue(scalarPtr,ruv);
		scalarPtr.incrementPtr(m_state); //by one.
	      }
	  }
      } //normal

    evalNodeEpilog();
    return evs;
  } //evalInitExpr

  EvalStatus NodeVarDecl::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    u32 len = nut->getTotalBitSize();

    if((classtype == UC_TRANSIENT) && (len > MAXSTATEBITS))
      return UNEVALUABLE;

    evalNodeProlog(0); //new current node eval frame pointer

    // return ptr to this local var (from NodeIdent's makeUlamValuePtr)
    UlamValue rtnUVPtr = makeUlamValuePtr();

    u32 absslot = m_state.m_funcCallStack.getAbsoluteStackIndexOfSlot(rtnUVPtr.getPtrSlotIndex());
    rtnUVPtr.setPtrSlotIndex(absslot);
    rtnUVPtr.setUlamValueTypeIdx(PtrAbs);

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeVarDecl::makeUlamValuePtr()
  {
    assert(!m_varSymbol->isSuper());
    // (from NodeIdent's makeUlamValuePtr)
    UlamValue ptr;
    if(m_varSymbol->isSelf())
      {
	//when "self/atom" is a quark, we're inside a func called on a quark (e.g. dm or local)
	//'atom' gets entire atom/element containing this quark; including its type!
	//'self' gets type/pos/len of the quark from which 'atom' can be extracted
	UlamValue selfuvp = m_state.m_currentSelfPtr;
	UTI ttype = selfuvp.getPtrTargetType();
	assert(m_state.okUTItoContinue(ttype));
	if((m_state.getUlamTypeByIndex(ttype)->getUlamClassType() == UC_QUARK))
	  {
	    if(m_varSymbol->isSelf())
	      selfuvp = m_state.getAtomPtrFromSelfPtr();
	    //else
	  }
	return selfuvp;
      } //done

    assert(!m_varSymbol->isAutoLocal()); //nodevarref, not here!
    assert(!m_varSymbol->isDataMember());
    //local variable on the stack; could be array ptr!
    ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
    return ptr;
  } //makeUlamValuePtr

  // parse tree in order declared, unlike the ST.
  void NodeVarDecl::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_varSymbol);
    assert(m_state.isComplete(getNodeType()));

    assert(!m_varSymbol->isDataMember()); //NodeVarDeclDM::genCode
    assert(!m_varSymbol->isAutoLocal()); //NodeVarRef::genCode

    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();

    if(m_nodeInitExpr)
      {
	m_nodeInitExpr->genCode(fp, uvpass);

	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write("("); // use constructor (not equals)
	fp->write(m_state.getTmpVarAsString(vuti, uvpass.getPassVarNum(), uvpass.getPassStorage()).c_str()); //VALUE
	if(m_state.isAtomRef(vuti))
	  fp->write(", uc");
	fp->write(")");

	fp->write(";"); GCNL; //func call args aren't NodeVarDecl's
	m_state.clearCurrentObjSymbolsForCodeGen();
	return; //done
      }

    //initialize T to default atom (might need "OurAtom" if data member ?)
    if(vclasstype == UC_ELEMENT)
      {
	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str());
	if(vut->isScalar() && m_nodeInitExpr) //default constructor includes a GetDefaultAtom
	  {
	    fp->write("(");
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(vuti).c_str());
	    fp->write(".GetDefaultAtom())"); //returns object of type T
	  }
	//else
      }
    else if(vclasstype == UC_QUARK)
      {
	//left-justified. no initialization
	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str());
      }
    else
      {
	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str()); //default 0
      }
    fp->write(";"); GCNL; //func call args aren't NodeVarDecl's
    m_state.clearCurrentObjSymbolsForCodeGen();
  } //genCode

  void NodeVarDecl::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    assert(0); //see NodeVarDeclDM data members only
  } //generateUlamClassInfo

} //end MFM
