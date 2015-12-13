/* -*- C++ -*- */

#include "UlamContext.h"
#include "Random.h"

namespace MFM{

  template<class EC, u32 POS>
  Ui_Ut_102321u Uq_10106Random10<EC,POS>::Uf_6create(const UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102321u Uv_3max) const
  {
    u32 max = Uv_3max.read();
    Random & random = const_cast<UlamContext<EC>&>(uc).GetRandom();
    return Ui_Ut_102321u(random.Create(max));
  }

  template<class EC, u32 POS>
  Ui_Ut_102321u Uq_10106Random10<EC,POS>::Uf_4bits(const UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102321u UvbitCount) const
  {
    u32 bits = UvbitCount.read();
    Random & random = const_cast<UlamContext<EC>&>(uc).GetRandom();
    u32 rbits = random.CreateBits(bits);
    return Ui_Ut_102321u(rbits);
  }

} //MFM
