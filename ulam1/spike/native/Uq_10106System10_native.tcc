/**                                      -*- mode:C++ -*- */

#include <stdio.h>   //for printf
#include <stdlib.h>  //for abort

namespace MFM{


  template<class CC, u32 POS>
  void Uq_10106System10<CC, POS>::Uf_5print(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_10131i Uv_3arg) //native
  {
    s32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(3); //mask
    printf("Int(3) Arg: 0x%x\n", tmp);
  }


  template<class CC, u32 POS>
  void Uq_10106System10<CC, POS>::Uf_5print(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_10141i Uv_3arg) //native
  {
    s32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(4); //mask
    printf("Int(4) Arg: 0x%x\n", tmp);
  }

  template<class CC, u32 POS>
  void Uq_10106System10<CC, POS>::Uf_5print(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_102321i Uv_3arg) //native
  {
    s32 tmp = Uv_3arg.read();
    printf("Int Arg: %d\n", tmp);
  }


  template<class CC, u32 POS>
  void Uq_10106System10<CC, POS>::Uf_5print(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_102321u Uv_3arg) //native
  {
    u32 tmp = Uv_3arg.read();
    printf("Unsigned Arg: %u\n", tmp);
  }

  template<class CC, u32 POS>
  void Uq_10106System10<CC, POS>::Uf_5print(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_10131y Uv_3arg) //native
  {
    u32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(3); //mask
    u32 count1s = PopCount(tmp);
    printf("Unary(3) Arg: 0x%x\n", count1s);
  }


  template<class CC, u32 POS>
  void Uq_10106System10<CC, POS>::Uf_5print(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_10131b Uv_3arg) //native
  {
    u32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(3); //mask
    bool b = _Bool32ToCbool(tmp, 3);
    printf("Bool(3) Arg: 0x%x (%s)\n", tmp, b ? "true" : "false");
  }


  template<class CC, u32 POS>
  void Uq_10106System10<CC, POS>::Uf_6assert(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_10111b Uv_1b) //native
  {
    bool btmp = Uv_1b.read();
    printf("assert: arg is %d\n",btmp);
    if(!btmp)
      {
	abort();
      }
    printf("after assert's abort: arg is %d\n",btmp);
  }

} //MFM
