/**                                      -*- mode:C++ -*- */

namespace MFM{

  template<class EC, u32 POS>
  Ui_Ut_10121u Uq_1010919SiteUtils10<EC, POS>::Uf_8getTouch(const UlamContext<EC>& uc, T& Uv_4self)
  {
    typedef typename EC::ATOM_CONFIG AC;
    const Site<AC> & site = uc.GetSite();
    return Ui_Ut_10121u(site.RecentTouch());

  }

  template<class EC, u32 POS>
  Ui_Ut_14181u Uq_1010919SiteUtils10<EC, POS>::Uf_5getIn(const UlamContext<EC>& uc, T& Uv_4self)
  {
    FAIL(INCOMPLETE_CODE);
#if 0
    //! Site.ulam:7:   ARGB getIn() { ARGB a; return a; }

    //! Site.ulam:7:   ARGB getIn() { ARGB a; return a; }
    const u32 Uh_tmpreg_loadable_14 = Uv_1a.read();
    const Ui_Ut_14181u Uh_tmpval_loadable_15(Uh_tmpreg_loadable_14);
#endif
    Ui_Ut_14181u Uv_1a;
    return Uv_1a;

  }

  template<class EC, u32 POS>
  Ui_Ut_14181u Uq_1010919SiteUtils10<EC, POS>::Uf_6getOut(const UlamContext<EC>& uc, T& Uv_4self)
  {
    typedef typename EC::ATOM_CONFIG AC;

    EventWindow<EC> & ew = uc.GetEventWindow();
    Base<AC> & base = ew.GetBase();

    Ui_Ut_14181u tmp(base.GetPaint());

    return tmp;

  }

  template<class EC, u32 POS>
  Ui_Ut_14181u Uq_1010919SiteUtils10<EC, POS>::Uf_6setOut(const UlamContext<EC>& uc, T& Uv_4self, Ui_Ut_14181u Uv_6newVal)
  {
    typedef typename EC::ATOM_CONFIG AC;

    EventWindow<EC> & ew = uc.GetEventWindow();
    Base<AC> & base = ew.GetBase();

    const u32 tmp = Uv_6newVal.read();

    base.SetPaint(tmp);

    return Uv_6newVal;

  }

  template<class EC, u32 POS>
  Ui_Ut_102961a<EC> Uq_1010919SiteUtils10<EC, POS>::Uf_7getBase(const UlamContext<EC>& uc, T& Uv_4self)
  {
    typedef typename EC::ATOM_CONFIG AC;

    EventWindow<EC> & ew = uc.GetEventWindow();
    Base<AC> & base = ew.GetBase();

    const T atom = base.GetBaseAtom();
    const Ui_Ut_102961a<EC> tmp(atom);
    return tmp;

  } // Uf_7getBase

  template<class EC, u32 POS>
  void Uq_1010919SiteUtils10<EC, POS>::Uf_7setBase(const UlamContext<EC>& uc, T& Uv_4self, Ui_Ut_102961a<EC> Uv_1a)
  {
    typedef typename EC::ATOM_CONFIG AC;

    EventWindow<EC> & ew = uc.GetEventWindow();
    Base<AC> & base = ew.GetBase();

    const T atom = Uv_1a.read();

    base.PutBaseAtom(atom);

  } // Uf_7setBase


} //MFM
