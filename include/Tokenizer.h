/**                                        -*- mode:C++ -*-
 * Tokenizer.h -  Basic returning of Tokens for ULAM
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
  \file Tokenizer.h -  Basic returning of Tokens for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include "Token.h"
#include "CompilerState.h"

namespace MFM{

  class Tokenizer
  {
  public:
    Tokenizer();
    virtual ~Tokenizer();

    /** pass through filename to underlying sourcestream */
    virtual u32 push(std::string filename, bool onlyOnce = true) = 0;


    /** read SourceStream and produce TOKEN: get next byte, skip white
	space, return special characters, identifier token (string of
	letters and digits beginning with a letter), special keywords
	returns keyword tokens, deal with EOF token, comments, quotes, etc.
    */
    virtual bool getNextToken(Token & returnTok) = 0;

    void unreadToken();


    /** returns Ulam version of current filename from underlying sourcestream; 0 is unknown */
    virtual u32 getFileUlamVersion() const = 0;

    /** passes through Ulam version of current filename to underlying sourcestream */
    virtual void setFileUlamVersion(u32 ver) = 0;

  protected:

      Token m_lastToken;
      bool m_haveUnreadToken;

  private:

  };

}

#endif //end TOKENIZER_H
