/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods implementation.
 */

namespace MFM{

  template<class EC>
  Ui_Ut_r102961a<EC> Uq_10109211EventWindow10<EC>::Uf_4aref(const UlamContext<EC> & uc, UlamRef<EC>& ur,
							       Ui_Ut_10161u<EC>& Uv_5index) const //native
  {
    u32 siteNumber = Uv_5index.read();
    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();
    return Ui_Ut_r102961a<EC>(ew.GetAtomBitStorage(siteNumber), 0, uc);
  }

  template<class EC>
  Ui_Ut_10111b<EC> Uq_10109211EventWindow10<EC>::Uf_6isLive(const UlamContext<EC> & uc,
								UlamRef<EC>& ur, Ui_Ut_10161u<EC>& Uv_5index) const
  {
    u32 siteNumber = Uv_5index.read();
    const EventWindow<EC> & ew = uc.GetEventWindow();
    return Ui_Ut_10111b<EC>(ew.IsLiveSiteSym(siteNumber));
  }

  template<class EC>
  Ui_Ut_10111b<EC> Uq_10109211EventWindow10<EC>::Uf_4swap(const UlamContext<EC> & uc, UlamRef<EC>& ur,
							      Ui_Ut_10161u<EC>& Uv_6index1,
							      Ui_Ut_10161u<EC>& Uv_6index2) const
  {
    u32 idx1 = Uv_6index1.read();
    u32 idx2 = Uv_6index2.read();
    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();
    if (!ew.IsLiveSiteSym(idx1) || !ew.IsLiveSiteSym(idx2))
      return Ui_Ut_10111b<EC>(false);
    ew.SwapAtomsSym(idx1,idx2);
    return Ui_Ut_10111b<EC>(true);
  }

  template<class EC>
  Ui_Uq_102323C2D10<EC> Uq_10109211EventWindow10<EC>::Uf_8getCoord(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10161u<EC>& Uv_7siteNum) const
  {
    //! EventWindow.ulam:21:     C2D ret;
    Ui_Uq_102323C2D10<EC> Uv_3ret;
    const EventWindow<EC> & ew = uc.GetEventWindow();
    u32 idx = Uv_7siteNum.read();
    SPoint loc = ew.MapToPointSymValid(idx);

    UlamRef<EC>(0u, 16u, Uv_3ret, NULL, UlamRef<EC>::PRIMITIVE, uc).Write(loc.GetX());
    UlamRef<EC>(16u, 16u, Uv_3ret, NULL, UlamRef<EC>::PRIMITIVE, uc).Write(loc.GetY());

    //! EventWindow.ulam:24:     return ret;
    const u32 Uh_tmpreg_loadable_240 = Uv_3ret.read();
    const Ui_Uq_102323C2D10<EC> Uh_tmpval_loadable_241(Uh_tmpreg_loadable_240);
    return (Uh_tmpval_loadable_241);
  } // Uf_8getCoord

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10109211EventWindow10<EC>::Uf_9213getSiteNumber(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Uq_102323C2D10<EC>& Uv_5coord) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    const EventWindow<EC> & ew = uc.GetEventWindow();

    const s32 x = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_5coord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 y = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_5coord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const SPoint loc(x,y);
    u32 ret;
    if (ew.InWindow(loc))
      ret = ew.MapToIndexSymValid(loc);
    else
      ret = EventWindow<EC>::SITE_COUNT;
    return Ui_Ut_10161u<EC>(ret);
  }

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10109211EventWindow10<EC>::Uf_9216getSiteNumberRaw(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Uq_102323C2D10<EC>& Uv_5coord) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    const EventWindow<EC> & ew = uc.GetEventWindow();

    const s32 x = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_5coord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 y = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_5coord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const SPoint loc(x,y);
    u32 ret;
    if (ew.InWindow(loc))
      ret = ew.MapToIndexDirectValid(loc);
    else
      ret = EventWindow<EC>::SITE_COUNT;
    return Ui_Ut_10161u<EC>(ret);
  }

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10109211EventWindow10<EC>::Uf_9228getSiteNumberFromRasterIndex(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_10161u<EC>& Uv_5index) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    const MDist<R> & md = MDist<R>::get();
    s32 sn = md.GetSiteNumberFromRasterIndex(Uv_5index.read());
    u32 ret;
    if (sn < 0) ret = EventWindow<EC>::SITE_COUNT;
    else ret = (u32) sn;
    return Ui_Ut_10161u<EC>(ret);
  } // Uf_9228getSiteNumberFromRasterIndex

  template<class EC>
  Ui_Ut_10161u<EC> Uq_10109211EventWindow10<EC>::Uf_9228getRasterIndexFromSiteNumber(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_10161u<EC>& Uv_7sitenum) const
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    const MDist<R> & md = MDist<R>::get();
    s32 idx = md.GetRasterIndexFromSiteNumber(Uv_7sitenum.read());
    u32 ret;
    if (idx < 0) ret = EventWindow<EC>::SITE_COUNT;
    else ret = (u32) idx;
    return Ui_Ut_10161u<EC>(ret);

  } // Uf_9228getRasterIndexFromSiteNumber


  //! EventWindow.ulam:28:   SiteNum size() native;
  template<class EC>
  Ui_Ut_10161u<EC> Uq_10109211EventWindow10<EC>::Uf_4size(const UlamContext<EC> & uc,
                                                               UlamRef<EC>& ur) const
  {
    return Ui_Ut_10161u<EC>(EventWindow<EC>::SITE_COUNT);
  }

  template<class EC>
  Ui_Ut_10131u<EC> Uq_10109211EventWindow10<EC>::Uf_9211getSymmetry(const UlamContext<EC> & uc,
                                                                    UlamRef<EC>& ur) const
  {
    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();

    const u32 sym = (u32) ew.GetSymmetry();
    return Ui_Ut_10131u<EC>(sym);
  }

  //! EventWindow.ulam:45:   Unsigned getRadius() native;
  template<class EC>
  Ui_Ut_102321u<EC> Uq_10109211EventWindow10<EC>::Uf_919getRadius(const UlamContext<EC> & uc,
                                                                 UlamRef<EC>& ur) const
  {
    const EventWindow<EC> & ew = uc.GetEventWindow();
    u32 boundary = ew.GetBoundary();
    if (boundary > 0) --boundary; // radius is one less than boundary
    return Ui_Ut_102321u<EC>(boundary);
  }

  template<class EC>
  Ui_Ut_10131u<EC> Uq_10109211EventWindow10<EC>::Uf_9214changeSymmetry(const UlamContext<EC> & uc,UlamRef<EC>& ur,
									    Ui_Ut_10131u<EC>& Uv_6newSym) const
  {
    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();

    const u32 oldSym = (u32) ew.GetSymmetry();
    const u32 newSym = (u32) Uv_6newSym.read();
    if (newSym < PSYM_SYMMETRY_COUNT)
      ew.SetSymmetry((PointSymmetry) newSym);
    return Ui_Ut_10131u<EC>(oldSym);
  } // Uf_9214changeSymmetry

  //! EventWindow.ulam:34:   C2D mapSym(C2D directCoord) {
  template<class EC>
  Ui_Uq_102323C2D10<EC> Uq_10109211EventWindow10<EC>::Uf_6mapSym(const UlamContext<EC> & uc, UlamRef<EC>& ur,
								      Ui_Uq_102323C2D10<EC>& Uv_9211directCoord) const
  {
    const EventWindow<EC> & ew = uc.GetEventWindow();

    const s32 x = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_9211directCoord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 y = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_9211directCoord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);

    const SPoint direct(x,y);

    SPoint mapped = SymMap(direct,ew.GetSymmetry(),direct);

    Ui_Uq_102323C2D10<EC> Uv_3ret;

    UlamRef<EC>(0u, 16u, Uv_3ret, NULL, UlamRef<EC>::PRIMITIVE, uc).Write(mapped.GetX());

    UlamRef<EC>(16u, 16u, Uv_3ret, NULL, UlamRef<EC>::PRIMITIVE, uc).Write(mapped.GetY());

    //! EventWindow.ulam:38:     return ret;
    return Ui_Uq_102323C2D10<EC>(Uv_3ret.read());
  } // Uf_6mapSym


  //! EventWindow.ulam:178:   SiteNum getSiteNumber(Atom& ar, Symmetry sym) {
  template<class EC>
  Ui_Ut_10161u<EC> Uq_10109211EventWindow10<EC>::Uf_9213getSiteNumber(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_r102961a<EC>& Ur_2ar, Ui_Ut_10131u<EC>& Uv_3sym) const
  {

    const u32 maxof = 63u;
    Ui_Ut_10161u<EC> failret(maxof); 

    if (!uc.HasEventWindow()) return failret;

    BitStorage<EC> * ptrstg = &(Ur_2ar.GetStorage());
    const EventWindow<EC> & ew = uc.GetEventWindow();

    enum { R = EC::EVENT_WINDOW_RADIUS };
    const MDist<R> & md = MDist<R>::get();

    u32 rad = ew.GetBoundary();
    if (rad > 0) --rad; // radius is one less than boundary

    u32 lastIdx = md.GetFirstIndex(rad + 1u);

    const AtomBitStorage<EC> * first = &(ew.GetAtomBitStorage(0));
    const AtomBitStorage<EC> * last = first + lastIdx;

    if (ptrstg < first || ptrstg >= last) return failret;

    // if ptrstg pointed at a const AtomBitStorage<EC> in ew, which one would it be?
    u32 rawsn = (u32) (((const AtomBitStorage<EC> *) ptrstg) - first);

    PointSymmetry sym = (PointSymmetry) Uv_3sym.read();
    u32 mappedsn = ew.MapIndexToIndexSymValid(rawsn,SymInverse(sym));

    Ui_Ut_10161u<EC> ret(mappedsn);
    return ret;

  } // Uf_9213getSiteNumber

  template<class EC>
  void Uq_10109211EventWindow10<EC>::Uf_7diffuse(const UlamContext<EC> & uc,
                                                   UlamRef<EC>& ur) const	 //native
  {
    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();
    ew.Diffuse();
  }


} //MFM
