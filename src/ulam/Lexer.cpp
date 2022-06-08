#include <iostream>
#include <assert.h>
#include <ctype.h>
#include "Lexer.h"
#include "Token.h"

namespace MFM {


  Lexer::Lexer(SourceStream & ss, CompilerState & state) : m_state(state), m_SS(ss) { }

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
	u32 idx = m_state.m_pool.getIndexForDataString("Invalid read");
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
	  else if(c == '@')
	    {
	      brtn = makeFlagToken(cstring, returnTok);
	    }
	  else if(c == '_')
	    {
	      brtn = makeSubstituteToken(cstring, returnTok);
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
    while((c >= 0) && (isalpha(c) || isdigit(c) || c == '_' ))
      {
	aname.push_back(c);
	c = m_SS.read();
      }

    unread();

    TokenType ttype = getTokenTypeFromString(aname);
    if(ttype != TOK_LAST_ONE)
      {
	SpecialTokenWork sptok = Token::getSpecialTokenWork(ttype);

	if(sptok == TOKSP_KEYWORD || sptok == TOKSP_TYPEKEYWORD || sptok == TOKSP_CTLKEYWORD)
	  {
	    tok.init(ttype,firstloc,0);
	    return 0;
	  }
	else if(sptok == TOKSP_OPERATORKEYWORD)
	  {
	    std::string opstr;
	    Token optok;
	    u32 rtn = makeOperatorToken(opstr, optok);
	    if(rtn == 0) //overloaded operator
	      {
		//do again in case of dual operator
		rtn = makeOperatorToken(opstr, optok);
		if(rtn == 0)
		  {
		    //make operator overload string identifier: appends 'op' in hex (like C);
		    //(can't use 'op' in func name, gcc complains: too many arguments, or 'op' ignored);
		    // and out-of-band; invalid 'op's eliminated by testing OPOL flag;
		    u32 opnameid = Token::getOperatorOverloadFullNameId(optok, &m_state);
		    if(opnameid != 0)
		      tok.init(TOK_IDENTIFIER,firstloc,opnameid);
		    else
		      {
			std::ostringstream errmsg;
			errmsg << "Weird Lex! <" << opstr;
			errmsg << "> isn't an overloadable operation";
			rtn = m_state.m_pool.getIndexForDataString(errmsg.str());
		      }
		  }
		//else
	      }
	    //else
	    return rtn; //short-circuit
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
	    errmsg << "> isn't a special keyword type..becomes identifier instead";
	    brtn = m_state.m_pool.getIndexForDataString(errmsg.str());
	  }
      }

    // build an identifier
    // index to data in map, vector
    // Type/Identifier length must be less than 256 (ulam-5)
    u32 slen = aname.length();
    if(slen > MAX_IDENTIFIER_LENGTH)
      {
	std::ostringstream errmsg;
	errmsg << "Lexer could not complete ";
	if(Token::isUpper(aname.at(0)))
	  errmsg << "Type ";
	errmsg << "Identifier <" << aname;
	errmsg << ">; Must be less than " << MAX_IDENTIFIER_LENGTH + 1;
	errmsg << " length, not " << slen;
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }

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
    bool unsignedflag = false;
    bool octalflag = false;
    bool hexflag = false;
    bool binaryflag = false;
    bool baddigit = false;
    u8 a = anumber[0] - '0';
    s32 c = m_SS.read();

    //set flags based on first and second digit
    if(c >= 0 && (isxdigit(c) || c=='x' || c=='X' || c=='.') )
      {
	if(a == 0)
	  {
	    unsignedflag = true;
	    if((c=='x' || c=='X'))
	      hexflag = true;
	    else if((c=='b' || c=='B'))
	      binaryflag = true;
	    else if(c == '.')
	      floatflag = true;
	    else if(c - '0' < 8)
	      octalflag = true;
	    else
	      baddigit = true;
	  }
	else if(c == '.')
	  floatflag = true; //error/t3105
	else if(!isdigit(c))
	  baddigit = true; //decimal

	anumber.push_back(c);
	c = m_SS.read();
      }
    //else no read, fall through

    // third digit and beyond..
    //not supporting floats anymore || c=='.' except for error msg
    while(c >= 0 && (isxdigit(c) || c=='.') )
      {
	if(c == '.')
	  floatflag = true;

	//note: error checking also done in NodeTerminal, strto64.cpp
	u8 anum = c - '0';
	if(octalflag && (anum > 7))
	  baddigit = true;
	else if(binaryflag && (anum > 1))
	  baddigit = true;
	else if(!hexflag && !isdigit(c))
	  baddigit = true; //t.f. decimal

	anumber.push_back(c);
	c = m_SS.read();
      }

    if(c == 'u' || c == 'U')
      {
	anumber.push_back(c);
	unsignedflag = true;
      }
    else
      unread();

    // build a number
    //data indexed in map, vector
    u32 idx = 0;

    if(baddigit)
      {
	std::ostringstream errmsg;
	errmsg << "Lexer formatted an invalid ";
	if(octalflag)
	  errmsg << "octal";
	else if(hexflag)
	  errmsg << "hex";
	else if(binaryflag)
	  errmsg << "binary";
	else
	  errmsg << "decimal";
	errmsg << " constant '" << anumber << "'";
	idx = m_state.m_pool.getIndexForDataString(errmsg.str());
      }
    else
      {
	idx = m_state.m_pool.getIndexForDataString(anumber);
	if(floatflag)
	  tok.init(TOK_NUMBER_FLOAT,firstloc,idx);
	else if(unsignedflag)
	  tok.init(TOK_NUMBER_UNSIGNED,firstloc,idx);
	else
	  tok.init(TOK_NUMBER_SIGNED,firstloc,idx);
	idx = 0; //ok
      }
    return idx;
  } //makeNumberToken

  //called because first byte was @
  u32 Lexer::makeFlagToken(std::string& aname, Token & tok)
  {
    u32 brtn = 0;
    Locator firstloc = m_SS.getLocator(); //save for token

    s32 c = m_SS.read(); //capital letter follows @ for a valid flag

    if(Token::isUpper(c))
      {
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
	    SpecialTokenWork sptok = Token::getSpecialTokenWork(ttype);
	    if(sptok == TOKSP_FLAGKEYWORD)
	      {
		tok.init(ttype,firstloc,0);
		return 0;
	      }
	  }
      }
    //else not a flag

    std::ostringstream errmsg;
    errmsg << "Weird Lex! <" << aname;
    errmsg << "> does not precede a valid flag keyword";
    brtn = m_state.m_pool.getIndexForDataString(errmsg.str());
    return brtn;
  } //makeFlagToken

  //called because first byte was _
  u32 Lexer::makeSubstituteToken(std::string& aname, Token & tok)
  {
    u32 brtn = 0;
    Locator firstloc = m_SS.getLocator(); //save for new token

    s32 c = m_SS.read(); //capital letter follows __ for a valid flag

    if(c == '_')
      {

	while((c >= 0) && (Token::isUpper(c) || (c == '_')))
	  {
	    //continue with uppercase alpha, or 2 underscore
	    aname.push_back(c);
	    c = m_SS.read();
	  }

	unread();

	TokenType ttype = getTokenTypeFromString(aname);
	if(ttype != TOK_LAST_ONE)
	  {
	    SpecialTokenWork sptok = Token::getSpecialTokenWork(ttype);
	    if(sptok == TOKSP_FLAGKEYWORD)
	      {
		if(ttype == TOK_KW_FLAG_INSERTFILE)
		  {
		    //substitute a string token of filename from this locator
		    std::string path = m_state.getPathFromLocator(firstloc);
		    u32 idx = 0;
		    u32 rtn = formatUserString(path, idx);
		    if(rtn == 0)
		      tok.init(TOK_DQUOTED_STRING, firstloc, idx);
		    return rtn; // == 0 == ok
		  }
		if(ttype == TOK_KW_FLAG_INSERTPATH)
		  {
		    //substitute a string token of file path from this locator
		    std::string path = m_state.getFullPathFromLocator(firstloc);
		    u32 idx = 0;
		    u32 rtn = formatUserString(path, idx);
		    if(rtn == 0)
		      tok.init(TOK_DQUOTED_STRING, firstloc, idx);
		    return rtn; // == 0 == ok
		  }
		else if(ttype == TOK_KW_FLAG_INSERTLINE)
		  {
		    //substitute a number token for line of file from this loc
		    std::ostringstream ss;
		    ss << firstloc.getLineNo();
		    std::string lineno = ss.str();
		    u32 idx = m_state.m_pool.getIndexForDataString(lineno);
		    tok.init(TOK_NUMBER_UNSIGNED, firstloc, idx);
		    return 0;
		  }
		else if(ttype == TOK_KW_FLAG_INSERTFUNC)
		  {
		    tok.init(ttype, firstloc, 0);
		    return 0;
		  }
		else if(ttype == TOK_KW_FLAG_INSERTCLASS)
		  {
		    tok.init(ttype, firstloc, 0);
		    return 0;
		  }
		else if(ttype == TOK_KW_FLAG_INSERTCLASSSIGNATURE)
		  {
		    tok.init(ttype, firstloc, 0);
		    return 0;
		  }
		else if(ttype == TOK_KW_FLAG_INSERTCLASSNAMEPRETTY)
		  {
		    tok.init(ttype, firstloc, 0);
		    return 0;
		  }
		else if(ttype == TOK_KW_FLAG_INSERTCLASSNAMESIMPLE)
		  {
		    tok.init(ttype, firstloc, 0);
		    return 0;
		  }
		else if(ttype == TOK_KW_FLAG_INSERTCLASSNAMEMANGLED)
		  {
		    tok.init(ttype, firstloc, 0);
		    return 0;
		  }
		// else not defined..fall thru
	      }
	  }
      }
    //else not a flag

    std::ostringstream errmsg;
    errmsg << "Weird Lex! <" << aname;
    errmsg << "> does not precede a valid flag keyword";
    brtn = m_state.m_pool.getIndexForDataString(errmsg.str());
    return brtn;
  } //makeSubstituteToken

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

    u32 idx = 0;
    u32 rtn = formatUserString(astring, idx);
    if(rtn == 0)
      tok.init(TOK_DQUOTED_STRING,firstloc,idx);
    return rtn; // == 0 == ok
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
	    if(d >= 0)
	      {
		switch(d)
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
		    abyte = '"'; //double quote 34
		    break;
		  case '\'':
		    abyte = '\''; //single quote 39
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
		  case -1:
		    m_state.abortShouldntGetHere(); //EOF
		    break;
		  default:
		    abyte = (u8) d; //save it
		  };
	      }
	    else
	      {
		if( d == -1) unread();
		std::ostringstream errmsg;
		errmsg << "Lexer could not complete last byte in quoted string";
		return m_state.m_pool.getIndexForDataString(errmsg.str());
	      }
	  }
	else
	  abyte = (u8) c; //save literally
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
    u32 runningcount = 0;
    s32 c;
    while((runningcount < 3) && (c = m_SS.read()) >= 0)
      {
	if(c >= '0' && c < '8')
	  {
	    runningtotal = runningtotal * 8 + (c - '0');
	    runningcount++;
	  }
	else
	  {
	    unread(); //t3955
	    break;
	  }
      }

    if((c < 0) || (runningcount == 0))
      {
	std::ostringstream errmsg;
	errmsg << "Lexer could not complete formatting an octal constant";
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }

    if(runningtotal <= MAX_NUMERICSTRING_LENGTH)
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
    // where \xhh is one or two hexadecimal digits (0...9, a...f, A...F).
    u32 runningtotal = 0;
    u32 runningcount = 0;
    s32 c;
    while((runningcount < 2u) && (c = m_SS.read()) >= 0)
      {
	s32 cnum = -1;
	switch(c)
	  {
	  case 'a':
	  case 'A':
	    cnum = 10;
	    runningcount++;
	    break;
	  case 'b':
	  case 'B':
	    cnum = 11;
	    runningcount++;
	    break;
	  case 'c':
	  case 'C':
	    cnum = 12;
	    runningcount++;
	    break;
	  case 'd':
	  case 'D':
	    cnum = 13;
	    runningcount++;
	    break;
	  case 'e':
	  case 'E':
	    cnum = 14;
	    runningcount++;
	    break;
	  case 'f':
	  case 'F':
	    cnum = 15;
	    runningcount++;
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
	    runningcount++;
	    break;
	  default:
	    {
	      unread();
	      break; //from switch
	    }
	  };
	if(cnum == -1)
	  break; //out of whileloop, cnum unset (t3424)
	runningtotal = runningtotal * 16 + cnum;
      } //while

    if((c < 0) || (runningcount == 0))
      {
	std::ostringstream errmsg;
	errmsg << "Lexer could not complete";
	errmsg << " formatting a hex constant";
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }

    if(runningtotal <= MAX_NUMERICSTRING_LENGTH)
      {
	rtn = (u8) runningtotal;
	return 0;
      }

    std::ostringstream errmsg;
    errmsg << "Lexer formatted an invalid hex constant '";
    errmsg << runningtotal << "'";
    return m_state.m_pool.getIndexForDataString(errmsg.str());
  } //formatHexConstant

  u32 Lexer::formatUserString(std::string& astring, u32& usrstridx)
  {
    //format user string; length must be less than 256
    u32 slen = astring.length();
    if(slen > MAX_USERSTRING_LENGTH)
      {
	std::ostringstream errmsg;
	errmsg << "Lexer could not complete double quoted string <" << astring;
	errmsg << ">; Must be less than " << MAX_USERSTRING_LENGTH + 1;
	errmsg << " length, not " << slen;
	return m_state.m_pool.getIndexForDataString(errmsg.str());
      }

    std::ostringstream newstr;
    if(slen == 0)
      newstr << (u8) 0;
    else
      newstr << (u8) slen << astring << (u8) 0; //slen doesn't include itself or terminating byte; see StringPoolUser.

    usrstridx = m_state.m_tokenupool.getIndexForDataString(newstr.str());
    return 0;
  } //formatUserString

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
		s32 f = m_SS.read();
		if(f == '/')
		  {
		    //found end of empty c-style comment; return next byte;
		    return m_SS.read();
		  }
		else
		  {
		    unread(); //wasn't end of empty c-style comment
		    isStructuredComment = true;
		    return makeStructuredCommentToken(rtnTok);
		  }
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
		  {
		    unread(); //to re-read; in case d is *, for example
		    scstr.push_back(c); //dont lose c's *
		  }
	      }
	    else
	      unread(); //to re-read as c, d error or eof
	  } // c is not *, get the next byte
	else
	  scstr.push_back(c);
      } //end while

    //no proper end to structured comment, make error token instead
    if(c < 0)
      {
	u32 idx = m_state.m_pool.getIndexForDataString("Malformed Structured Comment");
	tok.init(TOK_ERROR_LOWLEVEL, firstloc, idx); //t41404
      }
    return c; //to unread..
  } //makeStructuredCommentToken

  TokenType Lexer::getTokenTypeFromString(std::string str)
  {
    return Token::getTokenTypeFromString(str.c_str());
  }

} //end MFM
