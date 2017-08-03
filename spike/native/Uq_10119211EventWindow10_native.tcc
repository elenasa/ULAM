/* -*- C++ -*- */
/**
   Uq_10119211EventWindow_native.tcc: EventWindow native methods test (ulam-3) w operator overload.
 */

namespace MFM{

  template<class CC>
  Ui_Ut_r102961a<CC> Uq_10119211EventWindow10<CC>::Uf_9213_operator5b5d(const UlamContext<CC> & uc, UlamRef<CC>& ur, Ui_Ut_102321i<CC>& Uv_5index) const //native
  {
    u32 siteNumber = Uv_5index.read();
    EventWindow<CC> & ew = const_cast <UlamContext<CC> &>(uc).GetEventWindow();
    return Ui_Ut_r102961a<CC>(ew.GetAtomBitStorage(siteNumber), 0u, uc);
  }

} //MFM
