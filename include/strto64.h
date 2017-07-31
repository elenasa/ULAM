/**                                        -*- mode:C++ -*-
 * strto64.h - 64 bit stro(u)lish basic number conversion
 *
 * Copyright (C) 2016-2017 The Regents of the University of New Mexico.
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
  \file Strto64.h - 64 bit stro(u)lish basic number conversion
  \author David H. Ackley.
  \date (C) 2016-2017 All rights reserved.
  \gpl
*/

#ifndef STRTO64_H
#define STRTO64_H

#include "itype.h"

namespace MFM {

  /**
     Read a number pointed-to by ptr, advancing ptr while reading, and
     using mostly-typical prefix conventions for determining the base
     of the number: '0x' means hexadecimal, '0b' means base 2, '0'
     means octal, and '1'-'9' means decimal.  Alphabetic case is
     always ignored.

     Checks for and skips over leading whitespace, as well as a
     possible single leading '+' or '-', which is applied to the
     result if found.

     Parameters:

      - \c ptr - a reference to a pointer to the characters to be
        converted to a number.  This function advances \c ptr over the
        chars successfully read, leaving ptr pointing at the first
        byte not considered part of the number.

      - \c errMsg - reference to a pointer which this function sets to
        null at the beginning of the conversion attempt.  If a number
        is read successfully, \c errMsg will be null after this
        function returns.  If not, \c errMsg will be pointing to a
        fixed string briefly describing the detected error.  Caller
        must not free() a non-null returned \c errMsg.

      - validBits - a number indicating the range of legal numbers
        this function should accept.  A negative validBits indicates a
        signed range of width |validBits|, otherwise the function
        assumes an unsigned range of validBits.  Only \c validBits
        values of 64 and -64 have been significantly tested, as they
        are used by the convenience routines \c strtou64 and strtos64.

     Return value: On success, the function returns the converted
     result and sets \c errMsg to null.  On error, \c errMsg is set
     non-null and the returned value is undefined, as is the final
     value of \c ptr.  Note the return type is u64 even if the
     returned value is effectively signed.  The convenience routine
     \c strtos64 provides its result as an s64

     \sa strtos64, strtou64

   */
  u64 strto64(const char *& ptr, const char *& errMsg, s32 validBits) ;

  /** Read an Unsigned(64) from ptr.  See \c strto64 for details.

     \sa strtos64, strto64
   */
  inline u64 strtou64(const char *& ptr, const char *& errMsg)
  {
    return strto64(ptr, errMsg, 64);
  }

  /** Read an Int(64) from ptr.  See \c strto64 for details.

     \sa strtou64, strto64
   */
  inline s64 strtos64(const char *& ptr, const char *& errMsg)
  {
    return (s64) strto64(ptr, errMsg, -64);
  }

}

#endif //end STRTO64_H
