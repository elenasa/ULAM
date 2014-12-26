/* -*- C++ -*- */

#include "UlamContext.h"
#include "Random.h"

namespace MFM{

  template<class CC, u32 POS>
  Ui_Ut_102328Unsigned Uq_10106Random<CC,POS>::Uf_6create(UlamContext<CC> & uc, T& Uv_4self, Ui_Ut_102328Unsigned Uv_3max)
  {
    u32 max = Uv_3max.read();
    Random & random = uc.GetRandom();
    return Ui_Ut_102328Unsigned(random.Create(max));
  }

} //MFM
