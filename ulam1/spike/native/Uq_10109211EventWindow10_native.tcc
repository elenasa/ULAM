/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods test version (t3233).
 */

namespace MFM{

  template<class CC, u32 POS>
  Ui_Ut_102961a<CC> Uq_10109211EventWindow10<CC,POS>::Uf_4aref(const UlamContext<CC> & uc,
							       T& Uv_4atom, Ui_Ut_102321i<CC> Uv_5index) const //native
  {
    u32 siteNumber = Uv_5index.read();

    const EventWindow<CC> & ew = uc.GetEventWindow();
    const T & a = ew.GetAtomSym(siteNumber);

    return Ui_Ut_102961a<CC>(a);
  }

  template<class CC, u32 POS>
  void Uq_10109211EventWindow10<CC,POS>::Uf_4aset(const UlamContext<CC> & uc, T& Uv_4atom,
						  Ui_Ut_102321i<CC> Uv_5index, Ui_Ut_102961a<CC> Uv_1v) const //native
  {
    u32 siteNumber = Uv_5index.read();

    EventWindow<CC> & ew = const_cast <UlamContext<CC> &>(uc).GetEventWindow();
    ew.SetAtomSym(siteNumber, Uv_1v.read());
  }

} //MFM
