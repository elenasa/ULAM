#include "../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>

void func()
{
  int j = 50;
  while(j--)
    {
      if(j==52) goto f45;
      printf("%d\n",j);
    f45:
      printf("endwhile in func\n");
    }
}



int main()
{
  /*
  for(int i = 0; i < 5; ++i)
    if(i==0) continue;
  */

  int x = 0;

  for(int i = 0; i < 5; ++i)
    {
      goto f46;
      int j;  //declared outside so goto f45 (inside) doesn't 'skip initialization of j'
      for(j = 0; j < 5; ++j)
	{
	  goto f45;
	  continue;
	f45:
	  x=1;
	}
      goto f45;
    f46:
      x=2;
    }

  func();
 f45:
  printf("%d\n",x);

  return 0;

}
