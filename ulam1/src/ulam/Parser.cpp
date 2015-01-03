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
#include "NodeBlockClassEmpty.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeBreakStatement.h"
#include "NodeCast.h"
#include "NodeConditionalIs.h"
#include "NodeConditionalHas.h"
#include "NodeContinueStatement.h"
#include "NodeControlIf.h"
#include "NodeControlWhile.h"
#include "NodeLabel.h"
#include "NodeMemberSelect.h"
#include "NodeProgram.h"
#include "NodeReturnStatement.h"
#include "NodeSquareBracket.h"
#include "NodeTerminal.h"
#include "NodeTerminalIdent.h"
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


namespace MFM {

#define QUIETLY true
#define NOASSIGN false
#define SINGLEDECL true

  Parser::Parser(Tokenizer * izer, CompilerState & state): m_state(state), m_tokenizer(izer)
  {
    // need a block for ST of UlamType names; do this in classblock
    //    init();
  }

  Parser::~Parser()
  {}


  // change to return Node * rather than vector; change tests
  Node * Parser::parseProgram(std::string startstr, File * errOutput)
  {
    if(errOutput)
      m_state.m_err.setFileOutput(errOutput);

    // add name of the thing we are compiling to string pool (and node program);
    // dropping the .ulam suffix
    u32 foundSuffix = startstr.find(".ulam");
    if(foundSuffix == std::string::npos        // .ulam not found
       || foundSuffix != startstr.length()-5   // ensure it's a suffix
       || foundSuffix == 0)                    // and not also a prefix
      {
	std::ostringstream msg;
        msg << "File name <" << startstr << "> doesn't end with '.ulam'";
	MSG("",msg.str().c_str() , ERR);
	return NULL;
      }

    std::string compileThis = startstr.substr(0,foundSuffix);

    char c = compileThis.at(0);
    if(!Token::isUpper(c))
      {
	//compileThis.at(0) = 'A' + (c - 'a');  //uppercase
	std::ostringstream msg;
	msg << "File name <" << startstr << "> must match a valid class name (uppercase) to compile";
	MSG("", msg.str().c_str() , ERR);
	return  NULL;
      }

    u32 compileThisId = m_state.m_pool.getIndexForDataString(compileThis);
    m_state.m_compileThisId = compileThisId;  // for everyone

    initPrimitiveUlamTypes();

    // here's the start (first token)!!  preparser will handle the VERSION_DECL,
    // as well as USE and LOAD keywords. so the first thing we see may not be
    // "our guy"..but we'll know.

    do{
      m_state.m_currentBlock = NULL;  //reset for each new class block
    } while(!parseThisClass());

    // here when THIS class is done, or there was an error
    NodeProgram * P = NULL;
    Symbol * thisClassSymbol = NULL;

    if(m_state.m_programDefST.isInTable(compileThisId, thisClassSymbol))
      {
	NodeBlockClass * rootNode = ((SymbolClass *) thisClassSymbol)->getClassBlockNode();
	if(rootNode)
	  {
	    P = new NodeProgram(compileThisId, m_state);
	    P->setRootNode(rootNode);
	    P->setNodeLocation(rootNode->getNodeLocation());
	    assert(m_state.m_classBlock == rootNode);
	  }
      }


    if(!P)
      {
	MSG("", "No parse tree", ERR);
      }

    u32 warns = m_state.m_err.getWarningCount();
    if(warns > 0)
      {
	std::ostringstream msg;
	msg << warns << " warnings during parsing";
	MSG((P ? P->getNodeLocationAsString().c_str() : ""), msg.str().c_str(), INFO);
      }

    u32 errs = m_state.m_err.getErrorCount();
    if(errs > 0)
      {
	std::ostringstream msg;
	msg << errs << " TOO MANY PARSE ERRORS";
	MSG((P ? P->getNodeLocationAsString().c_str() : ""), msg.str().c_str(), INFO);
      }

    return (P);  //ownership transferred to caller
  }


  bool Parser::parseThisClass()
  {
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_EOF)
      {
        // Nothing else (but haven't found startstr yet)
	MSG(&pTok, "EOF before finding compilation target", ERR);
        return true;
      }

    if( (pTok.m_type != TOK_KW_ELEMENT) && (pTok.m_type != TOK_KW_QUARK) && (pTok.m_type != TOK_KW_QUARKUNION) )
      {
	std::ostringstream msg;
	msg << "Expecting 'quark' or 'element' KEYWORD, NOT <" << pTok.getTokenString() << ">";
	MSG(&pTok, msg.str().c_str(), ERR);

	return true;  //we're done.
      }

    //each class has its own parse tree; only compileThis has code generated later.
    Token iTok;
    getNextToken(iTok);

    //insure the class name starts with a capital letter, and is not a primitive (e.g. TOK_TYPE_INT)
    //if(!(iTok.m_type == TOK_IDENTIFIER && Token::isTokenAType(iTok,&m_state)))
    if(iTok.m_type != TOK_TYPE_IDENTIFIER)
      {
	std::ostringstream msg;
	msg << "Poorly named class <" << m_state.getTokenDataAsString(&iTok).c_str() << ">: Identifier must begin with an upper-case letter";;
	MSG(&iTok, msg.str().c_str(), ERR);
	return  true; //we're done unless we can gobble the rest up?
      }


    SymbolClass * cSym = NULL;
    if(!m_state.alreadyDefinedSymbolClass(iTok.m_dataindex, cSym))
      {
	m_state.addIncompleteClassSymbolToProgramTable(iTok.m_dataindex, cSym);
      }
    else
      {
	//if already defined, then must be incomplete, else duplicate!!
	if(cSym->getUlamClass() != UC_INCOMPLETE)
	  {
	    std::ostringstream msg;
	    msg << "Duplicate or incomplete class <" << m_state.m_pool.getDataAsString(cSym->getId()).c_str() << ">";
	    MSG(&iTok, msg.str().c_str(), ERR);

	    return  true;  //we're done unless we can gobble the rest up?
	  }
      }


    //set class type in UlamType (through its class symbol) since we know it
    switch(pTok.m_type)
      {
      case TOK_KW_ELEMENT:
	{
	  cSym->setUlamClass(UC_ELEMENT);
	  break;
	}
      case TOK_KW_QUARK:
	{
	  cSym->setUlamClass(UC_QUARK);
	  break;
	}
      case TOK_KW_QUARKUNION:
	{
	  cSym->setUlamClass(UC_QUARK);
	  cSym->setQuarkUnion();
	  break;
	}
      default:
	assert(0);
      };


    NodeBlockClass * classNode = parseClassBlock(cSym->getUlamTypeIdx()); //we know its type..sweeter?

    if(classNode)
      {
	cSym->setClassBlockNode(classNode);
      }
    else
      {
	// reset to incomplete
	cSym->setUlamClass(UC_INCOMPLETE);
	std::ostringstream msg;
	msg << "Empty/Incomplete Class Definition: <" << m_state.getTokenDataAsString(&iTok).c_str() << ">; possible missing ending curly brace";
	MSG(&pTok, msg.str().c_str(), WARN);
      }

    //return true when we've seen THIS class
    return (iTok.m_dataindex == m_state.m_compileThisId); //parseThisClass
  }


  NodeBlockClass * Parser::parseClassBlock(UTI utype)
  {
    Token pTok;
    NodeBlockClass * rtnNode = NULL;

    if(!getExpectedToken(TOK_OPEN_CURLY, pTok))
      return NULL;


    if(getExpectedToken(TOK_CLOSE_CURLY))
      {
	rtnNode = new NodeBlockClassEmpty(m_state.m_currentBlock, m_state);
	rtnNode->setNodeLocation(pTok.m_locator);
	rtnNode->setNodeType(utype);

	m_state.m_classBlock = rtnNode;    //2 ST:functions and data member decls, separate

	return rtnNode;  //allow empty class
      }

    NodeBlock * prevBlock = m_state.m_currentBlock;
    assert(prevBlock == NULL); //this is the class' first block

    rtnNode = new NodeBlockClass(prevBlock, m_state);
    rtnNode->setNodeLocation(pTok.m_locator);
    rtnNode->setNodeType(utype);

    // current, this block's symbol table added to parse tree stack
    //          for validating and finding scope of program/block variables

    m_state.m_currentBlock = rtnNode;
    m_state.m_classBlock = rtnNode;    //2 ST:functions and data member decls, separate

    //keep the data member var decls, starting with NodeBlockClass, keeping it for return
    NodeStatements * nextNode = rtnNode;
    NodeStatements * stmtNode = NULL;

    //initPrimitiveUlamTypes();

    while( parseDataMember(stmtNode))   //could be false, in case of function def
      {
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
    m_state.m_currentBlock = prevBlock;

    return rtnNode;  //parseBlockClass
  }


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
		getTokensUntil(TOK_SEMICOLON);  //does this help?
	      }
	    else
	      {
		brtn = true;
		nextNode = new NodeStatements(rtnNode, m_state);
		nextNode->setNodeLocation(rtnNode->getNodeLocation());
	      }
	  }
	  return brtn;
      } //typedef done.

    m_state.m_parsingElementParameterVariable = false;
    //static element parameter
    if(pTok.m_type == TOK_KW_ELEMENT)
      {
	//currently only permitted in elements, no quarks
	UTI cuti = m_state.m_classBlock->getNodeType();
	ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(cuti)->getUlamClass();
	if(classtype != UC_ELEMENT)
	  {
	    std::ostringstream msg;
	    msg << "Only elements may have element parameters: <" << m_state.m_pool.getDataAsString(m_state.m_compileThisId).c_str() << "> is a quark";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	m_state.m_parsingElementParameterVariable = true;
	getNextToken(pTok);
      }

    if(!Token::isTokenAType(pTok))
      {
	unreadToken();
	m_state.m_parsingElementParameterVariable = false;  //clear on error
	return false;
      }

    // check for Type bitsize specifier;
    u32 typebitsize = 0;
    s32 arraysize = NONARRAYSIZE;
    parseTypeBitsize(pTok, typebitsize, arraysize);

    getNextToken(iTok);
    //if(getExpectedToken(TOK_IDENTIFIER, iTok))
    if(iTok.m_type == TOK_IDENTIFIER)
      {
	//need another token to distinguish a function from a variable declaration (do so quietly)
	if(getExpectedToken(TOK_OPEN_PAREN, QUIETLY))
	  {
	    //eats the '(' when found
	    rtnNode = makeFunctionSymbol(pTok, typebitsize, iTok); //with params
	    Token qTok;
	    getNextToken(qTok);

	    if((qTok.m_type != TOK_CLOSE_CURLY) && (((NodeBlockFunctionDefinition *) rtnNode)->isNative() && qTok.m_type != TOK_SEMICOLON))
	      {
		//first remove the pointer to this node from its symbol
		if(rtnNode)
		  ((NodeBlockFunctionDefinition *) rtnNode)->getFuncSymbolPtr()->setFunctionNode((NodeBlockFunctionDefinition *) NULL); //deletes node
		rtnNode = NULL;
		MSG(&pTok, "INCOMPLETE Function Definition", ERR);
	      }
	    else
	      {
		if(rtnNode)
		  brtn = true;  //rtnNode belongs to the symbolFunction
	      }
	  }
	else
	  {
	    // not '(', so token is unread, and we know
	    // it's a variable, not a function;
	    // also handles arrays
	    rtnNode = makeVariableSymbol(pTok, typebitsize, arraysize, iTok);

	    if(rtnNode)
	      rtnNode = parseRestOfDecls(pTok, typebitsize, arraysize, iTok, rtnNode, NOASSIGN);

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
		    nextNode->setNodeLocation(rtnNode->getNodeLocation());
		  }
	      }
	  }
      }
    else
      {
	//user error!
	std::ostringstream msg;
	msg << "Name of variable/function <" << m_state.getTokenDataAsString(&iTok).c_str() << ">: Identifier must begin with a lower-case letter";
	MSG(&iTok, msg.str().c_str(), ERR);

	unreadToken();
      }

    m_state.m_parsingElementParameterVariable = false;
    return brtn;  //parseDataMember
  }


  Node * Parser::parseBlock()
  {
    Token pTok;
    NodeBlock * rtnNode = NULL;

    if(!getExpectedToken(TOK_OPEN_CURLY, pTok))
      return NULL;


    if(getExpectedToken(TOK_CLOSE_CURLY, QUIETLY))
      {
	rtnNode = new NodeBlockEmpty(m_state.m_currentBlock, m_state); // legal
	rtnNode->setNodeLocation(pTok.m_locator);
	return rtnNode;
      }

    rtnNode = new NodeBlock(m_state.m_currentBlock, m_state);
    rtnNode->setNodeLocation(pTok.m_locator);

    // current, this block's symbol table added to parse tree stack
    //          for validating and finding scope of program/block variables
    NodeBlock * prevBlock = m_state.m_currentBlock;
    m_state.m_currentBlock = rtnNode;

    NodeStatements * nextNode = (NodeStatements *) parseStatements();

    if(nextNode)  //could be Null, in case of errors
      rtnNode->setNextNode(nextNode);
    else
      {
	//replace NodeBlock with a NodeBlockEmpty
	delete rtnNode;
	rtnNode = NULL;

	rtnNode = new NodeBlockEmpty(prevBlock, m_state);  	// legal
	rtnNode->setNodeLocation(pTok.m_locator);

	m_state.m_currentBlock = rtnNode; //very temporary
      }

    if(!getExpectedToken(TOK_CLOSE_CURLY))
      {
	delete rtnNode;
	rtnNode = NULL;
	getTokensUntil(TOK_CLOSE_CURLY);
	m_state.m_currentBlock = NULL;   //very temporary
      }

    //this block's ST is no longer in scope
    if(m_state.m_currentBlock)
      m_state.m_currentFunctionBlockDeclSize -= m_state.m_currentBlock->getSizeOfSymbolsInTable();

    m_state.m_currentBlock = prevBlock;

    //sanity check
    assert(!rtnNode || rtnNode->getPreviousBlockPointer() == prevBlock);

    return rtnNode;  //parseBlock
  }  //parseBlock


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
	    return parseStatements();  //try again
	  }
	else
	  {
	    unreadToken();
	    return NULL;
	  }
      }

    NodeStatements * rtnNode = new NodeStatements(sNode, m_state);
    rtnNode->setNodeLocation(sNode->getNodeLocation());

    if(!getExpectedToken(TOK_CLOSE_CURLY, QUIETLY))
      {
	rtnNode->setNextNode((NodeStatements *) parseStatements()); // more statements
      }
    else
      unreadToken();

    return rtnNode; //parseStatements
  }


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
	rtnNode = parseSimpleStatement(); // may be null (only ;)
      }

    return rtnNode;   //parseStatement
  }


  Node * Parser::parseControlStatement()
  {
    Node * rtnNode = NULL;
    Token pTok;

    getNextToken(pTok);

    // should have already known to be true
    assert(Token::getSpecialTokenWork(pTok.m_type) == TOKSP_CTLKEYWORD);

    switch(pTok.m_type)
      {
      case TOK_KW_IF:
	rtnNode = parseControlIf(pTok);
	break;
      case TOK_KW_WHILE:
	{
	  m_state.m_parsingControlLoop = m_state.getNextTmpVarNumber();  //true
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
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  //eat error token
	}
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
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
      return NULL;  //stop this maddness

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
	return NULL;  //stop this maddness
      }

    // wrapping body in NodeStatements produces proper comment for genCode
    NodeStatements * trueStmtNode = new NodeStatements(trueNode, m_state);
    trueStmtNode->setNodeLocation(trueNode->getNodeLocation());

    Node * falseStmtNode = NULL;

    if(getExpectedToken(TOK_KW_ELSE, QUIETLY))
      {
	Node * falseNode = parseStatement();
	if(falseNode != NULL)
	  {
	    falseStmtNode = new NodeStatements(falseNode, m_state);
	    falseStmtNode->setNodeLocation(falseNode->getNodeLocation());
	  }
      }

    Node * rtnNode = new NodeControlIf(condNode, trueStmtNode, falseStmtNode, m_state);
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
      return NULL;  //stop this maddness

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
	return NULL;  //stop this maddness
      }

    // wrapping body in NodeStatements produces proper comment for genCode
    NodeStatements * trueStmtNode = new NodeStatements(trueNode, m_state);
    trueStmtNode->setNodeLocation(trueNode->getNodeLocation());

    // end of while loop label, linked to end of body (true statement)
    Node * labelNode = new NodeLabel(controlLoopLabelNum, m_state);
    labelNode->setNodeLocation(wTok.m_locator);

    NodeStatements * labelStmtNode = new NodeStatements(labelNode, m_state);
    labelStmtNode->setNodeLocation(wTok.m_locator);
    trueStmtNode->setNextNode(labelStmtNode);

    Node * rtnNode = new NodeControlWhile(condNode, trueStmtNode, m_state);
    rtnNode->setNodeLocation(wTok.m_locator);

    return rtnNode;
  } //parseControlWhile


  Node * Parser::parseControlFor(Token fTok)
  {
    if(!getExpectedToken(TOK_OPEN_PAREN))
      {
	return NULL;
      }

    s32 controlLoopLabelNum = m_state.m_parsingControlLoop; //save at the top

    //before parsing the for statement, need a new scope
    NodeBlock * rtnNode = new NodeBlock(m_state.m_currentBlock, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(fTok.m_locator);

    // current, this block's symbol table added to parse tree stack
    //          for validating and finding scope of program/block variables
    NodeBlock * prevBlock = m_state.m_currentBlock;
    m_state.m_currentBlock = rtnNode;


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
	delete declNode;  //stop this maddness
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
	    return NULL;  //stop this maddness
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
	condNode->setNodeLocation(qTok.m_locator);
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
	    return NULL;  //stop this maddness
	  }

	if(!getExpectedToken(TOK_CLOSE_PAREN))
	  {
	    delete rtnNode;
	    delete declNode;
	    delete condNode;
	    delete assignNode;
	    return NULL;  //stop this maddness
	  }
      } //done with assign expr, could be null

    Node * trueNode = NULL;
    if(m_state.m_parsingConditionalAs)
      {
	trueNode = setupAsConditionalBlockAndParseStatements((NodeConditionalAs *) condNode);
      }
    else
      {
	trueNode = parseStatement();  //even an empty statement has a node!
      }

    if(!trueNode)
      {
	delete rtnNode;
	delete declNode;
	delete condNode;
	delete assignNode;
	return NULL;  //stop this maddness
      }

    // link the pieces together..

    NodeStatements * nextNode = NULL;
    if(declNode)
      {
	nextNode = new NodeStatements(declNode, m_state);
	assert(nextNode);
	nextNode->setNodeLocation(declNode->getNodeLocation());
	rtnNode->setNextNode(nextNode); //***link decl as first in block
      }

    // wrapping body in NodeStatements produces proper comment for genCode
    // might need another block here ???
    NodeStatements * trueStmtNode = new NodeStatements(trueNode, m_state);
    assert(trueStmtNode);
    trueStmtNode->setNodeLocation(trueNode->getNodeLocation());

    // end of while loop label, linked to end of body, before assign statement
    Node * labelNode = new NodeLabel(controlLoopLabelNum, m_state);
    labelNode->setNodeLocation(rTok.m_locator);

    NodeStatements * labelStmtNode = new NodeStatements(labelNode, m_state);
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
      rtnNode->setNextNode(nextControlNode);  //***link while to rtn block (no decl)

    //this block's ST is no longer in scope
    if(m_state.m_currentBlock)
      m_state.m_currentFunctionBlockDeclSize -= m_state.m_currentBlock->getSizeOfSymbolsInTable();

    m_state.m_currentBlock = prevBlock;

    return rtnNode;
  } //parseControlFor


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

    // if empty still make auto
    NodeBlock * blockNode = new NodeBlock(m_state.m_currentBlock, m_state);
    blockNode->setNodeLocation(asNode->getNodeLocation());

    // current, this block's symbol table added to parse tree stack
    //          for validating and finding scope of program/block variables
    NodeBlock * prevBlock = m_state.m_currentBlock;
    m_state.m_currentBlock = blockNode;

    // after the new block is setup: install the auto symbol into ST, and
    // make its auto local variable to shadow the lhs of 'as' as rhs type
    NodeTerminalIdent * tmpnti = new NodeTerminalIdent(m_state.m_identTokenForConditionalAs, NULL, m_state);
    assert(tmpnti);

    Token typeTok = asNode->getTypeToken();
    UTI tuti = m_state.getUlamTypeFromToken(typeTok, 0, NONARRAYSIZE);
    UlamType * tut = m_state.getUlamTypeByIndex(tuti);
    Symbol * asymptr = NULL;  //a place to put the new symbol
    tmpnti->installSymbolVariable(typeTok, tut->getBitSize(), tut->getArraySize(), asymptr);
    assert(asymptr);
    asymptr->setAutoLocal();  //set auto flag

    delete tmpnti;            //done with nti
    tmpnti = NULL;
    m_state.m_parsingConditionalAs = false;  //done with flag and identToken.

    //insert var decl into NodeStatements..as if parseStatement was called..
    Node * varNode = new NodeVarDecl((SymbolVariable*) asymptr, m_state);
    varNode->setNodeLocation(asNode->getNodeLocation());

    NodeStatements * stmtsNode = new NodeStatements(varNode, m_state);
    stmtsNode->setNodeLocation(varNode->getNodeLocation());

    blockNode->setNextNode(stmtsNode);

    if(!singleStmtExpected)
      {
	if(!getExpectedToken(TOK_CLOSE_CURLY, QUIETLY))
	  {
	    stmtsNode->setNextNode((NodeStatements *) parseStatements()); // more statements
	    getExpectedToken(TOK_CLOSE_CURLY);
	  }
	//else
	//  unreadToken();
      }
    else
      {
	Node * sNode = parseStatement(); //get one statement only
	NodeStatements * nextNode = new NodeStatements(sNode, m_state);
	nextNode->setNodeLocation(sNode->getNodeLocation());
	stmtsNode->setNextNode(nextNode);
      }

    //this block's ST is no longer in scope
    if(m_state.m_currentBlock)
      m_state.m_currentFunctionBlockDeclSize -= m_state.m_currentBlock->getSizeOfSymbolsInTable();

    m_state.m_currentBlock = prevBlock;

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
	rtnNode = new NodeStatementEmpty(m_state);  	// empty statement
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
    else if(pTok.m_type == TOK_KW_RETURN)
      {
	unreadToken();               // needs location
	rtnNode = parseReturn();
      }
    else if(pTok.m_type == TOK_KW_BREAK)
      {
	if(m_state.m_parsingControlLoop)
	  {
	    rtnNode = new NodeBreakStatement(m_state);
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
	    rtnNode->setNodeLocation(pTok.m_locator);
	  }
	else
	  {
	    MSG(&pTok,"Continue statement not within loop" , ERR);
	  }
      }
    else if(pTok.m_type == TOK_ERROR_CONT)
      {
	MSG(&pTok, "Unexpected input!! ERROR Token, Continue",ERR);
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
    return rtnNode;   //parseSimpleStatement
  } //parseSimpleStatement


  // Typedefs are not transferred to generated code;
  // they are a short-hand for ulamtypes (e.g. arrays)
  // that may be used as function return types; scope-specific.
  Node * Parser::parseTypedef()
  {
    Node * rtnNode = NULL;
    Token pTok;
    getNextToken(pTok);

    if(Token::isTokenAType(pTok))
      {
	// check for Type bitsize specifier;
	u32 typebitsize = 0;
	s32 arraysize = NONARRAYSIZE;
	parseTypeBitsize(pTok, typebitsize, arraysize);

	Token iTok;
	getNextToken(iTok);
	//insure the typedef name starts with a capital letter
	//if(iTok.m_type == TOK_IDENTIFIER && Token::isTokenAType(iTok,&m_state))
	if(iTok.m_type == TOK_TYPE_IDENTIFIER)
	  {
	    rtnNode = makeTypedefSymbol(pTok, typebitsize, arraysize, iTok);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Invalid typedef Alias <" << m_state.getTokenDataAsString(&iTok).c_str() << ">, Type Identifier (2nd arg) requires capitalization";
	    MSG(&iTok, msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Invalid typedef Base Type <" << m_state.getTokenDataAsString(&pTok).c_str() << ">";
	MSG(&pTok, msg.str().c_str(), ERR);
      }
    return rtnNode;
  } //parseTypedef


  // used for data members or local function variables; or
  // 'singledecl' function parameters; no longer for function defs.
  Node * Parser::parseDecl(bool parseSingleDecl)
  {
    Node * rtnNode = NULL;
    Token pTok, iTok;

    getNextToken(pTok);

    // should have already known to be true
    assert(Token::isTokenAType(pTok));

    // check for Type bitsize specifier;
    u32 typebitsize = 0;
    s32 arraysize = NONARRAYSIZE;
    parseTypeBitsize(pTok, typebitsize, arraysize);

    getNextToken(iTok);
    if(iTok.m_type == TOK_IDENTIFIER)
      {
	rtnNode = makeVariableSymbol(pTok, typebitsize, arraysize, iTok);

	if(rtnNode && !parseSingleDecl)
	  {
	    // for multi's of same type, and/or its assignment
	    return parseRestOfDecls(pTok, typebitsize, arraysize, iTok, rtnNode);
	  }
      }
    else
      {
	//user error!
	std::ostringstream msg;
	msg << "Name of variable <" << m_state.getTokenDataAsString(&iTok).c_str() << ">: Identifier must begin with a lower-case letter";
	MSG(&iTok, msg.str().c_str(), ERR);

	unreadToken();
      }
    return rtnNode;  //parseDecl
  }


  void Parser::parseTypeBitsize(Token & typeTok, u32& typebitsize, s32& arraysize)
  {
    //u32 typebitsize = 0;

    Token bTok;
    getNextToken(bTok);

    if(bTok.m_type == TOK_OPEN_PAREN)
      {
	Node * bitsizeNode = parseExpression();
	if(!bitsizeNode)
	  {
	    std::ostringstream msg;
	    msg << "Bitsize of Open Paren is missing, type " << bTok.getTokenString() ;
	    MSG(&bTok, msg.str().c_str(), ERR);
	  }
	else
	  {
	    bitsizeNode = new NodeTypeBitsize(bitsizeNode, m_state);
	    bitsizeNode->setNodeLocation(typeTok.m_locator);

	    // eval what we need, and delete the node
	    ((NodeTypeBitsize *) bitsizeNode)->getTypeBitSizeInParen(typebitsize, m_state.getBaseTypeFromToken(typeTok));

	    delete bitsizeNode;   //done with it!
	    bitsizeNode = NULL;
	  }

	if(!getExpectedToken(TOK_CLOSE_PAREN))
	  {
	    typebitsize = 0;   //?
	  }
      }
    else if(bTok.m_type == TOK_DOT)
      {
	unreadToken();
	parseTypeFromAnotherClassesTypedef(typeTok, typebitsize, arraysize);
      }
    else
      {
	unreadToken(); //not open paren, bitsize is unspecified
      }
    return; //typebitsize;
  } //parseTypeBitsize


  //recursively parses classtypes and their typedefs (dot separated)
  // into their UTI alias; the typeTok and return bitsize ref args
  // represent the UTI; Admittedly, not the most elegant approach, but
  // it works with the existing code that installs symbol variables
  // using the type token, bitsize, and arraysize.
  void Parser::parseTypeFromAnotherClassesTypedef(Token & typeTok, u32& rtnbitsize, s32& rtnarraysize)
  {
    SymbolClass * csym = NULL;

    Token nTok;
    getNextToken(nTok);
    if(nTok.m_type != TOK_DOT)
      {
	unreadToken();
	return;  //done.
      }

    if(m_state.alreadyDefinedSymbolClass(typeTok.m_dataindex, csym))
      {
	bool saveUseMemberBlock = m_state.m_useMemberBlock;
	NodeBlockClass * saveMemberClassBlock = m_state.m_currentMemberClassBlock;
	NodeBlockClass * memberClassNode = csym->getClassBlockNode();
	assert(memberClassNode);  // e.g. forgot the closing brace on quark def once
	if(!memberClassNode)
	  {
	    std::ostringstream msg;
	    msg << "Trying to use typedef from another class <" << m_state.m_pool.getDataAsString(csym->getId()).c_str() << ">, before it has been defined. Cannot continue";
	    MSG(&typeTok, msg.str().c_str(),ERR);
	    return;
	  }

	//set up compiler state to use the member class block for symbol searches
	m_state.m_currentMemberClassBlock = memberClassNode;
	m_state.m_useMemberBlock = true;

	//after the dot
	getNextToken(typeTok);
	if(Token::isTokenAType(typeTok))
	  {
	    UTI tduti;
	    if(m_state.getUlamTypeByTypedefName(typeTok.m_dataindex, tduti))
	      {
		UlamType * tdut = m_state.getUlamTypeByIndex(tduti);
		ULAMCLASSTYPE tdclasstype = tdut->getUlamClass();
		const std::string tdname = tdut->getUlamTypeNameOnly(&m_state);

		if(tdclasstype == UC_NOTACLASS)
		  typeTok.init(Token::getTokenTypeFromString(tdname.c_str()), typeTok.m_locator, 0);
		else
		  typeTok.init(TOK_TYPE_IDENTIFIER, typeTok.m_locator, m_state.m_pool.getIndexForDataString(tdname));

		rtnbitsize = tdut->getBitSize();
		rtnarraysize = tdut->getArraySize();
	      }
	    //else
	      {
		//not a typedef, possibly its another class? go again..
		parseTypeFromAnotherClassesTypedef(typeTok, rtnbitsize, rtnarraysize);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Unexpected input!! Token: <" << typeTok.getTokenEnumName() << "> is not a type";
	    MSG(&typeTok, msg.str().c_str(),ERR);
	  }

	m_state.m_useMemberBlock = saveUseMemberBlock; //restore
	m_state.m_currentMemberClassBlock = saveMemberClassBlock;
      }
    else
      {
	std::ostringstream msg;
	msg << "Unexpected input!! Token: <" << typeTok.getTokenEnumName() << "> is not a class type: <" << m_state.getTokenDataAsString(&typeTok).c_str() << ">";
	MSG(&typeTok, msg.str().c_str(),ERR);
	//not a class!
      }
    return;
  } //parseTypeFromAnotherClassesTypedef


  Node * Parser::parseReturn()
  {
    Token pTok;
    getNextToken(pTok);

    Node * rtnNode = NULL;
    Node * rtnExprNode = parseAssignExpr(); // may be NULL
    if(!rtnExprNode)
      {
	rtnExprNode = new NodeStatementEmpty(m_state); //has Nav type
	rtnExprNode->setNodeLocation(pTok.m_locator);
      }

    rtnNode =  new NodeReturnStatement(rtnExprNode, m_state);
    rtnNode->setNodeLocation(pTok.m_locator);
    return rtnNode;
  } //parseReturn


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
	switch(cTok.m_type)
	  {
	  case TOK_KW_AS:
	    m_state.saveIdentTokenForConditionalAs(iTok); //sets m_state.m_parsingConditionalAs
	    unreadToken();
	    rtnNode = makeConditionalExprNode(rtnNode);  //done, could be NULL
	    break;
	  default:
	    unreadToken();
	  };
      }
    else
      return parseExpression();   //perhaps a number, true or false, cast..

    if(m_state.m_parsingConditionalAs)
      return rtnNode;

    // if nothing else follows, parseRestOfAssignExpr returns its argument
    return parseRestOfAssignExpr(rtnNode);  //parseAssignExpr
  } //parseConditionalExpr


  Node * Parser::parseAssignExpr()
  {
    Node * rtnNode = NULL;
    Token iTok;
    if(getExpectedToken(TOK_IDENTIFIER, iTok, QUIETLY))
      {
	// though function calls are not proper lhs values in assign
	// expression; they are parsed here (due to the two token look
	// ahead, which drops the Identifier Token before parseExpression) and is
	// caught during checkAndLabelType as NOT storeIntoAble.
	if(!(rtnNode = parseIdentExpr(iTok)))
	  return parseExpression();
      }
    else
      return parseExpression();   //perhaps a number, true or false, cast..

    // if nothing else follows, parseRestOfAssignExpr returns its argument
    return parseRestOfAssignExpr(rtnNode);  //parseAssignExpr
  } //parseAssignExpr


  Node * Parser::parseLvalExpr(Token identTok)
  {
    Token pTok;
    getNextToken(pTok);
    // function calls or func defs are not valid
    if(pTok.m_type == TOK_OPEN_PAREN)
      {
	std::ostringstream msg;
	msg << "Unexpected token <" << pTok.getTokenEnumName() << "> indicates a function call or definition; neither are valid here";
	MSG(&pTok, msg.str().c_str(), ERR);

	unreadToken();
	return NULL;
      }

    unreadToken();  //put whatever back

    Symbol * asymptr = NULL;

    //may continue when symbol not defined yet (e.g. Decl)
    m_state.alreadyDefinedSymbol(identTok.m_dataindex,asymptr);

    // make a variable;  symbol could be Null!
    Node * rtnNode = new NodeTerminalIdent(identTok, (SymbolVariable *) asymptr, m_state);
    assert(rtnNode);
    rtnNode->setNodeLocation(identTok.m_locator);

    return parseRestOfLvalExpr(rtnNode);  //in case of arrays
  } //parseLvalExpr


  // lvalExpr + func Calls + member select (dot)
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
	    msg << "Undefined function <" << m_state.getTokenDataAsString(&identTok).c_str() << "> that has already been declared as a variable";
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
	// else we have a variable, not a function call, nor member_select
	unreadToken();
	rtnNode = parseLvalExpr(identTok);
      }

    // bail if no node
    if(!rtnNode)
      return NULL;

    // check for member select expression
    Token qTok;
    getNextToken(qTok);

    if(qTok.m_type == TOK_DOT)
      {
	unreadToken();
	rtnNode = parseRestOfMemberSelectExpr(rtnNode);
      }
    else
      {
	unreadToken();  // not a member select
      }
    return rtnNode;  //parseIdentExpr
  }


  Node * Parser::parseMemberSelectExpr(Token memberTok)
  {
    // arg is an instance of a class, it will be/was
    // declared as a variable, either as a data member or locally,
    // WAIT To  search back through the block symbol tables during type labeling

    Node * classInstanceNode = new NodeTerminalIdent(memberTok, (SymbolVariable *) NULL, m_state);
    classInstanceNode->setNodeLocation(memberTok.m_locator);

    return parseRestOfMemberSelectExpr(classInstanceNode); //parseMemberSelect
  }


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
	// set up compiler state to NOT use the current class block
	// for symbol searches; may be unknown until type label
	m_state.m_currentMemberClassBlock = NULL;
	m_state.m_useMemberBlock = true;  //oddly =true

	rtnNode = new NodeMemberSelect(classInstanceNode, parseIdentExpr(iTok), m_state);
	rtnNode->setNodeLocation(iTok.m_locator);

	//clear up compiler state to no longer use the member class block for symbol searches
	m_state.m_useMemberBlock = false;
	m_state.m_currentMemberClassBlock = NULL;
      }
    //return rtnNode;
    return parseRestOfMemberSelectExpr(rtnNode); //recurse
  } //parseRestOfMemberSelectExpr


  Node * Parser::parseFunctionCall(Token identTok)
  {
    Symbol * asymptr = NULL;

    // cannot call a function if a local variable name shadows it
    if(m_state.m_currentBlock->isIdInScope(identTok.m_dataindex,asymptr))
      {
	std::ostringstream msg;
	msg << "'" << m_state.m_pool.getDataAsString(asymptr->getId()).c_str() << "' cannot be used as a function, already declared as a variable '" << m_state.getUlamTypeNameBriefByIndex(asymptr->getUlamTypeIdx()).c_str() << " " << m_state.m_pool.getDataAsString(asymptr->getId()) << "'";
	MSG(&identTok, msg.str().c_str(), ERR);
	return NULL;
      }

    //fill in func symbol during type labeling; supports function overloading
    NodeFunctionCall * rtnNode = new NodeFunctionCall(identTok, NULL, m_state);
    rtnNode->setNodeLocation(identTok.m_locator);

    //member selection doesn't apply to arguments (during parsing too)
    bool saveUseMemberBlock = m_state.m_useMemberBlock;
    NodeBlockClass * saveMemberClassBlock = m_state.m_currentMemberClassBlock;
    m_state.m_useMemberBlock = false;

    if(!parseRestOfFunctionCallArguments(rtnNode))
      {
	delete rtnNode;

	m_state.m_useMemberBlock = saveUseMemberBlock; //doesn't apply to arguments; restore
	m_state.m_currentMemberClassBlock = saveMemberClassBlock;

	return NULL;
      }

    m_state.m_useMemberBlock = saveUseMemberBlock; //doesn't apply to arguments; restore
    m_state.m_currentMemberClassBlock = saveMemberClassBlock;

    // can't do any checking since function may not have been seen yet
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
    msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
    MSG(&pTok, msg.str().c_str(),ERR);
    return false;
  }


  Node * Parser::parseExpression()
  {
    Node * rtnNode = parseLogicalExpression();

    if(!rtnNode)
      return NULL;  //stop this maddness

    // if not addop, parseRestOfExpression returns its arg
    return parseRestOfExpression(rtnNode);  //parseExpression
  }


  Node * Parser::parseLogicalExpression()
  {
    Node * rtnNode = parseBitExpression();

    if(!rtnNode)
      return NULL;  //stop this maddness

    // if not bitop, parseRestOfLogicalExpression returns its arg
    return parseRestOfLogicalExpression(rtnNode);  //parseLogicalExpression
  }


  Node * Parser::parseBitExpression()
  {
    Node * rtnNode = parseEqExpression();

    if(!rtnNode)
      return NULL;  //stop this maddness

    // if not eqop, parseRestOfBitExpression returns its arg
    return parseRestOfBitExpression(rtnNode);  //parseExpression
  }


  Node * Parser::parseEqExpression()
  {
    Node * rtnNode = parseCompareExpression();

    if(!rtnNode)
      return NULL;  //stop this maddness

    // if not compop, parseRestOfEqExpression returns its arg
    return parseRestOfEqExpression(rtnNode);  //parseExpression
  }


  Node * Parser::parseCompareExpression()
  {
    Node * rtnNode = parseShiftExpression();

    if(!rtnNode)
      return NULL;  //stop this maddness

    // if not shiftop, parseRestOfCompareExpression returns its arg
    return parseRestOfCompareExpression(rtnNode);  //parseCompareExpression
  }


  Node * Parser::parseShiftExpression()
  {
    Node * rtnNode = parseTerm();

    if(!rtnNode)
      return NULL;  //stop this maddness

    // if not addop, parseRestOfShiftExpression returns its arg
    return parseRestOfShiftExpression(rtnNode);  //parseShiftExpression
  }


  Node * Parser::parseTerm()
  {
    Node * rtnNode = parseFactor();

    if(!rtnNode)
      return NULL;  //stop this maddness!

    // if not mulop, parseRestOfTerm returns rtnNode
    return parseRestOfTerm(rtnNode); //parseTerm
  }


  Node * Parser::parseFactor()
  {
    Node * rtnNode = NULL;
    Token pTok;

    getNextToken(pTok);

    switch(pTok.m_type)
      {
      case TOK_IDENTIFIER:
	{
	  rtnNode = parseIdentExpr(pTok);
	  // test ahead for UNOP_EXPRESSION so that any consecutive binary
	  // ops aren't misinterpreted as a unary operator (e.g. +,-).
	  Token tTok;
	  getNextToken(tTok);
	  unreadToken();
	  if(tTok.m_type == TOK_KW_IS || tTok.m_type == TOK_KW_HAS)
	    rtnNode = parseRestOfFactor(rtnNode);
	}
	break;
      case TOK_NUMBER:
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	rtnNode = new NodeTerminal(pTok, m_state);
	rtnNode->setNodeLocation(pTok.m_locator);
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
	rtnNode = parseRestOfFactor(NULL);  //parseUnop
	break;
      case TOK_EOF:
      case TOK_CLOSE_CURLY:
      case TOK_SEMICOLON:
      case TOK_CLOSE_PAREN:
      case TOK_COMMA:  // for functionall args
	unreadToken();
	break;
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
	  MSG(&pTok, msg.str().c_str(),ERR);
	  return parseFactor(); //redo
	}
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, unreading it";
	  MSG(&pTok, msg.str().c_str(), DEBUG);
	  unreadToken(); //eat the error token
	}
      };
    return rtnNode;  //parseFactor
  }


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
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
	  MSG(&pTok, msg.str().c_str(), ERR);
	  //eat token
	}
	break;
      case TOK_ERROR_ABORT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, exiting..";
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
    return rtnNode;  //parseRestOfFactor
  }


  Node * Parser::parseRestOfCastOrExpression()
  {
    Node * rtnNode = NULL;
    // just saw an open paren..
    //if next token is a type this a user cast, o.w. expression
    Token tTok;
    getNextToken(tTok);
    if(Token::isTokenAType(tTok))
      {
	rtnNode = makeCastNode(tTok);   //also parses its child Factor
      }
    else
      {
	unreadToken();
	//rtnNode = parseExpression();  //grammar says assign_expr
	rtnNode = parseAssignExpr();    //grammar says assign_expr
	if(!getExpectedToken(TOK_CLOSE_PAREN))
	  {
	    delete rtnNode;
	    rtnNode = NULL;
	  }
      }
    return rtnNode;
  }


  Node * Parser::makeCastNode(Token typeTok)
  {
    Node * rtnNode = NULL;

    // check for Type bitsize specifier;
    u32 typebitsize = 0;
    s32 arraysize = NONARRAYSIZE;
    parseTypeBitsize(typeTok, typebitsize, arraysize);

    // allows for casting to a class (makes class type if newly seen)
    UTI typeToBe = m_state.getUlamTypeFromToken(typeTok, typebitsize, arraysize);

    if(getExpectedToken(TOK_CLOSE_PAREN))
      {
	rtnNode = new NodeCast(parseFactor(), typeToBe, m_state);
	rtnNode->setNodeLocation(typeTok.m_locator);
	((NodeCast *) rtnNode)->setExplicitCast();
      }
    return rtnNode;
  } //makeCastNode


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
    return rtnNode;  //parseRestOfTerm
  }


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
	rtnNode = parseRestOfShiftExpression(rtnNode);  //recursion of left-associativity
	break;
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
	  MSG(&pTok, msg.str().c_str(),ERR);
	  rtnNode = parseRestOfShiftExpression(leftNode);  //redo
	}
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;  //parseRestOfShiftExpression
  }


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
	rtnNode = parseRestOfCompareExpression(rtnNode);  //recursion of left-associativity
	break;
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
	  MSG(&pTok, msg.str().c_str(),ERR);
	  rtnNode = parseRestOfCompareExpression(leftNode);  //redo
	}
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;  //parseRestOfCompareExpression
  }


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
	rtnNode = parseRestOfEqExpression(rtnNode);  //recursion of left-associativity
	break;
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
	  MSG(&pTok, msg.str().c_str(),ERR);
	  rtnNode = parseRestOfEqExpression(leftNode);  //redo
	}
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;  //parseRestOfEQExpression
  }


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
	rtnNode = parseRestOfBitExpression(rtnNode);  //recursion of left-associativity
	break;
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
	  MSG(&pTok, msg.str().c_str(),ERR);
	  rtnNode = parseRestOfBitExpression(leftNode);  //redo
	}
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;  //parseRestOfBitExpression
  }


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
	rtnNode = parseRestOfLogicalExpression(rtnNode);  //recursion of left-associativity
	break;
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
	  MSG(&pTok, msg.str().c_str(),ERR);
	  rtnNode = parseRestOfLogicalExpression(leftNode);  //redo
	}
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;  //parseRestOfLogicalExpression
  }


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
	rtnNode = parseRestOfExpression(rtnNode);  //recursion of left-associativity
	break;
      case TOK_AMP:
      case TOK_PIPE:
      case TOK_HAT:
	unreadToken();
	rtnNode = parseRestOfLogicalExpression(leftNode);  //addOp
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
	rtnNode = parseRestOfExpression(rtnNode);  //any more?
	break;
      case TOK_STAR:
      case TOK_SLASH:
      case TOK_PERCENTSIGN:
	unreadToken();
	rtnNode = parseRestOfTerm(leftNode);       //mulOp
	rtnNode = parseRestOfExpression(rtnNode);  //any more?
	break;
      case TOK_KW_IS:
      case TOK_KW_HAS:
	unreadToken();
	rtnNode = parseRestOfFactor(leftNode);
	rtnNode = parseRestOfExpression(rtnNode);  //any more?
	break;
      case TOK_ERROR_CONT:
	{
	  std::ostringstream msg;
	  msg << "Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">";
	  MSG(&pTok, msg.str().c_str(),ERR);
	  rtnNode = parseRestOfExpression(leftNode);  //redo
	}
	break;
      default:
	{
	  unreadToken();
	  rtnNode = leftNode;
	}
      };
    return rtnNode;  //parseRestOfExpression
  }


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
	rtnNode->setNodeLocation(pTok.m_locator);
      }

    if(!getExpectedToken(TOK_CLOSE_SQUARE))
      {
	delete rtnNode;
	rtnNode = NULL;
      }
    return rtnNode;  //parseRestOfLValExpr
  }

  //assignOK true by default.
  Node * Parser::parseRestOfDecls(Token typeTok, u32 typebitsize, s32 arraysize, Token identTok, Node * dNode, bool assignOK)
  {
    Token pTok;
    getNextToken(pTok);

    if(pTok.m_type == TOK_EQUAL)
      {
	if(assignOK)
	  {
	    unreadToken();
	    return parseRestOfDeclAssignment(typeTok, typebitsize, arraysize, identTok, dNode); //pass args for more decls
	  }
	else
	  {
	    // assignments not permitted
	    std::ostringstream msg;
	    msg << "Cannot assign to data member <" << m_state.getTokenDataAsString(&identTok).c_str() << "> at the time of its declaration";
	    MSG(&pTok, msg.str().c_str(), ERR);
	    getTokensUntil(TOK_SEMICOLON);  //rest of statement is ignored.
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
	// another decl of same type typeTok
	Node * sNode = makeVariableSymbol(typeTok, typebitsize, arraysize, iTok);  //a decl
	if (sNode)
	  {
	    //rtnNode =  new NodeVarDeclList(dNode, sNode, m_state) ;
	    rtnNode =  new NodeStatements(dNode, m_state);
	    rtnNode->setNodeLocation(dNode->getNodeLocation());

	    NodeStatements * nextNode = new NodeStatements(sNode, m_state);
	    nextNode->setNodeLocation(dNode->getNodeLocation());
	    ((NodeStatements *) rtnNode)->setNextNode(nextNode);
	  }
	//else  error ???
      }
    else
      {
	//perhaps read until semi-colon
	getTokensUntil(TOK_SEMICOLON);
	unreadToken();
      }
    return parseRestOfDecls(typeTok, typebitsize, arraysize, iTok, rtnNode, assignOK);  //iTok in case of =
  } //parseRestOfDecls


  Node * Parser::parseRestOfDeclAssignment(Token typeTok, u32 typebitsize, s32 arraysize, Token identTok, Node * dNode)
  {
    NodeStatements * rtnNode = new NodeStatements(dNode, m_state);
    rtnNode->setNodeLocation(dNode->getNodeLocation());

    // makeup node for lhs; using same symbol as dNode(could be Null!)
    Symbol * dsymptr = NULL;
    assert(m_state.alreadyDefinedSymbol(identTok.m_dataindex, dsymptr));
    Node * leftNode = new NodeTerminalIdent(identTok, (SymbolVariable *) dsymptr, m_state);
    assert(leftNode);
    leftNode->setNodeLocation(dNode->getNodeLocation());

    Node * assignNode = makeAssignExprNode(leftNode);
    assert(assignNode);

    NodeStatements * nextNode = new NodeStatements(assignNode, m_state);
    nextNode->setNodeLocation(assignNode->getNodeLocation());
    rtnNode->setNextNode(nextNode);

    return parseRestOfDecls(typeTok, typebitsize, arraysize, identTok, rtnNode);  //any more?
  } //parseRestOfDeclAssignment


  Node * Parser::makeVariableSymbol(Token typeTok, u32 typebitsize, s32 arraysize, Token identTok)
  {
    assert(! Token::isTokenAType(identTok));  //capitalization check done by Lexer

    NodeVarDecl * rtnNode = NULL;
    Node * lvalNode = parseLvalExpr(identTok); //for optional [] array size

    if(lvalNode)
      {
	// lvalNode could be either a NodeTerminalIdent or a NodeSquareBracket
	// process identifier...check if already defined in current scope; if not, add it;
	// returned symbol could be symbolVariable or symbolFunction, detect first.
	Symbol * asymptr = NULL;
	//s32 arraysize = NONARRAYSIZE;

	if(!lvalNode->installSymbolVariable(typeTok, typebitsize, arraysize, asymptr))
	  {
	    if(asymptr)
	      {
		std::ostringstream msg;
		msg << m_state.m_pool.getDataAsString(asymptr->getId()).c_str() << " has a previous declaration as '" << m_state.getUlamTypeNameBriefByIndex(asymptr->getUlamTypeIdx()).c_str() << " " << m_state.m_pool.getDataAsString(asymptr->getId()) << "'";
		MSG(&typeTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		// installSymbol failed for other reasons (e.g. problem with [])
		// rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid variable declaration of Type: <" << m_state.getTokenAsATypeName(typeTok).c_str() << "> and Name: <" << m_state.getTokenDataAsString(&identTok).c_str() << "> (missing symbol)";
		MSG(&typeTok, msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    rtnNode =  new NodeVarDecl((SymbolVariable *) asymptr, m_state);
	    rtnNode->setNodeLocation(typeTok.m_locator);
	  }
	delete lvalNode;  //done with it
      }
    return rtnNode;  //makeVariableSymbol
  }


  Node * Parser::makeFunctionSymbol(Token typeTok, u32 typebitsize, Token identTok)
  {
    // first check that the function name begins with a lower case letter
    if(Token::isTokenAType(identTok))
      {
	std::ostringstream msg;
	msg << "Function <" << m_state.getTokenDataAsString(&identTok).c_str() << "> is not a valid (lower case) name";
	MSG(&identTok, msg.str().c_str(), ERR);

	// eat tokens until end of definition ?
	return NULL;
      }

    Symbol * asymptr = NULL;
    // ask current scope class block if this identifier name is there (no embedded funcs)
    // (checks functions and variables and typedefs); if not a function, BAIL;
    // check for overloaded function, after parameter types available
    if(m_state.m_classBlock->isIdInScope(identTok.m_dataindex,asymptr) && !asymptr->isFunction())
      {
	std::ostringstream msg;
	msg << m_state.m_pool.getDataAsString(asymptr->getId()).c_str() << " cannot be used again as a function, it has a previous definition as '" << m_state.getUlamTypeNameBriefByIndex(asymptr->getUlamTypeIdx()).c_str() << " " << m_state.m_pool.getDataAsString(asymptr->getId()).c_str() << "'";
	MSG(&typeTok, msg.str().c_str(), ERR);

	// eat tokens until end of definition ???
	return NULL;
      }

    // not in scope, or not yet defined, or possible overloading
    // o.w. build symbol for function: return type + name + parameter symbols
    Node * rtnNode = makeFunctionBlock(typeTok, typebitsize, identTok);

    // exclude the declaration & definition from the parse tree
    // (since in SymbolFunction) & return NULL.

    return rtnNode;  //makeFunctionSymbol
  }


  NodeBlockFunctionDefinition * Parser::makeFunctionBlock(Token typeTok, u32 typebitsize, Token identTok)
  {
    NodeBlockFunctionDefinition * rtnNode = NULL;

    // all functions are defined in this "class" block;
    // or external 'use' for declarations.
    // this is a block with its own ST
    NodeBlock * prevBlock = m_state.m_currentBlock;  //restore before returning
    assert(prevBlock == m_state.m_classBlock);

    // o.w. build symbol for function: name, return type, plus arg symbols
    UTI uti = m_state.getUlamTypeFromToken(typeTok, typebitsize, NONARRAYSIZE);
    SymbolFunction * fsymptr = new SymbolFunction(identTok.m_dataindex, uti, m_state);

    // WAIT for the parameters, so we can add it to the SymbolFunctionName map..
    //m_state.m_classBlock->addFuncIdToScope(fsymptr->getId(), fsymptr);
    rtnNode =  new NodeBlockFunctionDefinition(fsymptr, prevBlock, m_state);
    rtnNode->setNodeLocation(typeTok.m_locator);

    // symbol will have pointer to body (or just decl for 'use');
    fsymptr->setFunctionNode(rtnNode); // tfr ownership

    // set class type to custom array; the current class block
    // node type was set to its class symbol type after checkAndLabelType
    if(m_state.getCustomArrayGetFunctionNameId() == identTok.m_dataindex)
      {
	UTI cuti = m_state.m_classBlock->getNodeType();  //prevBlock
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	((UlamTypeClass *) cut)->setCustomArrayType(uti);
      }

    m_state.m_currentBlock = rtnNode;  //before parsing the args

    // use space on funcCallStack for return statement.
    // negative for parameters; allot space at top for the return value
    // currently, only scalar; determines start position of first arg "under".
    s32 returnArraySize = m_state.slotsNeeded(fsymptr->getUlamTypeIdx());

    //extra one for "hidden" first arg, Ptr to its Atom
    m_state.m_currentFunctionBlockDeclSize = -(returnArraySize + 1);
    m_state.m_currentFunctionBlockMaxDepth = 0;

#if 1
    // create "self" symbol whose index is that of the "hidden" first arg (i.e. a Ptr to an Atom);
    // immediately below the return value(s); and belongs to the function definition scope.
    u32 selfid = m_state.m_pool.getIndexForDataString("self");
    UTI cuti = m_state.m_classBlock->getNodeType();  //luckily we know this now for each class used
    if(m_state.getUlamTypeByIndex(cuti)->getUlamClass() == UC_QUARK)
      cuti = UAtom;  //use atom for quark functions

    SymbolVariableStack * selfsym = new SymbolVariableStack(selfid, cuti, m_state.determinePackable(cuti), m_state.m_currentFunctionBlockDeclSize, m_state);
    selfsym->setIsSelf();
    m_state.addSymbolToCurrentScope(selfsym); //ownership goes to the block
#endif

    // parse and add parameters to function symbol
    parseRestOfFunctionParameters(fsymptr);

    // Now, look specifically for a function with the same given name defined
    Symbol * fnSym = NULL;
    if(!m_state.m_classBlock->isFuncIdInScope(identTok.m_dataindex, fnSym))
      {
	// first time name used as a function..add symbol function name/type
	fnSym = new SymbolFunctionName(identTok.m_dataindex, uti, m_state);

	// ownership goes to the class block's ST
	m_state.m_classBlock->addFuncIdToScope(fnSym->getId(), fnSym);
      }


    // verify return types agree (definitely when new name) --- o.w. error!
    if(fnSym->getUlamTypeIdx() != fsymptr->getUlamTypeIdx())
      {
	std::ostringstream msg;
	msg << "Return Type <"  << m_state.getUlamTypeNameByIndex(fsymptr->getUlamTypeIdx()).c_str() << "> does not agree with return type of already defined function '" << m_state.m_pool.getDataAsString(fnSym->getId()) << "' with the same name and return type <" << m_state.getUlamTypeNameByIndex(fnSym->getUlamTypeIdx()).c_str() << ">";
	MSG(&typeTok, msg.str().c_str(),ERR);
	delete fsymptr;
	rtnNode = NULL;
      }

    if(rtnNode)
      {
	bool isAdded = ((SymbolFunctionName *) fnSym)->overloadFunction(fsymptr); //transfers ownership, if added
	if(!isAdded)
	  {
	    //this is a duplicate function definition with same parameters and given name!!
	    std::ostringstream msg;
	    msg << "Duplicate defined function '" << m_state.m_pool.getDataAsString(fsymptr->getId()) << "' with the same parameters" ;
	    MSG(&typeTok, msg.str().c_str(),ERR);
	    delete fsymptr;         //also deletes the NodeBlockFunctionDefinition
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
		msg << "Variable args (...) supported for native functions only at this time; not  <" << m_state.m_pool.getDataAsString(fsymptr->getId()).c_str() << ">";
		MSG(rtnNode->getNodeLocationAsString().c_str(), msg.str().c_str(),ERR);
	      }
	  }
	else
	  {
	    fsymptr->setFunctionNode((NodeBlockFunctionDefinition *) NULL); //deletes node
	    rtnNode = NULL;
	  }
      }

    // this block's ST is no longer in scope
    m_state.m_currentBlock = prevBlock;
    m_state.m_currentFunctionBlockDeclSize = 0;  //default zero for datamembers
    m_state.m_currentFunctionBlockMaxDepth = 0;  //reset

    return rtnNode;  //makeFunctionBlock
  }


 void Parser::parseRestOfFunctionParameters(SymbolFunction * fsym)
  {
    Token pTok;
    getNextToken(pTok);
    if(pTok.m_type == TOK_CLOSE_PAREN)
      {
	return;  //done with parameters
      }

    assert(fsym);
    // allows function name to be same as arg name
    // since the function starts a new "block" (i.e. ST);
    // the argument to parseDecl will prevent it from looking
    // for restofdecls
	// currently only for natives (detected after args done)
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

	// could be null symbol already in scope
	if(argNode)
	  {
	    //parameter IS a variable (declaration).
	    if(argNode->getSymbolPtr(argSym))
	      fsym->addParameterSymbol(argSym); //ownership stays with NodeBlockFunctionDefinition's ST
	    else
	      MSG(&pTok, "No symbol from parameter declaration", ERR);
	  }

	delete argNode;    //no longer needed
	if(fsym->takesVariableArgs() && argSym)
	  {
	    std::ostringstream msg;
	    msg << "Parameter <" << m_state.m_pool.getDataAsString(argSym->getId()).c_str() << "> appears after ellipses (...)";
	    MSG(&pTok, msg.str().c_str(),ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Expected 'A Type' Token!! got Token: <" << pTok.getTokenEnumName() << "> instead";
	MSG(&pTok, msg.str().c_str(),ERR);
	//continue or short-circuit???
      }

    if(getExpectedToken(TOK_COMMA, QUIETLY)) // if so, get next parameter; o.w. unread
      {
	if(fsym->takesVariableArgs())
	  MSG(&pTok, "Variable args indication (...) appears at end of parameter list", ERR);
      }

    return parseRestOfFunctionParameters(fsym);
  } //parseRestOfFunctionParameters


  bool Parser::parseFunctionBody(NodeBlockFunctionDefinition *& funcNode)
  {
    bool brtn = false;

    // if next token is { this is a definition; o.w. a declaration, alone invalid
    // however, keyword 'native' is exception.

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
	    //MSG(&pTok, "Empty Function Definition", WARN);
	    nextNode = new NodeBlockEmpty(m_state.m_currentBlock, m_state); //legal
	    nextNode->setNodeLocation(pTok.m_locator);
	  }
	else
	  {
	    nextNode = (NodeStatements *) parseStatements();
	  }

	funcNode->setNextNode(nextNode);

	if(!getExpectedToken(TOK_CLOSE_CURLY))
	  {
	    MSG(&pTok, "Not close curly", ERR);
	    assert(0);  //???
	  }
	else
	  unreadToken();
      }
    else if(qTok.m_type == TOK_KW_NATIVE)
      {
	NodeStatements * nextNode;
	nextNode = new NodeBlockEmpty(m_state.m_currentBlock, m_state); //legal
	nextNode->setNodeLocation(qTok.m_locator);
	funcNode->setNextNode(nextNode);

	funcNode->setNative();
#if 0
	//output messes up test answer comparison
	std::ostringstream msg;
	msg << "Native Function Definition: <" << funcNode->getName() << ">";
	MSG(&qTok, msg.str().c_str(), INFO);
#endif
	brtn = true;
      }
    else
      {
	unreadToken();
	std::ostringstream msg;
	msg << "Unexpected input!! Token: <" << qTok.getTokenEnumName() << "> after function declaration.";
	MSG(&qTok, msg.str().c_str(),ERR);
      }
    return brtn;
  } //parseFunctionBody


  Node * Parser::makeTypedefSymbol(Token typeTok, u32 typebitsize, s32 arraysize, Token identTok)
  {
    NodeTypedef * rtnNode = NULL;
    Node * lvalNode = parseLvalExpr(identTok);

    if(lvalNode)
      {
	// lvalNode could be either a NodeTerminalIdent or a NodeSquareBracket
	// process identifier...check if already defined in current scope; if not, add it;
	// returned symbol could be symbolVariable or symbolFunction, detect first.
	Symbol * asymptr = NULL;
	//s32 arraysize = NONARRAYSIZE;
	UTI ut;
	if(m_state.getUlamTypeByTypedefName(typeTok.m_dataindex, ut))
	  {
	    arraysize = m_state.getArraySize(ut); //typedef built-in arraysize, no []
	    assert(typebitsize == 0);
	    typebitsize = m_state.getBitSize(ut);
	  }

	if(!lvalNode->installSymbolTypedef(typeTok, typebitsize, arraysize, asymptr))
	  {
	    if(asymptr)
	      {
		std::ostringstream msg;
		msg << m_state.m_pool.getDataAsString(asymptr->getId()).c_str() << " has a previous declaration as '" << m_state.getUlamTypeNameBriefByIndex(asymptr->getUlamTypeIdx()).c_str() << " " << m_state.m_pool.getDataAsString(asymptr->getId()) << "'";
		MSG(&typeTok, msg.str().c_str(), ERR);
	      }
	    else
	      {
		//installSymbol failed for other reasons (e.g. problem with []) , error already output.
		// rtnNode is NULL;
		std::ostringstream msg;
		msg << "Invalid typedef of Type: <" << m_state.getTokenAsATypeName(typeTok).c_str() << "> and Name: <" << m_state.getTokenDataAsString(&identTok).c_str() << "> (missing symbol)";
		MSG(&typeTok, msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    rtnNode =  new NodeTypedef((SymbolTypedef *) asymptr, m_state);
	    rtnNode->setNodeLocation(typeTok.m_locator);
	  }
	delete lvalNode;  //done with it
      }
    return rtnNode;  //makeTypedefSymbol
  }


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
	//default:
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
    return rtnNode;  //parseRestOfAssignExpr
  }


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
	msg << "RHS of operator <" << fTok.getTokenString() << "> is not a Type: " << tTok.getTokenString() << ", operation deleted";
	MSG(&tTok, msg.str().c_str(), ERR);
	delete leftNode;
	m_state.m_parsingConditionalAs = false;
	return NULL;
      }

    switch(fTok.m_type)
      {
      case TOK_KW_IS:
	rtnNode = new NodeConditionalIs(leftNode, tTok, m_state);
	break;
      case TOK_KW_HAS:
	rtnNode = new NodeConditionalHas(leftNode, tTok, m_state);
	break;
      case TOK_KW_AS:
	  rtnNode = new NodeConditionalAs(leftNode, tTok, m_state);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << " Unexpected input!! Token: <" << fTok.getTokenEnumName() << ">, aborting";
	  MSG(&fTok, msg.str().c_str(), DEBUG);
	  assert(0);
	}
	break;
      };

    assert(rtnNode);
    rtnNode->setNodeLocation(fTok.m_locator);
    return rtnNode;
  } //makeCondtionalExprNode


  NodeBinaryOpEqual * Parser::makeAssignExprNode(Node * leftNode)
  {
    NodeBinaryOpEqual * rtnNode = NULL;
    Token pTok;

    getNextToken(pTok);  //some flavor of equal token

    Node * rightNode = parseAssignExpr();
    if(!rightNode)
      {
	std::ostringstream msg;
	msg << "RHS of binary operator" << pTok.getTokenString() << " is missing, Assignment deleted";
	MSG(&pTok, msg.str().c_str(), DEBUG);
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
	      msg << " Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, aborting";
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
	      msg << " Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, aborting";
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
	      msg << " Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, aborting";
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
	      msg << " Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, aborting";
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
	      msg << " Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, aborting";
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
	      msg << " Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, aborting";
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
	      msg << " Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, aborting";
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
	      msg << " Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, aborting";
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
      }
    else
      {
	switch(pTok.m_type)
	  {
	  case TOK_MINUS:
	    rtnNode = new NodeUnaryOpMinus(factorNode, m_state);
	    break;
	  case TOK_PLUS:
	    rtnNode = new NodeUnaryOpPlus(factorNode, m_state);
	    break;
	  case TOK_BANG:
	    rtnNode = new NodeUnaryOpBang(factorNode, m_state);
	    break;
	  case TOK_PLUS_PLUS:
	    rtnNode = new NodeBinaryOpEqualArithAdd(factorNode, makeTerminalOne(pTok), m_state);
	    break;
	  case TOK_MINUS_MINUS:
	    rtnNode = new NodeBinaryOpEqualArithSubtract(factorNode, makeTerminalOne(pTok), m_state);
	    break;
	  default:
	    {
	      std::ostringstream msg;
	      msg << " Unexpected input!! Token: <" << pTok.getTokenEnumName() << ">, aborting";
	      MSG(&pTok, msg.str().c_str(), DEBUG);
	      assert(0);
	    }
	    break;
	  };
	assert(rtnNode);
	rtnNode->setNodeLocation(pTok.m_locator);
      }
    return rtnNode;
  } //makeFactorNode


  Node * Parser::makeTerminalOne(Token& locTok)
  {
	//make a '1' node
	Token oneTok;
	oneTok.init(TOK_NUMBER, locTok.m_locator, m_state.m_pool.getIndexForDataString("1"));
	Node * oneNode = new NodeTerminal(oneTok, m_state);
	oneNode->setNodeLocation(locTok.m_locator);
	assert(oneNode);
	return oneNode;
  } //makeOneTerminal


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
	    msg << "Unexpected token <" << pTok.getTokenEnumName() << ">; Expected <" << Token::getTokenAsString(eTokType) << ">";
	    MSG(&pTok, msg.str().c_str(), ERR);
	  }
	return false;
      }
    myTok = pTok;
    return true;
  }


  bool Parser::getNextToken(Token & tok)
  {
    bool brtn = m_tokenizer->getNextToken(tok);

    if(tok.m_type == TOK_ERROR_ABORT)
      {
	std::ostringstream msg;
	msg << "Unexpected token <" << tok.getTokenEnumName() << ">, exiting now";
	MSG(&tok, msg.str().c_str(), ERR);
	exit(1);
      }
    return brtn;
  }


  void Parser::unreadToken()
  {
    m_tokenizer->unreadToken();
  }

  // and including lastTok
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
  }


  void Parser::initPrimitiveUlamTypes()
  {
    // initialize primitive UlamTypes, in order!!
    UlamKeyTypeSignature nkey(m_state.m_pool.getIndexForDataString("Nav"), ULAMTYPE_DEFAULTBITSIZE[Nav]);
    UTI nidx = m_state.makeUlamType(nkey, Nav);
    assert(nidx == Nav);  //true for primitives

    UlamKeyTypeSignature vkey(m_state.m_pool.getIndexForDataString("Void"), ULAMTYPE_DEFAULTBITSIZE[Void]);
    UTI vidx = m_state.makeUlamType(vkey, Void);
    assert(vidx == Void);  //true for primitives

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

    UlamKeyTypeSignature ckey(m_state.m_pool.getIndexForDataString("Ut_Class"), ULAMTYPE_DEFAULTBITSIZE[Class]);  //bits tbd
    UTI cidx = m_state.makeUlamType(ckey, Class);
    assert(cidx == Class);

    UlamKeyTypeSignature akey(m_state.m_pool.getIndexForDataString("Atom"), ULAMTYPE_DEFAULTBITSIZE[UAtom]);
    UTI aidx = m_state.makeUlamType(akey, UAtom);
    assert(aidx == UAtom);

    UlamKeyTypeSignature pkey(m_state.m_pool.getIndexForDataString("Ut_Ptr"), ULAMTYPE_DEFAULTBITSIZE[Ptr]);
    UTI pidx = m_state.makeUlamType(pkey, Ptr);
    assert(pidx == Ptr);

    //initialize call stack with 'Int' UlamType pointer
    m_state.m_funcCallStack.init(iidx);
    m_state.m_nodeEvalStack.init(iidx);
    //m_state.m_eventWindow.init(iidx); //necessary?
  }


} //end MFM
