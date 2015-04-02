#include <string.h>
#include "Token.h"
#include "CompilerState.h"

namespace MFM {

#define XX(a,b,c) b,

  static const char * tok_string[] = {
#include "Token.inc"
  };

#undef XX


#define XX(a,b,c) "TOK_" #a,

  static const char * tok_name[] = {
#include "Token.inc"
  };

#undef XX


#define XX(a,b,c) TOKSP_##c,

  static const SpecialTokenWork tok_special[] = {
#include "Token.inc"
  };

#undef XX


  void Token::init(TokenType t, Locator l, u32 d)
    {
      m_type = t;
      m_locator = l;
      m_dataindex = d;
    }

  const char * Token::getTokenString()
  {
    return tok_string[m_type];
  }

  const char * Token::getTokenEnumName()
  {
    return tok_name[m_type];
  }

  const char *  Token::getTokenAsString(TokenType ttype)
  {
    return  tok_string[ttype];
  }

  SpecialTokenWork Token::getSpecialTokenWork(TokenType ttype)
  {
    return tok_special[ttype];
  }

  TokenType Token::getTokenTypeFromString(const char * aname)
  {
    TokenType ttype = TOK_LAST_ONE;
    for(u32 t = 0; t < TOK_LAST_ONE; t++)
      {
	if(strcmp(tok_string[t],aname) == 0)
	  {
	    ttype = (TokenType) t;
	    break;
	  }
      }
    return ttype;
  } //getTokenTypeFromString

  //oops! do the remaining letters have to be lower case? no.
  //for the typename use helper in CS:;getTokenAsATypeName
  //no verification as to existence.
  //bool Token::isTokenAType(Token tok, CompilerState * state)
  bool Token::isTokenAType(Token tok)
  {
    return ((getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD) || (tok.m_type == TOK_TYPE_IDENTIFIER));
  }

  void Token::print(File * fp, CompilerState * state)
  {
    fp->write(state->getPathFromLocator(m_locator).c_str());
    fp->write(58); // : ascii decimal
    fp->write_decimal(m_locator.getLineNo());
    fp->write(46); // . ascii decimal
    fp->write_decimal(m_locator.getByteNo());
    fp->write(58); // : ascii decimal
    fp->write(getTokenEnumName());
    if(m_dataindex > 0)
      {
	fp->write(40); // ( ascii decimal
	fp->write(state->getTokenDataAsString(this).c_str());
	fp->write(41); // ) ascii decimal
      }
    fp->write(10); // \n ascii decimal for LF
  }

  bool Token::isUpper(char c)
  {
    return(c > 0x40 && c < 0x5B);
  }

}
