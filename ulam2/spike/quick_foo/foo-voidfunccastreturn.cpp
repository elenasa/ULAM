#include <stdio.h>


void func()
{
  int a;
  return (void) a;
}

void printfunc()
{
  int a = 0;
  //foo-voidfunccastreturn.cpp:14:33: error: return-statement with a value, in function returning 'void' [-fpermissive]
  //return printf("a is %d\n", ++a);

  return (void) printf("a is %d\n", ++a);
}

int main()
  {
    func();
    printfunc();
    return 0;
  }
