#include <stdlib.h>
#include "NodeConstantDef.h"
#include "NodeConstant.h"
#include "NodeListClassInit.h"
#include "NodeTerminal.h"
#include "CompilerState.h"
#include "MapConstantDesc.h"
#include "NodeTypeDescriptorArray.h"
#include "Parity2D_4x4.h"

namespace MFM {

  NodeConstantDef::NodeConstantDef(SymbolWithValue * symptr, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_constSymbol(symptr), m_nodeExpr(NULL), m_nodeTypeDesc(nodetype), m_cid(0), m_currBlockNo(m_state.getCurrentBlockNo()), m_currBlockPtr(NULL)
  {
    if(symptr)
      {
	// node uses current block no, not the one saved in the symbol
	// (e.g. pending class args)
	m_cid = symptr->getId();
	symptr->setDeclNodeNo(getNodeNo());
	assert(!nodetype || nodetype->givenUTI() == symptr->getUlamTypeIdx()); //invariant?
      }
  }

  NodeConstantDef::NodeConstantDef(const NodeConstantDef& ref) : Node(ref), m_constSymbol(NULL), m_nodeExpr(NULL), m_nodeTypeDesc(NULL), m_cid(ref.m_cid), m_currBlockNo(ref.m_currBlockNo), m_currBlockPtr(NULL)
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
    assert(m_state.getCurrentBlockNo() == m_currBlockNo); //changes to m_constSymbol's ST
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
  }

  void NodeConstantDef::resetNodeLocations(Locator loc)
  {
    Node::setNodeLocation(loc);
    if(m_nodeExpr) m_nodeExpr->resetNodeLocations(loc);
  }

  void NodeConstantDef::printPostfix(File * fp)
  {
    //in case the node belongs to the template, use the symbol uti, o.w. 0Nav. (t41221)
    UTI suti = getNodeType();
    if(m_nodeTypeDesc)
      suti = m_nodeTypeDesc->givenUTI();
    else if(m_constSymbol)
      suti = m_constSymbol->getUlamTypeIdx();
    else
      m_state.abortNotImplementedYet();

    //see also SymbolConstantValue
    fp->write(" constant");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(suti).c_str());
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(m_cid).c_str());

    s32 arraysize = m_state.getArraySize(suti);
    //output the arraysize (optional) t3958
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

    if(m_nodeExpr)
      {
	fp->write(" =");
	m_nodeExpr->printPostfix(fp);
      }
    fp->write("; ");
  } //printPostfix

  void NodeConstantDef::noteTypeAndName(UTI cuti, s32 totalsize, u32& accumsize)
  {
    return; //bypass
  }

  void NodeConstantDef::genTypeAndNameEntryAsComment(File * fp, s32 totalsize, u32& accumsize)
  {
    return; //bypass
  }

  const char * NodeConstantDef::getName()
  {
    if(m_constSymbol)
      return m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
    return "CONSTDEF?";
  }

  u32 NodeConstantDef::getNameId()
  {
    return m_cid;
  }

  UTI NodeConstantDef::getGivenUTI()
  {
    assert(m_nodeTypeDesc);
    return m_nodeTypeDesc->givenUTI();
  }

  u32 NodeConstantDef::getTypeNameId()
  {
    //like NodeVarDecl; used for Ulam Class Signature for Target Map
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

  const std::string NodeConstantDef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeConstantDef::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_constSymbol;
    return (m_constSymbol != NULL); //true;
  }

  void NodeConstantDef::clearSymbolPtr()
  {
    //if symbol is in a stub, there's no guarantee the stub
    // won't be replace by another duplicate class once its
    // pending args have been resolved.
    if(!(m_constSymbol->isClassParameter() || m_constSymbol->isClassArgument()))
      {
	m_constSymbol = NULL;
	setBlock(NULL);
      }
  }

  void NodeConstantDef::setSymbolPtr(SymbolWithValue * cvsymptr)
  {
    assert(cvsymptr);
    m_constSymbol = cvsymptr;
    setBlockNo(cvsymptr->getBlockNoOfST()); //also clears block ptr (t41224)
    assert(m_currBlockNo);
  }

  u32 NodeConstantDef::getSymbolId()
  {
    return m_cid;
  }

  bool NodeConstantDef::setSymbolValue(const BV8K& bv)
  {
    assert(m_constSymbol);
    bool rtnb = false;
    if(m_constSymbol->isClassParameter())
      {
	assert(hasDefaultSymbolValue());
	m_constSymbol->setInitValue(bv); //t41438??
	rtnb = m_constSymbol->isInitValueReady();
      }
      else if(m_constSymbol->isClassArgument() && isClassArgumentItsDefaultValue())
	{
	  m_constSymbol->setValue(bv); //isReady now! (e.g. ClassArgument, ModelParameter)
	  rtnb = m_constSymbol->isReady();
	}
      else if(isDataMemberInit())
	{
	  m_constSymbol->setInitValue(bv); //(isInitValueReady now)! t41168
	  rtnb = m_constSymbol->isInitValueReady();
	}
      else
	{
	  m_constSymbol->setValue(bv); //isReady now! (e.g. ClassArgument, ModelParameter)
	  rtnb = m_constSymbol->isReady();
	}
      return rtnb;
  } //setSymbolValue

  bool NodeConstantDef::getNodeTypeDescriptorPtr(NodeTypeDescriptor *& nodetypedescref)
  {
    if(m_nodeTypeDesc)
      {
	nodetypedescref = m_nodeTypeDesc;
	return true;
      }
    return false;
  }

  bool NodeConstantDef::setNodeTypeDescriptor(NodeTypeDescriptor * nodetypedesc)
  {
    if(m_nodeTypeDesc == NULL)
      {
	m_nodeTypeDesc = nodetypedesc; //tfr ownership here
	//if(m_constSymbol)
	//  m_constSymbol->resetUlamType(m_nodeTypeDesc->givenUTI()); //or keep hazy
	////  m_nodeTypeDesc->resetGivenUTI(m_constSymbol->getUlamTypeIdx()); //invariant?
	return true;
      }
    return false;
  }

  bool NodeConstantDef::getNodeExprPtr(Node *& nodeexprref)
  {
    if(m_nodeExpr)
      {
	nodeexprref = m_nodeExpr;
	return true;
      }
    return false;
  }

  bool NodeConstantDef::setNodeExpr(Node * nodeexprref)
  {
    if(m_nodeExpr == NULL)
      {
	m_nodeExpr = nodeexprref; //tfr ownership here
	m_nodeExpr->updateLineage(getNodeNo());
	if(m_constSymbol)
	  m_constSymbol->setHasInitValue();
	return true;
      }
    return false;
  }


  bool NodeConstantDef::hasDefaultSymbolValue()
  {
    assert(m_constSymbol);
    return m_constSymbol->hasInitValue();
  }

  bool NodeConstantDef::isClassArgumentItsDefaultValue()
  {
    assert(m_constSymbol);
    assert(m_constSymbol->isClassParameter() || m_constSymbol->isClassArgument());
    return ((SymbolConstantValue *) m_constSymbol)->isClassArgDefaultValue();
  }

  bool NodeConstantDef::isDataMemberInit()
  {
    return false;
  }

  UTI NodeConstantDef::checkAndLabelType(Node * thisparentnode)
  {
    UTI nuti = getNodeType(); //expression type

    if(nuti == Nav)
      return Nav; //short-circuit, already failed.

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
    UTI contextForArgTypes = m_state.m_pendingArgTypeStubContext;
    bool changeScope = false;
    // type of the constant
    if(m_nodeTypeDesc)
      {
	bool changeScopeForTypesOnly = false;
	if(contextForArgTypes != Nouti)
	  {
	    assert(m_constSymbol->isClassParameter() || m_constSymbol->isClassArgument());
	    m_state.pushClassOrLocalContextAndDontUseMemberBlock(contextForArgTypes);
	    changeScopeForTypesOnly = true; //t41216, error/t41218
	  }

	UTI duti = m_nodeTypeDesc->checkAndLabelType(this); //clobbers any expr it
	if(duti == Nav)
	  suti = Nav;
	else if(m_state.okUTItoContinue(duti) && (suti != duti))
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
	    suti = duti;
	  }
	if(changeScopeForTypesOnly)
	  m_state.popClassContext(); //restore
      }
    else
      {
	//no m_nodeTypeDesc: e.g. class arg of unseen template, fixed later (t3370)
	assert(m_constSymbol && m_constSymbol->isClassArgument()); //sanity check
	assert((contextForArgTypes != Nouti) && m_state.isClassAStub(contextForArgTypes) && !m_state.isASeenClass(contextForArgTypes)); //sanity check, cuti might be its context
      }

    // move before m_nodeExpr "Void" check (t3883, error/t3451);
    // check completeness in case arraysize depends on number of items in initialization (t3890)
    if(!m_state.okUTItoContinue(suti) || !m_state.isComplete(suti))
      {
	std::ostringstream msg;
	if(suti == Nav)
	  msg << "Invalid Constant Def type ";
	else
	  msg << "Incomplete Constant Def for type";
	if(m_state.okUTItoContinue(suti) && !m_state.isHolder(suti))
	  msg << ": " << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	msg << " used with symbol name '" << getName() << "'";
	//msg << " while compiling UTI" << cuti;
	if(suti == Nav)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    suti = Hzy; //since not error; wait to goagain until not Nav
	  }
      }

    ULAMTYPE etyp = m_state.getUlamTypeByIndex(suti)->getUlamTypeEnum();
    if(etyp == Void)
      {
	//void only valid use is as a func return type (t3389)
	std::ostringstream msg;
	msg << "Invalid use of type ";
	msg << m_state.getUlamTypeNameByIndex(suti).c_str();
	msg << " with symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //could be clobbered by Hazy node expr

	if(changeScope)
	  m_state.popClassContext(); //restore

	return Nav;
      }

    // NOASSIGN REQUIRED (e.g. for class parameters) doesn't have to have this!
    if(m_nodeExpr)
      {
	nuti = m_nodeExpr->checkAndLabelType(this);

	if(changeScope)
	  m_state.popClassContext(); //restore

	if(nuti == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for";
	    if(m_constSymbol && m_constSymbol->isClassArgument())
	      msg << " class argument";
	    else if(m_constSymbol && m_constSymbol->isClassParameter())
	      msg << " class parameter";
	    msg << ": ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is invalid";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	if(m_state.isStillHazy(nuti))
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is not ready, still hazy";
	    msg << ", while compiling " << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << " (UTI " << cuti << ")";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setNodeType(Hzy);
	    clearSymbolPtr();
	    m_state.setGoAgain();
	    return Hzy; //short-circuit
	  }

	//note: Void is flag that it's a list of constant initializers;
	// code lifted from NodeVarDecl.cpp c&l.
	if(nuti == Void)
	  {
	    if((m_nodeExpr != NULL) && ((NodeList *) m_nodeExpr)->isEmptyList())
	      {
		//keep empty list for constant def (t41202)
	      }
	    else
	      {
		//only possible if array type with initializers;
		if(m_state.okUTItoContinue(suti) && m_state.isScalar(suti))
		  {
		    //error scalar with {} error (t41389, t41390)
		    std::ostringstream msg;
		    msg << "Scalar constant '";
		    msg << getName() << "' has improper {} initialization";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		    return Nav;
		  }

		if(!m_state.okUTItoContinue(suti) && m_nodeTypeDesc && m_nodeTypeDesc->isEmptyArraysizeDecl())
		  {
		    UTI duti = m_nodeTypeDesc->getNodeType();
		    UlamType * dut = m_state.getUlamTypeByIndex(duti);
		    UTI scalarduti = m_nodeTypeDesc->getScalarType();

		    if(m_state.okUTItoContinue(scalarduti) && !dut->isComplete())
		      {
			//if here, empty arraysize depends on number of initializers
			s32 bitsize = m_state.getBitSize(scalarduti);
			u32 n = ((NodeList *) m_nodeExpr)->getNumberOfNodes();
			duti = m_nodeTypeDesc->givenUTI(); //Hzy not helpful, reload with given
			m_state.setUTISizes(duti, bitsize, n);

			if(m_state.isComplete(duti))
			  {
			    std::ostringstream msg;
			    msg << "REPLACING Symbol UTI" << nuti;
			    msg << ", " << m_state.getUlamTypeNameByIndex(nuti).c_str();
			    msg << " used with variable symbol name '" << getName();
			    msg << "' with node type descriptor array type (with initializers): ";
			    msg << m_state.getUlamTypeNameBriefByIndex(duti).c_str();
			    msg << " UTI" << duti << " while labeling class: ";
			    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			    m_constSymbol->resetUlamType(duti); //consistent!
			    suti = m_constSymbol->getUlamTypeIdx(); //reset after alias (t3890, t3891)
			  }
		      }
		    //else
		  }
	      }

	    if(m_state.okUTItoContinue(suti) && m_state.isComplete(suti))
	      {
		if(m_nodeExpr)
		  {
		    if(m_state.isScalar(suti))
		      {
			if(m_nodeExpr->isAList() && !m_state.isAClass(suti))
			  {
			    //error scalar class var with {} error/t41208
			    std::ostringstream msg;
			    msg << "Constant scalar primitive '";
			    msg << getName() << "' has improper {} initialization";
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			    setNodeType(Nav);
			    nuti = Nav;
			  }
			//else
		      }
		    else
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
			    nuti = Nav;
			  }
			else
			  {
			    m_nodeExpr->setNodeType(suti);
			    nuti = suti;
			  }
		      }
		  } //no node expr
		else
		  nuti = suti; //t41202
	      }
	    else
	      {
		assert(suti != Nav);
		nuti = Hzy;
	      }
	  } //end array initializers or empty class init (eit == Void)

	if(m_state.okUTItoContinue(nuti) && m_state.okUTItoContinue(suti))
	  {
	    if(m_state.isScalar(nuti) ^ m_state.isScalar(suti))
	      {
		std::ostringstream msg;
		msg << "Constant value expression for";
		if(m_constSymbol && m_constSymbol->isClassArgument())
		  msg << " class argument";
		else if(m_constSymbol && m_constSymbol->isClassParameter())
		  msg << " class parameter";
		msg << ": ";
		msg << m_state.m_pool.getDataAsString(m_cid).c_str();
		msg << ", array/scalar mismatch";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav);
		return Nav; //short-circuit (t3446, t3898)
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for";
	    if(m_constSymbol && m_constSymbol->isClassArgument())
	      msg << " class argument";
	    else if(m_constSymbol && m_constSymbol->isClassParameter())
	      msg << " class parameter";
	    msg << ": ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is ";
	    if(nuti == Nav || suti == Nav)
	      {
		msg << "invalid";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav);
		return Nav;
	      }
	    else
	      {
		msg << "not ready";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		setNodeType(Hzy);
		clearSymbolPtr();
		m_state.setGoAgain();
		return Hzy; //short-circuit (t3893)
	      }
	  }

	if(m_nodeExpr && !m_nodeExpr->isAConstant() && !m_state.isConstantRefType(suti))
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for";
	    if(m_constSymbol && m_constSymbol->isClassArgument())
	      msg << " class argument";
	    else if(m_constSymbol && m_constSymbol->isClassParameter())
	      msg << " class parameter";
	    msg << ": ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is not a constant";
	    if(m_constSymbol && m_state.isAClass(suti))
	      msg << "; Suggest '= {};' for default values"; //t41300
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //t3893 wrong loc!!
	    setNodeType(Nav);
	    return Nav; //short-circuit (error/t3453) after possible empty array init is deleted (t41202)
	  }
	//else t41192
      } //end node expression
    else
      {
	if(changeScope)
	  m_state.popClassContext(); //restore
      }

    if(m_state.isComplete(suti)) //reloads
      {
	ULAMTYPE eit = m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum();
	ULAMTYPE esuti = m_state.getUlamTypeByIndex(suti)->getUlamTypeEnum();
	if(m_state.okUTItoContinue(nuti) && (eit != esuti))
	  {
	    std::ostringstream msg;
	    msg << prettyNodeName().c_str() << " '" << getName();
	    msg << "' type <" << m_state.getUlamTypeByIndex(suti)->getUlamTypeNameOnly().c_str();
	    msg << "> does not match its value type <";
	    msg << m_state.getUlamTypeByIndex(nuti)->getUlamTypeNameOnly().c_str() << ">";
	    msg << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << " UTI" << cuti;
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }

    if(m_state.isComplete(nuti) && m_state.isAClass(nuti) && (UlamType::compare(nuti, suti, m_state) != UTIC_SAME))
      {
	if(m_constSymbol->isClassArgument())
	  {
	    std::ostringstream msg;
	    msg << "Class argument '" << getName();
	    msg << "' not satisfied by expression type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    if(m_state.isComplete(suti))
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav);
		return Nav; //t41210
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		setNodeType(Hzy);
		clearSymbolPtr();
		m_state.setGoAgain();
		return Hzy;
	      }
	  }
      }

    setNodeType(suti);

    if(!isReadyConstant() && m_nodeExpr) //error/t3451
      {
	UTI foldrtn = foldConstantExpression();
	if(foldrtn == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Problem in " << prettyNodeName().c_str() << " for type: ";
	    msg << m_state.getUlamTypeNameByIndex(suti).c_str();
	    msg << ", used with symbol name '" << getName() << "', after folding attempt";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	  }
	else if(m_state.isStillHazy(foldrtn))
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Constant Def for type: ";
	    msg << m_state.getUlamTypeNameByIndex(suti).c_str();
	    msg << ", used with symbol name '" << getName() << "', after folding";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //t41204
	    setNodeType(Hzy);
	  }
	else //if(!m_state.isAClass(foldrtn)) //t41198
	  {
	    if(!isReadyConstant() && !m_state.isConstantRefType(suti))
	      {
		std::ostringstream msg;
		msg << "Constant symbol '" << getName() << "' is not ready";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		setNodeType(Hzy);
	      }
	    //else t41192
	  }
      }
    if(getNodeType() == Hzy)
      {
	clearSymbolPtr();
	m_state.setGoAgain();
      }
    return getNodeType();
  } //checkAndLabelType

  void NodeConstantDef::checkForSymbol()
  {
    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    setBlock(currBlock);

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
	    msg << "(1) '" << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << "' is not a constant, and cannot be used as one";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) Named Constant '" << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "' is not defined, and cannot be used";
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
    m_currBlockPtr = NULL;
  }

  void NodeConstantDef::setBlock(NodeBlock * ptr)
  {
    m_currBlockPtr = ptr;
  }

  NodeBlock * NodeConstantDef::getBlock()
  {
    assert(m_currBlockNo);

    if(m_currBlockPtr)
      return m_currBlockPtr;

    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClassOrLocalsScope(m_currBlockNo);

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

  bool NodeConstantDef::isReadyConstant()
  {
    return m_constSymbol && (m_constSymbol->isReady() || m_constSymbol->isInitValueReady());
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
    if(isReadyConstant())
      return uti;

    if(m_state.isConstantRefType(uti))
      {
	assert(m_nodeExpr);
	if(!m_nodeExpr->isAConstant())
	  {
	    return uti; //no folding when not a constant expression (t41192)
	  }
      }

    if(!m_state.isScalar(uti))
      {
	// similar to NodeVarDecl (t3881); constant class array (t41261,2)
	if(!(isReadyConstant() || foldArrayInitExpression()))
	  {
	    assert(m_nodeExpr);
	    if((getNodeType() == Nav) || m_nodeExpr->getNodeType() == Nav)
	      return Nav;

	    if(!isReadyConstant())
	      {
		setNodeType(Hzy);
		m_state.setGoAgain(); //since not error
		return Hzy;
	      }
	  }
	else
	  {
	    if(!m_constSymbol->isClassParameter() && !isDataMemberInit())
	      {
		// class args/param values do not belong on the CNSTSTACK (t3894)
		u32 tmpslotnum = m_state.m_constantStack.getAbsoluteTopOfStackIndexOfNextSlot();
		assignConstantSlotIndex(tmpslotnum);
	      }
	  }
	return uti;
	//else scalar classes continue..to set up possible default values
      } //not scalar

    if(!m_nodeExpr)
      return uti; //scalar, wo init expr (e.g. t3326,7,8,9)

    //scalar classes wait until after c&l to build default value;
    // but pieces can be folded in advance;
    // t41198 (might be better to replace node with NodeInitDM?)
    if(m_state.isAClass(uti) || m_state.isAtom(uti)) //t41483 constant atom
      {
	UTI rtnuti = Nav;
	if(m_nodeExpr->isAList()) //t3451, t41209
	  {
	    if(!((NodeList *) m_nodeExpr)->isEmptyList())
	      {
		rtnuti =  ((NodeListClassInit *) m_nodeExpr)->foldConstantExpression();
	      }
	    else
	      rtnuti = uti;  //t41216

	    //ulam-4 we can't wait for batch-style Element-Type assignment (e.g. t41230)
	    //tries to pack bits if complete
	    if(buildDefaultValueForClassConstantDefs())
	      {
		u32 tmpslotnum = m_state.m_constantStack.getAbsoluteTopOfStackIndexOfNextSlot();
		assignConstantSlotIndex(tmpslotnum); //t41198
	      }
	    else
	      rtnuti = Hzy;
	  }
	else if(m_nodeExpr->isAConstant())
	  {
	    //constant class t3451; member select t41273; array item t41263
	    BV8K bvtmp;
	    if(m_nodeExpr->getConstantValue(bvtmp))
	      {
		setSymbolValue(bvtmp);
		rtnuti = m_nodeExpr->getNodeType();
		u32 tmpslotnum = m_state.m_constantStack.getAbsoluteTopOfStackIndexOfNextSlot();
		assignConstantSlotIndex(tmpslotnum);
	      }
	    else
	      rtnuti = Hzy; //t41509
	  }
	else
	  m_state.abortShouldntGetHere();

	return rtnuti;
      } //end isAClass

    // if here, must be a primitive constant..
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
	msg << "', is erronous";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    if(evs == NOTREADY)
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str();
	msg << "', is not yet ready";
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
    if(!ut->cast(cnstUV, uti))
      {
	std::ostringstream msg;
	msg << "Constant value expression for '";
	msg << getName() << "', was not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return Nav;
      }

    u32 wordsize = m_state.getTotalWordSize(uti);
    if(wordsize == MAXBITSPERINT)
      newconst = cnstUV.getImmediateData(m_state);
    else if(wordsize == MAXBITSPERLONG)
      newconst = cnstUV.getImmediateDataLong(m_state);
    else
      m_state.abortGreaterThanMaxBitsPerLong();

    // BUT WHY when Symbol is all we need/want? because it indicates
    // there's a default value before c&l (see SCNT::getTotalParametersWithDefaultValues) (t3526)
    //then do the surgery
    NodeTerminal * newnode;
    ULAMTYPE etyp = m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum();
    if(etyp == Int)
      newnode = new NodeTerminal((s64) newconst, uti, m_state);
    else
      newnode = new NodeTerminal(newconst, uti, m_state);
    newnode->setNodeLocation(getNodeLocation());
    delete m_nodeExpr;
    m_nodeExpr = newnode;

    BV8K bvtmp;
    u32 len = m_state.getTotalBitSize(uti);
    bvtmp.WriteLong(0u, len, newconst); //is newconst packed?
    setSymbolValue(bvtmp);

    //for primitive constants too; eval support for function constant parameters (t41240)
    u32 tmpslotnum = m_state.m_constantStack.getAbsoluteTopOfStackIndexOfNextSlot();
    assignConstantSlotIndex(tmpslotnum);

    return uti; //ok
  } //foldConstantExpression

  bool NodeConstantDef::foldArrayInitExpression()
  {
    //for arrays with constant expression initializers(local or dm)
    UTI nuti = getNodeType();
    if(!m_state.okUTItoContinue(nuti) || !m_state.isComplete(nuti))
      return false;

    assert(!m_state.isScalar(nuti));
    assert(m_constSymbol && !(m_constSymbol->isReady() || m_constSymbol->isInitValueReady()));
    //build BV8K: a use (i.e. NodeConstantArray) like a class arg,
    //or already folded initialization, to avoid invalid casting to
    //NodeListArrayInitialization (t3894)
    bool brtn = false;
    BV8K bvtmp;
    if(!m_nodeExpr)
      brtn = true; //t41202
    else if(m_nodeExpr->isAList())
      {
	if(((NodeList *) m_nodeExpr)->foldArrayInitExpression())
	  {
	    if(m_state.isAClass(nuti))
	      {
		BV8K bvclass;
		if(!m_state.getDefaultClassValue(nuti, bvclass))
		  return false; //possibly not ready (t41271,2,3)

		UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
		UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
		UlamType * nut = m_state.getUlamTypeByIndex(nuti);

		m_state.getDefaultAsArray(scalarut->getSizeofUlamType(), nut->getArraySize(), 0, bvclass, bvtmp); //t41179
	      }
	    //else (e.g. atom bvtmp is zeros)

	    if(((NodeList *) m_nodeExpr)->buildArrayValueInitialization(bvtmp)) //t41492 redo
	      brtn = true;
	  }
	//else no good (error/t41181)
      }
    else
      brtn = m_nodeExpr->getConstantValue(bvtmp); //t41277 memberselect, t41181 array

    if(brtn)
      {
	setSymbolValue(bvtmp);
      }
    return brtn;
  } //foldArrayInitExpression

  bool NodeConstantDef::buildDefaultValue(u32 wlen, BV8K& bvref)
  {
    UTI nuti = getNodeType();
    if(!m_state.okUTItoContinue(nuti) || !m_state.isComplete(nuti))
      {
	std::ostringstream msg;
	msg << "Building default value for member constant '";
	msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "' not ready";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	m_state.goAgain();
	return false; //t41478
      }

    bool rtnok = buildDefaultValueForClassConstantDefs();
    if(!rtnok)
      {
	std::ostringstream msg;
	msg << "Building default value for member constant '";
	msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "' failed";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //t41477
      }
    return rtnok;
  } //buildDefaultValue

  bool NodeConstantDef::buildDefaultValueForClassConstantDefs()
  {
    UTI nuti = getNodeType();
    assert(m_state.okUTItoContinue(nuti) && m_state.isComplete(nuti)); //t41478 (see caller above)

    if(!m_state.isAClass(nuti)) //t41198
      return true;

    if(isReadyConstant())
      return true;

    bool rtnok = false;
    BV8K bvtmp;
    if(!m_nodeExpr)
      {
	//no node expression, use default value
	if((rtnok = m_state.getDefaultClassValue(nuti, bvtmp))) //uses scalar uti
	  {
	    setSymbolValue(bvtmp);
	  }
	return rtnok;
      }

    //here, has nodeExpr, isn't ready, and is a class..
    if(m_nodeExpr->isAConstantClass())
      {
	if((rtnok = m_nodeExpr->getConstantValue(bvtmp)))
	  setSymbolValue(bvtmp);
      }
    else if(m_nodeExpr->isAList())
      {
	if(m_state.getDefaultClassValue(nuti, bvtmp)) //uses scalar uti
	  {
	    if(!((NodeList *) m_nodeExpr)->isEmptyList())
	      {
		BV8K bvmask;
		if((rtnok = ((NodeListClassInit *) m_nodeExpr)->initDataMembersConstantValue(bvtmp,bvmask))) //passes along default value
		  setSymbolValue(bvtmp);
	      }
	    else
	      {
		setSymbolValue(bvtmp); //empty list uses default
		rtnok = true;
	      }
	  } // else no default, perhaps not ready
      }
    else
      m_state.abortShouldntGetHere(); //why am i here?

    return rtnok;
  } //buildDefaultValueForClassConstantDefs

  void NodeConstantDef::fixPendingArgumentNode()
  {
    assert(m_constSymbol);
    // for unseen classes that needed their args "fixed" to proper param name
    // this fixes the saved m_cid while clonePendingClassArgumentsForStubClassInstance
    // (the m_cid is used during full instantiation).
    if((m_constSymbol->getId() != getSymbolId()))
      {
	m_cid = m_constSymbol->getId();
      }
  } //fixPendingArgumentNode

  bool NodeConstantDef::cloneTypeDescriptorForPendingArgumentNode(NodeConstantDef * templateparamdef)
  {
    bool aok = false;
    // for unseen classes that needed their args "fixed"
    // this grabs the Template's type descriptor after the class was seen
    // (the nodetypedescriptor is to help resolve pending argument types (t41211, t41209)
    if(m_nodeTypeDesc == NULL)
      {
	//clone the template's node type descriptor for this stub's pending argument
	NodeTypeDescriptor * pnodetypedesc = NULL;
	if(templateparamdef->getNodeTypeDescriptorPtr(pnodetypedesc))
	  {
	    NodeTypeDescriptor * copynodetypedesc = (NodeTypeDescriptor *) pnodetypedesc->instantiate(); //t41209?
	    assert(copynodetypedesc);
	    copynodetypedesc->setNodeLocation(getNodeLocation()); //same loc as this node

	    AssertBool isset = setNodeTypeDescriptor(copynodetypedesc); //resets givenuti too.
	    assert(isset);

	    assert(m_constSymbol && ((m_constSymbol->getUlamTypeIdx() == Hzy) || (m_constSymbol->getUlamTypeIdx() == m_nodeTypeDesc->givenUTI()))); //invariant? (likely null symbol, see checkForSymbol) t41361, t3326
	    aok = true;
	  }
      }
    return aok;
  } //cloneTypeDescriptorForPendingArgumentNode

    bool NodeConstantDef::cloneDefaultValueExpressionForPendingArgumentNode(NodeConstantDef * templateparamdef)
  {
    bool aok = false;
    // for unseen classes that needed their args "fixed"
    // this grabs the Template's m_nodeExpr after the class was seen
    // (the default value is to help resolve pending argument values (t41436)
    if(m_nodeExpr == NULL)
      {
	//clone the template's node type descriptor for this stub's pending argument
	Node * pnodeexpr = NULL;
	if(templateparamdef->getNodeExprPtr(pnodeexpr))
	  {
	    Node * copynodeexpr = pnodeexpr->instantiate();
	    assert(copynodeexpr);
	    copynodeexpr->setNodeLocation(getNodeLocation()); //same loc as this node

	    AssertBool isset = setNodeExpr(copynodeexpr);
	    assert(isset);

	    aok = true;
	  }
      }
    return aok;
  } //cloneDefaultValueExpressionForPendingArgumentNode

  EvalStatus NodeConstantDef::eval()
  {
    assert(m_constSymbol);
    if(isReadyConstant())
      return NORMAL;
    return evalStatusReturnNoEpilog(NOTREADY); //was ERROR
  }

  TBOOL NodeConstantDef::packBitsInOrderOfDeclaration(u32& offset)
  {
    //do nothing, but override
    return TBOOL_TRUE;
  }

  void NodeConstantDef::assignConstantSlotIndex(u32& cslotidx)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    if(nut->isPrimitiveType()) //t41240
      {
	u32 slotsneeded = m_state.slotsNeeded(nuti);
	assert(m_constSymbol);
	((SymbolConstantValue *) m_constSymbol)->setConstantStackFrameAbsoluteSlotIndex(cslotidx);
	Node::makeRoomForSlots(slotsneeded, CNSTSTACK);
	setupStackWithPrimitiveForEval(slotsneeded);
	cslotidx += slotsneeded;
      }
    else if(m_state.isAClass(nuti) || m_state.isAtom(nuti)) //t41198, t41483,4
      {
	//array of classes??
	// eval doesn't support transients (> atom size) (t41231)
	// without a slot index, eval will return NOTREADY (-12) (t41266)
	ULAMCLASSTYPE nclasstype = nut->getUlamClassType();
	if((nclasstype == UC_ELEMENT) || (nclasstype == UC_QUARK) || ((nclasstype == UC_TRANSIENT) && (nut->getTotalBitSize() <= MAXSTATEBITS)) || m_state.isAtom(nuti))
	  {
	    u32 slotsneeded = m_state.slotsNeeded(nuti);
	    assert(m_constSymbol);
	    ((SymbolConstantValue *) m_constSymbol)->setConstantStackFrameAbsoluteSlotIndex(cslotidx);
	    Node::makeRoomForSlots(slotsneeded, CNSTSTACK);
	    setupStackWithConstantClassForEval(slotsneeded);
	    cslotidx += slotsneeded;
	  }
      }
  } //assignConstantSlotIndex

  void NodeConstantDef::setupStackWithPrimitiveForEval(u32 slots)
  {
    //similar to NodeVarDecl method
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    assert(m_constSymbol->getUlamTypeIdx() == nuti);
    PACKFIT packFit = nut->getPackable();
    if(packFit == PACKEDLOADABLE)
      {
	u64 dval = 0;
	AssertBool gotVal = m_constSymbol->getValueReadyToPrint(dval);
	assert(gotVal);

	UlamValue immUV;
	u32 len = nut->getTotalBitSize();
	if(len <= MAXBITSPERINT)
	  immUV = UlamValue::makeImmediate(nuti, (u32) dval, m_state);
	else if(len <= MAXBITSPERLONG)
	  immUV = UlamValue::makeImmediateLong(nuti, dval, m_state);
	else
	  m_state.abortGreaterThanMaxBitsPerLong();

	m_state.m_constantStack.storeUlamValueAtStackIndex(immUV, ((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex());
      }
    else if(!m_nodeExpr)
      {
	//unpacked primitive array - defaults no eval
	UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	u32 baseslot =  ((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex();

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
	    m_state.m_constantStack.storeUlamValueAtStackIndex(immUV, baseslot + j);
	  }
      }
    else if(m_nodeExpr->isAList())
      {
	//unpacked primitive array - uses eval
	UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	u32 baseslot =  ((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex();

	UlamValue immUV;
	u32 itemlen = nut->getBitSize();
	if(itemlen <= MAXBITSPERINT)
	  immUV = UlamValue::makeImmediate(scalaruti, 0, m_state);
	else if(itemlen <= MAXBITSPERLONG)
	  immUV = UlamValue::makeImmediateLong(scalaruti, 0, m_state);
	else
	  m_state.abortGreaterThanMaxBitsPerLong();

	u32 n = ((NodeList *) m_nodeExpr)->getNumberOfNodes(); //may be fewer
	if(n == 0)
	  {
	    for(u32 j = 0; j < slots; j++)
	      m_state.m_constantStack.storeUlamValueAtStackIndex(immUV, baseslot + j);
	  }
	else
	  {
	    for(u32 j = 0; j < slots; j++)
	      {
		UlamValue itemUV;
		evalNodeProlog(0); //new current frame pointer
		makeRoomForNodeType(scalaruti); //offset a constant expression
		u32 k = j < n ? j : n - 1; //repeat last initializer if fewer
		EvalStatus evs = ((NodeList *) m_nodeExpr)->eval(k);
		assert(evs == NORMAL);

		itemUV = m_state.m_nodeEvalStack.popArg();
		evalNodeEpilog();

		m_state.m_constantStack.storeUlamValueAtStackIndex(itemUV, baseslot + j);
	      }
	  }
      }
    else //not a list, not packedloadable
      {
	assert(m_constSymbol->isClassArgument() || !nut->isScalar()); //?? t41277 String array
	//m_nodeExpr is NodeConstantArray, access items like a NodeSquareBracket
	u32 baseslot =  ((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex();

	evalNodeProlog(0); //new current frame pointer
	makeRoomForSlots(1); //always 1 slot for ptr
	EvalStatus evs = m_nodeExpr->evalToStoreInto();
	assert(evs == NORMAL);

	UlamValue pluv = m_state.m_nodeEvalStack.popArg();
	assert(m_state.isPtr(pluv.getUlamValueTypeIdx()) && (UlamType::compare(pluv.getPtrTargetType(), nuti, m_state) == UTIC_SAME));

	for(u32 j = 0; j < slots; j++)
	  {
	    UlamValue itemUV = pluv.getValAt(j, m_state);
	    m_state.m_constantStack.storeUlamValueAtStackIndex(itemUV, baseslot + j);
	  }
	evalNodeEpilog();
      }
  } //setupStackWithPrimitiveForEval

  void NodeConstantDef::setupStackWithConstantClassForEval(u32 slots)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    assert(m_constSymbol->getUlamTypeIdx() == nuti);

    //assert(nut->isScalar()); t41261
    //assert(slots == 1); //quark or element fit in one slot. transient total bitsize < 71.
    assert(m_nodeExpr); //empty init is empty list, not null (t41262); could be a NodeConstantClass

    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    assert((classtype == UC_QUARK) || (classtype == UC_ELEMENT) || ((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() <= MAXSTATEBITS)) || m_state.isAtom(nuti));

    PACKFIT packFit = nut->getPackable();
    if((packFit == PACKEDLOADABLE))
      {
	u64 dval = 0;
	AssertBool gotVal = m_constSymbol->getValueReadyToPrint(dval);

	UlamValue immUV;
	u32 len = nut->getTotalBitSize();
	if(len <= MAXBITSPERINT)
	  immUV = UlamValue::makeImmediateClass(nuti, (u32) dval, len);
	else if(len <= MAXBITSPERLONG)
	  immUV = UlamValue::makeImmediateLongClass(nuti, dval, len);
	else
	  m_state.abortGreaterThanMaxBitsPerLong();

	m_state.m_constantStack.storeUlamValueAtStackIndex(immUV, ((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex());
      }
    else if(m_nodeExpr->isAList() && ((NodeList*) m_nodeExpr)->isEmptyList())
      {
	//unpacked, no inits for class, use default; support arrays
	UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	u32 baseslot =  ((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex();

	UlamValue defaultUV;
	u32 itemlen = nut->getBitSize();

	if(classtype == UC_QUARK) //(t41262)
	  {
	    assert(itemlen <= MAXBITSPERINT);
	    u32 dval = 0;
	    AssertBool gotDefault = m_state.getDefaultQuark(nuti, dval); //uses scalar uti
	    //assert(gotDefault); //too dramatic for eval
	    defaultUV = UlamValue::makeImmediateClass(scalaruti, (u32) dval, itemlen);
	  }
	else if(classtype == UC_ELEMENT)
	  defaultUV = UlamValue::makeDefaultAtom(scalaruti, m_state);
	else
	  m_state.abortShouldntGetHere();

	for(u32 j = 0; j < slots; j++)
	  {
	    m_state.m_constantStack.storeUlamValueAtStackIndex(defaultUV, baseslot + j);
	  }
      }
    else //not packedloadable w inits for class/classarray
      {
	UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	u32 baseslot =  ((SymbolConstantValue *) m_constSymbol)->getConstantStackFrameAbsoluteSlotIndex();
	BV8K bvclass;
	AssertBool gotVal = m_constSymbol->getValueReadyToPrint(bvclass);
	assert(gotVal);

	u32 itemlen = nut->getBitSize();  //not 96 for elements

	for(u32 j = 0; j < slots; j++)
	  {
	    UlamValue classUV;
	    if(classtype == UC_QUARK) //t41261
	      {
		u32 qval = bvclass.Read((j * itemlen), itemlen);
		classUV = UlamValue::makeImmediateClass(scalaruti, qval, itemlen);
	      }
	    else if(classtype == UC_ELEMENT) //(t41230,8,9 t41243)
	      {
		itemlen = BITSPERATOM;
		BV8K elval;
		bvclass.CopyBV(j * itemlen, 0u, itemlen, elval); //fmpos, topos, len, destbv
		classUV = UlamValue::makeAtom(scalaruti);
		classUV.putDataBig(0u, itemlen, elval); //1st arg was ATOMFIRSTSTATEBITPOS
	      }
	    else if(m_state.isAtom(scalaruti)) //(t41483,4)
	      {
		BV8K atval;
		bvclass.CopyBV(j * itemlen, 0u, itemlen, atval); //fmpos, topos, len, destbv
		classUV = UlamValue::makeAtom();
		classUV.putDataBig(0, itemlen, atval);

		UTI euti = Nouti;
		u32 elewcorr = classUV.getAtomElementTypeIdx();
		u32 eletype = 0;
		AssertBool gotele = Parity2D_4x4::Remove2DParity(elewcorr, eletype);
		assert(gotele);
		ELE_TYPE ele = (ELE_TYPE) eletype;
		if(ele != UNDEFINED_ELEMENT_TYPE)
		  {
		    euti = m_state.lookupClassByElementType(ele);
		    if(euti != Nouti)
		      {
			assert(m_state.isAClass(euti));
			classUV.setUlamValueEffSelfTypeIdx(euti);
		      }
		  }
		else
		  classUV.setUlamValueEffSelfTypeIdx(m_state.getEmptyElementUTI());
	      }
	    else
	      m_state.abortNotImplementedYet(); //transient array?

	    m_state.m_constantStack.storeUlamValueAtStackIndex(classUV, baseslot + j);
	  }
      }
  } //setupStackWithConstantClassForEval

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
    assert(m_constSymbol);
    assert(m_state.isComplete(nuti));
    assert(UlamType::compare(m_constSymbol->getUlamTypeIdx(), nuti, m_state) == UTIC_SAME); //sanity check
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etyp = nut->getUlamTypeEnum();

    if(!nut->isScalar())
      {
	if(m_constSymbol->isLocalsFilescopeDef() ||  m_constSymbol->isDataMember())
	  {
	    //as a "data member", locals filescope, or class arguement:
	    // initialized in no-arg constructor (non-const)
	    m_state.indentUlamCode(fp);
	    fp->write("static ");
	    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write(";"); GCNL;
	  }
	else if(m_constSymbol->isClassArgument())
	  {
	    m_state.indentUlamCode(fp);
	    fp->write("static "); //? t3894
	    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write(";"); GCNL;
	  }
	else if(!m_nodeExpr)
	  {
	    //all zeros (unless a named constant class tbd)
	    m_state.indentUlamCode(fp);
	    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write("("); // use default? constructor (not equals)
	    fp->write(");"); GCNL;
	    m_state.clearCurrentObjSymbolsForCodeGen();
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
      } //end constant arrays
    else if(etyp == String)
      {
	u32 sval;
	AssertBool gotVal = m_constSymbol->getValueReadyToPrint(sval);

	//output comment for scalar constant value
	m_state.indentUlamCode(fp);
	fp->write("//");
	fp->write_hexadecimal(sval);
	fp->write(" -> ");
	m_constSymbol->printPostfixValue(fp);
	GCNL;
      }
    else if((etyp == Class) || (etyp == UAtom)) //t41483
      {
	std::string estr;
	AssertBool gotVal = m_constSymbol->getClassValueAsHexString(estr);
	assert(gotVal);

	if(m_constSymbol->isLocalsFilescopeDef() ||  m_constSymbol->isDataMember() || m_constSymbol->isClassArgument())
	  {
	    //defined in .tcc, only declared in .h as static (a "classic" GOTCHA!!)
	    m_state.indentUlamCode(fp);
	    fp->write("static ");
	    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write(";");

	    //output comment for scalar constant class value (t41209)
	    fp->write("//");
	    fp->write(estr.c_str());
	    GCNL;
	  }
	else
	  {
	    //an immediate, named constant in a function (t41232)
	    u32 len = nut->getSizeofUlamType();
	    m_state.indentUlamCode(fp);
	    fp->write("const u32 _init");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    if(len == 0)
	      fp->write("[1] = { ");
	    else
	      {
		fp->write("[(");
		fp->write_decimal_unsigned(len); //== [nwords]
		fp->write(" + 31)/32] = { ");
	      }
	    fp->write(estr.c_str());
	    fp->write(" };\n");

	    m_state.indentUlamCode(fp); //non const
	    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write("(_init");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write("); // tmp for constant ");
	    fp->write(getName()); //comment
	    GCNL;
	  }
      } //else do nothing
    return; //done
  } //genCode

  void NodeConstantDef::genCodeConstantArrayInitialization(File * fp)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(nut->isScalar() && (nut->getUlamTypeEnum() != Class))
      return;

    //no-arg constructor to initialize const arrays using special method
    // (based on: 'Init' + constants's mangled name)
    m_state.m_currentIndentLevel+=2;
    m_state.indent(fp);
    fp->write(", ");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("(Init");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("())\n");
    m_state.m_currentIndentLevel-=2;
  } //genCodeConstantArrayInitialization

  void NodeConstantDef::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    //ULAMCLASSTYPE classtype = nut->getUlamClassType();
    ULAMTYPE etyp = nut->getUlamTypeEnum();

    if(nut->isScalar() && !((etyp == Class) || (etyp == UAtom)))//t41483
      return;

    //constant array: Class or Primitive (not class arg primitive) //t3894
    u32 len = nut->getSizeofUlamType();
    s32 arraysize = nut->getArraySize();
    arraysize = arraysize == NONARRAYSIZE ? 1 : arraysize;

    if(declOnly)
      {
	//unique typedef for this constant array (number of u32's)
	m_state.indent(fp);
	fp->write("typedef u32 TypeForInit");
	fp->write(m_constSymbol->getMangledName().c_str());
	if(len == 0)
	  fp->write("[1];\n"); //t41520
	else
	  {
	    fp->write("[(");
	    fp->write_decimal_unsigned(len);
	    fp->write(" + 31)/32];\n");
	  }

	//unique function to initialize STATIC constant (classes and array) "data members";
	// not called in class no-arg constructor, but in .tcc (t41198)
	m_state.indent(fp);
	fp->write("static ");
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

    ULAMTYPE netyp = nut->getUlamTypeEnum();
    bool isString = (netyp == String);
    if(isString)
      m_constSymbol->printPostfixValueArrayStringAsComment(fp); //t3953,4

    m_state.indent(fp);
    fp->write("static ");
    fp->write("u32 ");
    fp->write("initVal");
    if(len == 0)
      fp->write("[1] = ");
    else
      {
	fp->write("[(");
	fp->write_decimal_unsigned(len);
	fp->write(" + 31)/32] = ");
      }

    if((netyp == Class) || (netyp == UAtom)) //t41483
      {
	std::string estr;
	AssertBool gotVal = m_constSymbol->getClassValueAsHexString(estr);
	assert(gotVal);
	fp->write("{ ");
	fp->write(estr.c_str());
	fp->write(" };"); GCNL;
      }
    else //primitive array
      {
	m_constSymbol->printPostfixValue(fp);
	fp->write(";"); GCNL;
      }
    m_state.indent(fp);
    fp->write("return initVal;"); GCNL;

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} // (static) Init");
    fp->write(m_constSymbol->getMangledName().c_str()); GCNL;
    fp->write("\n");

    //As static constant, output initialization in .tcc, something like this:
    //template<class EC>
    //typename MFM::Ui_Uq_102204QBar10<EC> MFM::Uq_10104QFoo10<EC>::Uc_6c_qbar(MFM::Uq_10104QFoo10<EC>::InitUc_6c_qbar());
    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("typename MFM::");
    fp->write(nut->getLocalStorageTypeAsString().c_str()); //immediate
    fp->write(" MFM::");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("(MFM::");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::Init");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("()); //Def constant ");
    fp->write(getName()); GCNL;
    fp->write("\n");
  } //generateBuiltinConstantClassOrArrayInitializationFunction

  void NodeConstantDef::cloneAndAppendNode(std::vector<Node *> & cloneVec)
  {
    //include scalars for generated comments; arrays for constructor initialization
    NodeConstantDef * cloneofme = (NodeConstantDef *) this->instantiate();
    assert(cloneofme);
    SymbolConstantValue * csymptr = NULL;
    AssertBool isSym = this->getSymbolPtr((Symbol *&) csymptr);
    assert(isSym);
    ((NodeConstantDef *) cloneofme)->setSymbolPtr(csymptr); //another ptr to same symbol
    cloneVec.push_back(cloneofme);
  }

  void NodeConstantDef::generateTestInstance(File * fp, bool runtest)
  {}

  void NodeConstantDef::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {}

  void NodeConstantDef::addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers)
  {
    assert(m_constSymbol);
    assert(m_constSymbol->isReady());

    ConstantDesc * descptr = new ConstantDesc((SymbolConstantValue *) m_constSymbol, classType, m_state);
    assert(descptr);

    //replace m_memberName with Ulam Type and Name (t3343, edit)
    std::ostringstream mnstr;
    mnstr << m_state.m_pool.getDataAsString(getTypeNameId()).c_str();
    mnstr << " " << descptr->m_memberName;

    descptr->m_memberName = ""; //clear base init
    descptr->m_memberName = mnstr.str();

    //concat mangled class and parameter names to avoid duplicate keys into map
    std::ostringstream fullMangledName;
    fullMangledName << descptr->m_mangledClassName << "_" << descptr->m_mangledMemberName;
    classmembers.insert(std::pair<std::string, ClassMemberDescHolder>(fullMangledName.str(), ClassMemberDescHolder(descptr)));
  } //addMemberDescriptionToInfoMap

} //end MFM
