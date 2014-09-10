/**                                        -*- mode:C++ -*-
 * UlamValue.h -  Basic handling of UlamValues for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
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
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef ULAMVALUE_H
#define ULAMVALUE_H

#include "itype.h"
#include "BitVector.h"
#include "Constants.h"
#include "UlamType.h"

namespace MFM{

  typedef BitVector<BITSPERATOM> AtomBitVector;

  class CompilerState; //forward

  struct UlamValue
  {
    union UV {

      struct RawAtom {
	u16 m_short;
	UTI m_utypeIdx;
	u8  m_bits[10];
      } m_rawAtom;

      /*
      struct IntValue {
	UTI m_utypeIdx;
	u16 m_pad;
	u32 m_val;
	u32 m_pad2;
	}*/

      struct PtrValue {
	s16 m_slotIndex;
	UTI m_utypeIdx;
	u8  m_posInAtom;
	u8  m_bitlenInAtom;
	u8  m_storage; //STORAGE
	u8  m_packed;  //bool
	u16 m_targetType;
	u16 m_pad;
      } m_ptrValue;

      struct Storage {
	AtomBitVector m_atom;  //0-15 UTI, 16-24 errcorr. 25-96 designed by element data members
      } m_storage;

    } m_uv;

    
    UlamValue();   //requires init to avoid Null ptr for type
    ~UlamValue();

    void init(UTI utype, u32 v, CompilerState& state);

    // returns cleared Atom with element type set
    static UlamValue makeAtom(UTI elementType);

    // returns cleared Atom
    static UlamValue makeAtom();

    // starts at the end per utype bits
    static UlamValue makeImmediate(UTI utype, u32 v, CompilerState& state);

    static UlamValue makeImmediate(UTI utype, u32 v, u32 len = 32);

    // returns a pointer to an UlamValue of type targetType; pos==0 determined from targettype
    static UlamValue makePtr(u32 slot, STORAGE storage, UTI targetType, bool packed, CompilerState& state, u32 pos = 0);

    static UlamValue makeScalarPtr(UlamValue arrayPtr, CompilerState& state);

    UTI  getUlamValueTypeIdx() const;

    void setUlamValueTypeIdx(UTI utype);

    UTI  getAtomElementTypeIdx();

    void setAtomElementTypeIdx(UTI utype);

    void getUlamValueAsString(char * valstr, CompilerState& state);

    u32 isArraySize(CompilerState& state);

    bool isTargetPacked();             // Ptr only

    UlamValue getValAt(u32 offset, CompilerState& state) const;   // Ptr only, arrays

    STORAGE getPtrStorage();

    void setPtrSlotIndex(s32 s);  

    s32 getPtrSlotIndex();  

    u32 getPtrPos();        

    u32 getPtrLen();        

    UTI getPtrTargetType(); 

    void setPtrTargetType(UTI type);

    void incrementPtr(CompilerState& state, s32 offset = 1);
    
    u32 getImmediateData(CompilerState& state) const;

    u32 getImmediateData(u32 len = 32) const;

    u32 getDataFromAtom(u32 pos, u32 len) const;

    void putDataIntoAtom(UlamValue p, UlamValue data);
    
    u32 getData(u32 pos, u32 len) const;

    void putData(u32 pos, u32 len, u32 data);

    UlamValue& operator=(const UlamValue& rhs);
    bool operator==(const UlamValue& rhs);
  };

}

#endif //end ULAMVALUE_H
