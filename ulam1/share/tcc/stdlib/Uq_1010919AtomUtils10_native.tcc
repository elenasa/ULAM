/* -*- C++ -*- */

#include "UlamDefs.h"

namespace MFM {

  //! AtomUtils.ulam:10:   Type getType(Atom a) {
  template<class EC>
  Ui_Ut_102321i<EC> Uq_1010919AtomUtils10<EC>::Uf_7getType(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_102961a<EC> Uv_1a) const
  {
    return Ui_Ut_102321i<EC>(ur.GetType());
  } // Uf_7getType


  //! AtomUtils.ulam:13:   Atom new(Type t) {
  template<class EC>
  Ui_Ut_102961a<EC> Uq_1010919AtomUtils10<EC>::Uf_3new(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_102321i<EC> Uv_1t) const
  {
    const s32 t = Uv_1t.read();
    if (t < 0 || t > U16_MAX)
      FAIL(ILLEGAL_ARGUMENT);
    T p3atom((u32) t);
    const Ui_Ut_102961a<EC> tmp(p3atom);
    return tmp;
  } // Uf_3new

} //MFM
