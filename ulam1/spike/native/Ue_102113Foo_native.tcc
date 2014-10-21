/**                                      -*- mode:C++ -*- */

//#include <iostream>
#include <stdio.h>

namespace MFM{

  template<class CC>
  void Ue_102113Foo<CC>::Uf_5print(T& Uv_4self, BitVector<32> Uv_3arg)
  {    
       s32 tmp = Ui_Ut_10143Int::Read(Uv_3arg);
       // std::cout << "Arg: " << tmp << std::endl;
       printf("Arg: 0x%x\n", tmp);
  }	


} //MFM