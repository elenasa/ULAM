/**                                      -*- mode:C++ -*- */

#include <stdio.h>

namespace MFM{

  template<class CC>
  void Ue_102113Foo10<CC>::Uf_5print(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_10141i Uv_3arg) //native
  {
    s32 tmp = Uv_3arg.read();
    printf("Arg: 0x%x\n", tmp);
  }

  template<class CC>
  void Ue_102113Foo10<CC>::Uf_5print(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_102321i Uv_3arg) //native
  {
    s32 tmp = Uv_3arg.read();
    printf("Arg: %d\n", tmp);
  }

} //MFM
