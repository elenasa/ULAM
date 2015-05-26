/* -*- C++ -*- */

#include "UlamDefs.h"

namespace MFM {

  //! AtomUtils.ulam:10:   Type getType(Atom a) {
  template<class EC, u32 POS>
  Ui_Ut_102321i Uq_1010919AtomUtils10<EC, POS>::Uf_7getType(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102961a<EC> Uv_1a)
  {
    T & atom = Uv_1a.getRef();
    return Ui_Ut_102321i(atom.GetType());
  } // Uf_7getType



  //! AtomUtils.ulam:13:   Atom new(Type t) {
  template<class EC, u32 POS>
  Ui_Ut_102961a<EC> Uq_1010919AtomUtils10<EC, POS>::Uf_3new(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102321i Uv_1t)
  {
    FAIL(INCOMPLETE_CODE);
#if 0
    //! AtomUtils.ulam:14:     Atom a;
    Ui_Ut_102961a<EC> Uv_1a;

    //! AtomUtils.ulam:15:     return a;
    const T Uh_tmpval_unpacked_14 = Uv_1a.read();
    const Ui_Ut_102961a<EC> Uh_tmpval_unpacked_15(Uh_tmpval_unpacked_14);
    return (Uh_tmpval_unpacked_15);
#endif
  } // Uf_3new

} //MFM
