/*                                              -*- mode:C++ -*-
  EmergentBoilerPlate.h Early test harness
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

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
  \file EmergentBoilerPlate.h Early test harness
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef EMERGENTBOILERPLATE_H
#define EMERGENTBOILERPLATE_H

namespace MFM {

//c++ -g2 -Wall -ansi -pedantic quark-method.cpp -o quark-method; ./quark-method

//itypes.h
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;

typedef unsigned long u64;
typedef signed long s64;

  //include VD.h
  struct VD {
    enum Type {
      INVALID=0,
      U32,
      S32,
      BOOL,
      UNARY,
      BITS,
      TYPE_COUNT
    };
  };

//Fail.h
#include <stdlib.h> // For abort()
#define FAIL() abort()

//BitVector.h
#include <string.h> //for memcpy
template <u32 S>
struct BitVector {
  enum { BPW = 32 };
  enum { SIZE = S };
  enum { WORDS = (SIZE + BPW - 1)/BPW };
  u32 m_stg[WORDS];
  BitVector() ;
  BitVector(const u32 * const values); 
  BitVector(const BitVector & other); //copy constructor
  BitVector(const u32 value);
  BitVector(const s32 value);

  u32 Read(u32 pos, u32 len) ;
  void Write(u32 pos, u32 len, u32 val) ;
};

//BitVector.tcc

  inline u32 _GetNOnes32(u32 bitwidth) {
    return (bitwidth >= 32) ? (u32) -1 : (((u32)1)<<bitwidth)-1;
  }

  inline u64 _GetNOnes64(u32 bitwidth) {
    return (bitwidth >= 64) ? (u64) -1L : (((u64)1)<<bitwidth)-1;
  }

  inline static u32 PopCount(const u32 bits) {
    return __builtin_popcount(bits); // GCC
  }

  //To INT:
  // i.e. Unsigned32toInt32
  inline s32 _SignExtend32(u32 val, u32 bitwidth) {
    return ((s32)(val<<(32-bitwidth)))>>(32-bitwidth);
  }

  // i.e. Unsigned64toInt64
  inline s64 _SignExtend64(u64 val, u32 bitwidth) {
    return ((s64)(val<<(64-bitwidth)))>>(64-bitwidth);
  }

  inline s32 _Int32ToInt32(s32 val, u32 bitwidth) {
    return val;
  }

  inline s64 _Int64ToInt64(s64 val, u32 bitwidth) {
    return val;
  }

  inline s32 _Bool32ToInt32(u32 val, u32 bitwidth) {
    s32 count1s = PopCount(val);
    if(count1s > (s32) (bitwidth - count1s))
      return 1;
    return 0;
  }

  inline s64 _Bool64ToInt64(u64 val, u32 bitwidth) {
    s32 count1s = PopCount(val);
    if(count1s > (s32) (bitwidth - count1s))
      return 1;
    return 0;
  }

  inline s32 _Unary32ToInt32(u32 val, u32 bitwidth) {
    return PopCount(val);
  }

  inline s64 _Unary64ToInt64(u64 val, u32 bitwidth) {
    return (s64) PopCount(val);
  }

  inline s32 _Bits32ToInt32(u32 val, u32 bitwidth) {
    return val;
  }

  inline s64 _Bits64ToInt64(u64 val, u32 bitwidth) {
    return val;
  }

  //To BOOL:

  inline bool _Int32ToBool(s32 val, u32 bitwidth) {
    return (val != 0);
  }

  inline bool _Int64ToBool(s64 val, u32 bitwidth) {
    return (val != 0);
  }

  inline bool _Unsigned32ToBool(u32 val, u32 bitwidth) {
    return (val != 0);
  }

  inline bool _Unsigned64ToBool(u64 val, u32 bitwidth) {
    return (val != 0);
  }

  inline bool _Unary32ToBool(u32 val, u32 bitwidth) {
    return (val != 0);
  }

  inline bool _Unary64ToBool(u64 val, u32 bitwidth) {
    return (val != 0);
  }

  inline bool _Bool32ToBool(u32 val, u32 bitwidth) {
    s32 count1s = PopCount(val);
    return (count1s > (s32) (bitwidth - count1s));  // == when even number bits is ignored (warning at def)
  }

  inline bool _Bool64ToBool(u64 val, u32 bitwidth) {
    s64 count1s = PopCount(val);
    return (count1s > (s64) (bitwidth - count1s));  // == when even number bits is ignored (warning at def)
  }

  inline bool _Bits32ToBool(u32 val, u32 bitwidth) {
    return (val != 0);
  }

  inline bool _Bits64ToBool(u64 val, u32 bitwidth) {
    return (val != 0);
  }

  //To UNSIGNED:
  inline u32 _Int32ToUnsigned32(s32 val, u32 bitwidth) {
    return ((u32) val);
  }

  inline u64 _Int64ToUnsigned64(s64 val, u32 bitwidth) {
    return ((u64) val);
  }


  inline u32 _ShiftToBitNumber32(u32 value, u32 bitpos) {
    return value<<bitpos;
  }

  inline u64 _ShiftToBitNumber64(u32 value, u32 bitpos) {
    return ((u64) value)<<bitpos;
  }

  inline u32 _ShiftFromBitNumber32(u32 value, u32 bitpos) {
    return value>>bitpos;
  }

  inline u64 _ShiftFromBitNumber64(u64 value, u32 bitpos) {
    return value>>bitpos;
  }

  inline u32 _GetMask32(u32 bitpos, u32 bitwidth) {
    return _ShiftToBitNumber32(_GetNOnes32(bitwidth),bitpos);
  }

  inline u64 _GetMask64(u32 bitpos, u32 bitwidth) {
    return _ShiftToBitNumber64(_GetNOnes64(bitwidth),bitpos);
  }

  inline u32  _ExtractField32(u32 val, u32 bitpos,u32 bitwidth) {
    return _ShiftFromBitNumber32(val,bitpos)&_GetNOnes32(bitwidth);
  }

  inline u32  _ExtractUint32(u32 val, u32 bitpos,u32 bitwidth) {
    return _ExtractField32(val,bitpos,bitwidth);
  }

  inline s32  _ExtractSint32(u32 val, u32 bitpos,u32 bitwidth) {
    return _SignExtend32(_ExtractField32(val,bitpos,bitwidth),bitwidth);
  }

  inline u32 _getParity32(u32 v) {
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
  }

  // v must be <= 0x7fffffff
  inline u32 _getNextPowerOf2(u32 v) {
    v |= v >> 16;
    v |= v >> 8;
    v |= v >> 4;
    v |= v >> 2;
    v |= v >> 1;
    return v+1;
  }

template<u32 S>
BitVector<S>::BitVector() {
  for (u32 i = 0; i < WORDS; ++i) {
    m_stg[i] = 0;
  }
}
  
  template <u32 S>
  BitVector<S>::BitVector(const u32 * const values)
  {
    memcpy(m_stg,values,sizeof(m_stg));
  }

  template <u32 S>
  BitVector<S>::BitVector(const BitVector & other)
  {
    memcpy(m_stg,other.m_stg,sizeof(m_stg));
  }
  
  template <u32 S>
  BitVector<S>::BitVector(const u32 value)
  {
    m_stg[WORDS-1] = value;
  }

  template <u32 S>
  BitVector<S>::BitVector(const s32 value)
  {
    m_stg[WORDS-1] = (u32) value;
  }

  template<u32 S>
u32 BitVector<S>::Read(u32 pos, u32 len)
{
  u32 widx = pos/BPW;
  u32 bidx = pos%BPW;
  u32 eidx = bidx + len;
  if (len > BPW)
    FAIL(); // This stub doesn't even handle 32 bit fields
  if (eidx > BPW)
    FAIL(); // This stub doesn't handle fields crossing word boundaries
  u32 mask = (1<<len)-1;
  if(len==BPW) mask = -1;
  return (m_stg[widx]>>(BPW-eidx)) & mask;
}

template<u32 S>
void BitVector<S>::Write(u32 pos, u32 len, u32 val)
{
  u32 widx = pos/BPW;
  u32 bidx = pos%BPW;
  u32 eidx = bidx + len;
  if (len > BPW)
    FAIL(); // This stub doesn't even handle 32 bit fields
  if (eidx > BPW)
    FAIL(); // This stub doesn't handle fields crossing word boundaries
  u32 lshift = BPW - eidx;
  u32 mask = ((1<<len)-1)<<lshift;
  if(len==BPW) mask = -1;
  u32 mval = (val<<lshift) & mask;
  m_stg[widx] = (m_stg[widx] & ~mask) | mval;
}


//BitField.h
  template <class BV, u32 VDTYPE, u32 LEN, u32 POS>
  struct BitField {
    static u32 Read(BV & bv) ;
    static void Write(BV & bv, u32 val) ;
  };
  
//BitField.tcc
template <class BV, u32 VDTYPE, u32 LEN, u32 POS>
u32 BitField<BV,VDTYPE,LEN,POS>::Read(BV & bv)
{
  return bv.Read(POS,LEN);
}

template <class BV, u32 VDTYPE, u32 LEN, u32 POS>
void BitField<BV,VDTYPE,LEN,POS>::Write(BV & bv, u32 val)
{
  bv.Write(POS,LEN,val);
}

//AtomicParameterType.h
//#define AtomicParameterType BitField
template <class CC, u32 VDTYPE, u32 LEN, u32 POS>
struct AtomicParameterType
{
  typedef typename CC::PARAM_CONFIG P;
  typedef typename CC::ATOM_TYPE T;
  enum { BPA = P::BITS_PER_ATOM };

  static u32 Read(BitVector<BPA> & bv){ return BitField<BitVector<BPA>, VDTYPE, LEN, POS>::Read(bv); }
  static void Write(BitVector<BPA> & bv, u32 val) { BitField<BitVector<BPA>, VDTYPE, LEN, POS>::Write(bv, val); }
};


//Atom.h
template <class CC>
struct Atom
{
  typedef typename CC::PARAM_CONFIG P;
  typedef typename CC::ATOM_TYPE T;
  enum { BPA = P::BITS_PER_ATOM };
  BitVector<BPA> m_stg;
  BitVector<BPA> & GetBits() { return m_stg; }
};

//CoreConfig.h
template <class A, class P>
struct CoreConfig
{
  typedef A ATOM_TYPE;
  typedef P PARAM_CONFIG;
};

//ParamConfig.h
template <u32 BPA>
struct ParamConfig
{
  enum { BITS_PER_ATOM = BPA };
};

//P3Atom.h
template <class PC>
struct P3Atom : Atom<CoreConfig <P3Atom<PC>, PC> > {
};

//Element.h
template <class CC>
struct Element
{
  typedef typename CC::ATOM_TYPE T;
  typedef typename CC::PARAM_CONFIG P;
  enum { BPA = P::BITS_PER_ATOM };
  typedef BitVector<BPA> BV;
  typedef BitField<BV,VD::U32,0,16> BFType;

  bool m_hasType;

  Element() : m_hasType(false)
  {
  }

  void SetType(u16 type)
  {
    if (m_hasType)
      FAIL();
    BFType::Write(m_defaultAtom.GetBits(), type);
    m_hasType = true;
  }

  T m_defaultAtom;

  T & GetDefaultAtom()
  {
    if (!m_hasType)
      FAIL();
    return m_defaultAtom;
  }
};

} //MFM

#endif /* EMERGENTBOILERPLATE_H */
