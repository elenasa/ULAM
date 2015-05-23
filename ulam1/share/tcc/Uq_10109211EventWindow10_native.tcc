/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods implementation.
 */

namespace MFM{

  template<class EC, u32 POS>
  Ui_Ut_102961a<EC> Uq_10109211EventWindow10<EC,POS>::Uf_4aref(UlamContext<EC> & uc, T& Uv_4self,
							      Ui_Ut_10161u Uv_5index) //native
  {
    u32 siteNumber = Uv_5index.read();
    EventWindow<EC> & ew = uc.GetEventWindow();
    const T & a = ew.GetAtomSym(siteNumber);
    return Ui_Ut_102961a<EC>(a);
  }

  template<class EC, u32 POS>
  void Uq_10109211EventWindow10<EC,POS>::Uf_4aset(UlamContext<EC> & uc,
                                                T& Uv_4self, Ui_Ut_10161u Uv_5index,
                                                Ui_Ut_102961a<EC> Uv_1v) //native
  {
    u32 siteNumber = Uv_5index.read();
    EventWindow<EC> & ew = uc.GetEventWindow();
    ew.SetAtomSym(siteNumber, Uv_1v.read());
  }

  template<class EC, u32 POS>
  Ui_Ut_10111b Uq_10109211EventWindow10<EC,POS>::Uf_6isLive(UlamContext<EC> & uc,
                                                             T& Uv_4self, Ui_Ut_10161u Uv_5index)
  {
    u32 siteNumber = Uv_5index.read();
    EventWindow<EC> & ew = uc.GetEventWindow();
    return Ui_Ut_10111b(ew.IsLiveSiteSym(siteNumber));
  }

  template<class EC, u32 POS>
  Ui_Ut_10111b Uq_10109211EventWindow10<EC,POS>::Uf_4swap(UlamContext<EC> & uc, T& Uv_4self,
							 Ui_Ut_10161u Uv_6index1,
							 Ui_Ut_10161u Uv_6index2)
  {
    u32 idx1 = Uv_6index1.read();
    u32 idx2 = Uv_6index2.read();
    EventWindow<EC> & ew = uc.GetEventWindow();
    if (!ew.IsLiveSiteSym(idx1) || !ew.IsLiveSiteSym(idx2))
      return Ui_Ut_10111b(false);
    ew.SwapAtomsSym(idx1,idx2);
    return Ui_Ut_10111b(true);
  }

  template<class EC, u32 POS>
  Ui_Uq_102323C2D10<EC> Uq_10109211EventWindow10<EC, POS>::Uf_8getCoord(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_10161u Uv_7siteNum)
  {
    //! EventWindow.ulam:21:     C2D ret;
    Ui_Uq_102323C2D10<EC> Uv_3ret;
    EventWindow<EC> & ew = uc.GetEventWindow();
    u32 idx = Uv_7siteNum.read();
    SPoint loc = ew.MapToPointSymValid(idx);

    Ui_Uq_102323C2D10<EC>::Us::Up_Um_1x::Write(Uv_3ret.getBits(), loc.GetX());
    Ui_Uq_102323C2D10<EC>::Us::Up_Um_1y::Write(Uv_3ret.getBits(), loc.GetY());

    //! EventWindow.ulam:24:     return ret;
    const u32 Uh_tmpreg_loadable_240 = Uv_3ret.read();
    const Ui_Uq_102323C2D10<EC> Uh_tmpval_loadable_241(Uh_tmpreg_loadable_240);
    return (Uh_tmpval_loadable_241);
  } // Uf_8getCoord

  template<class EC, u32 POS>
  Ui_Ut_10161u Uq_10109211EventWindow10<EC, POS>::Uf_9213getSiteNumber(UlamContext<EC> & uc, T& Uv_4self, Ui_Uq_102323C2D10<EC> Uv_5coord)
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    EventWindow<EC> & ew = uc.GetEventWindow();

    const s32 x = _SignExtend32(Ui_Uq_102323C2D10<EC>::Us::Up_Um_1x::Read(Uv_5coord.getBits()), 16);
    const s32 y = _SignExtend32(Ui_Uq_102323C2D10<EC>::Us::Up_Um_1y::Read(Uv_5coord.getBits()), 16);
    const SPoint loc(x,y);
    u32 ret;
    if (ew.InWindow(loc))
      ret = ew.MapToIndexSymValid(loc);
    else
      ret = EventWindow<EC>::SITE_COUNT;
    return Ui_Ut_10161u(ret);
  }

  //! EventWindow.ulam:28:   SiteNum size() native;
  template<class EC, u32 POS>
  Ui_Ut_10161u Uq_10109211EventWindow10<EC,POS>::Uf_4size(UlamContext<EC> & uc,
                                                               T& Uv_4self) {
    return Ui_Ut_10161u(EventWindow<EC>::SITE_COUNT);
  }

  template<class EC, u32 POS>
  Ui_Ut_10131u Uq_10109211EventWindow10<EC, POS>::Uf_9214changeSymmetry(UlamContext<EC> & uc,T& Uv_4self,
								       Ui_Ut_10131u Uv_6newSym)
  {
    EventWindow<EC> & ew = uc.GetEventWindow();

    const u32 oldSym = (u32) ew.GetSymmetry();
    const u32 newSym = (u32) Uv_6newSym.read();
    if (newSym < PSYM_SYMMETRY_COUNT)
      ew.SetSymmetry((PointSymmetry) newSym);
    return Ui_Ut_10131u(oldSym);
  } // Uf_9214changeSymmetry

  //! EventWindow.ulam:34:   C2D mapSym(C2D directCoord) {
  template<class EC, u32 POS>
  Ui_Uq_102323C2D10<EC> Uq_10109211EventWindow10<EC, POS>::Uf_6mapSym(UlamContext<EC> & uc, T& Uv_4self,
								      Ui_Uq_102323C2D10<EC> Uv_9211directCoord)
  {
    EventWindow<EC> & ew = uc.GetEventWindow();

    const s32 x = _SignExtend32(Ui_Uq_102323C2D10<EC>::Us::Up_Um_1x::Read(Uv_9211directCoord.getBits()), 16);
    const s32 y = _SignExtend32(Ui_Uq_102323C2D10<EC>::Us::Up_Um_1y::Read(Uv_9211directCoord.getBits()), 16);
    const SPoint direct(x,y);

    SPoint mapped = SymMap(direct,ew.GetSymmetry(),direct);

    Ui_Uq_102323C2D10<EC> Uv_3ret;

    Ui_Uq_102323C2D10<EC>::Us::Up_Um_1x::Write(Uv_3ret.getBits(), mapped.GetX());
    Ui_Uq_102323C2D10<EC>::Us::Up_Um_1y::Write(Uv_3ret.getBits(), mapped.GetY());

    //! EventWindow.ulam:38:     return ret;
    return Ui_Uq_102323C2D10<EC>(Uv_3ret.read());
  } // Uf_6mapSym


  template<class EC, u32 POS>
  void Uq_10109211EventWindow10<EC,POS>::Uf_7diffuse(UlamContext<EC> & uc,
                                                   T& Uv_4self)	 //native
  {
    EventWindow<EC> & ew = uc.GetEventWindow();
    ew.Diffuse();
  }


} //MFM
