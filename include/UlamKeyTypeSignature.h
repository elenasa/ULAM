/**                                        -*- mode:C++ -*-
 * UlamKeyTypeSignature.h -  Basic handling of UlamType Keys for ULAM
 *
 * Copyright (C) 2014-2017,2020 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017,2020 Ackleyshack LLC.
 * Copyright (C) 2020 The Living Computation Foundation.
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
  \file UlamKeyTypeSignature.h -  Basic handling of UlamType Keys for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017,2020   All rights reserved.
  \gpl
*/


#ifndef ULAMKEYTYPESIGNATURE_H
#define ULAMKEYTYPESIGNATURE_H

#include <string>
#include "itype.h"
#include "Constants.h"

namespace MFM{

  struct CompilerState;  //forward

  struct UlamKeyTypeSignature
  {
    u32 m_typeNameId;
    s32 m_bits;
    s32 m_arraySize;
    UTI m_classInstanceIdx;
    ALT m_referenceType;

    UlamKeyTypeSignature();
    UlamKeyTypeSignature(u32 nameid, s32 bitsize, s32 arraysize=NONARRAYSIZE, ALT reftype = ALT_NOT);
    UlamKeyTypeSignature(u32 nameid, s32 bitsize, s32 arraysize, UTI classinstanceIdx, ALT reftype = ALT_NOT);
    ~UlamKeyTypeSignature();

    void append(UTI cuti);

    const std::string & getUlamKeyTypeSignatureName(CompilerState * state);
    u32 getUlamKeyTypeSignatureNameId();
    s32 getUlamKeyTypeSignatureBitSize();
    s32 getUlamKeyTypeSignatureArraySize();
    UTI getUlamKeyTypeSignatureClassInstanceIdx();
    ALT getUlamKeyTypeSignatureReferenceType();

    const std::string & getUlamKeyTypeSignatureNameAndBitSize(CompilerState * state);
    const std::string & getUlamKeyTypeSignatureNameAndSize(CompilerState * state);

    const std::string & getUlamKeyTypeSignatureAsString(CompilerState * state);
    static const std::string & getUlamKeyTypeSignatureAsString(UlamKeyTypeSignature utk, CompilerState * state);

    bool operator<(const UlamKeyTypeSignature & key2) const;
    bool operator==(const UlamKeyTypeSignature & key2) const;
  };


}

#endif //end ULAMKEYTYPESIGNATURE_H
