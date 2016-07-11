/**                                        -*- mode:C++ -*/

#ifndef ULAMTEST_H
#define ULAMTEST_H

#include "../../include/itype.h"

namespace MFM{

  struct UlamTest
  {
    s32 a;
    s32 b;

    s32 test()
    {
      a = 1;
      b = 2;
      a += b;
      return (a);
    }

  };
} //MFM

#endif //end ULAMTEST_H

int main()
{
  MFM::UlamTest utest;
  return utest.test();
}
