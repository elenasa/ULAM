#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>


int main()
{

  int a[3];
  int b;
  a = 3;  //error: incompatible types in assignment of ‘int’ to ‘int [3]’
  1 = b;  //error: lvalue required as left operand of assignment
  a = a + b;  //error: incompatible types in assignment of ‘int*’ to ‘int [3]’
  a[0] = a + b; // error: invalid conversion from ‘int*’ to ‘int’ [-fpermissive]
  return 0;
}
