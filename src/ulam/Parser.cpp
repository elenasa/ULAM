#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include "Parser.h"
#include "NodeAtomof.h"
#include "NodeBinaryOpArithAdd.h"
#include "NodeBinaryOpArithDivide.h"
#include "NodeBinaryOpArithMultiply.h"
#include "NodeBinaryOpArithRemainder.h"
#include "NodeBinaryOpArithSubtract.h"
#include "NodeBinaryOpBitwiseAnd.h"
#include "NodeBinaryOpBitwiseOr.h"
#include "NodeBinaryOpBitwiseXor.h"
#include "NodeBinaryOpCompareEqualEqual.h"
#include "NodeBinaryOpCompareNotEqual.h"
#include "NodeBinaryOpCompareLessThan.h"
#include "NodeBinaryOpCompareGreaterThan.h"
#include "NodeBinaryOpCompareLessEqual.h"
#include "NodeBinaryOpCompareGreaterEqual.h"
#include "NodeBinaryOpEqual.h"
#include "NodeBinaryOpEqualArithAdd.h"
#include "NodeBinaryOpEqualArithDivide.h"
#include "NodeBinaryOpEqualArithMultiply.h"
#include "NodeBinaryOpEqualArithRemainder.h"
#include "NodeBinaryOpEqualArithSubtract.h"
#include "NodeBinaryOpEqualArithPreIncr.h"
#include "NodeBinaryOpEqualArithPreDecr.h"
#include "NodeBinaryOpEqualArithPostIncr.h"
#include "NodeBinaryOpEqualArithPostDecr.h"
#include "NodeBinaryOpEqualBitwiseAnd.h"
#include "NodeBinaryOpEqualBitwiseOr.h"
#include "NodeBinaryOpEqualBitwiseXor.h"
#include "NodeBinaryOpEqualShiftLeft.h"
#include "NodeBinaryOpEqualShiftRight.h"
#include "NodeBinaryOpLogicalAnd.h"
#include "NodeBinaryOpLogicalOr.h"
#include "NodeBinaryOpShiftLeft.h"
#include "NodeBinaryOpShiftRight.h"
#include "NodeBlock.h"
#include "NodeBlockEmpty.h"
#include "NodeBlockSwitch.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeBlockInvisible.h"
#include "NodeBreakStatement.h"
#include "NodeCast.h"
#include "NodeConditionalAs.h"
#include "NodeConditionalIs.h"
#include "NodeConstant.h"
#include "NodeConstantArray.h"
#include "NodeConstantof.h"
#include "NodeContinueStatement.h"
#include "NodeFuncDecl.h"
#include "NodeInitDM.h"
#include "NodeLabel.h"
#include "NodeListEmpty.h"
#include "NodeMemberSelect.h"
#include "NodeMemberSelectByBaseClassType.h"
#include "NodeMemberSelectOnConstructorCall.h"
#include "NodeModelParameter.h"
#include "NodeQuestionColon.h"
#include "NodeReturnStatement.h"
#include "NodeSquareBracket.h"
#include "NodeSimpleStatement.h"
#include "NodeStatementEmpty.h"
#include "NodeTerminal.h"
#include "NodeTerminalProxy.h"
#include "NodeTypeBitsize.h"
#include "NodeTypedef.h"
#include "NodeTypeDescriptorArray.h"
#include "NodeTypeDescriptorSelect.h"
#include "NodeUnaryOpBang.h"
#include "NodeUnaryOpMinus.h"
#include "NodeUnaryOpPlus.h"
#include "NodeUnaryOpTwiddle.h"
#include "NodeVarDeclDM.h"
#include "NodeVarRef.h"
#include "NodeVarRefAs.h"
#include "SymbolClass.h"
#include "SymbolClassName.h"
#include "SymbolConstantValue.h"
#include "SymbolFunction.h"
#include "SymbolFunctionName.h"
#include "SymbolModelParameterValue.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"

namespace MFM {

#define QUIETLY true
#define NOASSIGNOK false
#define SINGLEDECL true

  Parser::Parser(Tokenizer * izer, CompilerState & state): m_state(state), m_tokenizer(izer)
  {
    //moved here since parse can be called multiple times
    initPrimitiveUlamTypes();
  }

  Parser::~Parser()
  {}

  //return changed to number of errors
  //change to return Node * rather than vector; change tests
  u32 Parser::parseProgram(std::string startstr, File * errOutput)
  {
    if(errOutput)
      m_state.m_err.setFileOutput(errOutput);

    //add name of the thing we are compiling to string pool (and node program);
    //dropping the .ulam suffix
    u32 compileThisId;
    if(!m_state.getClassNameFromFileName(startstr, compileThisId))
      return 1; //1 error

    // The First Token locator is used for locals filescope defs; Preprocessing directive 'load'
    //  takes the loc of this first token, from the file that has the load; so, a non-ulamfilename
    //  with localdefs may be used by more than one ulam class. (ulam-6)
    //  The 'use' class directive (ulam filenames only) now keeps its own loc from its first token
    //  for their localdefs by queing the file. (ulam-6)
    Token firstTok;
    AssertBool firstpeek = peekFirstToken(firstTok); //t3872,t41130; error/t3893
    assert(firstpeek);
    m_state.saveFirstTokenForParsing(firstTok);

    //here's the start (first token)!!  preparser will handle the VERSION_DECL,
    //as well as USE and LOAD keywords.
    while(!parseThisClass());

    //here when no more classes, or there was an error
    Symbol * thisClassSymbol = NULL;
    NodeBlockClass * rootNode = NULL;

    if(m_state.m_programDefST.isInTable(compileThisId, thisClassSymbol))
      {
	UTI cuti = thisClassSymbol->getUlamTypeIdx();
	ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(cuti)->getUlamClassType();
	if(classtype == UC_UNSEEN)
	  {
	    std::ostringstream msg;
	    msg << "Invalid Type: " << m_state.m_pool.getDataAsString(compileThisId).c_str();
	    MSG(m_state.getFullLocationAsString(thisClassSymbol->getLoc()).c_str(), msg.str().c_str(), ERR); //t3866,t3867
	  }
	rootNode = ((SymbolClass *) thisClassSymbol)->getClassBlockNode();
      }

    if(rootNode == NULL)
      {
	std::ostringstream msg;
	msg << "No parse tree for This Class: ";
	msg << m_state.m_pool.getDataAsString(compileThisId).c_str();
	MSG("", msg.str().c_str(), ERR);
      }

    u32 warns = m_state.m_err.getWarningCount();
    if(warns > 0)
      {
	std::ostringstream msg;
	msg << warns << " warning" << (warns > 1 ? "s " : " ") << "during parsing: ";
	msg << m_state.m_pool.getDataAsString(compileThisId).c_str();
	MSG((rootNode ? rootNode->getNodeLocationAsString().c_str() : ""), msg.str().c_str(), INFO);
      }

    u32 errs = m_state.m_err.getErrorCount();
    if(errs > 0)
      {
	std::ostringstream msg;
	msg << errs << " TOO MANY PARSE ERRORS: ";
	msg << m_state.m_pool.getDataAsString(compileThisId).c_str();
	MSG((rootNode ? rootNode->getNodeLocationAsString().c_str() : ""), msg.str().c_str(), INFO);
      }

    m_state.clearFirstTokenForParsing();
    return (errs);
  } //parseProgram

  bool Parser::parseThisClass()
  {
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_EOF)
      {
        //Nothing else (but haven't found startstr yet)
	MSG(&pTok, "EOF before finding compilation target", DEBUG);
	m_state.clearStructuredCommentToken();
        return true;
      }

    if((pTok.m_type != TOK_KW_ELEMENT) && (pTok.m_type != TOK_KW_QUARK) && (pTok.m_type != TOK_KW_QUARKUNION) && (pTok.m_type != TOK_KW_TRANSIENT))
      {
	if(pTok.m_type == TOK_KW_LOCALDEF)
	  {
	    //m_state.setLocalsScopeForParsing(pTok);
	    m_state.setLocalsScopeForParsing();
	    parseLocalDef(); //returns bool
	    m_state.clearLocalsScopeForParsing();
	    return parseThisClass();
	  }
	else if(pTok.m_type == TOK_KW_FLAG_CONCRETECLASS)
	  {
	    //set a flag indicating this class should have no pure virtuals
	    //even if it is not instantiated yet.
	    m_state.setConcreteClassFlagForParsing();
	    return parseThisClass();
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid Class Type <";
	    msg << m_state.getTokenDataAsString(pTok).c_str();
	    msg << ">; KEYWORD should be '";
	    msg << Token::getTokenAsStringFromPool(TOK_KW_ELEMENT, &m_state).c_str();
	    msg << "', '";
	    msg << Token::getTokenAsStringFromPool(TOK_KW_QUARK, &m_state).c_str();
	    msg << "', '";
	    msg << Token::getTokenAsStringFromPool(TOK_KW_QUARKUNION, &m_state).c_str();
	    msg << "', or '";
	    msg << Token::getTokenAsStringFromPool(TOK_KW_TRANSIENT, &m_state).c_str();
	    msg << "', or '";
	    msg << Token::getTokenAsStringFromPool(TOK_KW_LOCALDEF, &m_state).c_str();
	    msg << "'";
	    MSG(&pTok, msg.str().c_str(), ERR);
	    m_state.clearStructuredCommentToken();
	    return true; //we're done.
	  }
      }

    //each class has its own parse tree; each "compileThis",
    //in turn, has code generated later.
    Token iTok;
    getNextToken(iTok);

    //insure the class name starts with a capital letter,
    //and is not a primitive (e.g. TOK_TYPE_INT)
    if(iTok.m_type != TOK_TYPE_IDENTIFIER)
      {
	std::ostringstream msg;
	msg << "Poorly named class '" << m_state.getTokenDataAsString(iTok).c_str();
	msg << "'; Identifier must begin with an upper-case letter";
	MSG(&iTok, msg.str().c_str(), ERR);
	m_state.clearStructuredCommentToken();
	return true; //we're done unless we can gobble the rest up?
      }

    SymbolClassName * cnSym = NULL;
    bool wasIncomplete = false;

    Token qTok;
    getNextToken(qTok);
    unreadToken();
    bool isTemplate = (qTok.m_type == TOK_OPEN_PAREN);

    if(!isTemplate)
      {
	if(!m_state.alreadyDefinedSymbolClassName(iTok.m_dataindex, cnSym))
	  {
	    m_state.addIncompleteParseThisClassSymbolToProgramTable(iTok, cnSym);
	  }
	else
	  {
	    //if already defined, then must be incomplete, else duplicate!!
	    if(cnSym->getUlamClass() != UC_UNSEEN)
	      {
		std::ostringstream msg;
		msg << "Duplicate or incomplete class '";
		msg << m_state.m_pool.getDataAsString(cnSym->getId()).c_str() << "'";
		MSG(&iTok, msg.str().c_str(), ERR);
		m_state.clearStructuredCommentToken();
		m_state.clearConcreteClassFlagForParsing();
		return  true; //we're done unless we can gobble the rest up?
	      }
	    else if(cnSym->isClassTemplate())
	      {
		std::ostringstream msg;
		msg << "Conflicting class args previously seen for class with no parameters '";
		msg << m_state.m_pool.getDataAsString(cnSym->getId()).c_str() << "'";
		MSG(&iTok, msg.str().c_str(), ERR);
		m_state.clearStructuredCommentToken();
		m_state.clearConcreteClassFlagForParsing();
		return  true; //we're done unless we can gobble the rest up?
	      }
	    else
	      {
		UTI uti = cnSym->getUlamTypeIdx();
		bool tmpisaclassholder = false;
		m_state.cleanupAllGeneratedLocalsTypedefsOfThisClassHolder(cnSym->getId(), uti, tmpisaclassholder); //t41519
		assert(uti == cnSym->getUlamTypeIdx()); //no change.
	      }
	    wasIncomplete = true;
	  }
      }
    else
      {
	SymbolClassNameTemplate * ctSym = NULL;
	if(!m_state.alreadyDefinedSymbolClassNameTemplate(iTok.m_dataindex, ctSym) && ctSym == NULL)
	  {
	    m_state.addIncompleteTemplateClassSymbolToProgramTable(iTok, ctSym);
	  }
	else
	  {
	    //if already defined, then must be incomplete, else duplicate!!
	    if(ctSym && ctSym->getUlamClass() != UC_UNSEEN)
	      {
		std::ostringstream msg;
		msg << "Duplicate or incomplete template class '";
		msg << m_state.m_pool.getDataAsString(ctSym->getId()).c_str() << "'";
		MSG(&iTok, msg.str().c_str(), ERR);
		m_state.clearStructuredCommentToken();
		m_state.clearConcreteClassFlagForParsing();
		return true; //we're done unless we can gobble the rest up?
	      }
	    if(ctSym && !ctSym->isClassTemplate())
	      {
		//error already output
		m_state.clearStructuredCommentToken();
		m_state.clearConcreteClassFlagForParsing();
		return true; //we're done unless we can gobble the rest up?
	      }
	    wasIncomplete = true;
	  }
	cnSym = ctSym;
      } //template

    assert(cnSym);
    UTI cuti = cnSym->getUlamTypeIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    m_state.setThisClassForParsing(cuti);

    if(m_state.getConcreteClassFlagForParsing())
      {
	cnSym->setConcreteClass();
	m_state.clearConcreteClassFlagForParsing();
      }

    //mostly needed for code gen later, but this is where we first know it!
    m_state.pushClassContext(cuti, cnSym->getClassBlockNode(), cnSym->getClassBlockNode(), false, NULL); //null blocks likely

    //set ulam class type in UlamType (through its class symbol) since we know it;
    //UC_UNSEEN if unseen so far. m_state.resetUnseenClassId(cnSym, iTok); wait..

    switch(pTok.m_type)
      {
      case TOK_KW_ELEMENT:
	{
	  AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(cut->getUlamKeyTypeSignature(), Class, UC_ELEMENT, cut->isCustomArray());
	  assert(isReplaced);
	  m_state.isEmptyElement(cuti); //t3802
	  break;
	}
      case TOK_KW_QUARK:
	{
	  AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(cut->getUlamKeyTypeSignature(), Class, UC_QUARK, cut->isCustomArray());
	  assert(isReplaced);
	  break;
	}
      case TOK_KW_QUARKUNION:
	{
	  AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(cut->getUlamKeyTypeSignature(), Class, UC_QUARK, cut->isCustomArray());
	  assert(isReplaced);
	  cnSym->setQuarkUnion();
	  break;
	}
      case TOK_KW_TRANSIENT:
	{
	  AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(cut->getUlamKeyTypeSignature(), Class, UC_TRANSIENT, cut->isCustomArray());
	  assert(isReplaced);
	  break;
	}
      default:
	m_state.abortUndefinedUlamClassType(); //checked prior
      };

    cnSym->setStructuredComment(); //also clears

    NodeBlockClass * classNode = parseClassBlock(cnSym, iTok); //we know its type too..sweeter
    assert(cnSym);
    if(classNode)
      {
	//Wait (better errmsgs) until classNode known before resetUnseen:
	//removes fm Unseen set, and changes class locator (t3866, t3867, t41432)
	m_state.resetUnseenClassId(cnSym, iTok);

	if(wasIncomplete && isTemplate)
	  ((SymbolClassNameTemplate *) cnSym)->fixAnyUnseenClassInstances();
      }
    else
      {
	//reset to incomplete (back to UC_UNSEEN)
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);

	AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(cut->getUlamKeyTypeSignature(), Class, UC_UNSEEN, cut->isCustomArray());
	assert(isReplaced);

	cnSym->setClassBlockNode(NULL); //not reset, t3130, etc.; t41451 assertion.
	std::ostringstream msg;
	msg << "Empty/Incomplete Class Definition '";
	msg << m_state.getTokenDataAsString(iTok).c_str();
	msg << "'; Possible missing ending curly brace";
	MSG(&pTok, msg.str().c_str(), ERR); //t41432
      }

    m_state.clearThisClassForParsing();
    return false; //keep going until EOF is reached
  } //parseThisClass

  NodeBlockClass * Parser::parseClassBlock(SymbolClassName * cnsym, const Token& identTok)
  {
    bool inherits = false;
    UTI utype = cnsym->getUlamTypeIdx(); //we know its type..sweeter
    NodeBlockClass * rtnNode = cnsym->getClassBlockNode(); //usually NULL
    if(rtnNode == NULL)
      {
	//this is the class' first block; o.w. null
	NodeBlock * prevBlock = m_state.getCurrentBlock();

	rtnNode = new NodeBlockClass(prevBlock, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(identTok.m_locator);
	rtnNode->setNodeType(utype);

	//set block before returning, so future class instances can link back to it
	cnsym->setClassBlockNode(rtnNode);
      }
    else
      {
	//this is the class' first block; o.w. null
	NodeBlock * prevBlock = m_state.getCurrentBlock();
	if(prevBlock != rtnNode)
	  rtnNode->setPreviousBlockPointer(prevBlock);
	rtnNode->setNodeLocation(identTok.m_locator); //missing
	assert(utype == rtnNode->getNodeType());
      }

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    //class block has 2 ST: functions and data member decls, separate
    //m_state.popClassContext(); //keep on stack for name id
    m_state.pushClassContext(utype, rtnNode, rtnNode, false, NULL);

    //automatically create a Self typedef symbol for this class type
    assert(!m_state.isReference(utype)); //for the record.
    Token SelfTok(TOK_KW_TYPE_SELF, identTok.m_locator, 0);
    SymbolTypedef * symtypedef = new SymbolTypedef(SelfTok, utype, utype, m_state); //selftype
    assert(symtypedef);
    m_state.addSymbolToCurrentScope(symtypedef);

    //need class block's ST before parsing any class parameters (i.e. named constants);
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_OPEN_PAREN)
      parseRestOfClassParameters((SymbolClassNameTemplate *) cnsym, rtnNode);
    else
      unreadToken();

    Token qTok;
    getNextToken(qTok);

    if(qTok.m_type == TOK_COLON)
      {
	inherits = parseRestOfMultiClassInheritance(cnsym, 0, true); //t41312
      }
    else
      {
	unreadToken();
	cnsym->setBaseClass(Nouti, 0); //clear

	//earliest ancestor when none designated; for all classes except UrSelf,
	if(!m_state.isUrSelf(cnsym->getUlamTypeIdx()))
	  setupSuperClassHelper(cnsym);
      }

    //ulam5 offers inheritance from multiple classes; superclass optional.
    Token rTok;
    getNextToken(rTok);
    unreadToken();

    if((rTok.m_type == TOK_PLUS))
      {
	parseMultipleClassInheritances(cnsym);
      }

    if(!getExpectedToken(TOK_OPEN_CURLY, pTok))
      {
	if(pTok.m_type == TOK_COLON)
	  {
	    std::ostringstream msg;
	    msg << "Inheritance for template class identifier '";
	    msg << m_state.getTokenDataAsString(identTok).c_str();
	    msg << "' unsupported";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	delete rtnNode;
	return NULL;
      }

    if(getExpectedToken(TOK_CLOSE_CURLY))
      {
	rtnNode->setEmpty();
	return rtnNode; //allow empty class
      }

    //append data members to class block's tree
    while(parseDataMember());

    //quietly, message later, e.g. 3130
    if(!getExpectedToken(TOK_CLOSE_CURLY))
      {
	delete rtnNode;
	rtnNode = NULL;
      }
    else
      {
	m_state.checkforAnyRemainingCulamGeneratedTypedefsInThisContext(cnsym->getUlamTypeIdx()); //t3347,t3544, ulamexports
	if (cnsym->isClassTemplate())
	  m_state.checkForAnyIncompleteTemplateClassInstances(cnsym->getUlamTypeIdx());
      }

    //this block's ST is no longer in scope
    m_state.popClassContext(); //m_currentBlock = prevBlock;

    if(inherits)
      m_state.popClassContext(); //m_currentBlock = prevBlock;

    return rtnNode;
  } //parseClassBlock

  void Parser::setupSuperClassHelper(SymbolClassName * cnsym)
  {
    u32 urid = m_state.m_pool.getIndexForDataString("UrSelf");
    assert(cnsym && cnsym->getId() != urid);

    SymbolClassName * ursym = NULL;
    AssertBool isDef = m_state.alreadyDefinedSymbolClassName(urid, ursym);
    assert(isDef);
    UTI urself = ursym->getUlamTypeIdx();
    assert(m_state.isUrSelf(urself)); //sanity
    cnsym->setBaseClass(urself, 0); //t41184
  }

  void Parser::setupSuperClassHelper(UTI superuti, SymbolClassName * cnsym)
  {
    assert(cnsym);

    NodeBlockClass * classblock = cnsym->getClassBlockNode();
    assert(classblock); //rtnNode in caller


    u32 superid = m_state.m_pool.getIndexForDataString("Super");
    Symbol * symtypedef = NULL;
    if(!classblock->isIdInScope(superid, symtypedef))
      {
	Token superTok(TOK_KW_TYPE_SUPER, classblock->getNodeLocation(), 0);
	symtypedef = new SymbolTypedef(superTok, superuti, superuti, m_state);
	assert(symtypedef);
	m_state.addSymbolToCurrentScope(symtypedef);
      }
    else //holder may have been made prior
      {
	assert(symtypedef->getId() == superid);
	UTI stuti = symtypedef->getUlamTypeIdx();
	if(stuti != superuti) //t3808, t3806, t3807
	  symtypedef->resetUlamType(superuti);
      }
  } //setupSuperClassHelper

  void Parser::parseRestOfClassParameters(SymbolClassNameTemplate * cntsym, NodeBlockClass * cblock)
  {
    assert(cntsym);

    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      {
	if(cntsym->getNumberOfParameters() == 0)
	  {
	    std::ostringstream msg;
	    msg << "Class Template '";
	    msg << m_state.m_pool.getDataAsString(cntsym->getId()).c_str();
	    msg << "' has NO parameters; Parens are inapplicable";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	return; //done with parameters
      }

    //allows class name to be same as parameter name
    //since the class starts a new "block" (i.e. ST);
    //the argument to parseDecl will prevent it from looking for restofdecls
    if(Token::isTokenAType(pTok) || (pTok.m_type == TOK_KW_LOCALDEF))
      {
	unreadToken();

	//once a parameter has a default value expression
	// subsequent parameters must also to avoid ambiguity when instaniated
	u32 numparams = cntsym->getNumberOfParameters();
	bool assignrequired = ( (numparams == 0) ? NOASSIGNOK : cntsym->parameterHasDefaultValue(numparams - 1));

	assert(m_state.m_parsingVariableSymbolTypeFlag == STF_NEEDSATYPE); //sanity
	m_state.m_parsingVariableSymbolTypeFlag = STF_CLASSPARAMETER; //t41213

	Node * argNode = parseClassParameterConstdef(assignrequired);

	m_state.m_parsingVariableSymbolTypeFlag = STF_NEEDSATYPE;

	//Symbol * argSym = NULL;

	//could be null symbol already in scope
	if(argNode)
	  {
	    //potentially needed to resolve its node type
	    assert(cblock);
	    cblock->addParameterNode(argNode);

	    //sanity check for default values
	    assert(!assignrequired || ((NodeConstantDef *) argNode)->hasConstantExpr());
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Problem with parameter definition " << numparams + 1;
	    msg << " for template class: ";
	    msg << m_state.m_pool.getDataAsString(cntsym->getId()).c_str();
	    MSG(&pTok, msg.str().c_str(), ERR); //t3524
	    return;
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Expected a 'Type' Token!! got Token <";
	msg << m_state.getTokenDataAsString(pTok).c_str();
	msg << "> instead for class parameter declaration";
	MSG(&pTok, msg.str().c_str(), ERR);
	return;
      }

    getExpectedToken(TOK_COMMA); //if so, get next parameter; o.w. unread

    return parseRestOfClassParameters(cntsym, cblock);
  } //parseRestOfClassParameters

  bool Parser::parseMultipleClassInheritances(SymbolClassName * cnsym)
  {
    bool rtninherits = false;
    assert(cnsym);

    //ulam5 offers inheritance of multiple classes
    Token rTok;
    getNextToken(rTok);

    while((rTok.m_type == TOK_PLUS))
      {
	rtninherits = parseRestOfMultiClassInheritance(cnsym, 1, true); //all bases are shared
	getNextToken(rTok);
      }

    unreadToken();
    return rtninherits;
  } //parseMultipleClassInheritances

  bool Parser::parseRestOfMultiClassInheritance(SymbolClassName * cnsym, u32 item, bool sharedVirtualBase)
  {
    bool rtninherits = false;
    assert(cnsym);
    UTI instance = cnsym->getUlamTypeIdx();

    //inheritance, all shared bases
    Token iTok;
    getNextToken(iTok);

    //forward declaration of localdef (t3874)
    if(iTok.m_type == TOK_KW_LOCALDEF)
      {

	if(!getExpectedToken(TOK_DOT))
	  {
	    return false;
	  }

	return parseRestOfMultiClassInheritanceUsingALocaldef(cnsym, item, sharedVirtualBase);
      } //done w explicit local. scope


    if(iTok.m_type == TOK_TYPE_IDENTIFIER)
      {
	TypeArgs typeargs;
	typeargs.init(iTok);

	m_state.m_parsingVariableSymbolTypeFlag = STF_CLASSINHERITANCE;

	Token pTok;
	getNextToken(pTok);
	unreadToken();

	//t3866,7, t41014
	bool isaclass = true;
	UTI baseuti = parseClassArguments(typeargs, isaclass);

	if(m_state.okUTItoContinue(baseuti))
	  {
	    u32 baseid = m_state.getUlamTypeNameIdByIndex(baseuti);
	    u32 instanceid = m_state.getUlamTypeNameIdByIndex(instance);

	    if((baseid == instanceid) || (iTok.m_dataindex == instanceid))
	      {
		std::ostringstream msg;
		msg << "Class Definition '";
		msg << m_state.getUlamTypeNameBriefByIndex(instance).c_str(); //t3900, t3901
		msg << "'; Provides an invalid Baseclass '";
		msg << m_state.getTokenDataAsString(iTok).c_str() << "'";
		MSG(&iTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		if(item == 0)
		  cnsym->setSuperClassForClassInstance(baseuti, instance); //set super here!!
		else
		  cnsym->appendBaseClassForClassInstance(baseuti, instance, sharedVirtualBase); //set here!!
		rtninherits = true;
	      }
	  }
	else if(baseuti == Hzy)
	  {
	    if(item == 0)
	      cnsym->setSuperClassForClassInstance(baseuti, instance); //set super here!!
	    else
	      cnsym->appendBaseClassForClassInstance(baseuti, instance, sharedVirtualBase); //set here!!
	    rtninherits = true;
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Class Definition '";
	msg << m_state.getUlamTypeNameBriefByIndex(instance).c_str(); //t3786
	msg << "'; Provides an invalid Class identifier '";
	msg << m_state.getTokenDataAsString(iTok).c_str() << "'";
	MSG(&iTok, msg.str().c_str(), ERR);
      }
    m_state.m_parsingVariableSymbolTypeFlag = STF_NEEDSATYPE; //reset
    return rtninherits;
  } //parseRestOfMultiClassInheritance

  bool Parser::parseRestOfMultiClassInheritanceUsingALocaldef(SymbolClassName * cnsym, u32 item, bool sharedVirtualBase)
  {
    bool rtninherits = false;
    assert(cnsym);
    UTI instance = cnsym->getUlamTypeIdx();

    NodeBlockLocals * localsblock = m_state.makeLocalsScopeBlock(m_state.getContextBlockLoc());
    assert(localsblock);

    m_state.pushClassContext(localsblock->getNodeType(), localsblock, localsblock, false, NULL);

    Token iTok;
    getNextToken(iTok);

    if(iTok.m_type == TOK_TYPE_IDENTIFIER) //t41312
      {
	u32 tokid = m_state.getTokenDataAsStringId(iTok);
	//check if aleady a typedef..else generate one in locals filescope
	UTI tduti = Nav;
	UTI tdscalaruti = Nouti;
	if(m_state.getUlamTypeByTypedefName(tokid, tduti, tdscalaruti) != TBOOL_TRUE)
	  {
	    //make a typedef holder for a class
	    UTI superuti = m_state.makeCulamGeneratedTypedefSymbolInCurrentContext(iTok, true);
	    if(item == 0)
	      cnsym->setSuperClassForClassInstance(superuti, instance); //set super here!!
	    else
	      cnsym->appendBaseClassForClassInstance(superuti, instance, sharedVirtualBase); //set here!!
	  }
	else
	  {
	    //typedef already defined! must be a localdef
	    UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
	    if(tdut->isPrimitiveType())
	      {
		//error! using non-class typedef (t41519)
		std::ostringstream msg;
		msg << "Class Definition '";
		msg << m_state.getUlamTypeNameBriefByIndex(cnsym->getUlamTypeIdx()).c_str();
		msg << "'; Inheritance from '";
		msg << m_state.m_pool.getDataAsString(tokid).c_str() << "', type: ";
		msg << m_state.getUlamTypeNameByIndex(tduti).c_str();
		MSG(&iTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		if(tdut->isHolder() && (tdut->getUlamClassType() == UC_NOTACLASS))
		  {
		    //now we know it's a class! iTok "Soo3" for loc  (t41010)
		    m_state.makeAnonymousClassFromHolder(tduti, iTok.m_locator);
		    //m_state.addUnseenClassId(tokid); //ulamexports?
		  }

		if(item == 0)
		  cnsym->setSuperClassForClassInstance(tduti, instance); //set super here!! (t41514)
		else
		  cnsym->appendBaseClassForClassInstance(tduti, instance, sharedVirtualBase); //set here!!
		rtninherits = true; //typedef exists (t41009, t41010)
	      }
	  }
      }
    else  //not proper type for inheritance
      {
	std::ostringstream msg;
	msg << "Class Definition '";
	msg << m_state.getUlamTypeNameBriefByIndex(cnsym->getUlamTypeIdx()).c_str();
	msg << "'; Multiple Inheritance from invalid Locals Filescope identifier '";
	msg << m_state.getTokenDataAsString(iTok).c_str() << "'";
	MSG(&iTok, msg.str().c_str(), ERR);
      }

    m_state.popClassContext(); //restore
    return rtninherits; //done w explicit local. scope
  } //parseRestOfMultiClassInheritanceUsingALocaldef

  bool Parser::parseLocalDef()
  {
    bool brtn = false;
    NodeBlockLocals * localsblock = m_state.makeLocalsScopeBlock(m_state.getLocalsScopeLocator());
    assert(localsblock);

    m_state.pushCurrentBlock(localsblock); //so Symbol get's correct ST NodeNo.

    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_KW_TYPEDEF)
      {
	//parse Typedef's starting with keyword first
	brtn = parseTypedef();
      }
    else if(pTok.m_type == TOK_KW_CONSTDEF)
      {
	//parse Named Constant starting with keyword first
	brtn = parseConstdef();
      }
    else
      {
	std::ostringstream msg;
	msg << "Local filescope definitions are named constants and typedefs; not '"; //t3854
	msg << m_state.getTokenDataAsString(pTok).c_str() << "'";
	MSG(&pTok, msg.str().c_str(), ERR);
	getTokensUntil(TOK_SEMICOLON); //does this help?
      }

    m_state.popClassContext();

    if(brtn)
      {
	if(!getExpectedToken(TOK_SEMICOLON))
	  {
	    getTokensUntil(TOK_SEMICOLON); //does this help?
	    brtn = false;
	  }
      }
    return brtn;
  } //parseLocalDef

  bool Parser::parseDataMember()
  {
    bool brtn = false;
    bool isAlreadyAppended = false;
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_CLOSE_CURLY)
      {
	unreadToken();
	return false; //done!
      }

    if(pTok.m_type == TOK_KW_TYPEDEF)
      {
	//parse Typedef's starting with keyword first
	m_state.m_parsingVariableSymbolTypeFlag = STF_MEMBERTYPEDEF;

	isAlreadyAppended = parseTypedef();

	m_state.m_parsingVariableSymbolTypeFlag = STF_NEEDSATYPE;
      }
    else if(pTok.m_type == TOK_KW_CONSTDEF)
      {
	//parse Named Constant starting with keyword first

	// simulated "data member" flag for constantdef symbols; to
	// distinguish between function scope and filescope constants;
	// not set in Symbol constructor, like Localfilescope flag, since
	// ClassParameter constant defs (e.g. t3328, t3329, t3330..)
	// also have the same BlockNoST as its class.
	m_state.m_parsingVariableSymbolTypeFlag = STF_MEMBERCONSTANT; //was STF_DATAMEMBER

	isAlreadyAppended = parseConstdef();

	m_state.m_parsingVariableSymbolTypeFlag = STF_NEEDSATYPE;
      }
    else if(pTok.m_type == TOK_KW_PARAMETER)
      {
	//static model parameter for elements, not quarks
	isAlreadyAppended = parseModelParameter();
      }
    else //regular data member: function or variable
      {
	bool isVirtual = false;
	bool insureVirtualOverride = false;
	if(pTok.m_type == TOK_KW_FLAG_VIRTUALOVERRIDE)
	  {
	    //override used to insure this function overrides superclass' func of same desc;
	    //(virtual keyword optional)
	    insureVirtualOverride = isVirtual = true; //t41096
	    getNextToken(pTok);
	  }

	if(pTok.m_type == TOK_KW_VIRTUAL)
	  isVirtual = true;
	else
	  unreadToken(); //put back for parsing type descriptor

	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, false, true);

	if(typeNode == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Expecting Type of data member/function";
	    msg << " (or, in the midst of errors); Got <";
	    msg << m_state.getTokenDataAsString(pTok).c_str() << ">";
	    MSG(&pTok, msg.str().c_str(), ERR);

	    m_state.clearStructuredCommentToken(); //missing
	    return false; //expecting a type, not sizeof, probably an error! (t3856,t41350)
	  }

	bool isConstr = false;

	Token iTok;
	getNextToken(iTok);
	if(iTok.m_type != TOK_IDENTIFIER)
	  {
	    if((iTok.m_type == TOK_OPEN_PAREN) && (pTok.m_type == TOK_KW_TYPE_SELF))
	      {
		unreadToken();
		isConstr = true; //special case!
		iTok = pTok;
		delete typeNode; //Void
		//constructor return type is Void; make one for consistency
		Token vTok(TOK_KW_TYPE_VOID, iTok.m_locator, 0);
		typeNode = new NodeTypeDescriptor(vTok, Void, m_state);
		assert(typeNode);
	      }
	    else
	      {
		//user error! (t41341,2)
		Token eTok = iTok;
		if(iTok.m_type == TOK_OPEN_PAREN)
		  eTok = pTok;
		std::ostringstream msg;
		msg << "Name of variable/function '";
		msg << m_state.getTokenDataAsString(eTok).c_str();
		if((Token::getSpecialTokenWork(eTok.m_type) == TOKSP_KEYWORD))
		  msg << "': Identifier must not be a reserved keyword";
		else
		  msg << "': Identifier must begin with a lower-case letter";
		MSG(&iTok, msg.str().c_str(), ERR);
		m_state.clearStructuredCommentToken();
		delete typeNode;
		typeNode = NULL;
		unreadToken();
		return false; //done!
	      }
	  }

	//need next token to distinguish a function from a variable declaration (quietly)
	if(getExpectedToken(TOK_OPEN_PAREN))
	  {
	    //eats the '(' when found; NULL if error occurred
	    NodeBlockFunctionDefinition * funcdefNode = makeFunctionSymbol(typeargs, iTok, typeNode, isVirtual, insureVirtualOverride, isConstr); //with params
	    if(funcdefNode)
	      {
		SymbolFunction * funcsym = funcdefNode->getFuncSymbolPtr();
		assert(funcsym);

		//append this little guy to tree to preserve order of function declarations (ulam-5)
		NodeFuncDecl * funcdecl = new NodeFuncDecl(funcsym,m_state);
		assert(funcdecl);
		funcdecl->setNodeLocation(iTok.m_locator);
		m_state.appendNodeToCurrentBlock(funcdecl);

		brtn = true; //funcdefNode belongs to the symbolFunction; not appended to tree
	      }
	    else
	      //MSG(&pTok, "INCOMPLETE Function Definition", ERR);
	      m_state.clearStructuredCommentToken();
	  }
	else
	  {
	    if(isVirtual)
	      {
		std::ostringstream msg;
		msg << "Unexpected input!! Token <";
		if(insureVirtualOverride)
		  msg << Token::getTokenAsStringFromPool(TOK_KW_FLAG_VIRTUALOVERRIDE, &m_state).c_str() << ">";
		else
		  msg << m_state.getTokenDataAsString(pTok).c_str() << ">";
		msg << " applies to functions";
		MSG(&pTok, msg.str().c_str(), ERR);
		//continue
	      }

	    if(typeargs.m_declRef != ALT_NOT)
	      {
		std::ostringstream msg;
		msg << "Reference as data member; Not supported";
		MSG(&pTok, msg.str().c_str(), ERR);
		m_state.clearStructuredCommentToken();
		delete typeNode;
		typeNode = NULL;
		return false; //done!
	      }

	    m_state.m_parsingVariableSymbolTypeFlag = STF_DATAMEMBER;

	    //not '(', so token is unread, and we know
	    //it's a variable, not a function; also handles arrays
	    UTI passuti = typeNode->givenUTI(); //before it may become an array
	    NodeVarDecl * dmNode = makeVariableSymbol(typeargs, iTok, typeNode);

	    if(dmNode)
	      {
		parseRestOfInitialization(iTok, dmNode);
		m_state.appendNodeToCurrentBlock(dmNode);
		isAlreadyAppended = parseRestOfDataMember(typeargs, passuti); //appended
	      }
	    else
	      m_state.clearStructuredCommentToken();

	    m_state.m_parsingVariableSymbolTypeFlag = STF_NEEDSATYPE;
	  }
      } //regular data member

    //common end processing, except for function defs
    if(isAlreadyAppended)
      {
	Token tmpTok;
	if(!getExpectedToken(TOK_SEMICOLON, tmpTok))
	  getTokensUntil(TOK_SEMICOLON); //does this help? t41432 bad typedef
	else
	  brtn = true;
      }
    return brtn;
  } //parseDataMember

  bool Parser::parseRestOfDataMember(TypeArgs& args, UTI passuti)
  {
    bool brtn = true;
    Token pTok;
    getNextToken(pTok);

    args.m_arraysize = NONARRAYSIZE; //clear for decl list (args ref)

    if(pTok.m_type != TOK_COMMA)
      {
	unreadToken();
	return true;
      }

    Token iTok;
    getNextToken(iTok);
    if(iTok.m_type == TOK_IDENTIFIER)
      {
	//just the top level as a basic uti (no selects, or arrays)
	NodeTypeDescriptor * typeNode = new NodeTypeDescriptor(args.m_typeTok, passuti, m_state);
	//another decl of same type
	NodeVarDecl * sNode = makeVariableSymbol(args, iTok, typeNode); //a decl
	if (sNode)
	  {
	    parseRestOfInitialization(iTok, sNode);
	    m_state.appendNodeToCurrentBlock(sNode);
	  }
	else  //error msg?
	  brtn = false;
      }
    else
      {
	//perhaps read until semi-colon
	getTokensUntil(TOK_SEMICOLON);
	unreadToken();
	brtn = false;
      }

    if(brtn)
      return parseRestOfDataMember(args, passuti); //iTok in case of =

    return false;
  } //parseRestOfDataMember

  bool Parser::parseRestOfInitialization(const Token& identTok, Node * dNode)
  {
    bool brtn = true;
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_EQUAL)
      {
	Token eTok;
	//check for possible start of array init
	getNextToken(eTok);
	unreadToken();
	Node * initnode = NULL;
	if(eTok.m_type == TOK_OPEN_CURLY)
	  {
	    //returns a NodeList
	    initnode = parseArrayOrClassInitialization(identTok.m_dataindex);
	  }
	else
	  {
	    initnode = parseExpression();
	  }

	if(initnode == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Initial value of '" << m_state.getTokenDataAsString(identTok).c_str();
	    msg << "' is missing";
	    MSG(&identTok, msg.str().c_str(), ERR);
	    brtn = false;
	  }
	else
	  ((NodeVarDecl *) dNode)->setInitExpr(initnode);
      }
    else if(pTok.m_type == TOK_OPEN_PAREN)
      {
	brtn = makeDeclConstructorCall(identTok, (NodeVarDecl *) dNode);
      }
    else
      unreadToken();
    return brtn; // true even if no assignment; false on error
  } //parseRestOfInitialization

  //refactor, open paren seen already, for data member initialization
  bool Parser::makeDeclConstructorCall(const Token& identTok, NodeVarDecl * dNode)
  {
    bool brtn = true;
    //open paren already eaten

    NodeFunctionCall * constrNode = parseConstructorCall(identTok);

    if(constrNode == NULL)
      {
	return false; //t3135
      }

    NodeIdent * classNode = new NodeIdent(identTok, NULL, m_state);
    assert(classNode);
    classNode->setNodeLocation(identTok.m_locator);

    NodeMemberSelectOnConstructorCall * memberSelectNode = new NodeMemberSelectOnConstructorCall(classNode, constrNode, m_state);
    assert(memberSelectNode);
    memberSelectNode->setNodeLocation(identTok.m_locator);

    ((NodeVarDecl *) dNode)->setInitExpr(memberSelectNode);
    return brtn; // true even if no assignment; false on error
  } //makeDeclConstructorCall

  Node * Parser::makeInstanceofConstructorCall(const Token& fTok, Node * memberNode, NodeTypeDescriptor * nodetype)
  {
    //open paren already eaten
    NodeFunctionCall * constrNode = parseConstructorCall(fTok); //tok for loc and errmsgs

    if(constrNode == NULL)
      {
	return NULL;
      }

    //make here to avoid seqfault by deleting nodetype twice! (t41402)
    NodeInstanceof * instanceofNode = new NodeInstanceof(memberNode, nodetype, m_state);
    assert(instanceofNode);
    instanceofNode->setNodeLocation(fTok.m_locator);

    NodeMemberSelectOnConstructorCall * memberSelectNode = new NodeMemberSelectOnConstructorCall(instanceofNode, constrNode, m_state);
    assert(memberSelectNode);
    memberSelectNode->setNodeLocation(fTok.m_locator);

    return memberSelectNode;
  } //makeInstanceofConstructorCall

  NodeFunctionCall * Parser::parseConstructorCall(const Token& identTok)
  {
    Token SelfTok(TOK_KW_TYPE_SELF, identTok.m_locator, 0);
    //if here, must be constructor used to initialize class-type variable!! (t41077)
    NodeFunctionCall * constrNode = (NodeFunctionCall *) parseFunctionCall(SelfTok); //type of variable known later

    if(constrNode == NULL)
      {
	std::ostringstream msg;
	msg << "Initial value of '" << m_state.getTokenDataAsString(identTok).c_str();
	msg << "' is missing its constructor arguments";
	MSG(&identTok, msg.str().c_str(), ERR);
      }
    else if(constrNode->getNumberOfArguments() == 0)
      {
	std::ostringstream msg;
	msg << "Initial value of '" << m_state.getTokenDataAsString(identTok).c_str();
	msg << "' has no constructor arguments";
	MSG(&identTok, msg.str().c_str(), ERR);
	delete constrNode;
	constrNode = NULL;
      }
    return constrNode;
  } //parseConstructorCall

  NodeBlock * Parser::parseBlock()
  {
    Token pTok;
    NodeBlock * rtnNode = NULL;
    NodeBlock * prevBlock = m_state.getCurrentBlock();

    if(!getExpectedToken(TOK_OPEN_CURLY, pTok))
      return NULL;

    if(getExpectedToken(TOK_CLOSE_CURLY))
      {
	rtnNode = new NodeBlockEmpty(prevBlock, m_state); //legal
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	return rtnNode;
      }

    rtnNode = new NodeBlock(prevBlock, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(pTok.m_locator);

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    m_state.pushCurrentBlock(rtnNode);

    if(!parseStatements())
      {
	//replace NodeBlock with a NodeBlockEmpty
	delete rtnNode;
	rtnNode = NULL;

	rtnNode = new NodeBlockEmpty(prevBlock, m_state); //legal
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	m_state.popClassContext();
	m_state.pushCurrentBlock(rtnNode); //very temporary
      }

    if(!getExpectedToken(TOK_CLOSE_CURLY))
      {
	delete rtnNode;
	rtnNode = NULL;
	getTokensUntil(TOK_CLOSE_CURLY);
	m_state.popClassContext();
	m_state.pushCurrentBlock(rtnNode); //very temporary, t41138
      }

    m_state.popClassContext();
    //sanity check
    assert(!rtnNode || rtnNode->getPreviousBlockPointer() == prevBlock);
    return rtnNode;
  } //parseBlock

  bool Parser::parseStatements()
  {
    bool brtn = true;
    Node * sNode = parseStatement();
    if(sNode == NULL)  //e.g. due to an invalid token perhaps; decl already appended
      {
	MSG("", "EMPTY STATEMENT!! when in doubt, count", DEBUG);

	Token pTok;
	getNextToken(pTok);
	unreadToken();
	if((pTok.m_type != TOK_CLOSE_CURLY) && (pTok.m_type != TOK_EOF))
	  return parseStatements(); //try again
	else
	  {
	    NodeBlock * currblock = m_state.getCurrentBlock();
	    assert(currblock);
	    //not the same if decl appended.
	    return (currblock != currblock->getLastStatementPtr());
	  }
      }
    //else continue...
    m_state.appendNodeToCurrentBlock(sNode);

    if(!getExpectedToken(TOK_CLOSE_CURLY))
      brtn = parseStatements(); //more statements, appended
    else
      unreadToken(); //brtn = false; t3126
    return brtn; //rtnNode;
  } //parseStatements

  Node * Parser::parseStatement()
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type == TOK_OPEN_CURLY)
      {
	unreadToken();
	rtnNode = parseBlock();
      }
    else if(Token::getSpecialTokenWork(pTok.m_type) == TOKSP_CTLKEYWORD)
      {
	unreadToken();
	rtnNode = parseControlStatement(); //last token either a } or ;
      }
    else
      {
	unreadToken();
	parseSimpleStatement(); //may be null (only ;) o.w. already appended
      }
    return rtnNode;
  } //parseStatement

  NodeBlock * Parser::parseStatementAsBlock()
  {
    NodeBlock * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    unreadToken();
    if(pTok.m_type == TOK_OPEN_CURLY)
      {
	rtnNode = parseBlock();
      }
    else
      {
	//create a fake block to hold the statement (e.g. decl)
	NodeBlock * currBlock = m_state.getCurrentBlock();
	rtnNode = new NodeBlockInvisible(currBlock, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);

	//current, this block's symbol table added to parse tree stack
	//        for validating and finding scope of program/block variables
	m_state.pushCurrentBlock(rtnNode); //without pop first
	Node * sNode = parseStatement();
	if(sNode)  //e.g. due to an invalid token perhaps; decl already appended
	  m_state.appendNodeToCurrentBlock(sNode);
	//else already appended
	//assert(m_state.getCurrentBlock() == rtnNode); //sanity
	m_state.popClassContext(); //restore
      }
    return rtnNode;
  } //parseStatementAsBlock

  Node * Parser::parseControlStatement()
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    //should have already known to be true
    assert(Token::getSpecialTokenWork(pTok.m_type) == TOKSP_CTLKEYWORD);

    switch(pTok.m_type)
      {
      case TOK_KW_IF:
	rtnNode = parseControlIf(pTok);
	break;
      case TOK_KW_WHILE:
	{
	  m_state.m_parsingControlLoopsSwitchStack.push(pTok, m_state.getNextTmpVarNumber());
	  rtnNode = parseControlWhile(pTok);
	  m_state.m_parsingControlLoopsSwitchStack.pop();
	}
	break;
      case TOK_KW_FOR:
	{
	  m_state.m_parsingControlLoopsSwitchStack.push(pTok, m_state.getNextTmpVarNumber());
	  rtnNode = parseControlFor(pTok);
	  m_state.m_parsingControlLoopsSwitchStack.pop();
	}
	break;
      case TOK_KW_SWITCH:
	{
	  m_state.m_parsingControlLoopsSwitchStack.push(pTok, m_state.getNextTmpVarNumber());
	  rtnNode = parseControlSwitch(pTok);
	  m_state.m_parsingControlLoopsSwitchStack.pop();
	}
	break;
      case TOK_ERROR_LOWLEVEL:
	//eat error token
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token <";
	  msg << m_state.getTokenDataAsString(pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  //unreadToken(); leads to infinite loop
	}
	break;
      };
    return rtnNode;
  } //parseControlStatement

  Node * Parser::parseControlIf(const Token& ifTok)
  {
    Token ftmpTok;
    if(!getExpectedToken(TOK_OPEN_PAREN,ftmpTok))
      return NULL; //non-quietly, t41467

    //before parsing the IF statement, need a new scope
    NodeBlock * currBlock = m_state.getCurrentBlock();
    NodeBlock * rtnNode = new NodeBlock(currBlock, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(ifTok.m_locator);

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    m_state.pushCurrentBlock(rtnNode); //without pop first

    Node * condNode = parseConditionalExpr();
    if(condNode == NULL)
      {
	std::ostringstream msg;
	msg << "Invalid if-condition"; //t3864
	MSG(&ifTok, msg.str().c_str(), ERR);
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    Token tmpTok; //not quietly t41293; fix effects t3247, t3371, t3398, t3603
    if(!getExpectedToken(TOK_CLOSE_PAREN, tmpTok))
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    NodeBlock * trueNode = NULL;
    if(m_state.m_parsingConditionalAs)
      trueNode = setupAsConditionalBlockAndParseStatements((NodeConditional *) condNode);
    else
      trueNode = parseStatementAsBlock();

    if(trueNode == NULL)
      {
	std::ostringstream msg;
	msg << "Incomplete true block; if-control";
	MSG(&ifTok, msg.str().c_str(), ERR);
	delete condNode;
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    NodeBlock * falseNode = NULL;
    if(getExpectedToken(TOK_KW_ELSE))
      falseNode = parseStatementAsBlock();

    NodeControlIf * ifNode = new NodeControlIf(condNode, trueNode, falseNode, m_state);
    assert(ifNode);
    ifNode->setNodeLocation(ifTok.m_locator);

    rtnNode->appendNextNode(ifNode);

    m_state.popClassContext(); //= prevBlock;
    return rtnNode;
  } //parseControlIf

  Node * Parser::parseControlWhile(const Token& wTok)
  {
    Token wtmpTok;
    if(!getExpectedToken(TOK_OPEN_PAREN, wtmpTok))
      return NULL; //non-quietly, t41467

    u32 controlLoopLabelNum = m_state.m_parsingControlLoopsSwitchStack.getLastExitNumber(); //save at the top

    //before parsing the conditional statement, need a new scope
    NodeBlock * currBlock = m_state.getCurrentBlock();
    NodeBlock * rtnNode = new NodeBlock(currBlock, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(wTok.m_locator);

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    m_state.pushCurrentBlock(rtnNode); //without pop first

    Node * condNode = parseConditionalExpr();
    if(condNode == NULL)
      {
	std::ostringstream msg;
	msg << "Invalid while-condition";
	MSG(&wTok, msg.str().c_str(), ERR);
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    Token tmpTok; //not quietly, like t41293
    if(!getExpectedToken(TOK_CLOSE_PAREN, tmpTok))
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    assert(controlLoopLabelNum == m_state.m_parsingControlLoopsSwitchStack.getNearestContinueExitNumber()); //sanity

    Node * whileNode = makeControlWhileNode(condNode, NULL, controlLoopLabelNum, wTok);
    if(whileNode == NULL)
      {
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    rtnNode->appendNextNode(whileNode);

    m_state.popClassContext(); //= prevBlock;
    return rtnNode;
  } //parseControlWhile

  Node * Parser::parseControlFor(const Token& fTok)
  {
    Token ftmpTok;
    if(!getExpectedToken(TOK_OPEN_PAREN, ftmpTok))
      return NULL; //non-quietly, t41467

    u32 controlLoopLabelNum = m_state.m_parsingControlLoopsSwitchStack.getLastExitNumber(); //save at the top

    //before parsing the FOR statement, need a new scope
    NodeBlock * currBlock = m_state.getCurrentBlock();
    NodeBlock * rtnNode = new NodeBlock(currBlock, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(fTok.m_locator);

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    m_state.pushCurrentBlock(rtnNode); //without pop first

    Token pTok;
    getNextToken(pTok);

    if(Token::isTokenAType(pTok))
      {
	unreadToken();
	bool hasdeclnode = parseDecl(); //updates symbol table
	if(!hasdeclnode)
	  {
	    std::ostringstream msg;
	    msg << "Malformed for-control, initial variable decl";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	getNextToken(pTok);
      }
    else if(pTok.m_type == TOK_IDENTIFIER)
      {
	unreadToken();
	Node * declNode = parseAssignExpr();
	if(declNode)
	  rtnNode->appendNextNode(declNode);
	else
	  {
	    std::ostringstream msg;
	    msg << "Malformed for-control, initial variable assignment";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	getNextToken(pTok);
      }

    if(pTok.m_type != TOK_SEMICOLON)
      {
	std::ostringstream msg;
	msg << "Malformed for-control";
	MSG(&pTok, msg.str().c_str(), ERR);
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL;
      }

    Node * condNode = NULL;
    Token qTok;
    getNextToken(qTok);
    Token saveIdentTok;

    if(qTok.m_type != TOK_SEMICOLON)
      {
	unreadToken();
	condNode = parseConditionalExpr();
	saveIdentTok = m_state.m_identTokenForConditionalAs; //t41043

	if(condNode == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Invalid for-condition";
	    MSG(&qTok, msg.str().c_str(), ERR);
	    m_state.popClassContext(); //the pop
	    delete rtnNode;
	    return NULL; //stop this maddness
	  }

	Token tmpTok;
	if(!getExpectedToken(TOK_SEMICOLON, tmpTok))
	  {
	    m_state.popClassContext(); //the pop
	    delete rtnNode;
	    delete condNode;
	    return NULL; //stop this maddness
	  }
      }
    else
      {
	//make a 'true' (default)
	Token trueTok;
	trueTok.init(TOK_KW_TRUE, qTok.m_locator, 0);
	condNode = new NodeTerminal(trueTok, m_state);
	assert(condNode);
      } //conditional expres

    Node * assignNode = NULL;
    Token rTok;
    getNextToken(rTok);

    if(rTok.m_type != TOK_CLOSE_PAREN)
      {
	unreadToken();
	assignNode = parseAssignExpr();
	if(assignNode == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Malformed assignment; for-control";
	    MSG(&rTok, msg.str().c_str(), ERR);

	    m_state.popClassContext(); //the pop
	    delete rtnNode;
	    delete condNode;
	    return NULL; //stop this maddness
	  }

	Token tmpTok; //non quietly like t41293
	if(!getExpectedToken(TOK_CLOSE_PAREN, tmpTok))
	  {
	    m_state.popClassContext(); //the pop
	    delete rtnNode;
	    delete condNode;
	    delete assignNode;
	    return NULL; //stop this maddness
	  }
      } //done with assign expr, could be null

    assert(controlLoopLabelNum == m_state.m_parsingControlLoopsSwitchStack.getNearestContinueExitNumber()); //sanity


    //restore before calling setupAsConditionalBlockAndParseStatements (t41043)
    m_state.m_identTokenForConditionalAs = saveIdentTok;

    Node * whileNode = makeControlWhileNode(condNode, assignNode, controlLoopLabelNum, fTok);
    if(whileNode == NULL)
      {
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    //links while to decl or to rtn block (no decl)
    rtnNode->appendNextNode(whileNode);

    m_state.popClassContext(); //= prevBlock;
    return rtnNode;
  } //parseControlFor

  NodeControlWhile * Parser::makeControlWhileNode(Node * condNode, Node * assignNodeForLoop, u32 loopnum, const Token& fwtok)
  {
    assert(condNode);

    NodeBlock * trueNode = NULL;
    if(m_state.m_parsingConditionalAs)
      trueNode = setupAsConditionalBlockAndParseStatements((NodeConditional *) condNode);
    else
      trueNode = parseStatementAsBlock(); //even an empty statement has a node!

    if(trueNode == NULL)
      {
	std::ostringstream msg;
	msg << "Incomplete true block; ";
	msg << Token::getTokenAsStringFromPool(fwtok.m_type, &m_state).c_str();
	msg << "-loop";
	MSG(&fwtok, msg.str().c_str(), ERR);
	delete condNode;
	delete assignNodeForLoop;
	return NULL; //stop this maddness
      }

    NodeStatements * truestmt = new NodeStatements(trueNode, m_state);
    assert(truestmt);

    //link the pieces together..using statements, not blocks (t3902, t41002,3,4)
    // decl was first in the block for-loop;
    //loose pieces joined by NodeControlWhile:
    //end of while loop label, linked to end of body; (for-loop) before assign statement
    // skip continue loop label if 'continue' never used, like switch breaks;
    NodeStatements * labelstmt = NULL;
    u32 contloopnum = m_state.m_parsingControlLoopsSwitchStack.getNearestContinueExitNumberIfUsed();
    if(contloopnum > 0)
      {
	assert(contloopnum == loopnum);
	Node * labelNode = new NodeLabel(loopnum, m_state);
	assert(labelNode);
	labelNode->setNodeLocation(fwtok.m_locator);

	labelstmt = new NodeStatements(labelNode, m_state);
	assert(labelstmt);
	truestmt->setNextNode(labelstmt);
      }
    else
      m_state.abortNeedsATest();

    NodeStatements * assignstmt = NULL; //for-loop
    if(assignNodeForLoop)
      {
	assignstmt = new NodeStatements(assignNodeForLoop, m_state);
	assert(assignstmt);
	if(labelstmt != NULL)
	  labelstmt->setNextNode(assignstmt);
	else
	  truestmt->setNextNode(assignstmt); //for optional label
      }

    NodeControlWhile * whileNode = new NodeControlWhile(condNode, truestmt, m_state);
    assert(whileNode);
    whileNode->setNodeLocation(fwtok.m_locator);

    return whileNode;
  } //makeControlWhileNode

  Node * Parser::parseControlSwitch(const Token& swTok)
  {
    Token stmpTok;
    if(!getExpectedToken(TOK_OPEN_PAREN, stmpTok))
      return NULL; //non-quietly, t41467

    u32 switchnum = m_state.getNextTmpVarNumber();

    //before parsing the SWITCH statement, need a new scope
    NodeBlock * currBlock = m_state.getCurrentBlock();
    NodeBlockSwitch * rtnNode = new NodeBlockSwitch(currBlock, switchnum, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(swTok.m_locator);

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    m_state.pushCurrentBlock(rtnNode); //without pop first

    //peak ahead for close-paran, implying 'true' which-condition
    Token qTok;
    getNextToken(qTok);
    unreadToken();

    bool whichtrue = (qTok.m_type == TOK_CLOSE_PAREN);

    Node * condNode = parseAssignExpr(); //no 'as' (t41044)

    if((condNode == NULL) && !whichtrue)
      {
	std::ostringstream msg;
	msg << "Invalid which-control condition";
	MSG(&swTok, msg.str().c_str(), ERR);
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    Token tmpTok; //non quietly, like t41293
    if(!getExpectedToken(TOK_CLOSE_PAREN, tmpTok))
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    if(!getExpectedToken(TOK_OPEN_CURLY, tmpTok))
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    UTI huti = whichtrue ? (UTI) Bool : m_state.makeUlamTypeHolder();

    std::string swtypedefname = m_state.getSwitchTypedefNameAsString(switchnum);
    Token swtypeTok(TOK_TYPE_IDENTIFIER, swTok.m_locator, m_state.m_pool.getIndexForDataString(swtypedefname));

    //give a name to this new ulam type
    SymbolTypedef * symtypedef = new SymbolTypedef(swtypeTok, huti, Nav, m_state);
    assert(symtypedef);
    m_state.addSymbolToCurrentScope(symtypedef);
    if(!whichtrue) //t41046
      m_state.addUnknownTypeTokenToThisClassResolver(swtypeTok, huti);

    //for consistency
    NodeTypeDescriptor * typenode = new NodeTypeDescriptor(swtypeTok, huti, m_state);
    assert(typenode);

    NodeTypedef * swtypedefnode = new NodeTypedef(symtypedef, typenode, m_state); //was NULL
    assert(swtypedefnode);
    swtypedefnode->setNodeLocation(swTok.m_locator);
    m_state.appendNodeToCurrentBlock(swtypedefnode);

    NodeIdent * swvalueIdent = NULL;
    if(!whichtrue)
      {
	//evaluate switch value ONCE; put into a tmp switch var
	std::string tmpvarname = m_state.getSwitchConditionNumAsString(switchnum);
	Token tidTok(TOK_IDENTIFIER, swTok.m_locator, m_state.m_pool.getIndexForDataString(tmpvarname));
	SymbolVariableStack * swsym = new SymbolVariableStack(tidTok, huti, 0, m_state);
	assert(swsym);
	m_state.addSymbolToCurrentScope(swsym); //ownership goes to the block

	//for consistency (t41436)
	NodeTypeDescriptor * typenode2 = new NodeTypeDescriptor(swtypeTok, huti, m_state);
	assert(typenode2);

	NodeVarDecl * swvalueDecl = new NodeVarDecl(swsym, typenode2, m_state);
	assert(swvalueDecl);
	swvalueDecl->setNodeLocation(swTok.m_locator);
	swvalueDecl->setInitExpr(condNode);
	m_state.appendNodeToCurrentBlock(swvalueDecl);

	//pass switch-value variable around to each case to copy for comparison
	swvalueIdent = new NodeIdent(tidTok, swsym, m_state);
	assert(swvalueIdent);
	swvalueIdent->setNodeLocation(swTok.m_locator);
      }
    //else swvalueIdent is NULL (true)

    NodeControlIf * casesNode = NULL;
    Node * defaultcaseNode = NULL;
    parseNextCase(swvalueIdent, casesNode, defaultcaseNode);

    Token pTok;
    if(!getExpectedToken(TOK_CLOSE_CURLY, pTok))
      {
	delete casesNode;
	delete swvalueIdent;
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    //got all the cases: (empty switch ok t41036)
    if(casesNode)
      rtnNode->appendNextNode(casesNode);

    //label for end-of-switch breaks (t41018); bypass label if no breaks (t41580)
    u32 swexitnum = m_state.m_parsingControlLoopsSwitchStack.getNearestBreakExitNumberIfUsed();
    if(swexitnum > 0)
      {
	Node * labelNode = new NodeLabel(swexitnum, m_state);
	assert(labelNode);
	labelNode->setNodeLocation(pTok.m_locator);
	rtnNode->appendNextNode(labelNode);
      }

    if(defaultcaseNode != NULL)
      {
	//	((NodeBlockSwitch*)rtnNode)->setDefaultCaseNodeNo(defaultcaseNode->getNodeNo()); //t41046,..
	rtnNode->setDefaultCaseNodeNo(defaultcaseNode->getNodeNo()); //t41046,..
      }

    delete swvalueIdent; //copies made, clean up
    m_state.popClassContext(); //the pop
    return rtnNode;
  } //parseControlSwitch

  Node * Parser::parseNextCase(const NodeIdent * swvalueIdent, NodeControlIf *& switchNode, Node *& defaultNode)
  {
    //get as many cases that share the same body
    Node * casecondition = NULL;
    casecondition = parseRestOfCase(swvalueIdent, casecondition, defaultNode);

    if(casecondition == NULL)
      {
	Token eTok;
	if(!getExpectedToken(TOK_CLOSE_CURLY, eTok, QUIETLY))
	  {
	    std::ostringstream msg;
	    msg << "Incomplete condition; which-control failure";
	    MSG(&eTok, msg.str().c_str(), ERR); //e.g. t41027
	  }
	unreadToken();
	return NULL; //done
      }

    Token pTok;
    getNextToken(pTok);
    unreadToken();
    if(pTok.m_type != TOK_OPEN_CURLY)
      {
	std::ostringstream msg;
	msg << "Block expected for condition; which-control failure";
	MSG(&pTok, msg.str().c_str(), ERR); //t41023, t41024, t41028
	delete casecondition;
	return NULL;
      }

    bool isdefaultcase = (casecondition == defaultNode); //t41580
    NodeBlock * trueNode = NULL;
    // support as-condition as case expression (t41045,46,47,48)
    if(m_state.m_parsingConditionalAs)
      trueNode = setupAsConditionalBlockAndParseStatements((NodeConditional *) casecondition); //t41046
    else
      trueNode = parseStatementAsBlock(); //even an empty statement has a node!

    if(trueNode == NULL)
      {
	std::ostringstream msg;
	msg << "Incomplete true block; which-control failure";
	MSG(casecondition->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	delete casecondition;
	return NULL; //stop this maddness
      }

    //no cases after default case; 'if' needed when defaultcase is first/only (t41037)
    NodeControlIf * ifNode = NULL;
    if(!isdefaultcase || (switchNode == NULL))
      {
	ifNode = new NodeControlIf(casecondition, trueNode, NULL, m_state);
	assert(ifNode);
	ifNode->setNodeLocation(casecondition->getNodeLocation());

	if(switchNode != NULL)
	  switchNode->setElseNode(ifNode); //link to previous if-
	else
	  switchNode = ifNode; //set parent ref, t41037,8
      }
    else
      {
	assert(switchNode != NULL);
	switchNode->setElseNode(trueNode); //link to previous if-
	delete casecondition; //terminal true unneeded
	casecondition = NULL;
	defaultNode = trueNode;
      }

    return parseNextCase(swvalueIdent, ifNode, defaultNode); //recurse
  } //parseNextCase

  Node * Parser::parseRestOfCase(const NodeIdent * swvalueIdent, Node * caseCond, Node *& defaultcase)
  {
    Token cTok;
    getNextToken(cTok);
    if(! ((cTok.m_type == TOK_KW_CASE) || (cTok.m_type == TOK_KW_DEFAULT)))
      {
	unreadToken();
	return caseCond; //done
      }

    Node * casecondition = NULL;
    if(cTok.m_type == TOK_KW_CASE)
      {
	if(defaultcase != NULL)
	  {
	    std::ostringstream msg;
	    msg << "Case appears after 'otherwise' clause in which-control statement; 'otherwise' label on line ";
	    msg << defaultcase->getNodeLocation().getLineNo();
	    MSG(&cTok, msg.str().c_str(), ERR);
	    delete caseCond;
	    return NULL; //stop this maddness (e.g. t41037)
	  }

	Node * rightNode = parseConditionalExpr(); //t41039, t41046
	if(rightNode == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete case expression; which-control failure";
	    MSG(&cTok, msg.str().c_str(), ERR);
	    delete caseCond;
	    return NULL; //stop this maddness
	  }

	if(m_state.m_parsingConditionalAs)
	  {
	    Token tmpTok; //non quietly, like t41293
	    if(!getExpectedToken(TOK_COLON, tmpTok))
	      {
		delete rightNode;
		delete caseCond;
		return NULL; //stop this maddness
	      }

	    //error! can't combine conditions with 'as'
	    if(caseCond != NULL)
	      {
		std::ostringstream msg;
		msg << "Invalid case expression; which-control failure";
		msg << ": compound 'as' condition";
		MSG(&cTok, msg.str().c_str(), ERR);
		delete rightNode;
		delete caseCond;
		return NULL; //stop this maddness (t41047)
	      }

	    //which-condition type must be default true
	    if(swvalueIdent != NULL)
	      {
		std::ostringstream msg;
		msg << "Invalid case expression; which-control failure";
		msg << ": 'as' condition requires empty which-value";
		MSG(&cTok, msg.str().c_str(), ERR);
		delete rightNode;
		delete caseCond;
		return NULL; //stop this maddness (t41049)
	      }

	    return rightNode; //t41046
	  } //done

	if(swvalueIdent != NULL)
	  {
	    NodeIdent * leftNodeCopy = new NodeIdent(*swvalueIdent);
	    assert(leftNodeCopy);
	    leftNodeCopy->resetNodeNo(m_state.getNextNodeNo()); //unique invariant
	    //setup symbol now since all alike
	    SymbolVariable * symptr = NULL;
	    AssertBool gotSym = swvalueIdent->getSymbolPtr(symptr);
	    assert(gotSym);
	    leftNodeCopy->setSymbolPtr(symptr);

	    casecondition = new NodeBinaryOpCompareEqualEqual(leftNodeCopy, rightNode, m_state);
	    assert(casecondition);
	    casecondition->setNodeLocation(cTok.m_locator);
	  }
	else
	  casecondition = rightNode; //if true

      }
    else //default:
      {
	if(defaultcase == NULL)
	  {
	    casecondition = new NodeTerminal((u64) 1, Bool, m_state);
	    assert(casecondition);
	    casecondition->setNodeLocation(cTok.m_locator);
	    defaultcase = casecondition;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Multiple 'otherwise' labels in one which-control statement; first one on line ";
	    msg << defaultcase->getNodeLocation().getLineNo();
	    MSG(&cTok, msg.str().c_str(), ERR);
	    delete caseCond;
	    return NULL; //stop this maddness (e.g. t41022)
	  }
      }

    Token tmpTok; //non quietly, t41295
    if(!getExpectedToken(TOK_COLON, tmpTok))
      {
	delete casecondition;
	delete caseCond;
	return NULL; //stop this maddness
      }

    //multi-cases (t41020), t41580 (skip defaultcase's if(true))
    if((caseCond != NULL) && (casecondition != defaultcase))
      {
	NodeBinaryOpLogicalOr * newcaseCond = new NodeBinaryOpLogicalOr(caseCond, casecondition, m_state);
	assert(newcaseCond);
	newcaseCond->setNodeLocation(cTok.m_locator);
	casecondition = newcaseCond;
      }
    return parseRestOfCase(swvalueIdent, casecondition, defaultcase); //recurse
  } //parseRestOfCase

  Node * Parser::parseConditionalExpr()
  {
    return parseAssignExpr();
  } //parseConditionalExpr

  NodeBlock * Parser::setupAsConditionalBlockAndParseStatements(NodeConditional * asNode)
  {
    assert(m_state.m_parsingConditionalAs);
    assert(asNode);
    if(!asNode->asConditionalNode())
      {
	std::ostringstream msg;
	msg << "As-Conditional: " << asNode->prettyNodeName().c_str();
	msg << " is not a valid form of '(ident as Type)'; Operation deleted";
	MSG(asNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	m_state.m_parsingConditionalAs = false;
	return NULL; //asNode deleted by caller (t3406)
      }

    //requires an open brace to make the block
    bool singleStmtExpected = false;
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type != TOK_OPEN_CURLY)
      {
	//single statement situation; create a "block" anyway
	unreadToken();
	singleStmtExpected = true;
      }

    //if empty still make auto
    NodeBlock * currBlock = m_state.getCurrentBlock();
    NodeBlock * blockNode = new NodeBlock(currBlock, m_state);
    assert(blockNode);
    blockNode->setNodeLocation(asNode->getNodeLocation());

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    m_state.pushCurrentBlock(blockNode);

    //after the new block is setup: install the auto symbol into ST, and
    //make its auto local variable to shadow the lhs of 'as' as rhs type
    NodeIdent * tmpni = new NodeIdent(m_state.m_identTokenForConditionalAs, NULL, m_state);
    assert(tmpni);

    UTI ruti = asNode->getRightType(); //what if Self? (error/t3406); holder (t3831)
    assert(m_state.okUTItoContinue(ruti));
    u32 asrid = asNode->getRightTypeNameId();
    Token typeTok(TOK_TYPE_IDENTIFIER, asNode->getNodeLocation(), asrid);
    UTI tuti = ruti;
    UlamType * tut = m_state.getUlamTypeByIndex(tuti);

    TypeArgs typeargs;
    typeargs.init(typeTok);
    typeargs.m_bitsize = tut->getBitSize();
    typeargs.m_arraysize = tut->getArraySize();
    typeargs.m_classInstanceIdx = tuti; //?
    typeargs.m_declRef = ALT_AS;
    typeargs.m_referencedUTI = ruti;

    Symbol * asymptr = NULL; //a place to put the new symbol; not a decl list, nor typedef from another class
    tmpni->installSymbolVariable(typeargs, asymptr);
    assert(asymptr);

    delete tmpni; //done with nti
    tmpni = NULL;
    m_state.m_parsingConditionalAs = false; //done with flag and identToken.

    NodeTypeDescriptor * typeNode = new NodeTypeDescriptor(typeargs.m_typeTok, tuti, m_state, ALT_AS, ruti);
    assert(typeNode);

    //insert var decl into NodeStatements..as if parseStatement was called..
    NodeVarRefAs * varNode = new NodeVarRefAs((SymbolVariable*) asymptr, typeNode, m_state);
    assert(varNode);
    varNode->setNodeLocation(asNode->getNodeLocation());

    blockNode->appendNextNode(varNode);

    if(!singleStmtExpected)
      {
	if(!getExpectedToken(TOK_CLOSE_CURLY))
	  {
	    parseStatements(); //more statements, appended
	    Token tmpTok; //non quietly, like t41293
	    getExpectedToken(TOK_CLOSE_CURLY, tmpTok);
	  }
	//else
      }
    else
      {
	Node * sNode = parseStatement(); //get one statement only
	if(sNode)
	  blockNode->appendNextNode(sNode);
	//else error msg? (or a decl alreadyappended)
      }

    m_state.popClassContext(); //= prevBlock;
    return blockNode;
  } //setupAsConditionalBlockAndParseStatements

  bool Parser::parseSimpleStatement()
  {
    bool brtn = false;
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_SEMICOLON)
      {
	unreadToken();
	NodeStatementEmpty * emptyNode = new NodeStatementEmpty(m_state); //empty statement
	assert(emptyNode);
	emptyNode->setNodeLocation(pTok.m_locator);
	m_state.appendNodeToCurrentBlock(emptyNode);
	brtn = true;
      }
    else if(Token::isTokenAType(pTok))
      {
	unreadToken();
	brtn = parseDecl(); //updates symbol table & parse tree
      }
    else if(pTok.m_type == TOK_KW_LOCALDEF)
      {
	unreadToken();
	brtn = parseDecl(); //updates symbol table & parse tree (t3857)
      }
    else if(pTok.m_type == TOK_KW_TYPEDEF)
      {
	brtn = parseTypedef();
      }
    else if(pTok.m_type == TOK_KW_CONSTDEF)
      {
	brtn = parseConstdef();
      }
    else if(pTok.m_type == TOK_KW_RETURN)
      {
	unreadToken(); //needs location
	brtn = parseReturn();
      }
    else if(pTok.m_type == TOK_KW_BREAK)
      {
	if(m_state.m_parsingControlLoopsSwitchStack.okToParseABreak())
	  {
	    u32 bexitnum = m_state.m_parsingControlLoopsSwitchStack.getNearestBreakExitNumber();
	    NodeBreakStatement * brkNode = new NodeBreakStatement(bexitnum, m_state);
	    assert(brkNode);
	    brkNode->setNodeLocation(pTok.m_locator);
	    m_state.appendNodeToCurrentBlock(brkNode);
	    brtn = true;
	  }
	else
	  {
	    MSG(&pTok,"Break statement not within loop or which-control" , ERR);
	  }
      }
    else if(pTok.m_type == TOK_KW_CONTINUE)
      {
	u32 cexitnum = m_state.m_parsingControlLoopsSwitchStack.getNearestContinueExitNumber();
	if(cexitnum != 0)
	  {
	    NodeContinueStatement * contNode = new NodeContinueStatement(cexitnum, m_state);
	    assert(contNode);
	    contNode->setNodeLocation(pTok.m_locator);
	    m_state.appendNodeToCurrentBlock(contNode);
	    brtn = true;
	  }
	else
	  MSG(&pTok,"Continue statement not within loop" , ERR);
      }
    else if(pTok.m_type == TOK_KW_FLAG_VIRTUALOVERRIDE)
      {
	//eat error token, t41098
	std::ostringstream msg;
	msg << "Statement starts with flag keyword '";
	msg << m_state.getTokenDataAsString(pTok).c_str() << "'";
	MSG(&pTok, msg.str().c_str(), ERR);
      }
    else if(pTok.m_type == TOK_ERROR_LOWLEVEL)
      {
	//eat error token
      }
    else
      {
	unreadToken();
	Node * expNode = parseAssignExpr();
	if(expNode)
	  {
	    NodeSimpleStatement * simpleNode = new NodeSimpleStatement(expNode,m_state);
	    assert(simpleNode);
	    simpleNode->setNodeLocation(expNode->getNodeLocation());
	    m_state.appendNodeToCurrentBlock(simpleNode);
	    brtn = true;
	  }
      }

    if(!getExpectedToken(TOK_SEMICOLON, pTok, QUIETLY))
      {
	MSG(&pTok, "Invalid Statement (possible missing semicolon)", ERR);
	getTokensUntil(TOK_SEMICOLON);
      }
    return brtn;
  } //parseSimpleStatement

  //Typedefs are not transferred to generated code;
  //they are a short-hand for ulamtypes (e.g. arrays)
  //that may be used as function return types; scope-specific.
  bool Parser::parseTypedef()
  {
    bool brtn = false;
    Token pTok;
    getNextToken(pTok);

    if(Token::isTokenAType(pTok) || (pTok.m_type == TOK_KW_LOCALDEF))
      {
	unreadToken();
	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, false, false);

	if(typeNode == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Invalid typedef base Type";
	    MSG(&pTok, msg.str().c_str(), ERR); //t3866, t3861, t3862
	  }

	Token iTok;
	getNextToken(iTok);

	//insure the typedef name starts with a capital letter
	if(iTok.m_type == TOK_TYPE_IDENTIFIER)
	  {
	    if(iTok.m_dataindex == m_state.getUlamTypeNameIdByIndex(m_state.getThisClassForParsing()))
	      {
		std::ostringstream msg;
		msg << "Typedef Alias <" << m_state.getTokenDataAsString(iTok).c_str();
		msg << ">, has same name as the class it belongs";
		MSG(&iTok, msg.str().c_str(), ERR);
		delete typeNode;
		typeNode = NULL;
		brtn = false;
	      }
	    else
	      {
		NodeTypedef * rtnNode = makeTypedefSymbol(typeargs, iTok, typeNode);
		if(rtnNode)
		  {
		    m_state.appendNodeToCurrentBlock(rtnNode);
		    brtn = true;
		  }
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid typedef Alias <" << m_state.getTokenDataAsString(iTok).c_str();
	    msg << ">, Type Identifier (2nd arg) ";
	    if((Token::getSpecialTokenWork(iTok.m_type) == TOKSP_TYPEKEYWORD))
	      msg << "must not be a reserved keyword";
	    else
	      msg << "requires capitalization";
	    MSG(&iTok, msg.str().c_str(), ERR);
	    delete typeNode;
	    typeNode = NULL;
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Invalid typedef Base Type <";
	msg << m_state.getTokenDataAsString(pTok).c_str() << ">";
	MSG(&pTok, msg.str().c_str(), ERR);
      }
    return brtn;
  } //parseTypedef

  bool Parser::parseConstdef()
  {
    bool brtn = false;
    Token pTok;
    getNextToken(pTok);

    if( (Token::isTokenAType(pTok) || (pTok.m_type == TOK_KW_LOCALDEF)) && (pTok.m_type != TOK_KW_TYPE_VOID))// && (pTok.m_type != TOK_KW_TYPE_ATOM)) t41483
      {
	unreadToken();
	TypeArgs typeargs;
	typeargs.m_hasConstantTypeModifier = true;

	NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, false, false);
	if(typeNode == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Invalid named constant Type '";
	    msg << m_state.getTokenDataAsString(pTok).c_str() << "'";
	    MSG(&pTok, msg.str().c_str(), ERR); //t3857?
	    getTokensUntil(TOK_SEMICOLON);
	    return false; //done
	  }

	typeargs.m_assignOK = true;
	typeargs.m_isStmt = true;

	Token iTok;
	getNextToken(iTok);
	//insure the constant def name starts with a lower case letter
	if(iTok.m_type == TOK_IDENTIFIER)
	  {
	    //Constant refs are more like Variable refs than constant defs;
	    // They don't have a constant value; Like Function Parameters (also variables).
	    if(typeargs.m_declRef == ALT_CONSTREF)
	      {
		NodeVarDecl * rtnNode = makeVariableSymbol(typeargs, iTok, typeNode);
		if(rtnNode)
		  {
		    parseRestOfDeclInitialization(typeargs, iTok, rtnNode);
		    m_state.appendNodeToCurrentBlock(rtnNode);
		    brtn = true;
		  }
	      }
	    else
	      {
		NodeConstantDef * constNode = makeConstdefSymbol(typeargs, iTok, typeNode);
		if(constNode)
		  {
		    m_state.appendNodeToCurrentBlock(constNode);
		    brtn = true;
		  }
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid constant definition Alias '";
	    msg << m_state.getTokenDataAsString(iTok).c_str();
	    msg << "', Constant Identifier (2nd arg) ";
	    if((Token::getSpecialTokenWork(iTok.m_type) == TOKSP_KEYWORD))
	      msg << "must not be a reserved keyword";
	    else
	      msg << "requires lower-case";
	    MSG(&iTok, msg.str().c_str(), ERR);
	    delete typeNode;
	    typeNode = NULL;
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Invalid constant definition Type '";
	msg << m_state.getTokenDataAsString(pTok).c_str() << "'";
	MSG(&pTok, msg.str().c_str(), ERR);
	getTokensUntil(TOK_SEMICOLON);
      }
    return brtn;
  } //parseConstdef

  //Named constants (constdefs) are not transferred to generated code;
  //they are a short-hand for scalar constant expressions (e.g. terminals),
  //that are not 'storeintoable'; scope-specific.
  //doubles as class parameter without keyword or assignment.
  NodeConstantDef * Parser::parseClassParameterConstdef(bool assignREQ)
  {
    NodeConstantDef * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    if( (Token::isTokenAType(pTok) || (pTok.m_type == TOK_KW_LOCALDEF)) && (pTok.m_type != TOK_KW_TYPE_VOID) && (pTok.m_type != TOK_KW_TYPE_ATOM))
      {
	unreadToken();
	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, false, false);
	if(typeNode == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Invalid class parameter Type '";
	    msg << m_state.getTokenDataAsString(pTok).c_str() << "'";
	    MSG(&pTok, msg.str().c_str(), ERR); //t3857?
	    Token tmpTok;
	    getNextToken(tmpTok); //by pass identTok only
	    return NULL; //done
	  }

	typeargs.m_assignOK = assignREQ;
	typeargs.m_isStmt = false;

	Token iTok;
	getNextToken(iTok);
	//insure the constant def name starts with a lower case letter
	if(iTok.m_type == TOK_IDENTIFIER)
	  {
	    rtnNode = makeConstdefSymbol(typeargs, iTok, typeNode);
	    //don't append a class parameter to parse tree; its scope is different
	    // (i.e. it can be shadowed by a class data member or other named constant)
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid class parameter constant definition Alias '";
	    msg << m_state.getTokenDataAsString(iTok).c_str();
	    msg << "'; identifer (2nd arg) requires lower-case";
	    MSG(&iTok, msg.str().c_str(), ERR);
	    delete typeNode;
	    typeNode = NULL;
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Invalid class parameter Type '";
	msg << m_state.getTokenDataAsString(pTok).c_str() << "'";
	MSG(&pTok, msg.str().c_str(), ERR);
	Token tmpTok;
	getNextToken(tmpTok); //by pass identTok only
      }
    return rtnNode;
  } //parseClassParameterConstdef

  bool Parser::parseModelParameter()
  {
    bool brtn = false;
    Token pTok;
    getNextToken(pTok);

    //permitted in only in elements
    if(m_state.getUlamTypeByIndex(m_state.getCompileThisIdx())->getUlamClassType() != UC_ELEMENT)
      {
	std::ostringstream msg;
	msg << "Model Parameters cannot survive within a quark";
	MSG(&pTok, msg.str().c_str(), ERR);
	getTokensUntil(TOK_SEMICOLON); //does this help?
	return false;
      }

    if((Token::isTokenAType(pTok) || (pTok.m_type == TOK_KW_LOCALDEF)) && (pTok.m_type != TOK_KW_TYPE_VOID) && (pTok.m_type != TOK_KW_TYPE_ATOM))
      {
	unreadToken();
	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, false, true);
	if(typeNode)
	  {
	    typeargs.m_assignOK = true;

	    Token iTok;
	    getNextToken(iTok);
	    if(iTok.m_type == TOK_IDENTIFIER)
	      {
		NodeModelParameterDef * modelNode = makeModelParameterSymbol(typeargs, iTok, typeNode);
		if(modelNode)
		  {
		    m_state.appendNodeToCurrentBlock(modelNode);
		    brtn = true;
		  }
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Invalid Model Parameter Name '";
		msg << m_state.getTokenDataAsString(iTok).c_str();
		msg << "', Parameter Identifier requires lower-case";
		MSG(&iTok, msg.str().c_str(), ERR);
		delete typeNode;
		typeNode = NULL;
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid Model Parameter Type '";
	    msg << m_state.getTokenDataAsString(pTok).c_str() << "'";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Invalid Model Parameter Type <";
	msg << m_state.getTokenDataAsString(pTok).c_str();
	msg << ">; Only primitive types beginning with an ";
	msg << "upper-case letter may be a Model Parameter";
	MSG(&pTok, msg.str().c_str(), ERR);
	unreadToken();
      }
    return brtn;
  } //parseModelParameter

  //used for includingloclocal function variables; or
  //'singledecl' function parameters; no longer for function defs.
  bool Parser::parseDecl()
  {
    bool brtn = true;
    TypeArgs typeargs;
    NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, false, false);

    Token iTok;
    getNextToken(iTok);

    if(typeNode == NULL)
      {
	std::ostringstream msg;
	msg << "Invalid Type for named variable";
	MSG(&iTok, msg.str().c_str(), ERR);
	unreadToken(); //for location
	return false;
      }

    if(iTok.m_type == TOK_IDENTIFIER)
      {
	if(typeargs.m_danglingDot)
	  {
	    std::ostringstream msg;
	    msg << "Unexpected dot before named variable '";
	    msg << m_state.getTokenDataAsString(iTok).c_str() << "'";
	    MSG(&iTok, msg.str().c_str(), ERR); //t41560
	    //unreadToken();
	    return false;
	  }

	UTI passuti = typeNode->givenUTI(); //before it may become an array
	NodeVarDecl * rtnNode = makeVariableSymbol(typeargs, iTok, typeNode);
	if(rtnNode)
	  {
	    parseRestOfDeclInitialization(typeargs, iTok, rtnNode);
	    m_state.appendNodeToCurrentBlock(rtnNode);
	    //for multi's of same type (rtnType), and/or its assignment
	    return parseRestOfDecls(typeargs, passuti);
	  }
	else //error msg?
	  brtn = false;
      }
    else
      {
	//user error!
	std::ostringstream msg;
	msg << "Name of variable '" << m_state.getTokenDataAsString(iTok).c_str();
	if((Token::getSpecialTokenWork(iTok.m_type) == TOKSP_KEYWORD))
	  msg << "': Identifier must not be a reserved keyword";
	else
	  msg << "': Identifier must begin with a lower-case letter";
	MSG(&iTok, msg.str().c_str(), ERR);
	unreadToken();
	delete typeNode;
	typeNode = NULL;
	brtn = false;
      }
    return brtn;
  } //parseDecl

  NodeTypeDescriptor * Parser::parseTypeDescriptorIncludingLocalsScope(TypeArgs& typeargs, bool isaclass, bool delAfterDotFails)
  {
    UTI dropCastUTI = Nouti;
    return parseTypeDescriptorIncludingLocalsScope(typeargs, dropCastUTI, isaclass, delAfterDotFails);
  }

  NodeTypeDescriptor * Parser::parseTypeDescriptorIncludingLocalsScope(TypeArgs& typeargs, UTI& castUTI, bool isaclass, bool delAfterDotFails)
  {
    //typeargs initialized later, first token re-read here
    Token pTok;
    getNextToken(pTok);

    NodeTypeDescriptor * typeNode = NULL;

    if(pTok.m_type == TOK_KW_LOCALDEF)
      {
	NodeBlockLocals * localsblock = m_state.makeLocalsScopeBlock(pTok.m_locator);
	assert(localsblock);
	UTI luti = localsblock->getNodeType();
	Token dTok;
	getNextToken(dTok);
	if(dTok.m_type == TOK_DOT)
	  {
	    Token nTok;
	    getNextToken(nTok);
	    if(Token::isTokenAType(nTok))
	      {
		typeargs.init(nTok);
		typeargs.m_classInstanceIdx = luti; //t41517
		m_state.pushClassContext(luti, localsblock, localsblock, false, NULL);

		typeNode = parseTypeDescriptor(typeargs, castUTI, isaclass, delAfterDotFails);

		NodeTypeDescriptor * localdot = new NodeTypeDescriptor(nTok, luti, m_state);
		assert(localdot);

		//link this selection to NodeTypeDescriptor;
		//keep typedef alias name here (i.e. pTok)
		NodeTypeDescriptorSelect * selNode = new NodeTypeDescriptorSelect(nTok, castUTI, localdot, m_state);
		delete typeNode;
		typeNode = selNode; //t41517

		m_state.popClassContext();
	      }
	    else
	      {
		unreadToken();

		std::ostringstream msg;
		msg << "Expected a Type after the keyword 'local' for filescope: ";
		msg << m_state.getPathFromLocator(pTok.m_locator).c_str();
		msg << "; used incorrectly";
		MSG(&pTok, msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Keyword 'local' for filescope: ";
	    msg << m_state.getPathFromLocator(pTok.m_locator).c_str();
	    msg << "; used incorrectly in this context";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
      }
    else if(pTok.m_type == TOK_KW_CONSTDEF)
      {
	//get next token..
	getNextToken(pTok);
	if(Token::isTokenAType(pTok))
	  {
	    typeargs.init(pTok);
	    assert(m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCPARAMETER);
	    typeargs.m_hasConstantTypeModifier = true;
	    typeNode = parseTypeDescriptor(typeargs, castUTI, isaclass, delAfterDotFails);
	  }
	else
	  {
	    unreadToken();

	    std::ostringstream msg;
	    msg << "Expected a Type after the keyword 'constant'; "; // for function parameter
	    msg << "got Token <";
	    msg << m_state.getTokenDataAsString(pTok).c_str();
	    msg << "> instead";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
      }
    else if(Token::isTokenAType(pTok))
      {
	//unreadToken();
	typeargs.init(pTok);
	typeNode = parseTypeDescriptor(typeargs, castUTI, isaclass, delAfterDotFails);
      }
    else
      unreadToken();

    return typeNode; //can be NULL
  } //parseTypeDescriptorIncludingLocalsScope (new helper)

  //original helper, excluding locals scope; defaults isaclass false, delafterdot false.
  NodeTypeDescriptor * Parser::parseTypeDescriptor(TypeArgs& typeargs, bool isaclass, bool delAfterDotFails)
  {
    Token pTok;
    getNextToken(pTok);

    if(!Token::isTokenAType(pTok))
      {
	//note: this means it can't be a typedef from another class;
	//might be a class constructor (t41077)
	unreadToken();
	return NULL;
      }

    typeargs.init(pTok); //initialize token (possibly again) here

    UTI dropCastUTI = Nouti;
    return parseTypeDescriptor(typeargs, dropCastUTI, isaclass, delAfterDotFails); //isaclassarg unknown
  }//parseTypeDescriptor (helper)

  NodeTypeDescriptor * Parser::parseTypeDescriptor(TypeArgs& typeargs, UTI& castUTI, bool isaclassarg, bool delAfterDotFails)
  {
    NodeTypeDescriptor * typeNode = NULL;
    Token pTok = typeargs.m_typeTok;
    ULAMTYPE etyp = m_state.getBaseTypeFromToken(pTok);
    bool maybeAClassType = (etyp == Class) || isaclassarg;
    bool assumeAClassType = (maybeAClassType || (etyp == Hzy) || (etyp == Holder));

    if(assumeAClassType)
      {
	//sneak peak at next tok for dot
	Token dTok;
	getNextToken(dTok);
	unreadToken();

	//t3796, .maxof/.minof/.sizeof isn't always a class. darn.
	maybeAClassType |= (dTok.m_type == TOK_DOT); //another clue for Hzy and Holder

	if(maybeAClassType && (etyp == Holder))
	  {
	    u32 tokid = m_state.getTokenDataAsStringId(pTok); //t41312,3
	    UTI huti = Nav;
	    UTI tmpscalar= Nav;
	    AssertBool isTDDefined = (m_state.getUlamTypeByTypedefNameInClassHierarchyThenLocalsScope(tokid, huti, tmpscalar) == TBOOL_TRUE);
	    assert(isTDDefined);
	    m_state.makeAnonymousClassFromHolder(huti, pTok.m_locator); //don't need cnsym here
	  }

	//what to do with the UTI? could be a declref type; both args are refs!
	UTI cuti = parseClassArguments(typeargs, maybeAClassType);
	//if(!m_state.okUTItoContinue(cuti)) return NULL; //t41616 (continue, for better error messages..)

	if(maybeAClassType)
	  {
	    if(m_state.isAltRefType(cuti)) //e.g. refofSelf, ref to array of classes
	      {
		typeargs.m_classInstanceIdx = m_state.getUlamTypeAsDeref(cuti);
		typeargs.m_declRef = m_state.getReferenceType(cuti); //was ALT_REF
		typeargs.m_referencedUTI = typeargs.m_classInstanceIdx;
	      }
	    else if(m_state.isScalar(cuti))
	      typeargs.m_classInstanceIdx = cuti;
	    else
	      typeargs.m_classInstanceIdx = m_state.getUlamTypeAsScalar(cuti); //eg typedef class array
	    castUTI = cuti; //in case a ref, try this???Mon May  2 10:45:32 2016
	  }
	else
	  {
	    if(m_state.isScalar(cuti))
	      typeargs.m_declListOrTypedefScalarType = cuti; //this is what we wanted..
	    //else arraytype???

	    //DEREF'd cuti?
	    castUTI = cuti; //unless a dot is next
	  }
	typeNode = new NodeTypeDescriptor(typeargs.m_typeTok, cuti, m_state, typeargs.m_declRef, typeargs.m_referencedUTI); //t3728
	assert(typeNode);
      }
    else
      {
	//check for Type bitsize specifier;
	typeargs.m_bitsize = 0; //default bitsize possible, if not specified
	NodeTypeBitsize * bitsizeNode = parseTypeBitsize(typeargs);

	UTI tuti = m_state.getUlamTypeFromToken(typeargs);
	if(m_state.isScalar(tuti))
	  typeargs.m_declListOrTypedefScalarType = tuti; //this is what we wanted..
	//else arraytype?
	castUTI = tuti;

	//bitsize is unknown, e.g. based on a Class.sizeof
	typeNode = new NodeTypeDescriptor(typeargs.m_typeTok, tuti, m_state);
	assert(typeNode);

	if(bitsizeNode)
	  typeNode->linkConstantExpressionBitsize(bitsizeNode); //tfr ownership
      }

    if(typeargs.m_forMemberSelect)
     return typeNode; //t41310

    Token dTok;
    getNextToken(dTok);
    if(dTok.m_type == TOK_DOT)
      {
	unreadToken();
	if(!parseTypeFromAnotherClassesTypedef(typeargs, typeNode))
	  {
	    if(delAfterDotFails)
	      {
		delete typeNode;
		typeNode = NULL;
	      }
	    else
	      {
		getNextToken(dTok); //?
	      }
	  }
	else
	  {
	    typeargs.m_forMemberSelect = true; //t41621 could be so??
	    castUTI = typeargs.m_anothertduti;
	    getNextToken(dTok);
	  }
      }
    else if(typeargs.m_forFactor && (dTok.m_type == TOK_IDENTIFIER))
      {
	// not a dot (t41551, t3113)
	std::ostringstream msg;
	msg << "Type incorrectly used as a " ;
	if(m_state.m_parsingVariableSymbolTypeFlag == STF_NEEDSATYPE)
	  msg << "factor (no dot)";
	else
	  msg << m_state.getParserSymbolTypeFlagAsString().c_str(); //t41306..
	msg << ": ";
	msg << m_state.getTokenDataAsString(typeargs.m_typeTok).c_str();
	msg << "; followed by unexpected identifier <";
	msg << m_state.getTokenDataAsString(dTok).c_str() << ">";
	MSG(&pTok, msg.str().c_str(), ERR);

	if(delAfterDotFails)
	  {
	    delete typeNode;
	    typeNode = NULL;
	  }
	//else fall thru
      }
    //else fall thru

    if(dTok.m_type == TOK_AMP)
      {
	if(typeargs.m_danglingDot)
	  {
	    std::ostringstream msg;
	    msg << "Unexpected reference token '" ;
	    msg << m_state.getTokenDataAsString(dTok).c_str();
	    msg << "' after a dot";
	    MSG(&dTok, msg.str().c_str(), ERR);

	    if(delAfterDotFails)
	      {
		delete typeNode;
		typeNode = NULL;
	      }
	  }

	if(typeNode)
	  {
	    typeargs.m_declRef = typeargs.m_hasConstantTypeModifier ? ALT_CONSTREF : ALT_REF; //a declared reference (was ALT_REF)
	    typeargs.m_referencedUTI = castUTI; //?
	    typeargs.m_assignOK = true; //required
	    typeargs.m_isStmt = true; //unless a func param

	    // change uti to reference key
	    UTI refuti = castUTI;
	    if(m_state.okUTItoContinue(castUTI) && !m_state.isHolder(castUTI)) //t41153
	      refuti = m_state.getUlamTypeAsRef(castUTI, typeargs.m_declRef); //t3692
	    assert(typeNode);
	    typeNode->setReferenceType(typeargs.m_declRef, castUTI, refuti);
	  }
      }
    else
      unreadToken();

    return typeNode;
  } //parseTypeDescriptor

  UTI Parser::parseClassArguments(TypeArgs& typeargs, bool& isaclass)
  {
    Token pTok;
    getNextToken(pTok);
    u32 tokid = m_state.getTokenDataAsStringId(typeargs.m_typeTok);

    if(tokid == m_state.m_pool.getIndexForDataString("Self"))
      {
	//this is a constructor
	unreadToken();
	isaclass = true; //reset

	if(typeargs.m_forMemberSelect)
	  return Hzy; //t41621 ?

	return m_state.getCompileThisIdx(); //or Void?
      }
    else if(tokid == m_state.m_pool.getIndexForDataString("Super"))
      {
	unreadToken();
	isaclass = true; //reset

	if(typeargs.m_forMemberSelect)
	  return Hzy; //t41616

	if(m_state.m_parsingVariableSymbolTypeFlag != STF_CLASSINHERITANCE)
	  {
	    UTI tduti = Nav;
	    UTI tdscalaruti = Nouti;
	    TBOOL isTypedef = m_state.getUlamTypeByTypedefName(tokid, tduti, tdscalaruti);
	    if(isTypedef == TBOOL_TRUE)
	      {
		return tduti; //t41312
	      }
	    else if(isTypedef == TBOOL_HAZY)
	      {
		m_state.abortNeedsATest();
		return Hzy; //new case
	      }
	    //else Nav
	  }
	//this is an error!
	return Nav; //t41341
      }
    else if(pTok.m_type != TOK_OPEN_PAREN)
      {
	//regular class, not a template OR Self constructor
	unreadToken();

	//not a stub, could be member or localscope typedef shadowing a class;
	// unless local specifier (wouldn't be here), we must wait for member to be parsed
	// or if we already generated a member typedef, then use it.
	if(m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSINHERITANCE)
	  {
	    UTI tduti = Nav;
	    UTI tdscalaruti = Nouti;
	    Symbol * tdsymptr = NULL;
	    if(m_state.getUlamTypeByTypedefName(tokid, tduti, tdscalaruti, tdsymptr) == TBOOL_TRUE)
	      {
		assert(tdsymptr);
		ALT tdalt = tdsymptr->getAutoLocalType();
		if((tdalt != ALT_NOT) && !m_state.isReference(tduti))
		  {
		    assert(m_state.isHolder(tduti));
		    typeargs.m_declRef = tdalt; //t3728
		    typeargs.m_referencedUTI = tduti;
		  }
		return tduti;
	      }

	    UTI huti = m_state.makeCulamGeneratedTypedefSymbolInCurrentContext(typeargs.m_typeTok, true); //not locals scope, isaclass. (t41444)
	    return huti;
	  }
	//else not inheritance..

	//if in locals filescope, only if we already have a typedef should we not generate one.
	if(m_state.isThisLocalsFileScope())
	  {
	    UTI huti = m_state.makeCulamGeneratedTypedefSymbolInCurrentContext(typeargs.m_typeTok, isaclass);
	    typeargs.m_anothertduti = huti; //t41517
	    return huti; //locals scope (t3652)
	  }
	//else not locals filescope

	//here, not specified localsscope (though one could still come), nor inheritance def:
	// a typedef, here or baseclass (still hazy), has precedence,
	// before a locals scope (perhaps not yet declared),
	// or before a 'global scope' class is used.
	UTI tduti = Nav;
	UTI tdscalaruti = Nouti;
	Symbol * tdsymptr = NULL;
	TBOOL isTypedef = m_state.getUlamTypeByTypedefName(tokid, tduti, tdscalaruti, tdsymptr);
	if(isTypedef == TBOOL_TRUE)
	  {
	    assert(tdsymptr);
	    ALT tdalt = tdsymptr->getAutoLocalType();
	    if((tdalt != ALT_NOT) && !m_state.isReference(tduti))
	      {
		assert(m_state.isHolder(tduti));
		typeargs.m_declRef = tdalt; //t3728
	      }

	    ULAMTYPE bUT = m_state.getUlamTypeByIndex(tduti)->getUlamTypeEnum();
	    isaclass |= (bUT == Class); //or Hzy or Holder?

	    if(isaclass && (bUT == Holder)) //not isHolder(tduti)
	      {
		m_state.makeAnonymousClassFromHolder(tduti, typeargs.m_typeTok.m_locator); //t3862
		//m_state.addUnseenClassId(tokid); //ulamexports?
	      }
	    return tduti; //done. (could be an array; or refselftype)
	  }
	else //hazy or false in current class scope (t3102, etc..)
	  {
	    //generate a class member typedef, class scope (even if tokid classname exists already)
	    UTI huti = m_state.makeCulamGeneratedTypedefSymbolInCurrentContext(typeargs.m_typeTok); //isaclass? no.
	    return huti;
	  }
	m_state.abortShouldntGetHere();
      } //not open paren

    //must be a template class
    UTI cuti = m_state.getCompileThisIdx();
    SymbolClassNameTemplate * ctsym = NULL;
    if(!m_state.alreadyDefinedSymbolClassNameTemplate(tokid, ctsym))
      {
	if(ctsym == NULL) //was undefined, template; will fix instances' argument names later
	  m_state.addIncompleteTemplateClassSymbolToProgramTable(typeargs.m_typeTok, ctsym);
	else
	  {
	    //error have a class without parameters already defined (error output)
	    getTokensUntil(TOK_CLOSE_PAREN); //rest of statement is ignored.
	    return Nav; //short-circuit
	  }
      }
    assert(ctsym);

    //handle possible 2nd sighting of an unseen template class (t41166)
    UTI ctuti = ctsym->getUlamTypeIdx();
    UlamType * ctut = m_state.getUlamTypeByIndex(ctuti);
    bool unseenTemplate = (ctut->getUlamClassType() == UC_UNSEEN);

    u32 numParams = ctsym->getNumberOfParameters();
    u32 numParamDefaults = unseenTemplate ? 0 : ctsym->getTotalParametersWithDefaultValues();

    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      {
	if((numParams > 0) && (numParamDefaults != numParams))
	  {
	    //params but no args
	    std::ostringstream msg;
	    msg << "NO Class Arguments for an instance stub of class template '";
	    msg << m_state.m_pool.getDataAsString(ctsym->getId()).c_str();
	    msg << "' with " << numParams << " parameters";
	    MSG(&pTok, msg.str().c_str(), ERR);
	    return Nav; //ok to return
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "No Class Arguments for an instance stub of class template '";
	    msg << m_state.m_pool.getDataAsString(ctsym->getId()).c_str();
	    msg << "' with " << numParams << " parameters";
	    msg << "; using " << numParamDefaults << " default values";
	    MSG(&pTok, msg.str().c_str(), DEBUG);
	  }
      }

    unreadToken(); //not close paren yet; unless using defaults (u.1.2.1)

    //make a (shallow) Class Instance Stub to collect class args as SymbolConstantValues;
    //has its own uti that will become part of its key; (too soon for a deep copy!)
    UTI stubuti = m_state.makeUlamType(typeargs.m_typeTok, UNKNOWNSIZE, NONARRAYSIZE, Nouti, ALT_NOT, ctsym->getUlamClass()); //overwrites the template type here; possibly UC_UNSEEN

    if(ctut->isCustomArray())
      {
	UlamType * stubut = m_state.getUlamTypeByIndex(stubuti);
	((UlamTypeClass *) stubut)->setCustomArray();
      }

    SymbolClass * stubcsym = ctsym->makeAStubClassInstance(typeargs.m_typeTok, stubuti);
    if(stubcsym == NULL)
      {
	std::ostringstream msg;
	msg << "While parsing a ";
	msg << m_state.getParserSymbolTypeFlagAsString().c_str();
	msg << " for ";
	msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str() ;
	msg << ": due to unrecoverable problems in template: ";
	msg << m_state.m_pool.getDataAsString(ctsym->getId()).c_str() ;
	msg << ", additional errors are unlikely to be useful";
	MSG(&typeargs.m_typeTok, msg.str().c_str(), ERR);
	return Nav; //needs a test, was t41166
      }

    //note: class resolver cnstr initializes value/type contexts to: cuti/stubuti
    //assert(stubcsym->getContextForPendingArgValues() == Nouti);
    //NOOP! always...t3328,t3332,6,t3497,t3886,7,8,t3890,1,2,t41223,1,8
    stubcsym->setContextForPendingArgValues(cuti);
    // to support dependent parameter class stubs, use typeargs to pass along
    // the outermost stub type in case of recursion (e.g. t41214, error/t41210)
    if(typeargs.m_classInstanceIdx != Nouti)
      {
	stubcsym->setContextForPendingArgTypes(typeargs.m_classInstanceIdx);
      }
    else if(m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSPARAMETER)
      {
	stubcsym->setContextForPendingArgTypes(stubuti);
	typeargs.m_classInstanceIdx = cuti;
	m_state.setStubFlagForThisClassTemplate(stubuti); //applies to stubs only, t41224
      }
    else if(m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSINHERITANCE)
      {
	stubcsym->setContextForPendingArgTypes(stubuti); //t41223, t41221, t41224, t41226
	typeargs.m_classInstanceIdx = stubuti;
      }
    else
      {
	stubcsym->setContextForPendingArgTypes(stubuti);
	typeargs.m_classInstanceIdx = stubuti;
      }

    u32 parmidx = 0;
    parseRestOfClassArguments(stubcsym, ctsym, parmidx, typeargs);

    bool ctUnseen = (ctsym->getUlamClass() == UC_UNSEEN);
    if(!ctUnseen && (parmidx < ctsym->getNumberOfParameters()))
      {
	if(!ctsym->parameterHasDefaultValue(parmidx))
	  {
	    std::ostringstream msg;
	    msg << "Too few Class Arguments parsed, ";
	    msg << "(" << parmidx << "), for template: ";
	    msg << m_state.m_pool.getDataAsString(ctsym->getId()).c_str() ;
	    msg << ", by " << m_state.getUlamTypeNameBriefByIndex(stubuti).c_str() ;
	    MSG(&typeargs.m_typeTok, msg.str().c_str(), ERR);
	    stubuti = Nav;
	  }
	else
	  {
	    //(similar to fixAnyUnseenClassInstances for unseen templates)
	    //copy the default parameter symbols and nodeconstantdef's as pending args (t3526)
	    ctsym->fixAClassStubsDefaultArgs(stubcsym, parmidx);
	  }
      }
    if(stubuti != Nav)
      isaclass = true; //we know now for sure
    return stubuti;
  } //parseClassArguments

  void Parser::parseRestOfClassArguments(SymbolClass * csym, SymbolClassNameTemplate * ctsym, u32& parmIdx, TypeArgs & typeargs)
  {
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      return;

    unreadToken();

    Node * exprNode = parseExpression(); //constant expression req'd

    assert(csym);
    if(exprNode == NULL)
      {
	std::ostringstream msg;
	msg << "Class Argument after Open Paren is missing, for template '";
	msg << m_state.m_pool.getDataAsString(csym->getId()).c_str() << "'";
	MSG(&pTok, msg.str().c_str(), ERR);
	// what else is missing?
	return;
      }

    assert(ctsym);
    //this is possible if cnsym is UC_UNSEEN, must check args later..
    bool ctUnseen = (ctsym->getUlamClass() == UC_UNSEEN);
    if(!ctUnseen && parmIdx >= ctsym->getNumberOfParameters())
      {
	std::ostringstream msg;
	msg << "Too many Class Arguments " << "(" << parmIdx + 1 << "); Class Template '";
	msg << m_state.m_pool.getDataAsString(ctsym->getId()).c_str();
	msg << "' expects " << ctsym->getNumberOfParameters();
	MSG(&pTok, msg.str().c_str(), ERR);
	delete exprNode;
	exprNode = NULL;
      }
    else
      {
	//assert(m_state.m_parsingVariableSymbolTypeFlag != STF_CLASSPARAMETER); (e.g. t3525, 3886,..)
	//try to continue..
	UTI cuti = csym->getUlamTypeIdx();
	NodeBlockClass * cblock = csym->getClassBlockNode();
	m_state.pushCurrentBlock(cblock); //reset here for new arg's ST; no change to compilethisidx

	SymbolConstantValue * argSym = NULL;
	if(!ctUnseen)
	  {
	    NodeBlockClass * templateclassNode = ctsym->getClassBlockNode();
	    assert(templateclassNode);
	    NodeConstantDef * paramConstDef = (NodeConstantDef *) templateclassNode->getParameterNode(parmIdx);
	    assert(paramConstDef);
	    u32 pid = paramConstDef->getSymbolId();
	    UTI puti = paramConstDef->getTypeDescriptorGivenType();

	    Token argTok(TOK_IDENTIFIER, pTok.m_locator, pid); //use current locator
	    UTI auti = m_state.mapIncompleteUTIForAClassInstance(cuti, puti, pTok.m_locator);

	    //like param, not clone t3526, t3862, t3615, t3892; error msg loc (error/t3893)
	    argSym = new SymbolConstantValue(argTok, auti, m_state);
	    if(m_state.isHolder(auti))
	      {
		//does auti get added to this class, or its template?
		//t41216, also in fixAnyUnseen for templates seen afterwards;
		if(m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSPARAMETER)
		  m_state.addUnknownTypeTokenToThisClassResolver(argTok, auti);
		//else t41220 FUNCLOCALVAR v
	      }
	    if(m_state.isAClass(auti) && m_state.isClassAStub(auti))
	      {
		//reset its context for pending arg types to csym uti (t41214, error/t41210)
		SymbolClass * argcsym = NULL;
		AssertBool isDef = m_state.alreadyDefinedSymbolClass(auti, argcsym);
		assert(isDef);

		//CONUNDRUM!! the goal?
		argcsym->setContextForPendingArgTypes(cuti); //t41223, t41224
		argcsym->setContextForPendingArgValues(cuti);
	      }
	  }
	else
	  {
	    std::ostringstream sname;
	    sname << "_" << parmIdx;
	    u32 snameid = m_state.m_pool.getIndexForDataString(sname.str());
	    Token argTok(TOK_IDENTIFIER, pTok.m_locator, snameid); //use current locator
	    argSym = new SymbolConstantValue(argTok, Hzy, m_state); //fixed after template is seen
	  }

	assert(argSym);

	if( m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSPARAMETER)
	  argSym->setClassParameterFlag(); //mutally exclusive to class argument
	else
	  argSym->setClassArgumentFlag(cuti); //t41229

	//scope updated to new class instance in parseClassArguments (t3326,t3328?)
	u32 argid = argSym->getId();
	Symbol * oldArgSym = NULL;
	if(m_state.isIdInCurrentScope(argid, oldArgSym))
	  {
	    //assert(oldArgSym->getUlamTypeIdx()==Hzy); //sanity check..before.. ????
	    UTI olduti = oldArgSym->getUlamTypeIdx();
	    UTI goinguti = argSym->getUlamTypeIdx();
	    if(olduti == Hzy)
	      oldArgSym->resetUlamType(argSym->getUlamTypeIdx()); //replacing Hzy ????
	    else if(olduti != goinguti)
	      {
		UTI oldutia = m_state.lookupUTIAlias(olduti);
		UTI goingutia = m_state.lookupUTIAlias(goinguti);
		if((oldutia == olduti) && (goingutia == goinguti))
		  m_state.updateUTIAliasForced(goinguti,olduti); //t41455?
	      }
	    //patched in DataMembers when instance stub made
	    delete argSym;
	    assert(oldArgSym->isConstant());
	    argSym = (SymbolConstantValue *) oldArgSym;
	  }
	else
	  m_state.addSymbolToCurrentScope(argSym);

	//make Node with argument symbol wo trying to fold const expr;
	// add to list of unresolved for this uti
	// NULL node type descriptor, no token, yet known uti
	NodeConstantDef * constNode = new NodeConstantDef(argSym, NULL, m_state);
	assert(constNode);
	constNode->setNodeLocation(pTok.m_locator);
	constNode->setConstantExpr(exprNode);

	//clone the seen template's node type descriptor for this stub's pending argument;
	// including classes that might be holders (e.g. t41220)
	if(!ctUnseen)
	  {
	    NodeBlockClass * templateblock = ctsym->getClassBlockNode();
	    NodeConstantDef * paramConstDef = (NodeConstantDef *) templateblock->getParameterNode(parmIdx);
	    assert(paramConstDef);
	    m_state.pushClassContext(cuti, cblock, cblock, false, NULL); //came from Parser parseRestOfClassArguments says null blocks likely (t41214, t3349)
	    constNode->cloneTypeDescriptorForPendingArgumentNode(paramConstDef);
	    m_state.popClassContext();
	  }

	//fold later; non ready expressions saved by UTI in m_nonreadyClassArgSubtrees (stub)
	csym->linkConstantExpressionForPendingArg(constNode); //also adds argnode to cblock
	m_state.popClassContext(); //restore after making NodeConstantDef, so current context is class
      } //too many args

    getNextToken(pTok);
    if(pTok.m_type != TOK_COMMA)
      unreadToken();
    else
      {
	//comma followed by close paren is a parse error
	Token qTok;
	getNextToken(qTok);
	unreadToken();
	if(qTok.m_type == TOK_CLOSE_PAREN)
	  {
	    std::ostringstream msg;
	    msg << "Class Argument after Comma is missing, for template '";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str() << "'";
	    MSG(&qTok, msg.str().c_str(), ERR);
	  }
      }
    return parseRestOfClassArguments(csym, ctsym, ++parmIdx, typeargs); //recurse
  } //parseRestOfClassArguments

  //return NodeTypeBitsize when unsuccessful evaluating its constant expression
  //up to caller to delete it
  NodeTypeBitsize * Parser::parseTypeBitsize(TypeArgs& args)
  {
    NodeTypeBitsize * rtnNode = NULL;
    Token bTok;
    getNextToken(bTok);

    if(bTok.m_type == TOK_OPEN_PAREN)
      {
	Node * bitsizeNode = parseExpression(); //constant expression req'd
	if(bitsizeNode == NULL)
	  {
	    std::ostringstream msg;
	    msg << "Bit size after Open Paren is missing, type: ";
	    msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str();
	    if(args.m_typeTok.m_type == TOK_KW_TYPE_VOID)
	      msg << "; Void size is zero"; //t41105,t41370
	    MSG(&bTok, msg.str().c_str(), ERR);
	  }
	else
	  {
	    if(args.m_typeTok.m_type == TOK_KW_TYPE_VOID)
	      {
		std::ostringstream msg;
		msg << "Void bitsize expression disregarded; size is zero";
		MSG(&bTok, msg.str().c_str(), WARN);

		args.m_bitsize = 0;
		delete bitsizeNode;
		bitsizeNode = NULL;
	      }
	    else if(args.m_typeTok.m_type == TOK_KW_TYPE_STRING)
	      {
		std::ostringstream msg;
		msg << "String bitsize expression disregarded; size is " << STRINGIDXBITS;
		MSG(&bTok, msg.str().c_str(), WARN);

		args.m_bitsize = STRINGIDXBITS; //t3987
		delete bitsizeNode;
		bitsizeNode = NULL;
	      }
	    else
	      {
		rtnNode = new NodeTypeBitsize(bitsizeNode, m_state);
		assert(rtnNode);
		rtnNode->setNodeLocation(args.m_typeTok.m_locator);
		args.m_bitsize = UNKNOWNSIZE; //no eval yet
	      }
	  }

	Token tmpTok; //non quietly, like t41293
	if(!getExpectedToken(TOK_CLOSE_PAREN, tmpTok))
	  {
	    delete rtnNode;
	    rtnNode = NULL;
	  }
      }
    else
      {
	unreadToken(); //not open paren, bitsize is unspecified
      }

    return rtnNode; //typebitsize const expr
  } //parseTypeBitsize

  bool Parser::parseTypeFromAnotherClassesTypedef(TypeArgs& args, NodeTypeDescriptor *& rtnTypeDesc)
  {
    u32 numDots = 0;
    bool foundTypedef = true;
    parseTypeFromAnotherClassesTypedef(args, foundTypedef, numDots, rtnTypeDesc);

    if(numDots > 1)
      return true; //definitely found a typedef

    return foundTypedef; //one time only
  } //parseTypeFromAnotherClassesTypedef (call)

  //recursively parses classtypes and their typedefs (dot separated)
  //into their UTI alias; the typeTok and return bitsize ref args
  //represent the UTI; false if 'sizeof' or something other than a type
  //was found after the first dot; countDots.
  void Parser::parseTypeFromAnotherClassesTypedef(TypeArgs& args, bool& rtnb, u32& numDots, NodeTypeDescriptor *& rtnTypeDesc)
  {
    Token nTok;
    getNextToken(nTok);
    if(nTok.m_type != TOK_DOT)
      {
	unreadToken();
	return; //done.
      }
    numDots++;

    SymbolClassName * cnsym = NULL;
    Symbol * asym = NULL; //or a typedef that we've already seen
    bool hazyKin = false; //don't care
    u32 tokid = m_state.getTokenDataAsStringId(args.m_typeTok); //t41312
    if(!m_state.alreadyDefinedSymbol(tokid, asym, hazyKin) && !m_state.alreadyDefinedSymbolClassName(tokid, cnsym)) //t3497
      {
	//if here, the last typedef might have been a holder for some unknown type
	// now we know (thanks to the dot and subsequent type token) that its a holder
	// for a class. but we don't know it's real name, yet!

	// we need to add it to our table as an anonymous classes for now
	// using its the string of its UTI as its temporary out-of-band name.

	// in the future, we will only be able to find it via its UTI, not a token.
	// the UTI should be the anothertduti (unless numdots is only 1).

	if(numDots > 1)//t41437 pTok is maxof, was '&& Token::isTokenAType(pTok))'
	  {
	    //make an 'anonymous class'
	    UTI aclassuti = args.m_anothertduti;
	    Token atok = args.m_typeTok;
	    cnsym = m_state.makeAnonymousClassFromHolder(aclassuti, atok.m_locator); //t3385
	    args.m_classInstanceIdx = aclassuti; //since we didn't know last time
	  }
	else
	  {
	    unreadToken(); //put dot back, minof or maxof perhaps?
	    numDots--; //update count here??

	    std::ostringstream msg;
	    msg << "Unexpected input!! Token <" << args.m_typeTok.getTokenEnumNameFromPool(&m_state).c_str();
	    msg << "> is not a 'seen' class type: <";
	    msg << m_state.getTokenDataAsString(args.m_typeTok).c_str() << ">";
	    MSG(&args.m_typeTok, msg.str().c_str(), DEBUG);
	    rtnb = false;
	    return;
	  }
      }
    else
      {
	if(asym && !cnsym)
	  {
	    if(asym->isTypedef())
	      {
		SymbolClass * tmpcsym = NULL;
		UTI acuti = asym->getUlamTypeIdx();

		if(m_state.isHolder(acuti) && !m_state.isAClass(acuti))
		  {
		    m_state.makeAnonymousClassFromHolder(acuti, args.m_typeTok.m_locator); //anonymous class (t41437)
		    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(acuti, tmpcsym);
		    assert(isDefined); //t3643
		  }
		else if(!m_state.alreadyDefinedSymbolClass(acuti, tmpcsym))
		  {
		    unreadToken(); //put dot back, minof or maxof perhaps?
		    std::ostringstream msg;
		    msg << "Unexpected input!! Token <" << args.m_typeTok.getTokenEnumNameFromPool(&m_state).c_str();
		    msg << "> is not a 'seen' class typedef <";
		    msg << m_state.getTokenDataAsString(args.m_typeTok).c_str() << ">";
		    MSG(&args.m_typeTok, msg.str().c_str(), DEBUG);
		    rtnb = false;
		    return;
		  }

		//else
		  {
		    u32 cid = tmpcsym->getId();
		    AssertBool isDefined = m_state.alreadyDefinedSymbolClassName(cid, cnsym);
		    assert(isDefined);
		    args.m_classInstanceIdx = acuti; //?
		  }
	      }
	  }
      }

    // either found a class or made one up, continue..
    assert(cnsym);
    SymbolClass * csym = NULL;
    if(cnsym->isClassTemplate() && (args.m_classInstanceIdx != Nouti))
      {
	if(args.m_classInstanceIdx == cnsym->getUlamTypeIdx())
	  csym = cnsym; //t41383
	else if(! ((SymbolClassNameTemplate *)cnsym)->findClassInstanceByUTI(args.m_classInstanceIdx, csym))
	  {
	    std::ostringstream msg;
	    msg << "Trying to use typedef from another class template '";
	    msg << m_state.m_pool.getDataAsString(cnsym->getId()).c_str();
	    msg << "', but instance cannot be found";
	    MSG(&args.m_typeTok, msg.str().c_str(), ERR);
	    numDots = 0;
	    rtnb = false;
	    getTokensUntil(TOK_SEMICOLON); //rest of statement is ignored.
	    return; //failed
	  }
      }
    else
      csym = cnsym; //regular class

    assert(csym);
    NodeBlockClass * memberClassBlock = csym->getClassBlockNode();
    if(memberClassBlock == NULL) //e.g. forgot the closing brace on quark def once; or UNSEEN
      {
	//hail mary pass..possibly a sizeof of unseen class
	getNextToken(nTok);
	if((nTok.m_type != TOK_KW_SIZEOF) && (nTok.m_type != TOK_KW_INSTANCEOF) && (nTok.m_type != TOK_KW_ATOMOF) && (nTok.m_type != TOK_KW_CLASSIDOF) && (nTok.m_type != TOK_KW_CONSTANTOF)&& (nTok.m_type != TOK_KW_POSOF))
	  {
	    std::ostringstream msg;
	    msg << "Trying to use typedef from another class '";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str();
	    msg << "', before it has been defined; Cannot continue with (token) ";
	    msg << m_state.getTokenDataAsString(nTok).c_str();
	    MSG(&args.m_typeTok, msg.str().c_str(), ERR);
	    getTokensUntil(TOK_SEMICOLON); //rest of statement is ignored.
	  }
	else
	  {
	    args.m_bitsize = UNKNOWNSIZE; //t.f. unknown bitsize or arraysize or both?
	    unreadToken(); //put the 'sizeof' back
	  }
	rtnb = false;
	return;
      }

    //set up compiler state to use the member class block for symbol searches
    m_state.pushClassContextUsingMemberClassBlock(memberClassBlock);

    args.m_danglingDot = true;
    Token pTok;
    //after the dot; this is when the dot is lost and becomes "optional" (see t41551)
    getNextToken(pTok);
    if(Token::isTokenAType(pTok))
      {
	UTI tduti;
	UTI tdscalaruti = Nav;
	bool isclasstd = false;
	u32 ptokid = m_state.getTokenDataAsStringId(pTok); //t41312

	if(m_state.getUlamTypeByTypedefName(ptokid, tduti, tdscalaruti) != TBOOL_TRUE)
	  {
	    //make one up!! if UN_SEEN class
	    UTI mcuti = memberClassBlock->getNodeType();
	    ULAMCLASSTYPE mclasstype = m_state.getUlamTypeByIndex(mcuti)->getUlamClassType();
	    if(mclasstype == UC_UNSEEN)
	      {
		bool isaclass = (pTok.m_type == TOK_KW_TYPE_SUPER);
		m_state.makeCulamGeneratedTypedefSymbolInCurrentContext(pTok, isaclass);
	      }
	  } //end make one up, now fall through

	TBOOL gotTypedef = m_state.getUlamTypeByTypedefName(ptokid, tduti, tdscalaruti);
	if(gotTypedef == TBOOL_TRUE)
	  {
	    UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
	    if(!tdut->isComplete())
	      {
		std::ostringstream msg;
		msg << "Incomplete type!! " << m_state.getUlamTypeNameByIndex(tduti).c_str();
		msg << " UTI" << tduti;
		msg << " found for Typedef <" << m_state.getTokenDataAsString(pTok).c_str();
		msg << ">, belonging to class: ";
		msg << m_state.m_pool.getDataAsString(csym->getId()).c_str();
		MSG(&pTok, msg.str().c_str(), DEBUG);
	      }
	    if(!m_state.okUTItoContinue(tduti) || m_state.isHolder(tduti))
	      {
		args.m_typeTok = pTok; //t3339, t41437
	      }
	    else
	      {
		ULAMCLASSTYPE tdclasstype = tdut->getUlamClassType();
		const std::string tdname = tdut->getUlamTypeNameOnly();

		//update token argument
		if(tdclasstype == UC_NOTACLASS)
		  args.m_typeTok.init(Token::getTokenTypeFromString(tdname.c_str()), pTok.m_locator, 0); //(not pTok.m_dataindex, e.g.t3380, t3381, t3382)
		else
		  {
		    args.m_typeTok.init(TOK_TYPE_IDENTIFIER, pTok.m_locator, m_state.m_pool.getIndexForDataString(tdname));
		    isclasstd = true;
		  }
	      }

	    //don't update rest of argument refs;
	    //typedefs have their own bit and array sizes to look up
	    //possibly another class? go again..
	    if(isclasstd)
	      {
		if(args.m_anothertduti != Nouti)
		  args.m_classInstanceIdx = args.m_anothertduti;
		else
		  args.m_classInstanceIdx = tduti;
	      }

	    args.m_anothertduti = tduti; //don't lose it!
	    args.m_declListOrTypedefScalarType = tdscalaruti;
	    args.m_danglingDot = false;
	    rtnb = true;

	    //link this selection to NodeTypeDescriptor;
	    //keep typedef alias name here (i.e. pTok)
	    NodeTypeDescriptorSelect * selNode = new NodeTypeDescriptorSelect(pTok, tduti, rtnTypeDesc, m_state);
	    rtnTypeDesc = selNode;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Unexpected input!! Token <";
	    msg << m_state.getTokenDataAsString(pTok).c_str();
	    msg << "> is not a typedef belonging to class: ";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str();
	    if(gotTypedef == TBOOL_HAZY)
	      {
		MSG(&pTok, msg.str().c_str(), DEBUG); //ish 20230116 or TBOOL to wait?
		m_state.abortNeedsATest();
	      }
	    else
	      {
		MSG(&pTok, msg.str().c_str(), ERR);
		rtnb = false;
	      }
	  }

	//possibly another class? go again..
	parseTypeFromAnotherClassesTypedef(args, rtnb, numDots, rtnTypeDesc);
      }
    else
      {
	args.m_bitsize = UNKNOWNSIZE; //t.f. unknown bitsize or arraysize or both?
	unreadToken(); //put the whatever came after the dot (e.g. 'sizeof', name constant) back
	rtnb = false;
      }

    m_state.popClassContext(); //restore
    return;
  } //parseTypeFromAnotherClassesTypedef

  Node * Parser::parseNamedConstantFromAnotherClass(const TypeArgs& args, NodeTypeDescriptor * typedescNode)
  {
    //when a lower case ident comes after a Type ident, it must be a
    //constant, constant array, or model parameter; could be from
    //another class; (t41146,47,48,49,50,51)
    Node * rtnNode = NULL;
    Token iTok;
    getNextToken(iTok);
    assert(iTok.m_type == TOK_IDENTIFIER);

    Token tmpTok;
    getNextToken(tmpTok);
    unreadToken();

    if(tmpTok.m_type == TOK_OPEN_PAREN)
      {
	u32 basetypeid = typedescNode->getTypeNameId();
	std::ostringstream msg;
	msg << "Suspect class type '";
	msg << m_state.m_pool.getDataAsString(basetypeid).c_str();
	msg << "' is followed by a function call to: ";
	msg << m_state.getTokenDataAsString(iTok).c_str();
	msg << ", and cannot be used in this context; an object is required";
	MSG(&tmpTok, msg.str().c_str(), ERR); //t41556
      }
    else
      {
	//fix during NodeConstant c&l
	rtnNode = new NodeConstant(iTok, NULL, typedescNode, m_state); //t41621?
	assert(rtnNode);
	rtnNode->setNodeLocation(iTok.m_locator);
      }
    return rtnNode;
  } //parseNamedConstantFromAnotherClass

  bool Parser::parseReturn()
  {
    Token pTok;
    getNextToken(pTok);

    NodeReturnStatement * returnNode = NULL;

    SYMBOLTYPEFLAG saveTheFlag = m_state.m_parsingVariableSymbolTypeFlag;
    m_state.m_parsingVariableSymbolTypeFlag = STF_RETURNSTMT; //t41635

    Node * rtnExprNode = parseAssignExpr(); //may be NULL
    if(rtnExprNode == NULL)
      {
	rtnExprNode = new NodeStatementEmpty(m_state); //has Nouti type
	assert(rtnExprNode);
	rtnExprNode->setNodeLocation(pTok.m_locator);
      }
    returnNode =  new NodeReturnStatement(rtnExprNode, m_state);
    assert(returnNode);
    returnNode->setNodeLocation(pTok.m_locator);

    m_state.appendNodeToCurrentBlock(returnNode);
    m_state.m_parsingVariableSymbolTypeFlag = saveTheFlag;

    return true;
  } //parseReturn

  Node * Parser::parseAssignExpr()
  {
    Node * rtnNode = NULL;
    rtnNode = parseExpression(); //perhaps a number, true or false, cast..

    if(rtnNode == NULL) return NULL;

    //if nothing else follows, parseRestOfAssignExpr returns its argument
    return parseRestOfAssignExpr(rtnNode);
  } //parseAssignExpr

  Node * Parser::parseLvalExpr(const Token& identTok)
  {
    //function calls and func defs are not valid; until constructor calls came along
    //if(getNextToken(pTok).m_type == TOK_OPEN_PAREN) no longer necessarily an error

    Symbol * asymptr = NULL;
    // may continue when symbol not defined yet (e.g. Decl)
    // don't return a NodeConstant, instead of NodeIdent, without arrays
    // even if already defined as one. lazy evaluate.
    // handle 'super' and 'self' as KEYWORDS instead of identifiers (ulam-5) t41337
    bool isDefined = m_state.isIdInCurrentScope(m_state.getTokenDataAsStringId(identTok), asymptr); //t3887, t3219, t41013, t41007, t3455
    bool makealocalsconstant = (!isDefined && (identTok.m_type == TOK_IDENTIFIER) && m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSINHERITANCE);

    if(makealocalsconstant)
      {
	bool locDefined = m_state.isIdInLocalFileScope(identTok.m_dataindex, asymptr);
	NodeBlockLocals * localsblock = m_state.getLocalsScopeBlock(m_state.getContextBlockLoc());
	if(!locDefined && (localsblock != NULL))
	  {
	    m_state.pushClassContext(localsblock->getNodeType(), localsblock, localsblock, false, NULL);

	    //make holder for this localdef constant not seen yet! (t3873)
	    UTI huti = m_state.makeUlamTypeHolder();
	    SymbolConstantValue * constsym = new SymbolConstantValue(identTok, huti, m_state);
	    m_state.addSymbolToCurrentScope(constsym);
	    asymptr = constsym;

	    m_state.popClassContext();
	  }
	//else (t41007)
      }

    //DM init with DM, clear symbol and lookup during c&l (t41698)
    if(isDefined && (m_state.m_parsingVariableSymbolTypeFlag == STF_DATAMEMBER) && asymptr->isDataMember())
      {
	asymptr= NULL; //t41698
      }

    //o.w. make a variable; symbol could be Null! a constant/array, or a model parameter!
    // fixed during c&l.
    Node * rtnNode = new NodeIdent(identTok, (SymbolVariable *) asymptr, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(identTok.m_locator);

    return parseRestOfLvalExpr(rtnNode); //in case of arrays
  } //parseLvalExpr

  //lvalExpr + func Calls + member select (dot)
  Node * Parser::parseIdentExpr(const Token& identTok)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    //function call, otherwise lvalExpr or member select
    if(pTok.m_type == TOK_OPEN_PAREN)
      {
	//function call, here
	rtnNode = parseFunctionCall(identTok);
      }
    else if(pTok.m_type == TOK_DOT)
      {
	unreadToken();
	//don't recurse..(t3189, t3450)
	rtnNode = new NodeIdent(identTok, NULL, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(identTok.m_locator);
      }
    else
      {
	//else we have a variable, not a function call, nor member_select
	unreadToken();
	rtnNode = parseLvalExpr(identTok);
      }
    return rtnNode;
  } //parseIdentExpr

  Node * Parser::parseRestOfMemberSelectExpr(Node * classInstanceNode)
  {
    Node * rtnNode = classInstanceNode; //first one
    Token pTok;

    TypeArgs typeargs;
    typeargs.m_forMemberSelect = true;

    bool hasOneBaseTypeSelector = false; //one is sufficient, all bases shared (ulam-5)

    //use loop rather than recursion to get a left-associated tree;
    // needed to support, for example: a.b.c.atomof (t3905)
    while(getExpectedToken(TOK_DOT, pTok, QUIETLY)) //if not, quietly unreads
      {
	Token iTok;
	getNextToken(iTok);
	if((iTok.m_type == TOK_IDENTIFIER) || (iTok.m_type == TOK_KW_SELF) || (iTok.m_type == TOK_KW_SUPER))
	  {
	    //set up compiler state to NOT use the current class block
	    //for symbol searches; may be unknown until type label
	    m_state.pushClassContextUsingMemberClassBlock(NULL); //oddly =true

	    Node * nextmember = parseIdentExpr(iTok); //includes array item, func call, etc.
	    if(nextmember == NULL)
	      {
		delete rtnNode; //t41078
		rtnNode = NULL;
		m_state.popClassContext(); //restore
		break;
	      }

	    NodeMemberSelect * ms = new NodeMemberSelect(rtnNode, nextmember, m_state);
	    assert(ms);
	    ms->setNodeLocation(iTok.m_locator);
	    rtnNode = ms;

	    //clear up compiler state to no longer use the member class block
	    // for symbol searches (e.g. t3262)
	    m_state.popClassContext(); //restore
	  }
	else if(Token::isTokenAType(iTok)) //t41314 m_a4 = self.Super.vfunca();
	  {
	    unreadToken();

	    if(hasOneBaseTypeSelector)
	      {
		std::ostringstream msg;
		msg << "Member Select By Base Type: ";
		msg << m_state.getTokenDataAsString(iTok).c_str();
		msg << " is redundant; All base classes are shared. Pick one!";
		MSG(&iTok, msg.str().c_str(), ERR);
	      }

	    ULAMTYPE etyp = m_state.getBaseTypeFromToken(iTok);
	    if(etyp != Class)
	      {
		std::ostringstream msg;
		msg << "Member Select followed by a Type: ";
		msg << m_state.getTokenDataAsString(iTok).c_str();
		msg << " must be a Class type";
		if((etyp == Hzy) || (etyp == Holder))
		  MSG(&iTok, msg.str().c_str(), DEBUG); //t41399,t41400
		else
		  {
		    MSG(&iTok, msg.str().c_str(), ERR);
		    delete rtnNode; //also deletes leftNode
		    rtnNode = NULL;
		    return rtnNode; //NULL (t41398)
		  }
	      } //else t41401

	    NodeTypeDescriptor * nextmembertypeNode = parseTypeDescriptor(typeargs, true); //isaclass
	    assert(nextmembertypeNode);

	    Node * vtrnNode = NULL;
	    Token vtrnTok;
	    getNextToken(vtrnTok);
	    if(vtrnTok.m_type == TOK_OPEN_SQUARE)
	      {
		if(typeargs.m_danglingDot)
		  {
		    std::ostringstream msg;
		    msg << "Unexpected '[' after a dot" ;
		    MSG(&vtrnTok, msg.str().c_str(), ERR);
		    delete rtnNode; //also deletes leftNode
		    rtnNode = NULL;
		    delete nextmembertypeNode;
		    nextmembertypeNode = NULL;
		    return NULL;
		  }

		vtrnNode = parseExpression();
		if(vtrnNode == NULL)
		  {
		    std::ostringstream msg;
		    msg << "Expecting ClassId for VTtable lookup";
		    MSG(&vtrnTok, msg.str().c_str(), ERR);
		    delete rtnNode; //also deletes leftNode
		    rtnNode = NULL;
		    delete nextmembertypeNode;
		    nextmembertypeNode = NULL;
		    return NULL;
		  }
		vtrnNode->setNodeLocation(vtrnTok.m_locator); //t41376

		Token eTok; //non quietly, t41294
		if(!getExpectedToken(TOK_CLOSE_SQUARE, eTok))
		  {
		    delete rtnNode; //also deletes leftNode
		    rtnNode = NULL;
		    delete nextmembertypeNode;
		    nextmembertypeNode = NULL;
		    delete vtrnNode;
		    vtrnNode = NULL;
		    return NULL;
		  }
	      }
	    else
	      unreadToken();

	    NodeMemberSelect * ms = new NodeMemberSelectByBaseClassType(rtnNode, nextmembertypeNode, vtrnNode, m_state);
	    assert(ms);
	    ms->setNodeLocation(iTok.m_locator);
	    rtnNode = ms;
	    //hasOneBaseTypeSelector = true; t41616?

	    Token tmpTok;
	    getNextToken(tmpTok);
	    unreadToken();
	    if((tmpTok.m_type != TOK_DOT) && !typeargs.m_danglingDot)
	      {
		std::ostringstream msg;
		msg << "Expected data member or function call to follow member select by type '";
		msg << m_state.getTokenDataAsString(iTok).c_str();
		msg << "'";
		MSG(&iTok, msg.str().c_str(), ERR); //t41356
	      }
	  }
	else
	  {
	    //end of dot chain
	    unreadToken(); //pTok
	    rtnNode = parseMinMaxSizeofType(rtnNode, Nouti, NULL); //ate dot, possible min/max/sizeof
	    if(rtnNode == NULL)
	      {
		delete classInstanceNode; //t41110 leak
		typeargs.m_danglingDot = true;
	      }
	  }
      } //while
    return rtnNode;
  } //parseRestOfMemberSelectExpr

  Node * Parser::parseMinMaxSizeofType(Node * memberNode, UTI utype, NodeTypeDescriptor * nodetype)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type != TOK_DOT)
      unreadToken(); //optional, dot might have already been eaten

    Token fTok;
    getNextToken(fTok);

    switch(fTok.m_type)
      {
      case TOK_KW_SIZEOF:
	rtnNode = new NodeTerminalProxy(memberNode, utype, fTok, nodetype, m_state);
	break;
      case TOK_KW_MAXOF:
	rtnNode = new NodeTerminalProxy(memberNode, utype, fTok, nodetype, m_state);
	break;
      case TOK_KW_MINOF:
	rtnNode = new NodeTerminalProxy(memberNode, utype, fTok, nodetype, m_state);
	break;
      case TOK_KW_CONSTANTOF:
	{
	  Token cTok;
	  getNextToken(cTok);
	  unreadToken();
	  if(cTok.m_type == TOK_OPEN_PAREN)
	    {
	      std::ostringstream msg;
	      msg << "Unsupported request: '" << m_state.getTokenDataAsString(fTok).c_str();
	      msg << "' constructor call";
	      MSG(&fTok, msg.str().c_str(), ERR); //t41508
	    }
	  else
	    {
	      rtnNode = new NodeConstantof(memberNode, nodetype, m_state);
	      rtnNode->setNodeLocation(fTok.m_locator);
	    }
	}
	break;
      case TOK_KW_INSTANCEOF:
	{
	Token cTok;
	getNextToken(cTok);
	if(cTok.m_type == TOK_OPEN_PAREN)
	  {
	    Node * cctrNode = makeInstanceofConstructorCall(fTok, memberNode, nodetype);
	    if(cctrNode == NULL)
	      {
		delete memberNode; //don't delete nodetype here, seqfault t41220
		memberNode = NULL;
		rtnNode = NULL;
		//error
	      }
	    else
	      rtnNode = cctrNode;
	  }
	else
	  {
	    unreadToken();
	    rtnNode = new NodeInstanceof(memberNode, nodetype, m_state);
	    rtnNode->setNodeLocation(fTok.m_locator);
	  }
	}
	break;
      case TOK_KW_ATOMOF:
	{
	  if(memberNode == NULL)
	    {
	      assert(nodetype);
	      //caller will clean up nodetype
	      std::ostringstream msg;
	      msg << "Unsupported request: '" << m_state.getTokenDataAsString(fTok).c_str();
	      msg << "' of Type <" << m_state.m_pool.getDataAsString(nodetype->getTypeNameId()).c_str() << ">";
	      MSG(&fTok, msg.str().c_str(), ERR); //t3680
	    }
	  else
	    {
	      //input uti wasn't complete, or an atom
	      rtnNode = new NodeAtomof(memberNode, nodetype, m_state);
	      rtnNode->setNodeLocation(fTok.m_locator);
	    }
	}
	break;
      case TOK_KW_LENGTHOF:
	rtnNode = new NodeTerminalProxy(memberNode, utype, fTok, nodetype, m_state);
	break;
      case TOK_KW_CLASSIDOF:
	rtnNode = new NodeTerminalProxy(memberNode, utype, fTok, nodetype, m_state);
	break;
      case TOK_KW_POSOF:
	rtnNode = new NodeTerminalProxy(memberNode, utype, fTok, nodetype, m_state);
	break;
      default:
	unreadToken();
      };
    return rtnNode; //may be null if not minof, maxof, sizeof, but a member or func selected
  } //parseMinMaxSizeofType

  Node * Parser::parseFunctionCall(const Token& identTok)
  {
    //fill in func symbol during type labeling; supports function overloading
    NodeFunctionCall * rtnNode = new NodeFunctionCall(identTok, NULL, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(identTok.m_locator);

    //member selection doesn't apply to arguments (during parsing too)
    NodeBlock * currBlock = m_state.getCurrentBlock(); //t41652,t41557
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    SYMBOLTYPEFLAG saveTheFlag = m_state.m_parsingVariableSymbolTypeFlag;
    m_state.m_parsingVariableSymbolTypeFlag = STF_FUNCARGUMENT;

    if(!parseRestOfFunctionCallArguments(rtnNode))
      {
	m_state.m_parsingVariableSymbolTypeFlag = saveTheFlag;
	m_state.popClassContext();
	delete rtnNode;
	return NULL; //stop this maddness
      }

    m_state.m_parsingVariableSymbolTypeFlag = saveTheFlag;
    m_state.popClassContext();

    //can't do any checking since function may not have been seen yet
    return rtnNode;
  } //parseFunctionCall

  bool Parser::parseRestOfFunctionCallArguments(NodeFunctionCall * callNode)
  {
    Node * nextArg = parseAssignExpr();
    if(nextArg)
      callNode->addArgument(nextArg);

    //if close paren we're done, o.w. comma, go again...(eat the token)
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      return true;

    if(pTok.m_type == TOK_COMMA)
      return parseRestOfFunctionCallArguments(callNode);

    std::ostringstream msg;
    msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(pTok).c_str() << ">";
    MSG(&pTok, msg.str().c_str(), ERR);
    return false;
  } //parseRestOfFunctionCallArguments

  Node * Parser::parseExpression()
  {
    Node * rtnNode = parseLogicalExpression();
    if(rtnNode == NULL) return NULL; //stop this maddness

    //if not addop, parseRestOfExpression returns its arg
    return parseRestOfExpression(rtnNode);
  } //parseExpression

  Node * Parser::parseLogicalExpression()
  {
    Node * rtnNode = parseBitExpression();
    if(rtnNode == NULL) return NULL; //stop this maddness

    //if not bitop, parseRestOfLogicalExpression returns its arg
    return parseRestOfLogicalExpression(rtnNode);
  } //parseLogicalExpression

  Node * Parser::parseBitExpression()
  {
    Node * rtnNode = parseEqExpression();
    if(rtnNode == NULL) return NULL; //stop this maddness

    //if not eqop, parseRestOfBitExpression returns its arg
    return parseRestOfBitExpression(rtnNode);
  } //parseBitExpression

  Node * Parser::parseEqExpression()
  {
    Node * rtnNode = parseCompareExpression();
    if(rtnNode == NULL) return NULL; //stop this maddness

    //if not compop, parseRestOfEqExpression returns its arg
    return parseRestOfEqExpression(rtnNode);
  } //parseEqExpression

  Node * Parser::parseCompareExpression()
  {
    Node * rtnNode = parseShiftExpression();
    if(rtnNode == NULL) return NULL; //stop this maddness

    //if not shiftop, parseRestOfCompareExpression returns its arg
    return parseRestOfCompareExpression(rtnNode);
  } //parseCompareExpression

  Node * Parser::parseShiftExpression()
  {
    Node * rtnNode = parseTerm();
    if(rtnNode == NULL) return NULL; //stop this maddness

    //if not addop, parseRestOfShiftExpression returns its arg
    return parseRestOfShiftExpression(rtnNode);
  } //parseShiftExpression

  Node * Parser::parseTerm()
  {
    Node * rtnNode = parseFactor();
    if(rtnNode == NULL) return NULL; //stop this maddness!

    //if not mulop, parseRestOfTerm returns rtnNode
    return parseRestOfTerm(rtnNode);
  } //parseTerm

  Node * Parser::parseFactor(bool localbase)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    //check for min/max/sizeof type, to make a constant terminal
    //only way to see a Type here! sizeof array when typedef.
    if(Token::isTokenAType(pTok))
      {
	unreadToken();
	bool allowrefcast = (m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCARGUMENT) || (m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCLOCALREF) || (m_state.m_parsingVariableSymbolTypeFlag == STF_RETURNSTMT); //not STF_FUNCLOCALCONSTREF t41255
	bool allowcasts = m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCLOCALCONSTREF ? false : true;
	return parseFactorStartingWithAType(pTok, allowrefcast, allowcasts); //rtnNode could be NULL
      } //not a Type

    //continue as normal..
    switch(pTok.m_type)
      {
      case TOK_IDENTIFIER:
      case TOK_KW_SELF:
      case TOK_KW_SUPER:
	{
	  m_state.saveIdentTokenForPendingConditionalAs(pTok); //always prepared for As-condition (t41337 super invalid as-cond)
	  rtnNode = parseIdentExpr(pTok);
	  //test ahead for UNOP_EXPRESSION so that any consecutive binary
	  //ops aren't misinterpreted as a unary operator (e.g. +,-).
	  rtnNode = parseRestOfFactor(rtnNode);
	}
	break;
      case TOK_NUMBER_SIGNED:
      case TOK_NUMBER_UNSIGNED:
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	rtnNode = new NodeTerminal(pTok, m_state);
	assert(rtnNode);
	break;
      case TOK_SQUOTED_STRING:
      case TOK_DQUOTED_STRING:
	rtnNode = new NodeTerminal(pTok, m_state);
	assert(rtnNode);
	rtnNode = parseRestOfFactor(rtnNode); //t3941
	break;
      case TOK_KW_FLAG_INSERTFUNC:
	{
	  if(m_state.m_parsingFUNCid == 0)
	    {
	      std::ostringstream msg;
	      msg << "Keyword <" << m_state.getTokenDataAsString(pTok).c_str();
	      msg << "> must be in a function";
	      MSG(&pTok, msg.str().c_str(), ERR); //t41369
	    }
	  else
	    {
	      assert(m_state.m_parsingFUNCid == m_state.getTokenDataAsStringId(m_state.m_parsingFuncDefToken));
	      std::string fname = m_state.getTokenDataAsString(m_state.m_parsingFuncDefToken);
	      u32 stringidx = m_state.formatAndGetIndexForDataUserString(fname);
	      rtnNode = new NodeTerminal((u64) stringidx, String, m_state);
	      assert(rtnNode);
	      rtnNode->setNodeLocation(pTok.m_locator);
	      rtnNode = parseRestOfFactor(rtnNode); //supports [] after (t41368)
	    }
	}
	break;
      case TOK_KW_FLAG_INSERTCLASS:
	{
	  u32 cid = m_state.getCompileThisId();
	  std::string cname = m_state.m_pool.getDataAsString(cid);
	  u32 stringidx = m_state.formatAndGetIndexForDataUserString(cname);
	  rtnNode = new NodeTerminal((u64) stringidx, String, m_state);
	  assert(rtnNode);
	  rtnNode->setNodeLocation(pTok.m_locator);
	  rtnNode = parseRestOfFactor(rtnNode); //supports [] after
	}
	break;
      case TOK_KW_FLAG_INSERTCLASSSIGNATURE:
      case TOK_KW_FLAG_INSERTCLASSNAMESIMPLE:
      case TOK_KW_FLAG_INSERTCLASSNAMEPRETTY:
      case TOK_KW_FLAG_INSERTCLASSNAMEMANGLED:
	{
	  UTI cuti = m_state.getCompileThisIdx();
	  rtnNode = new NodeTerminalProxy(NULL, cuti, pTok, NULL, m_state);
	  rtnNode = parseRestOfFactor(rtnNode); //supports [] after
	}
	break;
      case TOK_NUMBER_FLOAT:
	{
	  std::ostringstream msg;
	  msg << "Unsupported Number Type, Float <" << m_state.getTokenDataAsString(pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	}
	break;
      case TOK_OPEN_PAREN:
	{
	  bool allowrefcast = (m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCARGUMENT) || (m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCLOCALREF) || (m_state.m_parsingVariableSymbolTypeFlag == STF_RETURNSTMT); //t3862, t41067, t41635
	  bool allowcasts = m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCLOCALCONSTREF ? false : true;
	  rtnNode = parseRestOfCastOrExpression(allowrefcast, allowcasts);
	  rtnNode = parseRestOfFactor(rtnNode); //t41057, t41259
	}
	break;
      case TOK_MINUS:
      case TOK_PLUS:
      case TOK_BANG:
      case TOK_TWIDDLE:
      case TOK_PLUS_PLUS:
      case TOK_MINUS_MINUS:
	unreadToken();
	rtnNode = makeFactorNodePreUnaryOp(); //unary op
	break;
      case TOK_EOF:
      case TOK_CLOSE_CURLY:
      case TOK_SEMICOLON:
      case TOK_CLOSE_PAREN:
      case TOK_COMMA: //for functionall args
	unreadToken();
	break;
      case TOK_KW_LOCALDEF:
	{
	  if(localbase)
	    {
	      std::ostringstream msg;
	      msg << "Invalid local base of already local factor";
	      MSG(&pTok, msg.str().c_str(), ERR); //t3867
	    }
	  else
	    {
	      //makes 'locals' scope if used before defined
	      NodeBlockLocals * localsblock = m_state.makeLocalsScopeBlock(pTok.m_locator);
	      if(localsblock == NULL)
		{
		  std::ostringstream msg;
		  msg << "Keyword 'local' for filescope: ";
		  msg << m_state.getPathFromLocator(pTok.m_locator).c_str();
		  msg << "; has no defs";
		  MSG(&pTok, msg.str().c_str(), ERR);
		}
	      else
		{
		  Token dTok;
		  getNextToken(dTok);
		  if(dTok.m_type == TOK_DOT)
		    {
		      m_state.pushClassContext(localsblock->getNodeType(), localsblock, localsblock, false, NULL);

		      rtnNode = parseFactor(true); //recurse (t3861, t3862)

		      m_state.popClassContext();
		    }
		  else
		    {
		      std::ostringstream msg;
		      msg << "Keyword 'local' for filescope: ";
		      msg << m_state.getPathFromLocator(pTok.m_locator).c_str();
		      msg << "; used incorrectly as a factor (no dot)";
		      MSG(&pTok, msg.str().c_str(), ERR);
		    }
		}
	    }
	}
	break;
      case TOK_ERROR_LOWLEVEL:
	{
	  std::ostringstream msg;
	  msg << m_state.getTokenDataAsString(pTok).c_str();
	  MSG(&pTok, msg.str().c_str(), ERR);
	  return parseFactor(); //redo
	}
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token <";
	  msg << m_state.getTokenDataAsString(pTok).c_str() << ">; Unreading it";
	  MSG(&pTok, msg.str().c_str(), DEBUG);
	  unreadToken(); //eat the error token
	}
      };
    return rtnNode;
  } //parseFactor

  Node * Parser::parseFactorStartingWithAType(const Token& tTok, bool allowrefcast, bool allowcasts)
  {
    assert(Token::isTokenAType(tTok)); //was unread.

    if(m_state.isThisLocalsFileScope())
      {
	if((tTok.m_type == TOK_KW_TYPE_SELF) || tTok.m_type == TOK_KW_TYPE_SUPER)
	  {
	    std::ostringstream msg;
	    msg << "Illegal token <" << m_state.getTokenDataAsString(tTok).c_str();
	    msg << "> in locals filescope context";
	    MSG(&tTok, msg.str().c_str(), ERR);
	    getTokensUntil(TOK_SEMICOLON); //perhaps read until semi-colon
	    return NULL; //t41550
	  }
      }

    TypeArgs typeargs;
    typeargs.m_forFactor = true; //t41551
    NodeTypeDescriptor * typeNode = parseTypeDescriptor(typeargs);
    assert(typeNode);
    UTI uti = typeNode->givenUTI();

    Token dTok;
    getNextToken(dTok);
    unreadToken();

    Node * rtnNode = NULL;
    if(typeargs.m_danglingDot || (dTok.m_type == TOK_DOT))
      //returns either a terminal or proxy
      rtnNode = parseMinMaxSizeofType(NULL, uti, typeNode); //optionally, gets next dot token

    if(rtnNode == NULL)
      {
	Token iTok;
	getNextToken(iTok);
	if(iTok.m_type == TOK_IDENTIFIER)
	  {
	    if(typeargs.m_danglingDot || (dTok.m_type == TOK_DOT))
	      {
		//assumes we saw a 'dot' prior, be careful
		unreadToken();
		rtnNode = parseNamedConstantFromAnotherClass(typeargs, typeNode);
		rtnNode = parseRestOfFactor(rtnNode); //dm of constant class? t41198
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Expected a dot prior to named constant '";
		msg << m_state.getTokenDataAsString(iTok).c_str() << "'";
		MSG(&iTok, msg.str().c_str(), ERR); //t41551,t41306
	      }
	  }
	else if(iTok.m_type == TOK_CLOSE_PAREN)
	  {
	    unreadToken();
	    bool castok = false;
	    if(allowcasts)
	      castok = makeCastNode(tTok, allowrefcast, typeNode, rtnNode);
	    else
	      {
		std::ostringstream msg;
		msg << "Casts not allowed in this context";
		MSG(&iTok, msg.str().c_str(), ERR);
	      }
	    if(!castok)
	      {
		delete typeNode; //owns the dangling type descriptor
		typeNode = NULL;
	      }
	  }
	else
	  {
	    //t41402 segfault, errors t3453,4,t3680,t3705,t41138
	    //clean up, some kind of error parsing min/max/sizeof
	    delete typeNode;
	    typeNode = NULL;
	    unreadToken(); //t3680
	  }
      }
    else
      {
	rtnNode = parseRestOfFactor(rtnNode);  //any more? t41375
      }
    return rtnNode; //rtnNode could be NULL!
  } //parseFactorStartingWithAType

  Node * Parser::parseRestOfFactor(Node * leftNode)
  {
    if(leftNode == NULL) return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    unreadToken();

    switch(pTok.m_type)
      {
      case TOK_KW_AS:
	m_state.confirmParsingConditionalAs(pTok); //SETS other related globals //t3249
	//fallthru to IS..
      case TOK_KW_IS:
	rtnNode = makeConditionalExprNode(leftNode);
	rtnNode = parseRestOfFactor(rtnNode); //any more???
	break;
      case TOK_DOT:
	rtnNode = parseRestOfMemberSelectExpr(leftNode); //t390
	rtnNode = parseRestOfFactor(rtnNode); //any more? t3921
	break;
      case TOK_SQUARE:
      case TOK_OPEN_SQUARE:
	rtnNode = parseRestOfLvalExpr(leftNode); //t41074, lhs, t3941
	rtnNode = parseRestOfFactor(rtnNode); //any more?
	break;
      case TOK_PLUS_PLUS: //t3903
      case TOK_MINUS_MINUS: //t3903
	rtnNode = makeAssignExprNode(leftNode);
	rtnNode = parseRestOfFactor(rtnNode); //any more???
	break;
      case TOK_ERROR_LOWLEVEL:
	getNextToken(pTok); //eat token
	getNextToken(pTok); //eat token
	break;
      default:
	rtnNode = leftNode;
      };
    return rtnNode;
  } //parseRestOfFactor

  Node * Parser::parseRestOfCastOrExpression(bool allowRefCast, bool allowCasts)
  {
    Node * rtnNode = NULL;
    //just saw an open paren..
    //if next token is a type this a user cast, min/max/sizeof, named constant, o.w. expression
    bool isacast = false;

    Token tTok;
    getNextToken(tTok);
    unreadToken();

    if(Token::isTokenAType(tTok) || (tTok.m_type == TOK_KW_LOCALDEF))
      {
	rtnNode = parseFactorStartingWithAType(tTok, allowRefCast, allowCasts);
	isacast = (rtnNode && rtnNode->isExplicitCast());
      }

    if(isacast)
      {
	return rtnNode; //no close paren, no wrapFactor  t41055
      }

    if(rtnNode)
      {
	//not a cast. Is min/max/sizeof a Type
	rtnNode = wrapFactor(rtnNode); //t3232,t41279 ascent-parse
      }
    else
      {
	//not a cast or min/max/sizeof of named constant;
	//e.g. question colon (t41280);
	rtnNode = parseAssignExpr(); //grammar says assign_expr
	//will parse rest of assign expr before close paren
      }

    Token pTok;
    if(!getExpectedToken(TOK_CLOSE_PAREN, pTok, QUIETLY))
      {
	    std::ostringstream msg;
	    msg << "Unexpected token <" << m_state.getTokenDataAsString(pTok).c_str();
	    msg << ">; expected '";
	    msg << Token::getTokenAsStringFromPool(TOK_CLOSE_PAREN, &m_state).c_str();
	    msg << "'";
	    MSG(&pTok, msg.str().c_str(), ERR);
	    delete rtnNode;
	    rtnNode = NULL;
      } //t41279
    return rtnNode;
  } //parseRestOfCastOrExpression

  Node * Parser::parseRestOfTerm(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    switch(pTok.m_type)
      {
      case TOK_STAR:
      case TOK_SLASH:
      case TOK_PERCENTSIGN:
	{
	  unreadToken();
	  rtnNode = makeTermNode(leftNode);
	  rtnNode = parseRestOfTerm(rtnNode);
	  break;
	}
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //parseRestOfTerm

  Node * Parser::parseRestOfShiftExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_PLUS:
      case TOK_MINUS:
	unreadToken();
	rtnNode = makeShiftExpressionNode(leftNode);
	rtnNode = parseRestOfShiftExpression(rtnNode); //recursion of left-associativity
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = parseRestOfShiftExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //parseRestOfShiftExpression

  Node * Parser::parseRestOfCompareExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_SHIFT_LEFT:
      case TOK_SHIFT_RIGHT:
	unreadToken();
	rtnNode = makeCompareExpressionNode(leftNode);
	rtnNode = parseRestOfCompareExpression(rtnNode); //recursion of left-associativity
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = parseRestOfCompareExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //parseRestOfCompareExpression

  Node * Parser::parseRestOfEqExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_LESS_THAN:
      case TOK_GREATER_THAN:
      case TOK_LESS_EQUAL:
      case TOK_GREATER_EQUAL:
	unreadToken();
	rtnNode = makeEqExpressionNode(leftNode);
	rtnNode = parseRestOfEqExpression(rtnNode); //recursion of left-associativity
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = parseRestOfEqExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //parseRestOfEQExpression

  Node * Parser::parseRestOfBitExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_EQUAL_EQUAL:
      case TOK_NOT_EQUAL:
	unreadToken();
	rtnNode = makeBitExpressionNode(leftNode);
	rtnNode = parseRestOfBitExpression(rtnNode); //recursion of left-associativity
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = parseRestOfBitExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //parseRestOfBitExpression

  Node * Parser::parseRestOfLogicalExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_AMP:
      case TOK_PIPE:
      case TOK_HAT:
	unreadToken();
	rtnNode = makeLogicalExpressionNode(leftNode);
	rtnNode = parseRestOfLogicalExpression(rtnNode); //recursion of left-associativity
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = parseRestOfLogicalExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //parseRestOfLogicalExpression

  Node * Parser::parseRestOfExpression(Node * leftNode)
  {
    if(leftNode == NULL) return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_AMP_AMP:
      case TOK_PIPE_PIPE:
	unreadToken();
	rtnNode = makeExpressionNode(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //recursion of left-associativity
	break;
      case TOK_QUESTION:
	unreadToken();
	rtnNode = makeQuestionColonNode(leftNode); //t41063,5,7,9,t41071,2,3 (not restOfAssignExpr)
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = parseRestOfExpression(leftNode); //redo
	break;
      default:
	unreadToken();
	rtnNode = leftNode;
      };
    return rtnNode;
  } //parseRestOfExpression

  Node * Parser::parseRestOfLvalExpr(Node * leftNode)
  {
    if(leftNode == NULL) return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_SQUARE)
      {
	rtnNode = new NodeSquareBracket(leftNode, NULL, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	return rtnNode; //t3768
      }

    if(pTok.m_type != TOK_OPEN_SQUARE)
      {
	unreadToken();
	return leftNode;
      }

    //context reverts to non-member block for array index
    m_state.pushCurrentBlockAndDontUseMemberBlock(m_state.getCurrentBlock()); //t41597

    Node * rightNode = parseExpression();
    //Array size may be blank if initialized (not array item!!);
    rtnNode = new NodeSquareBracket(leftNode, rightNode, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(pTok.m_locator);

    m_state.popClassContext(); //restore

    Token tmpTok; //non quietly, t41294
    if(!getExpectedToken(TOK_CLOSE_SQUARE, tmpTok))
      {
	delete rtnNode; //also deletes leftNode
	rtnNode = NULL;
	return rtnNode;
      }
    //for custom array of (custom) arrays
    return parseRestOfLvalExpr(rtnNode); //t3942,3,4 (was return rtnNode)
  } //parseRestOfLValExpr

  Node * Parser::parseRestOfAssignExpr(Node * leftNode)
  {
    if(leftNode == NULL) return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_EQUAL:
      case TOK_PLUS_EQUAL:
      case TOK_MINUS_EQUAL:
      case TOK_STAR_EQUAL:
      case TOK_SLASH_EQUAL: //t3853
      case TOK_PERCENTSIGN_EQUAL:
      case TOK_AMP_EQUAL:
      case TOK_PIPE_EQUAL:
      case TOK_HAT_EQUAL:
      case TOK_SHIFT_LEFT_EQUAL:
      case TOK_SHIFT_RIGHT_EQUAL:
      case TOK_PLUS_PLUS: //t3903
      case TOK_MINUS_MINUS: //t3903
	unreadToken();
	rtnNode = makeAssignExprNode(leftNode);
	rtnNode = parseRestOfAssignExpr(rtnNode); //any more?
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = parseRestOfAssignExpr(leftNode); //try again?
	break;
      case TOK_SEMICOLON:
      case TOK_CLOSE_PAREN:
      case TOK_COMMA: /* array item init */
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //parseRestOfAssignExpr

Node * Parser::wrapFactor(Node * leftNode)
  {
    if(leftNode == NULL) return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    unreadToken();

    switch(pTok.m_type)
      {
      case TOK_IDENTIFIER:
      case TOK_SQUOTED_STRING:
      case TOK_DQUOTED_STRING:
      case TOK_NUMBER_FLOAT:
      case TOK_OPEN_PAREN:
      case TOK_KW_FLAG_INSERTFUNC:
      case TOK_KW_FLAG_INSERTCLASS:
      case TOK_KW_FLAG_INSERTCLASSSIGNATURE:
      case TOK_KW_FLAG_INSERTCLASSNAMESIMPLE:
      case TOK_KW_FLAG_INSERTCLASSNAMEPRETTY:
      case TOK_KW_FLAG_INSERTCLASSNAMEMANGLED:
	rtnNode = parseFactor();
	rtnNode = parseRestOfFactor(rtnNode);
	break;
      case TOK_ERROR_LOWLEVEL:
	getNextToken(pTok); //eat token?
	getNextToken(pTok); //eat token?
	rtnNode = wrapFactor(leftNode); //redo
	break;
      case TOK_MINUS: //t41055
      case TOK_PLUS:
      case TOK_BANG:
      case TOK_TWIDDLE:
      case TOK_PLUS_PLUS:
      case TOK_MINUS_MINUS:
      case TOK_NUMBER_SIGNED:
      case TOK_NUMBER_UNSIGNED:
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
      case TOK_KW_LOCALDEF:
      default:
	rtnNode = wrapTerm(leftNode); //ascend
      };
    return rtnNode;
  } //wrapFactor

  Node * Parser::wrapTerm(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    switch(pTok.m_type)
      {
      case TOK_STAR:
      case TOK_SLASH:
      case TOK_PERCENTSIGN:
	{
	  unreadToken();
	  rtnNode = makeTermNode(leftNode);
	  rtnNode = wrapTerm(rtnNode); //before close paren, any more.. t41279
	  break;
	}
      case TOK_ERROR_LOWLEVEL:
	rtnNode = wrapTerm(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = wrapShiftExpression(leftNode); //ascend
	}
      };
    return rtnNode;
  } //wrapTerm

  Node * Parser::wrapShiftExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_PLUS:
      case TOK_MINUS:
	unreadToken();
	rtnNode = makeShiftExpressionNode(leftNode);
	rtnNode = wrapShiftExpression(rtnNode); //before close paren, any more.. t41279
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = wrapShiftExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = wrapCompareExpression(leftNode); //ascend
	}
      };
    return rtnNode;
  } //wrapShiftExpression

  Node * Parser::wrapCompareExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_SHIFT_LEFT:
      case TOK_SHIFT_RIGHT:
	unreadToken();
	rtnNode = makeCompareExpressionNode(leftNode);
	rtnNode = wrapCompareExpression(rtnNode); //before close paren, any more.. t41279
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = wrapCompareExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = wrapEqExpression(leftNode); //ascend
	}
      };
    return rtnNode;
  } //wrapCompareExpression

  Node * Parser::wrapEqExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_LESS_THAN:
      case TOK_GREATER_THAN:
      case TOK_LESS_EQUAL:
      case TOK_GREATER_EQUAL:
	unreadToken();
	rtnNode = makeEqExpressionNode(leftNode);
	rtnNode = wrapEqExpression(rtnNode); //before close paren, any more.. t41279
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = wrapEqExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = wrapBitExpression(leftNode); //ascend
	}
      };
    return rtnNode;
  } //wrapEQExpression

  Node * Parser::wrapBitExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_EQUAL_EQUAL:
      case TOK_NOT_EQUAL:
	unreadToken();
	rtnNode = makeBitExpressionNode(leftNode);
	rtnNode = wrapBitExpression(rtnNode); //before close paren, any more.. t41279
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = wrapBitExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = wrapLogicalExpression(leftNode); //ascend
	}
      };
    return rtnNode;
  } //wrapBitExpression

  Node * Parser::wrapLogicalExpression(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_AMP:
      case TOK_PIPE:
      case TOK_HAT:
	unreadToken();
	rtnNode = makeLogicalExpressionNode(leftNode);
	rtnNode = wrapLogicalExpression(rtnNode); //before close paren, any more.. t41279
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = wrapLogicalExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = wrapExpression(leftNode); //ascend
	}
      };
    return rtnNode;
  } //wrapLogicalExpression

  Node * Parser::wrapExpression(Node * leftNode)
  {
    if(leftNode == NULL) return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_AMP_AMP:
      case TOK_PIPE_PIPE:
	unreadToken();
	rtnNode = makeExpressionNode(leftNode);
	rtnNode = wrapExpression(rtnNode); //before close paren, any more.. t41279
	break;
      case TOK_QUESTION:
	unreadToken();
	rtnNode = makeQuestionColonNode(leftNode); //t41280
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = wrapExpression(leftNode); //redo
	break;
      default:
	unreadToken();
	rtnNode = wrapAssignExpr(leftNode); //ascend
      };
    return rtnNode;
  } //wrapExpression

  Node * Parser::wrapAssignExpr(Node * leftNode)
  {
    if(leftNode == NULL) return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_EQUAL:
      case TOK_PLUS_EQUAL:
      case TOK_MINUS_EQUAL:
      case TOK_STAR_EQUAL:
      case TOK_SLASH_EQUAL: //t3853
      case TOK_PERCENTSIGN_EQUAL:
      case TOK_AMP_EQUAL:
      case TOK_PIPE_EQUAL:
      case TOK_HAT_EQUAL:
      case TOK_SHIFT_LEFT_EQUAL:
      case TOK_SHIFT_RIGHT_EQUAL:
      case TOK_PLUS_PLUS: //t3903
      case TOK_MINUS_MINUS: //t3903
	unreadToken();
	rtnNode = makeAssignExprNode(leftNode);
	rtnNode = wrapAssignExpr(rtnNode); //before close paren, any more.. t41279
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = wrapAssignExpr(leftNode); //redo
	break;
      case TOK_SEMICOLON:
      case TOK_CLOSE_PAREN:
      case TOK_COMMA: /* array item init */
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //wrapAssignExpr

  //assignOK true by default. These assignments are for local
  // variables, not data members.  They create a parse subtree for the
  // NodeVarDecl; and do not have to be constant expressions. Data
  // member initialization expressions are constant expressions, and
  // are a child of the NodeVarDeclDM subclass (see parseDataMember).
  // References (locals only) save their initialized value, a
  // storeintoable ("lval") expression, in their NodeVarRef.
  bool Parser::parseRestOfDecls(TypeArgs& args, UTI passuti)
  {
    bool brtn = true;
    Token pTok;
    getNextToken(pTok);

    args.m_arraysize = NONARRAYSIZE; //clear for decl list (args ref)

    if(pTok.m_type != TOK_COMMA)
      {
	unreadToken(); //likely semicolon
	return true; //done
      }

    Token iTok;
    getNextToken(iTok);

    if(iTok.m_type == TOK_AMP)
      {
	args.m_declRef = args.m_hasConstantTypeModifier ? ALT_CONSTREF : ALT_REF; //a declared reference (was ALT_REF)
	args.m_referencedUTI = passuti; //?
	getNextToken(iTok);
      }

    if(iTok.m_type == TOK_IDENTIFIER)
      {
	//just the top level as a basic uti (no selects, or arrays, or refs???)
	NodeTypeDescriptor * typeNode = new NodeTypeDescriptor(args.m_typeTok, passuti, m_state);
	//another decl of same type
	NodeVarDecl * sNode = makeVariableSymbol(args, iTok, typeNode); //a decl !!
	if(sNode)
	  {
	    parseRestOfDeclInitialization(args, iTok, sNode);
	    m_state.appendNodeToCurrentBlock(sNode);
	  }
	else //error msg?
	  brtn = false;
      }
    else
      {
	std::ostringstream msg;
	msg << "Variable name is missing after comma; Token <";
	msg << m_state.getTokenDataAsString(pTok).c_str() << ">";
	MSG(&iTok, msg.str().c_str(), ERR);
	brtn = false;
      }

    if(brtn)
      return parseRestOfDecls(args, passuti); //recurse

    getTokensUntil(TOK_SEMICOLON); //perhaps read until semi-colon
    unreadToken();
    return false;
  } //parseRestOfDecls

  bool Parser::parseRestOfDeclInitialization(TypeArgs& args, const Token& identTok, NodeVarDecl * dNode)
  {
    assert(dNode);
    Token eTok;
    getNextToken(eTok);

    if(eTok.m_type == TOK_EQUAL) //first check for '='
      {
	//assignments are always permitted, continue..
	assert(args.m_assignOK);
      }
    else if((args.m_declRef == ALT_REF) || (args.m_declRef == ALT_CONSTREF))
      {
	//assignments required for references
	std::ostringstream msg;
	msg << "Must assign to reference '" << m_state.getTokenDataAsString(identTok).c_str();
	msg << "' at the time of its declaration";
	MSG(&eTok, msg.str().c_str(), ERR);
	getTokensUntil(TOK_SEMICOLON); //rest of statement is ignored.
	unreadToken();
	return false; //done
      }
    else if(eTok.m_type == TOK_OPEN_PAREN)
      {
	if((Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD) && (args.m_typeTok.m_type != TOK_KW_TYPE_SELF) && (args.m_typeTok.m_type != TOK_KW_TYPE_SUPER))
	  {
	    std::ostringstream msg;
	    msg << "Unexpected token <" << m_state.getTokenDataAsString(eTok).c_str();
	    msg << "> indicates a constructor function call or function definition; ";
	    msg << "neither are valid here for primitive variable '";
	    msg << m_state.getTokenDataAsString(identTok).c_str() << "'";
	    MSG(&eTok, msg.str().c_str(), ERR); //t41082
	    unreadToken();
	    return false; //done
	  } //else returning special keyword type Self, or Super (t41084)
      }

    //continuing..
    bool brtn = true;
    //update dNode with init expression: lval for ref, assign for local car
    if((args.m_declRef == ALT_REF) || (args.m_declRef == ALT_CONSTREF))
      {
	brtn = parseRestOfRefInitialization(identTok, args.m_declRef, dNode);
	args.m_declRef = ALT_NOT; //clear flag in case of decl list
	//keep args.m_referenced type???
      } //ref done
    else
      {
	unreadToken();
	brtn = parseRestOfInitialization(identTok, dNode);
      }
    return brtn;
  } //parseRestOfDeclInitialization

  bool Parser::parseRestOfRefInitialization(const Token& identTok, ALT reftype, NodeVarDecl * dNode)
  {
    bool brtn = true;
    SYMBOLTYPEFLAG saveTheFlag = m_state.m_parsingVariableSymbolTypeFlag;
    if(m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCLOCALVAR)
      {
	if(reftype == ALT_REF)
	  m_state.m_parsingVariableSymbolTypeFlag = STF_FUNCLOCALREF;
	else if(reftype == ALT_CONSTREF)
	  m_state.m_parsingVariableSymbolTypeFlag = STF_FUNCLOCALCONSTREF;
	else
	  m_state.abortShouldntGetHere();
      }

    Node * initnode = parseExpression();
    if(initnode == NULL)
      {
	std::ostringstream msg;
	msg << "Initial value of reference '" << m_state.getTokenDataAsString(identTok).c_str();
	msg << "' is missing";
	MSG(&identTok, msg.str().c_str(), ERR);
	brtn = false;
      }
    else
      ((NodeVarRef *) dNode)->setInitExpr(initnode);

    m_state.m_parsingVariableSymbolTypeFlag = saveTheFlag; //restore flag
    return brtn;
  } //parseRestOfRefInitialization

  Node * Parser::parseArrayOrClassInitialization(u32 identId)
  {
    Token aTok;
    getNextToken(aTok); //eat the curly, pass along its loc
    assert(aTok.m_type == TOK_OPEN_CURLY);

    Token bTok;
    getNextToken(bTok);
    unreadToken();

    if(bTok.m_type == TOK_DOT)
      {
	return parseClassInstanceInitialization(identId, aTok.m_locator);
      }
    else if(bTok.m_type == TOK_CLOSE_CURLY)
      {
	//can't tell whether this is an empty array initializer, or empty class initializer (t41206)
	NodeListEmpty * rtnList = new NodeListEmpty(m_state);
	assert(rtnList);
	rtnList->setNodeLocation(aTok.m_locator);
	getNextToken(bTok); //eat token
	return rtnList;
      }
    //else
    return parseArrayInitialization(identId, aTok.m_locator);
  } //parseArrayOrClassInitialization

  Node * Parser::parseArrayInitialization(u32 identId, Locator loc)
  {
    NodeListArrayInitialization * rtnList = new NodeListArrayInitialization(m_state); //delete if error
    assert(rtnList);
    rtnList->setNodeLocation(loc);

    if(!parseArrayItemInit(identId, rtnList))
      {
	delete rtnList;
	rtnList = NULL; //quit? read until close_curly? semi-colon, or comma?
      }
    return rtnList;
  } //parseArrayInitialization

  bool Parser::parseArrayItemInit(u32 identId, NodeListArrayInitialization * rtnList)
  {
    Token aTok;
    getNextToken(aTok);

    if(aTok.m_type == TOK_CLOSE_CURLY)
      {
	return true; //done
      }
    else if((aTok.m_type == TOK_SEMICOLON))
      {
	unreadToken();
	return true; //done  t41658
      }
    else if((aTok.m_type != TOK_COMMA))
      {
	u32 n = rtnList->getNumberOfNodes();
	// dot indicates a class initialization, not a primitive array
	// open curly indicates this or another array, e.g. an array of classes (t41170)
	if(aTok.m_type == TOK_OPEN_CURLY)
	  {
	    //this first item, or another array, or class initialization
	    // eat it!?
	  }
	else
	  {
	    unreadToken();
	    if((n != 0) || (aTok.m_type == TOK_DOT))
	      {
		std::ostringstream msg;
		msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(aTok).c_str();
		msg << "> while parsing array variable " << m_state.m_pool.getDataAsString(identId).c_str();
		msg << ", item " << (n + 1);
		MSG(&aTok, msg.str().c_str(), ERR);
		return false; //original caller owns rtnList, should delete if empty!
	      }
	  }
      }
    //else is a comma, eat and continue..

    Token rTok;
    getNextToken(rTok);
    unreadToken();

    Node * assignNode = NULL;
    if(rTok.m_type == TOK_DOT)
      assignNode = parseClassInstanceInitialization(identId, aTok.m_locator); //t41170
    else if((rTok.m_type == TOK_CLOSE_CURLY) && (aTok.m_type != TOK_COMMA)) //t41659
      {
	//uses default class values, no data members with values listed within {};
	getNextToken(rTok); //eat token
	Token eTok;
	getNextToken(eTok); //peak ahead
	unreadToken();

	//comma or another close curly, but not semicolon
	if((eTok.m_type == TOK_COMMA) || (eTok.m_type == TOK_CLOSE_CURLY))
	  {
	    Symbol * tmpcsym = NULL;
	    UTI cuti = Hzy; //default, wait until c&l if unseen
	    if(m_state.isIdInCurrentScope(identId, tmpcsym))
	      cuti = tmpcsym->getUlamTypeIdx(); //could be an array or holder; wait for c&l

	    NodeListClassInit * noList = new NodeListClassInit(cuti, identId, m_state); //t41658
	    assert(noList); //note: not the same as NodeListEmpty
	    noList->setNodeLocation(rTok.m_locator);
	    assignNode = noList;
	  }
	//else assignNode = NULL; likely SEMICOLON.
      }
    else
      assignNode = parseAssignExpr(); //did not peak ahead

    if(assignNode == NULL) //t41658
      {
	u32 n = rtnList->getNumberOfNodes();
	std::ostringstream msg;
	msg << "Initial value of array variable '" << m_state.m_pool.getDataAsString(identId).c_str();
	msg << "', item " << (n + 1);
	msg << " is missing";

	if(n > 0)
	  MSG(&aTok, msg.str().c_str(), DEBUG); //dangling comma allowed by g++
	else
	  {
	    MSG(&aTok, msg.str().c_str(), ERR);
	    return false; //original caller owns rtnList, should delete if empty!
	  }
      }
    else
      rtnList->addNodeToList(assignNode);
    return parseArrayItemInit(identId, rtnList); //recurse
  } //parseArrayItemInit

  Node * Parser::parseClassInstanceInitialization(u32 classvarId, Locator loc)
  {
    Symbol * tmpcsym = NULL;
    UTI cuti = Hzy; //default, wait until c&l if unseen
    if(m_state.isIdInCurrentScope(classvarId, tmpcsym))
      cuti = tmpcsym->getUlamTypeIdx(); //could be an array or holder; wait for c&l (t41185, t41232,3,4)

    NodeListClassInit * rtnList = new NodeListClassInit(cuti, classvarId, m_state); //delete if error
    assert(rtnList);
    rtnList->setNodeLocation(loc);

    if(!parseClassItemInit(classvarId, cuti, rtnList))
      {
	delete rtnList;
	rtnList = NULL; //quit? read until close_curly? semi-colon, or comma?
      }

    return rtnList;
  } //parseClassInstanceInitialization

  bool Parser::parseClassItemInit(u32 classvarId, UTI classUTI, NodeListClassInit * rtnList)
  {
    Token aTok;
    getNextToken(aTok);

    assert(rtnList);
    u32 n = rtnList->getNumberOfNodes();

    if(aTok.m_type == TOK_CLOSE_CURLY)
      {
	return true; //done
      }
    else if((aTok.m_type != TOK_COMMA))
      {
	// dot indicates a class initialization, not a primitive array
	// what if an array of classes?
	if(!((n == 0) && (aTok.m_type == TOK_DOT)))
	  {
	    unreadToken();

	    std::ostringstream msg;
	    msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(aTok).c_str();
	    msg << "> while parsing class variable '" << m_state.m_pool.getDataAsString(classvarId).c_str();
	    msg << "', init item " << (n + 1);
	    MSG(&aTok, msg.str().c_str(), ERR);
	    return false; //original caller owns rtnList, should delete if empty!
	  }
	//else n is 0 and a dot
      }
    else //is a comma
      {
	getNextToken(aTok);
	if(!((aTok.m_type == TOK_DOT) || (aTok.m_type == TOK_CLOSE_CURLY)))
	  {
	    unreadToken();

	    std::ostringstream msg;
	    msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(aTok).c_str();
	    msg << "> while parsing class variable '" << m_state.m_pool.getDataAsString(classvarId).c_str();
	    msg << "', init item " << (n + 1);
	    MSG(&aTok, msg.str().c_str(), ERR);
	    return false; //original caller owns rtnList, should delete if empty!
	  }
	else if(aTok.m_type == TOK_CLOSE_CURLY)
	  return true;
      }

    //pls continue..get data member name
    Token iTok;
    if(!getExpectedToken(TOK_IDENTIFIER, iTok, QUIETLY))
      {
	std::ostringstream msg;
	msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(iTok).c_str();
	msg << "> while parsing class variable '" << m_state.m_pool.getDataAsString(classvarId).c_str();
	msg << "', init item " << (n + 1) << ", expected a data member identifier";
	MSG(&iTok, msg.str().c_str(), ERR);
	return false; //original caller owns rtnList, should delete if empty!
      }

    // get =
    Token pTok;
    if(!getExpectedToken(TOK_EQUAL, pTok, QUIETLY))
      {
	std::ostringstream msg;
	msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(pTok).c_str();
	msg << "> while parsing class variable '" << m_state.m_pool.getDataAsString(classvarId).c_str();
	msg << "', init item " << (n + 1) << ", expected '='";
	MSG(&pTok, msg.str().c_str(), ERR);
	return false; //original caller owns rtnList, should delete if empty!
      }

    Token rTok;
    getNextToken(rTok);
    unreadToken();

    Node * assignNode = NULL;
    if(rTok.m_type == TOK_OPEN_CURLY)
      assignNode = parseArrayOrClassInitialization(iTok.m_dataindex); //t41168, t41169
    else
      assignNode = parseAssignExpr();

    if(assignNode == NULL) //t41658
      {
	std::ostringstream msg;
	msg << "Initial value of class variable '" << m_state.m_pool.getDataAsString(classvarId).c_str();
	msg << "', item " << (n + 1) << " '" << m_state.m_pool.getDataAsString(iTok.m_dataindex).c_str();
	msg << "' is missing";
	MSG(&pTok, msg.str().c_str(), ERR);
	return false; //original caller owns rtnList, should delete if empty!
      }
    else
      {
	NodeInitDM * dmInitNode = new NodeInitDM(iTok.m_dataindex, assignNode, classUTI, m_state);
	assert(dmInitNode);
	dmInitNode->setNodeLocation(iTok.m_locator);

	rtnList->addNodeToList(dmInitNode);
      }
    return parseClassItemInit(classvarId, classUTI, rtnList); //recurse
  } //parseClassItemInit

  NodeConstantDef * Parser::parseRestOfConstantDef(NodeConstantDef * constNode, bool assignREQ, bool isStmt)
  {
    assert(constNode);
    NodeConstantDef * rtnNode = constNode;
    u32 constId = constNode->getSymbolId();

    Token pTok;
    if(getExpectedToken(TOK_EQUAL, pTok, QUIETLY))
      {
	//check for possible start of array init
	Token eTok;
	getNextToken(eTok);
	unreadToken();
	Node * exprNode = NULL;
	if(eTok.m_type == TOK_OPEN_CURLY)
	  {
	    exprNode = parseArrayOrClassInitialization(constId); //returns a NodeList
	  }
	else
	  {
	    exprNode = parseExpression(); //makeAssignExprNode(leftNode);
	  }

	if(exprNode != NULL)
	  constNode->setConstantExpr(exprNode);
	else
	  {
	    std::ostringstream msg;
	    msg << "Missing named constant definition after '=' for '";
	    msg << m_state.m_pool.getDataAsString(constId).c_str() << "'";
	    MSG(&pTok, msg.str().c_str(), ERR);
	    delete constNode; //also deletes the symbol, and nodetypedesc.
	    constNode = NULL;
	    rtnNode = NULL;
	  }
      }
    else
      {
	//let the = constant expr be optional in case of class params (error/t3524)
	if(assignREQ)
	  {
	    std::ostringstream msg;
	    msg << "Missing '=' after named constant definition '";
	    msg << m_state.m_pool.getDataAsString(constId).c_str() << "'";
	    MSG(&pTok, msg.str().c_str(), ERR);

	    if(isStmt)
	      {
		//perhaps read until semi-colon
		getTokensUntil(TOK_SEMICOLON);
		unreadToken();
	      }
	    delete constNode; //also deletes the symbol, and nodetypedesc.
	    constNode = NULL;
	    rtnNode = NULL;
	  }
	//else unreadToken(); //class param doesn't have equal; wait for the class arg
      }

    if(isStmt)
      {
	if(!getExpectedToken(TOK_SEMICOLON))
	  {
	    std::ostringstream msg;
	    msg << "Missing ';' after named constant definition '";
	    msg << m_state.m_pool.getDataAsString(constId).c_str() << "'";
	    msg << "; Lists not supported";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	else
	  unreadToken();
      }
    return rtnNode;
  } //parseRestOfConstantDef

  NodeModelParameterDef * Parser::parseRestOfParameterDef(NodeModelParameterDef * paramNode)
  {
    NodeModelParameterDef * rtnNode = paramNode;
    u32 mpId = paramNode->getSymbolId();
    Node * exprNode = NULL;

    Token pTok;
    if(getExpectedToken(TOK_EQUAL, pTok, QUIETLY))
      {
	//check for possible start of array init
	Token eTok;
	getNextToken(eTok);
	unreadToken();
	if(eTok.m_type == TOK_OPEN_CURLY)
	  {
	    std::ostringstream msg;
	    msg << "Model Parameter '";
	    msg << m_state.m_pool.getDataAsString(mpId).c_str();
	    msg << "' cannot be based on a class nor array type";
	    MSG(&eTok, msg.str().c_str(), ERR); //t3217
	  }
	else
	  {
	    exprNode = parseExpression(); //makeAssignExprNode(leftNode);
	  }
      }

    if(exprNode)
      paramNode->setConstantExpr(exprNode);
    else
      {
	std::ostringstream msg;
	msg << "Missing model parameter definition after '=' for '";
	msg << m_state.m_pool.getDataAsString(mpId).c_str() << "'";
	MSG(&pTok, msg.str().c_str(), ERR);

	//perhaps read until semi-colon
	getTokensUntil(TOK_SEMICOLON);
	unreadToken();
	delete paramNode; //also deletes the symbol, and nodetypedesc.
	paramNode = NULL;
	rtnNode = NULL;
      }

    if(!getExpectedToken(TOK_SEMICOLON))
      {
	std::ostringstream msg;
	msg << "Missing ';' after model parameter definition '";
	msg << m_state.m_pool.getDataAsString(paramNode->getSymbolId()).c_str() << "'";
	msg << "; Lists not supported";
	MSG(&pTok, msg.str().c_str(), ERR);
      }
    else
      unreadToken();

    return rtnNode;
  } //parseRestOfParameterDef

  NodeBlockFunctionDefinition * Parser::makeFunctionBlock(TypeArgs& args, const Token& identTok, NodeTypeDescriptor * nodetype, bool isVirtual, bool insureVirtualOverride, bool isConstr)
  {
    NodeBlockFunctionDefinition * rtnNode = NULL;

    //all functions are defined in this "class" block; or external 'use' for declarations.
    //this is a block with its own ST
    NodeBlockContext * cblock = m_state.getContextBlock();
    assert(cblock && cblock->isAClassBlock());
    NodeBlockClass * currClassBlock = (NodeBlockClass *) cblock;
    NodeBlock * prevBlock = m_state.getCurrentBlock();
    if(prevBlock != currClassBlock)
      {
	std::ostringstream msg;
	msg << "Sanity check failed. Likely an error caught before '";
	msg << m_state.getTokenDataAsString(identTok).c_str() << "'";
	msg << " function block";
	MSG(&identTok, msg.str().c_str(), ERR);

	m_state.popClassContext(); //m_currentBlock = prevBlock;
	return NULL;
      }

    assert(nodetype);
    UTI rtnuti = nodetype->givenUTI();

    SymbolFunction * fsymptr = new SymbolFunction(identTok, rtnuti, m_state);
    fsymptr->setStructuredComment(); //also clears

    if(isVirtual)
      fsymptr->setVirtualFunction();

    if(insureVirtualOverride)
      fsymptr->setInsureVirtualOverrideFunction(); //t41096

    if(isConstr)
      fsymptr->setConstructorFunction(); //t41077

    if(m_state.getUlamClassForThisClass() == UC_QUARK)
      fsymptr->setDefinedInAQuark();

    //WAIT for the parameters, so we can add it to the SymbolFunctionName map..
    rtnNode =  new NodeBlockFunctionDefinition(fsymptr, prevBlock, nodetype, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(args.m_typeTok.m_locator);

    //symbol will have pointer to body (or just decl for 'use');
    fsymptr->setFunctionNode(rtnNode); //tfr ownership

    u32 ftokid = m_state.getTokenDataAsStringId(identTok); //t41077,t41134,t41314
    m_state.m_parsingFUNCid = ftokid;
    m_state.m_parsingFuncDefToken = identTok;

    //set class type to custom array; the current class block
    //node type was set to its class symbol type after checkAndLabelType
    // caType is the return type of the 'aget' method (set here).
    if(m_state.getCustomArrayGetFunctionNameId() == ftokid)
      {
	UTI cuti = currClassBlock->getNodeType(); //prevBlock
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	((UlamTypeClass *) cut)->setCustomArray();
      }

    //Here before push to get correct class block NodeNo
    //Now, look specifically for a function with the same given name defined
    Symbol * fnSym = NULL;
    if(!currClassBlock->isFuncIdInScope(ftokid, fnSym))
      {
	//first time name used as a function..add symbol function name/typeNav
	fnSym = new SymbolFunctionName(identTok, Nav, m_state);

	//ownership goes to the class block's ST
	currClassBlock->addFuncIdToScope(fnSym->getId(), fnSym);
      }

    bool isOperatorOverload = false;
    if(identTok.isOperatorOverloadIdentToken(&m_state))
      {
	((SymbolFunctionName *) fnSym)->setOperatorOverloadFunctionName();
	isOperatorOverload = true;
      }

    m_state.pushCurrentBlock(rtnNode); //before parsing the args

    //use space on funcCallStack for return statement.
    //negative for parameters; allot space at top for the return value
    //currently, only scalar; determines start position of first arg "under".

    //maxdepth now re-calculated after parsing due to some still unknown sizes;
    m_state.m_parsingVariableSymbolTypeFlag = STF_FUNCPARAMETER;

    //create "self" symbol for the class type; btw, it's really a ref.
    //belongs to the function definition scope.
    UTI cuti = currClassBlock->getNodeType(); //luckily we know this now for each class used
    assert(m_state.okUTItoContinue(cuti));
    Token selfTok(TOK_KW_SELF, identTok.m_locator, 0);
    SymbolVariableStack * selfsym = new SymbolVariableStack(selfTok, m_state.getUlamTypeAsRef(cuti, ALT_REF), m_state);
    selfsym->setAutoLocalType(ALT_REF);
    selfsym->setIsSelf();
    m_state.addSymbolToCurrentScope(selfsym); //ownership goes to the funcdef block

    //wait until c&l to create "super" symbol for the Super class type (t41162);
    //btw, it's really a ref. belongs to the function definition scope.

    //parse and add parameters to function symbol (not in ST yet!)
    //intentionally keeps going regardless of errors for better messages.
    parseRestOfFunctionParameters(fsymptr, rtnNode);

    if(rtnNode)
      {
	u32 numParams = rtnNode->getNumberOfParameters();
	if(isConstr && (numParams == 0))
	  {
	    std::ostringstream msg;
	    msg << "Default Constructor not allowed";
	    MSG(&args.m_typeTok, msg.str().c_str(), ERR);
	    delete fsymptr; //also deletes the NodeBlockFunctionDefinition
	    rtnNode = NULL;
	  }
	else if(isOperatorOverload && (numParams > 1))
	  {
	    u32 ulamnameid = identTok.getUlamNameIdForOperatorOverloadToken(&m_state);
	    std::ostringstream msg;
	    msg << "Operator overload function definition: ";
	    msg << m_state.m_pool.getDataAsString(ulamnameid).c_str();
	    msg << " must take none or 1 argument, not " << numParams; //C says "zero or.."
	    MSG(&identTok, msg.str().c_str(), ERR); //t41107, t41113
	    delete fsymptr; //also deletes the NodeBlockFunctionDefinition
	    rtnNode = NULL;
	  }
	else
	  {
	    //transfers ownership, if added
	    SymbolFunction * anyotherSym = NULL;
	    bool isAdded = ((SymbolFunctionName *) fnSym)->overloadFunction(fsymptr, anyotherSym);
	    if(!isAdded)
	      {
		//this is a duplicate function definition with same parameters and given name!!
		//return types may differ
		std::ostringstream msg;
		msg << "Duplicate defined function '";
		msg << m_state.m_pool.getDataAsString(fsymptr->getFunctionNameId()); //t41546 op+
		msg << "' with the same parameters." ;
		MSG(&args.m_typeTok, msg.str().c_str(), ERR);

		assert(anyotherSym);
		std::ostringstream note;
		note << anyotherSym->getFunctionNameWithTypes().c_str(); //t41545
		MSG(m_state.getFullLocationAsString(anyotherSym->getLoc()).c_str(), note.str().c_str(), NOTE);

		if(UlamType::compare(fsymptr->getUlamTypeIdx(), anyotherSym->getUlamTypeIdx(), m_state) != UTIC_SAME)
		  {
		    std::ostringstream note2;
		    note2 << "(even when return types differ)";
		    MSG(&args.m_typeTok, note2.str().c_str(), NOTE);
		  }

		delete fsymptr; //also deletes the NodeBlockFunctionDefinition
		rtnNode = NULL;
	      }
	  }
      }

    if(rtnNode)
      {
	//starts with positive one for local variables
	m_state.m_parsingVariableSymbolTypeFlag = STF_FUNCLOCALVAR;

	//parse body definition
	if(parseFunctionBody(rtnNode))
	  {
	    rtnNode->setDefinition();
	    if(fsymptr->takesVariableArgs() && !rtnNode->isNative())
	      {
		fsymptr->markForVariableArgs(false);
		std::ostringstream msg;
		msg << "Variable args (...) supported for native functions only; not '";
		msg << m_state.m_pool.getDataAsString(fsymptr->getFunctionNameId()).c_str() << "'";
		MSG(rtnNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    fsymptr->setFunctionNode((NodeBlockFunctionDefinition *) NULL); //deletes node
	    rtnNode = NULL;
	  }
      }

    //this block's ST is no longer in scope
    m_state.popClassContext(); //= prevBlock;
    m_state.m_parsingVariableSymbolTypeFlag = STF_NEEDSATYPE;
    m_state.m_parsingFUNCid = 0;
    return rtnNode;
  } //makeFunctionBlock

  void Parser::parseRestOfFunctionParameters(SymbolFunction * fsym, NodeBlockFunctionDefinition * fblock)
  {
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      return; //done with parameters

    assert(fsym);

    //allows function name to be same as arg name since function starts a new "block" (ST);
    //the arg to parseDecl will prevent it from looking for restofdecls (i.e. singledecl);
    //ellipsis currently only for natives (detected after args done)
    if(pTok.m_type == TOK_ELLIPSIS)
      {
	if(!fsym->takesVariableArgs())
	  {
	    fsym->markForVariableArgs();
	  }
	else
	  {
	    MSG(&pTok, "Variable args (...) indicated multiple times", ERR);
	  }
      }
    else if(Token::isTokenAType(pTok) || (pTok.m_type == TOK_KW_LOCALDEF) || (pTok.m_type == TOK_KW_CONSTDEF))
      {
	//local.Type allowed (t3870,71)
	unreadToken();
	NodeVarDecl * argNode = parseFunctionParameterDecl();

	//could be null symbol already in scope
	if(argNode)
	  {
	    //parameter IS a variable (declaration).
	    //ownership stays with NodeBlockFunctionDefinition's ST
	    fblock->addParameterNode(argNode); //transfer owner

	    if(fsym->takesVariableArgs())
	      {
		std::ostringstream msg;
		msg << "Parameter '";
		msg << m_state.m_pool.getDataAsString(argNode->getNameId()).c_str();
		msg << "' appears after ellipses (...)";
		MSG(&pTok, msg.str().c_str(), ERR);
	      }
	  } //else error given
      }
    else if(pTok.m_type == TOK_EQUAL)
      {
	//special reminder (t3990)
	MSG(&pTok, "Default values for function parameters is currently not supported", ERR);
      }
    else
      {
	std::ostringstream msg;
	msg << "Expected 'A Type' Token!! got Token <";
	msg << m_state.getTokenDataAsString(pTok).c_str();
	msg << "> instead";
	MSG(&pTok, msg.str().c_str(), ERR);
	return; //e.g. TOK_EOF caused infinite loop (t3983)
      }

    if(getExpectedToken(TOK_COMMA)) //if so, get next parameter; o.w. unread
      {
	if(fsym->takesVariableArgs())
	  MSG(&pTok, "Variable args indication (...) appears at end of parameter list", ERR);
      }
    return parseRestOfFunctionParameters(fsym, fblock);
  } //parseRestOfFunctionParameters

  //similar to parseDecl, except singletons and no assignments
  NodeVarDecl * Parser::parseFunctionParameterDecl()
  {
    NodeVarDecl * rtnNode = NULL;
    TypeArgs typeargs;
    NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, false, false);

    Token iTok;
    getNextToken(iTok);

    if(typeNode == NULL)
      {
	std::ostringstream msg;
	msg << "Invalid Type for function parameter";
	MSG(&iTok, msg.str().c_str(), ERR);
	unreadToken();
	return NULL;
      }

    typeargs.m_isStmt = false; //for function params (e.g. refs)

    if(iTok.m_type == TOK_IDENTIFIER)
      {
	rtnNode = makeVariableSymbol(typeargs, iTok, typeNode);
      }
    else
      {
	//user error!
	std::ostringstream msg;
	msg << "Function parameter '" << m_state.getTokenDataAsString(iTok).c_str();
	if((Token::getSpecialTokenWork(iTok.m_type) == TOKSP_KEYWORD))
	  msg << "': Name must not be a reserved keyword";
	else
	  msg << "': Name must begin with a lower-case letter";
	MSG(&iTok, msg.str().c_str(), ERR);
	//unreadToken();
	delete typeNode;
	typeNode = NULL;
      }
    return rtnNode;
  } //parseFunctionParameterDecl

  bool Parser::parseFunctionBody(NodeBlockFunctionDefinition *& funcNode)
  {
    bool brtn = false;
    //if next token is { this is a definition; o.w. a declaration, alone invalid
    //however, keyword 'native' is exception.
    Token qTok;
    getNextToken(qTok);

    if(qTok.m_type == TOK_OPEN_CURLY)
      {
	brtn = true;
	//build definition!! (not a new block)
	Token pTok;
	getNextToken(pTok);
	unreadToken();

	if(pTok.m_type == TOK_CLOSE_CURLY)
	  {
	    NodeStatements * nextNode = new NodeBlockEmpty(m_state.getCurrentBlock(), m_state); //legal
	    assert(nextNode);
	    nextNode->setNodeLocation(pTok.m_locator);
	    funcNode->setNextNode(nextNode);
	  }
	else
	  parseStatements();

	if(!getExpectedToken(TOK_CLOSE_CURLY))
	  {
	    MSG(&pTok, "Not close curly", ERR);
	    brtn = false; //missing, need a pop?
	  }
	else
	  unreadToken();
      }
    else if(qTok.m_type == TOK_KW_NATIVE)
      {
	NodeStatements * nextNode;
	nextNode = new NodeBlockEmpty(m_state.getCurrentBlock(), m_state); //legal
	assert(nextNode);
	nextNode->setNodeLocation(qTok.m_locator);
	funcNode->setNextNode(nextNode);

	funcNode->setNative();

	//output messes up test answer comparison
	std::ostringstream msg;
	msg << "Native Function Definition <" << funcNode->getName() << ">";
	MSG(&qTok, msg.str().c_str(), INFO);

	brtn = true;
      }
    else if(qTok.m_type == TOK_SEMICOLON)
      {
	SymbolFunction * fsymFromDef = funcNode->getFuncSymbolPtr();
	assert(fsymFromDef);
	if(fsymFromDef->isVirtualFunction())
	  {
	    NodeStatements * nextNode;
	    nextNode = new NodeBlockEmpty(m_state.getCurrentBlock(), m_state); //legal
	    assert(nextNode);
	    nextNode->setNodeLocation(qTok.m_locator);
	    funcNode->setNextNode(nextNode);

	    fsymFromDef->setPureVirtualFunction();

	    std::ostringstream msg;
	    msg << "Pure Virtual Function '" << funcNode->getName() << "'";
	    MSG(&qTok, msg.str().c_str(), INFO);

	    unreadToken();
	    brtn = true;
	  }
	else
	  {
	    unreadToken();
	    std::ostringstream msg;
	    msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(qTok).c_str();
	    msg << "> after non-virtual function declaration";
	    MSG(&qTok, msg.str().c_str(), ERR);
	  }
      }
    else
      {
	unreadToken();
	std::ostringstream msg;
	msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(qTok).c_str();
	msg << "> after function declaration";
	MSG(&qTok, msg.str().c_str(), ERR);
      }
    return brtn;
  } //parseFunctionBody

  NodeBlockFunctionDefinition * Parser::makeFunctionSymbol(TypeArgs& args, const Token& identTok, NodeTypeDescriptor * nodetype, bool isVirtual, bool insureVirtualOverride, bool isConstr)
  {
    //first check that the function name begins with a lower case letter
    if(!isConstr && Token::isTokenAType(identTok))
      {
	std::ostringstream msg;
	msg << "Function '" << m_state.getTokenDataAsString(identTok).c_str();
	msg << "' is not a valid (lower case) name";
	MSG(&identTok, msg.str().c_str(), ERR);

	//eat tokens until end of definition ?
	delete nodetype;
	return NULL;
      }

    Symbol * asymptr = NULL;
    //ask current scope class block if this identifier name is there (no embedded funcs)
    //(checks functions and variables and typedefs); if not a function, BAIL;
    //check for overloaded function, after parameter types available
    NodeBlockContext * cblock = m_state.getContextBlock();
    assert(cblock && cblock->isAClassBlock());
    NodeBlockClass * currClassBlock = (NodeBlockClass *) cblock;

    if(!isConstr && currClassBlock->isIdInScope(identTok.m_dataindex,asymptr) && !asymptr->isFunction())
      {
	UTI auti = asymptr->getUlamTypeIdx();

	std::ostringstream msg;
	msg << "'" << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
	msg << "' cannot be used again as a function, it has a previous definition";

	if(m_state.okUTItoContinue(auti) && !m_state.isHolder(auti))
	  {
	    msg << " of type '";
	    msg << m_state.getUlamTypeNameByIndex(auti).c_str();
	    msg << "'";
	  }
	//else

	msg << " at: ."; //..
	MSG(&args.m_typeTok, msg.str().c_str(), ERR);

	std::ostringstream imsg;
	imsg << ".. this location";
	MSG(m_state.getFullLocationAsString(asymptr->getLoc()).c_str(), imsg.str().c_str(), ERR);
      } //keep going..more errors (t3284)

    //not in scope, or not yet defined, or possible overloading
    //o.w. build symbol for function: return type + name + parameter symbols
    NodeBlockFunctionDefinition * rtnNode = makeFunctionBlock(args, identTok, nodetype, isVirtual, insureVirtualOverride, isConstr);

    //exclude the declaration & definition from the parse tree
    //(since in SymbolFunction) & return NULL.
    Token qTok;
    getNextToken(qTok);

    if(rtnNode)
      {
	if((qTok.m_type != TOK_CLOSE_CURLY) && (rtnNode->isNative() && qTok.m_type != TOK_SEMICOLON))
	  {
	    //first remove the pointer to this node from its symbol
	    rtnNode->getFuncSymbolPtr()->setFunctionNode((NodeBlockFunctionDefinition *) NULL); //deletes node
	    rtnNode = NULL;
	    MSG(&qTok, "INCOMPLETE Function Definition", ERR);
	  }
      }
    return rtnNode;
  } //makeFunctionSymbol

  NodeVarDecl * Parser::makeVariableSymbol(TypeArgs& args, const Token& identTok, NodeTypeDescriptor *& nodetyperef)
  {
    assert(!Token::isTokenAType(identTok)); //capitalization check done by Lexer

    NodeVarDecl * rtnNode = NULL;
    Node * lvalNode = parseLvalExpr(identTok); //for optional [] array size
    if(lvalNode)
      {
	//lvalNode could be either a NodeIdent, a NodeSqBracket, or NodeSqBracket subtree
	//process identifier...check if already defined in current scope; if not, add it;
	//returned symbol could be symbolVariable or symbolFunction, detect first.
	Symbol * asymptr = NULL;
	if(!lvalNode->installSymbolVariable(args, asymptr))
	  {
	    if(asymptr)
	      {
		UTI auti = asymptr->getUlamTypeIdx();

		std::ostringstream msg;
		msg << "'" << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
		msg << "' cannot be a variable because it has a previous declaration";

		if(m_state.okUTItoContinue(auti) && !m_state.isHolder(auti))
		  {
		    msg << " of type '";
		    msg << m_state.getUlamTypeNameByIndex(auti).c_str();
		    msg << "'";
		  }
		//else

		msg << " at: ."; //..
		MSG(&args.m_typeTok, msg.str().c_str(), ERR); //t3129

		std::ostringstream imsg;
		imsg << ".. this location";
		MSG(m_state.getFullLocationAsString(asymptr->getLoc()).c_str(), imsg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (e.g. problem with []);
		// rtnNode is NULL; (t3255, t41155)
		std::ostringstream msg;
		msg << "Invalid variable declaration of type <";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str() << "> and name '";
		msg << m_state.getTokenDataAsString(identTok).c_str() << "' (missing symbol)";
		MSG(&args.m_typeTok, msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    UTI auti = asymptr->getUlamTypeIdx();

	    //chain to NodeType descriptor if array (i.e. non scalar), o.w. delete lval
	    linkOrFreeConstantExpressionArraysize(auti, args, (NodeSquareBracket *)lvalNode, nodetyperef);

	    syncTypeDescriptorWithSymbolType(auti, args, nodetyperef); //t3611,t3613,t3816 might change nodetyperef

	    // tfr owner of node type descriptor to node var decl
	    if(asymptr->isDataMember())
	      {
		rtnNode =  new NodeVarDeclDM((SymbolVariableDataMember *) asymptr, nodetyperef, m_state);
		asymptr->setStructuredComment(); //also clears
	      }
	    else if(asymptr->isAutoLocal())
	      {
		rtnNode =  new NodeVarRef((SymbolVariable *) asymptr, nodetyperef, m_state);
		m_state.clearStructuredCommentToken();
	      }
	    else
	      {
		rtnNode =  new NodeVarDecl((SymbolVariable *) asymptr, nodetyperef, m_state);
		m_state.clearStructuredCommentToken();
	      }
	    assert(rtnNode);
	    rtnNode->setNodeLocation(args.m_typeTok.m_locator);

	    //in case of decl list, set type of symbol in args ref
	    // scalar types return their own uti, not a new one.
	    assert(asymptr);
	    if((args.m_declListOrTypedefScalarType == Nouti) && (args.m_anothertduti == Nouti))
	      args.m_declListOrTypedefScalarType = m_state.getUlamTypeAsScalar(asymptr->getUlamTypeIdx());
	  }

	if(rtnNode == NULL)
	  {
	    delete lvalNode; //done with it
	    delete nodetyperef;
	    nodetyperef = NULL;
	  }
      }
    else
      {
	delete nodetyperef;
	nodetyperef = NULL;
      }
    return rtnNode;
  } //makeVariableSymbol

  NodeTypedef * Parser::makeTypedefSymbol(TypeArgs& args, const Token& identTok, NodeTypeDescriptor *& nodetyperef)
  {
    NodeTypedef * rtnNode = NULL;
    Node * lvalNode = parseLvalExpr(identTok);
    if(lvalNode)
      {
	//lvalNode could be either a NodeIdent or a NodeSquareBracket
	//process identifier...check if already defined in current scope; if not, add it;
	//returned symbol could be symbolVariable or symbolFunction, detect first.
	Symbol * asymptr = NULL;
	bool aok = lvalNode->installSymbolTypedef(args, asymptr);
	if(!aok)
	  {
	    if(asymptr)
	      {
		u32 asymid = asymptr->getId(); //Self and Super no longer exceptions
		UTI auti = asymptr->getUlamTypeIdx();
		std::ostringstream msg;
		msg << "'" << m_state.m_pool.getDataAsString(asymid).c_str();
		msg << "' cannot be a typedef because it is already declared";

		if(m_state.okUTItoContinue(auti) && !m_state.isHolder(auti))
		  {
		    msg << " as type '";
		    msg << m_state.getUlamTypeNameBriefByIndex(auti).c_str();
		  }

		msg << " at: ."; //..
		MSG(&args.m_typeTok, msg.str().c_str(), ERR); //t3698, t3391

		std::ostringstream imsg;
		imsg << ".. this location";
		MSG(m_state.getFullLocationAsString(asymptr->getLoc()).c_str(), imsg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (t3254)
		//(e.g. problem with []) , error already output. rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid typedef of base type <";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str();
		msg << "> and name '" << m_state.getTokenDataAsString(identTok).c_str();
		msg << "' (missing symbol)";
		MSG(&identTok, msg.str().c_str(), ERR);
	      }
	    m_state.clearStructuredCommentToken();
	  }

	if(aok)
	  {
	    UTI auti = asymptr->getUlamTypeIdx();

	    //chain to NodeType descriptor if array (i.e. non scalar), o.w. delete lval
	    linkOrFreeConstantExpressionArraysize(auti, args, (NodeSquareBracket *)lvalNode, nodetyperef);

	    syncTypeDescriptorWithSymbolType(auti, args, nodetyperef);//t3378 skips, t3668

	    // tfr owner of nodetyperef to node typedef
	    rtnNode =  new NodeTypedef((SymbolTypedef *) asymptr, nodetyperef, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(args.m_typeTok.m_locator);
	    asymptr->setStructuredComment(); //also clears
	    ((SymbolTypedef *) asymptr)->clearCulamGeneratedTypedef(); //for locals scope
	    //m_state.resetUnseenClass(asymptr->getId()); //ulamexports
	  }

	if(rtnNode == NULL)
	  {
	    delete lvalNode; //done with it
	    delete nodetyperef;
	    nodetyperef = NULL;
	  }
      }
    else
      {
	delete nodetyperef;
	nodetyperef = NULL;
	m_state.clearStructuredCommentToken();
      }
    return rtnNode;
  } //makeTypedefSymbol

  NodeConstantDef * Parser::makeConstdefSymbol(TypeArgs& args, const Token& identTok, NodeTypeDescriptor *& nodetyperef)
  {
    NodeConstantDef * rtnNode = NULL;
    Node * lvalNode = parseIdentExpr(identTok); //calls parseLvalExpr
    if(lvalNode)
      {
	//lvalNode could be either a NodeIdent or a NodeSquareBracket,
	// though arrays not legal in this context!!!
	//process identifier...check if already defined in current scope; if not, add it;
	//return a SymbolConstantValue else some sort of primitive
	Symbol * asymptr = NULL;
	if(!lvalNode->installSymbolConstantValue(args, asymptr))
	  {
	    if(asymptr != NULL)
	      {
		UTI auti = asymptr->getUlamTypeIdx();
		std::ostringstream msg;
		msg << "'" << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
		msg << "' cannot be a named constant because it is already declared";

		if(m_state.okUTItoContinue(auti) && !m_state.isHolder(auti))
		  {
		    msg << " as type '";
		    msg << m_state.getUlamTypeNameByIndex(auti).c_str(); //t41463
		    msg << "'";
		  }
		//else

		msg << " at: ."; //..
		MSG(&args.m_typeTok, msg.str().c_str(), ERR); //t41130,t3872,t41163

		std::ostringstream imsg;
		imsg << ".. this location";
		MSG(m_state.getFullLocationAsString(asymptr->getLoc()).c_str(), imsg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (e.g. problem with []),
		//error already output. rtnNode is NULL; (t3446, t3451, t3460)
		std::ostringstream msg;
		msg << "Invalid constant definition of type <";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str();
		msg << "> and name '" << m_state.getTokenDataAsString(identTok).c_str() << "'";
		MSG(&identTok, msg.str().c_str(), ERR);
	      }

	    delete lvalNode; //done with it
	    delete nodetyperef;
	    nodetyperef = NULL;

	    //perhaps read until semi-colon
	    if(args.m_isStmt)
	      {
		getTokensUntil(TOK_SEMICOLON);
		unreadToken();
	      }
	    //else class parameter list
	    m_state.clearStructuredCommentToken();
	  }
	else
	  {
	    UTI auti = asymptr->getUlamTypeIdx();

	    //chain to NodeType descriptor if array (i.e. non scalar), o.w. deletes lval
	    linkOrFreeConstantExpressionArraysize(auti, args, (NodeSquareBracket *)lvalNode, nodetyperef);
	    syncTypeDescriptorWithSymbolType(auti, args, nodetyperef);

	    //tfr owner of node type descriptor to nodeconstantdef
	    NodeConstantDef * constNode =  new NodeConstantDef((SymbolConstantValue *) asymptr, nodetyperef, m_state);
	    assert(constNode);
	    constNode->setNodeLocation(args.m_typeTok.m_locator);
	    if(args.m_isStmt)
	      asymptr->setStructuredComment(); //also clears

	    if(m_state.m_parsingVariableSymbolTypeFlag == STF_MEMBERCONSTANT)
	      asymptr->setDataMemberClass(m_state.getCompileThisIdx()); //t3862, t3865, t3868

	    rtnNode = parseRestOfConstantDef(constNode, args.m_assignOK, args.m_isStmt);

	    if(rtnNode == NULL)
	      m_state.clearStructuredCommentToken(); //t3524
	    else
	      {
		if(m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSPARAMETER)
		  {
		    ((SymbolConstantValue *) asymptr)->setClassParameterFlag();
		    if(((NodeConstantDef *) rtnNode)->hasConstantExpr()) //before any folding
		      ((SymbolWithValue *) asymptr)->setHasInitValue(); //default value
		  }
	      }
	  }
      }
    else
      {
	delete nodetyperef;
	nodetyperef = NULL;
	m_state.clearStructuredCommentToken();
      }
    return rtnNode;
  } //makeConstdefSymbol

  NodeModelParameterDef * Parser::makeModelParameterSymbol(TypeArgs& args, const Token& identTok, NodeTypeDescriptor *& nodetyperef)
  {
    NodeModelParameterDef * rtnNode = NULL;
    Node * lvalNode = parseIdentExpr(identTok); //calls parseLvalExpr
    if(lvalNode)
      {
	//lvalNode could be either a NodeIdent or a NodeSquareBracket,
	// though arrays not legal in this context!!!
	//process identifier...check if already defined in current scope; if not, add it;
	//return a SymbolModelParameterValue else some sort of primitive
	Symbol * asymptr = NULL;
	if(!lvalNode->installSymbolModelParameterValue(args, asymptr))
	  {
	    if(asymptr != NULL)
	      {
		UTI auti = asymptr->getUlamTypeIdx();

		std::ostringstream msg;
		msg << "'" << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
		msg << "' cannot be a Model Parameter data member because it is already declared as ";

		if(m_state.okUTItoContinue(auti) && !m_state.isHolder(auti))
		  {
		    msg << " as type '";
		    msg << m_state.getUlamTypeNameByIndex(auti).c_str();
		    msg << "'";
		  }
		//else

		msg << " at: ."; //..
		MSG(&args.m_typeTok, msg.str().c_str(), ERR);

		std::ostringstream imsg;
		imsg << ".. this location";
		MSG(m_state.getFullLocationAsString(asymptr->getLoc()).c_str(), imsg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (e.g. problem with []),
		//error already output; rtnNode is NULL.
		std::ostringstream msg;
		msg << "Invalid Model Parameter: ";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str();
		msg << " " << m_state.getTokenDataAsString(identTok).c_str();
		MSG(&identTok, msg.str().c_str(), ERR);
	      }

	    delete lvalNode; //done with it
	    delete nodetyperef;
	    nodetyperef = NULL;

	    if(args.m_assignOK)
	      {
		getTokensUntil(TOK_SEMICOLON);
		unreadToken();
	      }

	    m_state.clearStructuredCommentToken();
	  }
	else
	  {
	    UTI auti = asymptr->getUlamTypeIdx();
	    //chain to NodeType descriptor if array (i.e. non scalar), o.w. deletes lval
	    linkOrFreeConstantExpressionArraysize(auti, args, (NodeSquareBracket *)lvalNode, nodetyperef);

	    NodeModelParameterDef * paramNode =  new NodeModelParameterDef((SymbolModelParameterValue *) asymptr, nodetyperef, m_state);
	    assert(paramNode);
	    paramNode->setNodeLocation(args.m_typeTok.m_locator);
	    asymptr->setStructuredComment(); //also clears

	    rtnNode = parseRestOfParameterDef(paramNode);
	  }
      }
    else
      {
	delete nodetyperef;
	nodetyperef = NULL;
	m_state.clearStructuredCommentToken();
      }
    return rtnNode;
  } //makeModelParameterSymbol

  Node * Parser::makeConditionalExprNode(Node * leftNode)
  {
    Node * rtnNode = NULL;

    Token saveIdentTok = m_state.m_identTokenForConditionalAs;

    //reparse (is-has-as) function token
    Token fTok;
    getNextToken(fTok);

    //parse Type
    Token tTok;
    getNextToken(tTok);
    unreadToken();

    TypeArgs typeargs;

    //allow types with preceding dots (e.g. from another class)
    //t3407, 3826,27,30; t3868,61,62
    NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, true, false);

    if(typeNode == NULL)
      {
	std::ostringstream msg;
	msg << "Right operand of conditional-" << fTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is not a valid Type: ";
	msg << tTok.getTokenStringFromPool(&m_state).c_str() << "; Operation deleted";
	MSG(&tTok, msg.str().c_str(), ERR);
	delete leftNode;
	m_state.m_parsingConditionalAs = false;
	return NULL;
      }

    UTI cuti = typeNode->givenUTI();
    typeargs.setdeclref(fTok, cuti);

    //consider possibility of Class Instance as Type (future work)
    if(!m_state.isScalar(cuti))
      {
	std::ostringstream msg;
	msg << "Right operand of conditional-" << fTok.getTokenStringFromPool(&m_state).c_str() << " is an array: ";
	msg << tTok.getTokenStringFromPool(&m_state).c_str() << "; Operation deleted";
	MSG(&tTok, msg.str().c_str(), ERR);
	delete leftNode;
	delete typeNode;
	m_state.m_parsingConditionalAs = false;
	return NULL;
      }

    switch(fTok.m_type)
      {
      case TOK_KW_IS:
	rtnNode = new NodeConditionalIs(leftNode, typeNode, m_state);
	break;
      case TOK_KW_AS:
	rtnNode = new NodeConditionalAs(leftNode, typeNode, m_state);
	break;
      default:
	abortUnexpectedToken(fTok);
	break;
      };
    assert(rtnNode);
    rtnNode->setNodeLocation(fTok.m_locator);

    //restore before calling setupAsConditionalBlockAndParseStatements (t3336, t41043)
    m_state.m_identTokenForConditionalAs = saveIdentTok;

    return rtnNode;
  } //makeConditionalExprNode

  NodeBinaryOpEqual * Parser::makeAssignExprNode(Node * leftNode)
  {
    NodeBinaryOpEqual * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok); //some flavor of equal token

    //post increment/decrement
    if(pTok.m_type == TOK_PLUS_PLUS)
      {
	rtnNode = new NodeBinaryOpEqualArithPostIncr(leftNode, makeTerminal(pTok, (s64) 1, Int), m_state);
	rtnNode->setNodeLocation(pTok.m_locator);
	return rtnNode;
      }

    if(pTok.m_type == TOK_MINUS_MINUS)
      {
	rtnNode = new NodeBinaryOpEqualArithPostDecr(leftNode, makeTerminal(pTok, (s64) 1, Int), m_state);
	rtnNode->setNodeLocation(pTok.m_locator);
	return rtnNode;
      }

    Node * rightNode = parseAssignExpr(); //t3136 y = x = times(4,5); not parseExpression.
    if(rightNode == NULL)
      {
	std::ostringstream msg;
	msg << "Right operand of binary " << pTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is missing; Assignment deleted";
	MSG(&pTok, msg.str().c_str(), ERR);
	delete leftNode;
      }
    else
      {
	switch(pTok.m_type)
	  {
	  case TOK_EQUAL:
	    rtnNode = new NodeBinaryOpEqual(leftNode, rightNode, m_state);
	    break;
	  case TOK_PLUS_EQUAL:
	    rtnNode = new NodeBinaryOpEqualArithAdd(leftNode, rightNode, m_state);
	    break;
	  case TOK_MINUS_EQUAL:
	    rtnNode = new NodeBinaryOpEqualArithSubtract(leftNode, rightNode, m_state);
	    break;
	  case TOK_STAR_EQUAL:
	    rtnNode = new NodeBinaryOpEqualArithMultiply(leftNode, rightNode, m_state);
	    break;
	  case TOK_SLASH_EQUAL:
	    rtnNode = new NodeBinaryOpEqualArithDivide(leftNode, rightNode, m_state);
	    break;
	  case TOK_PERCENTSIGN_EQUAL:
	    rtnNode = new NodeBinaryOpEqualArithRemainder(leftNode, rightNode, m_state);
	    break;
	  case TOK_AMP_EQUAL:
	    rtnNode = new NodeBinaryOpEqualBitwiseAnd(leftNode, rightNode, m_state);
	    break;
	  case TOK_PIPE_EQUAL:
	    rtnNode = new NodeBinaryOpEqualBitwiseOr(leftNode, rightNode, m_state);
	    break;
	  case TOK_HAT_EQUAL:
	    rtnNode = new NodeBinaryOpEqualBitwiseXor(leftNode, rightNode, m_state);
	    break;
	  case TOK_SHIFT_LEFT_EQUAL:
	    rtnNode = new NodeBinaryOpEqualShiftLeft(leftNode, rightNode, m_state);
	    break;
	  case TOK_SHIFT_RIGHT_EQUAL:
	    rtnNode = new NodeBinaryOpEqualShiftRight(leftNode, rightNode, m_state);
	    break;
	  default:
	    abortUnexpectedToken(pTok);
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeAssignExprNode

  Node * Parser::makeQuestionColonNode(Node * condNode)
  {
    Token pTok;
    if(!getExpectedToken(TOK_QUESTION, pTok, QUIETLY))
      {
	m_state.abortShouldntGetHere();
	delete condNode;
	return NULL;
      }

    Node * trueNode = parseExpression();
    if(trueNode == NULL)
      {
	std::ostringstream msg;
	msg << "Incomplete true expression; question-colon-control";
	MSG(&pTok, msg.str().c_str(), ERR);
	delete condNode;
	return NULL; //stop this maddness
      }

    Token tmpTok; //non quietly, like t41293
    if(!getExpectedToken(TOK_COLON, tmpTok))
      {
	delete condNode;
	delete trueNode;
	return NULL;
      }

    Node * falseNode = parseExpression();
    if(falseNode == NULL)
      {
	std::ostringstream msg;
	msg << "Incomplete false expression; question-colon-control";
	MSG(&pTok, msg.str().c_str(), ERR);
	delete condNode;
	delete trueNode;
	return NULL; //stop this maddness
      }


    NodeQuestionColon * rtnNode = new NodeQuestionColon(condNode, trueNode, falseNode, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(pTok.m_locator);

    return rtnNode;
  } //makeQuestionColonNode

  NodeBinaryOp * Parser::makeExpressionNode(Node * leftNode)
  {
    NodeBinaryOp * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * rightNode = parseLogicalExpression();
    if(rightNode == NULL)
      {
	std::ostringstream msg;
	msg << "Right operand of binary " << pTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is missing; Operation deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	delete leftNode;
      }
    else
      {
	switch(pTok.m_type)
	  {
	  case TOK_AMP_AMP:
	    rtnNode = new NodeBinaryOpLogicalAnd(leftNode, rightNode, m_state);
	    break;
	  case TOK_PIPE_PIPE:
	    rtnNode = new NodeBinaryOpLogicalOr(leftNode, rightNode, m_state);
	    break;
	  default:
	    abortUnexpectedToken(pTok);
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeExpressionNode

  NodeBinaryOp * Parser::makeLogicalExpressionNode(Node * leftNode)
  {
    NodeBinaryOp * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * rightNode = parseBitExpression();
    if(rightNode == NULL)
      {
	std::ostringstream msg;
	msg << "Right operand of binary " << pTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is missing; Operation deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	delete leftNode;
      }
    else
      {
	switch(pTok.m_type)
	  {
	  case TOK_AMP:
	    rtnNode = new NodeBinaryOpBitwiseAnd(leftNode, rightNode, m_state);
	    break;
	  case TOK_PIPE:
	    rtnNode = new NodeBinaryOpBitwiseOr(leftNode, rightNode, m_state);
	    break;
	  case TOK_HAT:
	    rtnNode = new NodeBinaryOpBitwiseXor(leftNode, rightNode, m_state);
	    break;
	  default:
	    abortUnexpectedToken(pTok);
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeLogicalExpressionNode

  NodeBinaryOp * Parser::makeBitExpressionNode(Node * leftNode)
  {
    NodeBinaryOp * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * rightNode = parseEqExpression();
    if(rightNode == NULL)
      {
	std::ostringstream msg;
	msg << "Right operand of binary " << pTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is missing; Operation deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	delete leftNode;
      }
    else
      {
	switch(pTok.m_type)
	  {
	  case TOK_EQUAL_EQUAL:
	    rtnNode = new NodeBinaryOpCompareEqualEqual(leftNode, rightNode, m_state);
	    break;
	  case TOK_NOT_EQUAL:
	    rtnNode = new NodeBinaryOpCompareNotEqual(leftNode, rightNode, m_state);
	    break;
	  default:
	    abortUnexpectedToken(pTok);
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeBitExpressionNode

  NodeBinaryOp * Parser::makeEqExpressionNode(Node * leftNode)
  {
    NodeBinaryOp * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * rightNode = parseCompareExpression();
    if(rightNode == NULL)
      {
	std::ostringstream msg;
	msg << "Right operand of binary " << pTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is missing; Operation deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	delete leftNode;
      }
    else
      {
	switch(pTok.m_type)
	  {
	  case TOK_LESS_THAN:
	    rtnNode = new NodeBinaryOpCompareLessThan(leftNode, rightNode, m_state);
	    break;
	  case TOK_GREATER_THAN:
	    rtnNode = new NodeBinaryOpCompareGreaterThan(leftNode, rightNode, m_state);
	    break;
	  case TOK_LESS_EQUAL:
	    rtnNode = new NodeBinaryOpCompareLessEqual(leftNode, rightNode, m_state);
	    break;
	  case TOK_GREATER_EQUAL:
	    rtnNode = new NodeBinaryOpCompareGreaterEqual(leftNode, rightNode, m_state);
	    break;
	  default:
	    abortUnexpectedToken(pTok);
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeEqExpressionNode

  NodeBinaryOp * Parser::makeCompareExpressionNode(Node * leftNode)
  {
    NodeBinaryOp * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * rightNode = parseShiftExpression();
    if(rightNode == NULL)
      {
	std::ostringstream msg;
	msg << "Right operand of binary " << pTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is missing; Operation deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	delete leftNode;
      }
    else
      {
	switch(pTok.m_type)
	  {
	  case TOK_SHIFT_LEFT:
	    rtnNode = new NodeBinaryOpShiftLeft(leftNode, rightNode, m_state);
	    break;
	  case TOK_SHIFT_RIGHT:
	    rtnNode = new NodeBinaryOpShiftRight(leftNode, rightNode, m_state);
	    break;
	  default:
	    abortUnexpectedToken(pTok);
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeCompareExpressionNode

  NodeBinaryOp * Parser::makeShiftExpressionNode(Node * leftNode)
  {
    NodeBinaryOp * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * rightNode = parseTerm();
    if(rightNode == NULL)
      {
	std::ostringstream msg;
	msg << "Right operand of binary " << pTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is missing; Operation deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	delete leftNode;
      }
    else
      {
	switch(pTok.m_type)
	  {
	  case TOK_PLUS:
	    rtnNode = new NodeBinaryOpArithAdd(leftNode, rightNode, m_state);
	    break;
	  case TOK_MINUS:
	    rtnNode = new NodeBinaryOpArithSubtract(leftNode, rightNode, m_state);
	    break;
	  default:
	    abortUnexpectedToken(pTok);
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeShiftExpressionNode

  NodeBinaryOp * Parser::makeTermNode(Node * leftNode)
  {
    NodeBinaryOp * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * rightNode = parseFactor();
    if(rightNode == NULL)
      {
	std::ostringstream msg;
	msg << "Right operand of binary " << pTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is missing; Operation deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	delete leftNode;
      }
    else
      {
	switch(pTok.m_type)
	  {
	  case TOK_STAR:
	    rtnNode = new NodeBinaryOpArithMultiply(leftNode, rightNode, m_state);
	    break;
	  case TOK_SLASH:
	    rtnNode = new NodeBinaryOpArithDivide(leftNode, rightNode, m_state);
	    break;
	  case TOK_PERCENTSIGN:
	    rtnNode = new NodeBinaryOpArithRemainder(leftNode, rightNode, m_state);
	    break;
	  default:
	    abortUnexpectedToken(pTok);
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeTermNode

  Node * Parser::makeFactorNodePreUnaryOp()
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * factorNode = parseFactor();
    if(factorNode == NULL)
      {
	std::ostringstream msg;
	msg << "Factor is missing; Unary operation ";
	msg << pTok.getTokenStringFromPool(&m_state).c_str() << " deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	return NULL;
      }

    switch(pTok.m_type)
      {
      case TOK_MINUS:
	//3849, 3767 constantFoldAToken was premature (e.g. cast involved)
	rtnNode = new NodeUnaryOpMinus(factorNode, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      case TOK_PLUS:
	rtnNode = new NodeUnaryOpPlus(factorNode, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      case TOK_BANG:
	rtnNode = new NodeUnaryOpBang(factorNode, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      case TOK_TWIDDLE:
	rtnNode = new NodeUnaryOpTwiddle(factorNode, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      case TOK_PLUS_PLUS:
	rtnNode = new NodeBinaryOpEqualArithPreIncr(factorNode, makeTerminal(pTok, (s64) 1, Int), m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      case TOK_MINUS_MINUS:
	rtnNode = new NodeBinaryOpEqualArithPreDecr(factorNode, makeTerminal(pTok, (s64) 1, Int), m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      default:
	abortUnexpectedToken(pTok);
	break;
      };

    return rtnNode;
  } //makeFactorNodePreUnaryOp

  bool Parser::makeCastNode(const Token& typeTok, bool allowRefCasts, NodeTypeDescriptor * typeNode, Node *& rtnNodeRef)
  {
    bool aok = true;
    Node * rtnNode = NULL;
    assert(typeNode);
    UTI typeToBe = typeNode->givenUTI();
    if((typeNode->getReferenceType() != ALT_NOT) && !allowRefCasts)
      {
	std::ostringstream msg;
	msg << "Explicit Reference casts (Type&) ";
	msg << "are valid for reference variable initialization";
	msg << " (including function call arguments and return statements); not in this context";
	MSG(&typeTok, msg.str().c_str(), ERR);
	return false; //t3791
      } //reference cast

    if(getExpectedToken(TOK_CLOSE_PAREN))
      {
	Node * nodetocast = parseFactor(); //not pparseExpression (t3158)
	if(nodetocast)
	  {
	    rtnNode = new NodeCast(nodetocast, typeToBe, typeNode, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(typeNode->getNodeLocation());
	    rtnNode->updateLineage(nodetocast->getYourParentNo());
	    ((NodeCast *) rtnNode)->setExplicitCast();
	  }
	else
	  aok = false;
      }
    else
      aok = false;

    rtnNodeRef = rtnNode;
    return aok;
  } //makeCastNode

  Node * Parser::makeTerminal(Token& locTok, s64 val, UTI utype)
  {
    Node * termNode = new NodeTerminal(val, utype, m_state);
    assert(termNode);
    termNode->setNodeLocation(locTok.m_locator);
    return termNode;
  } //makeTerminal

  Node * Parser::makeTerminal(Token& locTok, u64 val, UTI utype)
  {
    Node * termNode = new NodeTerminal(val, utype, m_state);
    assert(termNode);
    termNode->setNodeLocation(locTok.m_locator);
    return termNode;
  } //makeTerminal

  void Parser::linkOrFreeConstantExpressionArraysize(UTI auti, const TypeArgs& args, NodeSquareBracket * ceForArraySize, NodeTypeDescriptor *& nodetyperef)
  {
    //auti is incomplete.
    s32 arraysize = m_state.getArraySize(auti);

    //link arraysize subtree for arraytype based on scalar from another class, OR
    // a local arraytype based on a local scalar uti; o.w. delete.
    // don't keep the ceForArraySize if the type belongs to another class!
    // we link an array type to its scalar type

    if(!((arraysize == UNKNOWNSIZE) || (args.m_arraysize == UNKNOWNSIZE))) //t3172
      {
	delete ceForArraySize;
	return;
      }

    //typedef is an array from another class
    if((args.m_anothertduti != Nouti) && (args.m_anothertduti == auti))
      {
	assert(!m_state.isALocalsFileScope(auti));
	delete ceForArraySize;
	return;
      }

    assert(m_state.okUTItoContinue(nodetyperef->givenUTI()));

    // could be local array typedef, no square brackets this time (else)
    if(m_state.isScalar(nodetyperef->givenUTI()))
      {
	NodeTypeDescriptorArray * nodetypearray = new NodeTypeDescriptorArray(args.m_typeTok, auti, nodetyperef, m_state);
	assert(nodetypearray);
	nodetypearray->linkConstantExpressionArraysize(ceForArraySize);
	nodetyperef = nodetypearray;
      }
    else
      delete ceForArraySize;
  } //linkOrFreeConstantExpressionArraysize

  void Parser::syncTypeDescriptorWithSymbolType(UTI auti, const TypeArgs& args, NodeTypeDescriptor * nodetyperef)
  {
    if(!m_state.okUTItoContinue(auti)) //t3339, t3342 might be Hazy, nothing to sync
      return;

    assert(nodetyperef);
    //if(m_state.isScalar(auti)) //t3816, t3223, or refs to arrays t3668
      {
	ALT refalt = m_state.getReferenceType(auti);
	ALT usealt = args.m_declRef; //default
	if((args.m_declRef == ALT_NOT) && (refalt != ALT_NOT))
	  usealt = refalt; //t3728
	else if((args.m_declRef != ALT_NOT) && (refalt == ALT_NOT))
	  usealt = args.m_declRef; //t3728
	//auti as holder might not have correct reference type
	if((refalt != usealt))
	  nodetyperef->setReferenceType(usealt, args.m_referencedUTI, auti); //invariant
	else
	  nodetyperef->resetGivenUTI(auti);
      }
  }

  bool Parser::peekFirstToken(Token & firsttok)
  {
    bool rtnb = m_tokenizer->peekFirstToken(firsttok);
    return rtnb;
  }

  bool Parser::getExpectedToken(TokenType eTokType)
  {
    Token dropTok;
    return getExpectedToken(eTokType, dropTok, QUIETLY); //shh..always quiet.
  }

  bool Parser::getExpectedToken(TokenType eTokType, Token & myTok, bool quietly)
  {
    bool brtn = true;
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type != eTokType)
      {
	unreadToken();

	if(!quietly)
	  {
	    std::ostringstream msg;
	    msg << "Unexpected token <" << pTok.getTokenEnumNameFromPool(&m_state).c_str();
	    msg << ">; Expected ";
	    msg << Token::getTokenAsStringFromPool(eTokType, &m_state).c_str();
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	brtn = false;
      }
    myTok = pTok; //copy even if unexpected for any extra error msg by caller
    return brtn;
  } //getExpectedToken

  bool Parser::getNextToken(Token & tok)
  {
    bool brtn = m_tokenizer->getNextToken(tok);

    if(tok.m_type == TOK_ERROR_LOWLEVEL)
      {
	std::ostringstream msg;
	msg << m_state.getTokenDataAsString(tok).c_str();
	MSG(&tok, msg.str().c_str(), ERR);
	brtn = m_tokenizer->getNextToken(tok); //and yet, we go on..
      }
    else if(tok.m_type == TOK_STRUCTURED_COMMENT)
      {
	Token scTok = tok; //save in case next token is a class or parameter
	brtn = m_tokenizer->getNextToken(tok);
	m_state.saveStructuredCommentToken(scTok);
      }
    return brtn;
  } //getNextToken

  void Parser::unreadToken()
  {
    m_tokenizer->unreadToken();
  }

  //and including lastTok
  void Parser::getTokensUntil(TokenType lastTok)
  {
    Token aTok;
    getNextToken(aTok);
    while(aTok.m_type != lastTok)
      {
	if(aTok.m_type == TOK_EOF)
	  {
	    unreadToken();
	    break;
	  }
	getNextToken(aTok);
      }
    return;
  } //getTokensUntil

  void Parser::abortUnexpectedToken(Token& tok)
  {
    std::ostringstream msg;
    msg << " Unexpected input!! Token <" << m_state.getTokenDataAsString(tok).c_str();
    msg << ">, aborting";
    MSG(&tok, msg.str().c_str(), DEBUG);
    assert(0);
  }

  void Parser::initPrimitiveUlamTypes()
  {
    //initialize "primitive" UlamTypes, in order!!
    // unfortunately, Nav, Atom, Class (except quarks with toInt), Ptr (PtrAbs) and Holder
    // are not considered PRIMITIVE during type processing (use ut->isPrimitiveType());
    {
      UlamKeyTypeSignature nokey(m_state.m_pool.getIndexForDataString("0Nouti"), ULAMTYPE_DEFAULTBITSIZE[Nouti]);
      AssertBool isNouti = (m_state.makeUlamType(nokey, Nouti, UC_NOTACLASS) == Nouti);
      assert(isNouti); //true for primitives
    }

    {
      UlamKeyTypeSignature nkey(m_state.m_pool.getIndexForDataString("0Nav"), ULAMTYPE_DEFAULTBITSIZE[Nav]);
      AssertBool isNav = (m_state.makeUlamType(nkey, Nav, UC_NOTACLASS) == Nav);
      assert(isNav); //true for primitives
    }

    {
      UlamKeyTypeSignature zkey(m_state.m_pool.getIndexForDataString("0Hzy"), ULAMTYPE_DEFAULTBITSIZE[Hzy]);
      AssertBool isHzy = (m_state.makeUlamType(zkey, Hzy, UC_NOTACLASS) == Hzy);
      assert(isHzy); //true for primitives
    }

    {
      UlamKeyTypeSignature vkey(m_state.m_pool.getIndexForDataString("Void"), ULAMTYPE_DEFAULTBITSIZE[Void]);
      AssertBool isVoid = (m_state.makeUlamType(vkey, Void, UC_NOTACLASS) == Void);
      assert(isVoid); //true for primitives
    }

    {
      UlamKeyTypeSignature ikey(m_state.m_pool.getIndexForDataString("Int"), ULAMTYPE_DEFAULTBITSIZE[Int]);
      AssertBool isInt = (m_state.makeUlamType(ikey, Int, UC_NOTACLASS) == Int);
      assert(isInt);
    }

    {
      UlamKeyTypeSignature uikey(m_state.m_pool.getIndexForDataString("Unsigned"), ULAMTYPE_DEFAULTBITSIZE[Unsigned]);
      AssertBool isUnsigned = (m_state.makeUlamType(uikey, Unsigned, UC_NOTACLASS) == Unsigned);
      assert(isUnsigned);
    }

    {
      UlamKeyTypeSignature bkey(m_state.m_pool.getIndexForDataString("Bool"), ULAMTYPE_DEFAULTBITSIZE[Bool]);
      AssertBool isBool = (m_state.makeUlamType(bkey, Bool, UC_NOTACLASS) == Bool);
      assert(isBool);
    }

    {
      UlamKeyTypeSignature ukey(m_state.m_pool.getIndexForDataString("Unary"), ULAMTYPE_DEFAULTBITSIZE[Unary]);
      AssertBool isUnary = (m_state.makeUlamType(ukey, Unary, UC_NOTACLASS) == Unary);
      assert(isUnary);
    }

    {
      UlamKeyTypeSignature bitskey(m_state.m_pool.getIndexForDataString("Bits"), ULAMTYPE_DEFAULTBITSIZE[Bits]);
      AssertBool isBits = (m_state.makeUlamType(bitskey, Bits, UC_NOTACLASS) == Bits);
      assert(isBits);
    }

    {
      UlamKeyTypeSignature ckey(m_state.m_pool.getIndexForDataString("0Class"), ULAMTYPE_DEFAULTBITSIZE[Class]); //bits tbd
      AssertBool isClass = (m_state.makeUlamType(ckey, Class, UC_UNSEEN) == Class);
      assert(isClass);
    }

    {
      UlamKeyTypeSignature akey(m_state.m_pool.getIndexForDataString("Atom"), ULAMTYPE_DEFAULTBITSIZE[UAtom]);
      AssertBool isUAtom = (m_state.makeUlamType(akey, UAtom, UC_NOTACLASS) == UAtom);
      assert(isUAtom);
    }

    {
      UlamKeyTypeSignature pkey(m_state.m_pool.getIndexForDataString("0Ptr"), ULAMTYPE_DEFAULTBITSIZE[Ptr]);
      AssertBool isPtrRel = (m_state.makeUlamType(pkey, Ptr, UC_NOTACLASS) == Ptr);
      assert(isPtrRel);
    }

    {
      UlamKeyTypeSignature hkey(m_state.m_pool.getIndexForDataString("0Holder"), UNKNOWNSIZE);
      AssertBool isHolder = (m_state.makeUlamType(hkey, Holder, UC_NOTACLASS) == Holder);
      assert(isHolder);
    }

    {
      //UTI for local defs within the scope of an ulam file; first one is UrSelf.ulam
      UlamKeyTypeSignature lkey(m_state.m_pool.getIndexForDataString("UrSelf.ulam"), ULAMTYPE_DEFAULTBITSIZE[LocalsFileScope]); //bits 0
      AssertBool isLocalsFileScope = (m_state.makeUlamType(lkey, LocalsFileScope, UC_NOTACLASS) == LocalsFileScope);
      assert(isLocalsFileScope);
    }

    {
      //String, index into UserStringPool
      UlamKeyTypeSignature strkey(m_state.m_pool.getIndexForDataString("String"), ULAMTYPE_DEFAULTBITSIZE[String]);
      AssertBool isString = (m_state.makeUlamType(strkey, String, UC_NOTACLASS) == String);
      assert(isString);
    }

    /*----end of UlamType.inc here----------------*/

    {
      //a Ptr for absolute indexing (i.e. reference class params) in eval; first after end of UlamType.inc. (ALT_PTR not really used; update PtrAbs in Constants.h)
      UlamKeyTypeSignature apkey(m_state.m_pool.getIndexForDataString("0Ptr"), ULAMTYPE_DEFAULTBITSIZE[Ptr], NONARRAYSIZE, ALT_REF);
      AssertBool isPtrAbs = (m_state.makeUlamType(apkey, Ptr, UC_NOTACLASS) == PtrAbs);
      assert(isPtrAbs);
    }

    {
      //a Ref for .atomof; comes after PtrAbs; update UAtomRef in Constants.h
      UlamKeyTypeSignature arefkey(m_state.m_pool.getIndexForDataString("Atom"), ULAMTYPE_DEFAULTBITSIZE[UAtom], NONARRAYSIZE, ALT_REF);
      AssertBool isAtomRef = (m_state.makeUlamType(arefkey, UAtom, UC_NOTACLASS) == UAtomRef);
      assert(isAtomRef);
    }

    {
      //Unsigned(8) shorthand for ASCII in Constants.h
      UlamKeyTypeSignature asckey(m_state.m_pool.getIndexForDataString("Unsigned"), MAXBITSPERASCIIBYTE);
      AssertBool isAscii = (m_state.makeUlamType(asckey, Unsigned, UC_NOTACLASS) == ASCII);
      assert(isAscii);
    }

    // next in line, the 64 basics:
    m_state.getLongUTI();
    m_state.getUnsignedLongUTI();
    m_state.getBigBitsUTI();

    //initialize call stack with 'Int' UlamType pointer
    m_state.m_funcCallStack.init(Int);
    m_state.m_nodeEvalStack.init(Int);
    m_state.m_constantStack.init(Int); //ulam-4 32-bit platform
  } //initPrimitiveUlamTypes

} //end MFM
