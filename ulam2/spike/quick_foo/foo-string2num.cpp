#include "../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>

MFM::u32 pow10(MFM::u32 e)
{
  MFM::u32 val = 1;
  for(MFM::u32 i = 0; i < e; i++)
    val *= 10;
  return val;
}


int main()
{
  std::string numstr("123");

  //  convert a string to an integer
  MFM::u32 strlen = numstr.length();
  const char * numlist = numstr.c_str();
  char * nEnd;
  MFM::s32 num = 0;
  for(MFM::s32 i = strlen - 1; i >= 0; i--)
    {
      std::cout << "numlist["<<i<<"] = " << numlist[i] << " is " << (int) (numlist[i] - '0') << " * 10 raise to power " << (strlen - i - 1) << " is " << pow10(strlen - i - 1) << std::endl;
      num += (numlist[i] - '0') * pow10(strlen - i - 1);
      std::cout << " num is now " << num << std::endl;
    }
  std::cout << "Convert string <" << numstr << "> to " << num << std::endl;
  std::cout << "Convert string <" << numstr << "> to " << strtol(numlist, &nEnd, 10) << " or " << atol(numlist) << std::endl;
 
  char zero = '1';
  
  std::cout << "zero " << zero << " is " << (int) (zero - '0') << std::endl;
  return num;
}
