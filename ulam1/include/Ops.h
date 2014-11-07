/**                                        -*- mode:C++ -*-
 * Ops.h Operator semantics for ULAM
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
  \file Ops.h Operator semantics for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#include "Casts.h" 

#ifndef OPS_H
#define OPS_H

namespace MFM {

#define FAIL() assert(0)


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

  // Ariths On INTS:
  inline s32 _BinOpAddInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    return vala + valb; //XXX
  }

  inline s64 _BinOpAddInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    return vala + valb; //XXX
  }

  inline s32 _BinOpSubtractInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    return vala - valb; //XXX
  }

  inline s64 _BinOpSubtractInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    return vala - valb; //XXX
  }

  inline s32 _BinOpMultiplyInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    return vala * valb; //XXX
  }

  inline s64 _BinOpMultiplyInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    return vala * valb; //XXX
  }


  inline s32 _BinOpDivideInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    return vala / valb; //XXX
  }

  inline s64 _BinOpDivideInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    return vala / valb; //XXX
  }


  // Ariths On UNSIGNED:
  inline u32 _BinOpAddUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    return vala + valb; //XXX
  }

  inline u64 _BinOpAddUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    return vala + valb; //XXX
  }

  inline u32 _BinOpSubtractUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    return vala - valb; //XXX
  }

  inline u64 _BinOpSubtractUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    return vala - valb; //XXX
  }

  inline u32 _BinOpMultiplyUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    return vala * valb; //XXX
  }

  inline u64 _BinOpMultiplyUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    return vala * valb; //XXX
  }

  inline u32 _BinOpDivideUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    return vala / valb; //XXX
  }

  inline u64 _BinOpDivideUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    if(valb == 0) FAIL();
    return vala / valb; //XXX
  }


} //MFM

#endif //OPS_H
