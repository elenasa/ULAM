/* -*- C++ -*- */

#include "UlamContext.h"
#include "AtomSerializer.h"
#include "Fail.h"

namespace MFM{

  //! Fail.ulam:7:   Void fail() native;
  template<class EC, u32 POS>
  void Uq_10104Fail0<EC,POS>::Uf_4fail(UlamContext<EC> & uc, T& Uv_4self)
  {
    s32 code = -1;
    u32 type = Uv_4self.GetType();
    typedef typename EC::ATOM_CONFIG AC;
    AtomSerializer<AC> as(Uv_4self);
    LOG.Message("FAIL(%d) (0x%04x) by %@", code, type, &as);
    FAIL_BY_NUMBER(code);
  }

  //! Fail.ulam:7:   Void fail(Int code) native;
  template<class EC, u32 POS>
  void Uq_10104Fail0<EC,POS>::Uf_4fail(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102321i Uv_4code)
  {
    s32 code = Uv_4code.read();
    u32 type = Uv_4self.GetType();
    typedef typename EC::ATOM_CONFIG AC;
    AtomSerializer<AC> as(Uv_4self);
    LOG.Message("FAIL(%d) (0x%04x) by %@", code, type, &as);
    FAIL_BY_NUMBER(code);
  }

} //MFM
