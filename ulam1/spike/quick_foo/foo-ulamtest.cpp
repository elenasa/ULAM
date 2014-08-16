/**                                        -*- mode:C++ -*/

#ifndef ULAMTEST_H
#define ULAMTEST_H

#include "../../include/itype.h"

namespace MFM{

  struct UlamTest
  {
    s32 a;

    s32 test()
    {
      return (a = fact (4 ));
    }

    s32 fact(s32 n)
    {
      if((bool) n)
        return (n * fact (n - 1 ));
      else
        return (1);
    }

  };
} //MFM

#endif //end ULAMTEST_H

int main()
{
  MFM::UlamTest utest;
  return utest.test();
}
