#include <iostream>
#include <assert.h>
#include <ctype.h>
#include "Lexer.h"
#include "Token.h"

namespace MFM {


  Lexer::Lexer(SourceStream & ss, CompilerState & state) : m_state(state), m_SS(ss)
  {
    //    std::string dummy = "dummy data";
    //m_dataAsString.push_back(dummy);
  }


  Lexer::~Lexer()
  {
    //m_SS = NULL;               //causes memory leaks
    //m_dataAsString.clear();
    //m_stringToDataIndex.clear(); //both? strings already destructed.
  }


  const std::string Lexer::getPathFromLocator(Locator& loc)
  {
    return m_SS.getPathFromLocator(loc);
  }


  bool Lexer::push(std::string filename, bool onlyOnce)
  {
    return m_SS.push(filename,onlyOnce);
  }


  u32 Lexer::getFileUlamVersion() const
  {
    return m_SS.getFileUlamVersion();
  }


  void Lexer::setFileUlamVersion(u32 ver)
  {
    m_SS.setFileUlamVersion(ver);
  }


  void Lexer::unread()
  {
    return m_SS.unread();
  }


  bool Lexer::getNextToken(Token & returnTok)
  {
    s32 c;

    if(m_haveUnreadToken)
      {
	returnTok = m_lastToken;
	m_haveUnreadToken = false;
	return true;
      }

    //get the first non-space character (newline included)
    do{
      c = eatComment();  //returns next byte
    } while(c >= 0 && isspace(c));

    if(c < -1)
      {
	returnTok.init(TOK_ERROR_ABORT, m_SS.getLocator(), 0); //is locator ok?
	m_lastToken = returnTok; //NEEDS SOME KIND OF TOK_ERROR
	return false;   //error case
      }

    // switch to any special processing based on the first byte:
    bool brtn = false;
    SpecialTokenWork sptok = TOKSP_UNCLEAR;
    std::string cstring(1, (char)c);

    TokenType ttype = getTokenTypeFromString(cstring);
    if(ttype != TOK_LAST_ONE)
      {
	returnTok.init(ttype, m_SS.getLocator(), 0);
	sptok = Token::getSpecialTokenWork(ttype);
      }

    switch(sptok)
      {
      case TOKSP_SINGLE:
	{
	  brtn = true;   //including EOF == -1
	  break;
	}
      case TOKSP_DQUOTE:
	{
	  //builds data string for returnTok
	  brtn = makeDoubleQuoteToken(cstring, returnTok);
	  break;
	}
      case TOKSP_SQUOTE:
	{
	  //builds data string for returnTok
	  brtn = makeSingleQuoteToken(cstring, returnTok);
	  break;
	}
      case TOKSP_UNCLEAR:
      default:
	{
	  //tokentype still unclear..
	  if(isalpha(c))
	    {
	      brtn =  makeWordToken(cstring, returnTok);
	    }
	  else if(isdigit(c))
	    {
	      brtn = makeNumberToken(cstring, returnTok);
	    }
	  else
	    {
	      brtn = makeOperatorToken(cstring, returnTok);
	    }
	  break;
	}
      }; //end switch

    //each Make_Token updates returnTok,
    //and uses m_SS;
    if(brtn)
      m_lastToken = returnTok;
    else
      {
	returnTok.init(TOK_ERROR_CONT, m_SS.getLocator(), 0);
	m_lastToken = returnTok;  //NEEDS SOME KIND OF TOK_ERROR!!!
      }
    return brtn;
  } //getNextToken


  //called because first byte was alpha
  bool Lexer::makeWordToken(std::string& aname, Token & tok)
  {
    Locator firstloc = m_SS.getLocator(); //save for token

    s32 c = m_SS.read();

    //continue with alpha or numeric or underscore
    while(c >= 0 && (isalpha(c) || isdigit(c) || c == '_' ))
      {
	aname.push_back(c);
	c = m_SS.read();
      }

    unread();

    TokenType ttype = getTokenTypeFromString(aname);
    if(ttype != TOK_LAST_ONE)
      {
	//SpecialTokenWork sptok = m_specialTokens[ttype];
	SpecialTokenWork sptok = Token::getSpecialTokenWork(ttype);

	if(sptok == TOKSP_KEYWORD || sptok == TOKSP_TYPEKEYWORD || sptok == TOKSP_CTLKEYWORD)
	  {
	    tok.init(ttype,firstloc,0);
	    return true;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Weird Lex! <" << aname << "> isn't a special keyword type..becomes identifier instead.";
	    MSG(m_state.getFullLocationAsString(firstloc).c_str(), msg.str().c_str(), ERR);
	  }
      }

    // build an identifier
    // index to data in map, vector
    u32 idx = m_state.m_pool.getIndexForDataString(aname);

    // if Capitalized, then TYPE_IDENT
    if(Token::isUpper(aname.at(0)))
      {
	tok.init(TOK_TYPE_IDENTIFIER,firstloc,idx);
      }
    else
      {
	tok.init(TOK_IDENTIFIER,firstloc,idx);
      }
    return true;
  } //makeWordToken


  //called because first byte was numeric
  bool Lexer::makeNumberToken(std::string& anumber, Token & tok)
  {
    Locator firstloc = m_SS.getLocator();
    bool floatflag = false;
    s32 c = m_SS.read();

    //not supporting floats anymore || c=='.' except for error msg
    while(c >= 0 && (isxdigit(c) || c=='x' || c=='X' || c=='.') )
      {
	if(c == '.')
	  floatflag = true;
	anumber.push_back(c);
	c = m_SS.read();
      }

    if(c == 'u' || c == 'U')
      {
	anumber.push_back(c);
	// build a number
	//data indexed in map, vector
	u32 idx = m_state.m_pool.getIndexForDataString(anumber);
	tok.init(TOK_NUMBER_UNSIGNED,firstloc,idx);
      }
    else
      {
	unread();
	// build a number
	//data indexed in map, vector
	u32 idx = m_state.m_pool.getIndexForDataString(anumber);
	if(floatflag)
	  tok.init(TOK_NUMBER_FLOAT,firstloc,idx);
	else
	  tok.init(TOK_NUMBER_SIGNED,firstloc,idx);
      }
    return true;
  } //makeNumberToken


  //starts with a non-alpha or non-digit, so
  //possibly a simple operator (e.g. +, =),
  //or a double operator
  bool Lexer::makeOperatorToken(std::string& astring, Token & tok)
  {
    Locator firstloc = m_SS.getLocator();

    s32 c = m_SS.read();

    // a dual operator, or error
    if(c >= 0)
      {
	// make dual operator: don't unread
	astring.push_back(c);

	TokenType ttype = getTokenTypeFromString(astring);
	if(ttype != TOK_LAST_ONE)
	  {
	    //special case, confirm 3rd dot
	    if(ttype == TOK_ELLIPSIS)
	      {
		if(!checkEllipsisToken(astring, firstloc))
		  return false;
	      }

	    tok.init(ttype, firstloc, 0);
	    return true;
	  }

	astring.erase(1);
      }

    unread();

    // simple operator
    // for example +, -, anything that could stand alone
    TokenType ttype = getTokenTypeFromString(astring);
    if(ttype != TOK_LAST_ONE)
      {
	tok.init(ttype, firstloc, 0);
	return true;
      }

    std::ostringstream msg;
    msg << "Weird! Lexer could not find match for: <" << astring << ">";
    MSG(m_state.getFullLocationAsString(firstloc).c_str(), msg.str().c_str(), ERR);

    return false;
  } //makeOperatorToken


  bool Lexer::checkEllipsisToken(std::string& astring, Locator firstloc)
  {
    bool bok = true;
    s32 c3 = m_SS.read();
    if(c3 >= 0)
      {
	astring.push_back(c3);
	if(c3 != '.')
	  {
	    std::ostringstream msg;
	    msg << "Lexer could not find match for: <" << astring << ">";
	    MSG(m_state.getFullLocationAsString(firstloc).c_str(), msg.str().c_str(), ERR);
	    unread();
	    bok = false;
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Lexer could not find last dot for ellipsis: <" << astring << ">";
	MSG(m_state.getFullLocationAsString(firstloc).c_str(), msg.str().c_str(), ERR);
	bok = false;
      }
    return bok;
  } //checkellipsistoken


  bool Lexer::makeDoubleQuoteToken(std::string& astring, Token & tok)
  {
    Locator firstloc = m_SS.getLocator();
    s32 c;
    //keep reading until end of quote or (EOF or error)
    //return last byte after comment
    //bypass a blackslash and nextcharacter
    while((c = m_SS.read()) >= 0)
      {
	astring.push_back(c);
	if( c == '"')
	  break;
	else if(c == '\\')
	  {
	    s32 d = m_SS.read();
	    astring.push_back(d);
	  }
      } //end while

    if(c < 0)
      {
	if( c == -1) unread();
	return false;
      }

    u32 idx = m_state.m_pool.getIndexForDataString(astring);
    tok.init(TOK_DQUOTED_STRING,firstloc,idx);
    return true;
  }


  bool Lexer::makeSingleQuoteToken(std::string& astring, Token & tok)
  {
    Locator firstloc = m_SS.getLocator();
    s32 c ;

    astring = ""; //clear single tic

    //get next byte; save as its ascii numeric value
    //bypass a blackslash for nextcharacter
    if((c = m_SS.read()) >= 0)
      {
	if(c == '\'')
	  return false; //disallow empty ''

	if(c == '\\')
	  {
	    s32 d = m_SS.read();
	    switch((char) d)
	      {
	      case 'a':
		astring.push_back('\a'); //bell/alert 7
		break;
	      case 'b':
		astring.push_back('\b'); //backspace 8
		break;
	      case 't':
		astring.push_back('\t'); //horizontal tab 9
		break;
	      case 'n':
		astring.push_back('\n'); //newline 10
		break;
	      case 'v':
		astring.push_back('\v'); //vertical tab 11
		break;
	      case 'f':
		astring.push_back('\f'); //formfeed 12
		break;
	      case 'r':
		astring.push_back('\r'); //carriage return 13
		break;
	      case '"':
		astring.push_back('\"'); //double quote 34
		break;
	      case '\'':
		astring.push_back('\''); //single quote 39
		break;
	      case '?':
		astring.push_back('\?'); //questionmark 63
		break;
	      case '\\':
		astring.push_back('\\'); //backslash escape 92
		break;
	      case '0':
	      case '1':
	      case '2':
	      case '3':
	      case '4':
	      case '5':
	      case '6':
	      case '7':
		{
		  unread();
		  u8 ooo;
		  if(formatOctalConstant(ooo))
		    astring.push_back(ooo); //octal number
		  else
		    return false;
		}
		break;
	      case 'x':
	      case 'X':
		{
		  u8 hh;
		  if(formatHexConstant(hh))
		    astring.push_back(hh);
		  else
		    return false;
		}
		break;
	      default:
		astring.push_back(d - '\0'); //save it
	      };
	  }
	else
	  astring.push_back(c - '\0'); //as a number
      }
    else //c < 0
      {
	if( c == -1) unread();
	return false;
      }

    //next byte must be a tic
    if((c = m_SS.read()) != '\'')
      {
	unread();
	return false;
      }

    u32 idx = m_state.m_pool.getIndexForDataString(astring);
    tok.init(TOK_SQUOTED_STRING,firstloc,idx);
    return true;
  } //makeSingleQuoteToken

  bool Lexer::formatOctalConstant(u8& rtn)
  {
    //where \ooo is one to three octal digits (0..7)
    u32 runningtotal = 0;
    s32 c;
    while((c = m_SS.read()) >= 0)
      {
	if(c == '\'')
	  {
	    unread();
	    break;
	  }
	if(c >= '0' && c < '8')
	  runningtotal = runningtotal * 8 + (c - '0');
	else
	  return false;
      }

    if(c < 0)
      return false;

    if(runningtotal < 256)
      {
	rtn = (u8) runningtotal;
	return true;
      }

    return false;
  } //formatOctalConstant

  bool Lexer::formatHexConstant(u8& rtn)
  {
    // where \xhh is one or more hexadecimal digits (0...9, a...f, A...F).
    u32 runningtotal = 0;
    s32 c;
    while((c = m_SS.read()) >= 0)
      {
	u32 cnum;
	if(c == '\'')
	  {
	    unread();
	    break;
	  }

	switch(c)
	  {
	  case 'a':
	  case 'A':
	    cnum = 10;
	    break;
	  case 'b':
	  case 'B':
	    cnum = 11;
	    break;
	  case 'c':
	  case 'C':
	    cnum = 12;
	    break;
	  case 'd':
	  case 'D':
	    cnum = 13;
	    break;
	  case 'e':
	  case 'E':
	    cnum = 14;
	    break;
	  case 'f':
	  case 'F':
	    cnum = 15;
	    break;
	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	    cnum = c - '0';
	    break;
	  default:
	    return false; //error!
	  };
	runningtotal = runningtotal * 16 + cnum;
      } //while

    if(c < 0)
      return false;

    if(runningtotal < 256)
      {
	rtn = (u8) runningtotal;
	return true;
      }
    return false;
  } //formatHexConstant

  s32 Lexer::eatComment()
  {
    s32 c = m_SS.read();

    // initially called with a value that is not EOF or space or digit or alpha
    // check for start of comment
    if(c == '/')
      {
	s32 d = m_SS.read();
	if( d == '/')
	  {
	    return eatLineComment();
	  }
	else if( d == '*')
	  {
	    return eatBlockComment();
	  }
	unread();
      }

    return c;
  }


  s32 Lexer::eatBlockComment()
  {
    s32 c;
    //keep reading until end of comment or (EOF or error)
    //return last byte after comment
    while((c = m_SS.read()) >= 0)
      {
	if(c == '*')
	  {
	    s32 d = m_SS.read();

	    if(d >= 0)
	      {
		if(d == '/')
		  {
		    //found end of comment; return next byte
		    return m_SS.read();
		  }
		else
		  {
		    unread(); //to re-read; in case d is *, for example
		  }
	      }
	    else
	      {
		// d error or eof
		return d;
	      }
	  } // c is not *, get the next byte

      } //end while

    return c;
  }


  s32 Lexer::eatLineComment()
  {
    s32 c;
    //keep reading until newline or (EOF or error)
    //return byte after comment
    while((c = m_SS.read()) >= 0 && c != '\n');

    return c;  // a newline or EOF or error
  }


  TokenType Lexer::getTokenTypeFromString(std::string str)
  {
    return Token::getTokenTypeFromString(str.c_str());
  }

} //end MFM
