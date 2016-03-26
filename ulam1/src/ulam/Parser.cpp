#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include "Parser.h"
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
#include "NodeBinaryOpEqualArithMultiply.h"
#include "NodeBinaryOpEqualArithSubtract.h"
#include "NodeBinaryOpEqualBitwiseAnd.h"
#include "NodeBinaryOpEqualBitwiseOr.h"
#include "NodeBinaryOpEqualBitwiseXor.h"
#include "NodeBinaryOpLogicalAnd.h"
#include "NodeBinaryOpLogicalOr.h"
#include "NodeBinaryOpShiftLeft.h"
#include "NodeBinaryOpShiftRight.h"
#include "NodeBlock.h"
#include "NodeBlockEmpty.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeBreakStatement.h"
#include "NodeCast.h"
#include "NodeConditionalAs.h"
#include "NodeConditionalIs.h"
#include "NodeConstant.h"
#include "NodeContinueStatement.h"
#include "NodeControlIf.h"
#include "NodeControlWhile.h"
#include "NodeIdent.h"
#include "NodeInstanceof.h"
#include "NodeLabel.h"
#include "NodeMemberSelect.h"
#include "NodeModelParameter.h"
#include "NodeReturnStatement.h"
#include "NodeStorageof.h"
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
#include "SymbolParameterValue.h"
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
	ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(cuti)->getUlamClass();
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

    if((pTok.m_type != TOK_KW_ELEMENT) && (pTok.m_type != TOK_KW_QUARK) && (pTok.m_type != TOK_KW_QUARKUNION))
      {
	std::ostringstream msg;
	msg << "Invalid Class Type <";
	msg << m_state.getTokenDataAsString(&pTok).c_str();
	msg << ">; KEYWORD should be '";
	msg << Token::getTokenAsStringFromPool(TOK_KW_ELEMENT, &m_state).c_str();
	msg << "', '";
	msg << Token::getTokenAsStringFromPool(TOK_KW_QUARK, &m_state).c_str();
	msg << "', or '";
	msg << Token::getTokenAsStringFromPool(TOK_KW_QUARKUNION, &m_state).c_str();
	msg << "'";
	MSG(&pTok, msg.str().c_str(), ERR);
	m_state.clearStructuredCommentToken();
	return true; //we're done.
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
	msg << "Poorly named class '" << m_state.getTokenDataAsString(&iTok).c_str();
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

    //mostly needed for code gen later, but this is where we first know it!
    m_state.pushClassContext(cnSym->getUlamTypeIdx(), cnSym->getClassBlockNode(), cnSym->getClassBlockNode(), false, NULL); //null blocks likely

    //set class type in UlamType (through its class symbol) since we know it;
    //UC_UNSEEN if unseen so far.
    m_state.resetUnseenClass(cnSym, iTok);

    switch(pTok.m_type)
      {
      case TOK_KW_ELEMENT:
	{
	  cnSym->setUlamClass(UC_ELEMENT);
	  break;
	}
      case TOK_KW_QUARK:
	{
	  cnSym->setUlamClass(UC_QUARK);
	  break;
	}
      case TOK_KW_QUARKUNION:
	{
	  cnSym->setUlamClass(UC_QUARK);
	  cnSym->setQuarkUnion();
	  break;
	}
      default:
	assert(0); //checked prior
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
	//reset to incomplete
	cnSym->setUlamClass(UC_UNSEEN);
	cnSym->setClassBlockNode(NULL);
	std::ostringstream msg;
	msg << "Empty/Incomplete Class Definition '";
	msg << m_state.getTokenDataAsString(&iTok).c_str();
	msg << "'; Possible missing ending curly brace";
	MSG(&pTok, msg.str().c_str(), ERR);
      }

    return false; //keep going until EOF is reached
  } //parseThisClass

  NodeBlockClass * Parser::parseClassBlock(SymbolClassName * cnsym, Token identTok)
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
      }


    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    //class block has 2 ST: functions and data member decls, separate
    //m_state.popClassContext(); //keep on stack for name id
    m_state.pushClassContext(cnsym->getUlamTypeIdx(), rtnNode, rtnNode, false, NULL);

    //automatically create a Self typedef symbol for this class type
    u32 selfid = m_state.m_pool.getIndexForDataString("Self");
    Token selfTok(TOK_TYPE_IDENTIFIER, identTok.m_locator, selfid);
    SymbolTypedef * symtypedef = new SymbolTypedef(selfTok, utype, utype, m_state); //refselftype
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
	    assert(supercsym);
	    NodeBlockClass * superclassblock = supercsym->getClassBlockNode();
	    assert(superclassblock);

	    //set super class' block after any parameters parsed;
	    // (separate from previous block which might be pointing to template
	    //  in case of a stub)
	    rtnNode->setSuperBlockPointer(NULL); //wait for c&l

	    //rearrange order of class context so that super class is traversed after subclass
	    m_state.popClassContext(); //m_currentBlock = prevBlock;
	    m_state.pushClassContext(superuti, superclassblock, superclassblock, false, NULL);
	    m_state.pushClassContext(cnsym->getUlamTypeIdx(), rtnNode, rtnNode, false, NULL); //redo

	    //automatically create a Super typedef symbol for this class' super type
	    u32 superid = m_state.m_pool.getIndexForDataString("Super");
	    Token superTok(TOK_TYPE_IDENTIFIER, qTok.m_locator, superid);
	    SymbolTypedef * symtypedef = new SymbolTypedef(superTok, superuti, superuti, m_state);
	    m_state.addSymbolToCurrentScope(symtypedef);
	  }
	//else errors may have occurred
      }
    else
      {
	unreadToken();
	cnsym->setSuperClass(Nouti); //clear
      }

    if(!getExpectedToken(TOK_OPEN_CURLY, pTok))
      {
	if(pTok.m_type == TOK_COLON)
	  {
	    std::ostringstream msg;
	    msg << "Inheritance for template class identifier '";
	    msg << m_state.getTokenDataAsString(&identTok).c_str();
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

    //keep the data member var decls, starting with NodeBlockClass, keeping it for return
    NodeStatements * nextNode = rtnNode;
    NodeStatements * stmtNode = NULL;

    while(parseDataMember(stmtNode))
      {
	//stmtNode could be false, in case of function def
	if(stmtNode)
	  {
	    nextNode->setNextNode(stmtNode);
	    nextNode = stmtNode;
	    stmtNode = NULL;
	  }
      }

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
    //the argument to parseDecl will prevent it from looking
    //for restofdecls
    if(Token::isTokenAType(pTok))
      {
	unreadToken();

	//once a parameter has a default value expression
	// subsequent parameters must also to avoid ambiguity when instaniated
	u32 numparams = cntsym->getNumberOfParameters();
	bool assignrequired = ( (numparams == 0) ? NOASSIGNOK : cntsym->parameterHasDefaultValue(numparams - 1));
	Node * argNode = parseConstdef(assignrequired, false); //2nd arg->not statement
	Symbol * argSym = NULL;

	//could be null symbol already in scope
	if(argNode)
	  {
	    //parameter IS a NodeConstantdef
	    if(argNode->getSymbolPtr(argSym))
	      {
		((SymbolConstantValue *) argSym)->setParameterFlag();
		//ownership stays with NodeBlockClass's ST
		cntsym->addParameterSymbol((SymbolConstantValue *) argSym);
	      }
	    else
	      MSG(&pTok, "No symbol from class parameter declaration", ERR);

	    //potentially needed to resolve its node type
	    cblock->addParameterNode(argNode);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Expected a 'Type' Token!! got Token '";
	msg << m_state.getTokenDataAsString(&pTok).c_str();
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

    if(iTok.m_type == TOK_TYPE_IDENTIFIER)
      {
	bool isaclass = true;
	superuti = parseClassArguments(iTok, isaclass);
	if(superuti != Nav)
	  {
	    cnsym->setSuperClassForClassInstance(superuti, cnsym->getUlamTypeIdx()); //set here!!
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(superuti, supercsym);
	    assert(isDefined);
	    rtninherits = true;
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Class Definition '";
	msg << m_state.getUlamTypeNameBriefByIndex(cnsym->getId()).c_str();
	msg << "'; Inheritance from invalid Class identifier '";
	msg << m_state.getTokenDataAsString(&iTok).c_str() << "'";
	MSG(&iTok, msg.str().c_str(), ERR);
      }
    return rtninherits;
  } //parseRestOfClassInheritance

  bool Parser::parseDataMember(NodeStatements *& nextNode)
  {
    bool brtn = false;
    Node * rtnNode = NULL;
    bool isFunc = false;
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
	rtnNode = parseTypedef();
      }
    else if(pTok.m_type == TOK_KW_CONSTDEF)
      {
	//parse Named Constant starting with keyword first
	rtnNode = parseConstdef();
      }
    else if(pTok.m_type == TOK_KW_PARAMETER)
      {
	//static model parameter for elements, not quarks
	rtnNode = parseParameter();
      }
    else //regular data member: function or variable
      {
	bool isVirtual = false;
	if(pTok.m_type == TOK_KW_VIRTUAL)
	  isVirtual = true;
	else
	  unreadToken(); //put back for parsing type descriptor

	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptor(typeargs, true);

	if(typeNode == NULL)
	  return false; //expecting a type, not sizeof, probably an error!

	Token iTok;
	getNextToken(iTok);
	if(iTok.m_type != TOK_IDENTIFIER)
	  {
	    //user error!
	    if(iTok.m_type == TOK_AMP)
	      {
		std::ostringstream msg;
		msg << "Reference as data member or function return type; Not supported";
		MSG(&iTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Name of variable/function <";
		msg << m_state.getTokenDataAsString(&iTok).c_str();
		msg << ">: Identifier must begin with a lower-case letter";
		MSG(&iTok, msg.str().c_str(), ERR);
	      }
	    m_state.clearStructuredCommentToken();
	    delete typeNode;
	    typeNode = NULL;
	    unreadToken();
	    return false; //done!
	  }

	//need next token to distinguish a function from a variable declaration (quietly)
	if(getExpectedToken(TOK_OPEN_PAREN))
	  {
	    isFunc = true;
	    //eats the '(' when found; NULL if error occurred
	    rtnNode = makeFunctionSymbol(typeargs, iTok, typeNode, isVirtual); //with params
	    if(rtnNode)
	      brtn = true; //rtnNode belongs to the symbolFunction
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
		msg << m_state.getTokenDataAsString(&pTok).c_str() << ">";
		msg << " applies to functions";
		MSG(&pTok, msg.str().c_str(), ERR);
		//continue
	      }

	    //not '(', so token is unread, and we know
	    //it's a variable, not a function; also handles arrays
	    UTI passuti = typeNode->givenUTI(); //before it may become an array
	    rtnNode = makeVariableSymbol(typeargs, iTok, typeNode);

	    if(rtnNode)
	      {
		parseRestOfDataMemberAssignment(typeargs, iTok, rtnNode, passuti);
		rtnNode = parseRestOfDataMember(typeargs, iTok, rtnNode, passuti);
	      }
	    else
	      m_state.clearStructuredCommentToken();
	  }
      } //regular data member

    //common end processing, except for function defs
    if(rtnNode && !isFunc)
      {
	if(!getExpectedToken(TOK_SEMICOLON))
	  {
	    delete rtnNode;
	    rtnNode = NULL;
	    getTokensUntil(TOK_SEMICOLON); //does this help?
	  }
	else
	  {
	    if(rtnNode)
	      {
		brtn = true;
		nextNode = new NodeStatements(rtnNode, m_state);
		assert(nextNode);
		nextNode->setNodeLocation(rtnNode->getNodeLocation());
	      }
	  }
      }
    return brtn;
  } //parseDataMember

  Node * Parser::parseRestOfDataMember(TypeArgs& args, Token identTok, Node * dNode, UTI passuti)
  {
    Token pTok;
    getNextToken(pTok);

    args.m_arraysize = NONARRAYSIZE; //clear for decl list (args ref)

    if(pTok.m_type != TOK_COMMA)
      {
	unreadToken();
	return dNode;
      }

    Node * rtnNode = dNode;
    Token iTok;
    getNextToken(iTok);
    if(iTok.m_type == TOK_IDENTIFIER)
      {
	//just the top level as a basic uti (no selects, or arrays)
	NodeTypeDescriptor * typeNode = new NodeTypeDescriptor(args.m_typeTok, passuti, m_state);
	//another decl of same type
	Node * sNode = makeVariableSymbol(args, iTok, typeNode); //a decl
	if (sNode)
	  {
	    parseRestOfDataMemberAssignment(args, identTok, sNode, passuti);

	    rtnNode =  new NodeStatements(dNode, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(dNode->getNodeLocation());

	    NodeStatements * nextNode = new NodeStatements(sNode, m_state);
	    assert(nextNode);
	    nextNode->setNodeLocation(dNode->getNodeLocation());
	    ((NodeStatements *) rtnNode)->setNextNode(nextNode);
	  }
	//else  error?
      }
    else
      {
	//perhaps read until semi-colon
	getTokensUntil(TOK_SEMICOLON);
	unreadToken();
      }
    return parseRestOfDataMember(args, iTok, rtnNode, passuti); //iTok in case of =
  } //parseRestOfDataMember

  void Parser::parseRestOfDataMemberAssignment(TypeArgs& args, Token identTok, Node * dNode, UTI passuti)
  {
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_EQUAL)
      {
	Node * initnode = parseExpression();
	if(initnode)
	  ((NodeVarDeclDM*) dNode)->setInitExpr(initnode);
	//else error
      }
    else
      unreadToken();

    return;
  } //parseRestOfDataMemberAssignment

  Node * Parser::parseBlock()
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

    NodeStatements * nextNode = (NodeStatements *) parseStatements();

    if(nextNode) //could be Null, in case of errors
      rtnNode->setNextNode(nextNode);
    else
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
	m_state.pushCurrentBlock(rtnNode); //very temporary
      }

    //this block's ST is no longer in scope
    NodeBlock * currBlock = m_state.getCurrentBlock();
    if(currBlock)
      m_state.m_currentFunctionBlockDeclSize -= currBlock->getSizeOfSymbolsInTable();

    m_state.popClassContext();
    currBlock = NULL; //no longer valid; don't need

    //sanity check
    assert(!rtnNode || rtnNode->getPreviousBlockPointer() == prevBlock);
    return rtnNode;
  } //parseBlock

  Node * Parser::parseStatements()
  {
    Node * sNode = parseStatement();
    if(!sNode)  //e.g. due to an invalid token perhaps
      {
	MSG("", "EMPTY STATEMENT!! when in doubt, count", DEBUG);

	Token pTok;
	getNextToken(pTok);
	if(pTok.m_type != TOK_CLOSE_CURLY && pTok.m_type != TOK_EOF)
	  {
	    unreadToken();
	    return parseStatements(); //try again
	  }
	else
	  {
	    unreadToken();
	    return NULL;
	  }
      }

    NodeStatements * rtnNode = new NodeStatements(sNode, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(sNode->getNodeLocation());

    if(!getExpectedToken(TOK_CLOSE_CURLY))
      {
	rtnNode->setNextNode((NodeStatements *) parseStatements()); //more statements
      }
    else
      unreadToken();
    return rtnNode;
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
	rtnNode = parseSimpleStatement(); //may be null (only ;)
      }
    return rtnNode;
  } //parseStatement

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
	  m_state.m_parsingControlLoop = m_state.getNextTmpVarNumber(); //true
	  rtnNode = parseControlWhile(pTok);
	  m_state.m_parsingControlLoop = 0;
	}
	break;
      case TOK_KW_FOR:
	{
	  m_state.m_parsingControlLoop = m_state.getNextTmpVarNumber(); //true
	  rtnNode = parseControlFor(pTok);
	  m_state.m_parsingControlLoop = 0;
	}
	break;
      case TOK_ERROR_LOWLEVEL:
	//eat error token
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token <";
	  msg << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  //unreadToken(); leads to infinite loop
	}
	break;
      };
    return rtnNode;
  } //parseControlStatement

  Node * Parser::parseControlIf(Token ifTok)
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
	m_state.popClassContext(); //the pop
	return NULL; //stop this maddness
      }

    if(!getExpectedToken(TOK_CLOSE_PAREN))
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	return NULL; //stop this maddness
      }

    Node * trueNode = NULL;
    if(m_state.m_parsingConditionalAs)
      {
	trueNode = setupAsConditionalBlockAndParseStatements((NodeConditional *) condNode);
      }
    else
      {
	trueNode = parseStatement();
      }

    if(!trueNode)
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	return NULL; //stop this maddness
      }

    //wrapping body in NodeStatements produces proper comment for genCode
    NodeStatements * trueStmtNode = new NodeStatements(trueNode, m_state);
    assert(trueStmtNode);
    trueStmtNode->setNodeLocation(trueNode->getNodeLocation());

    Node * falseStmtNode = NULL;

    if(getExpectedToken(TOK_KW_ELSE))
      {
	Node * falseNode = parseStatement();
	if(falseNode != NULL)
	  {
	    falseStmtNode = new NodeStatements(falseNode, m_state);
	    assert(falseStmtNode);
	    falseStmtNode->setNodeLocation(falseNode->getNodeLocation());
	  }
      }

    Node * ifNode = new NodeControlIf(condNode, trueStmtNode, falseStmtNode, m_state);
    assert(ifNode);
    ifNode->setNodeLocation(ifTok.m_locator);


    NodeStatements * nextControlNode = new NodeStatements(ifNode, m_state);
    assert(nextControlNode);
    nextControlNode->setNodeLocation(ifNode->getNodeLocation());

    rtnNode->setNextNode(nextControlNode); //***link if to rtn block

    //this block's ST is no longer in scope
    currBlock = m_state.getCurrentBlock(); //reload
    if(currBlock)
      m_state.m_currentFunctionBlockDeclSize -= currBlock->getSizeOfSymbolsInTable();

    m_state.popClassContext(); //= prevBlock;

    return rtnNode;
  } //parseControlIf

  Node * Parser::parseControlWhile(Token wTok)
  {
    if(!getExpectedToken(TOK_OPEN_PAREN))
      {
	return NULL;
      }

    //before parsing the IF statement, need a new scope
    NodeBlock * currBlock = m_state.getCurrentBlock();
    NodeBlock * rtnNode = new NodeBlock(currBlock, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(wTok.m_locator);

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    m_state.pushCurrentBlock(rtnNode); //without pop first


    s32 controlLoopLabelNum = m_state.m_parsingControlLoop; //save at the top
    Node * condNode = parseConditionalExpr();
    if(!condNode)
      {
	m_state.popClassContext(); //the pop
	return NULL; //stop this maddness
      }

    if(!getExpectedToken(TOK_CLOSE_PAREN))
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	return NULL; //stop this maddness
      }

    Node * trueNode = NULL;
    if(m_state.m_parsingConditionalAs)
      trueNode = setupAsConditionalBlockAndParseStatements((NodeConditional *) condNode);
    else
      trueNode = parseStatement();

    if(!trueNode)
      {
	delete condNode;
	m_state.popClassContext(); //the pop
	return NULL; //stop this maddness
      }

    //wrapping body in NodeStatements produces proper comment for genCode
    NodeStatements * trueStmtNode = new NodeStatements(trueNode, m_state);
    assert(trueStmtNode);
    trueStmtNode->setNodeLocation(trueNode->getNodeLocation());

    //end of while loop label, linked to end of body (true statement)
    Node * labelNode = new NodeLabel(controlLoopLabelNum, m_state);
    assert(labelNode);
    labelNode->setNodeLocation(wTok.m_locator);

    NodeStatements * labelStmtNode = new NodeStatements(labelNode, m_state);
    assert(labelStmtNode);
    labelStmtNode->setNodeLocation(wTok.m_locator);
    trueStmtNode->setNextNode(labelStmtNode);

    Node * whileNode = new NodeControlWhile(condNode, trueStmtNode, m_state);
    assert(whileNode);
    whileNode->setNodeLocation(wTok.m_locator);

    NodeStatements * nextControlNode = new NodeStatements(whileNode, m_state);
    assert(nextControlNode);
    nextControlNode->setNodeLocation(whileNode->getNodeLocation());

    rtnNode->setNextNode(nextControlNode); //***link while to rtn block

    //this block's ST is no longer in scope
    currBlock = m_state.getCurrentBlock(); //reload
    if(currBlock)
      m_state.m_currentFunctionBlockDeclSize -= currBlock->getSizeOfSymbolsInTable();

    m_state.popClassContext(); //= prevBlock;

    return rtnNode;
  } //parseControlWhile

  Node * Parser::parseControlFor(Token fTok)
  {
    if(!getExpectedToken(TOK_OPEN_PAREN))
      return NULL;

    s32 controlLoopLabelNum = m_state.m_parsingControlLoop; //save at the top

    //before parsing the for statement, need a new scope
    NodeBlock * currBlock = m_state.getCurrentBlock();
    NodeBlock * rtnNode = new NodeBlock(currBlock, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(fTok.m_locator);

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    m_state.pushCurrentBlock(rtnNode); //without pop first

    Token pTok;
    getNextToken(pTok);

    Node * declNode = NULL; //may be empty
    if(Token::isTokenAType(pTok))
      {
	unreadToken();
	declNode = parseDecl(); //updates symbol table
	getNextToken(pTok);
      }
    else if(pTok.m_type == TOK_IDENTIFIER)
      {
	unreadToken();
	declNode = parseAssignExpr();
	getNextToken(pTok);
      }

    if(pTok.m_type != TOK_SEMICOLON)
      {
	delete rtnNode;
	delete declNode; //stop this maddness
	m_state.popClassContext(); //where was it?
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
	    delete rtnNode;
	    delete declNode;
	    m_state.popClassContext(); //where was it?
	    return NULL; //stop this maddness
	  }

	if(!getExpectedToken(TOK_SEMICOLON))
	  {
	    delete rtnNode;
	    delete declNode;
	    delete condNode;
	    m_state.popClassContext(); //where was it?
	    return NULL;
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
	    delete rtnNode;
	    delete declNode;
	    delete condNode;
	    m_state.popClassContext(); //where was it?
	    return NULL; //stop this maddness
	  }

	if(!getExpectedToken(TOK_CLOSE_PAREN))
	  {
	    delete rtnNode;
	    delete declNode;
	    delete condNode;
	    delete assignNode;
	    m_state.popClassContext(); //where was it?
	    return NULL; //stop this maddness
	  }
      } //done with assign expr, could be null

    Node * trueNode = NULL;
    if(m_state.m_parsingConditionalAs)
      {
	trueNode = setupAsConditionalBlockAndParseStatements((NodeConditional *) condNode);
      }
    else
      {
	trueNode = parseStatement(); //even an empty statement has a node!
      }

    if(!trueNode)
      {
	delete rtnNode;
	delete declNode;
	delete condNode;
	delete assignNode;
	m_state.popClassContext(); //where was it?
	return NULL; //stop this maddness
      }

    //link the pieces together..
    NodeStatements * nextNode = NULL;
    if(declNode)
      {
	nextNode = new NodeStatements(declNode, m_state);
	assert(nextNode);
	nextNode->setNodeLocation(declNode->getNodeLocation());
	rtnNode->setNextNode(nextNode); //***link decl as first in block
      }

    //wrapping body in NodeStatements produces proper comment for genCode
    //might need another block here ?
    NodeStatements * trueStmtNode = new NodeStatements(trueNode, m_state);
    assert(trueStmtNode);
    trueStmtNode->setNodeLocation(trueNode->getNodeLocation());

    //end of while loop label, linked to end of body, before assign statement
    Node * labelNode = new NodeLabel(controlLoopLabelNum, m_state);
    assert(labelNode);
    labelNode->setNodeLocation(rTok.m_locator);

    NodeStatements * labelStmtNode = new NodeStatements(labelNode, m_state);
    assert(labelStmtNode);
    labelStmtNode->setNodeLocation(rTok.m_locator);
    trueStmtNode->setNextNode(labelStmtNode);

    if(assignNode)
      {
	NodeStatements * nextAssignNode = new NodeStatements(assignNode, m_state);
	assert(nextAssignNode);
	nextAssignNode->setNodeLocation(assignNode->getNodeLocation());
	labelStmtNode->setNextNode(nextAssignNode); //***link assign to label after truestmt
      }

    Node * whileNode = new NodeControlWhile(condNode, trueStmtNode, m_state);
    assert(whileNode);
    whileNode->setNodeLocation(fTok.m_locator);

    NodeStatements * nextControlNode = new NodeStatements(whileNode, m_state);
    assert(nextControlNode);
    nextControlNode->setNodeLocation(whileNode->getNodeLocation());

    if(declNode)
      nextNode->setNextNode(nextControlNode); //***link while to decl
    else
      rtnNode->setNextNode(nextControlNode); //***link while to rtn block (no decl)

    //this block's ST is no longer in scope
    currBlock = m_state.getCurrentBlock(); //reload
    if(currBlock)
      m_state.m_currentFunctionBlockDeclSize -= currBlock->getSizeOfSymbolsInTable();

    m_state.popClassContext(); //= prevBlock;
    return rtnNode;
  } //parseControlFor

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

	//next check for 'as' and 'has' ('is' is a Factor)
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

  Node * Parser::setupAsConditionalBlockAndParseStatements(NodeConditional * asNode)
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

    NodeStatements * stmtsNode = new NodeStatements(varNode, m_state);
    assert(stmtsNode);
    stmtsNode->setNodeLocation(varNode->getNodeLocation());

    blockNode->setNextNode(stmtsNode);

    if(!singleStmtExpected)
      {
	if(!getExpectedToken(TOK_CLOSE_CURLY))
	  {
	    stmtsNode->setNextNode((NodeStatements *) parseStatements()); //more statements
	    getExpectedToken(TOK_CLOSE_CURLY);
	  }
	//else
	//unreadToken();
      }
    else
      {
	Node * sNode = parseStatement(); //get one statement only
	NodeStatements * nextNode = new NodeStatements(sNode, m_state);
	assert(nextNode);
	nextNode->setNodeLocation(sNode->getNodeLocation());
	stmtsNode->setNextNode(nextNode);
      }

    //this block's ST is no longer in scope
    currBlock = m_state.getCurrentBlock();
    if(currBlock)
      m_state.m_currentFunctionBlockDeclSize -= currBlock->getSizeOfSymbolsInTable();

    m_state.popClassContext(); //= prevBlock;
    return blockNode;
  } //setupAsConditionalBlockAndParseStatements

  Node * Parser::parseSimpleStatement()
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_SEMICOLON)
      {
	unreadToken();
	rtnNode = new NodeStatementEmpty(m_state); //empty statement
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    else if(Token::isTokenAType(pTok))
      {
	unreadToken();
	rtnNode = parseDecl(); //updates symbol table
      }
    else if(pTok.m_type == TOK_KW_TYPEDEF)
      {
	rtnNode = parseTypedef();
      }
    else if(pTok.m_type == TOK_KW_CONSTDEF)
      {
	rtnNode = parseConstdef();
      }
    else if(pTok.m_type == TOK_KW_RETURN)
      {
	unreadToken(); //needs location
	rtnNode = parseReturn();
      }
    else if(pTok.m_type == TOK_KW_BREAK)
      {
	if(m_state.m_parsingControlLoop)
	  {
	    rtnNode = new NodeBreakStatement(m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(pTok.m_locator);
	  }
	else
	  {
	    MSG(&pTok,"Break statement not within loop or switch" , ERR);
	  }
      }
    else if(pTok.m_type == TOK_KW_CONTINUE)
      {
	if(m_state.m_parsingControlLoop)
	  {
	    rtnNode = new NodeContinueStatement(m_state.m_parsingControlLoop, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(pTok.m_locator);
	  }
	else
	  {
	    MSG(&pTok,"Continue statement not within loop" , ERR);
	  }
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
	    rtnNode = new NodeSimpleStatement(expNode,m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(expNode->getNodeLocation());
	  }
      }

    if(!getExpectedToken(TOK_SEMICOLON, pTok, QUIETLY))
      {
	//reportedly difficult to catch as an error, so special case error msg
	if(pTok.m_type == TOK_PLUS_PLUS || pTok.m_type == TOK_MINUS_MINUS)
	  {
	    std::ostringstream msg;
	    msg << "Unexpected input!! Try ";
	    msg << m_state.getTokenDataAsString(&pTok).c_str();
	    msg << " as a prefix operator";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }

	MSG(&pTok, "Invalid Statement (possible missing semicolon)", ERR);
	delete rtnNode;
	rtnNode = NULL;
	getTokensUntil(TOK_SEMICOLON);
      }
    return rtnNode;
  } //parseSimpleStatement

  //Typedefs are not transferred to generated code;
  //they are a short-hand for ulamtypes (e.g. arrays)
  //that may be used as function return types; scope-specific.
  Node * Parser::parseTypedef()
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    if(Token::isTokenAType(pTok))
      {
	unreadToken();
	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptor(typeargs);
	assert(typeNode);

	Token iTok;
	getNextToken(iTok);
	if(iTok.m_type == TOK_AMP)
	  {
	    typeargs.m_declRef = ALT_REF;
	    typeargs.m_referencedUTI = typeNode->getReferencedUTI(); //typeNode->givenUTI();
	    getNextToken(iTok);
	  }

	//insure the typedef name starts with a capital letter
	if(iTok.m_type == TOK_TYPE_IDENTIFIER)
	  {
	    rtnNode = makeTypedefSymbol(typeargs, iTok, typeNode);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid typedef Alias <" << m_state.getTokenDataAsString(&iTok).c_str();
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
	msg << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	MSG(&pTok, msg.str().c_str(), ERR);
      }
    return rtnNode;
  } //parseTypedef

  //Named constants (constdefs) are not transferred to generated code;
  //they are a short-hand for scalar constant expressions (e.g. terminals),
  //that are not 'storeintoable'; scope-specific.
  //doubles as class parameter without keyword or assignment.
  Node * Parser::parseConstdef(bool assignREQ, bool isStmt)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    if(Token::isTokenAType(pTok) && (pTok.m_type != TOK_KW_TYPE_VOID) && (pTok.m_type != TOK_KW_TYPE_ATOM))
      {
	unreadToken();
	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptor(typeargs);
	assert(typeNode);
	typeargs.m_assignOK = assignREQ;
	typeargs.m_isStmt = isStmt;

	Token iTok;
	getNextToken(iTok);
	//insure the typedef name starts with a lower case letter
	if(iTok.m_type == TOK_IDENTIFIER)
	  {
	    rtnNode = makeConstdefSymbol(typeargs, iTok, typeNode);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid constant definition Alias '";
	    msg << m_state.getTokenDataAsString(&iTok).c_str();
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
	msg << m_state.getTokenDataAsString(&pTok).c_str() << "'";
	MSG(&pTok, msg.str().c_str(), ERR);
	if(isStmt)
	  getTokensUntil(TOK_SEMICOLON);
	else
	  {
	    Token tmpTok;
	    getNextToken(tmpTok); //by pass identTok only
	  }
      }
    return rtnNode;
  } //parseConstdef

  Node * Parser::parseParameter()
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    //permitted in only in elements
    if(m_state.getUlamTypeByIndex(m_state.getCompileThisIdx())->getUlamClass() != UC_ELEMENT)
      {
	std::ostringstream msg;
	msg << "Model Parameters cannot survive within a quark";
	MSG(&pTok, msg.str().c_str(), ERR);
	getTokensUntil(TOK_SEMICOLON); //does this help?
	return NULL;
      }

    if(Token::isTokenAType(pTok) && pTok.m_type != TOK_KW_TYPE_VOID && pTok.m_type != TOK_KW_TYPE_ATOM)
      {
	unreadToken();
	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptor(typeargs, true);
	if(typeNode)
	  {
	    typeargs.m_assignOK = true;

	    Token iTok;
	    getNextToken(iTok);
	    if(iTok.m_type == TOK_IDENTIFIER)
	      {
		rtnNode = makeParameterSymbol(typeargs, iTok, typeNode);
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Invalid Model Parameter Name <";
		msg << m_state.getTokenDataAsString(&iTok).c_str();
		msg << ">, Parameter Identifier requires lower-case";
		MSG(&iTok, msg.str().c_str(), ERR);
		delete typeNode;
		typeNode = NULL;
	      }
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Invalid Model Parameter Type <";
	msg << m_state.getTokenDataAsString(&pTok).c_str();
	msg << ">; Only primitive types beginning with an ";
	msg << "upper-case letter may be a Model Parameter";
	MSG(&pTok, msg.str().c_str(), ERR);
	unreadToken();
      }
    return rtnNode;
  } //parseParameter

  //used for data members or local function variables; or
  //'singledecl' function parameters; no longer for function defs.
  Node * Parser::parseDecl(bool parseSingleDecl)
  {
    Node * rtnNode = NULL;
    TypeArgs typeargs;
    NodeTypeDescriptor * typeNode = parseTypeDescriptor(typeargs);
    assert(typeNode);
    if(parseSingleDecl)
      typeargs.m_isStmt = false; //for function params (e.g. refs)

    Token iTok;
    getNextToken(iTok);
    if(iTok.m_type == TOK_AMP)
      {
	typeargs.m_declRef = ALT_REF;
	typeargs.m_referencedUTI = typeNode->getReferencedUTI();
	getNextToken(iTok);
      }

    if(iTok.m_type == TOK_IDENTIFIER)
      {
	UTI passuti = typeNode->givenUTI(); //before it may become an array
	rtnNode = makeVariableSymbol(typeargs, iTok, typeNode);
	if(rtnNode && !parseSingleDecl)
	  {
	    //for multi's of same type (rtnType), and/or its assignment
	    return parseRestOfDecls(typeargs, iTok, (NodeVarDecl *) rtnNode, rtnNode, passuti);
	  }
      }
    else
      {
	//user error!
	std::ostringstream msg;
	msg << "Name of variable <" << m_state.getTokenDataAsString(&iTok).c_str();
	msg << ">: Identifier must begin with a lower-case letter";
	MSG(&iTok, msg.str().c_str(), ERR);
	unreadToken();
	delete typeNode;
	typeNode = NULL;
      }
    return rtnNode;
  } //parseDecl

  NodeTypeDescriptor * Parser::parseTypeDescriptor(TypeArgs& typeargs, bool delAfterDotFails)
  {
    Token pTok;
    getNextToken(pTok);

    if(!Token::isTokenAType(pTok))
      {
	unreadToken();
	return NULL;
      }

    typeargs.init(pTok); //initialize here

    UTI dropCastUTI;
    return parseTypeDescriptor(typeargs, dropCastUTI, delAfterDotFails);
  }//parseTypeDescriptor (helper)

  NodeTypeDescriptor * Parser::parseTypeDescriptor(TypeArgs& typeargs, UTI& castUTI, bool delAfterDotFails)
  {
    NodeTypeDescriptor * typeNode = NULL;
    Token pTok = typeargs.m_typeTok;
    ULAMTYPE etyp = m_state.getBaseTypeFromToken(pTok);
    bool isAClassType = ((etyp == Class) || (etyp == Hzy) || (etyp == Holder));

    if(isAClassType)
      {
	//sneak peak at next tok for dot
	Token dTok;
	getNextToken(dTok);
	unreadToken();
	isAClassType = (dTok.m_type == TOK_DOT); //another clue for Hzy and Holder

	if(isAClassType && (etyp == Holder))
	  {
	    UTI huti = Nav;
	    UTI tmpscalar= Nav;
	    AssertBool isTDDefined = m_state.getUlamTypeByTypedefName(pTok.m_dataindex, huti, tmpscalar);
	    assert(isTDDefined);
	    m_state.makeClassFromHolder(huti, pTok); //don't need cnsym here
	  }

	UTI cuti = parseClassArguments(pTok, isAClassType); //not sure what to do with the UTI? could be a declref type
	if(isAClassType)
	  {
	    if(m_state.isReference(cuti)) //e.g. refofSelf
	      {
		typeargs.m_classInstanceIdx = m_state.getUlamTypeAsDeref(cuti);
		typeargs.m_declRef = ALT_REF;
		typeargs.m_referencedUTI = typeargs.m_classInstanceIdx;
	      }
	    else if(m_state.isScalar(cuti))
	      typeargs.m_classInstanceIdx = cuti;
	    else
	      typeargs.m_classInstanceIdx = m_state.getUlamTypeAsScalar(cuti); //eg typedef class array
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
	//else arraytype???
	castUTI = tuti;

	//bitsize is unknown, e.g. based on a Class.sizeof
	typeNode = new NodeTypeDescriptor(typeargs.m_typeTok, tuti, m_state);
	assert(typeNode);

	if(bitsizeNode)
	  typeNode->linkConstantExpressionBitsize(bitsizeNode); //tfr ownership
      }

    Token dTok;
    getNextToken(dTok);
    unreadToken();
    if(dTok.m_type == TOK_DOT)
      {
	if(!parseTypeFromAnotherClassesTypedef(typeargs, typeNode))
	  {
	    if(delAfterDotFails)
	      {
		delete typeNode;
		typeNode = NULL;
	      }
	  }
	else
	  castUTI = typeargs.m_anothertduti;
      }
    else if(dTok.m_type == TOK_AMP)
      {
	typeargs.m_declRef = ALT_REF; //a declared reference
	typeargs.m_referencedUTI = castUTI; //?
	typeargs.m_assignOK = true; //required
	typeargs.m_isStmt = true; //unless a func param
	// change uti to reference key
	assert(typeNode);
	typeNode->setReferenceType(ALT_REF, castUTI);
      }
    return typeNode;
  } //parseTypeDescriptor

  UTI Parser::parseClassArguments(Token& typeTok, bool& isaclass)
  {
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type != TOK_OPEN_PAREN)
      {
	//regular class, not a template, OR Self
	unreadToken();

	SymbolClassName * cnsym = NULL;
	if(!m_state.alreadyDefinedSymbolClassName(typeTok.m_dataindex, cnsym))
	  {
	    //check if a typedef first..
	    UTI tduti = Nav;
	    UTI tdscalaruti = Nouti;
	    if(m_state.getUlamTypeByTypedefName(typeTok.m_dataindex, tduti, tdscalaruti))
	      return tduti; //done. (could be an array; or refselftype)
	    else
	      {
		// not necessarily a class!!
		if(isaclass)
		  m_state.addIncompleteClassSymbolToProgramTable(typeTok, cnsym);
		else
		  {
		    UTI huti = m_state.makeUlamTypeHolder();
		    m_state.addUnknownTypeTokenToThisClassResolver(typeTok, huti);
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

    //must be a template class
    bool unseenTemplate = false;
    SymbolClassNameTemplate * ctsym = NULL;
    if(!m_state.alreadyDefinedSymbolClassNameTemplate(typeTok.m_dataindex, ctsym))
      {
	unseenTemplate = true;
	if(ctsym == NULL)
	  m_state.addIncompleteTemplateClassSymbolToProgramTable(typeTok, ctsym); //was undefined, template; will fix instances' argument names later
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
    UTI stubuti = m_state.makeUlamType(typeTok, UNKNOWNSIZE, NONARRAYSIZE, Nouti); //overwrites the template type here.
    UlamType * stubut = m_state.getUlamTypeByIndex(stubuti);
    ((UlamTypeClass *) stubut)->setUlamClass(ctsym->getUlamClass()); //possibly UC_UNSEEN

    UlamType * ctut = m_state.getUlamTypeByIndex(ctuti);
    if(ctut->isCustomArray())
      ((UlamTypeClass *) stubut)->setCustomArray();

    SymbolClass * stubcsym = ctsym->makeAStubClassInstance(typeTok, stubuti);

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
      }
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
    if(!exprNode)
      {
	std::ostringstream msg;
	msg << "Class Argument after Open Paren is missing, for template '";
	msg << m_state.m_pool.getDataAsString(csym->getId()).c_str() << "'";
	MSG(&pTok, msg.str().c_str(), ERR);
	// what else is missing?
	return;
      }

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

	SymbolConstantValue * argSym;
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
	argSym->setArgumentFlag();
	m_state.addSymbolToCurrentScope(argSym); //scope updated to new class instance in parseClassArguments

	m_state.popClassContext(); //restore before making NodeConstantDef, so current context

	NodeTypeDescriptor * argTypeDesc = NULL;
	if(!ctUnseen)
	  {
	    //copy the type descriptor from the template for the stub
	    NodeBlockClass * templateblock = ctsym->getClassBlockNode();
	    NodeConstantDef * paramConstDef = (NodeConstantDef *) templateblock->getParameterNode(parmIdx);
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
	    if(args.m_typeTok.m_type != TOK_KW_TYPE_VOID)
	      {
		rtnNode = new NodeTypeBitsize(bitsizeNode, m_state);
		assert(rtnNode);
		rtnNode->setNodeLocation(args.m_typeTok.m_locator);
		args.m_bitsize = UNKNOWNSIZE; //no eval yet
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Void bitsize expression disregarded; size is zero";
		MSG(&bTok, msg.str().c_str(), WARN);

		args.m_bitsize = 0;
		delete bitsizeNode;
		bitsizeNode = NULL;
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
	    //cnsym = m_state.makeAnonymousClassFromHolder(args.m_anothertduti, args.m_typeTok.m_locator);
	    cnsym = m_state.makeClassFromHolder(args.m_anothertduti, args.m_typeTok);
	    args.m_classInstanceIdx = args.m_anothertduti; //since we didn't know last time
	  }
	else
	  {
	    unreadToken(); //put dot back, minof or maxof perhaps?
	    std::ostringstream msg;
	    msg << "Unexpected input!! Token <" << args.m_typeTok.getTokenEnumNameFromPool(&m_state).c_str();
	    msg << "> is not a 'seen' class type: <";
	    msg << m_state.getTokenDataAsString(&args.m_typeTok).c_str() << ">";
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
		    msg << m_state.getTokenDataAsString(&args.m_typeTok).c_str() << ">";
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
	if((nTok.m_type != TOK_KW_SIZEOF) && (nTok.m_type != TOK_KW_INSTANCEOF) && (nTok.m_type != TOK_KW_STORAGEOF))
	  {
	    std::ostringstream msg;
	    msg << "Trying to use typedef from another class '";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str();
	    msg << "', before it has been defined; Cannot continue with (token) ";
	    msg << m_state.getTokenDataAsString(&nTok).c_str();
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
	    ULAMCLASSTYPE mclasstype = m_state.getUlamTypeByIndex(mcuti)->getUlamClass();
	    if(mclasstype == UC_UNSEEN)
	      {
		UTI huti = m_state.makeUlamTypeHolder();
		SymbolTypedef * symtypedef = new SymbolTypedef(pTok, huti, Nav, m_state);
		assert(symtypedef);
		symtypedef->setBlockNoOfST(memberClassNode->getNodeNo());
		m_state.addSymbolToCurrentMemberClassScope(symtypedef);
		m_state.addUnknownTypeTokenToAClassResolver(mcuti, pTok, huti);
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
		msg << " found for Typedef <" << m_state.getTokenDataAsString(&pTok).c_str();
		msg << ">, belonging to class: ";
		msg << m_state.m_pool.getDataAsString(csym->getId()).c_str();
		MSG(&pTok, msg.str().c_str(), DEBUG);
	      }
	    assert(m_state.okUTItoContinue(tduti));
	    ULAMCLASSTYPE tdclasstype = tdut->getUlamClass();
	    const std::string tdname = tdut->getUlamTypeNameOnly();

	    //update token argument
	    if(tdclasstype == UC_NOTACLASS)
	      args.m_typeTok.init(Token::getTokenTypeFromString(tdname.c_str()), pTok.m_locator, 0);
	    else
	      {
		args.m_typeTok.init(TOK_TYPE_IDENTIFIER, pTok.m_locator, m_state.m_pool.getIndexForDataString(tdname));
		isclasstd = true;
	      }

	    //don't update rest of argument refs; typedefs have their own bit and array sizes to look up

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
	    msg << m_state.getTokenDataAsString(&pTok).c_str();
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
	unreadToken(); //put the whatever came after the dot (e.g. 'sizeof') back
	rtnb = false;
      }

    m_state.popClassContext(); //restore
    return;
  } //parseTypeFromAnotherClassesTypedef

  Node * Parser::parseReturn()
  {
    Token pTok;
    getNextToken(pTok);

    Node * rtnNode = NULL;
    Node * rtnExprNode = parseAssignExpr(); //may be NULL
    if(!rtnExprNode)
      {
	rtnExprNode = new NodeStatementEmpty(m_state); //has Nouti type
	assert(rtnExprNode);
	rtnExprNode->setNodeLocation(pTok.m_locator);
      }
    rtnNode =  new NodeReturnStatement(rtnExprNode, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(pTok.m_locator);
    return rtnNode;
  } //parseReturn

  Node * Parser::parseAssignExpr()
  {
    Node * rtnNode = NULL;
    Token iTok;
    if(getExpectedToken(TOK_IDENTIFIER, iTok, QUIETLY))
      {
	//check for a named constant already defined (e.g. class
	//parameter) and continue parsing expression instead of ident.
	Symbol * sym = NULL;
	bool hazyKin = false; //don't care
	if(m_state.alreadyDefinedSymbol(iTok.m_dataindex, sym, hazyKin))
	  {
	    if(sym->isConstant() || sym->isModelParameter())
	      {
		unreadToken();
		return parseExpression();
	      }
	  }
	//though function calls are not proper lhs values in assign
	//expression; they are parsed here (due to the two token look
	//ahead, which drops the Identifier Token before parseExpression) and is
	//caught during checkAndLabelType as NOT storeIntoAble.
	if(!(rtnNode = parseIdentExpr(iTok)))
	  return parseExpression();
      }
    else
      return parseExpression(); //perhaps a number, true or false, cast..

    //if nothing else follows, parseRestOfAssignExpr returns its argument
    return parseRestOfAssignExpr(rtnNode);
  } //parseAssignExpr

  Node * Parser::parseLvalExpr(Token identTok)
  {
    Token pTok;
    getNextToken(pTok);
    //function calls or func defs are not valid
    if(pTok.m_type == TOK_OPEN_PAREN)
      {
	std::ostringstream msg;
	msg << "Unexpected token <" << m_state.getTokenDataAsString(&pTok).c_str();
	msg << "> indicates a function call or definition; neither are valid here";
	MSG(&pTok, msg.str().c_str(), ERR);
	unreadToken();
	return NULL;
      }

    unreadToken(); //put whatever back

    Symbol * asymptr = NULL;
    bool hazyKin = false; //don't care
    //may continue when symbol not defined yet (e.g. Decl)
    // don't return a NodeConstant, instead of NodeIdent, without arrays
    // even if already defined as one.
    m_state.alreadyDefinedSymbol(identTok.m_dataindex, asymptr, hazyKin);

    //o.w. make a variable;  symbol could be Null!
    Node * rtnNode = new NodeIdent(identTok, (SymbolVariable *) asymptr, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(identTok.m_locator);

    return parseRestOfLvalExpr(rtnNode); //in case of arrays
  } //parseLvalExpr

  //lvalExpr + func Calls + member select (dot)
  Node * Parser::parseIdentExpr(Token identTok)
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
	    msg << "Undefined function <" << m_state.getTokenDataAsString(&identTok).c_str();
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
	rtnNode = parseMemberSelectExpr(identTok);
      }
    else
      {
	//else we have a variable, not a function call, nor member_select
	unreadToken();
	rtnNode = parseLvalExpr(identTok);
      }

    //bail if no node
    if(!rtnNode)
      return NULL;

    //check for member select expression
    Token qTok;
    getNextToken(qTok);

    if(qTok.m_type == TOK_DOT)
      {
	unreadToken();
	rtnNode = parseRestOfMemberSelectExpr(rtnNode);
      }
    else
      {
	unreadToken(); //not a member select
      }
    return rtnNode;
  } //parseIdentExpr

  Node * Parser::parseMemberSelectExpr(Token memberTok)
  {
    Node * rtnNode = NULL;
    Symbol * dsymptr = NULL;
    bool hazyKin = false; //don't care
    if(m_state.alreadyDefinedSymbol(memberTok.m_dataindex, dsymptr, hazyKin))
      rtnNode = parseMinMaxSizeofType(memberTok, dsymptr->getUlamTypeIdx(), NULL);
    else
      rtnNode = parseMinMaxSizeofType(memberTok);

    if(rtnNode) return rtnNode; //not min/max/sizeof..continue

    // type of dsymptr maybe holder; now we know it should be a class!
    if(dsymptr)
      {
	UTI duti = dsymptr->getUlamTypeIdx();
	UlamType * dut = m_state.getUlamTypeByIndex(duti);
	if(m_state.okUTItoContinue(duti) && (dut->getUlamClass() == UC_NOTACLASS) && (dut->getUlamTypeEnum() == Holder))
	  {
	    //m_state.makeAnonymousClassFromHolder(duti, memberTok.m_locator);
	    m_state.makeClassFromHolder(duti, memberTok);
	  }
      }

    //arg is an instance of a class, it will be/was
    //declared as a variable, either as a data member or locally,
    //WAIT To  search back through the block symbol tables during type labeling
    Node * classInstanceNode = new NodeIdent(memberTok, (SymbolVariable *) dsymptr, m_state);
    assert(classInstanceNode);
    classInstanceNode->setNodeLocation(memberTok.m_locator);

    //part of (recursive) restofmemberselectexpr here due to possible missing dot!
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type != TOK_DOT)
      {
	unreadToken();
      }

    Token iTok;
    if(getExpectedToken(TOK_IDENTIFIER, iTok))
      {
	//set up compiler state to NOT use the current class block
	//for symbol searches; may be unknown until type label
	m_state.pushClassContextUsingMemberClassBlock(NULL); //oddly =true

	rtnNode = new NodeMemberSelect(classInstanceNode, parseIdentExpr(iTok), m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(iTok.m_locator);

	//clear up compiler state to no longer use the member class block for symbol searches
	m_state.popClassContext();
      }
    else
      rtnNode = classInstanceNode;

    return parseRestOfMemberSelectExpr(rtnNode);
  } //parseMemberSelectExpr

  Node * Parser::parseRestOfMemberSelectExpr(Node * classInstanceNode)
  {
    Node * rtnNode = classInstanceNode;
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type != TOK_DOT)
      {
	unreadToken();
	return rtnNode;
      }

    Token iTok;
    if(getExpectedToken(TOK_IDENTIFIER, iTok))
      {
	//set up compiler state to NOT use the current class block
	//for symbol searches; may be unknown until type label
	m_state.pushClassContextUsingMemberClassBlock(NULL); //oddly =true

	rtnNode = new NodeMemberSelect(classInstanceNode, parseIdentExpr(iTok), m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(iTok.m_locator);

	//clear up compiler state to no longer use the member class block for symbol searches
	m_state.popClassContext(); //restore
      }
    return parseRestOfMemberSelectExpr(rtnNode); //recurse
  } //parseRestOfMemberSelectExpr

  Node * Parser::parseMinMaxSizeofType(Token memberTok, UTI utype, NodeTypeDescriptor * nodetype)
  {
    Node * rtnNode = NULL;
    UlamType * ut = m_state.getUlamTypeByIndex(utype);

    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type != TOK_DOT)
      unreadToken(); //optional, dot might have already been eaten

    Token fTok;
    getNextToken(fTok);

    switch(fTok.m_type)
      {
      case TOK_KW_SIZEOF:
	{
	  if(ut->isComplete())
	    {
	      assert(ut->getUlamClass() == UC_NOTACLASS); //can't be a class and complete
	      rtnNode = makeTerminal(fTok, (u64) ut->getTotalBitSize(), Unsigned);
	      delete nodetype; //unlikely
	      //nodetype = NULL; not a ref!
	    }
	  else
	    {
	      //input uti wasn't complete, i.e. based on sizeof some class
	      rtnNode = new NodeTerminalProxy(memberTok, utype, fTok, nodetype, m_state);
	    }
	}
	break;
      case TOK_KW_MAXOF:
	{
	  if(ut->isMinMaxAllowed())
	    {
	      if(ut->isComplete())
		{
		  rtnNode = makeTerminal(fTok, ut->getMax(), utype); //ut->getUlamTypeEnum());
		  delete nodetype; //unlikely
		  //nodetype = NULL; not a ref
		}
	      else
		rtnNode = new NodeTerminalProxy(memberTok, utype, fTok, nodetype, m_state);
	    }
	  else
	    {
	      std::ostringstream msg;
	      msg << "Unsupported request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	      msg << "' <" << m_state.getTokenDataAsString(&memberTok).c_str();
	      msg << ">, type: " << m_state.getUlamTypeNameByIndex(utype).c_str();
	      MSG(&fTok, msg.str().c_str(), ERR);
	    }
	}
	break;
      case TOK_KW_MINOF:
	{
	  if(ut->isMinMaxAllowed())
	    {
	      if(ut->isComplete())
		{
		  rtnNode = makeTerminal(fTok, ut->getMin(), utype); //ut->getUlamTypeEnum());
		  delete nodetype; //unlikely
		  //nodetype = NULL; not a ref
		}
	      else
		rtnNode = new NodeTerminalProxy(memberTok, utype, fTok, nodetype, m_state);
	    }
	  else
	    {
	      std::ostringstream msg;
	      msg << "Unsupported request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	      msg << "' <" << m_state.getTokenDataAsString(&memberTok).c_str();
	      msg << ">, type: " << m_state.getUlamTypeNameByIndex(utype).c_str();
	      MSG(&fTok, msg.str().c_str(), ERR);
	    }
	}
	break;
      case TOK_KW_INSTANCEOF:
	{
	  if(ut->isComplete())
	    {
	      assert(ut->getUlamClass() == UC_NOTACLASS); //can't be a class and complete
	      rtnNode = NULL; //caller will clean up nodetype

	      std::ostringstream msg;
	      msg << "Unsupported request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	      msg << "' <" << m_state.getTokenDataAsString(&memberTok).c_str();
	      msg << ">, non-class type: " << m_state.getUlamTypeNameByIndex(utype).c_str();
	      MSG(&fTok, msg.str().c_str(), ERR);
	    }
	  else
	    {
	      //input uti wasn't complete
	      rtnNode = new NodeInstanceof(memberTok, nodetype, m_state);
	    }
	}
	break;
      case TOK_KW_STORAGEOF:
	{
	  if(ut->isComplete())
	    {
	      assert(ut->getUlamClass() == UC_NOTACLASS); //can't be a class and complete
	      rtnNode = NULL; //caller will clean up nodetype

	      std::ostringstream msg;
	      msg << "Unsupported request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	      msg << "' <" << m_state.getTokenDataAsString(&memberTok).c_str();
	      msg << ">, non-class type: " << m_state.getUlamTypeNameByIndex(utype).c_str();
	      MSG(&fTok, msg.str().c_str(), ERR);
	    }
	  else
	    {
	      if(memberTok.m_type == TOK_TYPE_IDENTIFIER)
		{
		  rtnNode = NULL; //caller will clean up nodetype

		  std::ostringstream msg;
		  msg << "Unsupported request: '" << m_state.getTokenDataAsString(&fTok).c_str();
		  msg << "' <" << m_state.getTokenDataAsString(&memberTok).c_str();
		  msg << ">, a Type";
		  MSG(&fTok, msg.str().c_str(), ERR);
		}
	      else
		//input uti wasn't complete
		rtnNode = new NodeStorageof(memberTok, nodetype, m_state);
	    }
	}
	break;
      case TOK_IDENTIFIER:
	{
	  //possible named constant?
	  //rtnNode = new NodeConstantProxy(memberTok, utype, fTok, nodetype, m_state);

	}
	//break;
      default:
	{
	  //std::ostringstream msg;
	  //msg << "Undefined request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	  //msg << "' <" << m_state.getTokenDataAsString(&memberTok).c_str();
	  //msg << ">, type: " << m_state.getUlamTypeNameByIndex(utype).c_str();
	  //MSG(&fTok, msg.str().c_str(), DEBUG);
	  unreadToken();
	}
      };
    return rtnNode; //may be null if not minof, maxof, sizeof, but a member or func selected
  } //parseMinMaxSizeofType

  // overloaded for when type is not available
  Node * Parser::parseMinMaxSizeofType(Token memberTok)
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
	rtnNode = new NodeTerminalProxy(memberTok, Nouti, fTok, NULL, m_state);
	break;
      case TOK_KW_MAXOF:
	rtnNode = new NodeTerminalProxy(memberTok, Nouti, fTok, NULL, m_state);
	break;
      case TOK_KW_MINOF:
	rtnNode = new NodeTerminalProxy(memberTok, Nouti, fTok, NULL, m_state);
	break;
      case TOK_KW_INSTANCEOF:
	rtnNode = new NodeInstanceof(memberTok, NULL, m_state);
	break;
      case TOK_KW_STORAGEOF:
	rtnNode = new NodeStorageof(memberTok, NULL, m_state);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unsupported request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	  msg << "' <" << m_state.getTokenDataAsString(&memberTok).c_str();
	  msg << ">, type unavailable";
	  MSG(&fTok, msg.str().c_str(), DEBUG);
	  unreadToken();
	}
      };
    return rtnNode; //may be null if not minof, maxof, sizeof, but a member or func selected
  } //parseMinMaxSizeofType (overloaded, type unavail)

  Node * Parser::parseFunctionCall(Token identTok)
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

    if(!parseRestOfFunctionCallArguments(rtnNode))
      {
	delete rtnNode;
	m_state.popClassContext();
	return NULL;
      }

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
    msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
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
  } //parseExpression

  Node * Parser::parseEqExpression()
  {
    Node * rtnNode = parseCompareExpression();
    if(!rtnNode)
      return NULL; //stop this maddness

    //if not compop, parseRestOfEqExpression returns its arg
    return parseRestOfEqExpression(rtnNode);
  } //parseExpression

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

  Node * Parser::parseFactor()
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    //check for min/max/sizeof type, to make a constant terminal
    //only way to see a Type here! sizeof array when typedef.
    if(Token::isTokenAType(pTok))
      {
	unreadToken();

	TypeArgs typeargs;
	NodeTypeDescriptor * typeNode = parseTypeDescriptor(typeargs);
	assert(typeNode);
	UTI uti = typeNode->givenUTI();

	//returns either a terminal or proxy
	rtnNode = parseMinMaxSizeofType(pTok, uti, typeNode); //optionally, gets next dot token
	if(!rtnNode)
	  {
	    Token iTok;
	    getNextToken(iTok);
	    if(iTok.m_type == TOK_IDENTIFIER)
	      {
		rtnNode = makeConstdefSymbol(typeargs, iTok, typeNode);
	      }
	    else
	      {
		//clean up, some kind of error parsing min/max/sizeof
		delete typeNode;
		typeNode = NULL;
	      }
	  }
	return rtnNode; //rtnNode could be NULL!
      } //not a Type

    //continue as normal..
    switch(pTok.m_type)
      {
      case TOK_IDENTIFIER:
	{
	  Symbol * asymptr = NULL;
	  bool hazyKin = false; //don't care
	  if(m_state.alreadyDefinedSymbol(pTok.m_dataindex, asymptr, hazyKin))
	    {
	      //if already defined named constant, or model parameter, in current block,
	      //then return a NodeConstant (or NodeMP), instead of NodeIdent, without arrays.
	      if(asymptr->isConstant() || asymptr->isModelParameter())
		{
		  Token dTok;
		  getNextToken(dTok);
		  unreadToken();
		  if(dTok.m_type == TOK_DOT)
		    rtnNode = parseMinMaxSizeofType(pTok, asymptr->getUlamTypeIdx(), NULL);
		  else
		    {
		      if(asymptr->isConstant())
			rtnNode = new NodeConstant(pTok, (SymbolConstantValue *) asymptr, m_state);
		      else
			rtnNode = new NodeModelParameter(pTok, (SymbolParameterValue *) asymptr, m_state);
		      assert(rtnNode);
		      rtnNode->setNodeLocation(pTok.m_locator);
		    }
		  return rtnNode; //done.
		}
	    }

	  rtnNode = parseIdentExpr(pTok);
	  //test ahead for UNOP_EXPRESSION so that any consecutive binary
	  //ops aren't misinterpreted as a unary operator (e.g. +,-).
	  Token tTok;
	  getNextToken(tTok);
	  unreadToken();
	  if(tTok.m_type == TOK_KW_IS)
	    rtnNode = parseRestOfFactor(rtnNode);
	}
	break;
      case TOK_NUMBER_SIGNED:
      case TOK_NUMBER_UNSIGNED:
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
      case TOK_SQUOTED_STRING:
	rtnNode = new NodeTerminal(pTok, m_state);
	assert(rtnNode);
	break;
      case TOK_NUMBER_FLOAT:
	{
	  std::ostringstream msg;
	  msg << "Unsupported Number Type, Float <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	}
	break;
      case TOK_OPEN_PAREN:
	rtnNode = parseRestOfCastOrExpression();
	break;
      case TOK_MINUS:
      case TOK_PLUS:
      case TOK_BANG:
      case TOK_PLUS_PLUS:
      case TOK_MINUS_MINUS:
	unreadToken();
	rtnNode = parseRestOfFactor(NULL); //parseUnop
	break;
      case TOK_EOF:
      case TOK_CLOSE_CURLY:
      case TOK_SEMICOLON:
      case TOK_CLOSE_PAREN:
      case TOK_COMMA: //for functionall args
	unreadToken();
	break;
      case TOK_ERROR_LOWLEVEL:
	{
	  std::ostringstream msg;
	  msg << "(Low Level) " << m_state.getTokenDataAsString(&pTok).c_str();
	  MSG(&pTok, msg.str().c_str(), ERR);
	  return parseFactor(); //redo
	}
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token <";
	  msg << m_state.getTokenDataAsString(&pTok).c_str() << ">; Unreading it";
	  MSG(&pTok, msg.str().c_str(), DEBUG);
	  unreadToken(); //eat the error token
	}
      };
    return rtnNode;
  } //parseFactor

  Node * Parser::parseRestOfFactor(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_MINUS:
      case TOK_PLUS:
      case TOK_BANG:
      case TOK_PLUS_PLUS:
      case TOK_MINUS_MINUS:
	unreadToken();
	assert(!leftNode);
	rtnNode = makeFactorNode();
	break;
      case TOK_KW_IS:
	unreadToken();
	assert(leftNode);
	rtnNode = makeConditionalExprNode(leftNode);
	break;
      case TOK_ERROR_LOWLEVEL:
	//eat token
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //parseRestOfFactor

  Node * Parser::parseRestOfCastOrExpression()
  {
    Node * rtnNode = NULL;
    //just saw an open paren..
    //if next token is a type this a user cast, o.w. expression
    Token tTok;
    getNextToken(tTok);
    if(Token::isTokenAType(tTok))
      {
	rtnNode = makeCastNode(tTok); //also parses its child Factor
      }
    else
      {
	unreadToken();
	rtnNode = parseAssignExpr(); //grammar says assign_expr
	if(!getExpectedToken(TOK_CLOSE_PAREN))
	  {
	    delete rtnNode;
	    rtnNode = NULL;
	  }
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
      case TOK_AMP:
      case TOK_PIPE:
      case TOK_HAT:
	unreadToken();
	rtnNode = parseRestOfLogicalExpression(leftNode); //addOp
	rtnNode = parseRestOfExpression(rtnNode);
	break;
      case TOK_EQUAL_EQUAL:
      case TOK_NOT_EQUAL:
	unreadToken();
	rtnNode = parseRestOfBitExpression(leftNode);
	rtnNode = parseRestOfExpression(rtnNode);
	break;
      case TOK_LESS_THAN:
      case TOK_LESS_EQUAL:
      case TOK_GREATER_THAN:
      case TOK_GREATER_EQUAL:
	unreadToken();
	rtnNode = parseRestOfEqExpression(leftNode);
	rtnNode = parseRestOfExpression(rtnNode);
	break;
      case TOK_SHIFT_LEFT:
      case TOK_SHIFT_RIGHT:
	unreadToken();
	rtnNode = parseRestOfCompareExpression(leftNode);
	rtnNode = parseRestOfExpression(rtnNode);
	break;
      case TOK_PLUS:
      case TOK_MINUS:
	unreadToken();
	rtnNode = parseRestOfShiftExpression(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_STAR:
      case TOK_SLASH:
      case TOK_PERCENTSIGN:
	unreadToken();
	rtnNode = parseRestOfTerm(leftNode); //mulOp
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_KW_IS:
	unreadToken();
	rtnNode = parseRestOfFactor(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_ERROR_LOWLEVEL:
	rtnNode = parseRestOfExpression(leftNode); //redo
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;
  } //parseRestOfExpression

  Node * Parser::parseRestOfLvalExpr(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;

    if(!getExpectedToken(TOK_OPEN_SQUARE, pTok, QUIETLY))
      return leftNode;

    Node * rightNode = parseExpression();
    if(!rightNode)
      {
	MSG(&pTok, "Array item/size is missing; Square Bracket deleted", ERR);
	delete leftNode;
	rtnNode = NULL;
      }
    else
      {
	rtnNode = new NodeSquareBracket(leftNode, rightNode, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }

    if(!getExpectedToken(TOK_CLOSE_SQUARE))
      {
	delete rtnNode;
	rtnNode = NULL;
      }
    return rtnNode;
  } //parseRestOfLValExpr

  Node * Parser::parseRestOfAssignExpr(Node * leftNode)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_EQUAL:
      case TOK_PLUS_EQUAL:
      case TOK_MINUS_EQUAL:
      case TOK_STAR_EQUAL:
      case TOK_AMP_EQUAL:
      case TOK_PIPE_EQUAL:
      case TOK_HAT_EQUAL:
	unreadToken();
	rtnNode = makeAssignExprNode(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_SEMICOLON:
      case TOK_CLOSE_PAREN:
	{
	  unreadToken();
	  rtnNode = leftNode;
	  break;
	}
      default:
	{
	  //or duplicate restofexpression cases here? seems silly..
	  unreadToken();
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
  Node * Parser::parseRestOfDecls(TypeArgs& args, Token identTok, NodeVarDecl * dNode, Node * rtnNode, UTI passuti)
  {
    //rtnNode is NodeStatments on list recursion, contains the NodeVarD ptr
    //dNode is the NodeVarD ptr needed for assignments
    if(dNode == NULL)
      return rtnNode; //quit!

    Token pTok;
    getNextToken(pTok);

    args.m_arraysize = NONARRAYSIZE; //clear for decl list (args ref)

    if(pTok.m_type == TOK_EQUAL) //first check for '='
      {
	if(args.m_assignOK)
	  {
	    unreadToken();
	    return parseRestOfDeclAssignment(args, identTok, dNode, rtnNode, passuti); //pass args for more decls
	  }
	else
	  {
	    //assignments not permitted
	    std::ostringstream msg;
	    msg << "Cannot assign to data member <" << m_state.getTokenDataAsString(&identTok).c_str();
	    msg << "> at the time of its declaration";
	    MSG(&pTok, msg.str().c_str(), ERR);
	    getTokensUntil(TOK_SEMICOLON); //rest of statement is ignored.
	    unreadToken();
	    return rtnNode; //done
	  }
      }
    else if(args.m_declRef == ALT_REF)
      {
	//assignments required for references
	std::ostringstream msg;
	msg << "Must assign to reference <" << m_state.getTokenDataAsString(&identTok).c_str();
	msg << "> at the time of its declaration";
	MSG(&pTok, msg.str().c_str(), ERR);
	getTokensUntil(TOK_SEMICOLON); //rest of statement is ignored.
	unreadToken();
	return rtnNode; //done
      }
    else if(pTok.m_type != TOK_COMMA)
      {
	unreadToken(); //likely semicolon
	return rtnNode; //done
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
	//just the top level as a basic uti (no selects, or arrays)
	//NodeTypeDescriptor * typeNode = new NodeTypeDescriptor(args.m_typeTok, passuti, m_state, args.m_declRef);
	NodeTypeDescriptor * typeNode = new NodeTypeDescriptor(args.m_typeTok, passuti, m_state);

	//another decl of same type
	NodeVarDecl * sNode = (NodeVarDecl *) makeVariableSymbol(args, iTok, typeNode); //a decl !!
	if (sNode)
	  {
	    rtnNode =  new NodeStatements(rtnNode, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(dNode->getNodeLocation());

	    NodeStatements * nextNode = new NodeStatements(sNode, m_state);
	    assert(nextNode);
	    nextNode->setNodeLocation(dNode->getNodeLocation());
	    ((NodeStatements *) rtnNode)->setNextNode(nextNode);
	  }
	//else error?
	dNode = sNode; //replace dNode, no leaks
      }
    else
      {
	dNode = NULL;
	//perhaps read until semi-colon
	getTokensUntil(TOK_SEMICOLON);
	unreadToken();
      }
    return parseRestOfDecls(args, iTok, dNode, rtnNode, passuti); //iTok in case of =
  } //parseRestOfDecls

  Node * Parser::parseRestOfDeclAssignment(TypeArgs& args, Token identTok, NodeVarDecl * dNode, Node * rtnNode, UTI passuti)
  {
    assert(dNode);
    Token eTok;
    getNextToken(eTok);
    assert(eTok.m_type == TOK_EQUAL);

    //update dNode with init expression: lval for ref, assign for local car
    if(args.m_declRef == ALT_REF)
      {
	getNextToken(eTok);
	if(eTok.m_type == TOK_IDENTIFIER)
	  {
	    Node * rightNode = parseIdentExpr(eTok); //parseLvalExpr(eTok);
	    if(!rightNode)
	      {
		std::ostringstream msg;
		msg << "Value of reference " << identTok.getTokenStringFromPool(&m_state).c_str();
		msg << " is missing";
		MSG(&identTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		((NodeVarRef *) dNode)->setInitExpr(rightNode);
	      }
	  }
	else if(eTok.m_type == TOK_TYPE_IDENTIFIER)
	  {
	    unreadToken();
	    //can only be .instanceof
	    Node * rightNode = parseFactor(); //parseMinMaxSizeofType(eTok);
	    if(!rightNode)
	      {
		std::ostringstream msg;
		msg << "Value of instanceof reference type ";
		msg << eTok.getTokenStringFromPool(&m_state).c_str();
		msg << " is missing for '";
		//msg << identTok.getTokenStringFromPool(&m_state).c_str();
		msg << m_state.getTokenDataAsString(&identTok).c_str() << "'";
		MSG(&eTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		((NodeVarRef *) dNode)->setInitExpr(rightNode);
	      }
	  }
	else if(eTok.m_type == TOK_OPEN_PAREN)
	  {
	    //allows casting of reference initialization
	    Node * rightNode = parseRestOfCastOrExpression();
	    if(!rightNode)
	      {
		std::ostringstream msg;
		msg << "Value of casted reference type ";
		msg << " is missing for '";
		msg << m_state.getTokenDataAsString(&identTok).c_str() << "'";
		MSG(&eTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		((NodeVarRef *) dNode)->setInitExpr(rightNode);
	      }
	  }
	else //error
	  {
	    std::ostringstream msg;
	    msg << "Unexpected token <" << eTok.getTokenStringFromPool(&m_state).c_str();
	    msg << "> for initial value of reference";
	    MSG(&eTok, msg.str().c_str(), ERR);
	  }
	args.m_declRef = ALT_NOT; //clear flag in case of decl list
	//keep args.m_referenced type???
      } //ref done
    else
      {
	Node * assignNode = parseAssignExpr(); //makeAssignExprNode(leftNode);
	if(!assignNode)
	  {
	    std::ostringstream msg;
	    msg << "Initial value of variable " << identTok.getTokenStringFromPool(&m_state).c_str();
	    msg << " is missing";
	    MSG(&identTok, msg.str().c_str(), ERR);
	  }
	else
	  {
	    dNode->setInitExpr(assignNode);
	  }
      }
    return parseRestOfDecls(args, identTok, dNode, rtnNode, passuti); //any more?
  } //parseRestOfDeclAssignment

  NodeConstantDef * Parser::parseRestOfConstantDef(NodeConstantDef * constNode, bool assignREQ, bool isStmt)
  {
    NodeConstantDef * rtnNode = constNode;
    Token pTok;
    if(getExpectedToken(TOK_EQUAL, pTok, QUIETLY))
      {
	Node * exprNode = parseExpression();
	if(exprNode)
	  constNode->setConstantExpr(exprNode);
	else
	  {
	    std::ostringstream msg;
	    msg << "Missing named constant definition after '=' for '";
	    msg << m_state.m_pool.getDataAsString(constNode->getSymbolId()).c_str() << "'";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
      }
    else
      {
	//let the = constant expr be optional in case of class params
	if(assignREQ)
	  {
	    std::ostringstream msg;
	    msg << "Missing '=' after named constant definition '";
	    msg << m_state.m_pool.getDataAsString(constNode->getSymbolId()).c_str() << "'";
	    MSG(&pTok, msg.str().c_str(), ERR);

	    if(isStmt)
	      {
		//perhaps read until semi-colon
		getTokensUntil(TOK_SEMICOLON);
		unreadToken();

		delete constNode; //also deletes the symbol, and nodetypedesc.
		constNode = NULL;
		rtnNode = NULL;
	      }
	  }
	else
	  {
	    //unreadToken(); //class param doesn't have equal; wait for the class arg
	  }
      }

    if(isStmt)
      {
	if(!getExpectedToken(TOK_SEMICOLON))
	  {
	    std::ostringstream msg;
	    msg << "Missing ';' after named constant definition '";
	    msg << m_state.m_pool.getDataAsString(constNode->getSymbolId()).c_str() << "'";
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

  NodeBlockFunctionDefinition * Parser::makeFunctionBlock(TypeArgs& args, Token identTok, NodeTypeDescriptor * nodetype, bool isVirtual)
  {
    NodeBlockFunctionDefinition * rtnNode = NULL;

    //all functions are defined in this "class" block; or external 'use' for declarations.
    //this is a block with its own ST
    NodeBlockClass * currClassBlock = m_state.getClassBlock();
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

    assert(nodetype);
    UTI rtnuti = nodetype->givenUTI();

    SymbolFunction * fsymptr = new SymbolFunction(identTok, rtnuti, m_state);
    fsymptr->setStructuredComment(); //also clears

    if(isVirtual)
      fsymptr->setVirtualFunction();

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

    m_state.pushCurrentBlock(rtnNode); //before parsing the args

    //use space on funcCallStack for return statement.
    //negative for parameters; allot space at top for the return value
    //currently, only scalar; determines start position of first arg "under".
    s32 returnArraySize = m_state.slotsNeeded(fsymptr->getUlamTypeIdx());

    //extra one for "hidden" first arg, Ptr to its Atom
    //maxdepth now re-calculated after parsing due to some still unknown sizes;
    //blockdeclsize: 0, <1, >1 means: datamember, parameter, local variable, respectively
    m_state.m_currentFunctionBlockDeclSize = -(returnArraySize + 1);
    m_state.m_currentFunctionBlockMaxDepth = 0;

    //create "self" symbol for the class type; btw, it's really a ref.
    //belongs to the function definition scope.
    u32 selfid = m_state.m_pool.getIndexForDataString("self");
    UTI cuti = currClassBlock->getNodeType(); //luckily we know this now for each class used
    Token selfTok(TOK_IDENTIFIER, identTok.m_locator, selfid);
    SymbolVariableStack * selfsym = new SymbolVariableStack(selfTok, m_state.getUlamTypeAsRef(cuti, ALT_REF), m_state.m_currentFunctionBlockDeclSize, m_state);
    selfsym->setAutoLocalType(ALT_REF);
    selfsym->setIsSelf();
    m_state.addSymbolToCurrentScope(selfsym); //ownership goes to the block

    //parse and add parameters to function symbol (not in ST yet!)
    parseRestOfFunctionParameters(fsymptr, rtnNode);

    if(rtnNode)
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

    if(rtnNode)
      {
	//starts with positive one for local variables
	m_state.m_currentFunctionBlockDeclSize = 1;
	m_state.m_currentFunctionBlockMaxDepth = 0;

	//parse body definition
	if(parseFunctionBody(rtnNode))
	  {
	    rtnNode->setDefinition();
	    rtnNode->setMaxDepth(m_state.m_currentFunctionBlockMaxDepth);
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
    m_state.m_currentFunctionBlockDeclSize = 0; //default zero for datamembers
    m_state.m_currentFunctionBlockMaxDepth = 0; //reset
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
    else if(Token::isTokenAType(pTok))
      {
	unreadToken();
	Node * argNode = parseDecl(SINGLEDECL); //singleton
	Symbol * argSym = NULL;

	//could be null symbol already in scope
	if(argNode)
	  {
	    //parameter IS a variable (declaration).
	    if(argNode->getSymbolPtr(argSym))
	      {
		//ownership stays with NodeBlockFunctionDefinition's ST
		fsym->addParameterSymbol(argSym);
	      }
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
    else
      {
	std::ostringstream msg;
	msg << "Expected 'A Type' Token!! got Token <";
	msg << m_state.getTokenDataAsString(&pTok).c_str();
	msg << "> instead";
	MSG(&pTok, msg.str().c_str(), ERR);
	//continue or short-circuit?
      }

    if(getExpectedToken(TOK_COMMA)) //if so, get next parameter; o.w. unread
      {
	if(fsym->takesVariableArgs())
	  MSG(&pTok, "Variable args indication (...) appears at end of parameter list", ERR);
      }
    return parseRestOfFunctionParameters(fsym, fblock);
  } //parseRestOfFunctionParameters

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

	NodeStatements * nextNode;
	if(pTok.m_type == TOK_CLOSE_CURLY)
	  {
	    nextNode = new NodeBlockEmpty(m_state.getCurrentBlock(), m_state); //legal
	    assert(nextNode);
	    nextNode->setNodeLocation(pTok.m_locator);
	  }
	else
	  nextNode = (NodeStatements *) parseStatements();

	funcNode->setNextNode(nextNode);

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
	    msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(&qTok).c_str();
	    msg << "> after non-virtual function declaration";
	    MSG(&qTok, msg.str().c_str(), ERR);
	  }
      }
    else
      {
	unreadToken();
	std::ostringstream msg;
	msg << "Unexpected input!! Token <" << m_state.getTokenDataAsString(&qTok).c_str();
	msg << "> after function declaration";
	MSG(&qTok, msg.str().c_str(), ERR);
      }
    return brtn;
  } //parseFunctionBody

  Node * Parser::makeFunctionSymbol(TypeArgs& args, Token identTok, NodeTypeDescriptor * nodetype, bool isVirtual)
  {
    //first check that the function name begins with a lower case letter
    if(Token::isTokenAType(identTok))
      {
	std::ostringstream msg;
	msg << "Function <" << m_state.getTokenDataAsString(&identTok).c_str();
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
    if(m_state.getClassBlock()->isIdInScope(identTok.m_dataindex,asymptr) && !asymptr->isFunction())
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
    Node * rtnNode = makeFunctionBlock(args, identTok, nodetype, isVirtual);

    //exclude the declaration & definition from the parse tree
    //(since in SymbolFunction) & return NULL.
    Token qTok;
    getNextToken(qTok);

    if(rtnNode)
      {
	if((qTok.m_type != TOK_CLOSE_CURLY) && (((NodeBlockFunctionDefinition *) rtnNode)->isNative() && qTok.m_type != TOK_SEMICOLON))
	  {
	    //first remove the pointer to this node from its symbol
	    ((NodeBlockFunctionDefinition *) rtnNode)->getFuncSymbolPtr()->setFunctionNode((NodeBlockFunctionDefinition *) NULL); //deletes node
	    rtnNode = NULL;
	    MSG(&qTok, "INCOMPLETE Function Definition", ERR);
	  }
      }
    return rtnNode;
  } //makeFunctionSymbol

  Node * Parser::makeVariableSymbol(TypeArgs& args, Token identTok, NodeTypeDescriptor *& nodetyperef)
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
		//installSymbol failed for other reasons (e.g. problem with []); rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid variable declaration of base type <";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str() << "> and Name <";
		msg << m_state.getTokenDataAsString(&identTok).c_str() << "> (missing symbol)";
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

  Node * Parser::makeTypedefSymbol(TypeArgs& args, Token identTok, NodeTypeDescriptor *& nodetyperef)
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
		//installSymbol failed for other reasons
		//(e.g. problem with []) , error already output. rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid typedef of base type <";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str();
		msg << "> and Name <" << m_state.getTokenDataAsString(&identTok).c_str();
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

  Node * Parser::makeConstdefSymbol(TypeArgs& args, Token identTok, NodeTypeDescriptor *& nodetyperef)
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
		//installSymbol failed for other reasons (e.g. problem with []), error already output.
		//rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid constant definition of Type <";
		msg << m_state.getTokenAsATypeName(args.m_typeTok).c_str();
		msg << "> and Name <" << m_state.getTokenDataAsString(&identTok).c_str();
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

	    rtnNode = parseRestOfConstantDef(constNode, args.m_assignOK, args.m_isStmt);
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

  Node * Parser::makeParameterSymbol(TypeArgs& args, Token identTok, NodeTypeDescriptor *& nodetyperef)
  {
    NodeModelParameterDef * rtnNode = NULL;
    Node * lvalNode = parseIdentExpr(identTok); //calls parseLvalExpr
    if(lvalNode)
      {
	//lvalNode could be either a NodeIdent or a NodeSquareBracket,
	// though arrays not legal in this context!!!
	//process identifier...check if already defined in current scope; if not, add it;
	//return a SymbolParameterValue else some sort of primitive
	Symbol * asymptr = NULL;
	if(!lvalNode->installSymbolParameterValue(args, asymptr))
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
		msg << " " << m_state.getTokenDataAsString(&identTok).c_str();
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

	    NodeModelParameterDef * paramNode =  new NodeModelParameterDef((SymbolParameterValue *) asymptr, nodetyperef, m_state);
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
  } //makeParameterSymbol

  Node * Parser::makeConditionalExprNode(Node * leftNode)
  {
    Node * rtnNode = NULL;

    //reparse (is-has-as) function token
    Token fTok;
    getNextToken(fTok);

    //parse Type
    Token tTok;
    getNextToken(tTok);

    if(!Token::isTokenAType(tTok))
      {
	std::ostringstream msg;
	msg << "Right operand of conditional-" << fTok.getTokenStringFromPool(&m_state).c_str();
	msg << " is not a Type: ";
	msg << tTok.getTokenStringFromPool(&m_state).c_str() << "; Operation deleted";
	MSG(&tTok, msg.str().c_str(), ERR);
	delete leftNode;
	m_state.m_parsingConditionalAs = false;
	return NULL;
      }

    //consider possibility of Class Instance as Type
    bool isaclass = true;
    UTI cuti = parseClassArguments(tTok, isaclass);
    if(!m_state.isScalar(cuti))
      {
	std::ostringstream msg;
	msg << "Right operand of conditional-" << fTok.getTokenStringFromPool(&m_state).c_str() << " is an array: ";
	msg << tTok.getTokenStringFromPool(&m_state).c_str() << "; Operation deleted";
	MSG(&tTok, msg.str().c_str(), ERR);
	delete leftNode;
	m_state.m_parsingConditionalAs = false;
	return NULL;
      }

    TypeArgs typeargs;
    typeargs.init(tTok);
    typeargs.m_classInstanceIdx = cuti; //is scalar
    typeargs.setdeclref(fTok, cuti);

    NodeTypeDescriptor * typeNode = new NodeTypeDescriptor(tTok, cuti, m_state);
    assert(typeNode);

    switch(fTok.m_type)
      {
      case TOK_KW_IS:
	rtnNode = new NodeConditionalIs(leftNode, typeNode, m_state);
	break;
      case TOK_KW_AS:
	rtnNode = new NodeConditionalAs(leftNode, typeNode, m_state);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << " Unexpected input!! Token <";
	  msg << m_state.getTokenDataAsString(&fTok).c_str() << ">, aborting";
	  MSG(&fTok, msg.str().c_str(), DEBUG);
	  assert(0);
	}
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

    Node * rightNode = parseAssignExpr();
    if(!rightNode)
      {
	std::ostringstream msg;
	msg << "Right operand of binary operator" << pTok.getTokenStringFromPool(&m_state).c_str();
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
	  case TOK_AMP_EQUAL:
	    rtnNode = new NodeBinaryOpEqualBitwiseAnd(leftNode, rightNode, m_state);
	    break;
	  case TOK_PIPE_EQUAL:
	    rtnNode = new NodeBinaryOpEqualBitwiseOr(leftNode, rightNode, m_state);
	    break;
	  case TOK_HAT_EQUAL:
	    rtnNode = new NodeBinaryOpEqualBitwiseXor(leftNode, rightNode, m_state);
	    break;
	  default:
	    {
	      std::ostringstream msg;
	      msg << " Unexpected input!! Token <";
	      msg << m_state.getTokenDataAsString(&pTok).c_str() << ">, aborting";
	      MSG(&pTok, msg.str().c_str(), DEBUG);
	      assert(0);
	    }
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
	msg << "Right operand of binary operator" << pTok.getTokenStringFromPool(&m_state).c_str();
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
	    {
	      std::ostringstream msg;
	      msg << " Unexpected input!! Token <";
	      msg  << m_state.getTokenDataAsString(&pTok).c_str() << ">, aborting";
	      MSG(&pTok, msg.str().c_str(), DEBUG);
	      assert(0);
	    }
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
	msg << "Right operand of binary operator" << pTok.getTokenStringFromPool(&m_state).c_str();
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
	    {
	      std::ostringstream msg;
	      msg << " Unexpected input!! Token <";
	      msg << m_state.getTokenDataAsString(&pTok).c_str() << ">, aborting";
	      MSG(&pTok, msg.str().c_str(), DEBUG);
	      assert(0);
	    }
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
	msg << "Right operand of binary operator" << pTok.getTokenStringFromPool(&m_state).c_str();
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
	    {
	      std::ostringstream msg;
	      msg << " Unexpected input!! Token <";
	      msg << m_state.getTokenDataAsString(&pTok).c_str() << ">, aborting";
	      MSG(&pTok, msg.str().c_str(), DEBUG);
	      assert(0);
	    }
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
	msg << "Right operand of binary operator" << pTok.getTokenStringFromPool(&m_state).c_str();
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
	    {
	      std::ostringstream msg;
	      msg << " Unexpected input!! Token <";
	      msg << m_state.getTokenDataAsString(&pTok).c_str() << ">, aborting";
	      MSG(&pTok, msg.str().c_str(), DEBUG);
	      assert(0);
	    }
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
	msg << "Right operand of binary operator" << pTok.getTokenStringFromPool(&m_state).c_str();
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
	    {
	      std::ostringstream msg;
	      msg << " Unexpected input!! Token <";
	      msg << m_state.getTokenDataAsString(&pTok).c_str() << ">, aborting";
	      MSG(&pTok, msg.str().c_str(), DEBUG);
	      assert(0);
	    }
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
	msg << "Right operand of binary operator" << pTok.getTokenStringFromPool(&m_state).c_str();
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
	    {
	      std::ostringstream msg;
	      msg << " Unexpected input!! Token <";
	      msg << m_state.getTokenDataAsString(&pTok).c_str() << ">, aborting";
	      MSG(&pTok, msg.str().c_str(), DEBUG);
	      assert(0);
	    }
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
	msg << "Right operand of binary operator" << pTok.getTokenStringFromPool(&m_state).c_str();
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
	    {
	      std::ostringstream msg;
	      msg << " Unexpected input!! Token <";
	      msg << m_state.getTokenDataAsString(&pTok).c_str() << ">, aborting";
	      MSG(&pTok, msg.str().c_str(), DEBUG);
	      assert(0);
	    }
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeTermNode

  Node * Parser::makeFactorNode()
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    Node * factorNode = parseFactor();
    if(!factorNode)
      {
	std::ostringstream msg;
	msg << "Factor is missing; Unary operation " << pTok.getTokenStringFromPool(&m_state).c_str() << " deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	return NULL;
      }

    switch(pTok.m_type)
      {
      case TOK_MINUS:
	{
	  UTI futi = factorNode->getNodeType();
	  if((futi != Nouti) && factorNode->isAConstant())
	    {
	      factorNode->constantFoldAToken(pTok);
	      rtnNode = factorNode;
	    }
	  else
	    {
	      rtnNode = new NodeUnaryOpMinus(factorNode, m_state);
	      assert(rtnNode);
	      rtnNode->setNodeLocation(pTok.m_locator);
	    }
	}
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
	rtnNode = new NodeBinaryOpEqualArithAdd(factorNode, makeTerminal(pTok, (s64) 1, Int), m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      case TOK_MINUS_MINUS:
	rtnNode = new NodeBinaryOpEqualArithSubtract(factorNode, makeTerminal(pTok, (s64) 1, Int), m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << " Unexpected input!! Token <" << m_state.getTokenDataAsString(&pTok).c_str();
	  msg << ">, aborting";
	  MSG(&pTok, msg.str().c_str(), DEBUG);
	  assert(0);
	}
	break;
      };

    return rtnNode;
  } //makeFactorNode

  Node * Parser::makeCastNode(Token typeTok)
  {
    Node * rtnNode = NULL;
    UTI typeToBe = Nouti;
    TypeArgs typeargs;
    typeargs.init(typeTok);

    //we want the casting UTI, without deleting any failed dots because, why?
    // because it might be a minof,maxof,sizeof..which wouldn't be a cast at all!
    NodeTypeDescriptor * typeNode = parseTypeDescriptor(typeargs, typeToBe, false);
    assert(typeNode);

    Token eTok;
    getNextToken(eTok);
    if(eTok.m_type == TOK_AMP)
      {
	UTI refuti = m_state.getUlamTypeAsRef(typeNode->givenUTI());
	typeargs.m_declRef = ALT_REF;
	typeNode->setReferenceType(ALT_REF, typeNode->givenUTI(), refuti);
	typeargs.m_referencedUTI = typeNode->getReferencedUTI(); //typeNode->givenUTI();
	getNextToken(eTok);
      }

    if(eTok.m_type == TOK_CLOSE_PAREN)
      {
	//typeNode tfrs to owner to NodeCast
	Node * nodetocast = parseFactor();
	if(nodetocast)
	  {
	    rtnNode = new NodeCast(nodetocast, typeToBe, typeNode, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(typeargs.m_typeTok.m_locator);
	    ((NodeCast *) rtnNode)->setExplicitCast();
	  }
	else
	  {
	    delete typeNode;
	    typeNode = NULL;
	  }
      }
    else
      {
	//not a cast anymore..
	UTI uti = typeNode->givenUTI();
	//as in parseFactor, returns either a terminal or proxy
	//optionally, gets next dot token
	rtnNode = parseMinMaxSizeofType(typeTok, uti, typeNode);
	if(rtnNode)
	  rtnNode = parseRestOfExpression(rtnNode);

	if(!rtnNode)
	  {
	    delete typeNode;
	    typeNode = NULL;
	  }

	if(!getExpectedToken(TOK_CLOSE_PAREN))
	  {
	    delete rtnNode;
	    rtnNode = NULL;
	  }
      }
    return rtnNode;
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
	msg << "(Low Level) " << m_state.getTokenDataAsString(&tok).c_str();
	MSG(&tok, msg.str().c_str(), ERR);
	brtn = m_tokenizer->getNextToken(tok);
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

  void Parser::initPrimitiveUlamTypes()
  {
    //initialize "primitive" UlamTypes, in order!!
    // unfortunately, Nav, Atom, Class (except quarks with toInt), Ptr (PtrAbs) and Holder
    // are not considered PRIMITIVE during type processing (use ut->isPrimitiveType());
    UlamKeyTypeSignature nokey(m_state.m_pool.getIndexForDataString("0Nouti"), ULAMTYPE_DEFAULTBITSIZE[Nouti]);
    AssertBool isNouti = (m_state.makeUlamType(nokey, Nouti) == Nouti);
    assert(isNouti); //true for primitives

    UlamKeyTypeSignature nkey(m_state.m_pool.getIndexForDataString("0Nav"), ULAMTYPE_DEFAULTBITSIZE[Nav]);
    AssertBool isNav = (m_state.makeUlamType(nkey, Nav) == Nav);
    assert(isNav); //true for primitives

    UlamKeyTypeSignature zkey(m_state.m_pool.getIndexForDataString("0Hzy"), ULAMTYPE_DEFAULTBITSIZE[Hzy]);
    AssertBool isHzy = (m_state.makeUlamType(zkey, Hzy) == Hzy);
    assert(isHzy); //true for primitives

    UlamKeyTypeSignature vkey(m_state.m_pool.getIndexForDataString("Void"), ULAMTYPE_DEFAULTBITSIZE[Void]);
    AssertBool isVoid = (m_state.makeUlamType(vkey, Void) == Void);
    assert(isVoid); //true for primitives

    UlamKeyTypeSignature ikey(m_state.m_pool.getIndexForDataString("Int"), ULAMTYPE_DEFAULTBITSIZE[Int]);
    AssertBool isInt = (m_state.makeUlamType(ikey, Int) == Int);
    assert(isInt);

    UlamKeyTypeSignature uikey(m_state.m_pool.getIndexForDataString("Unsigned"), ULAMTYPE_DEFAULTBITSIZE[Unsigned]);
    AssertBool isUnsigned = (m_state.makeUlamType(uikey, Unsigned) == Unsigned);
    assert(isUnsigned);

    UlamKeyTypeSignature bkey(m_state.m_pool.getIndexForDataString("Bool"), ULAMTYPE_DEFAULTBITSIZE[Bool]);
    AssertBool isBool = (m_state.makeUlamType(bkey, Bool) == Bool);
    assert(isBool);

    UlamKeyTypeSignature ukey(m_state.m_pool.getIndexForDataString("Unary"), ULAMTYPE_DEFAULTBITSIZE[Unary]);
    AssertBool isUnary = (m_state.makeUlamType(ukey, Unary) == Unary);
    assert(isUnary);

    UlamKeyTypeSignature bitskey(m_state.m_pool.getIndexForDataString("Bits"), ULAMTYPE_DEFAULTBITSIZE[Bits]);
    AssertBool isBits = (m_state.makeUlamType(bitskey, Bits) == Bits);
    assert(isBits);

    UlamKeyTypeSignature ckey(m_state.m_pool.getIndexForDataString("0Class"), ULAMTYPE_DEFAULTBITSIZE[Class]); //bits tbd
    AssertBool isClass = (m_state.makeUlamType(ckey, Class) == Class);
    assert(isClass);

    UlamKeyTypeSignature akey(m_state.m_pool.getIndexForDataString("Atom"), ULAMTYPE_DEFAULTBITSIZE[UAtom]);
    AssertBool isUAtom = (m_state.makeUlamType(akey, UAtom) == UAtom);
    assert(isUAtom);

    UlamKeyTypeSignature pkey(m_state.m_pool.getIndexForDataString("0Ptr"), ULAMTYPE_DEFAULTBITSIZE[Ptr]);
    AssertBool isPtrRel = (m_state.makeUlamType(pkey, Ptr) == Ptr);
    assert(isPtrRel);

    UlamKeyTypeSignature hkey(m_state.m_pool.getIndexForDataString("0Holder"), UNKNOWNSIZE);
    AssertBool isHolder = (m_state.makeUlamType(hkey, Holder) == Holder);
    assert(isHolder);

    //a Ptr for absolute indexing (i.e. reference class params); comes after Holder.
    UlamKeyTypeSignature apkey(m_state.m_pool.getIndexForDataString("0Ptr"), ULAMTYPE_DEFAULTBITSIZE[Ptr], NONARRAYSIZE, ALT_PTR);
    AssertBool isPtrAbs = (m_state.makeUlamType(apkey, Ptr) == PtrAbs);
    assert(isPtrAbs);

    //a Ref for .storageof; comes after PtrAbs.
    UlamKeyTypeSignature arefkey(m_state.m_pool.getIndexForDataString("Atom"), ULAMTYPE_DEFAULTBITSIZE[UAtom], NONARRAYSIZE, ALT_REF);
    AssertBool isAtomRef = (m_state.makeUlamType(arefkey, UAtom) == UAtomRef);
    assert(isAtomRef);

    // next in line, the 64 basics:
    m_state.getLongUTI();
    m_state.getUnsignedLongUTI();
    m_state.getBigBitsUTI();

    //initialize call stack with 'Int' UlamType pointer
    m_state.m_funcCallStack.init(Int);
    m_state.m_nodeEvalStack.init(Int);
    //m_state.m_eventWindow.init(iidx); //necessary?
  } //initPrimitiveUlamTypes

} //end MFM
