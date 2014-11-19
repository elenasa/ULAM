#include "Ue_001613Foo.h"

namespace MFM{

  Ue_001613Foo::Ue_001613Foo(){}

  Ue_001613Foo::~Ue_001613Foo(){}

  Ut_161114Bool Ue_001613Foo::Uf_15check(Ut_0023213Int Uv_11v)
  {
    Ut_161114Bool Uv_13rba;
    Uv_13rba.b[1] = Uv_13rba.b[3] = Uv_13rba.b[5] = true;
    return (Uv_13rba);
  }

  Ut_0023213Int Ue_001613Foo::Uf_14test()
  {
    Ue_001613Foo Uv_11f;
    Uv_11f.Um_14m_ba.b[0] = true;
    Uv_11f.Um_14m_ba = Uv_11f.Uf_15check(1);
    Um_14m_ba = Uv_11f.Um_14m_ba;
    return (0);
  }

} //MFM

