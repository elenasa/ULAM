/**                                      -*- mode:C++ -*- */

//#include <iostream>
#include <stdio.h>

namespace MFM{

  template<class CC, u32 POS>
  void Uq_10173Bar<CC, POS>::Uf_5print(UlamContext<CC>& uc, T& Uv_4self, Ui_Ut_102323Int Uv_3arg)
  {
    s32 tmp = Uv_3arg.read();
    //std::cout << "Arg: " << tmp << std::endl;
    printf("Arg: 0x%x\n", tmp);
  }

} //MFM
