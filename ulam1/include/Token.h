/**                                        -*- mode:C++ -*-
 * Token.h -  Basic handling of Tokens for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
 *
 * This file is part of the ULAM programming language compilation system.
 *
 * The ULAM programming language compilation system is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The ULAM programming language compilation system is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the ULAM programming language compilation system
 * software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

/**
  \file Token.h -  Basic handling of Tokens for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef TOKEN_H
#define TOKEN_H

#include "Locator.h"
#include "itype.h"
#include "File.h"


namespace MFM{

  enum SpecialTokenWork { TOKSP_UNCLEAR=0, TOKSP_KEYWORD, TOKSP_TYPEKEYWORD, TOKSP_CTLKEYWORD, TOKSP_SINGLE, TOKSP_COMMENT, TOKSP_DQUOTE, TOKSP_SQUOTE, TOKSP_HASDATA, TOKSP_DEPRECATED, TOKSP_ERROR};


#define XX(a,b,c) TOK_##a,

  enum TokenType
  {
#include "Token.inc"
    TOK_LAST_ONE
  };

#undef XX

  class CompilerState; //forward

  struct Token
  {
    TokenType m_type;
    Locator m_locator;  //fileid, lineno, byteno
    u32 m_dataindex;

    Token();

    Token(TokenType t, Locator l, u32 d);

    Token(const Token& tok);

    ~Token();

    void init(TokenType t, Locator l, u32 d);

    const char * getTokenString();
    const char * getTokenEnumName();

    static SpecialTokenWork getSpecialTokenWork(TokenType ttype);
    static TokenType getTokenTypeFromString(const char * aname);
    static const char *  getTokenAsString(TokenType ttype);

    /**
	ulam says an identifier is a Type when it starts with a capital letter
	or is predefined (e.g. Int, Bool, etc).
    */
    static bool isTokenAType(Token tok);
    static bool isUpper(char c);

    void print(File * fp, CompilerState * state);

  };
}

#endif  //end TOKEN_H
