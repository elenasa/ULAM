#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>

class foo {

  class bar; //forward

public:

  //foo-classfuncs.cpp:19:7: error: ‘int foo::m(int)’ cannot be overloaded
  //foo-classfuncs.cpp:13:8: error: with ‘bool foo::m(int)’

  bool m(bool b)
  {
    return true;  //overload return types, if args differ too!
  }

  bool m(int a, bool b)
  {
    return true;  //overload return types, if args differ too!
  }

  //int m(int a);  //error: ‘int foo::m(int)’ cannot be overloaded with ‘int foo::m(int)’
  int m(int a)
  {
    return n(a);  //undefined at this point.
  }

  /*
    foo-classfuncs.cpp:27:7: error: ‘int foo::m(int, bool)’ cannot be overloaded
    foo-classfuncs.cpp:16:8: error: with ‘bool foo::m(int, bool)’
  */
  int m(bool b)
  {
    return n(b);
  }


  int n(int a)
  {
    return m(a);
  }

private:
  //int m;
};
