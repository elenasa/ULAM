/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods test version (t3233).
 */

namespace MFM{

  template<class EC, u32 POS>
  Ui_Ut_102964Atom<EC> Uq_10109211EventWindow11122n2<EC,POS>::Uf_4aref(UlamContext<EC> & uc,
                                                                T& Uv_4self, Ui_Ut_102323Int Uv_5index)	 //native
  {
    u32 siteNumber = Uv_5index.read();

    EventWindow<EC> & ew = uc.GetEventWindow();
    const T & a = ew.GetAtomSym(siteNumber);

    return Ui_Ut_102964Atom<EC>(a);
  }

  template<class EC, u32 POS>
  void Uq_10109211EventWindow11122n2<EC,POS>::Uf_4aset(UlamContext<EC> & uc,
                                                T& Uv_4self, Ui_Ut_102323Int Uv_5index, Ui_Ut_102964Atom<EC> Uv_1v) //native
  {
    u32 siteNumber = Uv_5index.read();

    EventWindow<EC> & ew = uc.GetEventWindow();
    ew.SetAtomSym(siteNumber, Uv_1v.read());
  }

} //MFM
