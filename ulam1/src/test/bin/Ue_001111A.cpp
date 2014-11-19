#include "Ue_001111A.h"

namespace MFM{

  Ue_001111A::Ue_001111A(){}

  Ue_001111A::~Ue_001111A(){}

  Ut_001114Bool Ue_001111A::Uf_13foo(Ut_0023213Int Uv_11m, Ut_001114Bool Uv_11b)
  {
    Ut_0023213Int Uv_11d;
    {
      Ut_1823213Int Uv_11e;
      Uv_11b;
    }
    Ut_001114Bool Uv_11c;
    Uv_11c = (Ut_001114Bool) (Uv_11d = Uv_11m);
    return (Uv_11c);
  }

  Ut_0023213Int Ue_001111A::Uf_14test()
  {
    Um_11c = Uf_13foo(1, true);
    return ((Ut_0023213Int) (Um_11c));
  }

} //MFM

