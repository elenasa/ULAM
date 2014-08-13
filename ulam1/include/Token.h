/* -*- c++ -*- */

#ifndef TOKEN_H
#define TOKEN_H

#include "Locator.h"
#include "itype.h"
#include "File.h"


namespace MFM{
  
  enum SpecialTokenWork { TOKSP_UNCLEAR=0, TOKSP_KEYWORD, TOKSP_TYPEKEYWORD, TOKSP_CTLKEYWORD, TOKSP_SINGLE, TOKSP_COMMENT, TOKSP_DQUOTE, TOKSP_SQUOTE, TOKSP_HASDATA, TOKSP_ERROR};
 

#define XX(a,b,c) TOK_##a,

  enum TokenType 
  {
#include "Token.inc"
    TOK_LAST_ONE
  };

#undef XX

  class CompilerState; //forward

  struct Token 
  {
    TokenType m_type;
    Locator m_locator;  //fileid, lineno, byteno
    u32 m_dataindex;

    void init(TokenType t, Locator l, u32 d);

    const char * getTokenString();
    const char * getTokenEnumName();

    static SpecialTokenWork getSpecialTokenWork(TokenType ttype);
    static TokenType getTokenTypeFromString(const char * aname);
    static const char *  getTokenAsString(TokenType ttype);

    /** 
	ulam says an identifier is a Type when it starts with a capital letter
	or is predefined (e.g. Int, Bool, etc). 
    */
    static bool isTokenAType(Token tok, CompilerState * state);
    static bool isUpper(char c);

    void print(File * fp, CompilerState * state);
    
  };  
}

#endif  //end TOKEN_H
