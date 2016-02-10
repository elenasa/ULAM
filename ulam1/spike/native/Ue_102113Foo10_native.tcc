/**                                      -*- mode:C++ -*- */

#include <stdio.h>

namespace MFM{

  template<class CC>
  void Ue_102113Foo10<CC>::Uf_5print(const UlamContext<CC>& uc, const UlamRef<CC>& ur, Ui_Ut_10141i<CC> Uv_3arg) const //native
  {
    s32 tmp = Uv_3arg.read();
    printf("Arg: 0x%x\n", tmp);
  }

  template<class CC>
  void Ue_102113Foo10<CC>::Uf_5print(const UlamContext<CC>& uc, const UlamRef<CC>& ur, Ui_Ut_102321i<CC> Uv_3arg) const //native
  {
    s32 tmp = Uv_3arg.read();
    printf("Arg: %d\n", tmp);
  }

} //MFM
