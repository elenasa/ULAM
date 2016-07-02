#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>

int m(int m)
{
  return abs(m);
}


int main()
{
  //int m’ redeclared as different kind of symbol
  //foo-varfuncnames.cpp:15:7: error: previous declaration of ‘int m(int)’

  //  int m(int);
  //  int bs = m(-8);
  //std::cout << " bs == " << bs << std::endl;
  int m;
#if 1
  int abs; 
  int f = -1;
  if(f < 0)
    {
      int m(int);
      //f = abs(f);  //error!
      f = m(f);  //error!
      std::cout << " f == " << f << std::endl;
    }
  //foo-varfuncnames.cpp:16:15: error: ‘abs’ cannot be used as a function
  //abs = abs(-8);
  //abs = m(-8); cannot be used as a function, already decalred a var
  std::cout << " abs == " << abs << std::endl;
#else
  int nabs;
  nabs = abs(-8);
  std::cout << " nabs == " << nabs << std::endl;
  int abs = nabs;
  std::cout << " abs == " << abs << std::endl;
#endif

  return 0;
}
