#include "Preparser.h"

namespace MFM {

  Preparser::Preparser(Tokenizer * izer, CompilerState & state): m_state(state), m_tokenizer(izer) {}

  Preparser::~Preparser(){}


  bool Preparser::push(std::string filename, bool onlyOnce)
  {
    return m_tokenizer->push(filename,onlyOnce);
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
	      case TOK_IDENTIFIER:
		{
		  pStr.append(m_state.m_pool.getDataAsString(pTok.m_dataindex));
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

  
} //end MFM
