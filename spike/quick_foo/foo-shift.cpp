#include <iostream>
#include <stdio.h>

//#include "itype.h"

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


int main()
  {
#if 0
    //first writeUnit
    u32 startIdx = 25;
    u32 length = 7;
    u32 value = 4 >> (32 - length);
#else
    //second writeUnit
    u32 startIdx = 0;
    u32 length = 25;
    u32 value = 4;
#endif
    u32 shift = BITS_PER_UNIT - (startIdx + length); 

    u32 mask = MakeMaskClip(length);
    char hexstring1[32];
    sprintf(hexstring1,"%x", mask);

    u32 mask2 = mask << shift;
    char hexstring2[32];
    sprintf(hexstring2,"%x", mask2);


    u32 mask3 = ~mask2;
    char hexstring3[32];
    sprintf(hexstring3,"%x", mask3);


    std::cerr << "shift = " << shift << ", mask(beforeshift) = " << mask << " (hex " << hexstring1 << "), mask(aftershift) = " << mask2 << " (hex " << hexstring2 << "), ~maskshifted = " << mask3 << " (hex " << hexstring3 << ")" << std::endl;

    //    u32 maskof1 = 1;
    //sprintf(hexstring,"%x", ~maskof1);
    //xstd::cerr << "maskof1 = " << maskof1 << ", ~==" << hexstring << std::endl;


    u32 value2 = value << shift & mask2;

    std::cerr << "value = " << value << ", value(aftershift) = " << value2 << std::endl;


    return 1;
  }
  
