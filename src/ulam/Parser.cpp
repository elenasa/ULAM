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
#include "NodeContinueStatement.h"
#include "NodeLabel.h"
#include "NodeMemberSelect.h"
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
	    MSG(m_state.getFullLocationAsString(thisClassSymbol->getLoc()).c_str(), msg.str().c_str(), ERR);
	  }
	rootNode = ((SymbolClass *) thisClassSymbol)->getClassBlockNode();
      }

    if(!rootNode)
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
	    m_state.setLocalsScopeForParsing(pTok);
	    parseLocalDef(); //returns bool
	    m_state.clearLocalsScopeForParsing();
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
	    m_state.addIncompleteClassSymbolToProgramTable(iTok, cnSym);
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
		return  true; //we're done unless we can gobble the rest up?
	      }
	    else if(cnSym->isClassTemplate())
	      {
		std::ostringstream msg;
		msg << "Conflicting class args previously seen for class with no parameters <";
		msg << m_state.m_pool.getDataAsString(cnSym->getId()).c_str() << ">";
		MSG(&iTok, msg.str().c_str(), ERR);
		m_state.clearStructuredCommentToken();
		return  true; //we're done unless we can gobble the rest up?
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
		return true; //we're done unless we can gobble the rest up?
	      }
	    if(ctSym && !ctSym->isClassTemplate())
	      {
		//error already output
		m_state.clearStructuredCommentToken();
		return true; //we're done unless we can gobble the rest up?
	      }
	    wasIncomplete = true;
	  }
	cnSym = ctSym;
      } //template

    UTI cuti = cnSym->getUlamTypeIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    //mostly needed for code gen later, but this is where we first know it!
    m_state.pushClassContext(cuti, cnSym->getClassBlockNode(), cnSym->getClassBlockNode(), false, NULL); //null blocks likely

    //set class type in UlamType (through its class symbol) since we know it;
    //UC_UNSEEN if unseen so far.
    m_state.resetUnseenClass(cnSym, iTok);

    switch(pTok.m_type)
      {
      case TOK_KW_ELEMENT:
	{
	  AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(cut->getUlamKeyTypeSignature(), Class, UC_ELEMENT, cut->isCustomArray());
	  assert(isReplaced);
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
    if(classNode)
      {
	if(wasIncomplete && isTemplate)
	  ((SymbolClassNameTemplate *) cnSym)->fixAnyClassInstances();
      }
    else
      {
	//reset to incomplete (UC_UNSEEN)
	UTI cuti = cnSym->getUlamTypeIdx();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);

	AssertBool isReplaced = m_state.replaceUlamTypeForUpdatedClassType(cut->getUlamKeyTypeSignature(), Class, UC_UNSEEN, cut->isCustomArray());
	assert(isReplaced);

	cnSym->setClassBlockNode(NULL);
	std::ostringstream msg;
	msg << "Empty/Incomplete Class Definition '";
	msg << m_state.getTokenDataAsString(iTok).c_str();
	msg << "'; Possible missing ending curly brace";
	MSG(&pTok, msg.str().c_str(), ERR);
      }

    return false; //keep going until EOF is reached
  } //parseThisClass

  NodeBlockClass * Parser::parseClassBlock(SymbolClassName * cnsym, const Token& identTok)
  {
    bool inherits = false;
    UTI utype = cnsym->getUlamTypeIdx(); //we know its type..sweeter
    NodeBlockClass * rtnNode = cnsym->getClassBlockNode(); //usually NULL
    if(!rtnNode)
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
    u32 selfid = m_state.m_pool.getIndexForDataString("Self");
    Token SelfTok(TOK_TYPE_IDENTIFIER, identTok.m_locator, selfid);
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
	SymbolClass * supercsym = NULL;
	UTI superuti = Nouti;
	inherits = parseRestOfClassInheritance(cnsym, supercsym, superuti);
	if(inherits)
	  {
	    setupSuperClassHelper(supercsym, cnsym);
	    //reset super block pointer since it will change if temporary (e.g. t3874)
	    if(supercsym->isStub() || m_state.isHolder(superuti) || (supercsym->getUlamClass() == UC_UNSEEN))
	      rtnNode->setSuperBlockPointer(NULL); //wait for c&l
	  }
      }
    else
      {
	unreadToken();
	cnsym->setSuperClass(Nouti); //clear

	//earliest ancestor when none designated; for all classes except UrSelf,
	if(!m_state.isUrSelf(cnsym->getUlamTypeIdx()))
	  setupSuperClassHelper(cnsym);
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

    if(!getExpectedToken(TOK_CLOSE_CURLY))
      {
	delete rtnNode;
	rtnNode = NULL;
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
    setupSuperClassHelper(ursym, cnsym);
  }

  void Parser::setupSuperClassHelper(SymbolClass * supercsym, SymbolClassName * cnsym)
  {
    assert(supercsym);
    UTI superuti = supercsym->getUlamTypeIdx();
    assert(cnsym);
    cnsym->setSuperClass(superuti);

    NodeBlockClass * superclassblock = supercsym->getClassBlockNode();
    assert(superclassblock);

    NodeBlockClass * classblock = cnsym->getClassBlockNode();
    assert(classblock); //rtnNode in caller

    //set super class' block after any parameters parsed;
    // (separate from previous block which might be pointing to template
    //  in case of a stub)
    //classblock->setSuperBlockPointer(NULL); //wait for c&l
    classblock->setSuperBlockPointer(superclassblock);

    //rearrange order of class context so that super class is traversed after subclass
    m_state.popClassContext(); //m_currentBlock = prevBlock;
    m_state.pushClassContext(superuti, superclassblock, superclassblock, false, NULL);
    m_state.pushClassContext(cnsym->getUlamTypeIdx(), classblock, classblock, false, NULL); //redo

    //automatically create a Super typedef symbol for this class' super type
    u32 superid = m_state.m_pool.getIndexForDataString("Super");
    Symbol * symtypedef = NULL;
    if(!classblock->isIdInScope(superid, symtypedef))
      {
	Token superTok(TOK_TYPE_IDENTIFIER, superclassblock->getNodeLocation(), superid);
	symtypedef = new SymbolTypedef(superTok, superuti, superuti, m_state);
	assert(symtypedef);
	m_state.addSymbolToCurrentScope(symtypedef);
      }
    else //holder may have been made prior
      {
	assert(symtypedef->getId() == superid);
	UTI stuti = symtypedef->getUlamTypeIdx();
	if(stuti != superuti)
	  m_state.updateUTIAliasForced(stuti, superuti); //t3808, t3806, t3807
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
	Node * argNode = parseClassParameterConstdef(assignrequired);
	Symbol * argSym = NULL;

	//could be null symbol already in scope
	if(argNode)
	  {
	    //parameter IS a NodeConstantdef
	    if(argNode->getSymbolPtr(argSym))
	      {
		((SymbolConstantValue *) argSym)->setClassParameterFlag();
		if(((NodeConstantDef *) argNode)->hasConstantExpr()) //before any folding
		  ((SymbolWithValue *) argSym)->setHasInitValue(); //default value

		//ownership stays with NodeBlockClass's ST
		cntsym->addParameterSymbol((SymbolConstantValue *) argSym);
	      }
	    else
	      MSG(&pTok, "No symbol from class parameter declaration", ERR);

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
	msg << "Expected a 'Type' Token!! got Token '";
	msg << m_state.getTokenDataAsString(pTok).c_str();
	msg << "' instead for class parameter declaration";
	MSG(&pTok, msg.str().c_str(), ERR);
	return;
      }

    getExpectedToken(TOK_COMMA); //if so, get next parameter; o.w. unread

    return parseRestOfClassParameters(cntsym, cblock);
  } //parseRestOfClassParameters

  bool Parser::parseRestOfClassInheritance(SymbolClassName * cnsym, SymbolClass *& supercsym, UTI& superuti)
  {
    bool rtninherits = false;
    assert(cnsym);
    //inheritance
    Token iTok;
    getNextToken(iTok);

    //forward declaration of localdef (t3874)
    if(iTok.m_type == TOK_KW_LOCALDEF)
      {
	if(!getExpectedToken(TOK_DOT))
	  {
	    return false;
	  }
	NodeBlockLocals * localsblock = m_state.makeLocalsScopeBlock(m_state.getContextBlockLoc());
	assert(localsblock);

	m_state.pushClassContext(localsblock->getNodeType(), localsblock, localsblock, false, NULL);

	getNextToken(iTok);
	if(iTok.m_type == TOK_TYPE_IDENTIFIER)
	  {
	    SymbolClassName * supercnsym = NULL;
	    if(!m_state.alreadyDefinedSymbolClassName(iTok.m_dataindex, supercnsym))
	      {
		//check if aleady a typedef..else make one
		UTI tduti = Nav;
		UTI tdscalaruti = Nouti;
		if(!m_state.getUlamTypeByTypedefName(iTok.m_dataindex, tduti, tdscalaruti))
		  {
		    //make a typedef holder for a class
		    UTI huti = m_state.makeUlamTypeHolder();
		    m_state.makeAnonymousClassFromHolder(huti, iTok.m_locator); //anonymous class
		    SymbolTypedef * symtypedef = new SymbolTypedef(iTok, huti, huti, m_state);
		    assert(symtypedef);
		    symtypedef->setBlockNoOfST(m_state.getContextBlockNo());
		    m_state.addSymbolToCurrentScope(symtypedef); //local scope

		    cnsym->setSuperClassForClassInstance(huti, cnsym->getUlamTypeIdx()); //set here!!
		    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(huti, supercsym);
		    assert(isDefined);
		  }
		else
		  {
		    //typedef already defined! must be a localdef
		    UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
		    if(tdut->isHolder() && (tdut->getUlamClassType() == UC_NOTACLASS))
		      {
			//now we know it's a class! iTok "Soo3" for loc  (t41010)
			m_state.makeAnonymousClassFromHolder(tduti, iTok.m_locator); //also returns supercsym
		      }

		    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(tduti, supercsym);
		    assert(isDefined);
		  }
		rtninherits = true; //typedef exists (t41009, t41010)
	      }
	    else
	      {
		//error! using keyword local. with an already defined class
		std::ostringstream msg;
		msg << "Invalid inheritance from local filescope; Class Definition '";
		msg << m_state.getUlamTypeNameBriefByIndex(cnsym->getUlamTypeIdx()).c_str();
		msg << "' is not a typedef";
		MSG(&iTok, msg.str().c_str(), ERR);
	      }
	  }
	m_state.popClassContext(); //restore
	return rtninherits;
      } //done w explicit local. scope

    //o.w. implicit localdef typedefs and constants
    if(iTok.m_type == TOK_TYPE_IDENTIFIER)
      {
	//typedefs, and class argument constants for ancestors maybe in local scope;
	//search class args for constants before local scope (t41007)
	//search locals scope for typedef before class (t3862, 3865, 3868, 3988, 3989)
	m_state.m_parsingVariableSymbolTypeFlag = STF_CLASSINHERITANCE;

	bool isaclass = true;
	superuti = parseClassArguments(iTok, isaclass);
	if(superuti != Nav)
	  {
	    UlamType * superut = m_state.getUlamTypeByIndex(superuti);
	    UlamKeyTypeSignature superkey = superut->getUlamKeyTypeSignature();
	    u32 superid = superkey.getUlamKeyTypeSignatureNameId();

	    UTI instance = cnsym->getUlamTypeIdx();
	    UlamType * instanceut = m_state.getUlamTypeByIndex(instance);
	    UlamKeyTypeSignature ikey = instanceut->getUlamKeyTypeSignature();
	    u32 instanceid = ikey.getUlamKeyTypeSignatureNameId();

	    if(superid == instanceid)
	      {
		std::ostringstream msg;
		msg << "Class Definition '";
		msg << m_state.getUlamTypeNameBriefByIndex(instance).c_str(); //t3900, t3901
		msg << "'; Inheritance from invalid Class '";
		msg << m_state.getTokenDataAsString(iTok).c_str() << "'";
		MSG(&iTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		cnsym->setSuperClassForClassInstance(superuti, instance); //set here!!
		AssertBool isDefined = m_state.alreadyDefinedSymbolClass(superuti, supercsym);
		assert(isDefined);
		rtninherits = true;
	      }
	  }
	m_state.m_parsingVariableSymbolTypeFlag = STF_NEEDSATYPE; //reset
      }
    else
      {
	std::ostringstream msg;
	msg << "Class Definition '";
	msg << m_state.getUlamTypeNameBriefByIndex(cnsym->getUlamTypeIdx()).c_str(); //t3786
	msg << "'; Inheritance from invalid Class identifier '";
	msg << m_state.getTokenDataAsString(iTok).c_str() << "'";
	MSG(&iTok, msg.str().c_str(), ERR);
      }
    return rtninherits;
  } //parseRestOfClassInheritance

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
	isAlreadyAppended = parseTypedef();
      }
    else if(pTok.m_type == TOK_KW_CONSTDEF)
      {
	//parse Named Constant starting with keyword first

	// simulated "data member" flag for constantdef symbols; to
	// distinguish between function scope and filescope constants;
	// not set in Symbol constructor, like Localfilescope flag, since
	// ClassParameter constant defs (e.g. t3328, t3329, t3330..)
	// also have the same BlockNoST as its class.
	m_state.m_parsingVariableSymbolTypeFlag = STF_DATAMEMBER;

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
	    m_state.clearStructuredCommentToken(); //missing
	    return false; //expecting a type, not sizeof, probably an error! (t3856)
	  }

	bool isConstr = false;

	Token iTok;
	getNextToken(iTok);
	if(iTok.m_type != TOK_IDENTIFIER)
	  {
	    if((iTok.m_type == TOK_OPEN_PAREN) && (pTok.m_dataindex == m_state.m_pool.getIndexForDataString("Self")))
	      {
		unreadToken();
		isConstr = true;
		iTok = pTok;
		delete typeNode;
		typeNode = NULL;
	      }
	    else
	      {
		//user error!
		std::ostringstream msg;
		msg << "Name of variable/function <";
		msg << m_state.getTokenDataAsString(iTok).c_str();
		if((Token::getSpecialTokenWork(iTok.m_type) == TOKSP_KEYWORD))
		  msg << ">: Identifier must not be a reserved keyword";
		else
		  msg << ">: Identifier must begin with a lower-case letter";
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
	      brtn = true; //funcdefNode belongs to the symbolFunction; not appended to tree
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
	if(!getExpectedToken(TOK_SEMICOLON))
	  getTokensUntil(TOK_SEMICOLON); //does this help?
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
	    //returns a NodeListArrayInitialization
	    initnode = parseArrayInitialization(identTok.m_dataindex);
	  }
	else
	  initnode = parseExpression();

	if(!initnode)
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

    if(!constrNode)
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

  Node * Parser::makeInstanceofConstructorCall(const Token& fTok, NodeInstanceof * instanceofNode)
  {
    //open paren already eaten
    NodeFunctionCall * constrNode = parseConstructorCall(fTok); //tok for loc and errmsgs

    if(!constrNode)
      {
	return NULL;
      }

    NodeMemberSelectOnConstructorCall * memberSelectNode = new NodeMemberSelectOnConstructorCall(instanceofNode, constrNode, m_state);
    assert(memberSelectNode);
    memberSelectNode->setNodeLocation(fTok.m_locator);

    return memberSelectNode;
  } //makeInstanceofConstructorCall

  NodeFunctionCall * Parser::parseConstructorCall(const Token& identTok)
  {
    u32 selfid = m_state.m_pool.getIndexForDataString("Self");
    Token SelfTok(TOK_TYPE_IDENTIFIER, identTok.m_locator, selfid);
    //if here, must be constructor used to initialize class-type variable!! (t41077)
    NodeFunctionCall * constrNode = (NodeFunctionCall *) parseFunctionCall(SelfTok); //type of variable known later

    if(!constrNode)
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
    if(!sNode)  //e.g. due to an invalid token perhaps; decl already appended
      {
	MSG("", "EMPTY STATEMENT!! when in doubt, count", DEBUG);

	Token pTok;
	getNextToken(pTok);
	unreadToken();
	if(pTok.m_type != TOK_CLOSE_CURLY && pTok.m_type != TOK_EOF)
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
    if(!getExpectedToken(TOK_OPEN_PAREN))
      {
	return NULL;
      }

    //before parsing the IF statement, need a new scope
    NodeBlock * currBlock = m_state.getCurrentBlock();
    NodeBlock * rtnNode = new NodeBlock(currBlock, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(ifTok.m_locator);

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    m_state.pushCurrentBlock(rtnNode); //without pop first

    Node * condNode = parseConditionalExpr();
    if(!condNode)
      {
	std::ostringstream msg;
	msg << "Invalid if-condition"; //t3864
	MSG(&ifTok, msg.str().c_str(), ERR);
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    if(!getExpectedToken(TOK_CLOSE_PAREN))
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

    if(!trueNode)
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
    if(!getExpectedToken(TOK_OPEN_PAREN))
      return NULL;

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
    if(!condNode)
      {
	std::ostringstream msg;
	msg << "Invalid while-condition";
	MSG(&wTok, msg.str().c_str(), ERR);
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    if(!getExpectedToken(TOK_CLOSE_PAREN))
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    assert(controlLoopLabelNum == m_state.m_parsingControlLoopsSwitchStack.getNearestContinueExitNumber()); //sanity

    Node * whileNode = makeControlWhileNode(condNode, NULL, controlLoopLabelNum, wTok);
    if(!whileNode)
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
    if(!getExpectedToken(TOK_OPEN_PAREN))
      return NULL;

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
	AssertBool hasdeclnode = parseDecl(); //updates symbol table
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

    if(qTok.m_type != TOK_SEMICOLON)
      {
	unreadToken();
	condNode = parseConditionalExpr();

	if(!condNode)
	  {
	    std::ostringstream msg;
	    msg << "Invalid for-condition";
	    MSG(&qTok, msg.str().c_str(), ERR);
	    m_state.popClassContext(); //the pop
	    delete rtnNode;
	    return NULL; //stop this maddness
	  }

	if(!getExpectedToken(TOK_SEMICOLON))
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
	if(!assignNode)
	  {
	    std::ostringstream msg;
	    msg << "Malformed assignment; for-control";
	    MSG(&rTok, msg.str().c_str(), ERR);

	    m_state.popClassContext(); //the pop
	    delete rtnNode;
	    delete condNode;
	    return NULL; //stop this maddness
	  }

	if(!getExpectedToken(TOK_CLOSE_PAREN))
	  {
	    m_state.popClassContext(); //the pop
	    delete rtnNode;
	    delete condNode;
	    delete assignNode;
	    return NULL; //stop this maddness
	  }
      } //done with assign expr, could be null

    assert(controlLoopLabelNum == m_state.m_parsingControlLoopsSwitchStack.getNearestContinueExitNumber()); //sanity

    Node * whileNode = makeControlWhileNode(condNode, assignNode, controlLoopLabelNum, fTok);
    if(!whileNode)
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

    if(!trueNode)
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
    Node * labelNode = new NodeLabel(loopnum, m_state);
    assert(labelNode);
    labelNode->setNodeLocation(fwtok.m_locator);

    NodeStatements * labelstmt = new NodeStatements(labelNode, m_state);
    assert(labelstmt);
    truestmt->setNextNode(labelstmt);

    NodeStatements * assignstmt = NULL; //for-loop
    if(assignNodeForLoop)
      {
	assignstmt = new NodeStatements(assignNodeForLoop, m_state);
	assert(assignstmt);
	labelstmt->setNextNode(assignstmt);
      }

    NodeControlWhile * whileNode = new NodeControlWhile(condNode, truestmt, m_state);
    assert(whileNode);
    whileNode->setNodeLocation(fwtok.m_locator);

    return whileNode;
  } //makeControlWhileNode

  Node * Parser::parseControlSwitch(const Token& swTok)
  {
    if(!getExpectedToken(TOK_OPEN_PAREN))
      {
	return NULL;
      }

    u32 switchnum = m_state.getNextTmpVarNumber();

    //before parsing the SWITCH statement, need a new scope
    NodeBlock * currBlock = m_state.getCurrentBlock();
    NodeBlock * rtnNode = new NodeBlockSwitch(currBlock, switchnum, m_state);
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
    //if(!condNode)
    if(!condNode && !whichtrue)
      {
	std::ostringstream msg;
	msg << "Invalid which-control condition";
	MSG(&swTok, msg.str().c_str(), ERR);
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    if(!getExpectedToken(TOK_CLOSE_PAREN))
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    if(!getExpectedToken(TOK_OPEN_CURLY))
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	delete rtnNode;
	return NULL; //stop this maddness
      }

    UTI huti = whichtrue ? Bool : m_state.makeUlamTypeHolder();

    std::string swtypedefname = m_state.getSwitchTypedefNameAsString(switchnum);
    Token swtypeTok(TOK_IDENTIFIER, swTok.m_locator, m_state.m_pool.getIndexForDataString(swtypedefname));

    //give a name to this new ulam type
    SymbolTypedef * symtypedef = new SymbolTypedef(swtypeTok, huti, Nav, m_state);
    assert(symtypedef);
    m_state.addSymbolToCurrentScope(symtypedef);
    if(!whichtrue) //t41046
      m_state.addUnknownTypeTokenToThisClassResolver(swtypeTok, huti);

    NodeTypedef * swtypedefnode = new NodeTypedef(symtypedef, NULL, m_state);
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

	NodeVarDecl * swvalueDecl = new NodeVarDecl(swsym, NULL, m_state);
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

    //label for end-of-switch breaks (t41018)
    u32 swexitnum = m_state.m_parsingControlLoopsSwitchStack.getNearestBreakExitNumber();
    Node * labelNode = new NodeLabel(swexitnum, m_state);
    assert(labelNode);
    labelNode->setNodeLocation(pTok.m_locator);
    rtnNode->appendNextNode(labelNode);

    delete swvalueIdent; //copies made, clean up
    m_state.popClassContext(); //the pop
    return rtnNode;
  } //parseControlSwitch

  Node * Parser::parseNextCase(const NodeIdent * swvalueIdent, NodeControlIf *& switchNode, Node *& defaultNode)
  {
    //get as many cases that share the same body
    Node * casecondition = NULL;
    casecondition = parseRestOfCase(swvalueIdent, casecondition, defaultNode);

    if(!casecondition)
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

    NodeBlock * trueNode = NULL;
    // support as-condition as case expression (t41045,46,47,48)
    if(m_state.m_parsingConditionalAs)
      trueNode = setupAsConditionalBlockAndParseStatements((NodeConditional *) casecondition); //t41046
    else
      trueNode = parseStatementAsBlock(); //even an empty statement has a node!

    if(!trueNode)
      {
	std::ostringstream msg;
	msg << "Incomplete true block; which-control failure";
	MSG(casecondition->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	delete casecondition;
	return NULL; //stop this maddness
      }

    NodeControlIf * ifNode = new NodeControlIf(casecondition, trueNode, NULL, m_state);
    assert(ifNode);
    ifNode->setNodeLocation(casecondition->getNodeLocation());

    if(switchNode != NULL)
      switchNode->setElseNode(ifNode); //link to previous if-
    else
      switchNode = ifNode; //set parent ref
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
	if(!rightNode)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete case expression; which-control failure";
	    MSG(&cTok, msg.str().c_str(), ERR);
	    delete caseCond;
	    return NULL; //stop this maddness
	  }

	if(m_state.m_parsingConditionalAs)
	  {
	    if(!getExpectedToken(TOK_COLON))
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
	    leftNodeCopy->resetNodeNo(m_state.getNextNodeNo()); //unique invarient
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

    if(!getExpectedToken(TOK_COLON))
      {
	delete casecondition;
	delete caseCond;
	return NULL; //stop this maddness
      }

    //multi-cases (t41020)
    if(caseCond != NULL)
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
    Node * rtnNode = NULL;
    Token iTok;
    if(getExpectedToken(TOK_IDENTIFIER, iTok, QUIETLY))
      {
	Symbol * asymptr = NULL;
	bool hazyKin = false; //don't care
	//may continue when symbol not defined yet (e.g. FuncCall)
	if(m_state.alreadyDefinedSymbol(iTok.m_dataindex, asymptr, hazyKin))
	  {
	    if(asymptr->isConstant()) //check for constant first
	      {
		unreadToken();
		return parseExpression();
	      }
	  }

	if(!(rtnNode = parseIdentExpr(iTok)))
	  return parseExpression(); //continue as parseAssignExpr

	//next check for 'as' ('is' is a Factor)
	Token cTok;
	getNextToken(cTok);
	unreadToken();
	if(cTok.m_type == TOK_KW_AS)
	  {
	    m_state.saveIdentTokenForConditionalAs(iTok, cTok); //SETS other related globals
	    rtnNode = makeConditionalExprNode(rtnNode); //done, could be NULL
	  }
      }
    else
      return parseExpression(); //perhaps a number, true or false, cast..

    if(m_state.m_parsingConditionalAs)
      return rtnNode;

    //if nothing else follows, parseRestOfAssignExpr returns its argument
    return parseRestOfAssignExpr(rtnNode);
  } //parseConditionalExpr

  NodeBlock * Parser::setupAsConditionalBlockAndParseStatements(NodeConditional * asNode)
  {
    assert(m_state.m_parsingConditionalAs);

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

    UTI ruti = asNode->getRightType(); //what if Self?
    assert(m_state.okUTItoContinue(ruti));
    UTI tuti = m_state.getUlamTypeAsRef(ruti, ALT_AS);

    UlamType * tut = m_state.getUlamTypeByIndex(tuti);
    const std::string tdname = tut->getUlamTypeNameOnly();
    Token typeTok;
    u32 tdid = m_state.m_pool.getIndexForDataString(tdname);
    typeTok.init(TOK_TYPE_IDENTIFIER, asNode->getNodeLocation(), tdid);

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
	    getExpectedToken(TOK_CLOSE_CURLY);
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
	msg << "Statement starts with flag keyword <";
	msg << m_state.getTokenDataAsString(pTok).c_str() << ">";
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

	if(!typeNode)
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
	    NodeTypedef * rtnNode = makeTypedefSymbol(typeargs, iTok, typeNode);
	    if(rtnNode)
	      {
		m_state.appendNodeToCurrentBlock(rtnNode);
		brtn = true;
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid typedef Alias <" << m_state.getTokenDataAsString(iTok).c_str();
	    msg << ">, Type Identifier (2nd arg) requires capitalization";
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

    if( (Token::isTokenAType(pTok) || (pTok.m_type == TOK_KW_LOCALDEF)) && (pTok.m_type != TOK_KW_TYPE_VOID) && (pTok.m_type != TOK_KW_TYPE_ATOM))
      {
	unreadToken();
	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, false, false);
	if(!typeNode)
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
	    NodeConstantDef * constNode = makeConstdefSymbol(typeargs, iTok, typeNode);
	    if(constNode)
	      {
		m_state.appendNodeToCurrentBlock(constNode);
		brtn = true;
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid constant definition Alias '";
	    msg << m_state.getTokenDataAsString(iTok).c_str();
	    msg << "', Constant Identifier (2nd arg) requires lower-case";
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
	if(!typeNode)
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
		msg << "Invalid Model Parameter Name <";
		msg << m_state.getTokenDataAsString(iTok).c_str();
		msg << ">, Parameter Identifier requires lower-case";
		MSG(&iTok, msg.str().c_str(), ERR);
		delete typeNode;
		typeNode = NULL;
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid Model Parameter Type <";
	    msg << m_state.getTokenDataAsString(pTok).c_str();
	    msg << ">";
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

    if(!typeNode)
      {
	std::ostringstream msg;
	msg << "Invalid Type for named variable";
	MSG(&iTok, msg.str().c_str(), ERR);
	unreadToken();
	return false;
      }

    if(iTok.m_type == TOK_IDENTIFIER)
      {
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
	msg << "Name of variable <" << m_state.getTokenDataAsString(iTok).c_str();
	if((Token::getSpecialTokenWork(iTok.m_type) == TOKSP_KEYWORD))
	  msg << ">: Identifier must not be a reserved keyword";
	else
	  msg << ">: Identifier must begin with a lower-case letter";
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
	//assuming locals defined before they are referred to
	NodeBlockLocals * localsblock = m_state.makeLocalsScopeBlock(pTok.m_locator);
	if(!localsblock)
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
		Token nTok;
		getNextToken(nTok);
		if(Token::isTokenAType(nTok))
		  {
		    typeargs.init(nTok);
		    m_state.pushClassContext(localsblock->getNodeType(), localsblock, localsblock, false, NULL);

		    typeNode = parseTypeDescriptor(typeargs, castUTI, isaclass, delAfterDotFails);

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
	//note: this means it can't be a typedef from another class; might be a class constructor (t41077)
	unreadToken();
	return NULL;
      }

    typeargs.init(pTok); //initialize here

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
	    UTI huti = Nav;
	    UTI tmpscalar= Nav;
	    AssertBool isTDDefined = m_state.getUlamTypeByTypedefName(pTok.m_dataindex, huti, tmpscalar);
	    assert(isTDDefined);
	    m_state.makeAnonymousClassFromHolder(huti, pTok.m_locator); //don't need cnsym here
	  }

	//not sure what to do with the UTI? could be a declref type; both args are refs!
	UTI cuti = parseClassArguments(pTok, maybeAClassType);
	if(maybeAClassType)
	  {
	    if(m_state.isReference(cuti)) //e.g. refofSelf, ref to array of classes
	      {
		typeargs.m_classInstanceIdx = m_state.getUlamTypeAsDeref(cuti);
		typeargs.m_declRef = ALT_REF;
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
	typeNode = new NodeTypeDescriptor(typeargs.m_typeTok, cuti, m_state);
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
	      getNextToken(dTok); //?
	  }
	else
	  {
	    castUTI = typeargs.m_anothertduti;
	    getNextToken(dTok);
	  }
      }

    //else fall thru
    if(dTok.m_type == TOK_AMP)
      {
	typeargs.m_declRef = ALT_REF; //a declared reference
	typeargs.m_referencedUTI = castUTI; //?
	typeargs.m_assignOK = true; //required
	typeargs.m_isStmt = true; //unless a func param
	// change uti to reference key
	UTI refuti = castUTI;
	if(m_state.okUTItoContinue(castUTI)) //t41153
	  refuti = m_state.getUlamTypeAsRef(castUTI); //t3692
	assert(typeNode);
	typeNode->setReferenceType(ALT_REF, castUTI, refuti);
      }
    else
      unreadToken();

    return typeNode;
  } //parseTypeDescriptor

  UTI Parser::parseClassArguments(Token& typeTok, bool& isaclass)
  {
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type != TOK_OPEN_PAREN)
      {
	//regular class, not a template, OR Self (unless a constructor)
	unreadToken();

	SymbolClassName * cnsym = NULL;
	if(!m_state.alreadyDefinedSymbolClassName(typeTok.m_dataindex, cnsym))
	  {
	    UTI tduti = Nav;
	    UTI tdscalaruti = Nouti;
	    //check if a typedef first..look localdefs first when parsing class inheritance
	    if(((m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSINHERITANCE) && m_state.getUlamTypeByTypedefNameinLocalsScope(typeTok.m_dataindex, tduti, tdscalaruti)) || m_state.getUlamTypeByTypedefName(typeTok.m_dataindex, tduti, tdscalaruti))
	      {
		ULAMTYPE bUT = m_state.getUlamTypeByIndex(tduti)->getUlamTypeEnum();
		isaclass |= (bUT == Class); //or Hzy or Holder?

		if(isaclass && (bUT == Holder)) //not isHolder(tduti)
		  {
		    m_state.makeAnonymousClassFromHolder(tduti, typeTok.m_locator); //t3862
		  }
		return tduti; //done. (could be an array; or refselftype)
	      }
	    else
	      {
		// not a typedef, but not necessarily a class either!!
		if(isaclass)
		  m_state.addIncompleteClassSymbolToProgramTable(typeTok, cnsym);
		else
		  {
		    UTI huti = m_state.makeUlamTypeHolder();
		    if(!m_state.isThisLocalsFileScope())
		      {
			m_state.addUnknownTypeTokenToThisClassResolver(typeTok, huti);

			// set contains possible unseen classes (ulamexports);
			// see if they exist without being too liberal about guessing
			m_state.m_unseenClasses.insert(typeTok.m_dataindex); //possible class
		      }
		    else //not yet defined in local file scope; add a holder (t3873)
		      {
			SymbolTypedef * symtypedef = new SymbolTypedef(typeTok, huti, Nav, m_state);
			assert(symtypedef);
			symtypedef->setBlockNoOfST(m_state.getContextBlockNo());
			m_state.addSymbolToCurrentScope(symtypedef); //locals scope
		      }
		    return huti;
		  }
	      }
	  }
	else
	  {
	    if(cnsym->isClassTemplate())
	      {
		//params but no args
		std::ostringstream msg;
		msg << "Missing Class Arguments for an instance stub of class template '";
		msg << m_state.m_pool.getDataAsString(cnsym->getId()).c_str() << "'";
		MSG(&pTok, msg.str().c_str(), ERR);
		return Nav;
	      }
	    else
	      isaclass = true; //reset
	  }
	assert(cnsym);
	return cnsym->getUlamTypeIdx();
      } //not open paren
    else if(typeTok.m_dataindex == m_state.m_pool.getIndexForDataString("Self"))
      {
	//this is a constructor
	unreadToken();
	isaclass = true; //reset
	return m_state.getCompileThisIdx(); //or Void?
      }

    //must be a template class
    bool unseenTemplate = false;
    SymbolClassNameTemplate * ctsym = NULL;
    if(!m_state.alreadyDefinedSymbolClassNameTemplate(typeTok.m_dataindex, ctsym))
      {
	unseenTemplate = true;
	if(ctsym == NULL) //was undefined, template; will fix instances' argument names later
	  m_state.addIncompleteTemplateClassSymbolToProgramTable(typeTok, ctsym);
	else
	  {
	    //error have a class without parameters already defined
	    getTokensUntil(TOK_CLOSE_PAREN); //rest of statement is ignored.
	    return Nav; //short-circuit
	  }
      }

    assert(ctsym);

    UTI ctuti = ctsym->getUlamTypeIdx();
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
    UTI stubuti = m_state.makeUlamType(typeTok, UNKNOWNSIZE, NONARRAYSIZE, Nouti, ALT_NOT, ctsym->getUlamClass()); //overwrites the template type here; possibly UC_UNSEEN

    UlamType * ctut = m_state.getUlamTypeByIndex(ctuti);
    if(ctut->isCustomArray())
      {
	UlamType * stubut = m_state.getUlamTypeByIndex(stubuti);
	((UlamTypeClass *) stubut)->setCustomArray();
      }

    SymbolClass * stubcsym = ctsym->makeAStubClassInstance(typeTok, stubuti);
    stubcsym->setContextForPendingArgs(m_state.getCompileThisIdx());

    u32 parmidx = 0;
    parseRestOfClassArguments(stubcsym, ctsym, parmidx);

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
	    MSG(&typeTok, msg.str().c_str(), ERR);
	    stubuti = Nav;
	  }
	else
	  {
	    //(similar to fixAnyClassInstances for unseen templates)
	    //copy the default parameter symbols and nodeconstantdef's as pending args (t3526)
	    ctsym->fixAClassStubsDefaultArgs(stubcsym, parmidx);
	  }
      }
    if(stubuti != Nav)
      isaclass = true; //we know now for sure
    return stubuti;
  } //parseClassArguments

  void Parser::parseRestOfClassArguments(SymbolClass * csym, SymbolClassNameTemplate * ctsym, u32& parmIdx)
  {
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      return;

    unreadToken();

    Node * exprNode = parseExpression(); //constant expression req'd

    assert(csym);
    if(!exprNode)
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
	//try to continue..
	m_state.pushCurrentBlock(csym->getClassBlockNode()); //reset here for new arg's ST

	SymbolConstantValue * argSym = NULL;
	if(!ctUnseen)
	  {
	    SymbolConstantValue * paramSym = ctsym->getParameterSymbolPtr(parmIdx);
	    assert(paramSym);
	    Token argTok(TOK_IDENTIFIER, pTok.m_locator, paramSym->getId()); //use current locator
	    argSym = new SymbolConstantValue(argTok, paramSym->getUlamTypeIdx(), m_state); //like param, not copy
	  }
	else
	  {
	    std::ostringstream sname;
	    sname << "_" << parmIdx;
	    u32 snameid = m_state.m_pool.getIndexForDataString(sname.str());
	    Token argTok(TOK_IDENTIFIER, pTok.m_locator, snameid); //use current locator
	    //stub id,  m_state.getUlamTypeOfConstant(Int) stub type, state
	    argSym = new SymbolConstantValue(argTok, Hzy, m_state);
	  }

	assert(argSym);
	argSym->setClassArgumentFlag();
	m_state.addSymbolToCurrentScope(argSym); //scope updated to new class instance in parseClassArguments

	NodeTypeDescriptor * argTypeDesc = NULL;
	if(!ctUnseen)
	  {
	    //copy the type descriptor from the template for the stub
	    NodeBlockClass * templateblock = ctsym->getClassBlockNode();
	    NodeConstantDef * paramConstDef = (NodeConstantDef *) templateblock->getParameterNode(parmIdx);
	    assert(paramConstDef);
	    NodeTypeDescriptor * paramTypeDesc = NULL;
	    if(paramConstDef->getNodeTypeDescriptorPtr(paramTypeDesc))
	      {
		m_state.pushClassContext(ctsym->getUlamTypeIdx(), templateblock, templateblock, false, NULL); //null blocks likely

		argTypeDesc = (NodeTypeDescriptor * ) paramTypeDesc->instantiate(); //copy it
		m_state.popClassContext();
	      }
	  }

	//make Node with argument symbol wo trying to fold const expr;
	// add to list of unresolved for this uti
	// NULL node type descriptor, no token, yet known uti
	NodeConstantDef * constNode = new NodeConstantDef(argSym, argTypeDesc, m_state);
	assert(constNode);
	constNode->setNodeLocation(pTok.m_locator);
	constNode->setConstantExpr(exprNode);
	//fold later; non ready expressions saved by UTI in m_nonreadyClassArgSubtrees (stub)
	csym->linkConstantExpressionForPendingArg(constNode);
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
    return parseRestOfClassArguments(csym, ctsym, ++parmIdx); //recurse
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
	if(!bitsizeNode)
	  {
	    std::ostringstream msg;
	    msg << "Bit size after Open Paren is missing, type: ";
	    msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str();
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
		msg << "String bitsize expression disregarded; size is " << MAXBITSPERINT;
		MSG(&bTok, msg.str().c_str(), WARN);

		args.m_bitsize = MAXBITSPERINT;
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

	if(!getExpectedToken(TOK_CLOSE_PAREN))
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
    if(!(m_state.alreadyDefinedSymbolClassName(args.m_typeTok.m_dataindex, cnsym) || m_state.alreadyDefinedSymbol(args.m_typeTok.m_dataindex, asym, hazyKin)))
      {
	//if here, the last typedef might have been a holder for some unknown type
	// now we know (thanks to the dot and subsequent type token) that its a holder
	// for a class. but we don't know it's real name, yet!

	// we need to add it to our table as an anonymous classes for now
	// using its the string of its UTI as its temporary out-of-band name.

	// in the future, we will only be able to find it via its UTI, not a token.
	// the UTI should be the anothertduti (unless numdots is only 1).

	Token pTok; //look ahead after the dot
	getNextToken(pTok);
	unreadToken();

	if(numDots > 1 && Token::isTokenAType(pTok))
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
		if(!m_state.alreadyDefinedSymbolClass(acuti, tmpcsym))
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
		else
		  {
		    u32 cid = tmpcsym->getId();
		    AssertBool isDefined = m_state.alreadyDefinedSymbolClassName(cid, cnsym);
		    assert(isDefined);
		  }
	      }
	  }
      }

    // either found a class or made one up, continue..
    assert(cnsym);
    SymbolClass * csym = NULL;
    if(cnsym->isClassTemplate() && (args.m_classInstanceIdx != Nouti))
      {
	if(! ((SymbolClassNameTemplate *)cnsym)->findClassInstanceByUTI(args.m_classInstanceIdx, csym))
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
    NodeBlockClass * memberClassNode = csym->getClassBlockNode();
    if(!memberClassNode) //e.g. forgot the closing brace on quark def once; or UNSEEN
      {
	//hail mary pass..possibly a sizeof of unseen class
	getNextToken(nTok);
	if((nTok.m_type != TOK_KW_SIZEOF) && (nTok.m_type != TOK_KW_INSTANCEOF) && (nTok.m_type != TOK_KW_ATOMOF))
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
    m_state.pushClassContextUsingMemberClassBlock(memberClassNode);

    Token pTok;
    //after the dot
    getNextToken(pTok);
    if(Token::isTokenAType(pTok))
      {
	UTI tduti;
	UTI tdscalaruti = Nav;
	bool isclasstd = false;
	if(!m_state.getUlamTypeByTypedefName(pTok.m_dataindex, tduti, tdscalaruti))
	  {
	    //make one up!! if UN_SEEN class
	    UTI mcuti = memberClassNode->getNodeType();
	    ULAMCLASSTYPE mclasstype = m_state.getUlamTypeByIndex(mcuti)->getUlamClassType();
	    if(mclasstype == UC_UNSEEN)
	      {
		UTI huti = m_state.makeUlamTypeHolder();
		SymbolTypedef * symtypedef = new SymbolTypedef(pTok, huti, Nav, m_state);
		assert(symtypedef);
		symtypedef->setBlockNoOfST(memberClassNode->getNodeNo());
		m_state.addSymbolToCurrentMemberClassScope(symtypedef); //not locals scope
		m_state.addUnknownTypeTokenToAClassResolver(mcuti, pTok, huti);
		//these will fail: t3373-8, t3380-1,5, t3764, and t3379
		//m_state.addUnknownTypeTokenToThisClassResolver(pTok, huti); //also, compiling this one
	      }
	  } //end make one up, now fall through

	if(m_state.getUlamTypeByTypedefName(pTok.m_dataindex, tduti, tdscalaruti))
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
	    assert(m_state.okUTItoContinue(tduti));
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
	    MSG(&pTok, msg.str().c_str(), ERR);
	    rtnb = false;
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

    //fix during NodeConstant c&l
    rtnNode = new NodeConstant(iTok, NULL, typedescNode, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(iTok.m_locator);
    return rtnNode;
  } //parseNamedConstantFromAnotherClass

  bool Parser::parseReturn()
  {
    Token pTok;
    getNextToken(pTok);

    NodeReturnStatement * returnNode = NULL;
    Node * rtnExprNode = parseAssignExpr(); //may be NULL
    if(!rtnExprNode)
      {
	rtnExprNode = new NodeStatementEmpty(m_state); //has Nouti type
	assert(rtnExprNode);
	rtnExprNode->setNodeLocation(pTok.m_locator);
      }
    returnNode =  new NodeReturnStatement(rtnExprNode, m_state);
    assert(returnNode);
    returnNode->setNodeLocation(pTok.m_locator);

    m_state.appendNodeToCurrentBlock(returnNode);
    return true;
  } //parseReturn

  Node * Parser::parseAssignExpr()
  {
    Node * rtnNode = NULL;
    Token iTok;
    if(getExpectedToken(TOK_IDENTIFIER, iTok, QUIETLY))
      {
	//though function calls are not proper lhs values in assign
	//expression; they are parsed here (due to the two token look
	//ahead, which drops the Identifier Token before parseExpression) and is
	//caught during checkAndLabelType as NOT storeIntoAble.
	if(!(rtnNode = parseIdentExpr(iTok)))
	  return parseExpression();
      }
    else
      rtnNode = parseExpression(); //perhaps a number, true or false, cast..

    Token qTok;
    getNextToken(qTok);
    unreadToken();
    if(qTok.m_type == TOK_QUESTION)
      {
	rtnNode = parseRestOfQuestionColonExpr(rtnNode); //t41072
      }

    //if nothing else follows, parseRestOfAssignExpr returns its argument
    return parseRestOfAssignExpr(rtnNode);
  } //parseAssignExpr

  Node * Parser::parseLvalExpr(const Token& identTok)
  {
    //function calls and func defs are not valid; until constructor calls came along
    //if(pTok.m_type == TOK_OPEN_PAREN) no longer necessarily an error

    Symbol * asymptr = NULL;
    // may continue when symbol not defined yet (e.g. Decl)
    // don't return a NodeConstant, instead of NodeIdent, without arrays
    // even if already defined as one. lazy evaluate.
    bool isDefined = m_state.isIdInCurrentScope(identTok.m_dataindex, asymptr); //t3887
    if(!isDefined && (identTok.m_type == TOK_IDENTIFIER) && m_state.m_parsingVariableSymbolTypeFlag == STF_CLASSINHERITANCE)
      {
	bool locDefined = m_state.isIdInLocalFileScope(identTok.m_dataindex, asymptr);
	NodeBlockLocals * localsblock = m_state.getLocalsScopeBlock(m_state.getContextBlockLoc());
	if(!locDefined && localsblock)
	  {
	    m_state.pushClassContext(localsblock->getNodeType(), localsblock, localsblock, false, NULL); //?

	    //make holder for this localdef constant not seen yet! (t3873)
	    UTI huti = m_state.makeUlamTypeHolder();
	    SymbolConstantValue * constsym = new SymbolConstantValue(identTok, huti, m_state);
	    constsym->setBlockNoOfST(m_state.getContextBlockNo());
	    m_state.addSymbolToCurrentScope(constsym);
	    asymptr = constsym;

	    m_state.popClassContext(); //?
	  }
	//else (t41007)
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
	Symbol * asymptr = NULL;
	bool hazyKin = false; //don't care
	//may continue when symbol not defined yet (e.g. FuncCall)
	m_state.alreadyDefinedSymbol(identTok.m_dataindex, asymptr, hazyKin);
	if(asymptr && !asymptr->isFunction())
	  {
	    std::ostringstream msg;
	    msg << "Undefined function <" << m_state.getTokenDataAsString(identTok).c_str();
	    msg << "> that has already been declared as a variable";
	    MSG(&identTok, msg.str().c_str(), ERR);
	    return  NULL; //bail
	  }
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

  Node * Parser::parseMemberSelectExpr(const Token& memberTok)
  {
    //arg is an instance of a class, it will be/was
    //declared as a variable, either as a data member or locally,
    //WAIT To  search back through the block symbol tables during type labeling
    m_state.pushClassContextUsingMemberClassBlock(NULL); //oddly =true

    Node * classInstanceNode = parseIdentExpr(memberTok); //incl array item, func call, etc.
    m_state.popClassContext();

    return parseRestOfMemberSelectExpr(classInstanceNode);
  }

  Node * Parser::parseRestOfMemberSelectExpr(Node * classInstanceNode)
  {
    Node * rtnNode = classInstanceNode; //first one
    Token pTok;

    //use loop rather than recursion to get a left-justified tree;
    // needed to support, for example: a.b.c.atomof (t3905)
    while(getExpectedToken(TOK_DOT, pTok, QUIETLY)) //if not, quietly unreads
      {
	Token iTok;
	getNextToken(iTok);
	if(iTok.m_type == TOK_IDENTIFIER)
	  {
	    //set up compiler state to NOT use the current class block
	    //for symbol searches; may be unknown until type label
	    m_state.pushClassContextUsingMemberClassBlock(NULL); //oddly =true
	    Node * nextmember = parseIdentExpr(iTok); //includes array item, func call, etc.
	    if(!nextmember)
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
	else
	  {
	    //end of dot chain
	    unreadToken(); //pTok
	    rtnNode = parseMinMaxSizeofType(rtnNode, Nouti, NULL); //ate dot, possible min/max/sizeof
	    if(!rtnNode)
	      delete classInstanceNode; //t41110 leak
	  }
      } //while
    return rtnNode;
  } //parseRestOfMemberSelectExpr

  Node * Parser::parseRestOfQuestionColonExpr(Node * condNode)
  {
    Token pTok;
    if(!getExpectedToken(TOK_QUESTION, pTok, QUIETLY))
      {
	m_state.abortShouldntGetHere();
	delete condNode;
	return NULL;
      }

    Node * trueNode = parseExpression();
    if(!trueNode)
      {
	std::ostringstream msg;
	msg << "Incomplete true expression; question-colon-control";
	MSG(&pTok, msg.str().c_str(), ERR);
	delete condNode;
	return NULL; //stop this maddness
      }

    if(!getExpectedToken(TOK_COLON))
      {
	delete condNode;
	delete trueNode;
	return NULL;
      }

    Node * falseNode = parseExpression();
    if(!falseNode)
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
  } //parseRestOfQuestionColonExpr

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
      case TOK_KW_INSTANCEOF:
	{
	rtnNode = new NodeInstanceof(memberNode, nodetype, m_state);
	rtnNode->setNodeLocation(fTok.m_locator);

	Token cTok;
	getNextToken(cTok);
	if(cTok.m_type == TOK_OPEN_PAREN)
	  {
	    Node * cctrNode = makeInstanceofConstructorCall(fTok, (NodeInstanceof *) rtnNode);
	    if(!cctrNode)
	      {
		delete rtnNode;
		rtnNode = NULL;
		//error
	      }
	    else
	      rtnNode = cctrNode;
	  }
	else
	  unreadToken();

	}
	break;
      case TOK_KW_ATOMOF:
	{
	  if(memberNode == NULL)
	    {
	      //caller will clean up nodetype
	      std::ostringstream msg;
	      msg << "Unsupported request: '" << m_state.getTokenDataAsString(fTok).c_str();
	      msg << "' of Type <" << m_state.getUlamTypeNameBriefByIndex(utype).c_str() << ">";
	      MSG(&fTok, msg.str().c_str(), ERR);
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
      default:
	unreadToken();
      };
    return rtnNode; //may be null if not minof, maxof, sizeof, but a member or func selected
  } //parseMinMaxSizeofType

  Node * Parser::parseFunctionCall(const Token& identTok)
  {
    Symbol * asymptr = NULL;
    NodeBlock * currBlock = m_state.getCurrentBlock();

    //cannot call a function if a local variable name shadows it
    if(currBlock && currBlock->isIdInScope(identTok.m_dataindex,asymptr))
      {
	std::ostringstream msg;
	msg << "'" << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
	msg << "' cannot be used as a function, already declared as a variable '";
	msg << m_state.getUlamTypeNameByIndex(asymptr->getUlamTypeIdx()).c_str();
	msg << " " << m_state.m_pool.getDataAsString(asymptr->getId()) << "'";
	MSG(&identTok, msg.str().c_str(), ERR);
	return NULL;
      }

    //fill in func symbol during type labeling; supports function overloading
    NodeFunctionCall * rtnNode = new NodeFunctionCall(identTok, NULL, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(identTok.m_locator);

    //member selection doesn't apply to arguments (during parsing too)
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
    if(!rtnNode)
      return NULL; //stop this maddness

    //if not addop, parseRestOfExpression returns its arg
    return parseRestOfExpression(rtnNode);
  } //parseExpression

  Node * Parser::parseLogicalExpression()
  {
    Node * rtnNode = parseBitExpression();
    if(!rtnNode)
      return NULL; //stop this maddness

    //if not bitop, parseRestOfLogicalExpression returns its arg
    return parseRestOfLogicalExpression(rtnNode);
  } //parseLogicalExpression

  Node * Parser::parseBitExpression()
  {
    Node * rtnNode = parseEqExpression();
    if(!rtnNode)
      return NULL; //stop this maddness

    //if not eqop, parseRestOfBitExpression returns its arg
    return parseRestOfBitExpression(rtnNode);
  } //parseBitExpression

  Node * Parser::parseEqExpression()
  {
    Node * rtnNode = parseCompareExpression();
    if(!rtnNode)
      return NULL; //stop this maddness

    //if not compop, parseRestOfEqExpression returns its arg
    return parseRestOfEqExpression(rtnNode);
  } //parseEqExpression

  Node * Parser::parseCompareExpression()
  {
    Node * rtnNode = parseShiftExpression();
    if(!rtnNode)
      return NULL; //stop this maddness

    //if not shiftop, parseRestOfCompareExpression returns its arg
    return parseRestOfCompareExpression(rtnNode);
  } //parseCompareExpression

  Node * Parser::parseShiftExpression()
  {
    Node * rtnNode = parseTerm();
    if(!rtnNode)
      return NULL; //stop this maddness

    //if not addop, parseRestOfShiftExpression returns its arg
    return parseRestOfShiftExpression(rtnNode);
  } //parseShiftExpression

  Node * Parser::parseTerm()
  {
    Node * rtnNode = parseFactor();
    if(!rtnNode)
      return NULL; //stop this maddness!

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
	bool allowrefcast = (m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCARGUMENT) || (m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCLOCALREF);
	return parseFactorStartingWithAType(pTok, allowrefcast); //rtnNode could be NULL
      } //not a Type

    //continue as normal..
    switch(pTok.m_type)
      {
      case TOK_IDENTIFIER:
	{
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
      case TOK_SQUOTED_STRING:
      case TOK_DQUOTED_STRING:
	rtnNode = new NodeTerminal(pTok, m_state);
	assert(rtnNode);
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
	  bool allowrefcast = (m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCARGUMENT) || (m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCLOCALREF); //t3862, t41067
	  rtnNode = parseRestOfCastOrExpression(allowrefcast);
	}
	break;
      case TOK_MINUS:
      case TOK_PLUS:
      case TOK_BANG:
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
	      if(!localsblock)
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

  Node * Parser::parseFactorStartingWithAType(const Token& tTok, bool allowrefcast)
  {
    assert(Token::isTokenAType(tTok)); //was unread.

    TypeArgs typeargs;
    NodeTypeDescriptor * typeNode = parseTypeDescriptor(typeargs);
    assert(typeNode);
    UTI uti = typeNode->givenUTI();

    //returns either a terminal or proxy
    Node * rtnNode = parseMinMaxSizeofType(NULL, uti, typeNode); //optionally, gets next dot token
    if(!rtnNode)
      {
	Token iTok;
	getNextToken(iTok);
	if(iTok.m_type == TOK_IDENTIFIER)
	  {
	    //assumes we saw a 'dot' prior, be careful
	    unreadToken();
	    rtnNode = parseNamedConstantFromAnotherClass(typeargs, typeNode);
	  }
	else if(iTok.m_type == TOK_CLOSE_PAREN)
	  {
	    unreadToken();
	    bool castok = makeCastNode(tTok, allowrefcast, typeNode, rtnNode);
	    if(!castok)
	      {
		delete typeNode; //owns the dangling type descriptor
		typeNode = NULL;
	      }
	  }
	else
	  {
	    //clean up, some kind of error parsing min/max/sizeof
	    delete typeNode;
	    typeNode = NULL;
	  }
      }
    return rtnNode; //rtnNode could be NULL!
  } //parseFactorStartingWithAType

  Node * Parser::parseRestOfFactor(Node * leftNode)
  {
    if(leftNode == NULL)
      return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    unreadToken();

    switch(pTok.m_type)
      {
      case TOK_KW_IS:
	rtnNode = makeConditionalExprNode(leftNode);
	break;
      case TOK_DOT:
	rtnNode = parseRestOfMemberSelectExpr(leftNode); //t3905
	break;
      case TOK_PLUS_PLUS: //t3903
      case TOK_MINUS_MINUS: //t3903
	rtnNode = makeAssignExprNode(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //any more?
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

  Node * Parser::parseRestOfCastOrExpression(bool allowRefCast)
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
	rtnNode = parseFactorStartingWithAType(tTok, allowRefCast);
	isacast = (rtnNode && rtnNode->isExplicitCast());
      }

    if(isacast)
      return rtnNode; //no close paren

    if(rtnNode)
      {
	//not a cast or min/max/sizeof or named constant
	rtnNode = parseRestOfAssignExpr(rtnNode);
      }
    else
      {
	rtnNode = parseAssignExpr(); //grammar says assign_expr
	//will parse rest of assign expr before close paren
      }

    if(!getExpectedToken(TOK_CLOSE_PAREN))
      {
	delete rtnNode;
	rtnNode = NULL;
      }
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
    if(leftNode == NULL)
      return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    unreadToken();

    switch(pTok.m_type)
      {
      case TOK_AMP_AMP:
      case TOK_PIPE_PIPE:
	rtnNode = makeExpressionNode(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //recursion of left-associativity
	break;
      case TOK_AMP:
      case TOK_PIPE:
      case TOK_HAT:
	rtnNode = parseRestOfLogicalExpression(leftNode); //addOp
	rtnNode = parseRestOfExpression(rtnNode);
	break;
      case TOK_EQUAL_EQUAL:
      case TOK_NOT_EQUAL:
	rtnNode = parseRestOfBitExpression(leftNode);
	rtnNode = parseRestOfExpression(rtnNode);
	break;
      case TOK_LESS_THAN:
      case TOK_LESS_EQUAL:
      case TOK_GREATER_THAN:
      case TOK_GREATER_EQUAL:
	rtnNode = parseRestOfEqExpression(leftNode);
	rtnNode = parseRestOfExpression(rtnNode);
	break;
      case TOK_SHIFT_LEFT:
      case TOK_SHIFT_RIGHT:
	rtnNode = parseRestOfCompareExpression(leftNode);
	rtnNode = parseRestOfExpression(rtnNode);
	break;
      case TOK_PLUS:
      case TOK_MINUS:
	rtnNode = parseRestOfShiftExpression(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_STAR:
      case TOK_SLASH:
      case TOK_PERCENTSIGN:
	rtnNode = parseRestOfTerm(leftNode); //mulOp
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_KW_IS:
	rtnNode = parseRestOfFactor(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_QUESTION:
	rtnNode = parseRestOfQuestionColonExpr(leftNode);
	break;
      case TOK_DOT:
	rtnNode = parseRestOfMemberSelectExpr(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //any more? t41057
	break;
      case TOK_OPEN_SQUARE:
	rtnNode = parseRestOfLvalExpr(leftNode); //t41074, t3941
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_ERROR_LOWLEVEL:
	getNextToken(pTok); //reread
	rtnNode = parseRestOfExpression(leftNode); //redo
	break;
      default:
	rtnNode = leftNode;
      };
    return rtnNode;
  } //parseRestOfExpression

  Node * Parser::parseRestOfLvalExpr(Node * leftNode)
  {
    if(leftNode == NULL)
      return NULL;

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

    Node * rightNode = parseExpression();
    //Array size may be blank if initialized (not array item!!);
    rtnNode = new NodeSquareBracket(leftNode, rightNode, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(pTok.m_locator);

    if(!getExpectedToken(TOK_CLOSE_SQUARE))
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
    if(leftNode == NULL)
      return NULL;

    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);
    unreadToken();

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
	rtnNode = makeAssignExprNode(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_SEMICOLON:
      case TOK_CLOSE_PAREN:
      case TOK_COMMA: /* array item init */
	{
	  rtnNode = leftNode;
	  break;
	}
      case TOK_DOT:
	{
	  rtnNode = parseRestOfMemberSelectExpr(leftNode); //t3905
	  rtnNode = parseRestOfAssignExpr(rtnNode);
	  break;
	}
      case TOK_SQUARE:
      case TOK_OPEN_SQUARE:
	{
	  rtnNode = parseRestOfLvalExpr(leftNode); //t41074, lhs
	  rtnNode = parseRestOfAssignExpr(rtnNode); //any more?
	  break;
	}
      default:
	{
	  //or duplicate restofexpression cases here? seems silly..
	  rtnNode = parseRestOfExpression(leftNode); //any more?
	  break;
	}
      };
    return rtnNode;
  } //parseRestOfAssignExpr

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
	args.m_declRef = ALT_REF;
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
    else if(args.m_declRef == ALT_REF)
      {
	//assignments required for references
	std::ostringstream msg;
	msg << "Must assign to reference <" << m_state.getTokenDataAsString(identTok).c_str();
	msg << "> at the time of its declaration";
	MSG(&eTok, msg.str().c_str(), ERR);
	getTokensUntil(TOK_SEMICOLON); //rest of statement is ignored.
	unreadToken();
	return false; //done
      }
    else if(eTok.m_type == TOK_OPEN_PAREN)
      {
	if(Token::getSpecialTokenWork(args.m_typeTok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    std::ostringstream msg;
	    msg << "Unexpected token <" << m_state.getTokenDataAsString(eTok).c_str();
	    msg << "> indicates a constructor function call or function definition; neither are valid ";
	    msg << "here for primitive variable '";
	    msg << m_state.getTokenDataAsString(identTok).c_str() << "'";
	    MSG(&eTok, msg.str().c_str(), ERR); //t41082
	    unreadToken();
	    return false; //done
	  }
      }

    //continuing..
    bool brtn = true;
    //update dNode with init expression: lval for ref, assign for local car
    if(args.m_declRef == ALT_REF)
      {
	brtn = parseRestOfRefInitialization(identTok, dNode);
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

  bool Parser::parseRestOfRefInitialization(const Token& identTok, NodeVarDecl * dNode)
  {
    bool brtn = true;
    SYMBOLTYPEFLAG saveTheFlag = m_state.m_parsingVariableSymbolTypeFlag;
    if(m_state.m_parsingVariableSymbolTypeFlag == STF_FUNCLOCALVAR)
      {
	m_state.m_parsingVariableSymbolTypeFlag = STF_FUNCLOCALREF;
      }

    Node * initnode = parseExpression();
    if(!initnode)
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

  Node * Parser::parseArrayInitialization(u32 identId)
  {
    Token aTok;
    getNextToken(aTok);

    assert(aTok.m_type == TOK_OPEN_CURLY);
    unreadToken();

    NodeListArrayInitialization * rtnList = new NodeListArrayInitialization(m_state); //delete if error
    rtnList->setNodeLocation(aTok.m_locator);
    assert(rtnList);

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
    else if((aTok.m_type != TOK_COMMA))
      {
	u32 n = rtnList->getNumberOfNodes();
	if(!(n == 0 && (aTok.m_type == TOK_OPEN_CURLY)))
	  {
	    unreadToken();

	    std::ostringstream msg;
	    msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(aTok).c_str();
	    msg << "> while parsing array variable " << m_state.m_pool.getDataAsString(identId).c_str();
	    msg << ", item " << (n + 1);
	    MSG(&aTok, msg.str().c_str(), ERR);
	    return false; //original caller owns rtnList, should delete if empty!
	  }
      }
    //else continue..

    Node * assignNode = parseAssignExpr();
    if(!assignNode)
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
	    exprNode = parseArrayInitialization(constId); //returns a NodeListArrayInitialization
	  }
	else
	  exprNode = parseExpression(); //makeAssignExprNode(leftNode);

	if(exprNode)
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
    Token pTok;
    if(getExpectedToken(TOK_EQUAL, pTok, QUIETLY))
      {
	Node * exprNode = parseExpression();
	if(exprNode)
	  paramNode->setConstantExpr(exprNode);
	else
	  {
	    std::ostringstream msg;
	    msg << "Missing model parameter definition after '=' for '";
	    msg << m_state.m_pool.getDataAsString(paramNode->getSymbolId()).c_str() << "'";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Missing '=' after model parameter definition";
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
	msg << m_state.m_pool.getDataAsString(identTok.m_dataindex).c_str() << "'";
	msg << " function block";
	MSG(&identTok, msg.str().c_str(), ERR);

	m_state.popClassContext(); //m_currentBlock = prevBlock;
	return NULL;
      }


    assert(isConstr || nodetype);
    //internal constructor return type is void
    UTI rtnuti = isConstr ? Void : nodetype->givenUTI();

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

    //set class type to custom array; the current class block
    //node type was set to its class symbol type after checkAndLabelType
    // caType is the return type of the 'aget' method (set here).
    if(m_state.getCustomArrayGetFunctionNameId() == identTok.m_dataindex)
      {
	UTI cuti = currClassBlock->getNodeType(); //prevBlock
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	((UlamTypeClass *) cut)->setCustomArray();
      }

    //Here before push to get correct class block NodeNo
    //Now, look specifically for a function with the same given name defined
    Symbol * fnSym = NULL;
    if(!currClassBlock->isFuncIdInScope(identTok.m_dataindex, fnSym))
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
    u32 selfid = m_state.m_pool.getIndexForDataString("self");
    UTI cuti = currClassBlock->getNodeType(); //luckily we know this now for each class used
    assert(m_state.okUTItoContinue(cuti));
    Token selfTok(TOK_IDENTIFIER, identTok.m_locator, selfid);
    SymbolVariableStack * selfsym = new SymbolVariableStack(selfTok, m_state.getUlamTypeAsRef(cuti, ALT_REF), m_state);
    selfsym->setAutoLocalType(ALT_REF);
    selfsym->setIsSelf();
    m_state.addSymbolToCurrentScope(selfsym); //ownership goes to the funcdef block

    //wait until c&l to create "super" symbol for the Super class type;
    //btw, it's really a ref. belongs to the function definition scope.

    //parse and add parameters to function symbol (not in ST yet!)
    //intentionally keeps going regardless of errors for better messages.
    parseRestOfFunctionParameters(fsymptr, rtnNode);

    if(rtnNode)
      {
	u32 numParams = fsymptr->getNumberOfParameters();
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
	    bool isAdded = ((SymbolFunctionName *) fnSym)->overloadFunction(fsymptr);
	    if(!isAdded)
	      {
		//this is a duplicate function definition with same parameters and given name!!
		//return types may differ
		std::ostringstream msg;
		msg << "Duplicate defined function '";
		msg << m_state.m_pool.getDataAsString(fsymptr->getId());
		msg << "' with the same parameters" ;
		MSG(&args.m_typeTok, msg.str().c_str(), ERR);
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
		msg << "Variable args (...) supported for native functions only; not <";
		msg << m_state.m_pool.getDataAsString(fsymptr->getId()).c_str() << ">";
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
    else if(Token::isTokenAType(pTok) || (pTok.m_type == TOK_KW_LOCALDEF))
      {
	//local.Type allowed (t3870,71)
	unreadToken();
	NodeVarDecl * argNode = parseFunctionParameterDecl();
	Symbol * argSym = NULL;

	//could be null symbol already in scope
	if(argNode)
	  {
	    //parameter IS a variable (declaration).
	    //ownership stays with NodeBlockFunctionDefinition's ST
	    if(argNode->getSymbolPtr(argSym))
	      fsym->addParameterSymbol(argSym);
	    else
	      MSG(&pTok, "No symbol from parameter declaration", ERR);

	    //potentially needed to resolve its node type
	    fblock->addParameterNode(argNode); //transfer owner

	    if(fsym->takesVariableArgs() && argSym)
	      {
		std::ostringstream msg;
		msg << "Parameter <";
		msg << m_state.m_pool.getDataAsString(argSym->getId()).c_str();
		msg << "> appears after ellipses (...)";
		MSG(&pTok, msg.str().c_str(), ERR);
	      }
	  }
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

    if(!typeNode)
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
	msg << "Function parameter <" << m_state.getTokenDataAsString(iTok).c_str();
	if((Token::getSpecialTokenWork(iTok.m_type) == TOKSP_KEYWORD))
	  msg << ">: Name must not be a reserved keyword";
	else
	  msg << ">: Name must begin with a lower-case letter";
	MSG(&iTok, msg.str().c_str(), ERR);
	unreadToken();
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
	    msg << "Pure Virtual Function <" << funcNode->getName() << ">";
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
	msg << "Function <" << m_state.getTokenDataAsString(identTok).c_str();
	msg << "> is not a valid (lower case) name";
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
	std::ostringstream msg;
	msg << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
	msg << " cannot be used again as a function, it has a previous definition as '";
	msg << m_state.getUlamTypeNameByIndex(asymptr->getUlamTypeIdx()).c_str();
	msg << " " << m_state.m_pool.getDataAsString(asymptr->getId()).c_str() << "'";
	MSG(&args.m_typeTok, msg.str().c_str(), ERR);

	//eat tokens until end of definition? which ending brace?
	delete nodetype;
	return NULL;
      }

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

    if(identTok.m_dataindex == m_state.m_pool.getIndexForDataString("self"))
      {
	std::ostringstream msg;
	msg << "The keyword 'self' may not be used as a variable name";
	MSG(&identTok, msg.str().c_str(), ERR);
	//return NULL;  keep going?
      }

    NodeVarDecl * rtnNode = NULL;
    Node * lvalNode = parseLvalExpr(identTok); //for optional [] array size
    if(lvalNode)
      {
	//lvalNode could be either a NodeIdent or a NodeSquareBracket
	//process identifier...check if already defined in current scope; if not, add it;
	//returned symbol could be symbolVariable or symbolFunction, detect first.
	Symbol * asymptr = NULL;
	if(!lvalNode->installSymbolVariable(args, asymptr))
	  {
	    if(asymptr)
	      {
		std::ostringstream msg;
		msg << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
		msg << " has a previous declaration as '";
		msg << m_state.getUlamTypeNameByIndex(asymptr->getUlamTypeIdx()).c_str();
		msg << "' and cannot be used as a variable";
		MSG(&args.m_typeTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (e.g. problem with []);
		// rtnNode is NULL; (t3255)
		std::ostringstream msg;
		msg << "Invalid variable declaration of base type <";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str() << "> and name <";
		msg << m_state.getTokenDataAsString(identTok).c_str() << "> (missing symbol)";
		MSG(&args.m_typeTok, msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    UTI auti = asymptr->getUlamTypeIdx();
	    //chain to NodeType descriptor if array (i.e. non scalar), o.w. delete lval
	    linkOrFreeConstantExpressionArraysize(auti, args, (NodeSquareBracket *)lvalNode, nodetyperef);

	    ALT refalt = m_state.getReferenceType(auti);
	    if(refalt != ALT_NOT)
	      nodetyperef->setReferenceType(refalt, args.m_referencedUTI, auti); //invariant

	    // tfr owner of nodetyperef to node var decl
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

	if(!rtnNode)
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
		msg << m_state.m_pool.getDataAsString(asymid).c_str();
		msg << " has a previous declaration as '";
		msg << m_state.getUlamTypeNameBriefByIndex(auti).c_str();
		msg << "' and cannot be used as a typedef";
		MSG(&args.m_typeTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (t3254)
		//(e.g. problem with []) , error already output. rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid typedef of base type <";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str();
		msg << "> and name <" << m_state.getTokenDataAsString(identTok).c_str();
		msg << "> (missing symbol)";
		MSG(&identTok, msg.str().c_str(), ERR);
	      }
	    m_state.clearStructuredCommentToken();
	  }

	if(aok)
	  {
	    UTI auti = asymptr->getUlamTypeIdx();
	    //chain to NodeType descriptor if array (i.e. non scalar), o.w. delete lval
	    linkOrFreeConstantExpressionArraysize(auti, args, (NodeSquareBracket *)lvalNode, nodetyperef);
	    // tfr owner of nodetyperef to node typedef
	    ALT refalt = m_state.getReferenceType(auti);
	    if(refalt != ALT_NOT)
	      nodetyperef->setReferenceType(refalt, args.m_referencedUTI, auti); //invariant

	    rtnNode =  new NodeTypedef((SymbolTypedef *) asymptr, nodetyperef, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(args.m_typeTok.m_locator);
	    asymptr->setStructuredComment(); //also clears
	  }

	if(!rtnNode)
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
	    if(asymptr)
	      {
		std::ostringstream msg;
		msg << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
		msg << " has a previous declaration as '";
		msg << m_state.getUlamTypeNameByIndex(asymptr->getUlamTypeIdx()).c_str();
		msg << "' and cannot be used as a named constant";
		MSG(&args.m_typeTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (e.g. problem with []),
		//error already output. rtnNode is NULL; (t3446, t3451, t3460)
		std::ostringstream msg;
		msg << "Invalid constant definition of type <";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str();
		msg << "> and name <" << m_state.getTokenDataAsString(identTok).c_str();
		msg << ">";
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

	    NodeConstantDef * constNode =  new NodeConstantDef((SymbolConstantValue *) asymptr, nodetyperef, m_state);
	    assert(constNode);
	    constNode->setNodeLocation(args.m_typeTok.m_locator);
	    if(args.m_isStmt)
	      asymptr->setStructuredComment(); //also clears

	    if(m_state.m_parsingVariableSymbolTypeFlag == STF_DATAMEMBER)
	      asymptr->setDataMemberClass(m_state.getCompileThisIdx()); //t3862, t3865, t3868

	    rtnNode = parseRestOfConstantDef(constNode, args.m_assignOK, args.m_isStmt);

	    if(!rtnNode)
	      m_state.clearStructuredCommentToken(); //t3524
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
	    if(asymptr)
	      {
		std::ostringstream msg;
		msg << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
		msg << " has a previous declaration as '";
		msg << m_state.getUlamTypeNameByIndex(asymptr->getUlamTypeIdx()).c_str();
		msg << "' and cannot be used as a Model Parameter data member";
		MSG(&args.m_typeTok, msg.str().c_str(), ERR);
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

    //reparse (is-has-as) function token
    Token fTok;
    getNextToken(fTok);

    //parse Type
    Token tTok;
    getNextToken(tTok);
    unreadToken();

    TypeArgs typeargs;

    //allow types with preceeding dots (e.g. from another class)
    //t3407, 3826,27,30; t3868,61,62
    NodeTypeDescriptor * typeNode = parseTypeDescriptorIncludingLocalsScope(typeargs, true, false);

    if(!typeNode)
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

    Node * rightNode = parseAssignExpr();
    if(!rightNode)
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

  NodeBinaryOp * Parser::makeExpressionNode(Node * leftNode)
  {
    NodeBinaryOp * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * rightNode = parseLogicalExpression();
    if(!rightNode)
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
    if(!rightNode)
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
    if(!rightNode)
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
    if(!rightNode)
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
    if(!rightNode)
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
    if(!rightNode)
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
    if(!rightNode)
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
    if(!factorNode)
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
	msg << " (including function call arguments); not in this context";
	MSG(&typeTok, msg.str().c_str(), ERR);
	return false;
      } //reference cast

    if(getExpectedToken(TOK_CLOSE_PAREN))
      {
	Node * nodetocast = parseFactor();
	if(nodetocast)
	  {
	    rtnNode = new NodeCast(nodetocast, typeToBe, typeNode, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(typeNode->getNodeLocation());
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

  void Parser::linkOrFreeConstantExpressionArraysize(UTI auti, TypeArgs args, NodeSquareBracket * ceForArraySize, NodeTypeDescriptor *& nodetyperef)
  {
    //auti is incomplete.

    s32 arraysize = m_state.getArraySize(auti);
    //link arraysize subtree for arraytype based on scalar from another class, OR
    // a local arraytype based on a local scalar uti; o.w. delete.
    // don't keep the ceForArraySize if the type belongs to another class!
    // we link an array type to its scalar type

    if(arraysize != UNKNOWNSIZE)
      {
	delete ceForArraySize;
	return;
      }

    //typedef is an array from another class
    if((args.m_anothertduti == Nouti) && (args.m_anothertduti == auti))
      {
	delete ceForArraySize;
	return;
      }

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
	    msg << "Unexpected token <" << pTok.getTokenEnumNameFromPool(&m_state).c_str() << ">; Expected ";
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
  } //initPrimitiveUlamTypes

} //end MFM
