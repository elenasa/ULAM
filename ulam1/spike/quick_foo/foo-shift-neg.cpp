#include <iostream>
#include <stdio.h>

//#include "itype.h"

typedef unsigned int u32;
typedef int s32;

int main()
  {
    s32 i = -1;
    s32 x = 1 << i;
    s32 y = 1 >> 1;
    std::cerr << "x is " << std::hex << x << "\ny is " << y << std::endl;
    
    return x;
  }
  
