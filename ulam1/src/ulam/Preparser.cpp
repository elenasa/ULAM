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
  }


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
	    tok.init(TOK_ERROR_ABORT, useTok.m_locator, 0);
	  }
      }

    return false;
  }


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
	    tok.init(TOK_ERROR_ABORT, loadTok.m_locator, 0);
	  }
      }

    return false;
  }


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
		  //m_tokenizer->unreadToken();  // eat it?
		  return true;       //done
		}
	      default:
		m_tokenizer->unreadToken();
		return false;
	      } //end switch

	  } //end while

	return false;
  }


  bool Preparser::preparseKeywordUlam(Token & tok)
  {
    bool rtnBool = false;
    Token ulamTok = tok;

    Token nTok;
    getNextToken(nTok);

    if(nTok.m_type == TOK_NUMBER)
      {
	std::string nstr = m_state.getTokenDataAsString(&nTok);
	const char * numlist = nstr.c_str();
	char * nEnd;
	s32 numval = strtol(numlist, &nEnd, 10);   //base 10
	setFileUlamVersion(numval);
	rtnBool = true;

	getNextToken(nTok);
	if(nTok.m_type != TOK_SEMICOLON)  //e.g. float used in error
	   {
	     tok.init(TOK_ERROR_ABORT, ulamTok.m_locator, 0);
	     rtnBool = false;
	   }
	else
	  {
	    rtnBool = getNextToken(tok);
	  }
      }
    else
      {
	tok.init(TOK_ERROR_ABORT, ulamTok.m_locator, 0);
	rtnBool = false;
      }

    return rtnBool;
  }


} //end MFM
