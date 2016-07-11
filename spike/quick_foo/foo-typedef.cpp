#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>



int main()
{

  {
    int x = 5;
    //ypedef char Field [x];  //ISO C++ forbids variable length array ‘Field’ [-Wvla]
    typedef char Field [5 * 10]; 
    typedef Field Green[3];
//using Field = char [10]; 
    Green g;
    g[2][5] = 'e';
    Field a;
    for(int i=0; i< 10; i++)
      a[i] = 'a' + i;

    for(int i=0; i< 10; i++)
      std::cout << a[i] << ", " ;
    std::cout << std::endl;

    {
      Field c;
      for(int i=0; i< 10; i++)
	c[i] = 'A' + i;

      for(int i=0; i< 10; i++)
	std::cout << c[i] << ", " ;
      std::cout << std::endl;
    
      {
	int Field = 0;  //shadow? ok.
      }

    }

    //error: ‘char Field [50]’ redeclared as different kind of symbol
    //foo-typedef.cpp:13:18: error: previous declaration of ‘typedef char Field [50]’
    //Field Field; 

  }

  //‘Field’ was not declared in this scope
  //Field b;  

  return 0;
}
