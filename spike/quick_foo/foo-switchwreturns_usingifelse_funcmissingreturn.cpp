#include "../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>

//is gcc smart enough to detect unnecessary return in func
// here, using if else structure output by ulam gencode

int func(int arg)
{
  /*
  switch(--arg)
    {
    case -1:
      {
	return 3;
      }
    case 0:
      {
	return 2;
      }
    default:
      {
	return 1;
      }
    } //optional ;
  */
  --arg;
  {
    if(arg == -1)
      {
	{
	  return 3;
	}
      }
    else
      {
	if(arg == 0)
	  {
	    {
	      return 2;
	    }
	  }
	else
	  {
	    if(1)
	      {
		{
		  return 1;
		}
	      }
	  }
      }
  endcontrolloop11:
    __attribute__((__unused__));
  }
  //missing return;
}


int main()
{
  int a = 1, b = 0;
  b = func(a);

  printf("%d\n",b);

  return 0;
}
