/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods test version (t3233).
 */

namespace MFM{

  template<class CC>
  Ui_Ut_r102961a<CC> Uq_10109211EventWindow10<CC>::Uf_4aref(const UlamContext<CC> & uc, UlamRef<CC>& ur, Ui_Ut_102321i<CC>& Uv_5index) const //native
  {
    u32 siteNumber = Uv_5index.read();
    EventWindow<CC> & ew = const_cast <UlamContext<CC> &>(uc).GetEventWindow();
    return Ui_Ut_r102961a<CC>(ew.GetAtomBitStorage(siteNumber), 0u, uc);
  }

} //MFM
