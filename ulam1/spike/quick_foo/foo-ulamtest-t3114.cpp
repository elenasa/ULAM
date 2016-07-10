/**                                        -*- mode:C++ -*/

#ifndef ULAMTEST_H
#define ULAMTEST_H

#include "../../include/itype.h"

namespace MFM{

  struct UlamTest
  {
    s32 d;

    s32 m(s32 m, bool b[3])
    {
      if(b[0])
        m = 1;
      else
        m = 2;
      return (m);
    }

    s32 test()
    {
      bool mybool[3];
      mybool[0] = true;
      mybool[1] = false;
      mybool[2] = false;
      d = m(7, mybool);
      return (d);
    }

  };
} //MFM

#endif //end ULAMTEST_H

int main()
{
  MFM::UlamTest utest;
  return utest.test();
}
