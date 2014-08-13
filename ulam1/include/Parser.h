/* -*- c++ -*- */

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "itype.h"
#include "Tokenizer.h"
#include "Node.h"
#include "NodeBlock.h"
#include "NodeStatements.h"
#include "NodeBinaryOpEqual.h"
#include "NodeBinaryOp.h"
#include "NodeUnaryOp.h"
#include "Symbol.h"
#include "SymbolFunction.h"
#include "CompilerState.h"
#include "NodeFunctionCall.h"

namespace MFM{
  /** 
      Preparser wrapper class for tokenizer's (e.g. Lexer), within CompilerState.
   */
  class Parser
  {
  public:
    //    Parser(Tokenizer * izer);
    Parser(Tokenizer * izer, CompilerState & state);
    ~Parser();
      
    /** 
	<PROGRAM> := "ulam" + <CLASS_BLOCK> + <EOF> 
	(takes an optional error output arg).
     */
    Node * parseProgram(File * errOutput = NULL); 


  private:
    
    // owned by parent, e.g. Compiler object.  
    CompilerState & m_state;

    // owned by parent, e.g. Compiler object.  used to get Tokens
    Tokenizer * m_tokenizer;


    /** 
	<CLASS_BLOCK> := '{' + (<FUNC_DEF> | <DECL>) * + '}'
    */

    NodeBlockClass * parseClassBlock();
    bool parseDataMember(NodeStatements *& nextNode);

    /** 
	<BLOCK> := '{' + <STATEMENTS> + '}' 
    */
    Node * parseBlock();

    /** 
	<STATEMENTS> := NULL | <STATEMENT> + <STATEMENTS> 
    */
    Node * parseStatements();


    /** 
	<STATEMENT> := <SIMPLE_STATEMENT> | <BLOCK> | <CONTROL_STATEMENTS> | <FUNC_DEF>
     */
    Node * parseStatement();


    /** 
	<CONTROL_STATEMENT> := <IF_STATEMENT> | <WHILE_STATEMENT>
    */
    Node * parseControlStatement();


    /** 
	<IF_STATEMENT> := 'if' + '(' + <ASSIGN_EXPR> + ')' + <STATEMENT> + <OPT_ELSE_STATEMENT>
	<OPT_ELSE_STATEMENT> := 0 | 'else' + <STATEMENT>
    */
    Node * parseControlIf(Token ifTok);

    /**
       <WHILE_STATEMENT> := 'while' + '(' + <ASSIGN_EXPR> + ')' + <STATEMENT>
    */
    Node * parseControlWhile(Token wTok);



    /** 
	<SIMPLE_STATEMENT> := ( 0 | <DECL> | <FUNC_DECL> | <TYPE_DEF> | <ASSIGNEXPR> ) + ';'
     */
    Node * parseSimpleStatement();


    /**
       <TYPEDEF> := 'typedef' + <TYPE> +( <IDENT> | <IDENT> + '[' + <EXPRESSION> + ']') 
    */
    Node * parseTypedef();

    /** helper for parseTypedef */
    Node * makeTypedefSymbol(Token typeTok, Token identTok);

    /**
       <DECL> := <TYPE> + <VAR_DECLS>
       when flag is true stops after one decl for function parameters.
    */
    Node * parseDecl(bool parseSingleDecl = false);


    /**
       <ASSIGNEXPR> := <EXPRESSION> | <LVAL_EXPRESSION> + '=' + <ASSIGNEXPR>
    */ 
    Node * parseAssignExpr();


    /**
       <LVAL_EXPRESSION> := <IDENT> | <IDENT> + '[' + <EXPRESSION> + ']' 
    */
    Node * parseLvalExpr(Token identTok);


    /**
       <IDENT_EXPRESSION> := <LVAL_EXPRESSION> | <FUNC_CALL>
    */
    Node * parseIdentExpr(Token identTok);


    /**
       <FUNC_CALL> := <IDENT> + '(' + <ARGS> + ')'
    */
    Node * parseFunctionCall(SymbolFunction * fsym, Token identTok);

    /**
       <ARGS>    := 0 | <ARG> | <ARG> + ',' + <ARGS>
       <ARG>     := <ASSIGNEXPR>
    */
    bool parseRestOfFunctionCallArguments(NodeFunctionCall * callNode);

    /**
       <EXPRESSION> := <TERM> | <EXPRESSION> <ADDOP> <TERM>
    */
    Node * parseExpression();    
    
    
    /** 
	<TERM> := <FACTOR> | <TERM> <MULOP> <FACTOR>
    */
    Node * parseTerm();


    /**
       <FACTOR> := <IDENT_EXPRESSION> | <NUMBER> | '(' + <EXPRESSION> + ')' | <UNOP> + <FACTOR>
     */
    Node * parseFactor();


    /** 
	<EQOP> := '='
    */
    Node * parseRestOfAssignExpr(Node * leftNode);


    /**
       '['
    */
    Node * parseRestOfLvalExpr(Node * leftNode);


    /** <VAR_DECLS> := <VAR_DECL> | <VAR_DECL> + ',' + <VAR_DECLS>
	<VAR_DECL> := <LVAL_EXPRESSION>
    */
    Node * parseRestOfDecls(Token typeTok, Node * dNode);


    /** helper for parseDecl and parseRestOfDecls */
    Node * makeVariableSymbol(Token typeTok, Token identTok);


    /** 
	<FUNC_DEF> := <FUNC_DECL> + <BLOCK>
	<FUNC_DECL> := <TYPE> + <IDENT> + '(' + <FUNC_PARAMS> + ')'
     */
    Node * makeFunctionSymbol(Token typeTok, Token identTok);

    /** helper method */
    NodeBlockFunction * makeFunctionBlock(Token typeTok, Token identTok);
    
    /**
	<FUNC_PARAMS> := 0 | <FUNC_PARAM> | <FUNC_PARAM> + ',' + <FUNC_PARAMS>
	<FUNC_PARAM>  := <TYPE> + <VAR_DECL> | <FUNC_DECL>  	
    */
    void parseRestOfFunctionParameters(SymbolFunction * sym);


    /** 
	helper method for function definition, populates funcNode,
	returns true if body parsed 
    */
    bool parseFunctionBody(NodeBlockFunction *& funcNode);

    /** 
	<ADDOP> := '+' | '-'
    */
    Node * parseRestOfExpression(Node * leftNode);
    

    /** 
	<MULOP> := '*' | '/'
     */
    Node * parseRestOfTerm(Node * leftNode);


    /** 
	<UNOP> := '-' | '+' | '!'
    */
    Node * parseRestOfFactor();


    /**
       helper method to make assigment nodes
    */
    NodeBinaryOpEqual * makeAssignExprNode(Node * leftNode);

    /**
       helper method to make binary expression nodes
    */
    NodeBinaryOp * makeExpressionNode(Node * leftNode);


    /**
       helper method to make binary term nodes
    */
    NodeBinaryOp * makeTermNode(Node * leftNode);


    /**
       helper method to make unary factor nodes
    */
    NodeUnaryOp * makeFactorNode();

    /** helper, gets CLOSE_PAREN for <FACTOR>, CLOSE_SQUARE rest of LVal */
    bool getExpectedToken(TokenType eTokType, Token & myTok, bool quietly = false);
    bool getExpectedToken(TokenType closeTokType, bool quietly = false);


    /** helper , passes through to tokenizer */
    bool getNextToken(Token & tok);


    /** helper , passes through to tokenizer */
    void unreadToken();


    /** 
	helper, bypasses token until end reached 
	if EOF reached, it will unread it before returning
     */
    void getTokensUntil(TokenType lastTok);


    /** 
	initializes primitive UlamTypes into classBlock Symbol Table
     */
    void initPrimitiveUlamTypes();

  };
  
}

#endif //end PARSER_H
