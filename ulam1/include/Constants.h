/**                                        -*- mode:C++ -*-
 * Constants.h Useful common constants for ULAM
 *
 * Copyright (C) 2014-2015 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2015 Ackleyshack LLC.
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
  \file Constants.h Useful common constants for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/

#include "itype.h"

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace MFM {

#ifndef UTI
#define UTI u16
#endif

#ifndef NNO
#define NNO u32
#endif

#ifndef SIZEOFACHAR
#define SIZEOFACHAR 8
#endif

  enum STORAGE { IMMEDIATE = 0, EVENTWINDOW, STACK, EVALRETURN, UNPACKEDSTRUCT, TMPREGISTER, TMPBITVAL, TMPAUTOREF, TMPTATOM};
  enum PACKFIT { UNPACKED = 0, PACKED, PACKEDLOADABLE};

#define WritePacked(p) (p == PACKED || p == PACKEDLOADABLE)

  enum ULAMTYPECOMPARERESULTS { UTIC_DONTKNOW = -1, UTIC_NOTSAME = 0, UTIC_SAME = 1, UTIC_MUSTBESAME = 2};

  enum FORECAST { CAST_BAD = 0, CAST_CLEAR, CAST_HAZY};

  enum ALT { ALT_NOT = 0, ALT_AS, ALT_HAS, ALT_REF, ALT_ARRAYITEM, ALT_CAST, ALT_PTR}; //autolocaltype

  typedef  bool __attribute__((unused)) AssertBool;

  enum TBOOL { TBOOL_FALSE = 0, TBOOL_TRUE, TBOOL_HAZY};

#ifndef BITSPERATOM
#define BITSPERATOM (96)
#endif //BITSPERATOM

#ifndef ATOMFIRSTSTATEBITPOS
#define ATOMFIRSTSTATEBITPOS (25)
#endif //ATOMFIRSTSTATEBITPOS

#ifndef MAXSTATEBITS
#define MAXSTATEBITS (BITSPERATOM - ATOMFIRSTSTATEBITPOS)
#endif //MAXSTATEBITS

#ifndef MAXBITSPERQUARK
#define MAXBITSPERQUARK (32)
#endif //MAXBITSPERQUARK

#ifndef MAXBITSPERINT
#define MAXBITSPERINT (32)
#endif //MAXBITSPERINT

#ifndef MAXBITSPERLONG
#define MAXBITSPERLONG (64)
#endif //MAXBITSPERLONG

#ifndef BITSPERBOOL
#define BITSPERBOOL (1)
#endif //BITSPERBOOL

#ifndef UNKNOWNSIZE
#define UNKNOWNSIZE (-2)
#endif //UNKNOWNSIZE

#ifndef NONARRAYSIZE
#define NONARRAYSIZE (-1)
#endif //NONARRAYSIZE


#ifndef BASE10
#define BASE10 (10)
#endif //BASE10

#ifndef MAX_ITERATIONS
#define MAX_ITERATIONS 99
#endif //MAX_ITERATIONS

#ifndef CYCLEFLAG
#define CYCLEFLAG (-12)
#endif //CYCLEFLAG

#ifndef EMPTYSYMBOLTABLE
#define EMPTYSYMBOLTABLE (-11)
#endif //EMPTYSYMBOLTABLE


#ifndef ULAMTYPE_DEFAULTBITSIZE
#define XY(a,b,c,d) c,

  static const s32 ULAMTYPE_DEFAULTBITSIZE[] = {
#include "UlamType.inc"
  };

#undef XY
#endif //ULAMTYPE_DEFAULTBITSIZE

  /** Number of bits (rounded up to nearest 32 bits) required to
      hold the bit size argument l */
#define calcWordSize(l) ((l / MAXBITSPERINT) * MAXBITSPERINT + ( (l % MAXBITSPERINT) > 0 ? MAXBITSPERINT : 0))

  /** Number of bits (rounded up to nearest 64 bits) required to
      hold the bit size argument l */
#define calcWordSizeLong(l) ((l / MAXBITSPERLONG) * MAXBITSPERLONG + ( (l % MAXBITSPERLONG) > 0 ? MAXBITSPERLONG : 0))

#define calcBitsizeSignedMax(l) (l == MAXBITSPERINT ? S32_MAX : ((1 << (l - 1)) - 1))
#define calcBitsizeSignedMin(l) (l == MAXBITSPERINT ? S32_MIN : _SignExtend32((1 << (l - 1)), l))
  //#define calcBitsizeSignedMin(l) (l == MAXBITSPERINT ? S32_MIN : (1 << (l - 1)))

#define calcBitsizeUnsignedMax(l) (l == MAXBITSPERINT ? U32_MAX : (1u << l) - 1)
#define calcBitsizeUnsignedMin(l) (0)


#define calcBitsizeSignedMaxLong(l) (l == MAXBITSPERLONG ? S64_MAX : ((1 << (l - 1)) - 1))
#define calcBitsizeSignedMinLong(l) (l == MAXBITSPERLONG ? S64_MIN : _SignExtend64((1 << (l - 1)), l))
  //#define calcBitsizeSignedMinLong(l) (l == MAXBITSPERLONG ? S64_MIN : (1 << (l - 1)))

#define calcBitsizeUnsignedMaxLong(l) (l == MAXBITSPERLONG ? U64_MAX : (1u << l) - 1)
#define calcBitsizeUnsignedMinLong(l) (0)

#define WSUBDIR true

} //MFM

#endif //CONSTANTS_H
