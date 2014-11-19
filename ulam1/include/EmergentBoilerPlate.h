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

  //Util.h

  inline static u32 PopCount(const u32 bits) {
    return __builtin_popcount(bits); // GCC
  }

  template <class T>
  inline T MAX(T x, T y) {
    return (x > y) ? x : y;
  }

  template <class T>
  inline T MIN(T x, T y) {
    return (x < y) ? x : y;
  }

  template <class T>
  inline T CLAMP(T min, T max, T val)
  {
    return val < min ? min : (val > max ? max : val);
  }

  //Casts.h


  inline u32 _GetNOnes32(const u32 bitwidth) 
  {
    return (bitwidth >= 32) ? (u32) -1 : (((u32)1)<<bitwidth)-1;
  }

  inline u64 _GetNOnes64(const u32 bitwidth) 
  {
    return (bitwidth >= 64) ? (u64) -1L : (((u64)1)<<bitwidth)-1;
  }


  inline s32 _SignExtend32(u32 val, const u32 bitwidth) 
  {
    return ((bitwidth < 32) ? (((s32)(val<<(32-bitwidth)))>>(32-bitwidth)) : (s32) val);
  }

  inline s64 _SignExtend64(u64 val, const u32 bitwidth) 
  {
    return ((bitwidth < 64) ? (((s64)(val<<(64-bitwidth)))>>(64-bitwidth)) : (s64) val);
  }

  //To INT:
  inline s32 _Int32ToInt32(s32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const s32 maxdestval = _GetNOnes32(destbitwidth-1);  //positive
    const s32 mindestval = ~maxdestval;
    return CLAMP<s32>(mindestval, maxdestval, val);
  }

  inline s64 _Int64ToInt64(s64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const s64 maxdestval = _GetNOnes64(srcbitwidth-1);
    const s64 mindestval = ~maxdestval;
    return CLAMP<s64>(mindestval, maxdestval, val);
  }

  inline s32 _Unsigned32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    u32 maxdestval = _GetNOnes32(destbitwidth - 1);
    return (s32) MIN<u32>(val, maxdestval);
  }

  inline s64 _Unsigned64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    u64 maxdestval = _GetNOnes64(destbitwidth - 1);
    return (s64) MIN<u64>(val, maxdestval);
  }

  inline s32 _Bool32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return (s32) ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline s64 _Bool64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return (s64) ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline s32 _Unary32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return PopCount(val & _GetNOnes32(srcbitwidth));
  }

  inline s64 _Unary64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (s64) PopCount(val & _GetNOnes64(srcbitwidth));
  }

  inline s32 _Bits32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (s32) (val & _GetNOnes32(srcbitwidth)); // no sign extend
  }

  inline s64 _Bits64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (s64) (val & _GetNOnes64(srcbitwidth)); // no sign extend
  }

//To BOOL:
  inline u32 _Bool32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    // == when even number bits is ignored (warning at def)
    return (u32) ((count1s > (s32) (srcbitwidth - count1s)) ? _GetNOnes32(destbitwidth) : 0); 
  }

  inline u32 _Bool64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    // == when even number bits is ignored (warning at def)
    return (u64) ((count1s > (s32) (srcbitwidth - count1s)) ? _GetNOnes64(destbitwidth) : 0); 
  }

  inline u32 _Int32ToBool32(s32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return  (u32) (((val & _GetNOnes32(srcbitwidth)) != 0) ? _GetNOnes32(destbitwidth) : 0);
  }

  inline u64 _Int64ToBool64(s64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return  (u64) (((val & _GetNOnes64(srcbitwidth)) != 0) ? _GetNOnes64(destbitwidth) : 0);
  }

  inline u32 _Unsigned32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return  (u32) (((val & _GetNOnes32(srcbitwidth)) != 0) ? _GetNOnes32(destbitwidth) : 0);
  }

  inline u64 _Unsigned64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return  (u64) (((val & _GetNOnes64(srcbitwidth)) != 0) ? _GetNOnes64(destbitwidth) : 0);
  }

  inline u32 _Unary32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return  (u32) (((val & _GetNOnes32(srcbitwidth)) != 0) ? _GetNOnes32(destbitwidth) : 0);
  }

  inline u64 _Unary64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return  (u64) (((val & _GetNOnes64(srcbitwidth)) != 0) ? _GetNOnes64(destbitwidth) : 0);
  }

  inline u32 _Bits32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (val & _GetNOnes32(MIN<u32>(srcbitwidth, destbitwidth))); //no change to Bit data
  }

  inline u64 _Bits64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (val & _GetNOnes64(MIN<u32>(srcbitwidth, destbitwidth))); //no change to Bit data
  }

  inline bool _Bool32ToCbool(u32 val, const u32 srcbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return (count1s > (s32) (srcbitwidth - count1s));  // == when even number bits is ignored (warning at def)
  }

  inline bool _Bool64ToCbool(u64 val, const u32 srcbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return (count1s > (s32) (srcbitwidth - count1s));  // == when even number bits is ignored (warning at def)
  }

  //To UNSIGNED:
  inline u32 _Int32ToUnsigned32(s32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    if(val <= 0)
      return 0;

    const u32 maxdestval = _GetNOnes32(destbitwidth);
    const u32 mindestval = 0;
    return CLAMP<u32>(mindestval, maxdestval, (u32) val);
  }

  inline u64 _Int64ToUnsigned64(s64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    if(val <= 0)
      return 0;

    const u64 maxdestval = _GetNOnes64(destbitwidth);
    const u64 mindestval = 0;
    return CLAMP<u64>(mindestval, maxdestval, (u64) val);
  }

  inline u32 _Unsigned32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const u32 maxdestval = _GetNOnes32(destbitwidth);
    const u32 mindestval = 0;
    return CLAMP<u32>(mindestval, maxdestval, val);
  }

  inline u64 _Unsigned64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const u64 maxdestval = _GetNOnes64(destbitwidth);
    const u64 mindestval = 0;
    return CLAMP<u64>(mindestval, maxdestval, val);
  }

  inline u32 _Bool32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline u64 _Bool64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline u32 _Unary32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return PopCount(val & _GetNOnes32(srcbitwidth));
  }

  inline u64 _Unary64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return PopCount(val & _GetNOnes64(srcbitwidth));
  }

  inline u32 _Bits32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (val & _GetNOnes32(srcbitwidth));
  }

  inline u64 _Bits64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (val & _GetNOnes64(srcbitwidth)); 
  }

  //To BITS:
  inline u32 _Int32ToBits32(s32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (u32) (val & _GetNOnes32(srcbitwidth));
  }

  inline u64 _Int64ToBits64(s64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (u64) (val & _GetNOnes64(srcbitwidth));
  }

  inline u32 _Unsigned32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return val & _GetNOnes32(srcbitwidth);
  }

  inline u64 _Unsigned64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return val & _GetNOnes64(srcbitwidth);
  }

  inline u32 _Bool32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return val & _GetNOnes32(srcbitwidth);
  }

  inline u64 _Bool64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return val & _GetNOnes64(srcbitwidth);
  }

  inline u32 _Unary32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return val & _GetNOnes32(srcbitwidth);
  }

  inline u64 _Unary64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return val & _GetNOnes64(srcbitwidth);
  }

  inline u32 _Bits32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return val & _GetNOnes32(srcbitwidth);
  }

  inline u64 _Bits64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return val & _GetNOnes64(srcbitwidth);
  }

  //To UNARY:
  inline u32 _Int32ToUnary32(s32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const s32 maxdestval = destbitwidth;
    const s32 mindestval = 0;
    return (u32) _GetNOnes32(CLAMP<s32>(mindestval, maxdestval, val));
  }

  inline u64 _Int64ToUnary64(s64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const s64 maxdestval = destbitwidth;
    const s64 mindestval = 0;
    return (u64) _GetNOnes64(CLAMP<s64>(mindestval, maxdestval, val));
  }

  inline u32 _Unsigned32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const u32 maxdestval = destbitwidth;
    const u32 mindestval = 0;
    return _GetNOnes32(CLAMP<u32>(mindestval, maxdestval, val));
  }

  inline u64 _Unsigned64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const u64 maxdestval = destbitwidth;
    const u64 mindestval = 0;
    return _GetNOnes64(CLAMP<u64>(mindestval, maxdestval, val));
  }

  inline u32 _Bool32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline u64 _Bool64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline u32 _Unary32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const u32 maxdestval = destbitwidth;
    const u32 mindestval = 0;
    u32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return _GetNOnes32(CLAMP<u32>(mindestval, maxdestval, count1s));
  }

  inline u64 _Unary64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    const u32 maxdestval = destbitwidth;
    const u32 mindestval = 0;
    u32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return _GetNOnes64(CLAMP<u32>(mindestval, maxdestval, count1s));
  }

  inline u32 _Bits32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (val & _GetNOnes32(MIN<u32>(srcbitwidth, destbitwidth)));
  }

  inline u64 _Bits64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return (val & _GetNOnes64(MIN<u32>(srcbitwidth, destbitwidth)));
  }

  //Ops.h

  //LOGICAL BANG:
  inline u32 _LogicalBangBool32(u32 val, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 count1s = PopCount(val & mask);
    return ((count1s > (s32) (bitwidth - count1s)) ? 0 : mask);  // == when even number bits is ignored (warning at def)
  }

  inline u64 _LogicalBangBool64(u64 val, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    s32 count1s = PopCount(val & mask);
    return ((count1s > (s32) (bitwidth - count1s)) ? 0 : mask);  // == when even number bits is ignored (warning at def)
  }


  inline s32 _UnaryMinusInt32(s32 val, u32 bitwidth) 
  {
    return -val;  //assumes sign extended ???
  }

  inline s64 _UnaryMinusInt64(s64 val, u32 bitwidth) 
  {
    return -val;  //assumes sign extended ???
  }

  //Bitwise Binary Ops:
  inline u32 _BitwiseOrUnary32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    u32 maska = _GetNOnes32(PopCount(vala & mask));
    u32 maskb = _GetNOnes32(PopCount(valb & mask));
    return maska | maskb;  // "max"
  }

  inline u64 _BitwiseOrUnary64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    u64 maska = _GetNOnes64(PopCount(vala & mask));
    u64 maskb = _GetNOnes64(PopCount(valb & mask));
    return maska | maskb;  //"max"
  }

  inline u32 _BitwiseAndUnary32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    u32 maska = _GetNOnes32(PopCount(vala & mask));
    u32 maskb = _GetNOnes32(PopCount(valb & mask));
    return maska & maskb;  //"min"
  }

  inline u64 _BitwiseAndUnary64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    u64 maska = _GetNOnes64(PopCount(vala & mask));
    u64 maskb = _GetNOnes64(PopCount(valb & mask));
    return maska & maskb;  //"min"
  }

  inline u32 _BitwiseXorUnary32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    u32 counta = PopCount(vala & mask);
    u32 countb = PopCount(valb & mask);
    return _GetNOnes32(MAX<u32>(counta, countb) - MIN<u32>(counta, countb)); //right-justified ^
  }

  inline u64 _BitwiseXorUnary64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    u32 counta = PopCount(vala & mask);
    u32 countb = PopCount(valb & mask);
    return _GetNOnes64(MAX<u32>(counta, countb) - MIN<u32>(counta, countb)); //right-justified ^
  }

  inline u32 _BitwiseOrBits32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    return  ( (vala | valb) & mask) ;  // "at least max"
  }

  inline u64 _BitwiseOrBits64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala | valb) & mask);  //"at least max"
  }

  inline u32 _BitwiseAndBits32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline u64 _BitwiseAndBits64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }


  inline u32 _BitwiseXorBits32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala ^ valb) & mask);
  }

  inline u64 _BitwiseXorBits64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala ^ valb) & mask); 
  }

  inline s32 _BitwiseOrInt32(s32 vala, s32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    return  ( (vala | valb) & mask) ;  // "at least max"
  }

  inline s64 _BitwiseOrInt64(s64 vala, s64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala | valb) & mask);  //"at least max"
  }

  inline s32 _BitwiseAndInt32(s32 vala, s32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline s64 _BitwiseAndInt64(s64 vala, s64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline s32 _BitwiseXorInt32(s32 vala, s32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala ^ valb) & mask);
  }

  inline s64 _BitwiseXorInt64(s64 vala, s64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala ^ valb) & mask); 
  }

  inline u32 _BitwiseOrUnsigned32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    return  ( (vala | valb) & mask) ;  // "at least max"
  }

  inline u64 _BitwiseOrUnsigned64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala | valb) & mask);  //"at least max"
  }

  inline u32 _BitwiseAndUnsigned32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline u64 _BitwiseAndUnsigned64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline u32 _BitwiseXorUnsigned32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala ^ valb) & mask);
  }

  inline u64 _BitwiseXorUnsigned64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala ^ valb) & mask); 
  }

  inline u32 _BitwiseOrBool32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    u32 binvalb = _Bool32ToBits32(valb, bitwidth, bitwidth);
    return _Bits32ToBool32(binvala | binvalb, bitwidth, bitwidth);
  }

  inline u64 _BitwiseOrBool64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    u64 binvalb = _Bool64ToBits64(valb, bitwidth, bitwidth);
    return _Bits64ToBool64(binvala | binvalb, bitwidth, bitwidth);
  }

  inline u32 _BitwiseAndBool32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    u32 binvalb = _Bool32ToBits32(valb, bitwidth, bitwidth);
    return _Bits32ToBool32(binvala & binvalb, bitwidth, bitwidth);
  }

  inline u64 _BitwiseAndBool64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    u64 binvalb = _Bool64ToBits64(valb, bitwidth, bitwidth);
    return _Bits64ToBool64(binvala & binvalb, bitwidth, bitwidth);
  }

  inline u32 _BitwiseXorBool32(u32 vala, u32 valb, u32 bitwidth) 
  {
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    u32 binvalb = _Bool32ToBits32(valb, bitwidth, bitwidth);
    return _Bits32ToBool32(binvala ^ binvalb, bitwidth, bitwidth);
  }

  inline u64 _BitwiseXorBool64(u64 vala, u64 valb, u32 bitwidth) 
  {
    u64 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    u64 binvalb = _Bool64ToBits64(valb, bitwidth, bitwidth);
    return _Bits64ToBool64(binvala ^ binvalb, bitwidth, bitwidth);
  }

  // Ariths On INT:
  inline s32 _BinOpAddInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _Int32ToInt32(extvala + extvalb, 32, bitwidth);
  }

  inline s64 _BinOpAddInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _Int64ToInt64(extvala + extvalb, 64, bitwidth);
  }

  inline s32 _BinOpSubtractInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _Int32ToInt32(extvala - extvalb, 32, bitwidth);
  }

  inline s64 _BinOpSubtractInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _Int64ToInt64(extvala - extvalb, 64, bitwidth);
  }

  inline s32 _BinOpMultiplyInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _Int32ToInt32(extvala * extvalb, 32, bitwidth);
  }

  inline s64 _BinOpMultiplyInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _Int64ToInt64(extvala * extvalb, 64, bitwidth);
  }

  inline s32 _BinOpDivideInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _Int32ToInt32(extvala / extvalb, 32, bitwidth);
  }

  inline s64 _BinOpDivideInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    u64 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _Int64ToInt64(extvala / extvalb, 64, bitwidth);
  }

  // Ariths On UNSIGNED:
  inline u32 _BinOpAddUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _Unsigned32ToUnsigned32((vala & mask) + (valb & mask), bitwidth, bitwidth);
  }

  inline u64 _BinOpAddUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _Unsigned64ToUnsigned64((vala & mask) + (valb & mask), bitwidth, bitwidth);
  }

  inline u32 _BinOpSubtractUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _Unsigned32ToUnsigned32((vala & mask) - (valb & mask), bitwidth, bitwidth);
  }

  inline u64 _BinOpSubtractUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _Unsigned64ToUnsigned64((vala & mask) - (valb & mask), bitwidth, bitwidth);
  }

  inline u32 _BinOpMultiplyUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _Unsigned32ToUnsigned32((vala & mask) * (valb & mask), bitwidth, bitwidth);
  }

  inline u64 _BinOpMultiplyUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _Unsigned64ToUnsigned64((vala & mask) * (valb & mask), bitwidth, bitwidth);
  }

  inline u32 _BinOpDivideUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    u32 mask = _GetNOnes32(bitwidth);
    return _Unsigned32ToUnsigned32((vala & mask) / (valb & mask), bitwidth, bitwidth);
  }

  inline u64 _BinOpDivideUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    u64 mask = _GetNOnes64(bitwidth);
    return _Unsigned64ToUnsigned64((vala & mask) / (valb & mask), bitwidth, bitwidth);
  }

  //Bin Op Arith on Unary (e.g. op equals)
  //convert to binary before the operation; then convert back to unary
  inline s32 _BinOpAddUnary32(s32 vala, s32 valb, u32 bitwidth)
  {
    s32 binvala = _Unary32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Unary32ToInt32(valb, bitwidth, 32);
    return _Int32ToUnary32(binvala + binvalb, 32, bitwidth);
  }

  inline s64 _BinOpAddUnary64(s64 vala, s64 valb, u32 bitwidth)
  {
    s64 binvala = _Unary64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Unary64ToInt64(valb, bitwidth, 64);
    return _Int64ToUnary64(binvala + binvalb, 64, bitwidth);
  }

  inline s32 _BinOpSubtractUnary32(s32 vala, s32 valb, u32 bitwidth)
  {
    s32 binvala = _Unary32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Unary32ToInt32(valb, bitwidth, 32);
    return _Int32ToUnary32(binvala - binvalb, 32, bitwidth);
  }

  inline s64 _BinOpSubtractUnary64(s64 vala, s64 valb, u32 bitwidth)
  {
    s64 binvala = _Unary64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Unary64ToInt64(valb, bitwidth, 64);
    return _Int64ToUnary64(binvala - binvalb, 64, bitwidth);
  }

  inline s32 _BinOpMultiplyUnary32(s32 vala, s32 valb, u32 bitwidth)
  {
    s32 binvala = _Unary32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Unary32ToInt32(valb, bitwidth, 32);
    return _Int32ToUnary32(binvala * binvalb, 32, bitwidth);
  }

  inline s64 _BinOpMultiplyUnary64(s64 vala, s64 valb, u32 bitwidth)
  {
    s64 binvala = _Unary64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Unary64ToInt64(valb, bitwidth, 64);
    return _Int64ToUnary64(binvala * binvalb, 64, bitwidth);
  }

  inline s32 _BinOpDivideUnary32(s32 vala, s32 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    s32 binvala = _Unary32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Unary32ToInt32(valb, bitwidth, 32);
    return _Int32ToUnary32(binvala / binvalb, 32, bitwidth);
  }

  inline s64 _BinOpDivideUnary64(s64 vala, s64 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    s64 binvala = _Unary64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Unary64ToInt64(valb, bitwidth, 64);
    return _Int64ToUnary64(binvala / binvalb, 64, bitwidth);
  }


  //Bin Op Arith on Bool (e.g. op equals)
  //convert to binary before the operation; then convert back to bool
  inline s32 _BinOpAddBool32(s32 vala, s32 valb, u32 bitwidth)
  {
    s32 binvala = _Bool32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Bool32ToInt32(valb, bitwidth, 32);
    return _Int32ToBool32(binvala + binvalb, 32, bitwidth);
  }

  inline s64 _BinOpAddBool64(s64 vala, s64 valb, u32 bitwidth)
  {
    s64 binvala = _Bool64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Bool64ToInt64(valb, bitwidth, 64);
    return _Int64ToBool64(binvala + binvalb, 64, bitwidth);
  }

  inline s32 _BinOpSubtractBool32(s32 vala, s32 valb, u32 bitwidth)
  {
    s32 binvala = _Bool32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Bool32ToInt32(valb, bitwidth, 32);
    return _Int32ToBool32(binvala - binvalb, 32, bitwidth);
  }

  inline s64 _BinOpSubtractBool64(s64 vala, s64 valb, u32 bitwidth)
  {
    s64 binvala = _Bool64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Bool64ToInt64(valb, bitwidth, 64);
    return _Int64ToBool64(binvala - binvalb, 64, bitwidth);
  }

  inline s32 _BinOpMultiplyBool32(s32 vala, s32 valb, u32 bitwidth)
  {
    s32 binvala = _Bool32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Bool32ToInt32(valb, bitwidth, 32);
    return _Int32ToBool32(binvala * binvalb, 32, bitwidth);
  }

  inline s64 _BinOpMultiplyBool64(s64 vala, s64 valb, u32 bitwidth)
  {
    s64 binvala = _Bool64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Bool64ToInt64(valb, bitwidth, 64);
    return _Int64ToBool64(binvala * binvalb, 64, bitwidth);
  }

  inline s32 _BinOpDivideBool32(s32 vala, s32 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    s32 binvala = _Bool32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Bool32ToInt32(valb, bitwidth, 32);
    return _Int32ToBool32(binvala / binvalb, 32, bitwidth);
  }

  inline s64 _BinOpDivideBool64(s64 vala, s64 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    s64 binvala = _Bool64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Bool64ToInt64(valb, bitwidth, 64);
    return _Int64ToBool64(binvala / binvalb, 64, bitwidth);
  }

  //end Ops.h

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

  u32 Read(u32 pos, u32 len) const;  
  void Write(u32 pos, u32 len, u32 val) ;
};

//BitVector.tcc

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
u32 BitVector<S>::Read(u32 pos, u32 len) const
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
    static u32 ReadRaw(const BV & bv) ;
    static void WriteRaw(BV & bv, u32 val) ;
    static u32 ReadArrayRaw(const BV & bv, u32 arraywidth, u32 len, u32 pos);
    static void WriteArrayRaw(BV & bv, u32 val, u32 arraywidth, u32 len, u32 pos);
  };
  
//BitField.tcc
template <class BV, u32 VDTYPE, u32 LEN, u32 POS>
u32 BitField<BV,VDTYPE,LEN,POS>::ReadRaw(const BV & bv)
{
  return bv.Read(POS,LEN);
}

template <class BV, u32 VDTYPE, u32 LEN, u32 POS>
void BitField<BV,VDTYPE,LEN,POS>::WriteRaw(BV & bv, u32 val)
{
  bv.Write(POS,LEN,val);
}

template <class BV, u32 VDTYPE, u32 LEN, u32 POS>
u32 BitField<BV,VDTYPE,LEN,POS>::ReadArrayRaw(const BV & bv, u32 arraywidth, u32 len, u32 pos)
{
  if(arraywidth != LEN)
    FAIL();

  if(pos < POS || pos >= POS+LEN)
    FAIL();

  return bv.Read(pos,len);
}


template <class BV, u32 VDTYPE, u32 LEN, u32 POS>
void BitField<BV,VDTYPE,LEN,POS>::WriteArrayRaw(BV & bv, u32 val, u32 arraywidth, u32 len, u32 pos)
{
  if(pos < POS || pos > POS+LEN)  //>= ???
    FAIL();

  bv.Write(pos,len,val);
}

//AtomicParameterType.h
//#define AtomicParameterType BitField
template <class CC, u32 VDTYPE, u32 LEN, u32 POS>
struct AtomicParameterType
{
  typedef typename CC::PARAM_CONFIG P;
  typedef typename CC::ATOM_TYPE T;
  enum { BPA = P::BITS_PER_ATOM };

  static u32 ReadRaw(const BitVector<BPA> & bv){ return BitField<BitVector<BPA>, VDTYPE, LEN, POS>::ReadRaw(bv); }

  static void WriteRaw(BitVector<BPA> & bv, u32 val) { BitField<BitVector<BPA>, VDTYPE, LEN, POS>::WriteRaw(bv, val); }

  static u32 ReadArrayRaw(const BitVector<BPA> & bv, u32 arraywidth, u32 len, u32 pos) { return BitField<BitVector<BPA>, VDTYPE, LEN, POS>::ReadArrayRaw(bv, arraywidth, len, pos); }


  static void WriteArrayRaw(BitVector<BPA> & bv, u32 val, u32 arraywidth, u32 len, u32 pos) { BitField<BitVector<BPA>, VDTYPE, LEN, POS>::WriteArrayRaw(bv, val, arraywidth, len, pos); }

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
  const BitVector<BPA> & GetBits() const { return m_stg; }
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
    BFType::WriteRaw(m_defaultAtom.GetBits(), type);
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
