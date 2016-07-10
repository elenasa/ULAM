#include "../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>


int main()
{
  int a = 0;
  while(1)
    {
      {
	if(a == 3)
	  break;    //can you break within double brackets?  YES!
	a++;
      }
    }

  if(a==3)
    break;
  else
    continue;

  for(int i =0; i < 3; i++)
    {
      if(a == 9)
	continue;
      else
	break;
    }
  return a;
}
