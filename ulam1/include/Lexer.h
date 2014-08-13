/* -*- c++ -*- */

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>
#include "itype.h"
#include "Tokenizer.h"
#include "SourceStream.h"
#include "CompilerState.h"

namespace MFM{
  
  class Lexer : public Tokenizer
  {
  public:
    Lexer(SourceStream & ss, CompilerState& state);
    ~Lexer();
    
    virtual bool push(std::string filename, bool onlyOnce = true);

    virtual const std::string getPathFromLocator(Locator& loc);  //calls m_SS      

    virtual bool getNextToken(Token & returnTok);

  private:
    CompilerState & m_state;
    
    SourceStream & m_SS;
    
    TokenType getTokenTypeFromString(std::string str);  //< returns TOK_LAST_ONE if not found
    
    bool makeWordToken(std::string& aname, Token & tok);
    bool makeNumberToken(std::string& anumber, Token & tok);
    bool makeOperatorToken(std::string& astring, Token & tok);
    
    bool makeDoubleQuoteToken(std::string& astring, Token & tok);
    bool makeSingleQuoteToken(std::string& astring, Token & tok);
    
    s32 eatComment();
    s32 eatBlockComment();
    s32 eatLineComment();

    /** current byte to be read again, request to underlying sourcestream */
    void unread();

  };
  
}

#endif //end LEXER_H
