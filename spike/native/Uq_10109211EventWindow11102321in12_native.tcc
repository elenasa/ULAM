/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods test version (t3338).
*/

namespace MFM{

  template<class EC>
  Ui_Ut_r102961a<EC> Uq_10109211EventWindow11102321in12<EC>::Uf_4aref(const UlamContext<EC> & uc,
								      UlamRef<EC>& ur,
								      Ui_Ut_102321i<EC>& Uv_5index) const //native
  {
    u32 siteNumber = Uv_5index.read();
    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();
    return Ui_Ut_r102961a<EC>(ew.GetAtomBitStorage(siteNumber), 0u, uc);
  }

} //MFM
