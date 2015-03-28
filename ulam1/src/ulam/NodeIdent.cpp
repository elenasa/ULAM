#include <stdlib.h>
#include "NodeConstant.h"
#include "NodeIdent.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "NodeTypeBitsize.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "SymbolTypedef.h"

namespace MFM {

  NodeIdent::NodeIdent(Token tok, SymbolVariable * symptr, CompilerState & state) : Node(state), m_token(tok), m_varSymbol(symptr), m_currBlockNo(0)
  {
    if(symptr)
      m_currBlockNo = symptr->getBlockNoOfST();
  }

  NodeIdent::NodeIdent(const NodeIdent& ref) : Node(ref), m_token(ref.m_token), m_varSymbol(NULL), m_currBlockNo(ref.m_currBlockNo) {}

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
    return m_state.getTokenDataAsString(&m_token).c_str();
  }

  const std::string NodeIdent::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeIdent::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return true;
  }

  void NodeIdent::setSymbolPtr(SymbolVariable * vsymptr)
  {
    assert(vsymptr);
    m_varSymbol = vsymptr;
    m_currBlockNo = vsymptr->getBlockNoOfST();
    assert(m_currBlockNo);
  }

  Token NodeIdent::getToken() const
  {
    return m_token;
  }

  UTI NodeIdent::checkAndLabelType()
  {
    UTI it = Nav;  //init

    //2 cases: use was before def, look up in class block; cloned unknown
    if(m_varSymbol == NULL)
      {
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
	m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

	Symbol * asymptr = NULL;
	if(m_state.alreadyDefinedSymbol(m_token.m_dataindex,asymptr))
	  {
	    if(!asymptr->isFunction() && !asymptr->isTypedef() && !asymptr->isConstant())
	      {
		setSymbolPtr((SymbolVariable *) asymptr);
		//assert(asymptr->getBlockNoOfST() == m_currBlockNo); not necessarily true
		// e.g. var used before defined, and then is a data member outside current func block.
	      }
	    else if(asymptr->isConstant())
	      {
		// replace ourselves with a constant node instead;
		// same node no, and loc
		NodeConstant * newnode = new NodeConstant(*this);
		Node * parentNode = m_state.findNodeNoInThisClass(Node::getYourParentNo());
		assert(parentNode);

		assert(parentNode->exchangeKids(this, newnode));

		std::ostringstream msg;
		msg << "Exchanged kids! <" << m_state.getTokenDataAsString(&m_token).c_str();
		msg << "> a named constant, in place of a variable with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);

		m_state.popClassContext(); //restore

		delete this; //suicide is painless..

		return newnode->checkAndLabelType();
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.getTokenDataAsString(&m_token).c_str();
		msg << "> is not a variable, and cannot be used as one with class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) <" << m_state.getTokenDataAsString(&m_token).c_str();
	    msg << "> is not defined, and cannot be used with class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	m_state.popClassContext(); //restore
      } //lookup symbol

    if(m_varSymbol)
      {
	it = m_varSymbol->getUlamTypeIdx();
      }

    setNodeType(it);
    setStoreIntoAble(true);
    return it;
  } //checkAndLabelType

  NNO NodeIdent::getBlockNo() const
  {
    return m_currBlockNo;
  }

  NodeBlock * NodeIdent::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  }

  EvalStatus NodeIdent::eval()
  {
    assert(m_varSymbol);
    evalNodeProlog(0); //new current frame pointer
    UTI nuti = getNodeType();

    //return the ptr for an array; square bracket will resolve down to the immediate data
    UlamValue uv;
    UlamValue uvp = makeUlamValuePtr();

    if(m_state.isScalar(nuti))
      {
	uv = m_state.getPtrTarget(uvp);

	// redo what getPtrTarget use to do, when types didn't match due to
	// an element/quark or a requested scalar of an arraytype
	if(uv.getUlamValueTypeIdx() != nuti)
	  {
	    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

	    if(nut->isCustomArray())
	      {
		UTI caType = ((UlamTypeClass *) nut)->getCustomArrayType();
		UlamType * caut = m_state.getUlamTypeByIndex(caType);
		if(caType == UAtom || caut->getBitSize() > 32)
		  {
		    uv = uvp; //customarray
		  }
		else
		  {
		    u32 datavalue = uv.getDataFromAtom(uvp, m_state);
		    uv = UlamValue::makeImmediate(nuti, datavalue, m_state);
		  }
	      }
	    else
	      {
		if(nuti == UAtom || nut->getUlamClass() == UC_ELEMENT)
		  {
		    uv = m_state.getPtrTarget(uvp); //UlamValue::makeAtom(caType);
		  }
		else
		  {
		    // does this handle a ptr to a ptr (e.g. "self")? (see makeUlamValuePtr)
		    assert(uv.getUlamValueTypeIdx() != Ptr);

		    u32 datavalue = uv.getDataFromAtom(uvp, m_state);
		    uv = UlamValue::makeImmediate(nuti, datavalue, m_state);
		  }
	      }
	  } // not node type
      } //scalar
    else
      uv = uvp;

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeIdent::evalToStoreInto()
  {
    assert(m_varSymbol);
    assert(isStoreIntoAble());

    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeIdent::makeUlamValuePtr()
  {
    UlamValue ptr;

    //instead of a ptr to "self" (already a ptr), return "self"
    if(m_varSymbol->isSelf())
      return m_state.m_currentSelfPtr;

    ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(getNodeType())->getUlamClass();
    if(classtype == UC_ELEMENT)
      {
	if(!m_varSymbol->isElementParameter())
	  // ptr to explicit atom or element, (e.g. 'f' in f.a=1;) to become new m_currentObjPtr
	  ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), UNPACKED, m_state, 0, m_varSymbol->getId());
	else
	  ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId()); //???
      }
    else
      {
	if(m_varSymbol->isDataMember())
	  {
	    if(!m_varSymbol->isElementParameter())
	      // return ptr to this data member within the m_currentObjPtr
	      // 'pos' modified by this data member symbol's packed bit position
	      ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());
	    else //same or not???
	      ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());
	  }
	else
	  {
	    //local variable on the stack; could be array ptr!
	    ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	  }
      }
    return ptr;
  } //makeUlamValuePtr

  //new
  UlamValue NodeIdent::makeUlamValuePtrForCodeGen()
  {
    s32 tmpnum = m_state.getNextTmpVarNumber();

    UlamValue ptr;
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());
    ULAMCLASSTYPE classtype = nut->getUlamClass();
    if(classtype == UC_ELEMENT)
      {
	// ptr to explicit atom or element, (e.g. 'f' in f.a=1;)
	ptr = UlamValue::makePtr(tmpnum, TMPREGISTER, getNodeType(), UNPACKED, m_state, 0, m_varSymbol->getId());
      }
    else
      {
	if(m_varSymbol->isDataMember() && !m_varSymbol->isElementParameter())
	  {
	    u32 pos = 0;
	    if(!m_state.m_currentObjSymbolsForCodeGen.empty())
	      {
		SymbolVariable * sym = (SymbolVariable *) m_state.m_currentObjSymbolsForCodeGen.back();
		pos = sym->getPosOffset();
	      }
	    // 'pos' modified by this data member symbol's packed bit position
	    ptr = UlamValue::makePtr(tmpnum, TMPREGISTER, getNodeType(), m_state.determinePackable(getNodeType()), m_state, /*m_state.m_currentObjPtr.getPtrPos() +*/ pos + m_varSymbol->getPosOffset(), m_varSymbol->getId());
	  }
	else
	    {
	      //local variable on the stack; could be array ptr! or element parameter
	      ptr = UlamValue::makePtr(tmpnum, TMPREGISTER, getNodeType(), m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	    }
      }
    return ptr;
  } //makeUlamValuePtrForCodeGen

  bool NodeIdent::installSymbolTypedef(ParserTypeArgs& args, Symbol *& asymptr)
  {
    bool brtn = false;
    UTI tduti = Nav;
    UTI tdscalaruti = Nav;
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex,asymptr))
      {
	if(asymptr->isTypedef() && asymptr->isFabricatedTmp())
	  {
	    tduti = asymptr->getUlamTypeIdx();
	    args.declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	    brtn = true;
	    //remove it! then continue using same uti..
	    Symbol * rmsym = NULL;
	    if(m_state.getCurrentBlock()->removeIdFromScope(m_token.m_dataindex, rmsym))
	      {
		assert(rmsym == asymptr); //sanity check removal
		asymptr = NULL;
	      }
	  }
	else
	  return false;    //already there
      }

    if(tduti==Nav)  //not a pre-fab
      {
	//typedef might have bitsize and arraysize info..
	if(args.anothertduti)
	  {
	    if(checkTypedefOfTypedefSizes(args, args.anothertduti)) //ref
	      {
		tduti = args.anothertduti;
		brtn = true;
	      }
	  }
	else if(m_state.getUlamTypeByTypedefName(args.typeTok.m_dataindex, tduti, tdscalaruti))
	  {
	    args.declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	    if(checkTypedefOfTypedefSizes(args, tduti)) //ref
	      {
		brtn = true;
	      }
	  }
	else if(Token::getSpecialTokenWork(args.typeTok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    //UlamTypes automatically created for the base types with different array sizes.
	    //but with typedef's "scope" of use, typedef needed to be checked first. scalar uti
	    tduti = m_state.makeUlamType(args.typeTok, args.bitsize, NONARRAYSIZE, Nav);
	    brtn = true;
	  }
	else
	  {
	    tduti = args.classInstanceIdx;
	    brtn = true;
	  }
      }

    if(brtn)
      {
	UTI uti = tduti;
	UTI scalarUTI = args.declListOrTypedefScalarType;
	if(m_state.isScalar(uti) && args.arraysize != NONARRAYSIZE)
	  {
	    args.declListOrTypedefScalarType = scalarUTI = uti;
	    // o.w. build symbol (with bit and array sizes);
	    // array's can't have their scalar as classInstance; o.w., no longer findable by token.
	    UlamType * ut = m_state.getUlamTypeByIndex(uti);
	    ULAMTYPE bUT = ut->getUlamTypeEnum();
	    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();

	    if(args.bitsize == 0)
	      args.bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

	    UlamKeyTypeSignature newarraykey(key.getUlamKeyTypeSignatureNameId(), args.bitsize, args.arraysize);
	    newarraykey.append(scalarUTI); //removed if not a class

	    uti = m_state.makeUlamType(newarraykey, bUT);
	  }

	//create a symbol for this new ulam type, a typedef, with its type
	SymbolTypedef * symtypedef = new SymbolTypedef(m_token.m_dataindex, uti, scalarUTI, m_state);
	m_state.addSymbolToCurrentScope(symtypedef);
	return (m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr)); //true
      }
    return false;
  } //installSymbolTypedef

  bool NodeIdent::installSymbolConstantValue(ParserTypeArgs& args, Symbol*& asymptr)
  {
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr))
      {
	if(asymptr->isFabricatedTmp())
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
    if(args.anothertduti)
      {
	if(checkConstantTypedefSizes(args, args.anothertduti))
	  {
	    brtn = true;
	    uti = args.anothertduti;
	  }
      }
    else if(m_state.getUlamTypeByTypedefName(args.typeTok.m_dataindex, uti, tdscalaruti))
      {
	args.declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	if(checkConstantTypedefSizes(args, uti))
	  {
	    brtn = true;
	  }
      }
    else if(Token::getSpecialTokenWork(args.typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first.
	// scalar uti
	uti = m_state.makeUlamType(args.typeTok, args.bitsize, NONARRAYSIZE, Nav);
	brtn = true;
      }
    else
      {
	// no class types for constants
	std::ostringstream msg;
	msg << "Named Constant '" << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' cannot be based on a class type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(args.classInstanceIdx).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    if(brtn)
      {
	//create a symbol for this new named constant, a constant-def, with its value
	SymbolConstantValue * symconstdef = new SymbolConstantValue(m_token.m_dataindex, uti, m_state);
	m_state.addSymbolToCurrentScope(symconstdef);

	//gets the symbol just created by makeUlamType
	return (m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr)); //true
      }
    return false;
  } //installSymbolConstantValue

  //see also NodeSquareBracket
  bool NodeIdent::installSymbolVariable(ParserTypeArgs& args, Symbol *& asymptr)
  {
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.getCurrentBlock()->isIdInScope(m_token.m_dataindex, asymptr))
      {
	if(asymptr->isFabricatedTmp())
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
	  {
	    if(!(asymptr->isFunction()) && !(asymptr->isTypedef() && !(asymptr->isConstant()) ))
	      setSymbolPtr((SymbolVariable *) asymptr); //updates Node's symbol, if is variable
	    return false; //already there
	  }
      }

    // verify typedef exists for this scope; or is a primitive keyword type
    // if a primitive (NONARRAYSIZE), we may need to make a new arraysize type for it;
    // or if it is a class type (quark, element).
    UTI auti = Nav;
    UTI tdscalaruti = Nav;
    bool brtn = false;

    //list of decls can use the same 'scalar' type (arg); adjusted for arrays
    if(args.declListOrTypedefScalarType)
      {
	if(!checkVariableTypedefSizes(args, args.declListOrTypedefScalarType))
	  return false;
	auti = args.declListOrTypedefScalarType;
	brtn = true;
      }
    else if(args.anothertduti)
      {
	if(!checkVariableTypedefSizes(args, args.anothertduti))
	  return false;
	//use another classes' typedef uti; (is classInstanceIdx relevant?)
	auti = args.anothertduti;
	brtn = true;
      }
    else if(m_state.getUlamTypeByTypedefName(args.typeTok.m_dataindex, auti, tdscalaruti))
      {
	args.declListOrTypedefScalarType = tdscalaruti; //not Nav when tduti is an array
	// check typedef types here..
	if(!checkVariableTypedefSizes(args, auti))
	  return false;
	brtn = true;
      }
    else if(Token::getSpecialTokenWork(args.typeTok.m_type) == TOKSP_TYPEKEYWORD)
      {
	//UlamTypes automatically created for the base types with different array sizes.
	//but with typedef's "scope" of use, typedef needed to be checked first.
	auti = m_state.makeUlamType(args.typeTok, args.bitsize, args.arraysize, Nav);
	brtn = true;
      }
    else
      {
	auti = args.classInstanceIdx;
	brtn = true;
      }

    if(brtn)
      {
	UTI uti = auti;
	UTI scalarUTI = args.declListOrTypedefScalarType;
	if(m_state.isScalar(uti) && args.arraysize != NONARRAYSIZE)
	  {
	    args.declListOrTypedefScalarType = scalarUTI = uti;
	    // o.w. build symbol (with bit and array sizes);
	    // array's can't have their scalar as classInstance; o.w., no longer findable by token.
	    UlamType * ut = m_state.getUlamTypeByIndex(uti);
	    ULAMTYPE bUT = ut->getUlamTypeEnum();
	    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();

	    if(args.bitsize == 0)
	      args.bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];

	    UlamKeyTypeSignature newarraykey(key.getUlamKeyTypeSignatureNameId(), args.bitsize, args.arraysize);
	    newarraykey.append(scalarUTI);
	    uti = m_state.makeUlamType(newarraykey, bUT);
	  }

	SymbolVariable * sym = makeSymbol(uti);
	m_state.addSymbolToCurrentScope(sym); //ownership goes to the block
	setSymbolPtr(sym); //sets m_varSymbol and st block no.
	asymptr = sym;
      }
    return brtn;
  } //installSymbolVariable

  SymbolVariable *  NodeIdent::makeSymbol(UTI auti)
  {
    //adjust decl count and max_depth, used for function definitions
    PACKFIT packit = m_state.determinePackable(auti);

    if(m_state.m_currentFunctionBlockDeclSize == 0)
      {
	// s32 arraysize = m_state.getArraySize(aut);
	// when current block and class block are the same, this is a data member
	// assert(m_state.m_currentBlock == (NodeBlock *) m_state.m_classBlock);
	// assert fails when using a data member inside a function block!!!
	//UTI but = aut;
	//
	// get UlamType for arrays
	//if(arraysize > NONARRAYSIZE)
	//  {
	//    but = m_state.getUlamTypeAsScalar(aut);
	//  }
	//
	//UlamValue val(aut, but);  //array, base ulamtype args
	//u32 baseslot = m_state.m_eventWindow.pushDataMember(aut,but);
	u32 baseslot = 1;  //no longer stored unpacked

	//variable-index, ulamtype, ulamvalue(ownership to symbol); always packed
	return (new SymbolVariableDataMember(m_token.m_dataindex, auti, packit, baseslot, m_state));
      }

    //Symbol is a parameter; always on the stack
    if(m_state.m_currentFunctionBlockDeclSize < 0)
      {
	//1 slot for scalar or packed array
	  m_state.m_currentFunctionBlockDeclSize -= m_state.slotsNeeded(auti);

	return (new SymbolVariableStack(m_token.m_dataindex, auti, packit, m_state.m_currentFunctionBlockDeclSize, m_state)); //slot after adjust
      }

    //(else) Symbol is a local variable, always on the stack
    SymbolVariableStack * rtnLocalSym = new SymbolVariableStack(m_token.m_dataindex, auti, packit, m_state.m_currentFunctionBlockDeclSize, m_state); //slot before adjustment

    m_state.m_currentFunctionBlockDeclSize += m_state.slotsNeeded(auti);

    //adjust max depth, excluding parameters and initial start value (=1)
    if(m_state.m_currentFunctionBlockDeclSize - 1 > m_state.m_currentFunctionBlockMaxDepth)
      m_state.m_currentFunctionBlockMaxDepth = m_state.m_currentFunctionBlockDeclSize - 1;

    return rtnLocalSym;
  } //makeSymbol

  bool NodeIdent::checkVariableTypedefSizes(ParserTypeArgs& args, UTI auti)
  {
    bool rtnb = true;
    UlamType * tdut = m_state.getUlamTypeByIndex(auti);
    s32 tdarraysize = tdut->getArraySize();
    if(args.arraysize >= 0)  //variable's
      {
	if(tdarraysize >= 0 && tdarraysize != args.arraysize)
	  {
	    //error can't support double arrays
	    std::ostringstream msg;
	    msg << "Arraysize [" << tdarraysize << "] is included in typedef: <";
	    msg <<  m_state.getTokenDataAsString(&args.typeTok).c_str() << ">, type: ";
	    msg << m_state.getUlamTypeNameByIndex(auti).c_str();
	    msg << ", and cannot be redefined by variable: <";
	    msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false;
	  }
      }
    else  //variable not array; leave as-is when unknown
      {
	if(args.arraysize == UNKNOWNSIZE || tdarraysize == UNKNOWNSIZE)
	  args.arraysize = UNKNOWNSIZE;
	else
	  args.arraysize = tdarraysize; //use whatever typedef is
      }

    s32 tdbitsize = tdut->getBitSize();
    if(args.bitsize > 0)  //variable's
      {
	if(tdbitsize != args.bitsize)  //was (tdbitsize > 0)
	  {
	    //error can't support different bitsizes
	    std::ostringstream msg;
	    msg << "Bit size (" << tdbitsize << ") is included in typedef: <";
	    msg <<  m_state.getTokenDataAsString(&args.typeTok).c_str() << ">, type: ";
	    msg << m_state.getUlamTypeNameByIndex(auti).c_str();
	    msg << ", and cannot be redefined by variable: <";
	    msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false;
	  }
      }
    else  //variable unknown bitsize
      {
	args.bitsize = tdbitsize; //use whatever typedef is
      }
    return rtnb;
  } //checkVariableTypedefSizes

  bool NodeIdent::checkTypedefOfTypedefSizes(ParserTypeArgs& args, UTI tduti)
  {
    bool rtnb = true;
    UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
    s32 tdarraysize = tdut->getArraySize();
    if(args.arraysize >= 0)
      {
	if(tdarraysize >= 0 && args.arraysize != tdarraysize)
	  {
	    //error can't support typedefs changing arraysizes
	    std::ostringstream msg;
	    msg << "Arraysize [" << tdarraysize << "] is included in typedef: <";
	    msg <<  m_state.getTokenDataAsString(&args.typeTok).c_str() << ">, type: ";
	    msg << m_state.getUlamTypeNameByIndex(args.anothertduti).c_str();
	    msg << ", and cannot be redefined by typedef: <";
	    msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	    msg << ">, to [" << args.arraysize << "]";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false;
	  }
      }
    else
      {
	if(args.arraysize == UNKNOWNSIZE || tdarraysize == UNKNOWNSIZE)
	  args.arraysize = UNKNOWNSIZE;
	else
	  args.arraysize = tdarraysize; //use whatever typedef is
      }

    args.bitsize = tdut->getBitSize(); //ok to use typedef bitsize
    return rtnb;
  } //checkTypedefOfTypedefSizes

  bool NodeIdent::checkConstantTypedefSizes(ParserTypeArgs& args, UTI tduti)
  {
    bool rtnb = true;
    UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
    s32 tdarraysize = tdut->getArraySize();
    if(args.arraysize != NONARRAYSIZE || tdarraysize != NONARRAYSIZE)
      {
	//error can't support named constant arrays
	std::ostringstream msg;
	msg << "Arraysize [" << tdarraysize << "] is included in typedef: <";
	msg <<  m_state.getTokenDataAsString(&args.typeTok).c_str() << ">, type: ";
	msg << m_state.getUlamTypeNameByIndex(args.anothertduti).c_str();
	msg << ", and cannot be used by a named constant: '";
	msg << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    assert(args.arraysize == NONARRAYSIZE);

    args.bitsize = tdut->getBitSize(); //ok to use typedef bitsize

    // constants can't be classes either
    if(tdut->getUlamClass() != UC_NOTACLASS)
      {
	std::ostringstream msg;
	msg << "Named Constant '" << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str();
	msg << "' cannot be based on a class type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(tduti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnb = false;
      }
    return rtnb;
  } //checkConstantTypedefSizes

  void NodeIdent::genCode(File * fp, UlamValue & uvpass)
  {
    //return the ptr for an array; square bracket will resolve down to the immediate data
    uvpass = makeUlamValuePtrForCodeGen();

    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol);  //************UPDATED GLOBAL;

    // UNCLEAR: should this be consistent with constants?
    genCodeReadIntoATmpVar(fp, uvpass);
  } //genCode

  void NodeIdent::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    //e.g. return the ptr for an array; square bracket will resolve down to the immediate data
    uvpass = makeUlamValuePtrForCodeGen();

    //******UPDATED GLOBAL; no restore!!!**************************
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol);  //************UPDATED GLOBAL;
  } //genCodeToStoreInto

  // overrides NodeTerminal that reads into a tmp var BitVector
  void NodeIdent::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    Node::genCodeReadIntoATmpVar(fp, uvpass);
  }

} //end MFM
