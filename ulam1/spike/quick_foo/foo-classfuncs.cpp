#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>

class foo {

  class bar; //forward

public:

  //int m(int a);  //error: ‘int foo::m(int)’ cannot be overloaded with ‘int foo::m(int)’
  int m(int a)
  {
    return n(a);  //undefined at this point. 
  }

  int n(int a)
  {
    return m(a);
  }

private:
  //int m;
};
