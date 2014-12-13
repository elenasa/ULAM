/* -*- C++ -*- */
/**
   Uq_10109211EventWindow_native.tcc: EventWindow native methods implementation.
 */

namespace MFM{

  template<class CC, u32 POS>
  Ui_Ut_102964Atom<CC> Uq_10109211EventWindow<CC,POS>::Uf_4aRef(T& Uv_4self, Ui_Ut_102323Int Uv_5index)	 //native
  {
    UlamContext<CC> & uc = UlamContext<CC>::Get();
    s32 siteNumber = Uv_5index.read();

    EventWindow<CC> & ew = uc.GetEventWindow();
    const T & a = ew.GetAtom((u32) siteNumber);

    return Ui_Ut_102964Atom<CC>(a);
  }

  template<class CC, u32 POS>
  void Uq_10109211EventWindow<CC,POS>::Uf_4aSet(T& Uv_4self, Ui_Ut_102323Int Uv_5index, Ui_Ut_102964Atom<CC> Uv_1v) //native
  {
    UlamContext<CC> & uc = UlamContext<CC>::Get();
    s32 siteNumber = Uv_5index.read();

    EventWindow<CC> & ew = uc.GetEventWindow();
    ew.SetAtom((u32) siteNumber, Uv_1v.read());
  }

  template<class CC, u32 POS>
  void Uq_10109211EventWindow<CC,POS>::Uf_7diffuse(T& Uv_4self)	 //native
  {
    UlamContext<CC> & uc = UlamContext<CC>::Get();
    EventWindow<CC> & ew = uc.GetEventWindow();
    ew.Diffuse();
  }

} //MFM
