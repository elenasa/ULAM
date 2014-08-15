/**                                        -*- mode:C++ -*-
 * Preparser.h -  Pre-parses Use/Load Tokens for ULAM
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
  \file Preparser.h -  Pre-parses Use/Load Tokens for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef PREPARSER_H
#define PREPARSER_H

#include <string>
#include <vector>
#include <map>
#include "itype.h"
#include "Tokenizer.h"
#include "SourceStream.h"

namespace MFM{
  /** 
      Preparser wrapper class for tokenizer's (e.g. Lexer)
   */
  class Preparser : public Tokenizer
  {
  public:
    Preparser(Tokenizer * izer, CompilerState & state);
    ~Preparser();
      
    virtual bool push(std::string filename, bool onlyOnce = true);

    virtual bool getNextToken(Token & returnTok);
    
  private:
    CompilerState & m_state;

    Tokenizer * m_tokenizer;
    
    bool preparseKeywordUse(Token & tok);
    bool preparseKeywordLoad(Token & tok);
    bool preparsePackageName(std::string & pStr);

  };
  
}

#endif //end PREPARSER_H
