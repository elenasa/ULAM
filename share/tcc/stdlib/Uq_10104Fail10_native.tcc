/* -*- C++ -*- */

#include "UlamContext.h"
#include "UlamByteWrappers.h"
#include "AtomSerializer.h"
#include "Fail.h"

namespace MFM{

  //! Fail.ulam:7:   Void fail() native;
  template<class EC>
  void Uq_10104Fail10<EC>::Uf_4fail(const UlamContext<EC> & uc, UlamRef<EC>& ur) const
  {
    Ui_Ut_102321i<EC> code(MFM_FAIL_CODE_NUMBER(UNSPECIFIED_EXPLICIT_FAIL));
    Uf_4fail(uc, ur, code);
  }

  template<class EC>
  void Uq_10104Fail10<EC>::Uf_4fail(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Uq_r10109210ByteStream10<EC>& Ur_2bs) const
  {
    UlamRef<EC> urbs(Ur_2bs,0,0u);
    VfuncPtr readByteFunc;
    UlamRef<EC> urvf(urbs, Uq_10109216ByteStreamReader10<EC>::VOWNED_IDX_Uf_8readByte10, Uq_10109216ByteStreamReader10<EC>::THE_INSTANCE, readByteFunc);

    _UlamByteSourceWrapper<EC> ubsw(uc,urvf,(typename Uq_10109216ByteStreamReader10<EC>::Uf_8readByte10) (readByteFunc));

    u32 type = ur.GetType();
    typedef typename EC::ATOM_CONFIG AC;
    SPoint ctr = uc.GetEventWindow().GetCenterInTile();
    SPointSerializer sctr(ctr);

    if (type != T::ATOM_UNDEFINED_TYPE) { // Have an actual element
      T atom = ur.CreateAtom();
      AtomSerializer<AC> as(atom);
      LOG.Message("FAIL (0x%04x) by %@ in %s site %@: %< ",
                  type, &as, uc.GetContextLabel(), &sctr, &ubsw);
    } else {
      LOG.Message("FAIL in %s site %@: %< ",
                  uc.GetContextLabel(), &sctr, &ubsw);
    }
    FAIL(DESCRIBED_FAILURE);
  }

  //! Fail.ulam:7:   Void fail(Int code) native;
  template<class EC>
  void Uq_10104Fail10<EC>::Uf_4fail(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_102321i<EC>& Uv_4code) const
  {
    s32 code = Uv_4code.read();
    u32 type = ur.GetType();
    typedef typename EC::ATOM_CONFIG AC;
    SPoint ctr = uc.GetEventWindow().GetCenterInTile();
    SPointSerializer sctr(ctr);

    if (type != T::ATOM_UNDEFINED_TYPE) { // Have an actual element
      T atom = ur.CreateAtom();
      AtomSerializer<AC> as(atom);
      LOG.Message("FAIL(%d/0x%08x) (0x%04x) by %@ in %s site %@ ",
                  code, code, type, &as, uc.GetContextLabel(), &sctr);
    } else {
      LOG.Message("FAIL(%d/0x%08x) in %s site %@ ",
                  code, code, uc.GetContextLabel(), &sctr);
    }
    FAIL_BY_NUMBER(code);
  }

} //MFM
