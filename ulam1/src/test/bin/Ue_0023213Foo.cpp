#include "Ue_0023213Foo.h"

namespace MFM{

  Ue_0023213Foo::Ue_0023213Foo(){}

  Ue_0023213Foo::~Ue_0023213Foo(){}

  Ut_001114Bool Ue_0023213Foo::Uf_15check(Ut_0023213Int Uv_11v)
  {
    return (true);
  }

  Ut_0023213Int Ue_0023213Foo::Uf_14test()
  {
    Ue_0023213Foo Uv_11f;
    Uv_11f.Um_13m_i = 3;
    Uv_11f.Uf_15check(1);
    Um_13m_i = 4;
    return (Uv_11f.Um_13m_i);
  }

} //MFM

