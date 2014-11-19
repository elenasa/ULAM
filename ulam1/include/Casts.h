/**                                        -*- mode:C++ -*-
 * Casts.h Cast methods for generated code for ULAM
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
  \file Casts.h Cast methods for generated code for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/

#include "itype.h"
#include "Util.h"  //for PopCount

#ifndef CASTS_H
#define CASTS_H

namespace MFM {

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
    // saturates narrowing
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
    return _SignExtend32((val & _GetNOnes64(srcbitwidth)), destbitwidth); // no sign extend
  }

  inline s64 _Bits64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth) 
  {
    return _SignExtend64((val & _GetNOnes64(srcbitwidth)), destbitwidth); // no sign extend
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

} //MFM

#endif //CASTS_H
