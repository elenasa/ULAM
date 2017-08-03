#include <stdio.h>

/*
gcc -ansi -Wall -pedantic foo-overloadfuncwithref.cpp
foo-overloadfuncwithref.cpp: In function ‘int main()’:
foo-overloadfuncwithref.cpp:17:13: error: call of overloaded ‘func(int&)’ is ambiguous
     func(mya);
*/

void func(int a)
{
  return (void) (a = 3);
}

void func(int& a)
{
  return (void) (a = 3);
}

int main()
  {
    int mya = 0;
    func(mya);
    return mya;
  }
