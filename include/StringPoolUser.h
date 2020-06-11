/**                                        -*- mode:C++ -*-
 * StringPoolUser.h -  Basic String Pool management for ULAM
 *
 * Copyright (C) 2016-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2016-2017 Ackleyshack LLC.
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
  \file StringPoolUser.h -  Basic String Pool management for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2016-2017 All rights reserved.
  \gpl
*/


#ifndef STRINGPOOLUSER_H
#define STRINGPOOLUSER_H

#include "StringPool.h"
#include "File.h"

namespace MFM
{
  class CompilerState; //forward

  class StringPoolUser : public StringPool
  {
  public:

    StringPoolUser();

    StringPoolUser(const StringPoolUser& spref);

    ~StringPoolUser();


    //format of input string: 1 byte for len, 1 null terminating byte,
    //8-bit clean data (that is, escaped characters use one stored
    //byte; backslash added back upon printing).
    virtual u32 getIndexForDataString(std::string str); //< makes a new entry in maps if nonexistent

    virtual u32 getIndexForNumberAsString(u32 num);

    //converts user string pool index to (formatted) compiler string pool index
    u32 getIndexForDataAsFormattedString(u32 dataindex, CompilerState * state);

    virtual const std::string & getDataAsString(u32 dataindex);

    const std::string & getDataAsFormattedString(u32 dataindex, CompilerState * state);

    u32 getStringLength(u32 dataindex);

    u8 getByteOf(u32 dataindex, u32 offset);

    u32 getUserStringPoolSize();

    void generateUserStringPoolEntries(File * fp, CompilerState * state);

    //used by Preparser for 'load'; returns compiler pool id.
    u32 formatDoubleQuotedFileNameUnquoted(u32 ustrid, CompilerState * state);

  private:

    u32 m_runningIndex; //used during parsing to assign indexes

    u32 formatDoubleQuotedString(const std::string& str, CompilerState * state);

    void writeDoubleQuotedString(File * fp, const std::string& str);
    void writeDblQuotedChar(File * fp, u8 c);
    void writeRawChar(File * fp, u8 c);
    void writeEscaped(File * fp, u8 c);
    void writeOpenCloseDblQuote(File * fp);
    void writeNullByte(File * fp);
  };
}

#endif  /* STRINGPOOLUSER_H */
