#include "../../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>

int m()
{
  return 7;
}


int main()
{
  MFM::u32 i = 6;

  {
    i++;
    std::cout << "first i, incr inside block now = " << i <<  ", expected 7) " << std::endl; 
    float i = 0.1;
    i++;
    std::cout << "2nd i, incr inside block now = " << i << ", expected 1.1) " << std::endl; 
    //foo-dupidents.cpp:16:11: error: ‘i’ has a previous declaration as ‘float i’
    //MFM::u32 i;
    MFM::u32 m = m();
    std::cout << "m is " << m <<  ", expected 7) " << std::endl; 
  }
  
  i ++;
  std::cout << "first i, outside block = " << i <<  ", expected 8) " << std::endl; 

  //foo-dupidents.cpp:31:13: error: ‘i’ cannot be used as a function
  {
    int j = m();
    std::cout << "j is " << j <<  ", expected 7) " << std::endl; 
  }

  return 0;
}
