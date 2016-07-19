#include "../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <math.h>

#if 0
std::string toupper(const std::string & s)
{
  std::string ret(s.size(), char());
  for(unsigned int i = 0; i < s.size(); ++i)
    ret[i] = (s[i] <= 'z' && s[i] >= 'a') ? s[i]-('a'-'A') : s[i];
  return ret;
}
#endif

std::string allcaps(const char * s)
{
  unsigned int len = strlen(s);
  std::ostringstream up;

  for(unsigned int i = 0; i < len; ++i)
    {
      std::string c(1, (s[i] <= 'z' && s[i] >= 'a') ? s[i]-('a'-'A') : s[i]);
      up << c;
    }
  return up.str();
}


int main()
{
  std::string numstr("Foo");

  //  convert a string to all uppercase
  const char * numlist = numstr.c_str();

  std::cout << "converted: " << numstr << " to " << allcaps(numlist) << std::endl;
  return 0;
}

