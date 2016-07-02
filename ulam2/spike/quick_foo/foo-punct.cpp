// string::find_last_of
#include <iostream>       // std::cout
#include <string>         // std::string
#include <ctype.h>

int main ()
{
  char c = '~';
  if(isalpha(c))
    std::cout << c << " is alpha " << '\n';

  if(isdigit(c))
    std::cout << c << " is digit " << '\n';

  if(ispunct(c))
    std::cout << c << " is punct " << '\n';

  return 0;
}
