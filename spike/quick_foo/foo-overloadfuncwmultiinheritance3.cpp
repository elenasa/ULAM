#include <stdio.h>

/*
   g++ -ansi -Wall -pedantic foo-overloadfuncwmultiinheritance3.cpp
*/

class q {
public:
  virtual int val() = 0; //pure
  virtual int sqr() { return val() * val(); }
};

class b : public q {
public:
  virtual int val()
  {
    return 3;
  }
};

class a : public q {
public:
  virtual int val()
  {
    return 2;
  }
};


class c : public a, b {
public:
  int func()
  {
    printf("a: %d, %d\n", a::val(), a::sqr());
    printf("b: %d, %d\n", b::val(), b::sqr());
    return a::sqr() + b::sqr();
  }

};

int main()
  {
    c csub;
    int rtn = csub.func();
    return rtn;
  }
