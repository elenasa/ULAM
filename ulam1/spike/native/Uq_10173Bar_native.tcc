/**                                      -*- mode:C++ -*- */

//#include <iostream>
#include <stdio.h>

namespace MFM{

  template<class CC, u32 POS>
  void Uq_10173Bar<CC, POS>::Uf_5print(T& Uv_4self, BitVector<32> Uv_3arg)
  {
    s32 tmp = Ui_Ut_102323Int::Read(Uv_3arg);
    //std::cout << "Arg: " << tmp << std::endl;
    printf("Arg: 0x%x\n", tmp);
  }	

} //MFM
