/* -*- C++ -*- */

#include "UlamContext.h"
#include "Random.h"

namespace MFM{

  template<class EC, u32 POS>
  Ui_Ut_102321u Uq_10106Random0<EC,POS>::Uf_6create(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102321u Uv_3max)
  {
    u32 max = Uv_3max.read();
    Random & random = uc.GetRandom();
    return Ui_Ut_102321u(random.Create(max));
  }

  template<class EC, u32 POS>
  Ui_Ut_102321u Uq_10106Random0<EC,POS>::Uf_4bits(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102321u UvbitCount)
  {
    u32 bits = UvbitCount.read();
    Random & random = uc.GetRandom();
    u32 rbits = random.CreateBits(bits);
    return Ui_Ut_102321u(rbits);
  }

} //MFM
