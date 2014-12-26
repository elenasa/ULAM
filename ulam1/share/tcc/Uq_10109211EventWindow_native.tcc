/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods implementation.
 */

namespace MFM{

  template<class CC, u32 POS>
  Ui_Ut_102964Atom<CC> Uq_10109211EventWindow<CC,POS>::Uf_4aref(UlamContext<CC> & uc,
                                                                T& Uv_4self, Ui_Ut_10168Unsigned Uv_5index)	 //native
  {
    u32 siteNumber = Uv_5index.read();

    EventWindow<CC> & ew = uc.GetEventWindow();
    const T & a = ew.GetAtomSym(siteNumber);

    return Ui_Ut_102964Atom<CC>(a);
  }

  template<class CC, u32 POS>
  void Uq_10109211EventWindow<CC,POS>::Uf_4aset(UlamContext<CC> & uc,
                                                T& Uv_4self, Ui_Ut_10168Unsigned Uv_5index, Ui_Ut_102964Atom<CC> Uv_1v) //native
  {
    u32 siteNumber = Uv_5index.read();

    EventWindow<CC> & ew = uc.GetEventWindow();
    ew.SetAtomSym(siteNumber, Uv_1v.read());
  }

  template<class CC, u32 POS>
  Ui_Ut_10114Bool Uq_10109211EventWindow<CC,POS>::Uf_6isLive(UlamContext<CC> & uc,
                                                             T& Uv_4self, Ui_Ut_10168Unsigned Uv_5index)
  {
    u32 siteNumber = Uv_5index.read();
    EventWindow<CC> & ew = uc.GetEventWindow();
    return Ui_Ut_10114Bool(ew.IsLiveSiteSym(siteNumber));
  }

  template<class CC, u32 POS>
  Ui_Ut_10114Bool Uq_10109211EventWindow<CC,POS>::Uf_4swap(UlamContext<CC> & uc,
                                                           T& Uv_4self, Ui_Ut_10168Unsigned Uv_6index1, Ui_Ut_10168Unsigned Uv_6index2)
  {
    u32 idx1 = Uv_6index1.read();
    u32 idx2 = Uv_6index2.read();
    EventWindow<CC> & ew = uc.GetEventWindow();
    if (!ew.IsLiveSiteSym(idx1) || !ew.IsLiveSiteSym(idx2))
      return Ui_Ut_10114Bool(false);
    ew.SwapAtomsSym(idx1,idx2);
    return Ui_Ut_10114Bool(true);
  }

  template<class CC, u32 POS>
  Ui_Uq_102323C2D<CC> Uq_10109211EventWindow<CC, POS>::Uf_8getCoord(UlamContext<CC> & uc,
                                                                    T& Uv_4self, Ui_Ut_10168Unsigned Uv_7siteNum)
  {
    //! EventWindow.ulam:21:     C2D ret;
    Ui_Uq_102323C2D<CC> Uv_3ret;

    EventWindow<CC> & ew = uc.GetEventWindow();

    u32 idx = Uv_7siteNum.read();
    SPoint loc = ew.MapToPointSymValid(idx);

    Ui_Uq_102323C2D<CC>::Us::Up_Um_1x::Write(Uv_3ret.getBits(), loc.GetX());
    Ui_Uq_102323C2D<CC>::Us::Up_Um_1y::Write(Uv_3ret.getBits(), loc.GetY());

    //! EventWindow.ulam:24:     return ret;
    const u32 Uh_tmpreg_loadable_240 = Uv_3ret.read();
    const Ui_Uq_102323C2D<CC> Uh_tmpval_loadable_241(Uh_tmpreg_loadable_240);
    return (Uh_tmpval_loadable_241);

  } // Uf_8getCoord



  template<class CC, u32 POS>
  Ui_Ut_10168Unsigned Uq_10109211EventWindow<CC, POS>::Uf_9213getSiteNumber(UlamContext<CC> & uc,
                                                                            T& Uv_4self, Ui_Uq_102323C2D<CC> Uv_5coord)
  {
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

    EventWindow<CC> & ew = uc.GetEventWindow();

    const s32 x = _SignExtend32(Ui_Uq_102323C2D<CC>::Us::Up_Um_1x::Read(Uv_5coord.getBits()), 16);
    const s32 y = _SignExtend32(Ui_Uq_102323C2D<CC>::Us::Up_Um_1y::Read(Uv_5coord.getBits()), 16);
    const SPoint loc(x,y);
    u32 ret;
    if (ew.InWindow(loc))
      ret = ew.MapToIndexSymValid(loc);
    else
      ret = EventWindow<CC>::SITE_COUNT;

    return Ui_Ut_10168Unsigned(ret);
  }

  //! EventWindow.ulam:28:   SiteNum size() native;
  template<class CC, u32 POS>
  Ui_Ut_10168Unsigned Uq_10109211EventWindow<CC,POS>::Uf_4size(UlamContext<CC> & uc,
                                                               T& Uv_4self) {
    return Ui_Ut_10168Unsigned(EventWindow<CC>::SITE_COUNT);
  }

  template<class CC, u32 POS>
  Ui_Ut_10138Unsigned Uq_10109211EventWindow<CC, POS>::Uf_9214changeSymmetry(UlamContext<CC> & uc,
                                                                             T& Uv_4self, Ui_Ut_10138Unsigned Uv_6newSym)
  {
    EventWindow<CC> & ew = uc.GetEventWindow();

    const u32 oldSym = (u32) ew.GetSymmetry();
    const u32 newSym = (u32) Uv_6newSym.read();
    if (newSym < PSYM_SYMMETRY_COUNT)
      ew.SetSymmetry((PointSymmetry) newSym);
    return Ui_Ut_10138Unsigned(oldSym);

  } // Uf_9214changeSymmetry

  //! EventWindow.ulam:34:   C2D mapSym(C2D directCoord) {
  template<class CC, u32 POS>
  Ui_Uq_102323C2D<CC> Uq_10109211EventWindow<CC, POS>::Uf_6mapSym(UlamContext<CC> & uc,
                                                                  T& Uv_4self, Ui_Uq_102323C2D<CC> Uv_9211directCoord)
  {
    EventWindow<CC> & ew = uc.GetEventWindow();

    const s32 x = _SignExtend32(Ui_Uq_102323C2D<CC>::Us::Up_Um_1x::Read(Uv_9211directCoord.getBits()), 16);
    const s32 y = _SignExtend32(Ui_Uq_102323C2D<CC>::Us::Up_Um_1y::Read(Uv_9211directCoord.getBits()), 16);
    const SPoint direct(x,y);

    SPoint mapped = SymMap(direct,ew.GetSymmetry(),direct);

    Ui_Uq_102323C2D<CC> Uv_3ret;

    Ui_Uq_102323C2D<CC>::Us::Up_Um_1x::Write(Uv_3ret.getBits(), mapped.GetX());
    Ui_Uq_102323C2D<CC>::Us::Up_Um_1y::Write(Uv_3ret.getBits(), mapped.GetY());

    //! EventWindow.ulam:38:     return ret;
    return Ui_Uq_102323C2D<CC>(Uv_3ret.read());

  } // Uf_6mapSym


  template<class CC, u32 POS>
  void Uq_10109211EventWindow<CC,POS>::Uf_7diffuse(UlamContext<CC> & uc,
                                                   T& Uv_4self)	 //native
  {
    EventWindow<CC> & ew = uc.GetEventWindow();
    ew.Diffuse();
  }


} //MFM
