/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods test version (t3233).
 */

namespace MFM{

  template<class CC, u32 POS>
  Ui_Ut_102964Atom<CC> Uq_10109211EventWindow<CC,POS>::Uf_4aref(UlamContext<CC> & uc,
                                                                T& Uv_4self, Ui_Ut_102323Int Uv_5index)	 //native
  {
    u32 siteNumber = Uv_5index.read();

    EventWindow<CC> & ew = uc.GetEventWindow();
    const T & a = ew.GetAtomSym(siteNumber);

    return Ui_Ut_102964Atom<CC>(a);
  }

  template<class CC, u32 POS>
  void Uq_10109211EventWindow<CC,POS>::Uf_4aset(UlamContext<CC> & uc,
                                                T& Uv_4self, Ui_Ut_102323Int Uv_5index, Ui_Ut_102964Atom<CC> Uv_1v) //native
  {
    u32 siteNumber = Uv_5index.read();

    EventWindow<CC> & ew = uc.GetEventWindow();
    ew.SetAtomSym(siteNumber, Uv_1v.read());
  }

} //MFM
