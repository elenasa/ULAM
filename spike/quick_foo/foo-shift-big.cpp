#include <iostream>
#include <stdio.h>

//#include "itype.h"

typedef unsigned int u32;
typedef int s32;

int main()
  {
    s32 i = 33;
    s32 x = 1 << i;
    s32 y = 1 >> i;

    //33: x is 2; y is 0
    //32: x is 1; y is 1
    printf("%d: x is %x; y is %d\n",i, x,y);

    //33: w is 2; z is 0
    //32: w is 1; z is 1
    u32 w = 1 << i;
    u32 z = 1 >> i;

    printf("%d: w is %u; z is %u\n",i, w,z);

    return 0;
  }
