#include <stdlib.h>
#include "NodeConstant.h"
#include "NodeConstantArray.h"
#include "NodeConstantClass.h"
#include "NodeConstantClassArray.h"
#include "NodeIdent.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "NodeMemberSelect.h"
#include "NodeModelParameter.h"
#include "NodeTypeBitsize.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "SymbolModelParameterValue.h"
#include "SymbolTypedef.h"
#include "SymbolVariable.h"

namespace MFM {

  NodeIdent::NodeIdent(const Token& tok, SymbolVariable * symptr, CompilerState & state) : Node(state), m_token(tok), m_varSymbol(symptr), m_currBlockNo(0), m_currBlockPtr(NULL)
  {
    if(symptr)
      setBlockNo(symptr->getBlockNoOfST());
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeIdent::NodeIdent(const NodeIdent& ref) : Node(ref), m_token(ref.m_token), m_varSymbol(NULL), m_currBlockNo(ref.m_currBlockNo), m_currBlockPtr(NULL) {}

  NodeIdent::~NodeIdent(){}

  Node * NodeIdent::instantiate()
  {
    return new NodeIdent(*this);
  }

  void NodeIdent::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }

  const char * NodeIdent::getName()
  {
    return m_state.getTokenDataAsString(m_token).c_str();
  }

  const std::string NodeIdent::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeIdent::setSymbolPtr(SymbolVariable * vsymptr)
  {
    assert(vsymptr);
    m_varSymbol = vsymptr;
    setBlockNo(vsymptr->getBlockNoOfST());
    assert(m_currBlockNo);
  }

  bool NodeIdent::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return (m_varSymbol != NULL); //true not-null
  }

  bool NodeIdent::getSymbolPtr(SymbolVariable *& symptrref) const
  {
    symptrref = m_varSymbol;
    return (m_varSymbol != NULL); //true not-null
  }

  bool NodeIdent::getStorageSymbolPtr(Symbol *& symptrref)
  {
    UTI nuti = getNodeType();
    assert(m_state.okUTItoContinue(nuti));
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    //only atom, and elements, and quark refs, are considered 'storage';
    // not isAltRefType, could be ALT_AS (t3835)
    if((classtype == UC_ELEMENT) || ((classtype == UC_QUARK) && nut->isReference()) || m_state.isAtom(nuti))
      {
	symptrref = m_varSymbol;
	return true;
      }
    return false;
  }

  bool NodeIdent::hasASymbolDataMember()
  {
    assert(m_varSymbol);
    return m_varSymbol->isDataMember();
  }

  bool NodeIdent::hasASymbolSuper()
  {
    assert(m_varSymbol);
    return m_varSymbol->isSuper();
  }

  bool NodeIdent::hasASymbolSelf()
  {
    assert(m_varSymbol);
    return m_varSymbol->isSelf();
  }

  bool NodeIdent::hasASymbolReference()
  {
    assert(m_varSymbol);
    return m_state.isReference(m_varSymbol->getUlamTypeIdx()); //not isAltRefType, could be ALT_AS (t3835)
  }

  bool NodeIdent::hasASymbolReferenceConstant()
  {
    assert(hasASymbolReference());
    //alternatively, m_varSymbol->isFunctionParameter() && isConstantFunctionParameter()
    return (m_state.isConstantRefType(m_varSymbol->getUlamTypeIdx()));
  }

  void NodeIdent::setupBlockNo()
  {
    //define before used, start search with current block
    if(m_currBlockNo == 0)
      {
	if(m_state.useMemberBlock())
	  {
	    NodeBlockClass * memberclass = m_state.getCurrentMemberClassBlock();
	    assert(memberclass);
	    setBlockNo(memberclass->getNodeNo());
	  }
	else
	  setBlockNo(m_state.getCurrentBlockNo());
      }
  } //setupBlockNo

  void NodeIdent::setBlockNo(NNO n)
  {
    assert(n > 0);
    m_currBlockNo = n;
    m_currBlockPtr = NULL; //not owned, just clear
  }

  NNO NodeIdent::getBlockNo() const
  {
    return m_currBlockNo;
  }

  void NodeIdent::setBlock(NodeBlock * ptr)
  {
    m_currBlockPtr = ptr;
  }

  NodeBlock * NodeIdent::getBlock()
  {
    assert(m_currBlockNo);

    if(m_currBlockPtr)
      return m_currBlockPtr;

    //case where the typebitsize is an expression in a stub; trying to resolve pending arg;
    // context is location of stub useage; use this version of findNodeNo that checks
    // m_pendingArgStubContext and starts the search there if so (t3626);
    // note: the template has its NodeIdent surgerized, but after the stub parsing.
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClassStubFirst(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  const Token& NodeIdent::getToken() const
  {
    return m_token;
  }

  bool NodeIdent::belongsToVOWN(UTI vown)
  {
    if(vown == Nouti)
       return true; //non-virtual func, like vown, doesnt apply delta

    if(hasASymbolDataMember())
      {
	UTI dmclass = m_varSymbol->getDataMemberClass();
	return (UlamType::compare(dmclass, vown, m_state) == UTIC_SAME);
      }
    return false;
  }

  bool NodeIdent::isAConstant()
  {
    bool rtn = false;
    //may not have known at parse time; no side-effects until c&l
    if(!m_varSymbol)
      {
	//is it a constant within the member?
	NodeBlockContext * memberclass = m_state.getContextBlock();
	assert(memberclass);

	m_state.pushCurrentBlock(memberclass);

	Symbol * asymptr = NULL;
	bool hazyKin = false;
	u32 tokid = m_state.getTokenDataAsStringId(m_token);
	if(m_state.alreadyDefinedSymbol(tokid, asymptr, hazyKin))
	  {
	    rtn = asymptr->isConstant();
	  }
	m_state.popClassContext(); //restore
      }
    else
      rtn = m_varSymbol->isConstant();
    return rtn;
  } //isAConstant

  void NodeIdent::setClassType(UTI cuti)
  {
    //noop
  }

  FORECAST NodeIdent::safeToCastTo(UTI newType)
  {
    //ulamtype checks for complete, non array, and type specific rules
    return m_state.getUlamTypeByIndex(newType)->safeCast(Node::getNodeType());
  } //safeToCastTo

  UTI NodeIdent::checkAndLabelType()
  {
    UTI it = Nouti;  //init (was Nav)
    u32 errCnt = 0;

    setupBlockNo(); //in case zero

    //checkForSymbol:
    //2 cases: use was before def, look up in class block; cloned unknown
    if(m_varSymbol == NULL)
      {
	UTI cuti = m_state.getCompileThisIdx(); //for error messages
	NodeBlock * currBlock = getBlock();
	setBlock(currBlock);

	if(m_state.useMemberBlock())
	  {
	    m_state.pushCurrentBlock(currBlock); //e.g. memberselect needed for already defined symbol
	    NodeBlockClass * memberblock = m_state.getCurrentMemberClassBlock();
	    assert(memberblock);
	    cuti = memberblock->getNodeType();
	  }
	else
	  m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

	Symbol * asymptr = NULL;
	bool hazyKin = false;
	u32 tokid = m_state.getTokenDataAsStringId(m_token);
	// must capture symbol ptr even if part of incomplete chain to do any necessary surgery
	// (e.g. stub class args t3526, t3525, inherited dm t3408), wait if hazyKin (t3572)?;
	if(m_state.alreadyDefinedSymbol(tokid, asymptr, hazyKin))
	  {
	    if(!asymptr->isFunction() && !asymptr->isTypedef() && !asymptr->isConstant() && !asymptr->isModelParameter())
	      {
		//check hazyiness after determined no surgery required
		if(!hazyKin)
		  {
		    setSymbolPtr((SymbolVariable *) asymptr);
		    //assert(asymptr->getBlockNoOfST() == m_currBlockNo); not necessarily true
		    // e.g. var used before defined, and then is a data member outside current func block.
		    setBlockNo(asymptr->getBlockNoOfST()); //refined
		    setBlock(currBlock);
		  }
		else
		  {
		    std::ostringstream msg;
		    msg << "Identifier '" << m_state.getTokenDataAsString(m_token).c_str();
		    msg << "' was used while still undeclared";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		    it = Hzy; //t3572, t3597, t41183, and ulamexports QBox
		    errCnt++;
		  }
		m_state.popClassContext(); //restore t3102
	      }
	    else
	      {
		if(replaceOurselves(asymptr))
		  {
		    m_state.popClassContext(); //restore
		    m_state.setGoAgain();

		    delete this; //suicide is painless..

		    return Hzy;
		  }
		else
		  {
		    std::ostringstream msg;
		    msg << "(1) '" << m_state.getTokenDataAsString(m_token).c_str();
		    msg << "' is not a variable, and cannot be used as one with class: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    it = Nav;
		    errCnt++;
		    m_state.popClassContext(); //restore
		  }
	      }
	  }
	else
	  {
	    //not found, look again...(t41344,5)
	    TBOOL foundit = lookagainincaseimplicitselfchanged(); //TBOOL_HAZY is good!
	    if(foundit != TBOOL_TRUE)
	      {
		std::ostringstream msg;
		msg << "Variable '" << m_state.getTokenDataAsString(m_token).c_str();
		msg << "' is not defined, or was used before declared in a function";
		if((foundit != TBOOL_HAZY))
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    it = Nav;
		  }
		else
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		    it = Hzy;
		  }
		errCnt++;
		//m_state.popClassContext(); //restore
	      }
	    m_state.popClassContext(); //restore
	  }
      } //lookup symbol done
    else
      {
	if(replaceOurselves(m_varSymbol))
	  {
	    m_state.setGoAgain();

	    delete this; //suicide is painless..

	    return Hzy;
	  }
	//else continue
      }

    if(!errCnt && m_varSymbol)
      {
	it = m_varSymbol->getUlamTypeIdx();
	Node::setStoreIntoAble(m_varSymbol->isConstant() ? TBOOL_FALSE : TBOOL_TRUE); //store into an array entotal? t3881
	if(m_varSymbol->isFunctionParameter() && ((SymbolVariableStack *) m_varSymbol)->isConstantFunctionParameter())
	  Node::setStoreIntoAble(TBOOL_FALSE); //as well as its referenceablity (t41186,8,9)

	if(m_state.isConstantRefType(it))
	  Node::setStoreIntoAble(TBOOL_FALSE); //as well as its referenceablity t41192

	//from NodeTypeDescriptor..e.g. for function call args in NodeList.
	if(!m_state.isComplete(it))
	  {
	    // if Nav, use token
	    UTI mappedUTI = it;
	    UTI cuti = m_state.getCompileThisIdx();

	    // the symbol associated with this type, was mapped during instantiation
	    // since we're call AFTER that (not during), we can look up our
	    // new UTI and pass that on up the line of NodeType Selects, if any.
	    if(m_state.mappedIncompleteUTI(cuti, it, mappedUTI))
	      {
		std::ostringstream msg;
		msg << "Substituting Mapped UTI" << mappedUTI;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
		msg << " for incomplete list type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << "' UTI" << it << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		m_state.mapTypesInCurrentClass(it, mappedUTI); //before setting equal?
		m_varSymbol->resetUlamType(mappedUTI); //consistent!
		it = mappedUTI;
	      }
	    else if(m_varSymbol->isSelf() || m_state.isAltRefType(it) || m_varSymbol->isSuper())
	      {
		m_state.completeAReferenceType(it);
	      }

	    if(!m_state.isComplete(it)) //reloads to recheck for debug message
	      {
		std::ostringstream msg;
		msg << "Incomplete identifier for type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << ", used with list symbol name '" << getName() << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		it = Hzy; //missing t3754 case 1
	      }
	  }
      }

    if(m_state.okUTItoContinue(it) && m_varSymbol)
      it = specifyimplicitselfexplicitly();

    if(m_state.okUTItoContinue(it) && m_varSymbol)
      it = checkUsedBeforeDeclared();

    setNodeType(it);
    if(it == Hzy) m_state.setGoAgain();
    return it;
  } //checkAndLabelType

  bool NodeIdent::replaceOurselves(Symbol * symptr)
  {
    assert(symptr);

    bool rtnb = false;
    UTI suti = symptr->getUlamTypeIdx();
    if(symptr->isConstant() && !m_state.isConstantRefType(suti))
      {
	// replace ourselves with a constant node instead;
	// same node no, and loc (e.g. t3573)
	Node * newnode = NULL;
	if(m_state.isAClass(suti))
	  {
	    if(m_state.isScalar(suti))
	      newnode = new NodeConstantClass(m_token, (SymbolWithValue *) symptr, NULL, m_state);
	    else
	      newnode = new NodeConstantClassArray(m_token, (SymbolWithValue *) symptr, NULL, m_state); //t41261
	  }
	else if(m_state.isScalar(suti))
	  newnode = new NodeConstant(m_token, (SymbolWithValue *) symptr, NULL, m_state);
	else
	  newnode = new NodeConstantArray(m_token, (SymbolWithValue *) symptr, NULL, m_state);
	assert(newnode);

	AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
	assert(swapOk);

	rtnb = true;
      }
    else if(symptr->isModelParameter())
      {
	// replace ourselves with a parameter node instead;
	// same node no, and loc
	NodeModelParameter * newnode = new NodeModelParameter(m_token, (SymbolModelParameterValue*) symptr, NULL, m_state);
	assert(newnode);

	AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
	assert(swapOk);

	rtnb = true;
      }
    //else did not replace ourselves
    return rtnb;
  } //replaceOurselves

  TBOOL NodeIdent::lookagainincaseimplicitselfchanged()
  {
    TBOOL rtn = TBOOL_FALSE;

    Symbol * symptr = NULL;
    bool hazyKin = false;

    if(!m_state.useMemberBlock())
      {
	//not found in current context, perhaps 'self' has changed scope (t41344)
	u32 selfid = m_state.m_pool.getIndexForDataString("self");
	Symbol * selfsym = NULL;
	bool hazykin = false; //unused
	bool gotSelf = m_state.alreadyDefinedSymbolHere(selfid, selfsym, hazykin);
	if(gotSelf)
	  {
	    UTI selfuti = selfsym->getUlamTypeIdx();
	    SymbolClass * csym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(selfuti, csym);
	    assert(isDefined);

	    NodeBlockClass * memberClassNode = csym->getClassBlockNode();
	    assert(memberClassNode);

	    UTI selfblockuti = memberClassNode->getNodeType();
	    if(m_state.okUTItoContinue(selfblockuti))
	      {
		//set up compiler state to use the member class block for variable
		m_state.pushClassContextUsingMemberClassBlock(memberClassNode);

		u32 tokid = m_state.getTokenDataAsStringId(m_token);
		bool foundit = m_state.alreadyDefinedSymbol(tokid, symptr, hazyKin);
		if(foundit)
		  rtn = TBOOL_TRUE;

		m_state.popClassContext(); //restore
	      }
	    else if(selfblockuti == Nav)
	      rtn = TBOOL_FALSE;
	    else if(selfblockuti == Hzy)
	      rtn = TBOOL_HAZY;
	    else if(selfblockuti == Nouti)
	      rtn = TBOOL_HAZY;
	    //else
	  } //ends gotself (t3415,t3431,t3455,t3460,t41283)
      } //ends not using memberblock

    if(rtn == TBOOL_TRUE)
      {
	setSymbolPtr((SymbolVariable *) symptr);
	setBlockNo(symptr->getBlockNoOfST()); //refined
	setBlock(NULL);
	UTI it = specifyimplicitselfexplicitly(); //returns Hzy
	if(it == Hzy)
	  rtn = TBOOL_HAZY;
	else if(!m_state.okUTItoContinue(it))
	  rtn = TBOOL_FALSE;
	//else still TBOOL_TRUE
      }
    return rtn;
  } //lookagainincaseimplicitselfchanged

  UTI NodeIdent::specifyimplicitselfexplicitly()
  {
    assert(m_varSymbol);
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    if(!m_varSymbol->isDataMember())
      return vuti; //no change

    if(m_varSymbol->isTmpVarSymbol())
      return vuti; //no change

    if(m_state.useMemberBlock())
      {
	return vuti; //t3337 (e.g. t.m_arr[1]) parent is sqbkt, parent's parent is '.'
      }

    //a data member/func call needs to be rhs of member select "."
    NodeBlock * currBlock = getBlock();

    NNO pno = Node::getYourParentNo();

    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock); //push again

    Node * parentNode = m_state.findNodeNoInThisClassForParent(pno);
    assert(parentNode);

    m_state.popClassContext(); //restore

    bool implicitself = true;

    if(parentNode->isAMemberSelect())
      {
	Symbol * rhsym = NULL;
	if(!parentNode->getSymbolPtr(rhsym))
	  vuti = Hzy; //t41152

	implicitself = (rhsym != m_varSymbol); //rhsym null wont match
      }
    //else

    if(!implicitself)
      return vuti; //done

    Token selfTok(TOK_KW_SELF, m_token.m_locator, 0);
    NodeIdent * explicitself = new NodeIdent(selfTok, NULL, m_state);
    assert(explicitself);
    explicitself->setNodeLocation(getNodeLocation());

    NodeMemberSelect * newnode = new NodeMemberSelect(explicitself, this, m_state);
    assert(newnode);
    NNO newnodeno = newnode->getNodeNo(); //for us after swap

    AssertBool swapOk = Node::exchangeNodeWithParent(newnode);
    assert(swapOk);

    resetNodeNo(newnodeno); //moved before update lineage (t3337)

    //redo look-up given explicit self
    m_varSymbol = NULL;
    m_currBlockNo = 0;

    //reusing this, no suicide
    return Hzy;
  } //specifyimplicitselfexplicitly

  UTI NodeIdent::checkUsedBeforeDeclared()
  {
    assert(m_varSymbol);
    UTI rtnuti = m_varSymbol->getUlamTypeIdx();

    if(m_varSymbol->isSuper())
      {
	return rtnuti; //short circuit as-cond super (t41338)
      }

    if(!m_varSymbol->isDataMember() && (((SymbolVariableStack *) m_varSymbol)->getDeclNodeNo() > getNodeNo()))
      {
	NodeBlock * currBlock = getBlock();
	currBlock = currBlock->getPreviousBlockPointer();
	std::ostringstream msg;
	msg << "Local variable '" << getName();
	msg << "' was used before declared";
	if(currBlock)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setBlockNo(currBlock->getNodeNo());
	    m_varSymbol = NULL; //t3881, t3306, t3323
	    rtnuti = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnuti = Nav; //error/t3797
	  }
      }
    return rtnuti;
  } //checkUsedBeforeDeclared

  bool NodeIdent::trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    //self as good as element until runtime t3905
    if((nut->getUlamClassType() == UC_ELEMENT) || m_varSymbol->isSelf())
      {
	if(fromleftnode)
	  *fromleftnode = NULL; //clear for future deletion
	rtnnodeptr = this;
	return true;
      }
    return false;
  } //trimToTheElement

  EvalStatus NodeIdent::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE);

    evalNodeProlog(0); //new current frame pointer

    //return the ptr for an array; square bracket will resolve down to the immediate data
    UlamValue uv;
    UlamValue uvp = makeUlamValuePtr();

    if(m_state.isScalar(nuti))
      {
	if(m_state.isAltRefType(uvp.getPtrTargetType()))
	  uvp = m_state.getPtrTarget(uvp);

	uv = m_state.getPtrTarget(uvp);
	UTI ttype = uv.getUlamValueTypeIdx();

	// redo what getPtrTarget use to do, when types didn't match due to
	// an element/quark or a requested scalar of an arraytype
	if(m_state.isPtr(ttype) || (UlamType::compare(ttype, nuti, m_state) == UTIC_NOTSAME))
	  {
	    if(m_state.isClassACustomArray(nuti))
	      {
		UTI caType = m_state.getAClassCustomArrayType(nuti);
		UlamType * caut = m_state.getUlamTypeByIndex(caType);
		if(m_state.isAtom(caType) || (caut->getBitSize() > MAXBITSPERINT))
		  {
		    uv = uvp; //customarray
		  }
		else
		  {
		    s32 len = uvp.getPtrLen();
		    assert(len != UNKNOWNSIZE);
		    if(len <= MAXBITSPERINT)
		      {
			u32 datavalue = uv.getDataFromAtom(uvp, m_state);
			uv = UlamValue::makeImmediate(nuti, datavalue, m_state);
		      }
		    else if(len <= MAXBITSPERLONG)
		      {
			u64 datavalue = uv.getDataLongFromAtom(uvp, m_state);
			uv = UlamValue::makeImmediateLong(nuti, datavalue, m_state);
		      }
		    else
		      m_state.abortGreaterThanMaxBitsPerLong();
		  }
	      }
	    else
	      {
		UlamType * nut = m_state.getUlamTypeByIndex(nuti);
		if((m_state.isAtom(nuti) || (classtype == UC_ELEMENT)) && (nut->isScalar() || nut->isAltRefType()))
		  {
		    uv = m_state.getPtrTarget(uvp); //error/t3676, error/t3677
		  }
		else
		  {
		    //includes (evaluable) transients (t3739)
		    UTI vuti = uv.getUlamValueTypeIdx();
		    // does this handle a ptr to a ptr (e.g. "self")? (see makeUlamValuePtr)
		    if(m_state.isPtr(vuti))
		      {
			uvp = uv;
			uv = m_state.getPtrTarget(uvp);
		      }

		    s32 len = uvp.getPtrLen();
		    assert(len != UNKNOWNSIZE);
		    if(len <= MAXBITSPERINT)
		      {
			u32 datavalue = uv.getDataFromAtom(uvp, m_state);
			uv = UlamValue::makeImmediate(nuti, datavalue, m_state);
		      }
		    else if(len <= MAXBITSPERLONG)
		      {
			u64 datavalue = uv.getDataLongFromAtom(uvp, m_state);
			uv = UlamValue::makeImmediateLong(nuti, datavalue, m_state);
		      }
		    else
		      m_state.abortGreaterThanMaxBitsPerLong();
		  }
	      }
	  } // not node type
      } //scalar
    else
      uv = uvp; //even if reference???

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeIdent::evalToStoreInto()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    assert(m_varSymbol);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE);

    TBOOL stor = Node::getStoreIntoAble();

    //the first reason for ALT_CONSTREF when called from evalArgumentsInReverseOrder
    // allow constant classes (t41198)
    if((stor != TBOOL_TRUE) && !m_state.isConstantRefType(nuti)) //i.e. an MP
      {
	if(m_varSymbol->isDataMember() || !((SymbolVariableStack *) m_varSymbol)->isConstantFunctionParameter())
	  {
	    std::ostringstream msg;
	    msg << "Variable '";
	    msg << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "' is not a valid lefthand side. Eval FAILS";
	    if(stor == TBOOL_HAZY)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		return evalStatusReturnNoEpilog(NOTREADY);
	      }
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return evalErrorReturn();
	  }
	//else continue if a constant function parameter; how a DM? (t41239)
      }

    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();

    //must remain a ptr!!!
    if(m_state.isAltRefType(rtnUVPtr.getPtrTargetType()) && (rtnUVPtr.getPtrStorage() == STACK))
      {
	UlamValue tmpref = m_state.getPtrTarget(rtnUVPtr);
	 if(tmpref.isPtr())
	   rtnUVPtr = tmpref;
	 //else no change? (e.g. t3407)
      }

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeIdent::makeUlamValuePtr()
  {
    if(m_varSymbol->isSelf())
      {
	// when "self" is a quark, we're inside a func called on a quark (e.g. dm or local)
	//'atomof' gets entire atom/element containing this quark; including its type!
	//'self' gets type/pos/len of the quark from which 'atom' can be extracted
	UlamValue selfuvp = m_state.m_currentSelfPtr;
	UTI ttype = selfuvp.getPtrTargetType();
	assert(m_state.okUTItoContinue(ttype));
	return selfuvp;
      } //done
    else if(m_varSymbol->isSuper())
      {
	// manufactured super for self as-cond. (t41338)
	UlamValue selfuvp = m_state.m_currentSelfPtr;
	UTI selfttype = selfuvp.getPtrTargetType();
	assert(m_state.okUTItoContinue(selfttype));
	UTI supertype = m_varSymbol->getUlamTypeIdx(); //ALT_AS
	u32 relposofsuper = 0;
	AssertBool gotpos = m_state.getABaseClassRelativePositionInAClass(selfttype, supertype, relposofsuper);
	assert(gotpos);
	selfuvp.setPtrPos(selfuvp.getPtrPos() + relposofsuper);
	selfuvp.setPtrTargetType(m_state.getUlamTypeAsDeref(supertype));
	selfuvp.setPtrLen(m_state.getBaseClassBitSize(supertype));
	selfuvp.setPtrNameId(0);
	return selfuvp; //now superuvp.
      } //done

    // can't use global m_currentAutoObjPtr, since there might be nested as conditional blocks.
    // NodeVarDecl for this autolocal sets AutoPtrForEval during its eval. Unlike ALT_AS,
    // ALT_REF, ALT_CONSTREF, ALT_ARRAYITEM cannot guarantee its NodeVarRef init was last encountered.
    if(m_varSymbol->getAutoLocalType() == ALT_AS)
      return ((SymbolVariableStack *) m_varSymbol)->getAutoPtrForEval(); //haha! we're done.

    UlamValue ptr;
    if(m_varSymbol->isDataMember())
      {
	UTI objclass = m_state.m_currentObjPtr.getPtrTargetType();
	UTI dmclass = m_varSymbol->getDataMemberClass();
	assert((UlamType::compareForUlamValueAssignment(dmclass, objclass, m_state) == UTIC_SAME) || m_state.isClassASubclassOf(objclass, dmclass)); //sanity, right? t3915
	// return ptr to this data member within the m_currentObjPtr
	// 'pos' modified by this data member symbol's packed bit position;
	// and relative position of its class in m_currentObjPtr (ulam-5);
	u32 relposofbase = 0;
	AssertBool gotpos = m_state.getABaseClassRelativePositionInAClass(objclass, dmclass, relposofbase);
	assert(gotpos);

	ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset() + relposofbase, m_varSymbol->getId());

	ptr.checkForAbsolutePtr(m_state.m_currentObjPtr); //t3810
      }
    else
      {
	//DEBUG ONLY!!, to view ptr saved with Ref's m_varSymbol.
#if 0
	if(m_varSymbol->isAutoLocal()) //ALT_REF, ALT_CONSTREF or ALT_ARRAYITEM
	  ptr = ((SymbolVariableStack *) m_varSymbol)->getAutoPtrForEval();
#endif
	//local variable on the stack; could be array ptr! could be 'super'
	ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
      }
    return ptr;
  } //makeUlamValuePtr

  //new
  void NodeIdent::makeUVPassForCodeGen(UVPass& uvpass)
  {
    assert(m_varSymbol);
    s32 tmpnum = m_state.getNextTmpVarNumber();

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    assert(UlamType::compare(nuti, m_varSymbol->getUlamTypeIdx(), m_state) == UTIC_SAME);

    u32 pos = uvpass.getPassPos(); //t41184
    if(m_varSymbol->isDataMember())
      {
	// 'pos' modified by this data member symbol's packed bit position;
	// except for array items, i.e. tmprefsymbols (t3910)
	// adjusted if dm of (unshared) base class by rel pos (t41320)
	if(!m_varSymbol->isTmpVarSymbol())
	  pos += m_varSymbol->getPosOffset();

	//might already be true when MemberSelectByBaseType; don't clobber.
	bool applydelta = uvpass.getPassApplyDelta(); //t41318

	uvpass = UVPass::makePass(tmpnum, nut->getTmpStorageTypeForTmpVar(), nuti, m_state.determinePackable(nuti), m_state, pos, applydelta, m_varSymbol->getId());
      }
    else
      {
	//local variable on the stack; could be array ptr!
	uvpass = UVPass::makePass(tmpnum, nut->getTmpStorageTypeForTmpVar(), nuti, m_state.determinePackable(nuti), m_state, pos, false, m_varSymbol->getId());
      }
  } //makeUVPassForCodeGen

  bool NodeIdent::installSymbolTypedef(TypeArgs& args, Symbol *& asymptr)
  {
    bool brtn = false;
    UTI tduti = Nav;
    UTI tdscalaruti = Nav;
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    assert(m_token.m_type == TOK_TYPE_IDENTIFIER);
    if(m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr))
      {
	tduti = asymptr->getUlamTypeIdx();
	if(asymptr->isTypedef())
	  {
	    if(m_state.isHolder(tduti))
	      {
		//not Nav when tduti's an array; might know?
		args.m_declListOrTypedefScalarType = tdscalaruti;

		UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
		ULAMCLASSTYPE tclasstype = tdut->getUlamClassType();
		// keep the out-of-band name; other's might refer to its UTI.
		// if its UTI is a unseen class, we can update the name of the class later
		// don't want to rush this step since we might have a class w args and diff UTI.
		// furthermore, unseen class may not be a class at all (e.g. dot assumption wrong .maxof)
		if((tclasstype == UC_NOTACLASS) || (tclasstype == UC_UNSEEN))
		  {
		    ULAMTYPE bUT = m_state.getBaseTypeFromToken(args.m_typeTok);

		    // if not a class, but a primitive type update the key
		    if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
		      {
			if(args.m_bitsize == 0)
			  args.m_bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

			// update the type of holder key
			UlamKeyTypeSignature newkey(m_state.getTokenAsATypeNameId(args.m_typeTok), args.m_bitsize, args.m_arraysize, Nouti, args.m_declRef);
			m_state.makeUlamTypeFromHolder(newkey, bUT, tduti, UC_NOTACLASS); //update key, same uti

			if(m_state.hasUnknownTypeInThisClassResolver(tduti))
			  m_state.removeKnownTypeTokenFromThisClassResolver(tduti);
		      }
		    else
		      {
			//update holder key with name_id and possible array (UNKNOWNSIZE)
			//possibly a class (t3379)
			UlamKeyTypeSignature newkey(m_state.getTokenAsATypeNameId(args.m_typeTok), args.m_bitsize, args.m_arraysize, args.m_classInstanceIdx, args.m_declRef);

			if(bUT == Class)
			  {
			    m_state.makeAnonymousClassFromHolder(tduti, args.m_typeTok.m_locator); //t41058, t3862, t3865, t41009, t3385

			    //calls updateUTIAlias like UNSEEN in next clause
			    if(args.m_classInstanceIdx != Nouti)
			      m_state.updateUTIAliasForced(tduti, args.m_classInstanceIdx);
			  }
			else
			  m_state.makeUlamTypeFromHolder(newkey, bUT, tduti, UC_NOTACLASS); //update holder key, same uti (no SymbolClassName needed)
		      }
		  }
		else if(tclasstype == UC_UNSEEN)
		  {
		    if(m_state.isThisLocalsFileScope() && args.m_classInstanceIdx != Nouti)
		      m_state.updateUTIAliasForced(tduti, args.m_classInstanceIdx); //t3874
		    //else (t3378, t3379)
		  }
		brtn = true;
	      } //holder done
	    else
	      brtn = true;
	  } //a typedef already there
	return brtn; //already there, and updated
      }

    SymbolClassName * prematureclass = NULL;
    bool isUnseenClass = false;
    UTI pmcuti = Nouti;
    if(m_state.alreadyDefinedSymbolClassName(m_token.m_dataindex, prematureclass))
      {
	pmcuti = prematureclass->getUlamTypeIdx();
	isUnseenClass = (m_state.getUlamTypeByIndex(pmcuti)->getUlamClassType() == UC_UNSEEN);

	std::ostringstream msg;
	msg << "Typedef alias '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' already exists as ";
	if(isUnseenClass)
	  msg << "an unseen ";
	msg << "class type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(pmcuti).c_str();
	msg << ", first noticed at: .";  //..
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //issue 5/6/16

	NodeBlockClass * ucblock = prematureclass->getClassBlockNode();
	assert(ucblock);
	std::ostringstream imsg;
	imsg << ".. Another typedef for '";
 	imsg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	imsg << "' visible from here might clear the ambiguity"; //t41008
	MSG(ucblock->getNodeLocationAsString().c_str(), imsg.str().c_str(), ERR); //ish 5/6/16,11/16/17

	return false; //quit!
      }

    if(args.m_anothertduti != Nouti)
      {
	//typedef might have bitsize and arraysize info..
	if(checkTypedefOfTypedefSizes(args, args.m_anothertduti)) //ref
	  {
	    tduti = args.m_anothertduti;
	    brtn = true;
	  }
      }
    else if(m_state.getUlamTypeByTypedefName(m_state.getTokenDataAsStringId(args.m_typeTok), tduti, tdscalaruti)) //t3674 Self;
      {
	args.m_declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	if(checkTypedefOfTypedefSizes(args, tduti)) //ref
	  {
	    brtn = true;
	  }
      }
    else if(args.m_declListOrTypedefScalarType != Nouti)
      {
	// if m_anothertduti fails first (this could be the scalar for it!)
	if(!checkTypedefOfTypedefSizes(args, args.m_declListOrTypedefScalarType))
	  return false;
	tduti = args.m_declListOrTypedefScalarType;
	brtn = true;
      }
    else if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first. scalar uti
	tduti = m_state.makeUlamType(args.m_typeTok, args.m_bitsize, NONARRAYSIZE, Nouti, args.m_declRef);
	brtn = true;
      }
    else
      {
	tduti = args.m_classInstanceIdx;
	brtn = true;
      }

    if(!m_state.okUTItoContinue(tduti))
      brtn = false;

    if(brtn)
      {
	UTI uti = tduti;
	UTI scalarUTI = args.m_declListOrTypedefScalarType;
	UlamType * ut = m_state.getUlamTypeByIndex(uti);
	ULAMTYPE bUT = ut->getUlamTypeEnum();
	UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
	ULAMCLASSTYPE classtype = ut->getUlamClassType();

	if(ut->isScalar() && (args.m_arraysize != NONARRAYSIZE))
	  {
	    args.m_declListOrTypedefScalarType = scalarUTI = uti;
	    // o.w. build symbol (with bit and array sizes);
	    // arrays can't have their scalar as classInstance;
	    // o.w., no longer findable by token.
	    if(args.m_bitsize == 0)
	      args.m_bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

	    UlamKeyTypeSignature newarraykey(key.getUlamKeyTypeSignatureNameId(), args.m_bitsize, args.m_arraysize, scalarUTI, args.m_declRef); //classinstanceidx removed if not a class
	    uti = m_state.makeUlamType(newarraykey, bUT, classtype);
	  }
	else if(m_state.getReferenceType(uti) != args.m_declRef)
	  {
	    // could be array or scalar ref
	    UlamKeyTypeSignature newkey(key.getUlamKeyTypeSignatureNameId(), key.getUlamKeyTypeSignatureBitSize(), key.getUlamKeyTypeSignatureArraySize(), key.getUlamKeyTypeSignatureClassInstanceIdx(), args.m_declRef); //classinstanceidx removed if not a class
	    uti = m_state.makeUlamType(newkey, bUT, classtype);
	  }
	//create a symbol for this new ulam type, a typedef, with its type
	SymbolTypedef * symtypedef = new SymbolTypedef(m_token, uti, scalarUTI, m_state);
	m_state.addSymbolToCurrentScope(symtypedef);

	//remember tduti for references
	symtypedef->setAutoLocalType(m_state.getReferenceType(uti));

	//check if also a holder (not necessary)
	//if(ut->isHolder()) m_state.addUnknownTypeTokenToThisClassResolver(m_token, uti);

	return (m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr)); //true
      }
    return false;
  } //installSymbolTypedef

  bool NodeIdent::installSymbolConstantValue(TypeArgs& args, Symbol*& asymptr)
  {
    // ask current scope block if this constant name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    UTI holdUTIForAlias = Nouti;
    if(m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr))
      {
	if(m_state.isHolder(asymptr->getUlamTypeIdx()))
	  holdUTIForAlias = asymptr->getUlamTypeIdx(); //t3862
	else
	  return false; //already there
      }

    // maintain specific type (see isAConstant() Node method)
    bool brtn = false;
    UTI uti = Nav;
    UTI tdscalaruti = Nav;

    if(args.m_anothertduti != Nouti)
      {
	if(checkConstantTypedefSizes(args, args.m_anothertduti))
	  {
	    uti = args.m_anothertduti;

	    if(args.m_arraysize != NONARRAYSIZE)
	      {
		uti = m_state.getUlamTypeAsArrayOfScalar(uti);
	      }
	    brtn = true;
	  }
      }
    else if(m_state.getUlamTypeByTypedefName(m_state.getTokenDataAsStringId(args.m_typeTok), uti, tdscalaruti))
      {
	args.m_declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	if(checkConstantTypedefSizes(args, uti))
	  {
	    if(args.m_arraysize != NONARRAYSIZE) //t41144
	      {
		uti = m_state.getUlamTypeAsArrayOfScalar(uti);
	      }
	    brtn = true;
	  }
      }
    else if(args.m_declListOrTypedefScalarType != Nouti)
      {
	if(!checkConstantTypedefSizes(args, args.m_declListOrTypedefScalarType))
	  return false;
	uti = args.m_declListOrTypedefScalarType;
	if(args.m_arraysize != NONARRAYSIZE) //t3890
	  {
	    uti = m_state.getUlamTypeAsArrayOfScalar(uti);
	  }
	brtn = true;
      }
    else if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first.
	uti = m_state.makeUlamType(args.m_typeTok, args.m_bitsize, args.m_arraysize, Nouti);
	brtn = true;
      }
    else
      {
	// support class types for constants (t41198)
	uti = args.m_classInstanceIdx;
	if(args.m_arraysize != NONARRAYSIZE) //t41261 support constant class arrays
	  {
	    uti = m_state.getUlamTypeAsArrayOfScalar(uti);
	  }
	brtn = true;
      }

    if(!m_state.okUTItoContinue(uti))
      brtn = false;

    if(brtn)
      {
	//constant refs allowed (t41192)
	uti = m_state.getUlamTypeAsRef(uti, args.m_declRef, args.m_hasConstantTypeModifier);

	if(!asymptr)
	  {
	    //create a symbol for this new named constant, a constant-def, with its value
	    SymbolConstantValue * symconstdef = new SymbolConstantValue(m_token, uti, m_state);
	    m_state.addSymbolToCurrentScope(symconstdef);
	  }
	else
	  {
	    UlamType * newut = m_state.getUlamTypeByIndex(uti);
	    UlamKeyTypeSignature newkey = newut->getUlamKeyTypeSignature();
	    m_state.makeUlamTypeFromHolder(newkey, newut->getUlamTypeEnum(), holdUTIForAlias, newut->getUlamClassType());
	  }

	if(holdUTIForAlias != Nouti)
	  m_state.updateUTIAliasForced(holdUTIForAlias, uti); //t3862

	//gets the symbol just created by makeUlamType; true.
	return (m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr));
      }
    return false;
  } //installSymbolConstantValue

  bool NodeIdent::installSymbolModelParameterValue(TypeArgs& args, Symbol*& asymptr)
  {
    assert(!m_state.useMemberBlock());
    // ask current scope block if this constant name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr))
      {
	if(m_state.isHolder(asymptr->getUlamTypeIdx()))
	  {
	    //remove it! then continue..
	    Symbol * rmsym = NULL;
	    if(m_state.getCurrentBlock()->removeIdFromScope(m_token.m_dataindex, rmsym))
	      {
		assert(rmsym == asymptr); //sanity check removal
		asymptr = NULL;
	      }
	  }
	else
	  return false; //already there
      }

    // maintain specific type (see isAConstant() Node method)
    bool brtn = false;
    UTI uti = Nav;
    UTI tdscalaruti = Nav;
    if(args.m_anothertduti != Nouti)
      {
	if(checkConstantTypedefSizes(args, args.m_anothertduti))
	  {
	    brtn = true;
	    uti = args.m_anothertduti;
	  }
      }
    else if(m_state.getUlamTypeByTypedefName(m_state.getTokenDataAsStringId(args.m_typeTok), uti, tdscalaruti))
      {
	args.m_declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	if(checkConstantTypedefSizes(args, uti))
	  {
	    brtn = true;
	  }
      }
    else if(args.m_declListOrTypedefScalarType != Nouti)
      {
	if(!checkConstantTypedefSizes(args, args.m_declListOrTypedefScalarType))
	  return false;
	uti = args.m_declListOrTypedefScalarType;
	brtn = true;
      }
    else if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first.
	// scalar uti
	uti = m_state.makeUlamType(args.m_typeTok, args.m_bitsize, NONARRAYSIZE, Nouti);
	brtn = true;
      }
    else
      {
	// no class types for model parameters
	std::ostringstream msg;
	msg << "Model Parameter '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' cannot be based on a class type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(args.m_classInstanceIdx).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    if(!m_state.okUTItoContinue(uti))
      brtn = false;

    if(brtn)
      {
	//create a symbol for this new model parameter, a parameter-def, with its value
	SymbolModelParameterValue * symparam = new SymbolModelParameterValue(m_token, uti, m_state);
	m_state.addSymbolToCurrentScope(symparam);

	//gets the symbol just created by makeUlamType; true.
	return (m_state.isIdInCurrentScope(m_token.m_dataindex, asymptr));
      }
    return false;
  } //installSymbolModelParameterValue

  //see also NodeSquareBracket
  bool NodeIdent::installSymbolVariable(TypeArgs& args, Symbol *& asymptr)
  {
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    u32 tokid = m_state.getTokenDataAsStringId(m_token); //3821 as-cond uses lhs token
    if(m_state.isIdInCurrentScope(tokid, asymptr))
      {
	if(!(asymptr->isFunction()) && !(asymptr->isTypedef()) && !(asymptr->isConstant()) && !(asymptr->isModelParameter()))
	  setSymbolPtr((SymbolVariable *) asymptr); //updates Node's symbol, if is variable
	return false; //already there
      }

    bool brtn = false;
    UTI auti = Nav;
    UTI tdscalaruti = Nav;
    // verify typedef exists for this scope; or is a primitive keyword type
    // if a primitive (NONARRAYSIZE), we may need to make a new arraysize type for it;
    // or if it is a class type (quark, element).
    //list of decls can use the same 'scalar' type (arg); adjusted for arrays
    if(args.m_anothertduti != Nouti)
      {
	if(!checkVariableTypedefSizes(args, args.m_anothertduti))
	  return false;
	//use another classes' typedef uti; (is classInstanceIdx relevant?)
	auti = args.m_anothertduti;
	if(args.m_arraysize != NONARRAYSIZE)
	  {
	    auti = m_state.getUlamTypeAsArrayOfScalar(auti);
	  }
	brtn = true;
      }
    else if(m_state.getUlamTypeByTypedefName(m_state.getTokenDataAsStringId(args.m_typeTok), auti, tdscalaruti))
      {
	args.m_declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	// check typedef types here..
	if(!checkVariableTypedefSizes(args, auti))
	  return false;
	if(args.m_arraysize != NONARRAYSIZE)
	  {
	    auti = m_state.getUlamTypeAsArrayOfScalar(auti);
	  }
	brtn = true;
      }
    else if(args.m_declListOrTypedefScalarType != Nouti)
      {
	if(!checkVariableTypedefSizes(args, args.m_declListOrTypedefScalarType))
	  return false;
	auti = args.m_declListOrTypedefScalarType;
	if(args.m_arraysize != NONARRAYSIZE)
	  {
	    auti = m_state.getUlamTypeAsArrayOfScalar(auti);
	  }
	brtn = true;
      }
    else if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first.
	auti = m_state.makeUlamType(args.m_typeTok, args.m_bitsize, args.m_arraysize, Nouti, args.m_declRef);
	brtn = true;
      }
    else
      {
	auti = args.m_classInstanceIdx;
	if(args.m_arraysize != NONARRAYSIZE)
	  {
	    auti = m_state.getUlamTypeAsArrayOfScalar(auti); //t3813, t3815, t3839
	  }
	brtn = true;
      }

    if(!m_state.okUTItoContinue(auti))
      brtn = false; //t41153

    if(brtn)
      {
	//ut not current; no deref.
	UTI uti = m_state.getUlamTypeAsRef(auti, args.m_declRef, args.m_hasConstantTypeModifier);

	SymbolVariable * sym = makeSymbol(uti, m_state.getReferenceType(uti), args);
	if(sym)
	  {
	    m_state.addSymbolToCurrentScope(sym); //ownership goes to the block
	    setSymbolPtr(sym); //sets m_varSymbol and st block no.
	    asymptr = sym;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Variable symbol '";
	    msg << m_state.getTokenDataAsString(m_token).c_str();
	    msg << "' cannot be a reference";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    brtn = false;
	  }
      }
    return brtn;
  } //installSymbolVariable

  SymbolVariable *  NodeIdent::makeSymbol(UTI auti, ALT reftype, const TypeArgs & args)
  {
    if(m_state.m_parsingVariableSymbolTypeFlag == STF_DATAMEMBER)
      {
	u32 baseslot = 1;  //unpacked fixed later
	//variable-index, ulamtype, ulamvalue(ownership to symbol); always packed
	if(reftype != ALT_NOT)
	  return NULL; //error! dm's not references
	return (new SymbolVariableDataMember(m_token, auti, baseslot, m_state));
      }

    //Symbol is a parameter; always on the stack
    if(m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCPARAMETER)
      {
	SymbolVariableStack * rtnSym = (new SymbolVariableStack(m_token, auti, m_state)); //slot after adjust
	assert(rtnSym);
	rtnSym->setAutoLocalType(reftype);
	rtnSym->setFunctionParameter();
	if(args.m_hasConstantTypeModifier)
	  rtnSym->setConstantFunctionParameter();
	return rtnSym;
      }

    //Symbol is an argument; always on the stack (t3962)
    if(m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCARGUMENT)
      {
	SymbolVariableStack * rtnSym = (new SymbolVariableStack(m_token, auti, m_state)); //slot after adjust
	assert(rtnSym);
	rtnSym->setAutoLocalType(reftype);
	return rtnSym;
      }

    assert(m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCLOCALVAR);
    //(else) Symbol is a local variable, always on the stack
    SymbolVariableStack * rtnLocalSym = new SymbolVariableStack(m_token, auti, m_state); //slot before adjustment
    assert(rtnLocalSym);
    rtnLocalSym->setAutoLocalType(reftype);
    return rtnLocalSym;
  } //makeSymbol

  bool NodeIdent::checkVariableTypedefSizes(TypeArgs& args, UTI auti)
  {
    bool rtnb = true;
    UlamType * tdut = m_state.getUlamTypeByIndex(auti);
    s32 tdarraysize = tdut->getArraySize();
    if(args.m_arraysize != NONARRAYSIZE && tdarraysize != NONARRAYSIZE)
      {
	//error can't support double arrays
	std::ostringstream msg;
	msg << "Array size [] is included in typedef '";
	msg <<  m_state.getTokenDataAsString(args.m_typeTok).c_str();
	msg << "', and cannot be redefined by variable '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    else  //variable not array; leave as-is when unknown
      {
	if(args.m_arraysize == UNKNOWNSIZE || tdarraysize == UNKNOWNSIZE)
	  args.m_arraysize = UNKNOWNSIZE;
	else
	  args.m_arraysize = tdarraysize; //use whatever typedef is
      }

    s32 tdbitsize = tdut->getBitSize();
    if(args.m_bitsize > 0)  //variable's
      {
	if(tdbitsize != args.m_bitsize)  //was (tdbitsize > 0)
	  {
	    //error can't support different bitsizes
	    std::ostringstream msg;
	    msg << "Bit size (" << tdbitsize << ") is included in typedef '";
	    msg <<  m_state.getTokenDataAsString(args.m_typeTok).c_str();
	    msg << "', and cannot be redefined by variable '";
	    msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false;
	  }
      }
    else  //variable unknown bitsize
      {
	args.m_bitsize = tdbitsize; //use whatever typedef is
      }
    return rtnb;
  } //checkVariableTypedefSizes

  bool NodeIdent::checkTypedefOfTypedefSizes(TypeArgs& args, UTI tduti)
  {
    bool rtnb = true;
    UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
    s32 tdarraysize = tdut->getArraySize();
    if(args.m_arraysize != NONARRAYSIZE && tdarraysize != NONARRAYSIZE)
      {
	//error can't support typedefs changing arraysizes
	std::ostringstream msg;
	msg << "Array size [] is included in typedef '";
	msg <<  m_state.getTokenDataAsString(args.m_typeTok).c_str();
	msg << "', and cannot be redefined by typedef '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    else
      {
	if(args.m_arraysize == UNKNOWNSIZE || tdarraysize == UNKNOWNSIZE)
	  args.m_arraysize = UNKNOWNSIZE;
	else
	  args.m_arraysize = tdarraysize; //use whatever typedef is
      }

    args.m_bitsize = tdut->getBitSize(); //ok to use typedef bitsize
    return rtnb;
  } //checkTypedefOfTypedefSizes

  bool NodeIdent::checkConstantTypedefSizes(TypeArgs& args, UTI tduti)
  {
    bool rtnb = true;
    UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
    s32 tdarraysize = tdut->getArraySize();
    if((args.m_arraysize != NONARRAYSIZE) && (tdarraysize != NONARRAYSIZE))
      {
	//error can't support named constant arrays (error/t3446)
	std::ostringstream msg;
	msg << "Array size [] is included in typedef '";
	msg <<  m_state.getTokenDataAsString(args.m_typeTok).c_str();
	msg  << "', and cannot be used by a named constant '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }
    args.m_bitsize = tdut->getBitSize(); //ok to use typedef bitsize

    // as of ulam-4, classes may be constants; (t41298,9 t41300,1)
    //assert(tdut->getUlamTypeEnum() != Class); //testing only

    return rtnb;
  } //checkConstantTypedefSizes

  void NodeIdent::genCode(File * fp, UVPass & uvpass)
  {
    //return the ptr for an array; square bracket will resolve down to the immediate data
    makeUVPassForCodeGen(uvpass);

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol); //*********UPDATED GLOBAL;

    // UNCLEAR: should this be consistent with constants?
    genCodeReadIntoATmpVar(fp, uvpass);
  } //genCode

  void NodeIdent::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    // an empty dot chain indicates uvpass has the info (e.g. casting);
    // keep 'self' (t3185); keep if tmpvarSymbol BaseType (t41321);
    if((uvpass.getPassStorage()==TMPAUTOREF) && !((uvpass.getPassNameId()==m_state.m_pool.getIndexForDataString("self")) || uvpass.getPassApplyDelta()))
      {
	Node::genCodeConvertATmpVarAutoRefIntoAutoRef(fp, uvpass); //uvpass becomes the autoref, and clears stack
      }

    //e.g. return the ptr for an array;
    //square bracket will resolve down to the immediate data
    makeUVPassForCodeGen(uvpass);

    //******UPDATED GLOBAL; no restore!!!**************************
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol);
  } //genCodeToStoreInto

  // overrides NodeTerminal that reads into a tmp var BitVector
  void NodeIdent::genCodeReadIntoATmpVar(File * fp, UVPass & uvpass)
  {
    Node::genCodeReadIntoATmpVar(fp, uvpass);
  }

} //end MFM
