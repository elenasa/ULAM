/**                                        -*- mode:C++ -*-
 * UlamValue.h -  Basic handling of UlamValues for ULAM
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
  \file UlamValue.h -  Basic handling of UlamValues for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017,2020 All rights reserved.
  \gpl
*/


#ifndef ULAMVALUE_H
#define ULAMVALUE_H

#include "itype.h"
#include "BitVector.h"
#include "Constants.h"

namespace MFM{

  typedef BitVector<BITSPERATOM> AtomBitVector;

  class CompilerState; //forward

  struct UlamValue
  {
    union UV {

      struct RawAtom {
	u16 m_short;
	UTI m_utypeIdx;
	u8  m_bits[8];  //oops! not 10 anymore
      } m_rawAtom;

      struct PtrValue {
	s16 m_slotIndex;
	UTI m_utypeIdx;
	u8  m_posInAtom;
	s8  m_bitlenInAtom;
	u8  m_storagetype; //STORAGE
	u8  m_packed; //PACKFIT
	u16 m_targetType;
	u16 m_nameid; //for code gen
      } m_ptrValue;

      struct Storage {
	//AtomBitVector m_atom;
	//0-15 UTI, 16-24 errcorr. 25-96 designed by element data members
	u32 m_atom[AtomBitVector::ARRAY_LENGTH];
      } m_storage;

    } m_uv;


    UlamValue(); //requires init to avoid Null ptr for type
    UlamValue(const UlamValue &); //explicit copy ctr needed for c++11
    ~UlamValue();

    void clear();

    void init(UTI utype, u32 v, CompilerState& state);

    // returns Atom with default values set
    static UlamValue makeDefaultAtom(UTI elementType, CompilerState& state);

    // returns cleared Atom with element type set
    static UlamValue makeAtom(UTI elementType);

    // returns cleared Atom
    static UlamValue makeAtom();

    // starts at the end per utype bits
    static UlamValue makeImmediate(UTI utype, u32 v, CompilerState& state);

    static UlamValue makeImmediate(UTI utype, u32 v, s32 len = 32);

    static UlamValue makeImmediateClass(UTI utype, u32 v, s32 len);

    static UlamValue makeImmediateLong(UTI utype, u64 v, CompilerState& state);

    static UlamValue makeImmediateLong(UTI utype, u64 v, s32 len = 64);

    static UlamValue makeImmediateLongClass(UTI utype, u64 v, s32 len);

    // returns a pointer to an UlamValue of type targetType; pos==0 determined from targettype
    static UlamValue makePtr(u32 slot, STORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos = 0);

    // overload for code gen to "pad" with symbol id, o.w. zero
    static UlamValue makePtr(u32 slot, STORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos, u32 id);

    static UlamValue makeScalarPtr(UlamValue arrayPtr, CompilerState& state);

    void checkForAbsolutePtr(const UlamValue fmptr);

    UTI  getUlamValueTypeIdx() const;

    void setUlamValueTypeIdx(UTI utype);

    UTI  getAtomElementTypeIdx();

    void setAtomElementTypeIdx(UTI utype);

    bool isPtr() const;

    bool isPtrAbs() const;

    PACKFIT isTargetPacked(); // Ptr only

    UlamValue getValAt(u32 offset, CompilerState& state) const; // Ptr only, arrays

    void setPtrStorage(STORAGE s);

    STORAGE getPtrStorage();

    void setPtrSlotIndex(s32 s);

    s32 getPtrSlotIndex();

    void setPtrPos(u32 pos);

    u32 getPtrPos();

    void setPtrLen(s32 len);

    s32 getPtrLen();

    UTI getPtrTargetType();

    void setPtrTargetType(UTI type);

    u16 getPtrNameId();

    void setPtrNameId(u32 id);

    bool incrementPtr(CompilerState& state, s32 offset = 1);

    static UlamValue getPackedArrayDataFromAtom(UlamValue p, UlamValue data, CompilerState& state);

    u32 getDataFromAtom(UlamValue p, CompilerState& state) const;

    u32 getDataFromAtom(u32 pos, s32 len) const;

    u64 getDataLongFromAtom(UlamValue p, CompilerState& state) const;

    u64 getDataLongFromAtom(u32 pos, s32 len) const;

    u32 getImmediateData(CompilerState& state) const;

    u32 getImmediateData(s32 len, CompilerState& state) const;

    u32 getImmediateClassData(CompilerState & state) const;
    u32 getImmediateClassData(s32 len) const;

    u64 getImmediateDataLong(CompilerState & state) const;

    u64 getImmediateDataLong(s32 len, CompilerState & state) const;

    u64 getImmediateDataLong(s32 len = 64) const;

    u64 getImmediateClassDataLong(CompilerState & state) const;
    u64 getImmediateClassDataLong(s32 len) const;

    void putDataIntoAtom(UlamValue p, UlamValue data, CompilerState& state);

    // called by putDataIntoAtom when packed array is not 'loadable' in a single integer
    void putPackedArrayDataIntoAtom(UlamValue p, UlamValue data, CompilerState& state);

    u32 getData(u32 pos, s32 len) const;

    u64 getDataLong(u32 pos, s32 len) const;

    void putData(u32 pos, s32 len, u32 data);

    void putDataLong(u32 pos, s32 len, u64 data);

    void putDataBig(u32 pos, s32 len, const BV8K& data);

    UlamValue& operator=(const UlamValue& rhs);

    bool operator==(const UlamValue& rhs);
  };

}

#endif //end ULAMVALUE_H
