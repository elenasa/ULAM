#include "../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>


int main()
{
  MFM::u32 i = 6;

  {
    i++;
    std::cout << "first i, incr inside block now = " << i <<  ", expected 7) " << std::endl; 
    MFM::u32 i = 0;
    i++;
    std::cout << "2nd i, incr inside block now = " << i << ", expected 1) " << std::endl; 
  }
  
  std::cout << "first i, outside block = " << i <<  ", expected 7) " << std::endl; 
  return i;
}
