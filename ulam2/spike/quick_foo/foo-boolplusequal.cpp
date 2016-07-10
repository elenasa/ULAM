#include "../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>


int main()
{
  bool a,b;
  a = false; 
  b = false;

  //  a /= true;
  //a /= b; //12912 Floating point exception(core dumped) ./a.out

  a = a + true;
  if(a)
    std::cout << a << " and " << b << std::endl;
  else
    std::cout << a << " but " << b << std::endl;
  return 0;
}
