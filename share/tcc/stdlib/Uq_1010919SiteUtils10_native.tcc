/**                                      -*- mode:C++ -*- */

namespace MFM{

  template<class EC>
  Ui_Ut_10121u<EC> Uq_1010919SiteUtils10<EC>::Uf_8getTouch(const UlamContext<EC>& uc, UlamRef<EC>& ur) const
  {
    typedef typename EC::ATOM_CONFIG AC;

    MFM_API_ASSERT_STATE(uc.HasEventWindow());
    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();

    const Site<AC> & site = ew.GetSite();
    return Ui_Ut_10121u<EC>(site.RecentTouch());
  }

  template<class EC>
  Ui_Ut_14181u<EC> Uq_1010919SiteUtils10<EC>::Uf_5getIn(const UlamContext<EC>& uc, UlamRef<EC>& ur) const
  {
    FAIL(INCOMPLETE_CODE);
#if 0
    //! Site.ulam:7:   ARGB getIn() { ARGB a; return a; }

    //! Site.ulam:7:   ARGB getIn() { ARGB a; return a; }
    const u32 Uh_tmpreg_loadable_14 = Uv_1a.read();
    const Ui_Ut_14181u<EC> Uh_tmpval_loadable_15(Uh_tmpreg_loadable_14);
#endif
    Ui_Ut_14181u<EC> Uv_1a;
    return Uv_1a;
  }

  template<class EC>
  Ui_Ut_14181u<EC> Uq_1010919SiteUtils10<EC>::Uf_6getOut(const UlamContext<EC>& uc, UlamRef<EC>& ur) const
  {
    typedef typename EC::ATOM_CONFIG AC;

    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();
    Base<AC> & base = ew.GetBase();

    Ui_Ut_14181u<EC> tmp(base.GetPaint());

    return tmp;
  }

  template<class EC>
  Ui_Ut_14181u<EC> Uq_1010919SiteUtils10<EC>::Uf_6setOut(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_14181u<EC>& Uv_6newVal) const
  {
    typedef typename EC::ATOM_CONFIG AC;

    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();
    Base<AC> & base = ew.GetBase();

    const u32 tmp = Uv_6newVal.read();

    base.SetPaint(tmp);

    return Uv_6newVal;
  }

  template<class EC>
  Ui_Ut_102961a<EC> Uq_1010919SiteUtils10<EC>::Uf_7getBase(const UlamContext<EC>& uc, UlamRef<EC>& ur) const
  {
    typedef typename EC::ATOM_CONFIG AC;

    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();
    Base<AC> & base = ew.GetBase();

    const T atom = base.GetBaseAtom();
    const Ui_Ut_102961a<EC> tmp(atom);
    return tmp;
  } // Uf_7getBase

  template<class EC>
  void Uq_1010919SiteUtils10<EC>::Uf_7setBase(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102961a<EC>& Uv_1a) const
  {
    typedef typename EC::ATOM_CONFIG AC;

    MFM_API_ASSERT_STATE(uc.HasEventWindow());
    EventWindow<EC> & ew = const_cast <UlamContext<EC> &>(uc).GetEventWindow();
    Base<AC> & base = ew.GetBase();

    const T atom = Uv_1a.ReadAtom();

    base.PutBaseAtom(atom);
  } // Uf_7setBase


} //MFM
