#include "UlamDefs.h"
#include "MDist.h"

namespace MFM {

  //! WindowUtils.ulam:83:   Unsigned getLastIndex(Radius radius) native;
  template<class CC>
  Ui_Ut_102328Unsigned Uf_9212getLastIndex(UlamContext<CC> & uc, T& Uv_4self, Ui_Ut_10138Unsigned Uv_6radius)
  {
    u32 radius = Uv_6radius.read();
    Ui_Ut_102328Unsigned ret(MDist::Get().GetLastIndex(radius));
    return ret;
  }

  //! WindowUtils.ulam:82:   Unsigned getFirstIndex(Radius radius) native;
  template<class CC>
  Ui_Ut_102328Unsigned Uf_9213getFirstIndex(UlamContext<CC> & uc, T& Uv_4self, Ui_Ut_10138Unsigned Uv_6radius)
  {
    u32 radius = Uv_6radius.read();
    Ui_Ut_102328Unsigned ret(MDist::Get().GetFirstIndex(radius));
    return ret;
  }
}
