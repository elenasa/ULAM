/**                                        -*- mode:C++ -*-
 * UVPass.h -  Basic handling of Passing Tmp Variables for ULAM Code Gen
 *
 * Copyright (C) 2016-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2016-2019 Ackleyshack LLC.
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
  \file UVPass.h -  Basic handling of Passing Tmp Variables for ULAM Code Gen
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2016-2019 All rights reserved.
  \gpl
*/


#ifndef UVPASS_H
#define UVPASS_H

#include "itype.h"
#include "Constants.h"

namespace MFM{

  class CompilerState; //forward

  class UVPass
  {
  public:

    UVPass(); //requires init to avoid Null ptr for type
    ~UVPass();

  //for backward compatibility, applydelta is false
    static UVPass makePass(u32 varnum, TMPSTORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos, u32 id);

    // overload for code gen to "pad" with symbol id, o.w. zero
    static UVPass makePass(u32 varnum, TMPSTORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos, bool applydelta, u32 id);


    PACKFIT isTargetPacked() const; // Pass only

    void setPassStorage(TMPSTORAGE s);

    TMPSTORAGE getPassStorage() const;

    void setPassVarNum(s32 s); //was slot index

    s32 getPassVarNum() const;

    void setPassPos(u32 pos);

    void setPassPosForced(u32 pos);

    void setPassPosForElementType(u32 pos, CompilerState& state);

    u32 getPassPos() const;

    u32 getPassLen() const;

    bool getPassApplyDelta() const;

    void setPassApplyDelta(bool apply);

    UTI getPassTargetType() const;

    void setPassTargetType(UTI type);

    u32 getPassNameId() const;

    void setPassNameId(u32 id);

    const std::string getTmpVarAsString(CompilerState & state) const;

  private:
    u32 m_varNum; //was slotIndex;
    u32 m_posInStorage;
    bool m_applyDelta; //for code gen, relpos of dm within virtual funcs
    u32 m_bitlenInStorage;
    u8  m_storagetype; //TMPSTORAGE
    u8  m_packed; //PACKFIT
    UTI m_targetType;
    u32 m_nameid; //for code gen
  };

} //MFM

#endif //end UVPASS_H
