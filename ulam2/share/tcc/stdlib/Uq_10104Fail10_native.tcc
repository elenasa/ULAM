/* -*- C++ -*- */

#include "UlamContext.h"
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
