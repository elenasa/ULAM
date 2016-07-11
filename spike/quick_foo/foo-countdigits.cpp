#include <stdlib.h>      //strtol
#include <iostream>       // std::cout
#include <sstream>       // std::iostream
#include <string>         // std::string
#include "../../include/itype.h"

namespace MFM
{
  static u32 countDigits(u32 num) 
  {
    const u32 BASE = 10;
    u32 digits = 0;
    do {
      ++digits;
      num /= BASE;
    } while (num > 0);
    return digits;
  }
}

int main ()
{
  MFM::u32 n = 0;
  MFM::u32 ndigits = MFM::countDigits(n);
  std::ostringstream outn;
  //if(n > 0)
    outn << ndigits << n;
    //else
    //outn << n;
  
  std::cout << outn.str()  << std::endl;

  std::string s = outn.str();
  const char * cstr = s.c_str();
  const char c = s.at(0);
  MFM::u32 i = c - '0';
  char * endc;
  MFM::u32 numval = strtol(cstr+1, &endc, 10);   //base 10

  std::cout << numval  << std::endl;
  return 0;
}
