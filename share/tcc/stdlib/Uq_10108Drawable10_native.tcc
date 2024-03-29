/**                                      -*- mode:C++ -*- */
#include "Utils.h"

namespace MFM{

  template<class EC>
  Ui_Ut_10111b<EC> Uq_10108Drawable10<EC>::Uf_7canDraw(const UlamContext<EC>& uc, UlamRef<EC>& ur) const
  {
    const bool canDraw = uc.HasEventWindowRenderer();
    Ui_Ut_10111b<EC> ret(canDraw);
    return ret; 
  } // Uf_7canDraw

  template<class EC>
  void Uq_10108Drawable10<EC>::Uf_9219drawMaskedLineUnits(const UlamContext<EC>& uc, UlamRef<EC>& ur,
                                                          Ui_Uq_102323C2D10<EC>& Uv_9210startUnits,
                                                          Ui_Uq_102323C2D10<EC>& Uv_8endUnits,
                                                          Ui_Ut_14181u<EC>& Uv_7onColor,
                                                          Ui_Ut_14181u<EC>& Uv_8offColor,
                                                          Ui_Ut_102321t<EC>& Uv_6mask32,
                                                          Ui_Ut_102321u<EC>& Uv_919maskUnits,
                                                          Ui_Ut_102321u<EC>& Uv_5width) const //native
  {
    const EventWindowRenderer<EC> & ewr = uc.GetEventWindowRenderer();
    Drawable * d = ewr.getDrawableOrNull();
    MFM_API_ASSERT_NONNULL(d);
    const s32 dps = d->GetDitsPerSite();
    const s32 ups = 1024; // TRACK Drawable.cUNITS_PER_SITE;

    const s32 sx = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_9210startUnits, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 sy = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_9210startUnits, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 ex = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_8endUnits, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 ey = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_8endUnits, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    
    const u32 onColor = Uv_7onColor.read();
    const u32 offColor = Uv_8offColor.read();
    const u32 mask = Uv_6mask32.read();
    const u32 maskUnits = Uv_919maskUnits.read();
    const u32 widthUnits = Uv_5width.read();

    d->DrawScaledMaskedLineDitColor(dps*sx/ups,
                                    dps*sy/ups,
                                    dps*ex/ups,
                                    dps*ey/ups,
                                    onColor,
                                    offColor,
                                    mask,
                                    MAX(1u,((u32)dps)*maskUnits/((u32)ups)), // convert to dits
                                    MAX(1u,((u32)dps)*widthUnits/((u32)ups))); // ditto
    
  }

  template<class EC>
  void Uq_10108Drawable10<EC>::Uf_9213fillRectangle(const UlamContext<EC>& uc, UlamRef<EC>& ur,
                                                    Ui_Uq_102323C2D10<EC>& Uv_3pos,
                                                    Ui_Uq_102323C2D10<EC>& Uv_3siz,
                                                    Ui_Ut_14181u<EC>& Uv_5color) const //native
  {
    const EventWindowRenderer<EC> & ewr = uc.GetEventWindowRenderer();
    Drawable * d = ewr.getDrawableOrNull();
    MFM_API_ASSERT_NONNULL(d);
    const s32 dps = d->GetDitsPerSite();
    const s32 ups = 1024; // TRACK Drawable.cUNITS_PER_SITE;

    const s32 sx = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_3pos, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 sy = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_3pos, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 w = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_3siz, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 h = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_3siz, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);

    const u32 color = Uv_5color.read();

    d->FillRectDit(dps*sx/ups,
                   dps*sy/ups,
                   dps*w/ups,
                   dps*h/ups,
                   color);
  }

} //MFM

