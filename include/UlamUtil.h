/*                                              -*- mode:C++ -*-
  UlamUtil.h Utility methods for ulam
  Copyright (C) 2014-2017 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2014-2017 Ackleyshack LLC.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file UlamUtil.h Utility methods for ulam
  \author David H. Ackley.
  \author Elena S. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \lgpl
 */
#ifndef ULAMUTIL_H
#define ULAMUTIL_H

#include "itype.h"
#include <string>

namespace MFM {

  /**
   * Encodes an unsigned integer length in leximited.
   *
   * @param len The number to encode.
   *
   * @return a string containing the leximited encoding of len.
   */
  extern std::string ToLeximitedHeader(u32 len);

  /**
   * Encodes an signed integer in leximited.
   *
   * @param num The number to encode.
   *
   * @return a string containing the leximited encoding of num.
   */
  extern std::string ToLeximitedNumber(s32 num);

  /**
   * Encodes an unsigned integer in leximited.
   *
   * @param num The number to encode.
   *
   * @return a string containing the leximited encoding of num.
   */
  extern std::string ToLeximitedNumber(u32 num);

  /**
   * Encodes an signed long in leximited.
   *
   * @param num The number to encode.
   *
   * @return a string containing the leximited encoding of num.
   */
  extern std::string ToLeximitedNumber64(s64 num);

  /**
   * Encodes an unsigned long in leximited.
   *
   * @param num The number to encode.
   *
   * @return a string containing the leximited encoding of num.
   */
  extern std::string ToLeximitedNumber64(u64 num);

  /**
   * Encode an arbitrary string in leximited.
   *
   * @param str The string to encode.
   *
   * @return a string containing str prefixed with the leximited
   * encoding of its length
   */
  extern std::string ToLeximited(const std::string & str);

  /**
   * Hex escape non-printing bytes of a string.
   *
   * @param source The string to escape
   *
   * @return a string containing source except all non-printing
   * characters and '%'s have been replaced with %XX hex escape
   * sequences
   */
  extern std::string HexEscape(const std::string & source);

  /**
   * Format an s64 in decimal as if by printf "%ld".  Apply .c_str()
   * to the result of this and use "%s" to print it
   *
   * @param data The data to format in decimal
   *
   * @return a string containing the decimal representation
   */
  extern std::string ToSignedDecimal(const s64 data);

  /**
   * Format a u64 in decimal as if by printf "%lu".  Apply .c_str()
   * to the result of this and use "%s" to print it
   *
   * @param data The data to format in decimal
   *
   * @return a string containing the decimal representation
   */
  extern std::string ToUnsignedDecimal(const u64 data);

  /**
   * Filename suffix is .ulam
   *
   * @param data The filename string
   *
   * @return position of suffix in string, o.w. zero if not
   */
  extern u32 FindUlamFilenameSuffix(std::string fnstr);
}

#endif /* ULAMUTIL_H */
