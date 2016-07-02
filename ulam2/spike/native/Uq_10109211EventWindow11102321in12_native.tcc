/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods test version (t3233).
 */

namespace MFM{

  template<class EC>
  Ui_Ut_102961a<EC> Uq_10109211EventWindow11102321in12<EC>::Uf_4aref(const UlamContext<EC> & uc,
									 UlamRef<EC>& ur, Ui_Ut_102321i<EC>& Uv_5index) const //native
  {
    u32 siteNumber = Uv_5index.read();
    const EventWindow<EC> & ew = uc.GetEventWindow();
    const T & a = ew.GetAtomSym(siteNumber);
    return Ui_Ut_102961a<EC>(a);
  }

  template<class EC>
  void Uq_10109211EventWindow11102321in12<EC>::Uf_4aset(const UlamContext<EC> & uc,
							    UlamRef<EC>& ur, Ui_Ut_102321i<EC>& Uv_5index, Ui_Ut_102961a<EC>& Uv_1v) const //native
  {
    u32 siteNumber = Uv_5index.read();

    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();
    ew.SetAtomSym(siteNumber, Uv_1v.ReadAtom());
  }

} //MFM
