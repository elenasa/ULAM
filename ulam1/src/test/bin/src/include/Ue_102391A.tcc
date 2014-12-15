/**                                      -*- mode:C++ -*- */

#include "Uq_10106System.h"

namespace MFM{

  template<class CC>
  Ue_102391A<CC>::Ue_102391A() : UlamElement<CC>(MFM_UUID_FOR("A", 0))
  {
    //XXXX  Element<CC>::SetAtomicSymbol("A");  // figure this out later
    Element<CC>::SetName("A");
  }

  template<class CC>
  Ue_102391A<CC>::~Ue_102391A(){}


  //! A.ulam:6: Int test(){Int e, a = 8, f = 7;
  template<class CC>
  Ui_Ut_102323Int Ue_102391A<CC>::Uf_4test(T& Uv_4self) const
  {

    //! A.ulam:6: Int test(){Int e, a = 8, f = 7;

    //! A.ulam:6: Int test(){Int e, a = 8, f = 7;

    //! A.ulam:6: Int test(){Int e, a = 8, f = 7;

    //! A.ulam:6: Int test(){Int e, a = 8, f = 7;

    //! A.ulam:6: Int test(){Int e, a = 8, f = 7;
    Ui_Ut_102323Int Uv_1e;

    //! A.ulam:6: Int test(){Int e, a = 8, f = 7;
    Ui_Ut_102323Int Uv_1a;

    //! A.ulam:6: Int test(){Int e, a = 8, f = 7;
    const s32 Uh_tmpreg_loadable_11 = 8;
    Uv_1a.write(Uh_tmpreg_loadable_11);

    //! A.ulam:6: Int test(){Int e, a = 8, f = 7;
    Ui_Ut_102323Int Uv_1f;

    //! A.ulam:6: Int test(){Int e, a = 8, f = 7;
    const s32 Uh_tmpreg_loadable_13 = 7;
    Uv_1f.write(Uh_tmpreg_loadable_13);

    //! A.ulam:7: d = 1;
    const s32 Uh_tmpreg_loadable_15 = 1;
    Up_Um_1d::Write(Uv_4self.GetBits(), Uh_tmpreg_loadable_15);

    //! A.ulam:8: while(a){
    while(true)
    {
      const u32 Uh_tmpreg_loadable_17 = Uv_1a.read();
      const s32 Uh_tmpreg_loadable_18 = _SignExtend32(Uh_tmpreg_loadable_17, 32);
      const u32 Uh_tmpreg_loadable_19 = _Int32ToBool32(Uh_tmpreg_loadable_18, 32, 1);

      if(!_Bool32ToCbool(Uh_tmpreg_loadable_19, 1))
        break;

      //! A.ulam:8: while(a){
      {

        //! A.ulam:9: d = d << 1;
        const s32 Uh_tmpreg_loadable_210 = 1;
        const u32 Uh_tmpreg_loadable_211 = Up_Um_1d::Read(Uv_4self.GetBits());
        const s32 Uh_tmpreg_loadable_212 = _SignExtend32(Uh_tmpreg_loadable_211, 32);
        const s32 Uh_tmpreg_loadable_213 = _ShiftOpLeftInt32(Uh_tmpreg_loadable_212, Uh_tmpreg_loadable_210, 32);
        Up_Um_1d::Write(Uv_4self.GetBits(), Uh_tmpreg_loadable_213);

        //! A.ulam:10: s.print(d);
        const u32 Uh_tmpreg_loadable_216 = Up_Um_1d::Read(Uv_4self.GetBits());
        const s32 Uh_tmpreg_loadable_217 = _SignExtend32(Uh_tmpreg_loadable_216, 32);
        const Ui_Ut_102323Int Uh_tmpval_loadable_218(Uh_tmpreg_loadable_217);
        Ut_Um_1s::Uf_5print(Uv_4self, Uh_tmpval_loadable_218);

        //! A.ulam:11: a = a >> 1;
        const s32 Uh_tmpreg_loadable_219 = 1;
        const u32 Uh_tmpreg_loadable_220 = Uv_1a.read();
        const s32 Uh_tmpreg_loadable_221 = _SignExtend32(Uh_tmpreg_loadable_220, 32);
        const s32 Uh_tmpreg_loadable_222 = _ShiftOpRightInt32(Uh_tmpreg_loadable_221, Uh_tmpreg_loadable_219, 32);
        Uv_1a.write(Uh_tmpreg_loadable_222);
      }
    } // end while

    //! A.ulam:13: return d;
    const u32 Uh_tmpreg_loadable_224 = Up_Um_1d::Read(Uv_4self.GetBits());
    const s32 Uh_tmpreg_loadable_225 = _SignExtend32(Uh_tmpreg_loadable_224, 32);
    const Ui_Ut_102323Int Uh_tmpval_loadable_226(Uh_tmpreg_loadable_225);
    return (Uh_tmpval_loadable_226);

  } // Uf_4test


  template<class CC>
  s32 Ue_102391A<CC>::PositionOfDataMemberType(const char * namearg) const
  {
    if(!strcmp(namearg,"System")) return (0);   //pos offset

    return (-1);   //not found
  }  //has

  template<class CC>
  bool Ue_102391A<CC>::internalCMethodImplementingIs(const T& targ) const
  {
    return (THE_INSTANCE.GetType() == targ.GetType());
  }   //is

} //MFM

