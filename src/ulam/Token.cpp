#include "Token.h"
#include "CompilerState.h"

namespace MFM {

#define XX(a,b,c,d) 0,

  static u32 tok_stringid[] = {
#include "Token.inc"
  };

#undef XX

#define XX(a,b,c,d) 0,

  static u32 tok_nameid[] = {
#include "Token.inc"
  };

#undef XX


#define XX(a,b,c,d) b,

  static const char * tok_string[] = {
#include "Token.inc"
  };

#undef XX


#define XX(a,b,c,d) "TOK_" #a,

  static const char * tok_name[] = {
#include "Token.inc"
  };

#undef XX


#define XX(a,b,c,d) TOKSP_##c,

  static const SpecialTokenWork tok_special[] = {
#include "Token.inc"
  };

#undef XX

#define XX(a,b,c,d) d,

  static const OperatorOverloadableFlag tok_opol[] = {
#include "Token.inc"
  };

#undef XX

  Token::Token() {} //requires init.

  Token::Token(TokenType t, Locator l, u32 d) : m_type(t), m_locator(l), m_dataindex(d) {}

  Token::Token(const Token& tok) : m_type(tok.m_type), m_locator(tok.m_locator), m_dataindex(tok.m_dataindex) {}

  Token::~Token() {}

  void Token::initTokenMap(CompilerState & state)
  {
#define XX(a,b,c,d) tok_stringid[TOK_##a] = state.m_pool.getIndexForDataString(std::string(b));
#include "Token.inc"
#undef XX

#define XX(a,b,c,d) tok_nameid[TOK_##a] = state.m_pool.getIndexForDataString(std::string(tok_name[TOK_##a]));
#include "Token.inc"
#undef XX

    //special case: EOF
    tok_stringid[TOK_EOF] = state.m_pool.getIndexForDataString("EOF");
    tok_nameid[TOK_EOF] = state.m_pool.getIndexForDataString("TOK_EOF");
  } //initTokenMap (static)

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

  u32 Token::getTokenStringId() const
  {
    return tok_stringid[m_type];
  }

  const std::string & Token::getTokenStringFromPool(CompilerState * state) const
  {
    assert(state);
    //if(m_type == TOK_EOF)
    //  return "EOF";
    return state->m_pool.getDataAsString(tok_stringid[m_type]);
  }

  const std::string & Token::getTokenAsStringFromPool(TokenType ttype, CompilerState * state)
  {
    assert(state);
    //if(ttype == TOK_EOF)
    //  return "EOF";
    return state->m_pool.getDataAsString(tok_stringid[ttype]);
  } //static

  const char * Token::getTokenEnumName()
  {
    return tok_name[m_type];
  }

  u32 Token::getTokenEnumNameId()
  {
    return tok_nameid[m_type];
  }

  const std::string & Token::getTokenEnumNameFromPool(CompilerState * state) const
  {
    assert(state);
    return state->m_pool.getDataAsString(tok_nameid[m_type]);
  }

  SpecialTokenWork Token::getSpecialTokenWork(TokenType ttype)
  {
    return tok_special[ttype];
  } //static

  OperatorOverloadableFlag Token::getTokenOperatorOverloadableFlag(TokenType ttype)
  {
    return tok_opol[ttype];
  } //static

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
  } //getTokenTypeFromString (static)

  //oops! do the remaining letters have to be lower case? no.
  //for the typename use helper in CS:;getTokenAsATypeName
  //no verification as to existence.
  bool Token::isTokenAType(const Token& tok)
  {
    return ((getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD) || (tok.m_type == TOK_TYPE_IDENTIFIER));
  }

  bool Token::isUpper(char c)
  {
    return(c > 0x40 && c < 0x5B);
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
	fp->write(state->getTokenDataAsString(*this).c_str());
	fp->write(41); // ) ascii decimal
      }
    fp->write(10); // \n ascii decimal for LF
  } //print

  bool Token::operator<(const Token & tok2) const
  {
    return (m_dataindex < tok2.m_dataindex);
  }

  Token& Token::operator=(const Token& tokref)
  {
    m_type = tokref.m_type;
    m_locator = tokref.m_locator;
    m_dataindex = tokref.m_dataindex;
    return *this;
  }

  u32 Token::getOperatorOverloadFullNameId(const Token & tok, CompilerState * state)
  {
    if(Token::getTokenOperatorOverloadableFlag(tok.m_type) == OPOL_NOT)
      return 0; //eliminates invalid 'op's

    std::ostringstream opname; //out-of-band leading underscore
    opname << "_" << Token::getTokenAsStringFromPool(TOK_KW_OPERATOR, state);
    opname << Token::getOperatorHexName(tok, state); //hex

    return state->m_pool.getIndexForDataString(opname.str());
  } //static

  const std::string & Token::getOperatorHexName(const Token & tok, CompilerState * state)
  {
    assert(Token::getTokenOperatorOverloadableFlag(tok.m_type) == OPOL_IS);
    return Token::getOperatorHexNameFromString(Token::getTokenAsStringFromPool(tok.m_type, state), state);
  } //static

  const std::string & Token::getOperatorHexNameFromString(const std::string & opname, CompilerState * state)
{
  std::ostringstream ophex;
  for(u32 i = 0; i < opname.length(); i++)
    ophex << std::hex << (u32) opname.at(i);
  //return ophex.str();
  u32 ophexid = state->m_pool.getIndexForDataString(ophex.str());
  return state->m_pool.getDataAsString(ophexid);
} //static

bool Token::isOperatorOverloadIdentToken(CompilerState * state) const
{
  bool brtn = false;
  if(m_dataindex > 0)
    {
      std::string idname = state->m_pool.getDataAsString(m_dataindex);
      brtn = (idname.compare(0,9, "_operator") == 0);
    }
  return brtn;
}

u32 Token::getUlamNameIdForOperatorOverloadToken(CompilerState * state) const
{
  assert(isOperatorOverloadIdentToken(state));
  std::string ophexname = state->m_pool.getDataAsString(m_dataindex);
  std::ostringstream ulamname;
  ulamname << Token::getTokenAsStringFromPool(TOK_KW_OPERATOR, state);
  for(u32 i = 9; i < ophexname.length(); i+=2)
    {
      std::string byte = ophexname.substr(i,2);
      ulamname << (char) (u32) strtol(byte.c_str(), NULL, 16);
    }
  return state->m_pool.getIndexForDataString(ulamname.str());
}

} //MFM
