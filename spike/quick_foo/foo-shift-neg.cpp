#include <iostream>
#include <stdio.h>

//#include "itype.h"

typedef unsigned int u32;
typedef int s32;

int main()
  {
    s32 i = -1;
    s32 x = 1 << i;
    /*
      foo-shift-neg.cpp:13:19: warning: right shift count is negative [enabled by default]
      s32 y = 1 >> -1;
      ^
      x is 80000000; y is 2
    */
    //s32 y = 1 >> -1;
    //s32 y = 1 >> 1;
    s32 y = 1 >> 0;
    //    std::cerr << "x is " << std::hex << x << "\ny is " << y << std::endl;
    printf("x is %x; y is %d\n",x,y);

    s32 z = i >> 8;

    printf("z is %x\n",z);
    return x;
  }
