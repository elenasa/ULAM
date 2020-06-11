/**                                        -*- mode:C++ -*-
 * File.h - Basic File handling for ULAM
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
  \file File.h - Basic File handling for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/

#ifndef FILE_H
#define FILE_H

#include "itype.h"

namespace MFM
{

  class File
  {
  public:

    File();

    /** The File destructor should automatically invoke close() */
    virtual ~File();

    /** Alternate to read, in combination, peek and consume */
    s32 peek();

    s32 consume();


    /** Return the next byte of the file, as a non-negative int.  If EOF is
	encountered, read() returns -1.  If this File is not open, or is
	not open for reading, or some other I/O problem occurs, read()
	returns a negative number less than -1 according to some scheme you
	devise.
    */
    virtual s32 read() = 0;


    /** Write the bottom 8 bits of ch to this File, and return 0 if
     successful.  If this File is not open, or is not open for output,
     or some other I/O problem occurs, write() returns a negative number
     according to some scheme you devise.
    */
    virtual s32 write(s32 ch) = 0;

    /** Convenience write, calls subclass write; streamlines testing
	of this method for multiple File subclasses per DHA; returns 0
	if successful, o.w. the return from write(int)
    */
    s32 write(const char * data);

    /** Convenience write for decimal numbers, calls subclass write;
	streamlines testing of this method for multiple File
	subclasses per DHA; returns 0 if successful, o.w. the return
	from write(int)
    */
    s32 write_decimal(const s32 data);
    s32 write_decimal_long(const s64 data);
    s32 write_decimal_unsigned(const u32 data);
    s32 write_decimal_unsignedlong(const u64 data);

    /** Convenience write for hexadecimal numbers, calls subclass write;
	returns 0 if successful, o.w. the return from write(int)
    */
    s32 write_hexadecimal(const u32 data);

    /** Convenience write for newlines to document source of generated code */
    s32 write_tagged_end(const char * filename, s32 lineno);

    /** Attempt to close this File, and return 0 if successful.  If this
     File is has already been closed, or some I/O problem occurs,
     close() returns a negative number according to some scheme you
     devise.  Except for the possibly changing return value, it should
     be harmless to call close() on an already-closed file.
    */
    virtual s32 close() = 0;

  private:

    s32 m_lastRead;
    bool m_haveLastRead;  // true when m_lastRead is valid
  };
}

#endif  /* FILE_H */
