/* -*- c++ -*- */

#ifndef PREPARSER_H
#define PREPARSER_H

#include <string>
#include <vector>
#include <map>
#include "itype.h"
#include "Tokenizer.h"
#include "SourceStream.h"

namespace MFM{
  /** 
      Preparser wrapper class for tokenizer's (e.g. Lexer)
   */
  class Preparser : public Tokenizer
  {
  public:
    Preparser(Tokenizer * izer, CompilerState & state);
    ~Preparser();
      
    virtual bool push(std::string filename, bool onlyOnce = true);

    virtual bool getNextToken(Token & returnTok);
    
  private:
    CompilerState & m_state;

    Tokenizer * m_tokenizer;
    
    bool preparseKeywordUse(Token & tok);
    bool preparseKeywordLoad(Token & tok);
    bool preparsePackageName(std::string & pStr);

  };
  
}

#endif //end PREPARSER_H
