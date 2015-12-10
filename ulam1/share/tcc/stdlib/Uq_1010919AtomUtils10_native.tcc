/* -*- C++ -*- */

#include "UlamDefs.h"

namespace MFM {

  //! AtomUtils.ulam:10:   Type getType(Atom a) {
  template<class EC, u32 POS>
  Ui_Ut_102321i Uq_1010919AtomUtils10<EC, POS>::Uf_7getType(const UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102961a<EC> Uv_1a)
  {
    T & atom = Uv_1a.getRef();
    return Ui_Ut_102321i(atom.GetType());
  } // Uf_7getType


  //! AtomUtils.ulam:13:   Atom new(Type t) {
  template<class EC, u32 POS>
  Ui_Ut_102961a<EC> Uq_1010919AtomUtils10<EC, POS>::Uf_3new(const UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102321i Uv_1t)
  {
    const s32 t = Uv_1t.read();
    if (t < 0 || t > U16_MAX)
      FAIL(ILLEGAL_ARGUMENT);
    T p3atom((u32) t);
    const Ui_Ut_102961a<EC> tmp(p3atom);
    return tmp;
  } // Uf_3new

} //MFM
