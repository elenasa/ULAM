/**                                        -*- mode:C++ -*-
 * Preparser.h -  Pre-parses Use/Load Tokens for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
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

    virtual u32 push(std::string filename, bool onlyOnce = true);

    /** returns Ulam version of current filename from underlying sourcestream; 0 is unknown */
    virtual u32 getFileUlamVersion() const;

    /** passes through Ulam version of current filename to underlying sourcestream */
    virtual void setFileUlamVersion(u32 ver);

    virtual bool getNextToken(Token & returnTok);

  private:
    CompilerState & m_state;

    Tokenizer * m_tokenizer;

    bool preparseKeywordUse(Token & tok);
    bool preparseKeywordLoad(Token & tok);
    bool preparsePackageName(std::string & pStr);
    bool preparseFileName(std::string & pStr);
    bool preparseKeywordUlam(Token & tok);

    /** returns false if file ulam version is greater than current compiler version; o.w. true */
    bool checkFileUlamVersion(u32 ver);
  };

}

#endif //end PREPARSER_H
