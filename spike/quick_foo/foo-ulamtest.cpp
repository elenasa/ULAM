/**                                        -*- mode:C++ -*/

#include "../../include/itype.h"

#ifndef Up_Ut_0232Int
#define Up_Ut_0232Int
namespace MFM{
  typedef s32 Ut_0232Int;
} //MFM
#endif /*Up_Ut_0232Int */

#ifndef Up_Ut_0232Float
#define Up_Ut_0232Float
namespace MFM{
  typedef float Ut_0232Float;
} //MFM
#endif /*Up_Ut_0232Float */

#ifndef Up_Ut_018Bool
#define Up_Ut_018Bool
namespace MFM{
  typedef bool Ut_018Bool;
} //MFM
#endif /*Up_Ut_018Bool */

#ifndef Up_Ut_18232Int
#define Up_Ut_18232Int
namespace MFM{
  struct Ut_18232Int
   {
    s32 i[8];
  };
} //MFM
#endif /*Up_Ut_18232Int */

/**                                        -*- mode:C++ -*/

#ifndef ULAMTEST_H
#define ULAMTEST_H

#include "../../include/itype.h"

namespace MFM{

  struct UlamTest
  {
    Ut_18232Int Um_d;

    Ut_18232Int Uf_foo(Ut_018Bool Uv_b)
    {
      Ut_18232Int Uv_m;
      if(Uv_b)
        Uv_m.i[0] = 1;
      else
        Uv_m.i[0] = 2;
      return (Uv_m);
    }

    Ut_0232Int Uf_test(void)
    {
      Ut_018Bool Uv_mybool;
      Uv_mybool = true;
      Um_d = Uf_foo(Uv_mybool);
      return (Um_d.i[0]);
    }

  };
} //MFM

#endif //end ULAMTEST_H

int main()
{
  MFM::UlamTest utest;
  return utest.Uf_test();
}
