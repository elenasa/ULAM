/**                                        -*- mode:C++ -*-
 * Parser.h -  Basic Parse handling for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
 *
 * This file is part of the ULAM programming language compilation system.
 *
 * The ULAM programming language compilation system is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The ULAM programming language compilation system is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the ULAM programming language compilation system
 * software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

/**
  \file Parser.h -  Basic Parse handling for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


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
	<PROGRAM> := <PROGRAM_DEF>* + <EOF>

	Ends when subject of compile (i.e. startstr) has been parsed
	(takes an optional error output arg).
     */
    Node * parseProgram(std::string startstr, File * errOutput = NULL); 


  private:
    
    // owned by parent, e.g. Compiler object.  
    CompilerState & m_state;

    // owned by parent, e.g. Compiler object.  used to get Tokens
    Tokenizer * m_tokenizer;


    /** 
	<PROGRAM_DEF> := <QUARK_DEF> | <ELEMENT_DEF>
	<ELEMENT_DEF> := 'element' + <TYPE_IDENT> + <CLASS_BLOCK>
	<QUARK_DEF>   := 'quark'   + <TYPE_IDENT> + <CLASS_BLOCK> 
    */
    bool parseThisClass();


    /** 
	<CLASS_BLOCK> := '{' + <DATA_MEMBERS> + '}'
    */
    NodeBlockClass * parseClassBlock();


    /**
       <DATA_MEMBERS> := ( <FUNC_DEF> | <DECL> + ';' | <TYPE_DEF> + ';' )*
     */
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
       <TYPEDEF> := 'typedef' + <TYPE> + <TYPE_EXPRESSION>
       <TYPE_EXPRESSION> := ( <TYPE_IDENT> | <TYPE_IDENT> + '[' + <EXPRESSION> + ']') 
    */
    Node * parseTypedef();


    /** helper for parseTypedef */
    Node * makeTypedefSymbol(Token typeTok, u32 typebitsize, Token identTok);


    /**
       <DECL> := <TYPE> + <VAR_DECLS>
       <TYPE_NAME> := 'Int' | 'Float' | 'Bool' | <TYPE_IDENT>
       <TYPE> := <TYPE_NAME> | <TYPE_NAME> + '(' + <EXPRESSION> + ')'

       <TYPE_IDENT> := /^[A-Z][A-Za-z0-9\_]*
       (when flag is true stops after one decl for function parameters).
    */
    Node * parseDecl(bool parseSingleDecl = false);

    /** helper for parsing type; returns bitsize or 0 */
    u32 parseTypeBitsize(Token typeTok);

    /**
       <RETURN_STATMENT> := 'return' + (0 | <ASSIGNEXPR>)
    */
    Node * parseReturn();


    /**
       <ASSIGNEXPR> := <EXPRESSION> | <LVAL_EXPRESSION> + '=' + <ASSIGNEXPR>
    */ 
    Node * parseAssignExpr();


    /**
       <LVAL_EXPRESSION> := <IDENT> | <IDENT> + '[' + <EXPRESSION> + ']'
       <IDENT> := /^[a-z][A-Za-z0-9\_]*
    */
    Node * parseLvalExpr(Token identTok);


    /**
       <IDENT_EXPRESSION> := <LVAL_EXPRESSION> | <FUNC_CALL> | <MEMBER_SELECT_EXPRESSION>
    */
    Node * parseIdentExpr(Token identTok);


    /** 
	<MEMBER_SELECT_EXPRESSION> := <IDENT_EXPRESSION> + '.' + <IDENT_EXPRESSION>
    */
    Node * parseMemberSelectExpr(Token memberTok);

    Node * parseRestOfMemberSelectExpr(Node * classInstanceNode); 


    /**
       <FUNC_CALL> := <IDENT> + '(' + <ARGS> + ')'
    */
    Node * parseFunctionCall(Token identTok);


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


    /** 
	<VAR_DECLS> := <VAR_DECL> | <VAR_DECL> + ',' + <VAR_DECLS>
	<VAR_DECL> := <LVAL_EXPRESSION>
    */
    Node * parseRestOfDecls(Token typeTok, u32 typebitsize, Node * dNode);


    /** helper for parseDecl and parseRestOfDecls */
    Node * makeVariableSymbol(Token typeTok, u32 typebitsize, Token identTok);


    /** 
	<FUNC_DEF>  := <FUNC_DECL> + <BLOCK>
	<FUNC_DECL> := <TYPE> + <IDENT> + '(' + <FUNC_PARAMS> + ')'
     */
    Node * makeFunctionSymbol(Token typeTok, u32 typebitsize, Token identTok);


    /** helper method */
    NodeBlockFunctionDefinition * makeFunctionBlock(Token typeTok, u32 typebitsize, Token identTok);
    

    /**
	<FUNC_PARAMS> := 0 | <FUNC_PARAM> | <FUNC_PARAM> + ',' + <FUNC_PARAMS>
	<FUNC_PARAM>  := <TYPE> + <VAR_DECL> | <FUNC_DECL>  	
    */
    void parseRestOfFunctionParameters(SymbolFunction * sym);


    /** 
	helper method for function definition, populates funcNode,
	returns true if body parsed 
    */
    bool parseFunctionBody(NodeBlockFunctionDefinition *& funcNode);


    /** 
	<ADDOP> := '+' | '-'
    */
    Node * parseRestOfExpression(Node * leftNode);
    

    /** 
	<MULOP> := '*' | '/'
     */
    Node * parseRestOfTerm(Node * leftNode);


    /** 
	<UNOP> := '-' | '+' | '!' | <CAST>
    */
    Node * parseRestOfFactor();


    /**
       <CAST> := '(' + <TYPE> + ')'
    */
    Node * makeCastNode(Token typeTok);
    Node * parseRestOfCastOrExpression();

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
