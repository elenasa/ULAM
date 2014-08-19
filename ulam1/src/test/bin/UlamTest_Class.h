/**                                        -*- mode:C++ -*/

#ifndef ULAMTESTCLASS_H
#define ULAMTESTCLASS_H

#include "UlamTest_Types.h"

namespace MFM{

  struct UlamTest_Class
  {
    Ut_1823213Int Um_11d;

    Ut_1823213Int Uf_13foo(Ut_101814Bool Uv_11b)
    {
      Ut_1823213Int Uv_11m;
      if(Uv_11b)
        Uv_11m.i[0] = 1;
      else
        Uv_11m.i[0] = 2;
      return (Uv_11m);
    }

    Ut_1023213Int Uf_14test()
    {
      Ut_101814Bool Uv_16mybool;
      Uv_16mybool = true;
      Um_11d = Uf_13foo(Uv_16mybool);
      return (Um_11d.i[0]);
    }

  };
} //MFM

#endif //end ULAMTESTCLASS_H
