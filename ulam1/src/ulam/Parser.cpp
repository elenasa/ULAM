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
#include "NodeConditionalIs.h"
#include "NodeConditionalHas.h"
#include "NodeConstant.h"
#include "NodeContinueStatement.h"
#include "NodeControlIf.h"
#include "NodeControlWhile.h"
#include "NodeLabel.h"
#include "NodeMemberSelect.h"
#include "NodeReturnStatement.h"
#include "NodeTerminalProxy.h"
#include "NodeSquareBracket.h"
#include "NodeTerminal.h"
#include "NodeIdent.h"
#include "NodeTypeBitsize.h"
#include "NodeTypedef.h"
#include "NodeSimpleStatement.h"
#include "NodeStatementEmpty.h"
#include "NodeUnaryOpMinus.h"
#include "NodeUnaryOpPlus.h"
#include "NodeUnaryOpBang.h"
#include "NodeVarDecl.h"
#include "SymbolFunction.h"
#include "SymbolFunctionName.h"
#include "SymbolVariableStack.h"
#include "SymbolClass.h"
#include "SymbolClassName.h"


namespace MFM {

#define QUIETLY true
#define NOASSIGN false
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
    m_state.m_parsingInProgress = true;

    if(errOutput)
      m_state.m_err.setFileOutput(errOutput);

    //add name of the thing we are compiling to string pool (and node program);
    //dropping the .ulam suffix
    u32 foundSuffix = startstr.find(".ulam");
    if(foundSuffix == std::string::npos        //.ulam not found
       || foundSuffix != startstr.length()-5   //ensure it's a suffix
       || foundSuffix == 0)                    //and not also a prefix
      {
	std::ostringstream msg;
        msg << "File name <" << startstr << "> doesn't end with '.ulam'";
	MSG("",msg.str().c_str() , ERR);
	return 1; //1 error
      }

    std::string compileThis = startstr.substr(0,foundSuffix);

    char c = compileThis.at(0);
    if(!Token::isUpper(c))
      {
	//compileThis.at(0) = 'A' + (c - 'a'); //uppercase
	std::ostringstream msg;
	msg << "File name <" << startstr << "> must match a valid class name (uppercase) to compile";
	MSG("", msg.str().c_str() , ERR);
	return  1; //1 error
      }

    u32 compileThisId = m_state.m_pool.getIndexForDataString(compileThis);

    //here's the start (first token)!!  preparser will handle the VERSION_DECL,
    //as well as USE and LOAD keywords.
    while(!parseThisClass());

    //here when no more classes, or there was an error
    Symbol * thisClassSymbol = NULL;
    NodeBlockClass * rootNode = NULL;

    if(m_state.m_programDefST.isInTable(compileThisId, thisClassSymbol))
      rootNode = ((SymbolClass *) thisClassSymbol)->getClassBlockNode();

    if(!rootNode)
      {
	std::ostringstream msg;
	msg << "No parse tree for This Class: " << compileThis;
	MSG("", msg.str().c_str(), ERR);
      }

    u32 warns = m_state.m_err.getWarningCount();
    if(warns > 0)
      {
	std::ostringstream msg;
	msg << warns << " warning" << (warns > 1 ? "s " : " ") << "during parsing: " << compileThis;
	MSG((rootNode ? rootNode->getNodeLocationAsString().c_str() : ""), msg.str().c_str(), INFO);
      }

    u32 errs = m_state.m_err.getErrorCount();
    if(errs > 0)
      {
	std::ostringstream msg;
	msg << errs << " TOO MANY PARSE ERRORS: " << compileThis;
	MSG((rootNode ? rootNode->getNodeLocationAsString().c_str() : ""), msg.str().c_str(), INFO);
      }

    m_state.m_parsingInProgress = false;
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
        return true;
      }

    if((pTok.m_type != TOK_KW_ELEMENT) && (pTok.m_type != TOK_KW_QUARK) && (pTok.m_type != TOK_KW_QUARKUNION))
      {
	std::ostringstream msg;
	msg << "Invalid Class Type: <";
	msg << m_state.getTokenDataAsString(&pTok).c_str();
	msg << ">; KEYWORD should be: '";
	msg << Token::getTokenAsString(TOK_KW_ELEMENT);
	msg << "', '";
	msg << Token::getTokenAsString(TOK_KW_QUARK);
	msg << "', or '";
	msg << Token::getTokenAsString(TOK_KW_QUARKUNION);
	msg << "'";
	MSG(&pTok, msg.str().c_str(), ERR);
	return true; //we're done.
      }

    //each class has its own parse tree; each "compileThis", in turn, has code generated later.
    Token iTok;
    getNextToken(iTok);

    //insure the class name starts with a capital letter, and is not a primitive (e.g. TOK_TYPE_INT)
    if(iTok.m_type != TOK_TYPE_IDENTIFIER)
      {
	std::ostringstream msg;
	msg << "Poorly named class <" << m_state.getTokenDataAsString(&iTok).c_str();
	msg << ">: Identifier must begin with an upper-case letter";
	MSG(&iTok, msg.str().c_str(), ERR);
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
		msg << "Duplicate or incomplete class <";
		msg << m_state.m_pool.getDataAsString(cnSym->getId()).c_str() << ">";
		MSG(&iTok, msg.str().c_str(), ERR);

		return  true; //we're done unless we can gobble the rest up?
	      }
	    wasIncomplete = true;
	  }
      }
    else
      {
	SymbolClassNameTemplate * ctSym;
	if(!m_state.alreadyDefinedSymbolClassNameTemplate(iTok.m_dataindex, ctSym))
	  {
	    m_state.addIncompleteClassSymbolToProgramTable(iTok, ctSym); //overloaded
	  }
	else
	  {
	    //if already defined, then must be incomplete, else duplicate!!
	    if(ctSym->getUlamClass() != UC_UNSEEN)
	      {
		std::ostringstream msg;
		msg << "Duplicate or incomplete template class <";
		msg << m_state.m_pool.getDataAsString(ctSym->getId()).c_str() << ">";
		MSG(&iTok, msg.str().c_str(), ERR);

		return  true; //we're done unless we can gobble the rest up?
	      }
	    wasIncomplete = true;
	  }
	cnSym = ctSym;
      } //template

    //mostly needed for code gen later, but this is where we first know it!
    m_state.pushClassContext(cnSym->getUlamTypeIdx(), cnSym->getClassBlockNode(), cnSym->getClassBlockNode(), false, NULL); //null blocks likely

    //set class type in UlamType (through its class symbol) since we know it;
    //UC_UNSEEN if unseen so far.
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
	msg << "Empty/Incomplete Class Definition: <";
	msg << m_state.getTokenDataAsString(&iTok).c_str();
	msg << ">; possible missing ending curly brace";
	MSG(&pTok, msg.str().c_str(), WARN);
      }

    return false; //keep going until EOF is reached
  } //parseThisClass

  NodeBlockClass * Parser::parseClassBlock(SymbolClassName * cnsym, Token identTok)
  {
    UTI utype = cnsym->getUlamTypeIdx(); //we know its type..sweeter
    NodeBlockClass * rtnNode = cnsym->getClassBlockNode(); //usually NULL
    if(!rtnNode)
      {
	NodeBlock * prevBlock = m_state.getCurrentBlock();
	assert(prevBlock == NULL); //this is the class' first block

	rtnNode = new NodeBlockClass(prevBlock, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(identTok.m_locator);
	rtnNode->setNodeType(utype);

	//set block before returning, so future class instances can link back to it
	cnsym->setClassBlockNode(rtnNode);
      }

    //current, this block's symbol table added to parse tree stack
    //        for validating and finding scope of program/block variables
    //class block has 2 ST: functions and data member decls, separate
    //m_state.popClassContext(); //keep on stack for name id
    m_state.pushClassContext(cnsym->getUlamTypeIdx(), rtnNode, rtnNode, false, NULL);

    //need class block's ST before parsing any class parameters (i.e. named constants);
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_OPEN_PAREN)
      parseRestOfClassParameters((SymbolClassNameTemplate *) cnsym);
    else
      unreadToken();

    if(!getExpectedToken(TOK_OPEN_CURLY, pTok))
      {
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

    return rtnNode;
  } //parseClassBlock

 void Parser::parseRestOfClassParameters(SymbolClassNameTemplate * cntsym)
  {
    assert(cntsym);

    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      {
	if(cntsym->getNumberOfParameters() == 0)
	  {
	    std::ostringstream msg;
	    msg << "Class Template has NO parameters: <";
	    msg << m_state.getUlamTypeNameByIndex(cntsym->getUlamTypeIdx()).c_str();
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
	Node * argNode = parseConstdef(NOASSIGN); //named constants
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
	  }
	delete argNode; //no longer needed
      }
    else
      {
	std::ostringstream msg;
	msg << "Expected 'A Type' Token!! got Token: <";
	msg << m_state.getTokenDataAsString(&pTok).c_str();
	msg << "> instead for class parameter declaration";
	MSG(&pTok, msg.str().c_str(), ERR);
	//continue or short-circuit?
      }

    getExpectedToken(TOK_COMMA, QUIETLY); //if so, get next parameter; o.w. unread

    return parseRestOfClassParameters(cntsym);
  } //parseRestOfClassParameters

  bool Parser::parseDataMember(NodeStatements *& nextNode)
  {
    bool brtn = false;
    Node * rtnNode = NULL;
    Token pTok, iTok;
    getNextToken(pTok);

    //parse Typedef's starting with keyword first
    if(pTok.m_type == TOK_KW_TYPEDEF)
      {
	if((rtnNode = parseTypedef()) )
	  {
	    if(!getExpectedToken(TOK_SEMICOLON))
	      {
		delete rtnNode;
		rtnNode = NULL;
		getTokensUntil(TOK_SEMICOLON); //does this help?
	      }
	    else
	      {
		brtn = true;
		nextNode = new NodeStatements(rtnNode, m_state);
		assert(nextNode);
		nextNode->setNodeLocation(rtnNode->getNodeLocation());
	      }
	  }
	  return brtn;
      } //typedef done.

    //parse Named Constant starting with keyword first
    if(pTok.m_type == TOK_KW_CONSTDEF)
      {
	if((rtnNode = parseConstdef()) )
	  {
	    if(!getExpectedToken(TOK_SEMICOLON))
	      {
		delete rtnNode;
		rtnNode = NULL;
		getTokensUntil(TOK_SEMICOLON); //does this help?
	      }
	    else
	      {
		brtn = true;
		nextNode = new NodeStatements(rtnNode, m_state);
		assert(nextNode);
		nextNode->setNodeLocation(rtnNode->getNodeLocation());
	      }
	  }
	return brtn;
      } //constdef done.

    m_state.m_parsingElementParameterVariable = false;
    //static element parameter
    if(pTok.m_type == TOK_KW_ELEMENT)
      {
	//currently only permitted in elements, no quarks
	UTI cuti = m_state.getClassBlock()->getNodeType();
	ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(cuti)->getUlamClass();
	if(classtype != UC_ELEMENT)
	  {
	    std::ostringstream msg;
	    msg << "Only elements may have element parameters: <";
	    msg << m_state.m_pool.getDataAsString(m_state.getCompileThisIdx()).c_str();
	    msg << "> is a quark";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	m_state.m_parsingElementParameterVariable = true;
	getNextToken(pTok);
      }

    if(!Token::isTokenAType(pTok))
      {
	unreadToken();
	m_state.m_parsingElementParameterVariable = false; //clear on error
	return false;
      }

    NodeTypeBitsize * bitsizeNode = NULL;
    ParserTypeArgs typeargs;
    typeargs.init(pTok);

    if(m_state.getBaseTypeFromToken(pTok) == Class)
      {
	UTI cuti = parseClassArguments(pTok); //not sure what to do with the UTI?
	typeargs.classInstanceIdx = cuti;
	Token dTok;
	getNextToken(dTok);
	unreadToken();
	if(dTok.m_type == TOK_DOT)
	  {
	    if(!parseTypeFromAnotherClassesTypedef(typeargs))
	      {
		m_state.m_parsingElementParameterVariable = false; //clear on error
		return false; //expecting a type, not sizeof, probably an error
	      }
	  }
      }
    else
      {
	//check for Type bitsize specifier;
	typeargs.bitsize = 0;
	bitsizeNode = parseTypeBitsize(typeargs);
      }

    getNextToken(iTok);
    if(iTok.m_type == TOK_IDENTIFIER)
      {
	//need another token to distinguish a function from a variable declaration (do so quietly)
	if(getExpectedToken(TOK_OPEN_PAREN, QUIETLY))
	  {
	    //eats the '(' when found; NULL if error occurred
	    rtnNode = makeFunctionSymbol(typeargs, iTok, bitsizeNode); //with params

	    Token qTok;
	    getNextToken(qTok);

	    if(rtnNode)
	      {
		if((qTok.m_type != TOK_CLOSE_CURLY) && (((NodeBlockFunctionDefinition *) rtnNode)->isNative() && qTok.m_type != TOK_SEMICOLON))
		  {
		    //first remove the pointer to this node from its symbol
		    ((NodeBlockFunctionDefinition *) rtnNode)->getFuncSymbolPtr()->setFunctionNode((NodeBlockFunctionDefinition *) NULL); //deletes node
		    rtnNode = NULL;
		    MSG(&pTok, "INCOMPLETE Function Definition", ERR);
		  }
		else
		  {
		    brtn = true; //rtnNode belongs to the symbolFunction
		  }
	      }
	    //else
	    //MSG(&pTok, "INCOMPLETE Function Definition", ERR);
	  }
	else
	  {
	    //not '(', so token is unread, and we know
	    //it's a variable, not a function;
	    //also handles arrays
	    typeargs.declListOrTypedefScalarType = Nav;
	    rtnNode = makeVariableSymbol(typeargs, iTok, bitsizeNode);

	    if(rtnNode)
	      {
		typeargs.assignOK = NOASSIGN;
		rtnNode = parseRestOfDecls(typeargs, iTok, rtnNode);
	      }
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
      }
    else
      {
	//user error!
	std::ostringstream msg;
	msg << "Name of variable/function <" << m_state.getTokenDataAsString(&iTok).c_str();
	msg << ">: Identifier must begin with a lower-case letter";
	MSG(&iTok, msg.str().c_str(), ERR);
	unreadToken();
      }
    m_state.m_parsingElementParameterVariable = false;
    return brtn;
  } //parseDataMember

  Node * Parser::parseBlock()
  {
    Token pTok;
    NodeBlock * rtnNode = NULL;
    NodeBlock * prevBlock = m_state.getCurrentBlock();

    if(!getExpectedToken(TOK_OPEN_CURLY, pTok))
      return NULL;

    if(getExpectedToken(TOK_CLOSE_CURLY, QUIETLY))
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

    if(nextNode)  //could be Null, in case of errors
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
	MSG("", "EMPTY STATEMENT!! when in doubt, count", WARN);

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

    if(!getExpectedToken(TOK_CLOSE_CURLY, QUIETLY))
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
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  //eat error token
	}
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
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

    Node * condNode = parseConditionalExpr();
    if(!condNode)
      return NULL; //stop this maddness

    if(!getExpectedToken(TOK_CLOSE_PAREN))
      {
	delete condNode;
	return NULL;
      }

    Node * trueNode = NULL;
    if(m_state.m_parsingConditionalAs)
      {
	trueNode = setupAsConditionalBlockAndParseStatements((NodeConditionalAs *) condNode);
      }
    else
      {
	trueNode = parseStatement();
      }

    if(!trueNode)
      {
	delete condNode;
	return NULL; //stop this maddness
      }

    //wrapping body in NodeStatements produces proper comment for genCode
    NodeStatements * trueStmtNode = new NodeStatements(trueNode, m_state);
    assert(trueStmtNode);
    trueStmtNode->setNodeLocation(trueNode->getNodeLocation());

    Node * falseStmtNode = NULL;

    if(getExpectedToken(TOK_KW_ELSE, QUIETLY))
      {
	Node * falseNode = parseStatement();
	if(falseNode != NULL)
	  {
	    falseStmtNode = new NodeStatements(falseNode, m_state);
	    assert(falseStmtNode);
	    falseStmtNode->setNodeLocation(falseNode->getNodeLocation());
	  }
      }

    Node * rtnNode = new NodeControlIf(condNode, trueStmtNode, falseStmtNode, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(ifTok.m_locator);
    return rtnNode;
  } //parseControlIf

  Node * Parser::parseControlWhile(Token wTok)
  {
    if(!getExpectedToken(TOK_OPEN_PAREN))
      {
	return NULL;
      }

    s32 controlLoopLabelNum = m_state.m_parsingControlLoop; //save at the top
    Node * condNode = parseConditionalExpr();
    if(!condNode)
      return NULL; //stop this maddness

    if(!getExpectedToken(TOK_CLOSE_PAREN))
      {
	delete condNode;
	return NULL;
      }

    Node * trueNode = NULL;
    if(m_state.m_parsingConditionalAs)
      trueNode = setupAsConditionalBlockAndParseStatements((NodeConditionalAs *) condNode);
    else
      trueNode = parseStatement();

    if(!trueNode)
      {
	delete condNode;
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

    Node * rtnNode = new NodeControlWhile(condNode, trueStmtNode, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(wTok.m_locator);
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
	declNode = parseDecl();        //updates symbol table
	getNextToken(pTok);
      }

    if(pTok.m_type != TOK_SEMICOLON)
      {
	delete rtnNode;
	delete declNode; //stop this maddness
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
	    return NULL; //stop this maddness
	  }

	if(!getExpectedToken(TOK_SEMICOLON))
	  {
	    delete rtnNode;
	    delete declNode;
	    delete condNode;
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
	    return NULL; //stop this maddness
	  }

	if(!getExpectedToken(TOK_CLOSE_PAREN))
	  {
	    delete rtnNode;
	    delete declNode;
	    delete condNode;
	    delete assignNode;
	    return NULL; //stop this maddness
	  }
      } //done with assign expr, could be null

    Node * trueNode = NULL;
    if(m_state.m_parsingConditionalAs)
      {
	trueNode = setupAsConditionalBlockAndParseStatements((NodeConditionalAs *) condNode);
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
	if(!(rtnNode = parseIdentExpr(iTok)))
	  return parseExpression();  	//continue as parseAssignExpr

	//next check for 'as' only (is-has are Factors now)
	Token cTok;
	getNextToken(cTok);
	unreadToken();
	if(cTok.m_type == TOK_KW_AS)
	  {
	    m_state.saveIdentTokenForConditionalAs(iTok); //sets m_state.m_parsingConditionalAs
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

  Node * Parser::setupAsConditionalBlockAndParseStatements(NodeConditionalAs * asNode)
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

    UTI tuti = asNode->getRightType();
    UlamType * tut = m_state.getUlamTypeByIndex(tuti);
    const std::string tdname = tut->getUlamTypeNameOnly();
    Token typeTok;
    typeTok.init(TOK_TYPE_IDENTIFIER, asNode->getNodeLocation(), m_state.m_pool.getIndexForDataString(tdname));

    ParserTypeArgs typeargs;
    typeargs.init(typeTok);
    typeargs.bitsize = tut->getBitSize();
    typeargs.arraysize = tut->getArraySize();
    typeargs.classInstanceIdx = tuti; //?

    Symbol * asymptr = NULL; //a place to put the new symbol; not a decl list, nor typedef from another class
    tmpni->installSymbolVariable(typeargs, asymptr);
    assert(asymptr);
    asymptr->setAutoLocal(); //set auto flag

    delete tmpni; //done with nti
    tmpni = NULL;
    m_state.m_parsingConditionalAs = false; //done with flag and identToken.

    //insert var decl into NodeStatements..as if parseStatement was called..
    Node * varNode = new NodeVarDecl((SymbolVariable*) asymptr, m_state);
    assert(varNode);
    varNode->setNodeLocation(asNode->getNodeLocation());

    NodeStatements * stmtsNode = new NodeStatements(varNode, m_state);
    assert(stmtsNode);
    stmtsNode->setNodeLocation(varNode->getNodeLocation());

    blockNode->setNextNode(stmtsNode);

    if(!singleStmtExpected)
      {
	if(!getExpectedToken(TOK_CLOSE_CURLY, QUIETLY))
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
	rtnNode = new NodeStatementEmpty(m_state);  	//empty statement
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    else if(Token::isTokenAType(pTok))
      {
	unreadToken();
	rtnNode = parseDecl();        //updates symbol table
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
	unreadToken();               //needs location
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
    else if(pTok.m_type == TOK_ERROR_CONT)
      {
	MSG(&pTok, "Unexpected input!! ERROR Token, Continue", ERR);
	//eat error token
      }
    else if(pTok.m_type == TOK_ERROR_ABORT)
      {
	MSG(&pTok, "Unexpected input!! ERROR Token, Exiting..", ERR);
	exit(1);
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

    if(!getExpectedToken(TOK_SEMICOLON))
      {
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
	NodeTypeBitsize * bitsizeNode = NULL;
	ParserTypeArgs typeargs;
	typeargs.init(pTok);
	typeargs.bitsize = UNKNOWNSIZE;

	if(m_state.getBaseTypeFromToken(pTok) == Class)
	  {
	    UTI cuti = parseClassArguments(pTok);
	    typeargs.classInstanceIdx = cuti;
	    Token dTok;
	    getNextToken(dTok);
	    unreadToken();
	    if(dTok.m_type == TOK_DOT)
	      {
		if(!parseTypeFromAnotherClassesTypedef(typeargs))
		  return rtnNode; //error if not a typedef, right?
	      }
	  }
	else
	  {
	    //check for Type bitsize specifier;
	    typeargs.bitsize = 0;
	    bitsizeNode = parseTypeBitsize(typeargs);
	  }

	Token iTok;
	getNextToken(iTok);
	//insure the typedef name starts with a capital letter
	if(iTok.m_type == TOK_TYPE_IDENTIFIER)
	  {
	    rtnNode = makeTypedefSymbol(typeargs, iTok, bitsizeNode);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid typedef Alias <" << m_state.getTokenDataAsString(&iTok).c_str();
	    msg << ">, Type Identifier (2nd arg) requires capitalization";
	    MSG(&iTok, msg.str().c_str(), ERR);
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
  Node * Parser::parseConstdef(bool assignOK)
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    if(Token::isTokenAType(pTok))
      {
	NodeTypeBitsize * bitsizeNode = NULL;
	ParserTypeArgs typeargs;
	typeargs.init(pTok);
	typeargs.bitsize = 0;
	typeargs.assignOK = assignOK;

	//check for Type bitsize specifier;
	bitsizeNode = parseTypeBitsize(typeargs); //ref

	Token iTok;
	getNextToken(iTok);
	//insure the typedef name starts with a lower case letter
	if(iTok.m_type == TOK_IDENTIFIER)
	  {
	    rtnNode = makeConstdefSymbol(typeargs, iTok, bitsizeNode);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid constant-def Alias <";
	    msg << m_state.getTokenDataAsString(&iTok).c_str();
	    msg << ">, Constant Identifier (2nd arg) requires lower-case";
	    MSG(&iTok, msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Invalid constant-def Type <";
	msg << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	MSG(&pTok, msg.str().c_str(), ERR);
      }
    return rtnNode;
  } //parseConstdef

  //used for data members or local function variables; or
  //'singledecl' function parameters; no longer for function defs.
  Node * Parser::parseDecl(bool parseSingleDecl)
  {
    Node * rtnNode = NULL;
    Token pTok, iTok;
    getNextToken(pTok);

    //should have already known to be true
    assert(Token::isTokenAType(pTok));

    NodeTypeBitsize * bitsizeNode = NULL;
    ParserTypeArgs typeargs;
    typeargs.init(pTok);

    if(m_state.getBaseTypeFromToken(pTok) == Class)
      {
	UTI cuti = parseClassArguments(pTok); //not sure what to do with the UTI?
	typeargs.classInstanceIdx = cuti;
	Token dTok;
	getNextToken(dTok);
	unreadToken();
	if(dTok.m_type == TOK_DOT)
	  {
	    if(!parseTypeFromAnotherClassesTypedef(typeargs)) //refs
	      return rtnNode; //decl with sizeof in type, not good.
	  }
      }
    else
      {
	//check for Type bitsize specifier;
	typeargs.bitsize = 0;
	bitsizeNode = parseTypeBitsize(typeargs); //refs
      }

    getNextToken(iTok);
    if(iTok.m_type == TOK_IDENTIFIER)
      {
	typeargs.declListOrTypedefScalarType = Nav; //first one!
	rtnNode = makeVariableSymbol(typeargs, iTok, bitsizeNode);
	if(rtnNode && !parseSingleDecl)
	  {
	    //for multi's of same type (rtnType), and/or its assignment
	    return parseRestOfDecls(typeargs, iTok, rtnNode);
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
      }
    return rtnNode;
  } //parseDecl

  UTI Parser::parseClassArguments(Token& typeTok)
  {
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type != TOK_OPEN_PAREN)
      {
	//regular class, not a template
	unreadToken();
	SymbolClassName * cnsym = NULL;
	if(!m_state.alreadyDefinedSymbolClassName(typeTok.m_dataindex, cnsym))
	  {
	    //check if a typedef first..if so, return its SCALAR uti
	    UTI tduti;
	    UTI tdscalaruti = Nav;
	    if(m_state.getUlamTypeByTypedefName(typeTok.m_dataindex, tduti, tdscalaruti))
	      {
		if(tdscalaruti != Nav)
		  return tdscalaruti;
		return m_state.getUlamTypeAsScalar(tduti); //may make new uti
	      }
	    else
	      m_state.addIncompleteClassSymbolToProgramTable(typeTok, cnsym);
	  }
	else
	  {
	    if(cnsym->isClassTemplate())
	      {
		//params but no args
		std::ostringstream msg;
		msg << "Missing Class Arguments for an instance stub of class template <";
		msg << m_state.m_pool.getDataAsString(cnsym->getId()).c_str() << ">";
		MSG(&pTok, msg.str().c_str(), ERR);
	      }
	  }
	assert(cnsym);
	return cnsym->getUlamTypeIdx();
      } //not open paren

    //must be a template class
    SymbolClassNameTemplate * ctsym = NULL;
    if(!m_state.alreadyDefinedSymbolClassNameTemplate(typeTok.m_dataindex, ctsym))
      m_state.addIncompleteClassSymbolToProgramTable(typeTok, ctsym); //was undefined, template

    assert(ctsym);

    UTI cuti = ctsym->getUlamTypeIdx();
    u32 numParams = ctsym->getNumberOfParameters();

    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      {
	if(numParams > 0)
	  {
	    //params but no args
	    std::ostringstream msg;
	    msg << "NO Class Arguments for an instance stub of class template <";
	    msg << m_state.m_pool.getDataAsString(ctsym->getId()).c_str();
	    msg << "> with " << numParams << " parameters";
	    MSG(&pTok, msg.str().c_str(), ERR);
	    cuti = Nav;
	  }
	return cuti; //ok to return
      }

    unreadToken(); //not close paren yet

    //make a (shallow) Class Instance Stub to collect class args as SymbolConstantValues;
    //has its own uti that will become part of its key; (too soon for a deep copy!)
    cuti = m_state.makeUlamType(typeTok, UNKNOWNSIZE, NONARRAYSIZE, Nav); //overwrites the template type here.
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    ((UlamTypeClass *) cut)->setUlamClass(ctsym->getUlamClass()); //possibly UC_UNSEEN

    UlamType * ctut = m_state.getUlamTypeByIndex(ctsym->getUlamTypeIdx());
    if(ctut->isCustomArray())
      ((UlamTypeClass *) cut)->setCustomArrayType(((UlamTypeClass *) ctut)->getCustomArrayType());

    SymbolClass * csym = ctsym->makeAStubClassInstance(typeTok, cuti);

    u32 parmidx = 0;
    parseRestOfClassArguments(csym, ctsym, parmidx);

    bool ctUnseen = (ctsym->getUlamClass() == UC_UNSEEN);
    if(!ctUnseen && (parmidx < ctsym->getNumberOfParameters()))
      {
	std::ostringstream msg;
	msg << "Too few Class Arguments parsed, " << "(" << parmidx << "), for template: ";
	msg << m_state.m_pool.getDataAsString(ctsym->getId()).c_str() ;
	msg << ", by " << m_state.getUlamTypeNameBriefByIndex(csym->getUlamTypeIdx()).c_str() ;
	MSG(&typeTok, msg.str().c_str(), ERR);
      }
    return cuti;
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
	msg << "Class Argument after Open Paren is missing, for template: <";
	msg << m_state.m_pool.getDataAsString(csym->getId()).c_str() << ">";
	MSG(&pTok, msg.str().c_str(), ERR);
	// what else is missing?
	return;
      }

    //this is possible if cnsym is UC_UNSEEN, must check args later..
    bool ctUnseen = (ctsym->getUlamClass() == UC_UNSEEN);
    if(!ctUnseen && parmIdx >= ctsym->getNumberOfParameters())
      {
	std::ostringstream msg;
	msg << "Too many Class Arguments " << "(" << parmIdx + 1 << "), class template: ";
	msg << m_state.m_pool.getDataAsString(ctsym->getId()).c_str();
	msg << " expects " << ctsym->getNumberOfParameters();
	MSG(&pTok, msg.str().c_str(), ERR);
      }
    else
      {
	//try to continue..
	m_state.pushCurrentBlock(csym->getClassBlockNode()); //reset here for new arg's ST

	SymbolConstantValue * argSym;
	if(!ctUnseen)
	  {
	    SymbolConstantValue * paramSym = (SymbolConstantValue * ) (ctsym->getParameterSymbolPtr(parmIdx));
	    assert(paramSym);
	    argSym = new SymbolConstantValue(paramSym->getId(), paramSym->getUlamTypeIdx(), m_state); //like param, not copy
	  }
	else
	  {
	    std::ostringstream sname;
	    sname << "_" << parmIdx;
	    u32 snameid = m_state.m_pool.getIndexForDataString(sname.str());
	    //stub id,  m_state.getUlamTypeOfConstant(Int) stub type, state
	    argSym = new SymbolConstantValue(snameid, Int, m_state);
	  }

	assert(argSym);
	argSym->setParameterFlag();
	m_state.addSymbolToCurrentScope(argSym); //scope updated to new class instance in parseClassArguments

	m_state.popClassContext(); //restore before making NodeConstantDef, so current context

	//make Node with argument symbol to try to fold const expr;
	//o.w. add to list of unresolved for this uti
	NodeConstantDef * constNode = new NodeConstantDef(argSym, m_state);
	assert(constNode);
	constNode->setNodeLocation(pTok.m_locator);
	constNode->setConstantExpr(exprNode);
	//fold later; non ready expressions saved by UTI in m_nonreadyClassArgSubtrees (stub)
	csym->linkConstantExpressionForPendingArg(constNode);
      } //too many args

    getNextToken(pTok);
    if(pTok.m_type != TOK_COMMA)
      unreadToken();

    return parseRestOfClassArguments(csym, ctsym, ++parmIdx); //recurse
  } //parseRestOfClassArguments

  //return NodeTypeBitsize when unsuccessful evaluating its constant expression
  //up to caller to delete it
  NodeTypeBitsize * Parser::parseTypeBitsize(ParserTypeArgs& args)
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
	    msg << m_state.getTokenAsATypeName(args.typeTok).c_str();
	    MSG(&bTok, msg.str().c_str(), ERR);
	  }
	else
	  {
	    rtnNode = new NodeTypeBitsize(bitsizeNode, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(args.typeTok.m_locator);
	    args.bitsize = UNKNOWNSIZE; //no eval yet
	  }

	if(!getExpectedToken(TOK_CLOSE_PAREN))
	  {
	    args.bitsize = UNKNOWNSIZE;
	    delete rtnNode;
	    rtnNode = NULL;
	  }
      }
    else if(bTok.m_type == TOK_DOT)
      {
	unreadToken();
	parseTypeFromAnotherClassesTypedef(args); //ref; might update anothertduti
	//else, what does false return mean? 'sizeof' or unseen class, perhaps.
      }
    else
      {
	unreadToken(); //not open paren, bitsize is unspecified
      }
    return rtnNode; //typebitsize const expr
  } //parseTypeBitsize

  bool Parser::parseTypeFromAnotherClassesTypedef(ParserTypeArgs& args)
  {
    u32 numDots = 0;
    bool foundTypedef = true;
    parseTypeFromAnotherClassesTypedef(args, foundTypedef, numDots);

    if(numDots > 1)
      return true; //definitely found a typedef

    return foundTypedef; //one time only
  } //parseTypeFromAnotherClassesTypedef (call)

  //recursively parses classtypes and their typedefs (dot separated)
  //into their UTI alias; the typeTok and return bitsize ref args
  //represent the UTI; false if 'sizeof' or something other than a type
  //was found after the first dot; countDots.
  void Parser::parseTypeFromAnotherClassesTypedef(ParserTypeArgs& args, bool& rtnb, u32& numDots)
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
    if(!m_state.alreadyDefinedSymbolClassName(args.typeTok.m_dataindex, cnsym))
      {
	//if here, the last typedef might have been a holder for some unknown type
	// now we know (thanks to the dot and subsequent type token) that its a holder
	//  for a class. but we don't know it's real name, yet!

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
	    cnsym = m_state.makeAnonymousClassFromHolder(args.anothertduti, args.typeTok.m_locator);
	    args.classInstanceIdx = args.anothertduti; //since we didn't know last time around
	  }
	else
	  {
	    unreadToken(); //put dot back, minof or maxof perhaps?
	    std::ostringstream msg;
	    msg << "Unexpected input!! Token: <" << args.typeTok.getTokenEnumName();
	    msg << "> is not a 'seen' class type: <";
	    msg << m_state.getTokenDataAsString(&args.typeTok).c_str() << ">";
	    MSG(&args.typeTok, msg.str().c_str(), DEBUG);
	    rtnb = false;
	    return;
	  }
      }

    // either found a class or made one up, continue..
    assert(cnsym);
    SymbolClass * csym = NULL;
    if(cnsym->isClassTemplate() && args.classInstanceIdx)
      {
	if(! ((SymbolClassNameTemplate *)cnsym)->findClassInstanceByUTI(args.classInstanceIdx, csym))
	  {
	    std::ostringstream msg;
	    msg << "Trying to use typedef from another class template <";
	    msg << m_state.m_pool.getDataAsString(cnsym->getId()).c_str();
	    msg << ">, but instance UTI";
	    msg << args.classInstanceIdx << " cannot be found";
	    MSG(&args.typeTok, msg.str().c_str(), ERR);
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
    if(!memberClassNode)  //e.g. forgot the closing brace on quark def once; or UNSEEN
      {
	//hail mary pass..possibly a sizeof of unseen class
	getNextToken(nTok);
	if(nTok.m_type != TOK_KW_SIZEOF)
	  {
	    std::ostringstream msg;
	    msg << "Trying to use typedef from another class <";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str();
	    msg << ">, before it has been defined. Cannot continue with (token) ";
	    msg << m_state.getTokenDataAsString(&nTok).c_str();
	    MSG(&args.typeTok, msg.str().c_str(), ERR);
	    getTokensUntil(TOK_SEMICOLON); //rest of statement is ignored.
	  }
	else
	  {
	    args.bitsize = UNKNOWNSIZE; //t.f. unknown bitsize or arraysize or both?
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
		SymbolTypedef * symtypedef = new SymbolTypedef(pTok.m_dataindex, huti, Nav, m_state);
		assert(symtypedef);
		symtypedef->setBlockNoOfST(memberClassNode->getNodeNo());
		m_state.addSymbolToCurrentMemberClassScope(symtypedef);
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
		msg << " found for Typedef: <" << m_state.getTokenDataAsString(&pTok).c_str();
		msg << ">, belonging to class: " << m_state.m_pool.getDataAsString(csym->getId()).c_str();
		MSG(&pTok, msg.str().c_str(), DEBUG);
	      }

	    ULAMCLASSTYPE tdclasstype = tdut->getUlamClass();
	    const std::string tdname = tdut->getUlamTypeNameOnly();

	    //update token argument
	    if(tdclasstype == UC_NOTACLASS)
	      args.typeTok.init(Token::getTokenTypeFromString(tdname.c_str()), pTok.m_locator, 0);
	    else
	      {
		args.typeTok.init(TOK_TYPE_IDENTIFIER, pTok.m_locator, m_state.m_pool.getIndexForDataString(tdname));
		isclasstd = true;
	      }
	    //update rest of argument refs
	    args.bitsize = tdut->getBitSize();
	    args.arraysize = tdut->getArraySize(); //becomes arg when installing symbol

	    //possibly another class? go again..
	    if(isclasstd)
	      {
		if(args.anothertduti != Nav)
		  args.classInstanceIdx = args.anothertduti;
		else
		  args.classInstanceIdx = tduti;
	      }

	    args.anothertduti = tduti; //don't lose it!
	    args.declListOrTypedefScalarType = tdscalaruti;
	    rtnb = true;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str();
	    msg << "> is not a typedef belonging to class: ";
	    msg << m_state.m_pool.getDataAsString(csym->getId()).c_str();
	    MSG(&pTok, msg.str().c_str(), ERR);
	    rtnb = false;
	  }

	//possibly another class? go again..
	parseTypeFromAnotherClassesTypedef(args, rtnb, numDots);
      }
    else
      {
	args.bitsize = UNKNOWNSIZE; //t.f. unknown bitsize or arraysize or both?
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
	rtnExprNode = new NodeStatementEmpty(m_state); //has Nav type
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
	if(m_state.alreadyDefinedSymbol(iTok.m_dataindex,sym))
	  {
	    if(sym->isConstant())
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
    //may continue when symbol not defined yet (e.g. Decl)
    // don't return a NodeConstant, instead of NodeIdent, without arrays
    // even if already defined as one.
    m_state.alreadyDefinedSymbol(identTok.m_dataindex,asymptr);

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
	//may continue when symbol not defined yet (e.g. FuncCall)
	m_state.alreadyDefinedSymbol(identTok.m_dataindex,asymptr);
	if(asymptr && !asymptr->isFunction())
	  {
	    std::ostringstream msg;
	    msg << "Undefined function <" << m_state.getTokenDataAsString(&identTok).c_str();
	    msg << "> that has already been declared as a variable";
	    MSG(&identTok, msg.str().c_str(), ERR);
	    return  NULL; //bail
	  }
	//function call
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
    if(m_state.alreadyDefinedSymbol(memberTok.m_dataindex, dsymptr))
      rtnNode = parseMinMaxSizeofType(memberTok, dsymptr->getUlamTypeIdx());
    else
      rtnNode = parseMinMaxSizeofType(memberTok);

    if(rtnNode) return rtnNode; //not min/max/sizeof..continue

    // type of dsymptr maybe holder; now we know it should be a class!
    if(dsymptr)
      {
	UTI duti = dsymptr->getUlamTypeIdx();
	UlamType * dut = m_state.getUlamTypeByIndex(duti);
	if(dut->getUlamClass() == UC_NOTACLASS && dut->getUlamTypeEnum() == Holder)
	  {
	    m_state.makeAnonymousClassFromHolder(duti, memberTok.m_locator);
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

  Node * Parser::parseMinMaxSizeofType(Token memberTok, UTI utype)
  {
    Node * rtnNode = NULL;
    UlamType * ut = m_state.getUlamTypeByIndex(utype);
    ULAMCLASSTYPE classtype = ut->getUlamClass();

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
	      assert(classtype == UC_NOTACLASS); //can't be a class and complete
	      rtnNode = makeTerminal(fTok, ut->getTotalBitSize(), Unsigned);
	    }
	  else
	    {
	      //input uti wasn't complete, i.e. based on sizeof some class
	      rtnNode = new NodeTerminalProxy(memberTok, utype, fTok, m_state);
	    }
	}
	break;
      case TOK_KW_MAXOF:
	{
	  if(ut->isMinMaxAllowed())
	    {
	      if(ut->isComplete())
		rtnNode = makeTerminal(fTok, ut->getMax(), utype); //ut->getUlamTypeEnum());
	      else
		rtnNode = new NodeTerminalProxy(memberTok, utype, fTok, m_state);
	    }
	  else
	    {
	      std::ostringstream msg;
	      msg << "Unsupported request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	      msg << "' of variable <" << m_state.getTokenDataAsString(&memberTok).c_str();
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
		rtnNode = makeTerminal(fTok, ut->getMin(), utype); //ut->getUlamTypeEnum());
	      else
		rtnNode = new NodeTerminalProxy(memberTok, utype, fTok, m_state);
	    }
	  else
	    {
	      std::ostringstream msg;
	      msg << "Unsupported request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	      msg << "' of variable <" << m_state.getTokenDataAsString(&memberTok).c_str();
	      msg << ">, type: " << m_state.getUlamTypeNameByIndex(utype).c_str();
	      MSG(&fTok, msg.str().c_str(), ERR);
	    }
	}
	break;
      default:
	{
	  //std::ostringstream msg;
	  //msg << "Undefined request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	  //msg << "' of variable <" << m_state.getTokenDataAsString(&memberTok).c_str();
	  //msg << ">, type: " << m_state.getUlamTypeNameByIndex(utype).c_str();
	  //MSG(&fTok, msg.str().c_str(), DEBUG);
	  unreadToken(); //?
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
	rtnNode = new NodeTerminalProxy(memberTok, Nav, fTok, m_state);
	break;
      case TOK_KW_MAXOF:
	rtnNode = new NodeTerminalProxy(memberTok, Nav, fTok, m_state);
	break;
      case TOK_KW_MINOF:
	rtnNode = new NodeTerminalProxy(memberTok, Nav, fTok, m_state);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unsupported request: '" << m_state.getTokenDataAsString(&fTok).c_str();
	  msg << "' of variable <" << m_state.getTokenDataAsString(&memberTok).c_str();
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
    msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
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
	NodeTypeBitsize * bitsizeNode = NULL;
	UTI uti = Nav;
	ParserTypeArgs typeargs;
	typeargs.init(pTok);

	if(m_state.getBaseTypeFromToken(pTok) == Class)
	  {
	    UTI cuti = parseClassArguments(pTok); //not sure what to do with the UTI?
	    typeargs.classInstanceIdx = cuti;
	    Token dTok;
	    getNextToken(dTok);
	    unreadToken();
	    if(dTok.m_type == TOK_DOT)
	      {
		//may recurse more dots and change pTok! and update uti.
		if(!parseTypeFromAnotherClassesTypedef(typeargs))
		  uti = cuti;
		else
		  uti = typeargs.anothertduti;
	      }
	    else
	      uti = cuti;
	  }
	else
	  {
	    //check for Type bitsize specifier;
	    typeargs.bitsize = 0;
	    bitsizeNode = parseTypeBitsize(typeargs); //refs

	    //could be an incomplete type (an unknown size), to be completed during checkandlabel
	    if(typeargs.anothertduti)
	      uti = typeargs.anothertduti;
	    else
	      uti = m_state.getUlamTypeFromToken(typeargs);
	  }

	//returns either a terminal or proxy
	rtnNode = parseMinMaxSizeofType(pTok, uti); //optionally, get's next dot token
	if(rtnNode)
	  {
	    //bitsize/arraysize is unknown, i.e. based on a Class.sizeof
	    linkFreeMapATypeAndConstantExpressions(uti, typeargs, bitsizeNode, NULL);
	  }
	else
	  {
	    //clean up, some kind of error parsing min/max/sizeof
	    delete bitsizeNode;
	    bitsizeNode = NULL;
	  }
	return rtnNode; //rtnNode could be NULL!
      } //not a Type

    //continue as normal..
    switch(pTok.m_type)
      {
      case TOK_IDENTIFIER:
	{
	  Symbol * asymptr = NULL;
	  if(m_state.alreadyDefinedSymbol(pTok.m_dataindex,asymptr))
	    {
	      //if its an already defined named constant, in current block,
	      //then return a NodeConstant, instead of NodeIdent, without arrays.
	      if(asymptr->isConstant())
		{
		  NodeConstant * rtnNode = new NodeConstant(pTok, (SymbolConstantValue *) asymptr, m_state);
		  assert(rtnNode);
		  rtnNode->setNodeLocation(pTok.m_locator);
		  return rtnNode; //done.
		}
	    }

	  rtnNode = parseIdentExpr(pTok);
	  //test ahead for UNOP_EXPRESSION so that any consecutive binary
	  //ops aren't misinterpreted as a unary operator (e.g. +,-).
	  Token tTok;
	  getNextToken(tTok);
	  unreadToken();
	  if(tTok.m_type == TOK_KW_IS || tTok.m_type == TOK_KW_HAS)
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
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  return parseFactor(); //redo
	}
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <";
	  msg << m_state.getTokenDataAsString(&pTok).c_str() << ">, unreading it";
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
      case TOK_KW_HAS:
	unreadToken();
	assert(leftNode);
	rtnNode = makeConditionalExprNode(leftNode);
	break;
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  //eat token
	}
	break;
      case TOK_ERROR_ABORT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str();
	  msg << ">, exiting..";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  //eat token
	  exit(1);
	}
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
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  rtnNode = parseRestOfShiftExpression(leftNode); //redo
	}
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
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  rtnNode = parseRestOfCompareExpression(leftNode); //redo
	}
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
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  rtnNode = parseRestOfEqExpression(leftNode); //redo
	}
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
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  rtnNode = parseRestOfBitExpression(leftNode); //redo
	}
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
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  rtnNode = parseRestOfLogicalExpression(leftNode); //redo
	}
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
      case TOK_KW_HAS:
	unreadToken();
	rtnNode = parseRestOfFactor(leftNode);
	rtnNode = parseRestOfExpression(rtnNode); //any more?
	break;
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  rtnNode = parseRestOfExpression(leftNode); //redo
	}
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
	MSG(&pTok, "RHS of Open Square is missing->Sq Bracket deleted", ERR);
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

  //assignOK true by default.
  Node * Parser::parseRestOfDecls(ParserTypeArgs& args, Token identTok, Node * dNode)
  {
    Token pTok;
    getNextToken(pTok);

    args.arraysize = NONARRAYSIZE; //clear for decl list (args ref)

    if(pTok.m_type == TOK_EQUAL)
      {
	if(args.assignOK)
	  {
	    unreadToken();
	    return parseRestOfDeclAssignment(args, identTok, dNode); //pass args for more decls
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
	    return dNode;
	  }
      }
    else if(pTok.m_type != TOK_COMMA)
      {
	unreadToken();
	return dNode;
      }

    Node * rtnNode = dNode;
    Token iTok;
    getNextToken(iTok);
    if(iTok.m_type == TOK_IDENTIFIER)
      {
	//another decl of same type
	Node * sNode = makeVariableSymbol(args, iTok, NULL); //a decl
	if (sNode)
	  {
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
    return parseRestOfDecls(args, iTok, rtnNode); //iTok in case of =
  } //parseRestOfDecls

  Node * Parser::parseRestOfDeclAssignment(ParserTypeArgs& args, Token identTok, Node * dNode)
  {
    NodeStatements * rtnNode = new NodeStatements(dNode, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(dNode->getNodeLocation());

    //makeup node for lhs; using same symbol as dNode(could be Null!)
    Symbol * dsymptr = NULL;
    assert(m_state.alreadyDefinedSymbol(identTok.m_dataindex, dsymptr));
    Node * leftNode = new NodeIdent(identTok, (SymbolVariable *) dsymptr, m_state);
    assert(leftNode);
    leftNode->setNodeLocation(dNode->getNodeLocation());

    Node * assignNode = makeAssignExprNode(leftNode);
    if(!assignNode)
      {
	//leftnode was deleted; dNode will be.
	delete rtnNode;
	return NULL;
      }

    NodeStatements * nextNode = new NodeStatements(assignNode, m_state);
    assert(nextNode);
    nextNode->setNodeLocation(assignNode->getNodeLocation());
    rtnNode->setNextNode(nextNode);

    return parseRestOfDecls(args, identTok, rtnNode); //any more?
  } //parseRestOfDeclAssignment

  NodeConstantDef * Parser::parseRestOfConstantDef(NodeConstantDef * constNode, bool assignOK)
  {
    NodeConstantDef * rtnNode = constNode;
    if(assignOK && getExpectedToken(TOK_EQUAL))
      {
	Node * exprNode = parseExpression();
	if(exprNode)
	  {
	    constNode->setConstantExpr(exprNode);
	    if(!constNode->foldConstantExpression())
	      {
		std::ostringstream msg;
		msg << "Named constant <" << constNode->getName() << "> is not 'ready' while parsing";
		MSG(constNode->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);

		//add to non-ready list of subtrees
		m_state.linkConstantExpression(constNode);
	      }
	  }
      }
    else
      {
	//let the = constant expr be optional for class params
	if(assignOK)
	  {
	    //perhaps read until semi-colon
	    getTokensUntil(TOK_SEMICOLON);
	    unreadToken();
	    delete constNode; //does this also delete the symbol?
	    constNode = NULL;
	    rtnNode = NULL;
	  }
	else
	  {
	    unreadToken(); //class param doesn't have equal; wait for the class arg
	  }
      }
    return rtnNode;
  } //parseRestOfConstantDef

  NodeBlockFunctionDefinition * Parser::makeFunctionBlock(ParserTypeArgs& args, Token identTok, NodeTypeBitsize * constExprForBitSize)
  {
    NodeBlockFunctionDefinition * rtnNode = NULL;

    //all functions are defined in this "class" block;
    //or external 'use' for declarations.
    //this is a block with its own ST
    NodeBlockClass * currClassBlock = m_state.getClassBlock();
    NodeBlock * prevBlock = m_state.getCurrentBlock();
    assert(prevBlock == currClassBlock);

    //o.w. build symbol for function: name, return type, plus arg symbols
    //array return types require a typedef alias; lookup is name-based, indep of size args.
    UTI rtnuti = Nav;
    if(args.anothertduti)
      rtnuti = args.anothertduti;
    else if(args.classInstanceIdx) //second in line???
      rtnuti = args.classInstanceIdx;
    else
      rtnuti = m_state.getUlamTypeFromToken(args);

    SymbolFunction * fsymptr = new SymbolFunction(identTok.m_dataindex, rtnuti, m_state);

    linkFreeMapATypeAndConstantExpressions(rtnuti, args, constExprForBitSize, NULL);

    //WAIT for the parameters, so we can add it to the SymbolFunctionName map..
    rtnNode =  new NodeBlockFunctionDefinition(fsymptr, prevBlock, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(args.typeTok.m_locator);

    //symbol will have pointer to body (or just decl for 'use');
    fsymptr->setFunctionNode(rtnNode); //tfr ownership

    //set class type to custom array; the current class block
    //node type was set to its class symbol type after checkAndLabelType
    if(m_state.getCustomArrayGetFunctionNameId() == identTok.m_dataindex)
      {
	UTI cuti = currClassBlock->getNodeType(); //prevBlock
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	((UlamTypeClass *) cut)->setCustomArrayType(rtnuti);
      }

    m_state.pushCurrentBlock(rtnNode); //before parsing the args

    //use space on funcCallStack for return statement.
    //negative for parameters; allot space at top for the return value
    //currently, only scalar; determines start position of first arg "under".
    s32 returnArraySize = m_state.slotsNeeded(fsymptr->getUlamTypeIdx());

    //extra one for "hidden" first arg, Ptr to its Atom
    m_state.m_currentFunctionBlockDeclSize = -(returnArraySize + 1);
    m_state.m_currentFunctionBlockMaxDepth = 0;

    //create "self" symbol whose index is that of the "hidden" first arg (i.e. a Ptr to an Atom);
    //immediately below the return value(s); and belongs to the function definition scope.
    u32 selfid = m_state.m_pool.getIndexForDataString("self");
    UTI cuti = currClassBlock->getNodeType(); //luckily we know this now for each class used
    if(m_state.getUlamTypeByIndex(cuti)->getUlamClass() == UC_QUARK)
      cuti = UAtom; //use atom for quark functions

    SymbolVariableStack * selfsym = new SymbolVariableStack(selfid, cuti, m_state.determinePackable(cuti), m_state.m_currentFunctionBlockDeclSize, m_state);
    selfsym->setIsSelf();
    m_state.addSymbolToCurrentScope(selfsym); //ownership goes to the block

    //parse and add parameters to function symbol (not in ST yet!)
    parseRestOfFunctionParameters(fsymptr);

    //Now, look specifically for a function with the same given name defined
    Symbol * fnSym = NULL;
    if(!currClassBlock->isFuncIdInScope(identTok.m_dataindex, fnSym))
      {
	//first time name used as a function..add symbol function name/typeNav
	fnSym = new SymbolFunctionName(identTok.m_dataindex, Nav, m_state);

	//ownership goes to the class block's ST
	currClassBlock->addFuncIdToScope(fnSym->getId(), fnSym);
      }

    if(rtnNode)
      {
	bool isAdded = ((SymbolFunctionName *) fnSym)->overloadFunction(fsymptr); //transfers ownership, if added
	if(!isAdded)
	  {
	    //this is a duplicate function definition with same parameters and given name!!
	    //return types may differ
	    std::ostringstream msg;
	    msg << "Duplicate defined function '" << m_state.m_pool.getDataAsString(fsymptr->getId());
	    msg << "' with the same parameters" ;
	    MSG(&args.typeTok, msg.str().c_str(), ERR);
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
		msg << "Variable args (...) supported for native functions only at this time; not  <";
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

 void Parser::parseRestOfFunctionParameters(SymbolFunction * fsym)
  {
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      return; //done with parameters

    assert(fsym);

    //allows function name to be same as arg name since the function starts a new "block" (i.e. ST);
    //the argument to parseDecl will prevent it from looking for restofdecls (i.e. singledecl);
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
	Node * argNode = parseDecl(SINGLEDECL);     //singletons
	Symbol * argSym = NULL;

	//could be null symbol already in scope
	if(argNode)
	  {
	    //parameter IS a variable (declaration).
	    if(argNode->getSymbolPtr(argSym))
	      fsym->addParameterSymbol(argSym); //ownership stays with NodeBlockFunctionDefinition's ST
	    else
	      MSG(&pTok, "No symbol from parameter declaration", ERR);
	  }

	delete argNode; //no longer needed
	if(fsym->takesVariableArgs() && argSym)
	  {
	    std::ostringstream msg;
	    msg << "Parameter <" << m_state.m_pool.getDataAsString(argSym->getId()).c_str();
	    msg << "> appears after ellipses (...)";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Expected 'A Type' Token!! got Token: <" << m_state.getTokenDataAsString(&pTok).c_str();
	msg << "> instead";
	MSG(&pTok, msg.str().c_str(), ERR);
	//continue or short-circuit?
      }

    if(getExpectedToken(TOK_COMMA, QUIETLY)) //if so, get next parameter; o.w. unread
      {
	if(fsym->takesVariableArgs())
	  MSG(&pTok, "Variable args indication (...) appears at end of parameter list", ERR);
      }
    return parseRestOfFunctionParameters(fsym);
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
	  MSG(&pTok, "Not close curly", ERR);
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
	msg << "Native Function Definition: <" << funcNode->getName() << ">";
	MSG(&qTok, msg.str().c_str(), INFO);

	brtn = true;
      }
    else
      {
	unreadToken();
	std::ostringstream msg;
	msg << "Unexpected input!! Token: <" << m_state.getTokenDataAsString(&qTok).c_str();
	msg << "> after function declaration.";
	MSG(&qTok, msg.str().c_str(), ERR);
      }
    return brtn;
  } //parseFunctionBody

  Node * Parser::makeFunctionSymbol(ParserTypeArgs& args, Token identTok, NodeTypeBitsize * constExprForBitSize)
  {
    //first check that the function name begins with a lower case letter
    if(Token::isTokenAType(identTok))
      {
	std::ostringstream msg;
	msg << "Function <" << m_state.getTokenDataAsString(&identTok).c_str();
	msg << "> is not a valid (lower case) name";
	MSG(&identTok, msg.str().c_str(), ERR);

	//eat tokens until end of definition ?
	delete constExprForBitSize;
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
	MSG(&args.typeTok, msg.str().c_str(), ERR);

	//eat tokens until end of definition?
	return NULL;
      }

    //not in scope, or not yet defined, or possible overloading
    //o.w. build symbol for function: return type + name + parameter symbols
    Node * rtnNode = makeFunctionBlock(args, identTok, constExprForBitSize);

    //exclude the declaration & definition from the parse tree
    //(since in SymbolFunction) & return NULL.
    return rtnNode;
  } //makeFunctionSymbol

  Node * Parser::makeVariableSymbol(ParserTypeArgs& args, Token identTok, NodeTypeBitsize * constExprForBitSize)
  {
    assert(!Token::isTokenAType(identTok)); //capitalization check done by Lexer

    if(identTok.m_dataindex == m_state.m_pool.getIndexForDataString("self"))
      {
	std::ostringstream msg;
	msg << "The keyword 'self' may not be used as a variable name";
	MSG(&identTok, msg.str().c_str(), ERR);
	//	return NULL;  keep going?
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
		msg << " " << m_state.m_pool.getDataAsString(asymptr->getId()) << "'";
		MSG(&args.typeTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (e.g. problem with [])
		//rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid variable declaration of Type: <";
		msg << m_state.getTokenAsATypeName(args.typeTok).c_str() << "> and Name: <";
		msg << m_state.getTokenDataAsString(&identTok).c_str() << "> (missing symbol)";
		MSG(&args.typeTok, msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    rtnNode =  new NodeVarDecl((SymbolVariable *) asymptr, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(args.typeTok.m_locator);

	    //in case of decl list, set type of symbol in args ref
	    // scalar types return their own uti, not a new one.
	    assert(asymptr);
	    if(args.declListOrTypedefScalarType == Nav && args.anothertduti == Nav)
	      args.declListOrTypedefScalarType = m_state.getUlamTypeAsScalar(asymptr->getUlamTypeIdx());
	  }

	//link square bracket for constant expression, if unknown array size
	//link last arg for constant expression, if unknown bit size
	//o.w. clean up!
	if(rtnNode)
	  {
	    linkFreeMapATypeAndConstantExpressions(asymptr->getUlamTypeIdx(), args, constExprForBitSize, (NodeSquareBracket *) lvalNode);
	  }
	else
	  {
	    delete lvalNode; //done with it
	    delete constExprForBitSize; //done with it
	  }
      }
    else
      {
	delete constExprForBitSize;
      }
    return rtnNode;
  } //makeVariableSymbol

  Node * Parser::makeTypedefSymbol(ParserTypeArgs& args, Token identTok, NodeTypeBitsize * constExprForBitSize)
  {
    NodeTypedef * rtnNode = NULL;
    Node * lvalNode = parseLvalExpr(identTok);
    if(lvalNode)
      {
	//lvalNode could be either a NodeIdent or a NodeSquareBracket
	//process identifier...check if already defined in current scope; if not, add it;
	//returned symbol could be symbolVariable or symbolFunction, detect first.
	Symbol * asymptr = NULL;
	if(!lvalNode->installSymbolTypedef(args, asymptr))
	  {
	    if(asymptr)
	      {
		std::ostringstream msg;
		msg << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
		msg << " has a previous declaration as '";
		msg << m_state.getUlamTypeNameByIndex(asymptr->getUlamTypeIdx()).c_str();
		msg << " " << m_state.m_pool.getDataAsString(asymptr->getId()) << "'";
		MSG(&args.typeTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (e.g. problem with []) , error already output.
		//rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid typedef of Type: <";
		msg << m_state.getTokenAsATypeName(args.typeTok).c_str();
		msg << "> and Name: <" << m_state.getTokenDataAsString(&identTok).c_str();
		msg << "> (missing symbol)";
		MSG(&identTok, msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    rtnNode =  new NodeTypedef((SymbolTypedef *) asymptr, m_state);
	    assert(rtnNode);
	    rtnNode->setNodeLocation(args.typeTok.m_locator);
	  }

	//link square bracket for constant expression, if unknown array size
	//link 2nd arg for constant expression, if unknown bit size
	//o.w. clean up!
	if(rtnNode)
	  {
	    linkFreeMapATypeAndConstantExpressions(asymptr->getUlamTypeIdx(), args, constExprForBitSize, (NodeSquareBracket *) lvalNode);
	  }
	else
	  {
	    delete lvalNode; //done with it
	    delete constExprForBitSize; //done with it
	  }
      }
    else
      {
	delete constExprForBitSize;
      }
    return rtnNode;
  } //makeTypedefSymbol

  Node * Parser::makeConstdefSymbol(ParserTypeArgs& args, Token identTok, NodeTypeBitsize * constExprForBitSize)
  {
    NodeConstantDef * rtnNode = NULL;
    Node * lvalNode = parseIdentExpr(identTok); //calls parseLvalExpr
    if(lvalNode)
      {
	//lvalNode could be either a NodeIdent or a NodeSquareBracket though arrays not legal in this context!!!
	//process identifier...check if already defined in current scope; if not, add it;
	//return a SymbolConstantValue.
	//else some sort of primitive
	Symbol * asymptr = NULL;
	if(!lvalNode->installSymbolConstantValue(args, asymptr))
	  {
	    if(asymptr)
	      {
		std::ostringstream msg;
		msg << m_state.m_pool.getDataAsString(asymptr->getId()).c_str();
		msg << " has a previous declaration as '";
		msg << m_state.getUlamTypeNameByIndex(asymptr->getUlamTypeIdx()).c_str() << " ";
		msg<< m_state.m_pool.getDataAsString(asymptr->getId());
		msg << "' and cannot be used as a named constant";
		MSG(&args.typeTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (e.g. problem with []) , error already output.
		//rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid constant-def of Type: <";
		msg << m_state.getTokenAsATypeName(args.typeTok).c_str();
		msg << "> and Name: <" << m_state.getTokenDataAsString(&identTok).c_str();
		msg << "> (problem with [])";
		MSG(&identTok, msg.str().c_str(), ERR);
	      }
	    //perhaps read until semi-colon
	    getTokensUntil(TOK_SEMICOLON);
	    unreadToken();
	  }
	else
	  {
	    NodeConstantDef * constNode =  new NodeConstantDef((SymbolConstantValue *) asymptr, m_state);
	    assert(constNode);
	    constNode->setNodeLocation(args.typeTok.m_locator);

	    rtnNode = parseRestOfConstantDef(constNode, args.assignOK); //refactored for readability
	  }

	//link square bracket for constant expression, if unknown array size;
	//link last arg for constant expression, if unknown bit size;
	//o.w. clean up!
	if(rtnNode)
	  {
	    linkFreeMapATypeAndConstantExpressions(asymptr->getUlamTypeIdx(), args, constExprForBitSize, (NodeSquareBracket *) lvalNode);
	  }
	else
	  {
	    delete lvalNode; //done with it
	    delete constExprForBitSize; //done with it
	  }
      }
    else
      {
	delete constExprForBitSize;
      }
    return rtnNode;
  } //makeConstdefSymbol

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
	msg << "RHS of operator <" << fTok.getTokenString() << "> is not a Type: ";
	msg << tTok.getTokenString() << ", operation deleted";
	MSG(&tTok, msg.str().c_str(), ERR);
	delete leftNode;
	m_state.m_parsingConditionalAs = false;
	return NULL;
      }

    //consider possibility of Class Instance as Type
    UTI cuti = parseClassArguments(tTok);

    switch(fTok.m_type)
      {
      case TOK_KW_IS:
	rtnNode = new NodeConditionalIs(leftNode, cuti, m_state);
	break;
      case TOK_KW_HAS:
	rtnNode = new NodeConditionalHas(leftNode, cuti, m_state);
	break;
      case TOK_KW_AS:
	  rtnNode = new NodeConditionalAs(leftNode, cuti, m_state);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << " Unexpected input!! Token: <" << m_state.getTokenDataAsString(&fTok).c_str() << ">, aborting";
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
	msg << "RHS of binary operator" << pTok.getTokenString() << " is missing, Assignment deleted";
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
	      msg << " Unexpected input!! Token: <";
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
	msg << "RHS of binary operator" << pTok.getTokenString() << " is missing, operation deleted";
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
	      msg << " Unexpected input!! Token: <";
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
	msg << "RHS of binary operator" << pTok.getTokenString() << " is missing, operation deleted";
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
	      msg << " Unexpected input!! Token: <";
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
	msg << "RHS of binary operator" << pTok.getTokenString() << " is missing, operation deleted";
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
	      msg << " Unexpected input!! Token: <";
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
	msg << "RHS of binary operator" << pTok.getTokenString() << " is missing, operation deleted";
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
	      msg << " Unexpected input!! Token: <";
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
	msg << "RHS of binary operator" << pTok.getTokenString() << " is missing, operation deleted";
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
	      msg << " Unexpected input!! Token: <";
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
	msg << "RHS of binary operator" << pTok.getTokenString() << " is missing, operation deleted";
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
	      msg << " Unexpected input!! Token: <";
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
	msg << "RHS of binary operator" << pTok.getTokenString() << " is missing, operation deleted";
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
	      msg << " Unexpected input!! Token: <";
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
	msg << "Factor is missing, unary operation " << pTok.getTokenString() << " deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
	return NULL;
      }

    switch(pTok.m_type)
      {
      case TOK_MINUS:
	{
	  UTI futi = factorNode->getNodeType();
	  if( (futi != Nav) && factorNode->isAConstant())
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
	rtnNode = new NodeBinaryOpEqualArithAdd(factorNode, makeTerminal(pTok, 1, Int), m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      case TOK_MINUS_MINUS:
	rtnNode = new NodeBinaryOpEqualArithSubtract(factorNode, makeTerminal(pTok, 1, Int), m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << " Unexpected input!! Token: <" << m_state.getTokenDataAsString(&pTok).c_str();
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
    UTI typeToBe = Nav;
    NodeTypeBitsize * bitsizeNode = NULL;
    ParserTypeArgs typeargs;
    typeargs.init(typeTok);

    if(m_state.getBaseTypeFromToken(typeTok) == Class)
      {
	UTI cuti = parseClassArguments(typeTok);
	typeargs.classInstanceIdx = cuti;
	typeToBe = cuti; //unless a dot is next
	Token dTok;
	getNextToken(dTok);
	unreadToken();
	if(dTok.m_type == TOK_DOT)
	  {
	    if(parseTypeFromAnotherClassesTypedef(typeargs))
	      typeToBe = typeargs.anothertduti;
	  }
      }
    else
      {
	//check for Type bitsize specifier;
	typeargs.bitsize = 0;
	bitsizeNode = parseTypeBitsize(typeargs); //refs

	//allows for casting to a class (makes class type if newly seen)
	if(typeargs.anothertduti == Nav)
	  typeToBe = m_state.getUlamTypeFromToken(typeargs);
	else
	  typeToBe = typeargs.anothertduti;
      }

    if(bitsizeNode)
      {
	//bitsize/arraysize is unknown, i.e. based on a Class.sizeof
	linkFreeMapATypeAndConstantExpressions(typeToBe, typeargs, bitsizeNode, NULL);
      }

    if(getExpectedToken(TOK_CLOSE_PAREN))
      {
	rtnNode = new NodeCast(parseFactor(), typeToBe, m_state);
	assert(rtnNode);
	rtnNode->setNodeLocation(typeargs.typeTok.m_locator);
	((NodeCast *) rtnNode)->setExplicitCast();
      }
    return rtnNode;
  } //makeCastNode

  Node * Parser::makeTerminal(Token& locTok, s32 val, UTI utype)
  {
    Node * termNode = new NodeTerminal(val, utype, m_state);
    assert(termNode);
    termNode->setNodeLocation(locTok.m_locator);
    return termNode;
  } //makeTerminal

  Node * Parser::makeTerminal(Token& locTok, u32 val, UTI utype)
  {
    Node * termNode = new NodeTerminal(val, utype, m_state);
    assert(termNode);
    termNode->setNodeLocation(locTok.m_locator);
    return termNode;
  } //makeTerminal

  void Parser::linkFreeMapATypeAndConstantExpressions(UTI auti, ParserTypeArgs args, NodeTypeBitsize * ceForBitSize, NodeSquareBracket * ceForArraySize)
  {
    UlamType * aut = m_state.getUlamTypeByIndex(auti);
    if(aut->isComplete())
      {
	//auti is complete, free all!
	delete ceForArraySize; //done with it
	delete ceForBitSize; //done with it
	return;
      }

    //link arraysize subtree for arraytype based on scalar from another class, OR
    // a local arraytype based on a local scalar uti; o.w. delete.
    // don't keep the ceForArraySize if the type belongs to another class!
    // when also unknown bitsize, we link array to its scalar (below)
    linkOrFreeConstantExpressionArraysize(auti,args,ceForArraySize);

    if(ceForBitSize != NULL) //bitsize subtree
      linkOrFreeConstantExpressionBitsize(auti,args,ceForBitSize);
    else
      linkOrMapAType(auti,args);
  } //linkFreeMapATypeAndConstantExpressions

  void Parser::linkOrMapAType(UTI auti, ParserTypeArgs args)
  {
    //if bitsize is known..(and no bitsize subtree)
    if(m_state.getBitSize(auti) != UNKNOWNSIZE)
      {
	//t.f. arraysize isn't known, since type is not complete
	if(args.classInstanceIdx != Nav)
	  {
	    m_state.linkUnknownTypedefFromAnotherClass(args.anothertduti, args.classInstanceIdx);
	    if(auti != args.anothertduti)
	      {
		//e.g. an array type based on anothertduti scalar.
		m_state.linkIncompleteArrayTypeToItsBaseScalarType(auti, args.anothertduti);
	      }
	  }
	return;
      }

    //bitsize is unknown (and no bitsize subtree)
    //possibly decl list that's shared;
    //scalarUTI of array type should also be included, if not a class
    if(m_state.getArraySize(auti) != NONARRAYSIZE) //an array
      {
	if(args.classInstanceIdx != Nav) //the other class
	  {
	    m_state.linkUnknownTypedefFromAnotherClass(args.anothertduti, args.classInstanceIdx);
	    if(auti != args.anothertduti) //e.g. an array type based on anothertduti scalar.
	      m_state.linkIncompleteArrayTypeToItsBaseScalarType(auti, args.anothertduti);
	    else if(args.declListOrTypedefScalarType != Nav)
	      {
		//an array type based on anothertduti array
		m_state.linkUnknownTypedefFromAnotherClass(args.declListOrTypedefScalarType, args.classInstanceIdx);
	      }
	  }
	else if(args.declListOrTypedefScalarType != Nav && auti != args.declListOrTypedefScalarType) //local array typedef
	  m_state.linkIncompleteArrayTypeToItsBaseScalarType(auti, args.declListOrTypedefScalarType);
	else
	  assert(0); //a bug, likely typedef related (e.g. missing scalarUTI)
      }
    else
      {
	// not an array, and no bitsize subtree
	ULAMCLASSTYPE aclasstype = m_state.getUlamTypeByIndex(auti)->getUlamClass();

	bool fromUnseenClass = false;
	if(args.classInstanceIdx != Nav) //the other class
	  {
	    ULAMCLASSTYPE cclasstype = m_state.getUlamTypeByIndex(args.classInstanceIdx)->getUlamClass();
	    if(cclasstype == UC_UNSEEN)
	      fromUnseenClass = true;
	  }

	if(fromUnseenClass)
	  {
	    mapTypeFromUnseenClass(auti, args);
	  }
	else if(aclasstype == UC_NOTACLASS)
	  {
	    // find the scalardecllist, clone the ceNode for this scalar auti
	    // (not compare, actual uti's equal)
	    if(args.declListOrTypedefScalarType != Nav && auti != args.declListOrTypedefScalarType)
	      m_state.cloneAndLinkConstantExpression(args.declListOrTypedefScalarType, auti);
	    else if(args.classInstanceIdx != Nav)
	      {
		assert(auti == args.anothertduti);
		m_state.linkUnknownTypedefFromAnotherClass(args.anothertduti, args.classInstanceIdx);
	      }
	  }
	else
	  {
	    //auti is a class (no bitsize subtree),
	    //must be a class or a typedef from another class
	    if(args.classInstanceIdx != Nav)
	      {
		if(auti == args.anothertduti)
		  m_state.linkUnknownTypedefFromAnotherClass(args.anothertduti, args.classInstanceIdx);
		else
		  {
		    // nothing to do if a class type (not a typedef), unless auti is UNSEEN.
		    if(aclasstype == UC_UNSEEN)
		      m_state.mapTypesInCurrentClass(auti, args.classInstanceIdx, args.typeTok.m_locator);
		  }
	      }
	  }
      }
  } //linkOrMapAType

  void Parser::mapTypeFromUnseenClass(UTI auti, ParserTypeArgs args)
  {
    // scalar auti may not be the same as anothertduti;
    if(args.anothertduti)
      {
	// map UTIs, and let UT 'compare' help
	m_state.linkUnknownTypedefFromAnotherClass(args.anothertduti, args.classInstanceIdx);
	if(auti != args.anothertduti)
	  {
	    m_state.mapTypesInCurrentClass(auti, args.anothertduti, args.typeTok.m_locator);
	  }
      }
    else
      {
	//e.g. lone class typedef
	m_state.mapTypesInCurrentClass(auti, args.classInstanceIdx, args.typeTok.m_locator);
      }
  } //mapTypeFromUnseenClass

  void Parser::linkOrFreeConstantExpressionArraysize(UTI auti, ParserTypeArgs args, NodeSquareBracket * ceForArraySize)
  {
    //auti is incomplete.

    s32 arraysize = m_state.getArraySize(auti);
    //link arraysize subtree for arraytype based on scalar from another class, OR
    // a local arraytype based on a local scalar uti; o.w. delete.
    // don't keep the ceForArraySize if the type belongs to another class!
    // when also unknown bitsize, we link array to its scalar (below)

    if(arraysize != UNKNOWNSIZE)
      {
	delete ceForArraySize;
	return;
      }

    //typedef is an array from another class
    if(args.anothertduti && args.anothertduti == auti)
      {
	delete ceForArraySize;
	return;
      }

      m_state.linkConstantExpression(auti, ceForArraySize); //tfr owner, or deletes if dup or anothertd

#if 0
    //array here of a typedef from another class's scalar
    if(args.anothertduti && args.anothertduti != auti)
      m_state.linkConstantExpression(auti, ceForArraySize); //tfr owner, or deletes if dup or anothertd
    else
      {
	if(args.classInstanceIdx == Nav)
	  {
	    //local array and scalar
	    if(args.declListOrTypedefScalarType && args.declListOrTypedefScalarType != auti)
	      m_state.linkConstantExpression(auti, ceForArraySize); //tfr owner, or deletes if dup or anothertd
	    else if (m_state.getUlamTypeByIndex(auti)->isHolder()) //didn't know it was an array at the time holder was made
	      m_state.linkConstantExpression(auti, ceForArraySize); //tfr owner, or deletes if dup or anothertd
	    else
	      delete ceForArraySize;
	  }
	else
	  {
	    //an array of classes (typedef)
	    if(args.declListOrTypedefScalarType == args.classInstanceIdx)
	      m_state.linkConstantExpression(auti, ceForArraySize); //tfr owner, or deletes if dup or anothertd
	    else
	      delete ceForArraySize;
	  }
      }
#endif
  } //linkOrFreeConstantExpressionArraysize

  void Parser::linkOrFreeConstantExpressionBitsize(UTI auti, ParserTypeArgs args, NodeTypeBitsize * ceForBitSize)
  {
    //bitsize is known..ok free
    if(m_state.getBitSize(auti) != UNKNOWNSIZE || !ceForBitSize)
      {
	delete ceForBitSize;
	return;
      }

    // we have a bitsize subtree
    //give its scalar type the subtree for unknown bitsize, and link auti to it
    if(m_state.getArraySize(auti) != NONARRAYSIZE) //an array
      {
	UTI scalarUTI = args.declListOrTypedefScalarType;
	if(scalarUTI == Nav)
	  {
	    scalarUTI = m_state.getUlamTypeAsScalar(auti); //may make a new uti
	  }
	assert(scalarUTI != args.anothertduti); //can't be!
	m_state.linkConstantExpression(scalarUTI, ceForBitSize); //tfr owner
	m_state.linkIncompleteArrayTypeToItsBaseScalarType(auti, scalarUTI);
      }
    else
      {
	assert(auti != args.anothertduti); //can't be!
	m_state.linkConstantExpression(auti, ceForBitSize); //tfr owner
      }
  } //linkOrFreeConstantExpressionBitsize

  bool Parser::getExpectedToken(TokenType eTokType, bool quietly)
  {
    Token dropTok;
    return getExpectedToken(eTokType, dropTok, QUIETLY);
  }

  bool Parser::getExpectedToken(TokenType eTokType, Token & myTok, bool quietly)
  {
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type != eTokType)
      {
	unreadToken();

	if(!quietly)
	  {
	    std::ostringstream msg;
	    msg << "Unexpected token <" << pTok.getTokenEnumName() << ">; Expected <";
	    msg << Token::getTokenAsString(eTokType) << ">";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	return false;
      }
    myTok = pTok;
    return true;
  } //getExpectedToken

  bool Parser::getNextToken(Token & tok)
  {
    bool brtn = m_tokenizer->getNextToken(tok);

    if(tok.m_type == TOK_ERROR_ABORT)
      {
	std::ostringstream msg;
	msg << "Unexpected token <" << m_state.getTokenDataAsString(&tok).c_str();
	msg << ">, exiting now";
	MSG(&tok, msg.str().c_str(), ERR);
	exit(1);
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
    //initialize primitive UlamTypes, in order!!
    UlamKeyTypeSignature nkey(m_state.m_pool.getIndexForDataString("0Nav"), ULAMTYPE_DEFAULTBITSIZE[Nav]);
    UTI nidx = m_state.makeUlamType(nkey, Nav);
    assert(nidx == Nav); //true for primitives

    UlamKeyTypeSignature vkey(m_state.m_pool.getIndexForDataString("Void"), ULAMTYPE_DEFAULTBITSIZE[Void]);
    UTI vidx = m_state.makeUlamType(vkey, Void);
    assert(vidx == Void); //true for primitives

    UlamKeyTypeSignature ikey(m_state.m_pool.getIndexForDataString("Int"), ULAMTYPE_DEFAULTBITSIZE[Int]);
    UTI iidx = m_state.makeUlamType(ikey, Int);
    assert(iidx == Int);

    UlamKeyTypeSignature uikey(m_state.m_pool.getIndexForDataString("Unsigned"), ULAMTYPE_DEFAULTBITSIZE[Unsigned]);
    UTI uiidx = m_state.makeUlamType(uikey, Unsigned);
    assert(uiidx == Unsigned);

    UlamKeyTypeSignature bkey(m_state.m_pool.getIndexForDataString("Bool"), ULAMTYPE_DEFAULTBITSIZE[Bool]);
    UTI bidx = m_state.makeUlamType(bkey, Bool);
    assert(bidx == Bool);

    UlamKeyTypeSignature ukey(m_state.m_pool.getIndexForDataString("Unary"), ULAMTYPE_DEFAULTBITSIZE[Unary]);
    UTI uidx = m_state.makeUlamType(ukey, Unary);
    assert(uidx == Unary);

    UlamKeyTypeSignature bitskey(m_state.m_pool.getIndexForDataString("Bits"), ULAMTYPE_DEFAULTBITSIZE[Bits]);
    UTI bitsidx = m_state.makeUlamType(bitskey, Bits);
    assert(bitsidx == Bits);

    UlamKeyTypeSignature ckey(m_state.m_pool.getIndexForDataString("0Class"), ULAMTYPE_DEFAULTBITSIZE[Class]); //bits tbd
    UTI cidx = m_state.makeUlamType(ckey, Class);
    assert(cidx == Class);

    UlamKeyTypeSignature akey(m_state.m_pool.getIndexForDataString("Atom"), ULAMTYPE_DEFAULTBITSIZE[UAtom]);
    UTI aidx = m_state.makeUlamType(akey, UAtom);
    assert(aidx == UAtom);

    UlamKeyTypeSignature pkey(m_state.m_pool.getIndexForDataString("0Ptr"), ULAMTYPE_DEFAULTBITSIZE[Ptr]);
    UTI pidx = m_state.makeUlamType(pkey, Ptr);
    assert(pidx == Ptr);

    UlamKeyTypeSignature hkey(m_state.m_pool.getIndexForDataString("0Holder"), UNKNOWNSIZE);
    UTI hidx = m_state.makeUlamType(hkey, Holder);
    assert(hidx == Holder);

    //initialize call stack with 'Int' UlamType pointer
    m_state.m_funcCallStack.init(iidx);
    m_state.m_nodeEvalStack.init(iidx);
    //m_state.m_eventWindow.init(iidx); //necessary?
  } //initPrimitiveUlamTypes

} //end MFM
