#include "UlamDefs.h"
#include "MDist.h"

namespace MFM {

  template<class CC, u32 POS>
  Ui_Ut_102328Unsigned Uq_10105MDist<CC,POS>::Uf_9212getLastIndex(T& Uv_4self, Ui_Ut_10138Unsigned Uv_6radius)
  {
    enum { R = CC::PARAM_CONFIG::EVENT_WINDOW_RADIUS };

    u32 radius = Uv_6radius.read();
    Ui_Ut_102328Unsigned ret(MDist<R>::get().GetLastIndex(radius));
    return ret;
  }

  template<class CC, u32 POS>
  Ui_Ut_102328Unsigned Uq_10105MDist<CC,POS>::Uf_9213getFirstIndex(T& Uv_4self, Ui_Ut_10138Unsigned Uv_6radius)
  {
    enum { R = CC::PARAM_CONFIG::EVENT_WINDOW_RADIUS };

    u32 radius = Uv_6radius.read();
    Ui_Ut_102328Unsigned ret(MDist<R>::get().GetFirstIndex(radius));
    return ret;
  }
}
