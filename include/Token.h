/**                                        -*- mode:C++ -*-
 * Token.h -  Basic handling of Tokens for ULAM
 *
 * Copyright (C) 2014-2020 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2020 Ackleyshack LLC.
 * Copyright (C) 2020 The Living Computation Foundation.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2020 All rights reserved.
  \gpl
*/


#ifndef TOKEN_H
#define TOKEN_H

#include "Locator.h"
#include "itype.h"
#include "File.h"
#include <string>

namespace MFM{

  enum SpecialTokenWork { TOKSP_UNCLEAR=0, TOKSP_KEYWORD, TOKSP_TYPEKEYWORD, TOKSP_CTLKEYWORD, TOKSP_FLAGKEYWORD, TOKSP_OPERATORKEYWORD, TOKSP_SINGLE, TOKSP_COMMENT, TOKSP_DQUOTE, TOKSP_SQUOTE, TOKSP_HASDATA, TOKSP_DEPRECATED, TOKSP_ERROR};

  enum OperatorOverloadableFlag { OPOL_NOT, OPOL_IS };

#define XX(a,b,c,d) TOK_##a,

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

    static void initTokenMap(CompilerState & state);

    void init(TokenType t, Locator l, u32 d);

    u32 getTokenStringId() const;
    u32 getTokenEnumNameId();

    const char * getTokenString();
    const char * getTokenEnumName();

    const std::string & getTokenStringFromPool(CompilerState * state) const;
    static const std::string & getTokenAsStringFromPool(TokenType ttype, CompilerState * state);
    const std::string & getTokenEnumNameFromPool(CompilerState * state) const;

    static SpecialTokenWork getSpecialTokenWork(TokenType ttype);
    static OperatorOverloadableFlag getTokenOperatorOverloadableFlag(TokenType ttype);
    static TokenType getTokenTypeFromString(const char * aname);

    /**
	ulam says an identifier is a Type when it starts with a capital letter
	or is predefined (e.g. Int, Bool, etc).
    */
    static bool isTokenAType(const Token& tok);
    static bool isUpper(char c);

    void print(File * fp, CompilerState * state);

    bool operator<(const Token & tok2) const;
    Token& operator=(const Token&); //explicit for c++11

    static u32 getOperatorOverloadFullNameId(const Token & tok, CompilerState * state);

    bool isOperatorOverloadIdentToken(CompilerState * state) const;
    u32 getUlamNameIdForOperatorOverloadToken(CompilerState * state) const;

  private:

    static const std::string & getOperatorHexName(const Token & tok, CompilerState * state);
    static const std::string & getOperatorHexNameFromString(const std::string & opname, CompilerState * state);

  };
}

#endif  //end TOKEN_H
