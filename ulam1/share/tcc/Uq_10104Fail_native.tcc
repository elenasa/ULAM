/* -*- C++ -*- */

#include "UlamContext.h"
#include "Fail.h"

namespace MFM{

  //! Fail.ulam:7:   Void fail(Int code) native;
  template<class CC, u32 POS>
  void Uq_10104Fail<CC,POS>::Uf_4fail(UlamContext<CC> & uc, T& Uv_4self, Ui_Ut_102323Int Uv_4code)
  {
    s32 code = Uv_4code.read();
    FAIL_BY_NUMBER(code);
  }

} //MFM
