/**                                        -*- mode:C++ -*-
 * Lexer.h - Basic Lexical handling for ULAM
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
  \file Lexer.h - Basic Lexical handling for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>
#include "itype.h"
#include "Tokenizer.h"
#include "SourceStream.h"
#include "CompilerState.h"

namespace MFM{
  
  class Lexer : public Tokenizer
  {
  public:
    Lexer(SourceStream & ss, CompilerState& state);
    ~Lexer();
    
    virtual bool push(std::string filename, bool onlyOnce = true);

    /** returns Ulam version of current filename from underlying sourcestream; 0 is unknown */
    virtual u32 getFileUlamVersion() const;
    
    /** passes through Ulam version of current filename to underlying sourcestream */
    virtual void setFileUlamVersion(u32 ver);

    virtual const std::string getPathFromLocator(Locator& loc);  //calls m_SS      

    virtual bool getNextToken(Token & returnTok);

  private:
    CompilerState & m_state;
    
    SourceStream & m_SS;
    
    TokenType getTokenTypeFromString(std::string str);  //< returns TOK_LAST_ONE if not found
    
    bool makeWordToken(std::string& aname, Token & tok);
    bool makeNumberToken(std::string& anumber, Token & tok);
    bool makeOperatorToken(std::string& astring, Token & tok);
    
    bool makeDoubleQuoteToken(std::string& astring, Token & tok);
    bool makeSingleQuoteToken(std::string& astring, Token & tok);
    
    s32 eatComment();
    s32 eatBlockComment();
    s32 eatLineComment();

    /** current byte to be read again, request to underlying sourcestream */
    void unread();

  };
  
}

#endif //end LEXER_H
