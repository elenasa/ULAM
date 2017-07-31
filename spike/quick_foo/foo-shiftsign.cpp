#include <iostream>
#include <stdio.h>

//#include "itype.h"
typedef signed int s32;
typedef unsigned int u32;
typedef u32 BitUnitType;
#define CHAR_BIT (8)

static const u32 BITS_PER_UNIT = sizeof(BitUnitType) * CHAR_BIT;

//static const u32 ARRAY_LENGTH = (BITS + BITS_PER_UNIT - 1) / BITS_PER_UNIT;

/**
 * Right-aligned mask generation.  Returns 0xFFFFFFFF if \a length
 * >= 32
 */
inline static u32 MakeMaskClip(const u32 length) {
  if (length<32) return (1u << length) - 1;
  return -1;
}

inline s32 _SignExtend32(u32 val, u32 bitwidth) {
  //  return ((s32)(val<<(32-bitwidth)))>>bitwidth;
  return ((s32)(val<<(32-bitwidth)))>>(32-bitwidth);
}

inline u32 _GetNOnes32(u32 bitwidth) {
  return (((u32)1)<<bitwidth)-1;
}

inline u32 _ShiftToBitNumber32(u32 value, u32 bitpos) {
  return value<<bitpos;
}

inline u32 _ShiftFromBitNumber32(u32 value, u32 bitpos) {
  return value>>bitpos;
}


int main()
  {

    u32 bits = 3;
    s32 uv = 7;
    s32 sv = _SignExtend32(uv, bits);
    u32 uv2 = _ShiftFromBitNumber32(sv, 32-bits);

    char hexstring1[32];
    sprintf(hexstring1,"0x%x -> 0x%x (%d), 0x%x", uv, sv, sv, uv2);
    

    std::cerr << "sign extend " << bits << " bits of " << uv  << " == (hex) " << hexstring1 << std::endl;

    s32 n2 = -1;
    u32 n1 = (u32) n2;
    if(n2 != 0)
      std::cerr << "n2 not zero" << std::endl;
    if(n1 != 0)
      std::cerr << "n1 not zero" << std::endl;

    return _GetNOnes32(bits);
  }
  
