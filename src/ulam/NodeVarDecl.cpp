#include <stdlib.h>
#include "NodeVarDecl.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"
//#include "NodeConstantArray.h"
//#include "NodeConstantClassArray.h"
//#include "NodeListArrayInitialization.h"
//#include "NodeMemberSelect.h"
//#include "NodeSquareBracket.h"
#include "NodeVarRef.h"

namespace MFM {

  NodeVarDecl::NodeVarDecl(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_varSymbol(sym), m_vid(0), m_nodeInitExpr(NULL), m_nodeTypeDesc(nodetype), m_currBlockNo(0), m_currBlockPtr(NULL)
  {
    if(sym)
      {
	m_vid = sym->getId();
	m_currBlockNo = sym->getBlockNoOfST();
	sym->setDeclNodeNo(getNodeNo());
      }
  }

  NodeVarDecl::NodeVarDecl(const NodeVarDecl& ref) : Node(ref), m_varSymbol(NULL), m_vid(ref.m_vid), m_nodeInitExpr(NULL), m_nodeTypeDesc(NULL), m_currBlockNo(ref.m_currBlockNo), m_currBlockPtr(NULL)
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
    if(hasInitExpr())
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

  void NodeVarDecl::resetNodeNo(NNO no) //for constant folding
  {
    Node::resetNodeNo(no);
    if(m_varSymbol)
      m_varSymbol->setDeclNodeNo(no);
  }

  bool NodeVarDecl::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    if(hasInitExpr() && m_nodeInitExpr->findNodeNo(n, foundNode))
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

	    csym->notePureFunctionSignatures(); //t3607, t3609
	  }
      }
  } //checkAbstractInstanceErrors

  //see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarDecl::printPostfix(File * fp)
  {
    printTypeAndName(fp);
    if(hasInitExpr())
      {
	fp->write(" =");
	m_nodeInitExpr->printPostfix(fp);
      }
    fp->write("; ");
  } //printPostfix

  void NodeVarDecl::printTypeAndName(File * fp)
  {
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    UlamKeyTypeSignature vkey = vut->getUlamKeyTypeSignature();

    fp->write(" ");
    if(vut->getUlamTypeEnum() != Class)
      fp->write(vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
    else
      fp->write(vut->getUlamTypeClassNameBrief(vuti).c_str());

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
    return m_state.m_pool.getDataAsString(m_vid).c_str();
  }

  u32 NodeVarDecl::getTypeNameId()
  {
    if(m_nodeTypeDesc)
      return m_nodeTypeDesc->getTypeNameId();

    UTI nuti = getNodeType();
    assert(m_state.okUTItoContinue(nuti));
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    //skip bitsize if default size
    if(nut->getBitSize() == ULAMTYPE_DEFAULTBITSIZE[nut->getUlamTypeEnum()])
      return m_state.m_pool.getIndexForDataString(nut->getUlamTypeNameOnly());
    return m_state.m_pool.getIndexForDataString(m_state.getUlamTypeNameBriefByIndex(nuti));
  } //getTypeNameId

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

  bool NodeVarDecl::foldArrayInitExpression()
  {
    //for arrays with constant expression initializers(local or dm)
    UTI nuti = getNodeType();
    if(!m_state.okUTItoContinue(nuti) || !m_state.isComplete(nuti))
      return false;

    assert(!m_state.isScalar(nuti));
    assert(m_varSymbol && !(m_varSymbol->isInitValueReady()));

    //similar to NodeConstantDef's foldArrayInitExpression
    bool brtn = false;
    BV8K bvtmp;
    if(!hasInitExpr())
      {
	brtn = true; //t41205
      }
    else if(m_nodeInitExpr->isAList())
      {
	if(((NodeList *) m_nodeInitExpr)->foldArrayInitExpression())
	  brtn = ((NodeList *) m_nodeInitExpr)->buildArrayValueInitialization(bvtmp);
	//else no good
      }
    else if(m_nodeInitExpr->isAConstant())
      {
	brtn = m_nodeInitExpr->getConstantValue(bvtmp);
      }
    else
      m_state.abortShouldntGetHere(); //what then?

    if(brtn)
      m_varSymbol->setInitValue(bvtmp);

    return brtn;
  } //foldArrayInitExpression

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
		msg << m_state.getUlamTypeNameBriefByIndex(newType).c_str();
		msg << ", are incompatible";
		if(newt->isAltRefType() && newt->getUlamClassType() == UC_QUARK)
		  msg << "; .atomof may help";
		if(rscr == CAST_HAZY)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    else
	      {
		//not an atom
		ULAMTYPE etyp = m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum();
		ULAMTYPE fmetyp = m_state.getUlamTypeByIndex(newType)->getUlamTypeEnum();
		std::ostringstream msg;
		if(etyp == Bool)
		  msg << "Use a comparison operation";
		else if(fmetyp == String)
		  msg << "Invalid";
		else if(!m_state.isScalar(newType) || !m_state.isScalar(nuti))
		  msg << "Not possible";
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
	      } //not atom
	  } //not safe
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
	newType = Hzy;
	return false;
      }

    if(UlamType::compare(fromType, newType, m_state) == UTIC_SAME)
      return true;

    bool rtnOK = true;
    FORECAST scr = safeToCastTo(fromType); //reversed
    if(scr == CAST_HAZY)
      {
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
    if(m_state.getUlamTypeByIndex(uti)->isAltRefType())
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
	Node * parentNode = m_state.findNodeNoInThisClassForParent(pno);
	assert(parentNode);

	newnode->setNodeLocation(getNodeLocation());
	newnode->setYourParentNo(pno);
	newnode->resetNodeNo(getNodeNo()); //and symbol declnodeno

	AssertBool swapOk = parentNode->exchangeKids(this, newnode);
	assert(swapOk);

	{
	  std::ostringstream msg;
	  msg << "Exchanged kids! <" << m_state.m_pool.getDataAsString(m_vid).c_str();
	  msg << "> a reference variable, in place of a variable within class: ";
	  msg << m_state.getUlamTypeNameByIndex(cuti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}

	if(m_nodeTypeDesc)
	  newnode->m_nodeTypeDesc = (NodeTypeDescriptor *) m_nodeTypeDesc->instantiate();

	if(hasInitExpr())
	  newnode->m_nodeInitExpr = (Node *) m_nodeInitExpr->instantiate();

	delete this; //suicide is painless..

	// we must be the last thing called by checkandlabel
	// to return properly to our parent
	return newnode->checkAndLabelType();
      }
    return true; //ok
  } //checkReferenceCompatibility

  UTI NodeVarDecl::checkAndLabelType()
  {
    // instantiate, look up in current block
    if(m_varSymbol == NULL)
      checkForSymbol();

    //short circuit, avoid assert
    if(m_varSymbol == NULL)
      {
	setNodeType(Nav);
	return Nav;
      }

    assert(m_varSymbol);
    UTI vit = m_varSymbol->getUlamTypeIdx(); //base type has arraysize
    UTI cuti = m_state.getCompileThisIdx();
    if(m_nodeTypeDesc)
      {
	UTI duti = m_nodeTypeDesc->checkAndLabelType(); //sets goagain
	if(m_state.okUTItoContinue(duti) && (duti != vit))
	  {
	    std::ostringstream msg;
	    msg << "REPLACING Symbol UTI" << vit;
	    msg << ", " << m_state.getUlamTypeNameByIndex(vit).c_str();
	    msg << " used with variable symbol name '" << getName();
	    msg << "' with node type descriptor type: ";
	    msg << m_state.getUlamTypeNameByIndex(duti).c_str();
	    msg << " UTI" << duti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_varSymbol->resetUlamType(duti); //consistent!
	    m_state.mapTypesInCurrentClass(vit, duti);
	    //m_state.updateUTIAliasForced(it, duti); //help?NOPE, t3808
	    vit = duti;
	  }
      }

    ULAMTYPE etyp = m_state.getUlamTypeByIndex(vit)->getUlamTypeEnum();
    if(etyp == Void)
      {
	//void only valid use is as a func return type
	std::ostringstream msg;
	msg << "Invalid use of type ";
	msg << m_state.getUlamTypeNameBriefByIndex(vit).c_str();
	msg << " with variable symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //could be clobbered by Hazy node expr
	return Nav;
      }

    if(!m_state.okUTItoContinue(vit) || !m_state.isComplete(vit))
      {
	std::ostringstream msg;
	msg << "Incomplete Variable Decl for type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(vit).c_str();
	msg << ", used with variable symbol name '" << getName() << "'";
	if(m_state.okUTItoContinue(vit) || (vit == Hzy))
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    vit = Hzy; //t41201, error/t41165
	  }
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    setNodeType(vit); //t41169 pass along to class init expression nodes

    if(hasInitExpr())
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
	    setNodeType(Hzy);
	    m_state.setGoAgain(); //since not error
	    return Hzy; //short-circuit
	  }

	if(m_state.getCurrentBlock()->isASwitchBlock())
	  {
	    //e.g. switch condition variable (t41016-19)
	    UTI vit = m_varSymbol->getUlamTypeIdx(); //base type has arraysize
	    if(m_state.isHolder(vit) && m_state.isComplete(eit))
	      {
		m_state.cleanupExistingHolder(vit, eit);
		m_state.statusUnknownTypeInThisClassResolver(vit);
		vit = eit;
	      }
	  }

	//note: Void flags a list of constant initializers (unknown type).
	// t3768,69,70,77, t3853, t3863, t3946, t3974,77,79
	// could be either empty array initialization list, or empty class init list
	if(m_nodeInitExpr->isAList() && (eit == Void))
	  {
	    //only possible if array type with initializers
	    m_varSymbol->setHasInitValue(); //might not be ready yet

	    //if(!m_state.okUTItoContinue(vit) && m_nodeTypeDesc)
	    if(!m_state.isComplete(vit) && m_nodeTypeDesc) //error/t41165
	      {
		UTI duti = m_nodeTypeDesc->getNodeType();
		UlamType * dut = m_state.getUlamTypeByIndex(duti);
		if(m_state.okUTItoContinue(duti) && !dut->isComplete())
		  {
		    //assert(!dut->isScalar()); t41201
		    //if here, assume arraysize depends on number of initializers
		    s32 bitsize = dut->getBitSize();
		    u32 n = ((NodeList *) m_nodeInitExpr)->getNumberOfNodes();
		    if(n>0)
		      m_state.setUTISizes(duti, bitsize, n);

		    if(m_state.isComplete(duti))
		      {
			std::ostringstream msg;
			msg << "REPLACING Symbol UTI" << vit;
			msg << ", " << m_state.getUlamTypeNameByIndex(vit).c_str();
			msg << " used with variable symbol name '" << getName();
			msg << "' with node type descriptor array type (with initializers): ";
			msg << m_state.getUlamTypeNameByIndex(duti).c_str();
			msg << " UTI" << duti << " while labeling class: ";
			msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			m_varSymbol->resetUlamType(duti); //consistent!
			//'it' is not OK, that's Hzy; don't map Hzy!!!
			//m_state.mapTypesInCurrentClass(vit, duti);
			//m_state.updateUTIAliasForced(vit, duti); //help?
			m_nodeInitExpr->setNodeType(duti); //replace Void too!
			vit = duti;
		      }
		  }
	      }


	    if(hasInitExpr() && m_state.isComplete(vit))
	      {
		if(((NodeList *) m_nodeInitExpr)->isEmptyList()) //t41201, t41205, t41206
		  {
		    if(m_state.okUTItoContinue(vit) && m_state.isScalar(vit) && !m_state.isAClass(vit))
		      {
			//error scalar class var with {} error/t41208, t41201
			std::ostringstream msg;
			msg << "Scalar primitive variable '";
			msg << getName() << "' has improper {} initialization";
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			setNodeType(Nav);
			return Nav;
		      }
		    //unlike NodeConstantDef we don't keep an empty list.
		    delete m_nodeInitExpr;
		    m_nodeInitExpr = NULL; //poof! gone hence forth.
		  }
		else
		  {
		    //arraysize specified, may have fewer initializers
		    //support no initializers (t41201)
		    s32 arraysize = m_state.getArraySize(vit);
		    assert(arraysize >= 0); //t3847
		    u32 n = ((NodeList *) m_nodeInitExpr)->getNumberOfNodes();
		    if((n > (u32) arraysize) && (arraysize > 0))
		      {
			std::ostringstream msg;
			msg << "Too many initializers (" << n << ") specified for array '";
			msg << getName() << "', size " << arraysize;
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			setNodeType(Nav);
			return Nav;
		      }
		    m_nodeInitExpr->setNodeType(vit); //replace Void t41201
		  }
	      }
	    eit = vit;
	  } //end array initializers list && (eit == Void)
	else
	  {
	    if(!m_state.isScalar(vit) && m_nodeInitExpr->isAConstant())
	      m_varSymbol->setHasInitValue(); //t3896
	  }

	setNodeType(vit); //needed before safeToCast, and folding

	if(!m_state.isScalar(eit))
	//if(!m_state.isScalar(eit) && !m_state.isAClass(eit)) //t41185 can't fold classes wo reliable pos
	  {
	    if(m_state.okUTItoContinue(eit) && m_state.isComplete(eit))
	      {
		assert(m_varSymbol);
		//constant fold if possible, set symbol value
		if(m_varSymbol->hasInitValue())
		  {
		    if(!(m_varSymbol->isInitValueReady()))
		      {
			if(!foldArrayInitExpression()) //sets init constant value
			  {
			    assert(m_nodeInitExpr);
			    if((getNodeType() == Nav) || (m_nodeInitExpr->getNodeType() == Nav))
			      return Nav;

			    if(!(m_varSymbol->isInitValueReady()))
			      {
				setNodeType(Hzy);
				m_state.setGoAgain(); //since not error
				return Hzy;
			      }
			  }
		      }
		  } //has init val
	      }
	  } //array initialization

	if(m_state.okUTItoContinue(eit) && m_state.okUTItoContinue(vit))
	  checkSafeToCastTo(eit, vit); //may side-effect 'vit'
      } //end node expression

    if(m_varSymbol && m_varSymbol->isFunctionParameter() && ((SymbolVariableStack *) m_varSymbol)->isConstantFunctionParameter())
      Node::setStoreIntoAble(TBOOL_FALSE); //t41186
    else
    Node::setStoreIntoAble(TBOOL_TRUE);
    setNodeType(vit);

    //checkReferenceCompatibilty must be called at the end since
    // NodeVarDecl may do surgery on itself (e.g. t3666)
    if(m_state.okUTItoContinue(vit) && !checkReferenceCompatibility(vit))
      {
	vit = Nav; //err msg by checkReferenceCompatibility
	setNodeType(Nav); //failed
      }

    if(vit == Hzy)
      m_state.setGoAgain(); //since not error

    return vit; //in case of surgery don't call getNodeType();
  } //checkAndLabelType

  void NodeVarDecl::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    setBlock(currBlock);

    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_vid, asymptr, hazyKin) && !hazyKin)
      {
	if(!asymptr->isTypedef() && !asymptr->isConstant() && !asymptr->isModelParameter() && !asymptr->isFunction())
	  {
	    m_varSymbol = (SymbolVariable *) asymptr;
	    m_varSymbol->setDeclNodeNo(getNodeNo());
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
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //t3577
      } //alreadyDefined

    m_state.popClassContext(); //restore
  } //checkForSymbol

  NNO NodeVarDecl::getBlockNo()
  {
    return m_currBlockNo;
  }

  void NodeVarDecl::setBlock(NodeBlock * ptr)
  {
    m_currBlockPtr = ptr;
  }

  NodeBlock * NodeVarDecl::getBlock()
  {
    assert(m_currBlockNo);

    if(m_currBlockPtr)
      return m_currBlockPtr;

    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClassOrLocalsScope(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  void NodeVarDecl::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    assert(m_varSymbol);
    s32 newslot = depth + base;
    ((SymbolVariable *) m_varSymbol)->setStackFrameSlotIndex(newslot);
    depth += m_state.slotsNeeded(getNodeType());

    if(hasInitExpr())
      m_nodeInitExpr->calcMaxDepth(depth, maxdepth, base);
  } //calcMaxDepth

  void NodeVarDecl::printUnresolvedLocalVariables(u32 fid)
  {
    assert(m_varSymbol);
    UTI vit = m_varSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(vit))
      {
	// e.g. error/t3298 Int(Fu.sizeof)
	std::ostringstream msg;
	msg << "Unresolved type <";
	msg << m_state.getUlamTypeNameBriefByIndex(vit).c_str();
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

    if(hasInitExpr())
      m_nodeInitExpr->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);

    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  EvalStatus NodeVarDecl::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    u32 len = nut->getTotalBitSize();

    if((classtype == UC_TRANSIENT) && (len > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE);

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

    if(hasInitExpr()) //t3706, t3587, t41167, t41171
      return evalInitExpr();

    return NORMAL;
  } //eval

  void NodeVarDecl::setupStackWithPrimitiveForEval(u32 slots)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    assert(m_varSymbol && m_varSymbol->getUlamTypeIdx() == nuti);
    bool hasInitVal = m_varSymbol->hasInitValue();
    PACKFIT packFit = nut->getPackable();
    if(packFit == PACKEDLOADABLE)
      {
	u64 dval = 0;
	if(hasInitVal)
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
	  m_state.abortGreaterThanMaxBitsPerLong();

	m_state.m_funcCallStack.storeUlamValueInSlot(immUV, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex());
      }
    else
      {
	//unpacked primitive array - uses eval
	UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	u32 baseslot =  ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex();

	if(!hasInitVal)
	  {
	    UlamValue immUV;
	    u32 itemlen = nut->getBitSize();
	    if(itemlen <= MAXBITSPERINT)
	      immUV = UlamValue::makeImmediate(scalaruti, 0, m_state);
	    else if(itemlen <= MAXBITSPERLONG)
	      immUV = UlamValue::makeImmediateLong(scalaruti, 0, m_state);
	    else
	      m_state.abortGreaterThanMaxBitsPerLong();

	    for(u32 j = 0; j < slots; j++)
	      {
		UlamValue itemUV = immUV;
		m_state.m_funcCallStack.storeUlamValueInSlot(itemUV, baseslot + j);
	      }
	  }
	else if(hasInitVal)
	  {
	    //unpacked constant array, either list or named constant array;
	    // (items cast during constant folding) (t3704, t3769, t3896)
	    u32 itemlen = nut->getBitSize();

	    for(u32 j = 0; j < slots; j++)
	      {
		UlamValue itemUV;
		if(itemlen <= MAXBITSPERINT)
		  {
		    u32 ival = 0;
		    AssertBool gotVal = m_varSymbol->getArrayItemInitValue(j, ival);
		    assert(gotVal);
		    itemUV = UlamValue::makeImmediate(scalaruti, ival, m_state);
		  }
		else if(itemlen <= MAXBITSPERLONG)
		  {
		    u64 ivalong = 0;
		    AssertBool gotVal = m_varSymbol->getArrayItemInitValue(j, ivalong);
		    assert(gotVal);
		    itemUV = UlamValue::makeImmediateLong(scalaruti, ivalong, m_state);
		  }
		else
		  m_state.abortGreaterThanMaxBitsPerLong();

		m_state.m_funcCallStack.storeUlamValueInSlot(itemUV, baseslot + j);
	      }
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
	      m_state.abortGreaterThanMaxBitsPerLong(); //not write load packable!
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
	    else
	      m_state.abortGreaterThanMaxBitsPerLong(); //not write load packable!
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
      return NORMAL;

    UTI nuti = getNodeType();

    assert(m_nodeInitExpr);
    if(m_nodeInitExpr->isClassInit())
      return NORMAL; //t41171, t3706

    //note: continue with classes for their default values, varSymbol may not have an init value!

    EvalStatus evs = NORMAL; //init
    // quark or non-class data member;
    evalNodeProlog(0); //new current node eval frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr

    evs = evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1);
    u32 slots = makeRoomForNodeType(nuti);

    evs = m_nodeInitExpr->eval();
    if(evs != NORMAL) return evalStatusReturn(evs);


    if(m_nodeInitExpr->isAConstructorFunctionCall())
      {
	//Void to be avoided.
      }
    else if(slots == 1)
      {
	UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slots+1); //immediate scalar + 1 for pluv
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

	u32 slotoff = 1 + 1;
	for(u32 j = 0; j < slots; j++)
	  {
	    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slotoff+j); //immediate scalar
	    m_state.assignValue(scalarPtr,ruv);
	    scalarPtr.incrementPtr(m_state); //by one.
	  }
      }

    evalNodeEpilog();
    return NORMAL;
  } //evalInitExpr

  EvalStatus NodeVarDecl::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    u32 len = nut->getTotalBitSize();

    if((classtype == UC_TRANSIENT) && (len > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE);

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

    if(hasInitExpr())
      {
	//distinction between variable and instanceof
	bool varcomesfirst = (m_nodeInitExpr->isAConstructorFunctionCall() && (m_nodeInitExpr->getReferenceAble() == TBOOL_TRUE)) || m_nodeInitExpr->isClassInit(); //t41077, t41085, t41171

	if(varcomesfirst)
	  {
	    //provide default variable first (t41077)
	    //left-justified. no initialization
	    m_state.indentUlamCode(fp);
	    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_varSymbol->getMangledName().c_str());
	    fp->write(";"); GCNL; //func call args aren't NodeVarDecl's
	  }

	if(m_nodeInitExpr->isClassInit()) //t41171-4
	  m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol); //*********UPDATED GLOBAL;

	UVPass uvpass2clear;
	uvpass = uvpass2clear; //refresh

	m_nodeInitExpr->genCode(fp, uvpass);

	m_state.clearCurrentObjSymbolsForCodeGen(); //************CLEAR

	if(!varcomesfirst)
	  {
	    m_state.indentUlamCode(fp);
	    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_varSymbol->getMangledName().c_str());
	    fp->write("("); // use constructor (not equals)

	    if(UlamType::compareForString(vuti, m_state) == UTIC_SAME)
	      {
		TMPSTORAGE vstor = uvpass.getPassStorage();
		if((vstor == TMPBITVAL) || (vstor == TMPAUTOREF))
		  {
		    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
		    fp->write(".getRegistrationNumber(), ");
		    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
		    fp->write(".getStringIndex());"); GCNL; //t3948
		  }
		else
		  {
		    fp->write(m_state.getStringMangledName().c_str());
		    fp->write("::getRegNum(");
		    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
		    fp->write("), ");
		    fp->write(m_state.getStringMangledName().c_str());
		    fp->write("::getStrIdx(");
		    fp->write(uvpass.getTmpVarAsString(m_state).c_str());
		    fp->write("));"); GCNL;
		  }
	      }
	    else
	      {
		fp->write(uvpass.getTmpVarAsString(m_state).c_str());
		if((uvpass.getPassStorage() == TMPBITVAL) && m_nodeInitExpr->isExplicitCast())
		  fp->write(".read()"); //ulamexports: WallPort->QPort4->Cell (e.g. t3922, t3715)
		else if(m_state.isAtomRef(vuti))
		  fp->write(", uc");
		fp->write(");"); GCNL; //func call args aren't NodeVarDecl's
	      }
	  }
	m_state.clearCurrentObjSymbolsForCodeGen();
	return; //done
      } //has a nodeInitExpr

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
	    fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
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

  void NodeVarDecl::genCodeConstantArrayInitialization(File * fp)
  {
    m_state.abortShouldntGetHere(); //see NodeVarDeclDM
  }

  void NodeVarDecl::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {
    m_state.abortShouldntGetHere(); //see NodeVarDeclDM
  }

  void NodeVarDecl::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    m_state.abortShouldntGetHere(); //see NodeVarDeclDM data members only
  }

  void NodeVarDecl::addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers)
  {
    m_state.abortShouldntGetHere(); //see NodeVarDeclDM data members only
  }

} //end MFM
