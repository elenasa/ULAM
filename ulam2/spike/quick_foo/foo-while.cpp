#include "../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>


int main()
{
  while(1)
    int a;  //warning: unused variable ‘a’

  while(1)
    {
      int a(){} //error: a function-definition is not allowed here before ‘{’ token;
    }

  //a = 3;  //error: ‘a’ was not declared in this scope

  return 0;
}
