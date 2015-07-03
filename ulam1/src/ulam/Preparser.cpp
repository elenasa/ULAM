#include <stdlib.h>
#include "Preparser.h"

namespace MFM {

  Preparser::Preparser(Tokenizer * izer, CompilerState & state): m_state(state), m_tokenizer(izer) {}

  Preparser::~Preparser(){}

  bool Preparser::push(std::string filename, bool onlyOnce)
  {
    return m_tokenizer->push(filename,onlyOnce);
  }

  u32 Preparser::getFileUlamVersion() const
  {
    return m_tokenizer->getFileUlamVersion();
  }

  void Preparser::setFileUlamVersion(u32 ver)
  {
    m_tokenizer->setFileUlamVersion(ver);
  }

  bool Preparser::getNextToken(Token & returnTok)
  {
    if(m_haveUnreadToken)
      {
	returnTok = m_lastToken;
	m_haveUnreadToken = false;
      }
    else
      {
	m_tokenizer->getNextToken(returnTok);
	m_lastToken = returnTok;
      }

    if(returnTok.m_type == TOK_KW_USE)
      {
	return preparseKeywordUse(returnTok);
      }
    else if(returnTok.m_type == TOK_KW_LOAD)
      {
	return preparseKeywordLoad(returnTok);
      }
    else if(returnTok.m_type == TOK_KW_ULAM)
      {
	return preparseKeywordUlam(returnTok);
      }
    return true;
  } //getNextToken

  bool Preparser::preparseKeywordUse(Token & tok)
  {
    std::string pkgname;
    Token useTok = tok;

    if(preparsePackageName(pkgname))
      {
	if(push(pkgname))
	  {
	    return getNextToken(tok);
	  }
	else
	  {
	    std::ostringstream errmsg;
	    errmsg << "Cannot fufill 'use " << pkgname << "' request";
	    u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	    tok.init(TOK_ERROR_ABORT, useTok.m_locator, idx);
	  }
      }
    return false;
  } //preparseKeywordUse

  bool Preparser::preparseKeywordLoad(Token & tok)
  {
    std::string pkgname;
    Token loadTok = tok;

    if(preparsePackageName(pkgname))
      {
	if(push(pkgname,false))
	  {
	    return getNextToken(tok);
	  }
	else
	  {
	    std::ostringstream errmsg;
	    errmsg << "Cannot fufill 'load " << pkgname << "' request";
	    u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	    tok.init(TOK_ERROR_ABORT, loadTok.m_locator, idx);
	  }
      }
    return false;
  } //preparseKeywordLoad

  bool Preparser::preparsePackageName(std::string & pStr)
  {
    Token pTok;
    while(m_tokenizer->getNextToken(pTok))
      {
	switch(pTok.m_type)
	  {
	  case TOK_TYPE_IDENTIFIER:
	  case TOK_IDENTIFIER:
	    {
	      pStr.append(m_state.getTokenDataAsString(&pTok));
	      break;
	    }
	  case TOK_DOT:
	    {
	      pStr.append(1,'/'); //substitute slash for dot
	      break;
	    }
	  case TOK_SEMICOLON:
	    {
	      pStr.append(".ulam"); // ulam suffix
	      return true; //done, ate ;
	    }
	  default:
	    m_tokenizer->unreadToken();
	    return false;
	  } //end switch
      } //end while
    return false;
  } //preparsePackageName

  bool Preparser::preparseKeywordUlam(Token & tok)
  {
    bool rtnBool = false;
    Token ulamTok = tok;

    Token nTok;
    getNextToken(nTok);

    if(nTok.m_type == TOK_NUMBER_SIGNED)
      {
	std::string nstr = m_state.getTokenDataAsString(&nTok);
	const char * numlist = nstr.c_str();
	char * nEnd;
	s32 numval = strtol(numlist, &nEnd, 0);   //base 10, 8, or 16
	if (*numlist == 0 || *nEnd != 0)
	  {
	    std::ostringstream errmsg;
	    errmsg << " Bad 'ulam' version number: " << nstr;
	    u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	    tok.init(TOK_ERROR_ABORT, ulamTok.m_locator, idx);
	    rtnBool = false;
	  }
	else
	  {
	    setFileUlamVersion(numval);
	    rtnBool = true;
	  }
      }
    else
      {
	std::ostringstream errmsg;
	errmsg << " Non-numeric 'ulam version': ";
	errmsg << nTok.getTokenString();
	u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	tok.init(TOK_ERROR_ABORT, ulamTok.m_locator, idx);
	rtnBool = false;
      }

    getNextToken(nTok);
    if(nTok.m_type != TOK_SEMICOLON)  //e.g. float used in error
      {
	std::ostringstream errmsg;
	errmsg << " 'ulam version' ended with <";
	errmsg << nTok.getTokenString() << ">";
	u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	tok.init(TOK_ERROR_ABORT, ulamTok.m_locator, idx);
	rtnBool = false;
      }
    else
      rtnBool = getNextToken(tok);

    return rtnBool;
  } //preparseKeywordUlam

} //end MFM
