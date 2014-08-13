/* -*- c++ -*- */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include "Token.h"
#include "CompilerState.h"

namespace MFM{

  class Tokenizer
  {
  public:
    Tokenizer();
    virtual ~Tokenizer();

    /** pass through filename to underlying sourcestream */
    virtual bool push(std::string filename, bool onlyOnce = true) = 0;
    

    /** read SourceStream and produce TOKEN: get next byte, skip white
	space, return special characters, identifier token (string of
	letters and digits beginning with a letter), special keywords
	returns keyword tokens, deal with EOF token, comments, quotes, etc.
    */
    virtual bool getNextToken(Token & returnTok) = 0;
    
    void unreadToken();

  protected:

      Token m_lastToken;
      bool m_haveUnreadToken;

  private:
    
  };

}

#endif //end TOKENIZER_H
