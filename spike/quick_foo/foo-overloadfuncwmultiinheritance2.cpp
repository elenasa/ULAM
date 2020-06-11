#include <stdio.h>

/*
gcc -ansi -Wall -pedantic foo-overloadfuncwmultiinheritance2.cpp
see also foo-overloadfuncwithref.cpp

same results as foo-overloadfuncwmultiinheritance.cpp

when c inherits from both a, and b; and c does not define int func(int):
foo-overloadfuncwmultiinheritance.cpp:45:20: error: request for member ‘func’ is ambiguous
     int rtn = csub.func(mya);
                    ^
foo-overloadfuncwmultiinheritance.cpp:16:7: note: candidates are: int d::func(int&)
   int func(int& a)
       ^
foo-overloadfuncwmultiinheritance.cpp:24:7: note:                 int a::func(int)
   int func(int a)
       ^

however, if c DOES define func, then c's is used.

when c inherits from b who inherits from a; and c does not define int func(int):
 gcc uses the first base class' func. (see foo-overloadfuncw3levelinheritance.cpp)

*/

class d {
public:
  int func(int& a)
  {
    return 2;
  }
};

class b :  public d{
public:
};

class a {
public:
  int func(int a)
  {
    return 1;
  }
};


class c : public a, b {
public:
  /*
  int func(int& a)
  {
    return 3;
  }
  */
};

int main()
  {
    int mya = 0;
    c csub;
    int rtn = csub.func(mya);
    return rtn;
  }
