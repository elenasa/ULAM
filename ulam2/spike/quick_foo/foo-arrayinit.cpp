#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>

/* compiled with g++ -ansi -Wall -pedantic foo-arrayinit.cpp; ./a.out */

int main()
{
  static const int S = 4;
  typedef int Arr[S];

  Arr poo = {5, 4, 3, 1,};
  // [] empty, > number of initializers (ok, extras 0), < number of initializers error:
  //foo-arrayinit.cpp:12:30: error: too many initializers for ‘int [2]’
  int foo[S] = {1-1, 3*1, 2-1}; //foo[] ok too!!
  //foo-arrayinit.cpp:11:21: error: scalar object ‘scfoo’ requires one element in initializer
  // int scfoo = {1-1,0};
  int scfoo = {1-1,}; //dangling , ok

  //int scfoo = {};
  //foo-arrayinit.cpp:18:16: warning: extended initializer lists only available with -std=c++11 or -std=gnu++11 [enabled by default]

  int i = 0;
  while(i < S)
    {
      std::cout << "foo sub " << i << " is " << foo[i]<< std::endl;
      std::cout << "poo sub " << i << " is " << poo[i]<< std::endl;
      i++; //separate to avoid warning (unless [3] is specified):
      //foo-arrayinit.cpp:14:69: warning: operation on ‘i’ may be undefined [-Wsequence-point]
    }
  return 0;
}
