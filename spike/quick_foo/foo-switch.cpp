#include "../../include/itype.h"
#include <stdio.h>
#include <stdlib.h>

// doesn't matter if default is first or last!
// AT MOST one default!
// breaks not required (falls through)
// NOOP switch, no cases/default, ok.
int main()
{
  int a = 1, b = 0;
  switch(a)
    {
    default:
      b = a;
      break;
    case 1:
      b = 4;
      break;
    };

  printf("%d\n",b);

  return 0;
}
