#include "UlamDefs.h"

namespace MFM {

  //! AtomUtils.ulam:10:   Type getType(Atom a) {
  template<class CC, u32 POS>
  Ui_Ut_102323Int Uq_1010919AtomUtils<CC, POS>::Uf_7getType(T& Uv_4self, Ui_Ut_102964Atom<CC> Uv_1a)
  {
    T & atom = Uv_1a.getRef();
    return Ui_Ut_102323Int(atom.GetType());
  } // Uf_7getType



  //! AtomUtils.ulam:13:   Atom new(Type t) {
  template<class CC, u32 POS>
  Ui_Ut_102964Atom<CC> Uq_1010919AtomUtils<CC, POS>::Uf_3new(T& Uv_4self, Ui_Ut_102323Int Uv_1t)
  {
    FAIL(INCOMPLETE_CODE);
    //! AtomUtils.ulam:14:     Atom a;
    Ui_Ut_102964Atom<CC> Uv_1a;

    //! AtomUtils.ulam:15:     return a;
    const T Uh_tmpval_unpacked_14 = Uv_1a.read();
    const Ui_Ut_102964Atom<CC> Uh_tmpval_unpacked_15(Uh_tmpval_unpacked_14);
    return (Uh_tmpval_unpacked_15);

  } // Uf_3new

}
