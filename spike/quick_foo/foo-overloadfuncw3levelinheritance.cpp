#include <stdio.h>

/*
gcc -ansi -Wall -pedantic foo-overloadfunc3levelinheritance.cpp
see also foo-overloadfuncwmultiinheritance.cpp

when c inherits from a, who inherits from b; and c does not define int func(int):
 gcc uses a's func

however, if c DOES define func, then c's is used.

no compilation errors
*/

class b {
public:
  int func(int& a)
  {
    return 2;
  }
  int vfunc()
  {
    return 2;
  }
};

class a : public b{
public:
  int func(int a)
  {
    return 1;
  }
  int vfunc()
  {
    return 1;
  }
};


class c : public a {
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
    rtn = csub.vfunc();
    return rtn;
  }
