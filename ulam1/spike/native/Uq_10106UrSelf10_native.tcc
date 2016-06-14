/**                                      -*- mode:C++ -*- */

namespace MFM{

//! UrSelf.ulam:16:   virtual ARGB getColor(Unsigned selector)
  template<class EC>
  Ui_Ut_14181u<EC> Uq_10106UrSelf10<EC>::Uf_8getColor(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& Uv_8selector)
  {

    Ui_Ut_14181u<EC> ret;
    const UlamClass<EC> * eff = ur.GetEffectiveSelf();
    if (!eff) return ret;  //black for wtf?

    ret.write(-1);  // white for default class
    const UlamElement<EC> * ue = eff->AsUlamElement();
    if (ue) ret.write(ue->GetElementColor());
    return ret;
  } // Uf_8getColor

} //MFM
