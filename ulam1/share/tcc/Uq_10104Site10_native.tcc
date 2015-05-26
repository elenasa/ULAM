/**                                      -*- mode:C++ -*- */

#include "Uq_10109210ColorUtils0.h"

namespace MFM{

  template<class EC, u32 POS>
  Ui_Ut_102321u Uq_10104Site10<EC, POS>::Uf_8getTouch(UlamContext<EC>& uc, T& Uv_4self)
  {
    typedef typename EC::ATOM_CONFIG AC;
    Base<AC> & base = uc.GetBase();
    u64 en = base.GetLastEventEventNumber();
    SiteSensors & ss = base.GetSensory();
    return Ui_Ut_102321u(ss.RecentTouch(en));

  } // Uf_8getTouch

  //! Site.ulam:7:   ARGB getIn() { ARGB a; return a; }
  template<class EC, u32 POS>
  Ui_Ut_14181u Uq_10104Site10<EC, POS>::Uf_5getIn(UlamContext<EC>& uc, T& Uv_4self)
  {

    //! Site.ulam:7:   ARGB getIn() { ARGB a; return a; }
    Ui_Ut_14181u Uv_1a;

    //! Site.ulam:7:   ARGB getIn() { ARGB a; return a; }
    const u32 Uh_tmpreg_loadable_14 = Uv_1a.read();
    const Ui_Ut_14181u Uh_tmpval_loadable_15(Uh_tmpreg_loadable_14);
    return (Uh_tmpval_loadable_15);

  } // Uf_5getIn



  //! Site.ulam:8:   ARGB getOut() { ARGB a; return a; }
  template<class EC, u32 POS>
  Ui_Ut_14181u Uq_10104Site10<EC, POS>::Uf_6getOut(UlamContext<EC>& uc, T& Uv_4self)
  {

    //! Site.ulam:8:   ARGB getOut() { ARGB a; return a; }
    Ui_Ut_14181u Uv_1a;

    //! Site.ulam:8:   ARGB getOut() { ARGB a; return a; }
    const u32 Uh_tmpreg_loadable_16 = Uv_1a.read();
    const Ui_Ut_14181u Uh_tmpval_loadable_17(Uh_tmpreg_loadable_16);
    return (Uh_tmpval_loadable_17);

  } // Uf_6getOut



  //! Site.ulam:9:   ARGB setOut(ARGB newVal) { ARGB a; return a; }
  template<class EC, u32 POS>
  Ui_Ut_14181u Uq_10104Site10<EC, POS>::Uf_6setOut(UlamContext<EC>& uc, T& Uv_4self, Ui_Ut_14181u Uv_6newVal)
  {

    //! Site.ulam:9:   ARGB setOut(ARGB newVal) { ARGB a; return a; }
    Ui_Ut_14181u Uv_1a;

    //! Site.ulam:9:   ARGB setOut(ARGB newVal) { ARGB a; return a; }
    const u32 Uh_tmpreg_loadable_18 = Uv_1a.read();
    const Ui_Ut_14181u Uh_tmpval_loadable_19(Uh_tmpreg_loadable_18);
    return (Uh_tmpval_loadable_19);

  } // Uf_6setOut


} //MFM
