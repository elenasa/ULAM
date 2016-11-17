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


  u32 Lexer::push(std::string filename, bool onlyOnce)
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

    bool isStructuredComment = false;
    //get the first non-space character (newline included)
    do{
      c = eatComment(returnTok, isStructuredComment);  //returns next byte
    } while(c >= 0 && isspace(c));

    if(c < -1)
      {
	std::ostringstream errmsg;
	errmsg << "Invalid read";
	u32 idx = m_state.m_pool.getIndexForDataString(errmsg.str());
	returnTok.init(TOK_ERROR_LOWLEVEL, m_SS.getLocator(), idx); //is locator ok?
	m_lastToken = returnTok; //NEEDS SOME KIND OF TOK_ERROR
	return false; //error case
      }

    if(isStructuredComment)
      {
	unread();
	m_lastToken = returnTok;
	return true;
      }

    // switch to any special processing based on the first byte:
    u32 brtn = 0;
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
	  brtn = 0;   //including EOF == -1
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
	      brtn = makeWordToken(cstring, returnTok);
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
    if(brtn == 0)
      m_lastToken = returnTok;
    else
      {
	returnTok.init(TOK_ERROR_LOWLEVEL, m_SS.getLocator(), brtn);
	m_lastToken = returnTok;  //NEEDS SOME KIND OF TOK_ERROR!!!
      }
    return (brtn == 0);
  } //getNextToken


  //called because first byte was alpha
  u32 Lexer::makeWordToken(std::string& aname, Token & tok)
  {
    u32 brtn = 0;
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
	    return 0;
	  }
	else if(sptok == TOKSP_DEPRECATED)
	  {
	    std::ostringstream errmsg;
	    errmsg << "DEPRECATED keyword <" << aname << ">";
	    brtn = m_state.m_pool.getIndexForDataString(errmsg.str());
	    return brtn; //short circuit
	  }
	else
	  {
	    std::ostringstream errmsg;
	    errmsg << "Weird Lex! <" << aname;
	    errmsg << "> isn't a special keyword type..becomes identifier instead.";
	    brtn = m_state.m_pool.getIndexForDataString(errmsg.str());
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
    return brtn;
  } //makeWordToken


  //called because first byte was numeric
  u32 Lexer::makeNumberToken(std::string& anumber, Token & tok)
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
    return 0;
  } //makeNumberToken


  //starts with a non-alpha or non-digit, so
  //possibly a simple operator (e.g. +, =),
  //or a double operator
  u32 Lexer::makeOperatorToken(std::string& astring, Token & tok)
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
		u32 emsg = checkEllipsisToken(astring, firstloc);
		if(emsg != 0)
		  return emsg;
	      }
	    else if((ttype == TOK_SHIFT_LEFT) || (ttype == TOK_SHIFT_RIGHT))
	      {
		u32 smsg = checkShiftEqualToken(astring, firstloc);
		if(smsg == 1)
		  ttype = getTokenTypeFromString(astring);
		else if(smsg > 1)
		  return smsg;
		//else no change
	      }

	    tok.init(ttype, firstloc, 0);
	    return 0;
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
	return 0;
      }

    std::ostringstream errmsg;
    errmsg << "Weird! Lexer could not find match for <" << astring << ">";
    return m_state.m_pool.getIndexForDataString(errmsg.str());
  } //makeOperatorToken


  u32 Lexer::checkEllipsisToken(std::string& astring, Locator firstloc)
  {
    u32 bok = 0;
    s32 c3 = m_SS.read();
    if(c3 >= 0)
      {
	astring.push_back(c3);
	if(c3 != '.')
	  {
	    std::ostringstream errmsg;
	    errmsg << "Lexer could not find match for <" << astring << ">";
	    bok = m_state.m_pool.getIndexForDataString(errmsg.str());
	    unread();
	  }
      }
    else
      {
	std::ostringstream errmsg;
	errmsg << "Lexer could not find last dot for ellipsis <" << astring << ">";
	bok = m_state.m_pool.getIndexForDataString(errmsg.str());
      }
    return bok;
  } //checkellipsistoken

  u32 Lexer::checkShiftEqualToken(std::string& astring, Locator firstloc)
  {
    u32 bok = 0;
    s32 c3 = m_SS.read();
    if(c3 >= 0)
      {
	if(c3 == '=')
	  {
	    astring.push_back(c3);
	    bok = 1;
	  }
	else
	  unread();
      }
    else
      {
	std::ostringstream errmsg;
	errmsg << "Lexer could not find anything after logical shift <" << astring << ">";
	bok = m_state.m_pool.getIndexForDataString(errmsg.str());
      }
    return bok;
  } //checkShiftEqualToken

  //redo using nextByte; don't include open/close double quotes;
  //prepend length, append null byte before inserting in StringPoolUser
  u32 Lexer::makeDoubleQuoteToken(std::string& astring, Token & tok)
  {
    Locator firstloc = m_SS.getLocator();
    s32 c;

    astring = ""; //clear open quote

    //keep reading until end of quote or (EOF or error)
    //return last byte after comment
    //bypass a blackslash and nextcharacter
    while((c = m_SS.read()) >= 0)
      {
	if(c == '"') //close quote, not included
	  break;
	else
	  {
	    unread();
	    u8 abyte;
	    u32 cu = nextByte(abyte);
	    if(cu == 0)
	      astring.push_back(abyte);
	    else //error
	      return cu;
	  }
      } //end while

    if(c < 0)
      {
	if( c == -1) unread();
	std::ostringstream errmsg;
	errmsg << "Lexer could not complete double quoted string <" << astring << ">";
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }

    //format user string; length must be less than 256
    u32 slen = astring.length();
    if(slen >= 256)
      {
	std::ostringstream errmsg;
	errmsg << "Lexer could not complete double quoted string <" << astring << ">; Must be less than 256 length";
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }

    std::ostringstream newstr;
    if(slen == 0)
      newstr << 0;
    else
      newstr << slen << astring << 0; //slen doesn't include itself or terminaing byte; see StringPoolUser.

    u32 idx = m_state.m_upool.getIndexForDataString(newstr.str());
    tok.init(TOK_DQUOTED_STRING,firstloc,idx);
    return 0;
  } //makeDoubleQuoteToken

  u32 Lexer::makeSingleQuoteToken(std::string& astring, Token & tok)
  {
    Locator firstloc = m_SS.getLocator();
    s32 c ;

    astring = ""; //clear single tic

    //get next byte; save as its ascii numeric value
    //bypass a blackslash for nextcharacter
    if((c = m_SS.read()) >= 0)
      {
	if(c == '\'')
	  {
	    std::ostringstream errmsg; //disallow empty ''
	    errmsg << "Lexer prevents empty single quoted constant";
	    return m_state.m_pool.getIndexForDataString(errmsg.str());
	  }
	else
	  {
	    unread();
	    u8 abyte;
	    u32 cu = nextByte(abyte);
	    if(cu == 0)
	      astring.push_back(abyte);
	    else //error
	      return cu;
	  }
      }

    //next byte must be a tic
    if((c = m_SS.read()) != '\'')
      {
	unread();
	std::ostringstream errmsg;
	errmsg << "Lexer could not find closing single quote <" << astring << ">";
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }

    u32 idx = m_state.m_pool.getIndexForDataString(astring);
    tok.init(TOK_SQUOTED_STRING,firstloc,idx);
    return 0;
  } //makeSingleQuoteToken

  u32 Lexer::nextByte(u8& abyte)
  {
    s32 c ;

    //get next byte; save as its ascii numeric value
    //bypass a blackslash for nextcharacter
    if((c = m_SS.read()) >= 0)
      {
	if(c == '\\')
	  {
	    s32 d = m_SS.read();
	    switch((char) d)
	      {
	      case 'a':
		abyte = '\a'; //bell/alert 7
		break;
	      case 'b':
		abyte = '\b'; //backspace 8
		break;
	      case 't':
		abyte = '\t'; //horizontal tab 9
		break;
	      case 'n':
		abyte = '\n'; //newline 10
		break;
	      case 'v':
		abyte = '\v'; //vertical tab 11
		break;
	      case 'f':
		abyte = '\f'; //formfeed 12
		break;
	      case 'r':
		abyte = '\r'; //carriage return 13
		break;
	      case '"':
		abyte = '\"'; //double quote 34
		break;
	      case '\'':
		abyte = '\''; //single quote 39
		break;
	      case '?':
		abyte = '\?'; //questionmark 63
		break;
	      case '\\':
		abyte = '\\'; //backslash escape 92
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
		  u32 crtn = formatOctalConstant(ooo);
		  if(crtn == 0)
		    abyte = ooo; //octal number
		  else
		    return crtn; //error
		}
		break;
	      case 'x':
	      case 'X':
		{
		  u8 hh;
		  u32 crtn = formatHexConstant(hh);
		  if(crtn == 0)
		    abyte = hh;
		  else
		    return crtn; //error
		}
		break;
	      default:
		abyte = (d - '\0'); //save it
	      };
	  }
	else
	  abyte = (c - '\0'); //as a number
      }
    else //c < 0
      {
	if( c == -1) unread();
	std::ostringstream errmsg;
	errmsg << "Lexer could not complete next byte in quoted string";
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }
    return 0;
  } //nextByte

  u32 Lexer::formatOctalConstant(u8& rtn)
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
	  {
	    std::ostringstream errmsg;
	    errmsg << "Lexer found invalid digit '" << c << "'";
	    errmsg << " while formatting an octal constant";
	    return m_state.m_pool.getIndexForDataString(errmsg.str());
	  }
      }

    if(c < 0)
      {
	std::ostringstream errmsg;
	errmsg << "Lexer could not complete formatting an octal constant";
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }

    if(runningtotal < 256)
      {
	rtn = (u8) runningtotal;
	return 0;
      }

    std::ostringstream errmsg;
    errmsg << "Lexer formatted an invalid octal constant '";
    errmsg << runningtotal << "'";
    return m_state.m_pool.getIndexForDataString(errmsg.str());
  } //formatOctalConstant

  u32 Lexer::formatHexConstant(u8& rtn)
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
	    {
	    std::ostringstream errmsg;
	    errmsg << "Lexer found invalid digit '" << c << "'";
	    errmsg << " while formatting a hex constant";
	    return m_state.m_pool.getIndexForDataString(errmsg.str());
	    }
	  };
	runningtotal = runningtotal * 16 + cnum;
      } //while

    if(c < 0)
      {
	std::ostringstream errmsg;
	errmsg << "Lexer could not complete";
	errmsg << " formatting a hex constant";
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }

    if(runningtotal < 256)
      {
	rtn = (u8) runningtotal;
	return 0;
      }

    std::ostringstream errmsg;
    errmsg << "Lexer formatted an invalid hex constant '";
    errmsg << runningtotal << "'";
    return m_state.m_pool.getIndexForDataString(errmsg.str());
  } //formatHexConstant

  s32 Lexer::eatComment(Token& rtnTok, bool& isStructuredComment)
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
	    s32 e = m_SS.read();
	    if( e == '*')
	      {
		isStructuredComment = true;
		return makeStructuredCommentToken(rtnTok);
	      }
	    else
	      {
		unread(); //wasn't a 2nd *
		return eatBlockComment();
	      }
	  }
	unread();
      }
    return c;
  } //eatComment

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
		  return m_SS.read(); //found end of comment; return next byte
		else
		  unread(); //to re-read; in case d is *, for example
	      }
	    else
	      return d; // d error or eof
	  } // c is not *, get the next byte
      } //end while
    return c;
  } //eatBlockComment

  s32 Lexer::eatLineComment()
  {
    s32 c;
    //keep reading until newline or (EOF or error)
    //return byte after comment
    while((c = m_SS.read()) >= 0 && c != '\n');

    return c;  // a newline or EOF or error
  } //eatLineComment

  s32 Lexer::makeStructuredCommentToken(Token& tok)
  {
    Locator firstloc = m_SS.getLocator();
    std::string scstr;
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
		    u32 idx = m_state.m_pool.getIndexForDataString(scstr);
		    tok.init(TOK_STRUCTURED_COMMENT, firstloc, idx);
		    return m_SS.read(); //found end of comment; return next byte
		  }
		else
		  unread(); //to re-read; in case d is *, for example
	      }
	    else
	      return d; // d error or eof
	  } // c is not *, get the next byte
	else
	  scstr.push_back(c);
      } //end while
    return c;
  } //makeStructuredCommentToken

  TokenType Lexer::getTokenTypeFromString(std::string str)
  {
    return Token::getTokenTypeFromString(str.c_str());
  }

} //end MFM
