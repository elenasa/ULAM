/* -*- C++ -*- */

#include "UlamContext.h"
#include "AtomSerializer.h"
#include "Fail.h"

namespace MFM{

  //! Fail.ulam:7:   Void fail() native;
  template<class EC, u32 POS>
  void Uq_10104Fail10<EC,POS>::Uf_4fail(const UlamContext<EC> & uc, T& Uv_4self) const
  {
    u32 type = Uv_4self.GetType();
    typedef typename EC::ATOM_CONFIG AC;
    AtomSerializer<AC> as(Uv_4self);
    const Tile<EC> & tile = uc.GetTile();
    SPoint ctr = uc.GetEventWindow().GetCenterInTile();
    SPointSerializer sctr(ctr);

    LOG.Message("FAIL() (0x%04x) by %@ in Tile %s site %@ ", type, &as, tile.GetLabel(), &sctr);
    FAIL(UNSPECIFIED_EXPLICIT_FAIL);
  }

  //! Fail.ulam:7:   Void fail(Int code) native;
  template<class EC, u32 POS>
  void Uq_10104Fail10<EC,POS>::Uf_4fail(const UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102321i<EC> Uv_4code)  const
  {
    s32 code = Uv_4code.read();
    u32 type = Uv_4self.GetType();
    typedef typename EC::ATOM_CONFIG AC;
    AtomSerializer<AC> as(Uv_4self);
    const Tile<EC> & tile = uc.GetTile();
    SPoint ctr = uc.GetEventWindow().GetCenterInTile();
    SPointSerializer sctr(ctr);

    LOG.Message("FAIL(%d) (0x%04x) by %@ in Tile %s site %@ ", code, type, &as, tile.GetLabel(), &sctr);
    FAIL_BY_NUMBER(code);
  }

} //MFM
