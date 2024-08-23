#include <stdlib.h>
#include "Preparser.h"
#include "UlamVersion.h"

namespace MFM {

  Preparser::Preparser(Tokenizer * izer, CompilerState & state): m_state(state), m_tokenizer(izer) {}

  Preparser::~Preparser(){}

  u32 Preparser::push(std::string filename, bool onlyOnce)
  {
    return m_tokenizer->push(filename,onlyOnce);
  }

  u32 Preparser::exists(std::string filename)
  {
    return m_tokenizer->exists(filename);
  }

  u32 Preparser::getFileUlamVersion() const
  {
    return m_tokenizer->getFileUlamVersion();
  }

  void Preparser::setFileUlamVersion(u32 ver)
  {
    m_tokenizer->setFileUlamVersion(ver);
  }

  bool Preparser::checkFileUlamVersion(u32 ver)
  {
    return (ver <= ULAMVERSION); //ok if le current compiler version (see UlamVersion.h)
  }

  bool Preparser::peekFirstToken(Token & firstTok)
  {
    assert(!m_haveUnreadToken);
    m_tokenizer->peekFirstToken(firstTok);
    return true;
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
	u32 pmsg = exists(pkgname);
	if(pmsg == 0)
	  {
	    return getNextToken(tok);
	  }
	else
	  {
	    tok.init(TOK_ERROR_LOWLEVEL, useTok.m_locator, pmsg);
	  }
      }
    else
      {
	std::ostringstream errmsg;
	errmsg << "Bad filename to 'use'";
	u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	tok.init(TOK_ERROR_LOWLEVEL, useTok.m_locator, idx);
      }
    return false;
  } //preparseKeywordUse

  bool Preparser::preparseKeywordLoad(Token & tok)
  {
    std::string filename;
    Token loadTok = tok;

    //backward compatible (look for package name first, might add suffix) (t3872)
    if(preparsePackageName(filename) || preparseFileName(filename))
      {
	u32 fmsg = push(filename,false);
	if(fmsg == 0)
	  {
	    return getNextToken(tok);
	  }
	else
	  {
	    tok.init(TOK_ERROR_LOWLEVEL, loadTok.m_locator, fmsg);
	  }
      }
    else
      {
	std::ostringstream errmsg;
	errmsg << "Bad filename to 'load'";
	u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	tok.init(TOK_ERROR_LOWLEVEL, loadTok.m_locator, idx);
      }
    return false;
  } //preparseKeywordLoad

  bool Preparser::preparseFileName(std::string & pStr)
  {
    bool rtnb = false;
    Token pTok;
    m_tokenizer->getNextToken(pTok);

    if(pTok.m_type == TOK_DQUOTED_STRING)
      {
	//without double quotes (t41130,4)
	u32 fnid = m_state.m_tokenupool.formatDoubleQuotedFileNameUnquoted(pTok.m_dataindex, & m_state);
	if(fnid > 0)
	  {
	    pStr.append(m_state.m_pool.getDataAsString(fnid));
	    Token qTok;
	    m_tokenizer->getNextToken(qTok);

	    if(qTok.m_type == TOK_SEMICOLON)
	      {
		rtnb = true;
	      }
	    else
	      {
		m_tokenizer->unreadToken();
	      }
	  }
	//else unprintable chars in filename (t41135)
      }
    else
	m_tokenizer->unreadToken();
    return rtnb;
  } //preparseFileName

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
	      pStr.append(m_state.getTokenDataAsString(pTok));
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
	std::string nstr = m_state.getTokenDataAsString(nTok);
	const char * numlist = nstr.c_str();
	char * nEnd;
	s32 numval = strtol(numlist, &nEnd, 0);   //base 10, 8, or 16
	if (*numlist == 0 || *nEnd != 0)
	  {
	    std::ostringstream errmsg;
	    errmsg << "Bad ulam version number: " << nstr;
	    u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	    tok.init(TOK_ERROR_LOWLEVEL, ulamTok.m_locator, idx);
	    rtnBool = false;
	  }
	else if(!checkFileUlamVersion(numval))
	  {
	    std::ostringstream errmsg;
	    errmsg << "Declared source version " << nstr;
	    errmsg << ", exceeds the current compiler version " << ULAMVERSION;
	    u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	    tok.init(TOK_ERROR_LOWLEVEL, ulamTok.m_locator, idx);
	    rtnBool = false;
	  }
	else
	  {
	    setFileUlamVersion(numval);
	    rtnBool = true;
	  }
      }
    else if(nTok.m_type == TOK_NUMBER_FLOAT)
      {
	std::ostringstream errmsg;
	errmsg << "Decimal point illegal in an ulam version number";
	u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	tok.init(TOK_ERROR_LOWLEVEL, ulamTok.m_locator, idx);
	rtnBool = false;
      }
    else
      {
	std::ostringstream errmsg;
	errmsg << "Invalid ulam version: ";
	errmsg << nTok.getTokenStringFromPool(&m_state).c_str();
	errmsg << "; Currently using version " << ULAMVERSION;
	u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	tok.init(TOK_ERROR_LOWLEVEL, ulamTok.m_locator, idx);
	rtnBool = false;
      }

    getNextToken(nTok); //should be semi-colon
    if(rtnBool)
      {
	//clear to replace error message, if need be
	if(nTok.m_type != TOK_SEMICOLON)  //e.g. float used in error
	  {
	    std::ostringstream errmsg;
	    errmsg << "ulam version ending with <";
	    errmsg << nTok.getTokenStringFromPool(&m_state).c_str() << ">";
	    u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	    tok.init(TOK_ERROR_LOWLEVEL, ulamTok.m_locator, idx);
	    rtnBool = false;
	  }
	else
	  rtnBool = getNextToken(tok);
      }
    return rtnBool;
  } //preparseKeywordUlam

} //end MFM
