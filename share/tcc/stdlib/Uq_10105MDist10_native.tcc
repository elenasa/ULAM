/* -*- C++ -*- */

#include "UlamDefs.h"
#include "MDist.h"

namespace MFM {

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10105MDist10<EC>::Uf_9212getLastIndex(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10131u<EC>& Uv_6radius) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };

    u32 radius = Uv_6radius.read();
    Ui_Ut_10161u<EC> ret(MDist<R>::get().GetLastIndex(radius));
    return ret;
  }

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10105MDist10<EC>::Uf_9213getFirstIndex(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10131u<EC>& Uv_6radius) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };

    u32 radius = Uv_6radius.read();
    Ui_Ut_10161u<EC> ret(MDist<R>::get().GetFirstIndex(radius));
    return ret;
  }

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10105MDist10<EC>::Uf_9216getFirstESLIndex(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_10151u<EC>& Uv_919eslRadius) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    u32 eslRadius = Uv_919eslRadius.read();
    Ui_Ut_10161u<EC> ret(MDist<R>::get().GetFirstESLIndex(eslRadius));
    return ret;
  }

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10105MDist10<EC>::Uf_9215getLastESLIndex(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_10151u<EC>& Uv_919eslRadius) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    u32 eslRadius = Uv_919eslRadius.read();
    Ui_Ut_10161u<EC> ret(MDist<R>::get().GetLastESLIndex(eslRadius));
    return ret;
  }

  template<class EC>
  Ui_Uq_102323C2D10<EC> Uq_10105MDist10<EC>::Uf_9212getSiteCoord(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_10161u<EC>& Uv_9210siteNumber) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    u32 sn = Uv_9210siteNumber.read();
    SPoint coord = MDist<R>::get().GetPoint(sn);
    Ui_Uq_102323C2D10<EC> ret;
    UlamRef<EC>(0u, 16u, ret, NULL, UlamRef<EC>::PRIMITIVE, uc).Write(coord.GetX());
    UlamRef<EC>(16u, 16u, ret, NULL, UlamRef<EC>::PRIMITIVE, uc).Write(coord.GetY());
    return ret;
  }

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10105MDist10<EC>::Uf_9213getSiteNumber(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Uq_102323C2D10<EC>& Uv_5coord) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    const s32 x = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_5coord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 y = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_5coord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const SPoint loc(x,y);

    s32 sn = MDist<R>::get().GetSiteNumber(loc);

    Ui_Ut_10161u<EC> ret(sn < 0 ? (1<<6)-1 : (u32) sn);
    return ret;
  }

  template<class EC>
  Ui_Uq_102323C2D10<EC> Uq_10105MDist10<EC>::Uf_6symMap(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Uq_102323C2D10<EC>& Uv_2pt, Ui_Ut_10131u<EC>& Uv_3sym) const
  {
    const s32 x = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_2pt, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 y = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_2pt, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const SPoint loc(x,y);
    u32 sym = Uv_3sym.read(); //gcnl:Node.cpp:832
    SPoint pt = SymMap(loc, (const PointSymmetry) sym, loc);
    Ui_Uq_102323C2D10<EC> ret;
    UlamRef<EC>(0u, 16u, ret, NULL, UlamRef<EC>::PRIMITIVE, uc).Write(pt.GetX());
    UlamRef<EC>(16u, 16u, ret, NULL, UlamRef<EC>::PRIMITIVE, uc).Write(pt.GetY());
    return ret;
  } // Uf_6symMap

} //MFM
