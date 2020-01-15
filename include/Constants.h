/**                                        -*- mode:C++ -*-
 * Constants.h Useful common constants for ULAM
 *
 * Copyright (C) 2014-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2018 Ackleyshack LLC.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2018 All rights reserved.
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

#define XY(a,b,c,d) a,

  enum ULAMTYPE
  {
#include "UlamType.inc"
    LASTTYPE
  };
#undef XY

#ifndef PtrAbs
#define PtrAbs (Ptr + 4)
#endif //PtrAbs

#ifndef UAtomRef
#define UAtomRef (Ptr + 5)
#endif //UAtomRef

#ifndef ASCII
#define ASCII (Ptr + 6)
#endif //ASCII

  //Linux FILENAME_MAX is 255; Eight (_Types.h) is our longest extension.
#ifndef MAX_FILENAME_LENGTH
#define MAX_FILENAME_LENGTH (255 - 8)
#endif //MAX_FILENAME_LENGTH

#ifndef MAX_IDENTIFIER_LENGTH
#define MAX_IDENTIFIER_LENGTH (255)
#endif //MAX_IDENTIFIER_LENGTH

#ifndef MAX_USERSTRING_LENGTH
#define MAX_USERSTRING_LENGTH (255)
#endif //MAX_USERSTRING_LENGTH

#ifndef MAX_NUMERICSTRING_LENGTH
#define MAX_NUMERICSTRING_LENGTH (255)
#endif //MAX_NUMERICSTRING_LENGTH

#ifndef ELE_TYPE
#define ELE_TYPE u16
#endif

  enum ULAMCLASSTYPE { UC_UNSEEN, UC_QUARK, UC_ELEMENT, UC_TRANSIENT, UC_NOTACLASS, UC_LOCALSFILESCOPE, UC_ATOM, UC_ERROR};

  // see TABLE_SIZE in MFM UlamClassRegistry.h for max registry number.
  enum { UNINITTED_REGISTRY_NUMBER = U32_MAX, MAX_REGISTRY_NUMBER = 1000 };

  // see TABLE_SIZE in MFM Element.h for max element type (reserved for empty element).
  enum { UNDEFINED_ELEMENT_TYPE = 0, EMPTY_ELEMENT_TYPE = U16_MAX, MAX_ELEMENT_TYPE = U16_MAX, ELEMENT_TYPE_BITS = 16 };

  enum SYMBOLTYPEFLAG { STF_NEEDSATYPE = -2, STF_FUNCPARAMETER = -1, STF_DATAMEMBER = 0,  STF_FUNCLOCALVAR = 1, STF_FUNCARGUMENT = 2, STF_CLASSINHERITANCE = 3, STF_CLASSPARAMETER = 4, STF_FUNCLOCALREF = 5, STF_FUNCLOCALCONSTREF = 6 };

  enum ULAMTYPECOMPARERESULTS { UTIC_DONTKNOW = -1, UTIC_NOTSAME = 0, UTIC_SAME = 1, UTIC_MUSTBESAME = 2};

  enum FORECAST { CAST_BAD = 0, CAST_CLEAR, CAST_HAZY};

  enum ALT { ALT_NOT = 0, ALT_AS, ALT_REF, ALT_ARRAYITEM, ALT_CONSTREF}; //autolocaltype

  enum STORAGE { IMMEDIATE = 0, EVENTWINDOW, STACK, EVALRETURN, UNPACKEDSTRUCT, CNSTSTACK};

  /**
      TMPSTORAGE: in uvpass, primarily used to name tmp variables, also to test for tmpbitval and tmpautoref, immediate types, that could need write/read(); terminal is a number; the rest are based on UlamType* and total bitsize (e.g. transients, element, atom are virtually special cases)
   */
  enum TMPSTORAGE { TERMINAL = 0, TMPREGISTER, TMPBITVAL, TMPAUTOREF, TMPTATOM, TMPATOMBS, TMPTBV, TMPARRAYIDX };

  enum PACKFIT { UNPACKED = 0, PACKED, PACKEDLOADABLE};

#define WritePacked(p) (p == PACKED || p == PACKEDLOADABLE)

  enum TBOOL { TBOOL_FALSE = 0, TBOOL_TRUE, TBOOL_HAZY};

  typedef  bool __attribute__((unused)) AssertBool;

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

#ifndef MAXBITSPERTRANSIENT
#define MAXBITSPERTRANSIENT (8192)
#endif //MAXBITSPERTRANSIENT

#ifndef ARRAY_LEN8K
#define ARRAY_LEN8K (MAXBITSPERTRANSIENT / MAXBITSPERINT)
#endif //ARRAY_LEN8K

#ifndef UNRELIABLEPOS
#define UNRELIABLEPOS (9999)
#endif //UNRELIABLEPOS

#ifndef MAXBITSPERINT
#define MAXBITSPERINT (32)
#endif //MAXBITSPERINT

#ifndef MAXBITSPERLONG
#define MAXBITSPERLONG (64)
#endif //MAXBITSPERLONG

#ifndef BITSPERBOOL
#define BITSPERBOOL (1)
#endif //BITSPERBOOL

#ifndef MAXBITSPERSTRING
#define MAXBITSPERSTRING (255)
#endif //MAXBITSPERSTRING

#ifndef MAXBITSPERASCIIBYTE
#define MAXBITSPERASCIIBYTE (8)
#endif //MAXBITSPERASCIIBYTE

#ifndef REGNUMBITS
#define REGNUMBITS (0)
#endif //REGNUMBITS

#ifndef STRINGIDXBITS
#define STRINGIDXBITS (32)
#endif //STRINGIDXBITS

#ifndef STRINGIDXMASK
#define STRINGIDXMASK (U32_MAX)
#endif //STRINGIDXMASK

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
      hold the bit size argument l (no longer required since exact BitVector uses number of u32's) */
#define calcWordSizeLong(l) ((l / MAXBITSPERLONG) * MAXBITSPERLONG + ( (l % MAXBITSPERLONG) > 0 ? MAXBITSPERLONG : 0))

#define calcBitsizeSignedMax(l) (l == MAXBITSPERINT ? S32_MAX : _GetNOnes31(l - 1))
#define calcBitsizeSignedMin(l) (l == MAXBITSPERINT ? S32_MIN : _SignExtend32(_GetNOnes31(l - 1) + 1, l))

#define calcBitsizeUnsignedMax(l) (l == MAXBITSPERINT ? U32_MAX : _GetNOnes31(l))
#define calcBitsizeUnsignedMin(l) (0)

#define calcBitsizeSignedMaxLong(l) (l == MAXBITSPERLONG ? S64_MAX : _GetNOnes63(l - 1))
#define calcBitsizeSignedMinLong(l) (l == MAXBITSPERLONG ? S64_MIN : _SignExtend64(_GetNOnes63(l - 1) + 1, l))

#define calcBitsizeUnsignedMaxLong(l) (l == MAXBITSPERLONG ? U64_MAX : _GetNOnes63(l))
#define calcBitsizeUnsignedMinLong(l) (0)

#define WSUBDIR true

} //MFM

#endif //CONSTANTS_H
